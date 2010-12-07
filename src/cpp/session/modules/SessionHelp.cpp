/*
 * SessionHelp.cpp
 *
 * Copyright (C) 2009-11 by RStudio, Inc.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 */

#include "SessionHelp.hpp"

#include <algorithm>

#include <boost/ref.hpp>
#include <boost/regex.hpp>
#include <boost/function.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/filter/aggregate.hpp>

#include <core/Error.hpp>
#include <core/Exec.hpp>
#include <core/Log.hpp>
#include <core/FilePath.hpp>

#include <core/http/Request.hpp>
#include <core/http/Response.hpp>
#include <core/http/URL.hpp>
#include <core/FileSerializer.hpp>

#define R_INTERNAL_FUNCTIONS
#include <r/RInternal.hpp>
#include <r/RSexp.hpp>
#include <r/RExec.hpp>
#include <r/RFunctionHook.hpp>

#include <session/SessionModuleContext.hpp>

// protect R against windows TRUE/FALSE defines
#undef TRUE
#undef FALSE

using namespace core;

namespace session {
namespace modules { 
namespace help {

namespace {   

// save computed help url prefix for comparison in rHelpUrlHandler
std::string s_localIP;
std::string s_localPort;
const char * const kHelpLocation = "/help";
const char * const kCustomLocation = "/custom";

std::string localURL(const std::string& address)
{
   return "http://" + address + ":" + s_localPort + "/";
}

bool isLocalURL(const std::string& url,
                const std::string& scope,
                std::string* pLocalURLPath)
{
   // first look for local ip prefix
   std::string urlPrefix = localURL(s_localIP);
   size_t pos = url.find(urlPrefix + scope);
   if (pos != std::string::npos)
   {
      *pLocalURLPath = url.substr(urlPrefix.length());
      return true;
   }

   // next look for localhost
   urlPrefix = localURL("localhost");
   pos = url.find(urlPrefix + scope);
   if (pos != std::string::npos)
   {
      *pLocalURLPath = url.substr(urlPrefix.length());
      return true;
   }

   // none found
   return false;
}

// replace the internal startHTTPD function (called from startDynamicHelp
// to run the http server). we simply record the ip/port info for future
// reference (comparision in rBrowseHelpUrlHandler)
SEXP startHTTPDHook(SEXP call, SEXP op, SEXP args, SEXP rho)
{
   // startHTTPD(ip, port)
   r::function_hook::checkArity(op, args, call);
   
   try
   {
      // save ip and port
      s_localIP = r::sexp::asString(CAR(args));
      s_localPort = r::sexp::asString(CADR(args));
   }
   CATCH_UNEXPECTED_EXCEPTION
  
   // return status 0L to indicate success
   r::sexp::Protect rProtect;
   return r::sexp::create(0, &rProtect);
}   

// replace the internal stopHTTPD function 
SEXP stopHTTPDHook(SEXP call, SEXP op, SEXP args, SEXP rho)
{   
   // stopHTTPD()
   r::function_hook::checkArity(op, args, call);
   
   return R_NilValue;
}    
   

// hook the browseURL function to look for calls to the R internal http
// server. for custom URLs remap the address to remote and then fire
// the browse_url event. for help URLs fire the appropraite show_help event
bool handleLocalHttpUrl(const std::string& url)
{
   // return false if the help url prefix hasn't been set yet
   if (s_localPort.empty())
      return false;
   
   // check for custom
   std::string customPath;
   if (isLocalURL(url, kCustomLocation, &customPath))
   {
      std::string customURL = "/" + customPath;
      ClientEvent event = browseUrlEvent(customURL);
      module_context::enqueClientEvent(event);
      return true;
   }

   // otherwise look for help (which would be all other localhost urls)
   std::string helpPath;
   if (isLocalURL(url, "", &helpPath))
   {
      ClientEvent helpEvent(client_events::kShowHelp, helpPath);
      module_context::enqueClientEvent(helpEvent);
      return true;
   }

   // wasn't a url of interest
   return false;
}
   
// As of R 2.10 RShowDoc still uses the legacy file::// mechanism for
// displaying the manual. Redirect these to the appropriate help event
bool handleRShowDocFile(const core::FilePath& filePath)
{
   boost::regex manualRegx(".*/lib/R/(doc/manual/[A-Za-z0-9_\\-]*\\.html)");
   boost::smatch match;
   if (regex_match(filePath.absolutePath(), match, manualRegx))
   {
      ClientEvent helpEvent(client_events::kShowHelp, match[1]);
      module_context::enqueClientEvent(helpEvent);
      return true;
   }
   else
   {
      return false;
   }
}
   
class HelpContentsFilter : public boost::iostreams::aggregate_filter<char>
{
public:
   typedef std::vector<char> Characters ;

   HelpContentsFilter(const http::Request& request)
   {
      requestUri_ = request.uri();
   }

   void do_filter(const Characters& src, Characters& dest)
   {
      std::string baseUrl = http::URL::uncomplete(
            requestUri_,
            kHelpLocation);

      // fixup hard-coded hrefs
      Characters tempDest;
      boost::algorithm::replace_all_copy(
            std::back_inserter(tempDest),
            boost::make_iterator_range(src.begin(), src.end()),
            "href=\"/",
            "href=\"" + baseUrl + "/");
      
      // fixup hard-coded src=
      boost::algorithm::replace_all_copy(
            std::back_inserter(dest),
            boost::make_iterator_range(tempDest.begin(), tempDest.end()),
            "src=\"/",
            "src=\"" + baseUrl + "/");
      
      // append javascript callbacks
      std::string js( "<script type=\"text/javascript\">\n"
                      "if (window.parent.helpNavigated)\n"
                      "   window.parent.helpNavigated(document, window);"
                      "</script>");
      std::copy(js.begin(), js.end(), std::back_inserter(dest));
   }
private:
   std::string requestUri_;
};

   
template <typename Filter>
void setDynamicContentResponse(const std::string& content,
                               const http::Request& request,
                               const Filter& filter,
                               http::Response* pResponse)
{
   // always attempt gzip
   if (request.acceptsEncoding(http::kGzipEncoding))
      pResponse->setContentEncoding(http::kGzipEncoding);
   
   // force cache revalidation since this is dynamic content
   pResponse->setCacheWithRevalidationHeaders();
   
   // set as cacheable content (uses eTag/If-None-Match)
   Error error = pResponse->setCacheableBody(content, request, filter);
   if (error)
   {
      pResponse->setError(http::status::InternalServerError,
                          error.code().message());
   }
}
   

void setDynamicContentResponse(const std::string& content,
                               const http::Request& request,
                               http::Response* pResponse)
{
   http::NullOutputFilter nullFilter;
   setDynamicContentResponse(content, request, nullFilter, pResponse);
}
   

template <typename Filter>
void handleHttpdResult(SEXP httpdSEXP, 
                       const http::Request& request, 
                       const Filter& htmlFilter,
                       http::Response* pResponse)
{
   // NOTE: this function is a port of process_request in Rhttpd.c
   // (that function is coupled to sending its results via the R http daemon, 
   // since we need to send the results via our daemon we need our own
   // implemetnation of the function). The port was completed 10/28/2009 so 
   // diffs in this function subsequent to that should be accounted for
   
   // defaults
   int code = 200;
   const char * const kTextHtml = "text/html";
   std::string contentType(kTextHtml);
   std::vector<std::string> headers;
   
   // if present, second element is content type
   if (LENGTH(httpdSEXP) > 1) 
   {
      SEXP ctSEXP = VECTOR_ELT(httpdSEXP, 1);     
      if (TYPEOF(ctSEXP) == STRSXP && LENGTH(ctSEXP) > 0)
         contentType = CHAR(STRING_ELT(ctSEXP, 0));
   }
   
   // if present, third element is headers vector
   if (LENGTH(httpdSEXP) > 2) 
   { 
      SEXP headersSEXP = VECTOR_ELT(httpdSEXP, 2);
      if (TYPEOF(headersSEXP) == STRSXP)
         r::sexp::extract(headersSEXP, &headers);
   }
   
   // if present, fourth element is HTTP code
   if (LENGTH(httpdSEXP) > 3) 
   {
      code = r::sexp::asInteger(VECTOR_ELT(httpdSEXP, 3));
   }
   
   // setup response
   pResponse->setStatusCode(code);
   pResponse->setContentType(contentType);

   // set headers
   std::for_each(headers.begin(), 
                 headers.end(),
                 boost::bind(&http::Response::setHeaderLine, pResponse, _1));
   
   // check payload
   SEXP payloadSEXP = VECTOR_ELT(httpdSEXP, 0);
   
   // payload = string
   if ((TYPEOF(payloadSEXP) == STRSXP || TYPEOF(payloadSEXP) == VECSXP) &&
        LENGTH(payloadSEXP) > 0)
   { 
      // get the names and the content string
      SEXP namesSEXP = r::sexp::getNames(httpdSEXP);
      std::string content;
      if (TYPEOF(payloadSEXP) == STRSXP)
         content = r::sexp::asString(STRING_ELT(payloadSEXP, 0));
      else if (TYPEOF(payloadSEXP) == VECSXP)
         content = r::sexp::asString(VECTOR_ELT(payloadSEXP, 0));
      
      // check for special file returns
      std::string fileName ;
      if (TYPEOF(namesSEXP) == STRSXP && LENGTH(namesSEXP) > 0 &&
          !std::strcmp(CHAR(STRING_ELT(namesSEXP, 0)), "file"))
      {
         fileName = content;
      }
      else if (LENGTH(payloadSEXP) > 1 && content == "*FILE*")
      {
         fileName = CHAR(STRING_ELT(payloadSEXP, 1));
      }
      
      // set the body
      if (!fileName.empty()) // from file
      {
         // get file path
         FilePath filePath(fileName);
         
         // cache with revalidation
         pResponse->setCacheWithRevalidationHeaders();
         
         // read file contents
         std::string contents;
         Error error = readStringFromFile(filePath, &contents);
         if (error)
         {
            pResponse->setError(error);
            return;
         }
          
         // set body (apply filter to html)
         if (pResponse->contentType() == kTextHtml)
         {
            pResponse->setCacheableBody(contents, request, htmlFilter);
         }
         else
         {
            pResponse->setCacheableBody(contents, request);
         }
      }
      else // from dynamic content
      {
         if (code == http::status::Ok)
         {
            // set body (apply filter to html)
            if (pResponse->contentType() == kTextHtml)
            {
               setDynamicContentResponse(content, 
                                         request, 
                                         htmlFilter, 
                                         pResponse);
            }
            else
            {
               setDynamicContentResponse(content, request, pResponse);
            }
         }
         else // could be a redirect or something else, don't interfere
         {
            pResponse->setBodyUnencoded(content);
         }
      }
   }
   
   // payload = raw buffer
   else if (TYPEOF(payloadSEXP) == RAWSXP)
   {
      std::string bytes((char*)(RAW(payloadSEXP)), LENGTH(payloadSEXP));
      setDynamicContentResponse(bytes, request, pResponse);
   }
   
   // payload = unexpected type
   else 
   {
      pResponse->setError(http::status::InternalServerError,
                          "Invalid response from R");
   }
}
 
   
// mirrors parse_query in Rhttpd.c
SEXP parseQuery(const http::Fields& fields, r::sexp::Protect* pProtect)
{
   if (fields.empty())
      return R_NilValue;
   else
      return r::sexp::create(fields, pProtect);
}

// mirrors parse_request_body in Rhttpd.c
SEXP parseRequestBody(const http::Request& request, r::sexp::Protect* pProtect)
{
   if (request.body().empty())
   {
      return R_NilValue;
   }
   else if (!request.formFields().empty())
   {
      return parseQuery(request.formFields(), pProtect);
   }
   else
   {
      // body bytes
      int contentLength = request.body().length();
      SEXP bodySEXP;
      pProtect->add(bodySEXP = Rf_allocVector(RAWSXP, contentLength));
      if (contentLength > 0)
         ::memcpy(RAW(bodySEXP), request.body().c_str(), contentLength);

      // content type
      if (!request.contentType().empty())
      {
         Rf_setAttrib(bodySEXP,
                      Rf_install("content-type"),
                      Rf_mkString(request.contentType().c_str()));
      }

      return bodySEXP;
   }
}

typedef boost::function<SEXP(const std::string&)> HandlerSource;


// NOTE: this emulates the calling portion of process_request in Rhttpd.c,
// to do this it uses low-level R functions and therefore must be wrapped
// in executeSafely
SEXP callHandler(const std::string& path,
                 const http::Request& request,
                 const HandlerSource& handlerSource,
                 r::sexp::Protect* pProtect)
{
   // uri decode the path
   std::string decodedPath = http::util::urlDecode(path, false);

   // construct "try(httpd(url, query, body), silent=TRUE)"

   SEXP trueSEXP;
   pProtect->add(trueSEXP = Rf_ScalarLogical(TRUE));
   SEXP queryStringSEXP = parseQuery(request.queryParams(), pProtect);
   SEXP requestBodySEXP = parseRequestBody(request, pProtect);

   SEXP callSEXP;
   pProtect->add(callSEXP = Rf_lang3(
         Rf_install("try"),
         Rf_lcons( (handlerSource(path)),
                   (Rf_list3(Rf_mkString(path.c_str()),
                             queryStringSEXP,
                             requestBodySEXP))),
         trueSEXP));

   SET_TAG(CDR(CDR(callSEXP)), Rf_install("silent"));

   // execute and return

   SEXP resultSEXP;
   pProtect->add(resultSEXP = Rf_eval(callSEXP,
                                      R_FindNamespace(Rf_mkString("tools"))));
   return resultSEXP;
}

template <typename Filter>
void handleHttpdRequest(const std::string& location,
                        const HandlerSource& handlerSource,
                        const http::Request& request, 
                        const Filter& filter,
                        http::Response* pResponse)
{
   // get the raw uri & strip its location prefix
   std::string uri = request.uri();
   if (!location.empty() && !uri.compare(0, location.length(), location))
      uri = uri.substr(location.length());

   // strip query string, will be passed separately
   size_t pos = uri.find("?");
   if (pos != std::string::npos)
      uri.erase(pos);
   
   // uri has now been reduced to path. url decode it (we noted that R
   // was url encoding dashes in e.g. help for memory-limits)
   std::string path = http::util::urlDecode(uri);

   if (path == "/library/R.css")
   {
      core::FilePath cssFile = options().rHelpCssFilePath();
      if (cssFile.exists())
      {
         pResponse->setFile(cssFile, request, filter);
         return;
      }
   }

   // evalute the handler
   r::sexp::Protect rp;
   SEXP httpdSEXP;
   Error error = r::exec::executeSafely<SEXP>(
         boost::bind(callHandler,
                        path,
                        boost::cref(request),
                        handlerSource,
                        &rp),
         &httpdSEXP);

   // error calling the function
   if (error)
   {
      pResponse->setError(http::status::InternalServerError,
                          error.code().message());
   }
   
   // error returned explicitly by httpd
   else if (TYPEOF(httpdSEXP) == STRSXP && LENGTH(httpdSEXP) > 0)
   {
      pResponse->setError(http::status::InternalServerError, 
                          r::sexp::asString(httpdSEXP));
   }
   
   // content returned from httpd
   else if (TYPEOF(httpdSEXP) == VECSXP && LENGTH(httpdSEXP) > 0)
   {
      handleHttpdResult(httpdSEXP, request, filter, pResponse);
   }
   
   // unexpected SEXP type returned from httpd
   else
   {
      pResponse->setError(http::status::InternalServerError,
                          "Invalid response from R");
   }
}

// this mirrors handler_for_path in Rhttpd.c. They cache the custom handlers
// env (not sure why). do the same for consistency
SEXP s_customHandlersEnv = NULL;
SEXP lookupCustomHandler(const std::string& uri)
{
   // pick name of handler out of uri
   boost::regex customRegx(".*/custom/([A-Za-z0-9_\\-]*).*");
   boost::smatch match;
   if (regex_match(uri, match, customRegx))
   {
      std::string handler = match[1];

      // load .httpd.handlers.env
      if (!s_customHandlersEnv)
      {
         s_customHandlersEnv = Rf_eval(Rf_install(".httpd.handlers.env"),
                                       R_FindNamespace(Rf_mkString("tools")));
      }

      // we only proceed if .httpd.handlers.env really exists
      if (TYPEOF(s_customHandlersEnv) == ENVSXP)
      {
         SEXP cl = Rf_findVarInFrame3(s_customHandlersEnv,
                                      Rf_install(handler.c_str()),
                                      TRUE);
         if (cl != R_UnboundValue && TYPEOF(cl) == CLOSXP) // need a closure
            return cl;
      }
   }

   // if we didn't find a handler then return handler lookup error
   return r::sexp::findFunction(".rs.handlerLookupError");
}

   
// .httpd.handlers.env
void handleCustomRequest(const http::Request& request, 
                         http::Response* pResponse)
{
   handleHttpdRequest("",
                      lookupCustomHandler,
                      request,
                      http::NullOutputFilter(),
                      pResponse);
}

// the ShowHelp event will result in the Help pane requesting the specified
// help url. we handle this request directly by calling the R httpd function
// to dynamically form the correct http response
void handleHelpRequest(const http::Request& request, http::Response* pResponse)
{
   handleHttpdRequest(kHelpLocation,
                      boost::bind(r::sexp::findFunction, "httpd", "tools"),
                      request,
                      HelpContentsFilter(request),
                      pResponse);
}
   
} // anonymous namespace
   
Error initialize()
{
   using boost::bind;
   using core::http::UriHandler;
   using namespace module_context;
   using namespace r::function_hook ;
   ExecBlock initBlock ;
   initBlock.addFunctions()
      (bind(registerReplaceHook, "startHTTPD", startHTTPDHook, (CCODE*)NULL))
      (bind(registerReplaceHook, "stopHTTPD", stopHTTPDHook, (CCODE*)NULL))
      (bind(registerRBrowseUrlHandler, handleLocalHttpUrl))
      (bind(registerRBrowseFileHandler, handleRShowDocFile))
      (bind(registerUriHandler, kHelpLocation, handleHelpRequest))
      (bind(registerUriHandler, kCustomLocation, handleCustomRequest))
      (bind(sourceModuleRFile, "SessionHelp.R"));
   return initBlock.execute();
}


} // namepsace help
} // namespace modules
} // namesapce session

