/*
 * ConsoleResources.java
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
package org.rstudio.studio.client.workbench.views.console;

import com.google.gwt.core.client.GWT;
import com.google.gwt.resources.client.ClientBundle;
import com.google.gwt.resources.client.CssResource;

public interface ConsoleResources extends ClientBundle
{
   public static final ConsoleResources INSTANCE = GWT.create(ConsoleResources.class);

   @CssResource.NotStrict
   ConsoleStyles consoleStyles();

   public interface ConsoleStyles extends CssResource
   {
      String console();
      String input();
      String prompt();
      String output();
      String command();
      String completionPopup();
      String completionGrid();
      String functionInfo();
      String functionInfoSignature();
      String functionInfoSummary();
      String paramInfoName();
      String paramInfoDesc();
      String promptFullHelp();
      String error();
      String selected();
   }
}
