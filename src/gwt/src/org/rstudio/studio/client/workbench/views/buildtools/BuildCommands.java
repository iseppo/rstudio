/*
 * BuildCommands.java
 *
 * Copyright (C) 2021 by RStudio, PBC
 *
 * Unless you have received this program directly from RStudio pursuant
 * to the terms of a commercial license agreement with RStudio, then
 * this program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 */
package org.rstudio.studio.client.workbench.views.buildtools;

import org.rstudio.core.client.resources.ImageResource2x;
import org.rstudio.studio.client.workbench.commands.Commands;
import org.rstudio.studio.client.workbench.model.SessionInfo;
import org.rstudio.studio.client.workbench.views.buildtools.ui.BuildPaneResources;

public class BuildCommands
{
   public static void setBuildCommandState(Commands commands, 
         SessionInfo sessionInfo)
   {
      // remove devtools commands if it isn't installed
      if (!sessionInfo.isDevtoolsInstalled())
      {
         commands.devtoolsLoadAll().remove();
      }
      
      // adapt or remove package commands if this isn't a package
      String type = sessionInfo.getBuildToolsType();
      if (type != SessionInfo.BUILD_TOOLS_PACKAGE)
      {
         commands.devtoolsLoadAll().remove();
         commands.buildSourcePackage().remove();
         commands.buildBinaryPackage().remove();
         commands.roxygenizePackage().remove();
         commands.checkPackage().remove();
         commands.testPackage().remove();
         commands.buildAll().setImageResource(
                           new ImageResource2x(
                              BuildPaneResources.INSTANCE.iconBuild2x()
                           ));
         commands.buildAll().setMenuLabel("_Build All");
         commands.buildAll().setButtonLabel("Build All");
         commands.buildAll().setDesc("Build all");
         
      }
      
      // remove makefile commands if this isn't a makefile
      if (type == SessionInfo.BUILD_TOOLS_CUSTOM ||
          type == SessionInfo.BUILD_TOOLS_WEBSITE ||
          type == SessionInfo.BUILD_TOOLS_QUARTO)
      {
         commands.rebuildAll().remove();
      }
      
      if (type == SessionInfo.BUILD_TOOLS_CUSTOM ||
          type == SessionInfo.BUILD_TOOLS_PACKAGE ||
          type == SessionInfo.BUILD_TOOLS_QUARTO)
      {
         commands.cleanAll().remove();
      }
      
      if (type != SessionInfo.BUILD_TOOLS_QUARTO)
      {
         commands.serveQuartoSite().remove();
      }
      
      if (type == SessionInfo.BUILD_TOOLS_QUARTO)
      {
         String projType = "Project";
         
         if (sessionInfo.getQuartoConfig().project_type.equals(
                      SessionInfo.QUARTO_PROJECT_TYPE_BOOK)) 
         {
            projType = "Book";
         }
         if (sessionInfo.getQuartoConfig().project_type.equals(
               SessionInfo.QUARTO_PROJECT_TYPE_WEBSITE)) 
         {
            projType = "Website";
         }
         commands.buildAll().setMenuLabel("_Render " + projType);
         commands.buildAll().setButtonLabel("Render " + projType);
         commands.buildAll().setDesc("Render " + projType.toLowerCase());
         commands.buildAll().setImageResource(commands.quartoRenderDocument().getImageResource());
         commands.serveQuartoSite().setMenuLabel("_Serve " + projType);
         commands.serveQuartoSite().setButtonLabel("Serve " + projType);
      }
      
      // remove all other commands if there are no build tools
      if (type == SessionInfo.BUILD_TOOLS_NONE)
      {
         commands.buildAll().remove();
         commands.rebuildAll().remove();
         commands.cleanAll().remove();
         commands.stopBuild().remove();
         commands.activateBuild().remove();
         commands.layoutZoomBuild().remove();
         commands.clearBuild().remove();
      }
   }
}
