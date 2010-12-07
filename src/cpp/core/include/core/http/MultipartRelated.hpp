/*
 * MultipartRelated.hpp
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

#ifndef CORE_HTTP_MULTIPART_RELATED_HPP
#define CORE_HTTP_MULTIPART_RELATED_HPP

#include <string>
#include <sstream>

#include <boost/utility.hpp>

namespace core {
namespace http {

class MultipartRelated : boost::noncopyable
{
public:
   MultipartRelated() {}
   // COPYING: boost::noncoypable

public:
   void addPart(const std::string& contentType,
                const std::string& body);

   void terminate();

   std::string contentType() const ;
   std::string body() const;

private:
   std::ostringstream bodyStream_;
};

} // namespace http
} // namespace core


#endif // CORE_HTTP_MULTIPART_RELATED_HPP
