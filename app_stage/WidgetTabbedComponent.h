/*
 Copyright 2010-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __WIDGET_TABBED_COMPONENT_H__
#define __WIDGET_TABBED_COMPONENT_H__

#include "Network.h"
#include "juce.h"
#include "WidgetViewComponent.h"
#include "TabbedComponent2.h"
#include "AlertDialogComponent.h"
#include "StatusDialogComponent.h"

class MainComponent;
class XMLRPCManager;
class StatusDialogComponent;

//==============================================================================


class WidgetTabbedComponent : public TabbedComponent2
{
public:
    //==============================================================================

    WidgetTabbedComponent(const TabbedButtonBar2::Orientation orientation);
    virtual ~WidgetTabbedComponent();

    void clearTabs();
    void clearHelp();
    
    //==============================================================================
    // mode functions
    //
    void setEditMode(const bool editMode);
    bool getEditMode() { return editMode_; }
    void setToolMode(CommandID toolMode, const MouseCursor& mouseCursor);
    void toggleGreyOutsideCanvas();
    void setGreyOutsideCanvas(bool greyOutsideCanvas);
    bool getGreyOutsideCanvas();
    void setDisplayOrientation(Desktop::DisplayOrientation orientation);
    void setShowAddButton(bool show);
    void setShowCloseButtons(bool show);
    bool getShowCloseButtons();
    void updateAddCloseButtons();
    void setRightMargin(int newMargin);
    void setTabVisible(int tabIndex, bool isVisible);
    void setTabVisibleXml(int tabIndex, bool isVisible);
    bool getTabVisible(int tabIndex);
    bool getTabVisibleXml(int tabIndex);
    void ensureCurrentTabVisible();
	void updateTabVisibilityFromPreferences();
    void updateShowAllTabs();
	bool isAspectModeVisible(int aspectMode);
    
    //==============================================================================
    // tab functions
    //
    void initTab(const String &tabName);
    void userAddTab(const String &tabName, float canvasWidthAspect = 4, float canvasHeightAspect = 3, int canvasAspectMode = 1);
    void userMoveTab(int currentTabIndex, int newTabIndex);
    void userRemoveTab(const int tabIndex, Component *componentToCentreAround);
    void userSetTabProperties(int tabIndex);

    //==============================================================================
    // overriden component functions
    //
    void currentTabChanged(const int newCurrentTabIndex, const String &newCurrentTabName);
    TabBarButton2* createTabButton(const String &tabName, const int tabIndex);
    void mouseDown(const MouseEvent &e);

    //==============================================================================
    // XMLRPC management functions
    //
    void setXMLRPCManager(XMLRPCManager* manager);
    void receiveTabChanges();
    void receiveTabAndWidgets(int tabIndex, std::map<unsigned,String> &updateTabXml, 
                              int numTabsTotal, int numWidgetsTotal, int &numWidgetsReceived,
                              StatusDialogComponent *statusDialogComponent);
    void getTabVisibleFromXml(int tabIndex, XmlElement* tabXmlElement);

#if STAGE_BUILD==DESKTOP
    // tab loading and saving
    void importTabs();
    void exportTab();
    void exportTabs();
#endif // STAGE_BUILD==DESKTOP    
private:
    
    bool editMode_;
    CommandID toolMode_;
    MouseCursor editMouseCursor_;
    bool greyOutsideCanvas_;
    XMLRPCManager* XMLRPCManager_;
    bool toolsOperateTabs_;
    bool showAddButton_;
    bool showCloseButtons_;
    bool doNotShowDeleteConfirm_;
    String sessionID_;

#if STAGE_BUILD==DESKTOP
    // file for importing and exporting tabs
    File xmlFile;
    File currentDirectory_;
#endif // STAGE_BUILD==DESKTOP
};



#endif // __WIDGET_TABBED_COMPONENT_H__
