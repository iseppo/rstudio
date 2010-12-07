/*
 * PlotsState.java
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
package org.rstudio.studio.client.workbench.views.plots.model;

import com.google.gwt.core.client.JavaScriptObject;

public class PlotsState extends JavaScriptObject
{
   protected PlotsState()
   {
   }
 
   public final native String getFilename() /*-{
      return this.filename;
   }-*/;
   
   public final native int getWidth() /*-{
      return this.width;
   }-*/;
   
   public final native int getHeight() /*-{
      return this.height;
   }-*/;
     
   public final native int getPlotIndex() /*-{
      return this.plotIndex;
   }-*/;
 
   public final native int getPlotCount() /*-{
      return this.plotCount;
   }-*/;
   
   public final native boolean getActivatePlots() /*-{
      return this.activatePlots;
   }-*/;
}
