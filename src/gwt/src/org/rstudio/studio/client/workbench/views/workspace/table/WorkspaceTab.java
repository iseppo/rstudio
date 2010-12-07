/*
 * WorkspaceTab.java
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
package org.rstudio.studio.client.workbench.views.workspace.table;

import com.google.inject.Inject;
import org.rstudio.studio.client.workbench.ui.DelayLoadTabShim;
import org.rstudio.studio.client.workbench.ui.DelayLoadWorkbenchTab;
import org.rstudio.studio.client.workbench.views.workspace.Workspace;

public class WorkspaceTab extends DelayLoadWorkbenchTab<Workspace>
{
   public abstract static class Shim
         extends DelayLoadTabShim<Workspace, WorkspaceTab> {}

   @Inject
   public WorkspaceTab(Shim shim)
   {
      super("Workspace", shim);
   }
}
