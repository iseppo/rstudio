/*
 * Debug.java
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
package org.rstudio.core.client;

import com.google.gwt.core.client.GWT;
import com.google.gwt.core.client.JavaScriptObject;
import com.google.gwt.json.client.JSONObject;
import org.rstudio.studio.client.server.ServerError;

public class Debug
{
   public static native void injectDebug() /*-{
      $Debug = {
         log: function(message) {
            @org.rstudio.core.client.Debug::log(Ljava/lang/String;)(message);
         }
      };
   }-*/;

   public static void log(String message)
   {
      GWT.log(message, null) ;
      logToConsole(message) ;
   }
   
   public static native void logToConsole(String message) /*-{
    if (typeof(console) != "undefined")
    {
         console.log(message) ;
    }
   }-*/ ;

   public static <T> T printValue(String label, T value)
   {
      Debug.log(label + '=' + value);
      return value;
   }

   public static void printStackTrace(String label)
   {
      StringBuffer buf = new StringBuffer(label + "\n");
      for (StackTraceElement ste : new Throwable().getStackTrace())
      {
         buf.append("\tat " + ste + "\n");
      }
      log(buf.toString());
   }

   public static void logError(ServerError error)
   {
      Debug.log(error.toString());
   }

   /**
    * Same as log() but for short-term messages that should not be checked
    * in. Making this a different method from log() allows devs to use Find
    * Usages to get rid of all calls before checking in changes. 
    */
   public static void devlog(String label)
   {
      log(label);
   }

   public static <T> T devlog(String label, T passthrough)
   {
      Debug.devlog(label);
      return passthrough;
   }

   public static void dump(JavaScriptObject object)
   {
      Debug.log(new JSONObject(object).toString());
   }
}
