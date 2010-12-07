/*
 * SessionOptions.hpp
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

#ifndef SESSION_SESSION_OPTIONS_HPP
#define SESSION_SESSION_OPTIONS_HPP

#include <string>

#include <boost/utility.hpp>

#include <core/SafeConvert.hpp>
#include <core/FilePath.hpp>
#include <core/system/System.hpp>

#include <R_ext/RStartup.h>

#include <session/SessionConstants.hpp>

namespace core {
   class ProgramStatus;
}

namespace session {
 

// singleton
class Options;
Options& options();
   
class Options : boost::noncopyable
{
private:
   Options()
   {
   }
   friend Options& options() ;
   
   // COPYING: boost::noncopyable

public:
   // read options  
   core::ProgramStatus read(int argc, char * const argv[]);   
   virtual ~Options() {}
   
   
   std::string programIdentity() const 
   { 
      return std::string(programIdentity_.c_str()); 
   }
   
   std::string programMode() const 
   { 
      return std::string(programMode_.c_str()); 
   }
   
   // agreement
   core::FilePath agreementFilePath() const
   { 
      if (!agreementFilePath_.empty())
         return core::FilePath(agreementFilePath_.c_str());
      else
         return core::FilePath();
   }
   
   // www
   std::string wwwLocalPath() const
   {
      return wwwLocalPath_;
   }

   std::string wwwPort() const
   {
      return std::string(wwwPort_.c_str());
   }

   std::string sharedSecret() const
   {
      return std::string(secret_.c_str());
   }

   int timeoutMinutes() const { return timeoutMinutes_; }

   unsigned int minimumUserId() const { return 100; }
   
   core::FilePath coreRSourcePath() const 
   { 
      return core::FilePath(coreRSourcePath_.c_str());
   }
   
   core::FilePath modulesRSourcePath() const 
   { 
      return core::FilePath(modulesRSourcePath_.c_str()); 
   }
   
   std::string rLibsUser() const
   {
      return std::string(rLibsUser_.c_str());
   }

   std::string rCRANRepos() const
   {
      return std::string(rCRANRepos_.c_str());
   }

   int rCompatibleGraphicsEngineVersion() const
   {
      return rCompatibleGraphicsEngineVersion_;
   }

   core::FilePath rHelpCssFilePath() const
   {
      return core::FilePath(rHelpCssFilePath_.c_str());
   }

   bool rShellEscape() const
   {
      return rShellEscape_;
   }

   bool autoReloadSource() const { return autoReloadSource_; }

   SA_TYPE saveWorkspace() const
   {
      if (saveWorkspace_ == "yes")
         return SA_SAVE;
      else if (saveWorkspace_ == "no")
         return SA_NOSAVE;
      else
         return SA_SAVEASK;
   }
   
   // limits
   int limitFileUploadSizeMb() const { return limitFileUploadSizeMb_; }
   int limitCpuTimeMinutes() const { return limitCpuTimeMinutes_; }

   int limitRpcClientUid() const { return limitRpcClientUid_; }

   bool limitXfsDiskQuota() const { return limitXfsDiskQuota_; }
   
   // external
   core::FilePath rpostbackPath() const
   {
      return core::FilePath(rpostbackPath_.c_str());
   }
   
   // user info
   std::string userIdentity() const 
   { 
      return std::string(userIdentity_.c_str()); 
   }
   
   core::FilePath userHomePath() const 
   { 
      return core::FilePath(userHomePath_.c_str());
   }
   
   core::FilePath userScratchPath() const 
   { 
      return core::FilePath(userScratchPath_.c_str()); 
   }

   core::FilePath userLogPath() const
   {
      return userScratchPath().childPath("log");
   }

private:
   // program
   std::string programIdentity_;
   std::string programMode_;

   // agreement
   std::string agreementFilePath_;
   
   // www
   std::string wwwLocalPath_;
   std::string wwwPort_;

   // session
   std::string secret_;
   int timeoutMinutes_;

   // r
   std::string coreRSourcePath_;
   std::string modulesRSourcePath_;
   std::string rLibsUser_;
   std::string rCRANRepos_;
   bool autoReloadSource_ ;
   std::string saveWorkspace_ ;
   int rCompatibleGraphicsEngineVersion_;
   std::string rHelpCssFilePath_;
   bool rShellEscape_;
   
   // limits
   int limitFileUploadSizeMb_;
   int limitCpuTimeMinutes_;
   int limitRpcClientUid_;
   bool limitXfsDiskQuota_;
   
   // external
   std::string rpostbackPath_;
   
   // user info
   std::string userIdentity_;
   std::string userHomePath_;
   std::string userScratchPath_;   
};
  
} // namespace session

#endif // SESSION_SESSION_OPTIONS_HPP

