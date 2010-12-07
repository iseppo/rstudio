/*
 * ServerValidateUser.cpp
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

#include <server/auth/ServerValidateUser.hpp>

#include <core/Error.hpp>
#include <core/Log.hpp>
#include <core/StringUtils.hpp>

#include <server/util/system/System.hpp>
#include <server/util/system/User.hpp>

#include <server/ServerOptions.hpp>

using namespace core;

namespace server {
namespace auth {

bool validateUser(const std::string& username)
{
   // short circuit if we aren't validating users
   if (!server::options().authValidateUsers())
      return true;
   
   // get the user
   util::system::user::User user;
   Error error = userFromUsername(username, &user);
   if (error)
   {
      // log the error only if it is unexpected
      if (!util::system::isUserNotFoundError(error))
         LOG_ERROR(error);

      // not found either due to non-existence or an unexpected error
      return false;
   }

   // validate user if necessary
   std::string requiredGroup = server::options().authRequiredUserGroup();
   if (!requiredGroup.empty())
   {    
      // see if they are a member of the "rstudio_users" group
      bool belongsToGroup ;
      error = util::system::userBelongsToGroup(username,
                                               requiredGroup,
                                               &belongsToGroup);
      if (error)
      {
         // log and return false
         LOG_ERROR(error);
         return false;
      }
      else
      {
         // return belongs status
         return belongsToGroup;
      }
   }
   else
   {
      // not validating (running in some type of dev mode where we
      // don't have a system account for every login)
      return true;
   }
}

} // namespace auth
} // namespace server



