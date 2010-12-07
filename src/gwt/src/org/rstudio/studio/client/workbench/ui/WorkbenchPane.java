/*
 * WorkbenchPane.java
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
package org.rstudio.studio.client.workbench.ui;

import com.google.gwt.user.client.Command;
import org.rstudio.studio.client.workbench.WorkbenchView;

public abstract class WorkbenchPane extends ToolbarPane
                                 implements WorkbenchView,
                                            WorkbenchTab
{
   protected WorkbenchPane(String title)
   {
      title_ = title ;
   }

   public void prefetch(Command continuation)
   {
      continuation.execute();
   }

   public String getTitle()
   {
      return title_ ;
   }

   // hook for subclasses to be notified right before & after they are selected
   public void onBeforeSelected()
   {
   }
   public void onSelected()
   {
   }

   private String title_;
}
