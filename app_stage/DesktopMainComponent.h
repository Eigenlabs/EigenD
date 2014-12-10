/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  7 Sep 2012 1:26:24pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_MAINCOMPONENT_DESKTOPMAINCOMPONENT_88B1275B__
#define __JUCER_HEADER_MAINCOMPONENT_DESKTOPMAINCOMPONENT_88B1275B__

//[Headers]     -- You can add your own extra header files here --
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

#include "Network.h"
#include "juce.h"
#include "WidgetViewComponent.h"
#include "WidgetTabbedComponent.h"
#include "Main.h"
#include "StagePreferences.h"
#include "ToolManager.h"
#include "AgentViewComponent.h"
#include "EigenLookAndFeel.h"
#include "AgentTreeView.h"
#include "HelpViewComponent.h"
#include "StatusDialogComponent.h"
#include "PreferencesDialogComponent.h"


class StageWindow;
class ToolManager;
class WidgetTabbedComponent;
class AgentViewComponent;
class HelpViewComponent;
class OSCManager;
class XMLRPCManager;
class EigenLookAndFeel;
class StatusDialogComponent;
class MainComponent;

//==============================================================================
//[/Headers]

#include "TabsFrameBox.h"
#include "BackgroundComponent.h"


//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class MainComponent  : public Component,
                       public DragAndDropContainer,
                       public MenuBarModel,
                       public ApplicationCommandTarget,
                       public MessageListener,
                       public Button::Listener
{
public:
    //==============================================================================
    MainComponent ();
    ~MainComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.

    void setEditMode(bool editMode);

    bool getEditMode() { return editMode_; }

    bool getLockMode() { return lockMode_; }

    bool keyPressed(const KeyPress& key);
    void modifierKeysChanged(const ModifierKeys &modifiers);

    void setMainWindow(StageWindow* stageWindow) { stageWindow_ = stageWindow; }
    StageWindow* getMainWindow() { return stageWindow_; }

    OSCManager* getOSCManager() { return OSCManager_; }
    XMLRPCManager* getXMLRPCManager() { return XMLRPCManager_; }
    WidgetTabbedComponent* getTabbedComponent() { return tabbedComponent; }
    AgentViewComponent* getAgentViewComponent() { return agentView_; }
    HelpViewComponent* getHelpViewComponent() { return helpView_; }

    void openAgentTree();
    void closeAgentTree();
    void openHelp();
    void closeHelp();

    void userEditPreferences();

    void initializeModel();
    void shutdown();
    bool nativeFullScreenSupport();
    
    String getVersion() { return JUCEApplication::getInstance()->getApplicationVersion(); }

    juce::Rectangle<int> getToolBarBounds(bool locked);

    //==============================================================================
    // inherited MenuBarModel functions

    StringArray getMenuBarNames();

    PopupMenu getMenuForIndex (int menuIndex, const String& menuName);

    void menuItemSelected (int menuItemID,
                           int topLevelMenuIndex);

    //==============================================================================
    // The following methods implement the ApplicationCommandTarget interface, allowing
    // this window to publish a set of actions it can perform, and which can be mapped
    // onto menus, keypresses, etc.

    // commands
    enum CommandIDs
    {
        // file menu (TODO)
        fileConnect             = 0x10000,
        fileImportTabs          = 0x10001,
        fileExportTab           = 0x10002,
        fileExportTabs          = 0x10003,
        filePreferences         = 0x10004,
        fileQuit                = 0x10005,

        // edit menu
        editNewTab              = 0x20000,
        editTabProperties       = 0x20001,
        editCloseTab            = 0x20002,
        editToggleLock          = 0x20003,

        // tool menu
        toolPerform             = 0x30000,
        toolCreate              = 0x30001,
        toolEdit                = 0x30002,
        toolMove                = 0x30003,
        toolSize                = 0x30004,
        toolDelete              = 0x30005,
        toolHelp                = 0x30006,
        toolMulti               = 0x30007,

        // view menu
        viewToggleFullScreen    = 0x40000,
        viewGreyOutsideCanvas   = 0x40001,

        // help menu
        helpStageHelp           = 0x50000,
        helpStageQuickStart     = 0x50001,

    };

    ApplicationCommandTarget* getNextCommandTarget();

    void getAllCommands (Array <CommandID>& commands);

    void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result);

    bool perform (const InvocationInfo& info);

    void setToolMode(CommandID toolMode);

    void toggleFullScreenMode();
    void setFullScreenMode(bool fullScreenMode);

    CommandID getToolMode() {
        return toolMode_;
    }

    //==============================================================================
    // inherited MouseListener functions
/*
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseDoubleClick (const MouseEvent& e);
*/
    void bringAgentViewToFront();
    void handleMessage(const Message& message);

    friend class eigenDPinger;
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);

    // Binary resources:
    static const char* lock_open_png;
    static const int lock_open_pngSize;
    static const char* lock_closed_png;
    static const int lock_closed_pngSize;
    static const char* dots_png;
    static const int dots_pngSize;
    static const char* fullscreen_off_png;
    static const int fullscreen_off_pngSize;
    static const char* fullscreen_on_png;
    static const int fullscreen_on_pngSize;
    static const char* logoText_2x_png;
    static const int logoText_2x_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    StageWindow* stageWindow_;

    bool editMode_;
    bool lockMode_;
    bool treeOpen_;

    CommandID toolMode_;
    CommandID prevToolMode_;

    XmlElement* treeXml;
    TreeViewItem* rootItem;

    int treeViewWidth_;
    int lastTreeViewWidth_;
    int relDragStartX_;

    DrawableButton* lockButton_;
    Toolbar* toolbar_;
    ToolManager* toolManager_;

    AgentViewComponent* agentView_;
    HelpViewComponent* helpView_;

    // animator for sliding toolbar
    ComponentAnimator* animator_;

    // tooltip window for displaying tooltips
    TooltipWindow tooltipWindow_;

    // xml rpc manager
    XMLRPCManager* XMLRPCManager_;

    // osc manager
    OSCManager* OSCManager_;

public:
    // Stage preferences
    StagePreferences* preferences_;

private:
    //[/UserVariables]

    //==============================================================================
    WidgetTabbedComponent* tabbedComponent;
    TabsFrameBox* tabsFrameBox;
    BackgroundComponent* toolbarHider;
    ImageButton* imageButton2;
    ImageButton* fullScreenButton2;
    Image cachedImage_logoText_2x_png;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent);
};


#endif   // __JUCER_HEADER_MAINCOMPONENT_DESKTOPMAINCOMPONENT_88B1275B__
