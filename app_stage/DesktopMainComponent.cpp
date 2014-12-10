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

//[Headers] You can add your own extra header files here...
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

#include "TreeTestData.h"
#include "Main.h"
//[/Headers]

#include "DesktopMainComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

// TODO:
//  1. ctrl-Tab and shift-ctrl-Tab to move through tabs
//  2. help tool command key in tools menu

//[/MiscUserDefs]

//==============================================================================
MainComponent::MainComponent ()
    : Component ("Main"),
      tabbedComponent (0),
      tabsFrameBox (0),
      toolbarHider (0),
      imageButton2 (0),
      fullScreenButton2 (0),
      cachedImage_logoText_2x_png (0)
{
    addAndMakeVisible (tabbedComponent = new WidgetTabbedComponent (TabbedButtonBar2::TabsAtTop));
    tabbedComponent->setTabBarDepth (24);
    tabbedComponent->setCurrentTabIndex (-1);

    addAndMakeVisible (tabsFrameBox = new TabsFrameBox());
    addAndMakeVisible (toolbarHider = new BackgroundComponent());
    addAndMakeVisible (imageButton2 = new ImageButton ("new button"));
    imageButton2->addListener (this);

    imageButton2->setImages (false, true, true,
                             Image(), 1.0000f, Colour (0x0),
                             Image(), 1.0000f, Colour (0x0),
                             Image(), 1.0000f, Colour (0x0));
    if(!nativeFullScreenSupport())
    {
        fullScreenButton2 = new ImageButton ("full screen button");

        addAndMakeVisible (fullScreenButton2);
        fullScreenButton2->setButtonText ("new button");
        fullScreenButton2->addListener (this);

        fullScreenButton2->setImages (false, true, true,
                ImageCache::getFromMemory (fullscreen_off_png, fullscreen_off_pngSize), 1.0000f, Colour (0x0),
                Image(), 1.0000f, Colour (0x0),
                Image(), 1.0000f, Colour (0x0));
    }
    cachedImage_logoText_2x_png = ImageCache::getFromMemory (logoText_2x_png, logoText_2x_pngSize);
    //[UserPreSize]
    //--------------------------------------------------------------------------
    // non-Jucer component initialization

    XMLRPCManager_ = 0;
    OSCManager_ = 0;
    preferences_ = 0;

    tabsFrameBox->setVisible(false);

    addAndMakeVisible(toolbar_=new Toolbar());
    toolbar_->setStyle(Toolbar::iconsOnly);
    toolbar_->setTopLeftPosition(10,10);
    toolManager_=new ToolManager(this, toolbar_);
    toolbar_->addDefaultItems(*toolManager_);
    toolbar_->toBack();
    toolbar_->setColour(Toolbar::backgroundColourId, Colour(0xff000000));
    toolbar_->setColour(Toolbar::buttonMouseOverBackgroundColourId, Colour(0xff000000));
    toolbar_->setColour(Toolbar::buttonMouseDownBackgroundColourId, Colour(0xff000000));

    lockButton_ = new DrawableButton("lock button", DrawableButton::ImageStretched);
    addAndMakeVisible(lockButton_);
    Drawable* lockImage = Drawable::createFromImageData(lock_closed_png, lock_closed_pngSize);
    Drawable* unlockImage = Drawable::createFromImageData(lock_open_png, lock_open_pngSize);
    lockButton_->setImages(unlockImage, 0, 0, 0,
                           lockImage, 0, 0, 0);
    lockButton_->setColour(DrawableButton::backgroundColourId, Colour(0));
    lockButton_->setColour(DrawableButton::backgroundOnColourId, Colour(0));
    lockButton_->addListener(this);

    animator_ = new ComponentAnimator();


    //--------------------------------------------------------------------------
    //[/UserPreSize]

    setSize (951, 758);


    //[Constructor] You can add your own custom stuff here..
    //--------------------------------------------------------------------------


    //--------------------------------------------------------------------------
    // GUI initialization

    // edit mode is true
    lockMode_ = true;
    editMode_ = false;
    toolMode_ = toolPerform;
    prevToolMode_ = toolPerform;

    tabbedComponent->setOutline(0);
    tabbedComponent->setShowAddButton(true);
    tabbedComponent->getTabbedButtonBar().hideAddCloseButtons(lockMode_);
    tabbedComponent->setEditMode(editMode_);
    tabbedComponent->setRightMargin(64);
    toolManager_->setTool(toolMode_);
    tabbedComponent->setToolMode(toolMode_, toolManager_->getMouseCursor(toolMode_));


    lockButton_->setToggleState(lockMode_, false);

    // listen to mouse events from the resizer
    //paneResizer->addMouseListener(this, false);

    //paneResizer->setVisible(false);

    treeViewWidth_ = -16;
    lastTreeViewWidth_ = 0;
    relDragStartX_ = 0;

    setWantsKeyboardFocus(true);

    // agent view window
    agentView_ = new AgentViewComponent();
    agentView_->setMainComponent(this);
    agentView_->setOpaque(false);
#ifdef WIN32
	agentView_->addToDesktop(0);
#else
	agentView_->addToDesktop(ComponentPeer::windowHasDropShadow);
#endif // WIN32
//    agentView_->setAlwaysOnTop(true);
    agentView_->toFront(false);

    // help view window
    helpView_ = new HelpViewComponent();
    helpView_->setOpaque(false);
#ifdef WIN32
	helpView_->addToDesktop(0);
#else
	helpView_->addToDesktop(ComponentPeer::windowHasDropShadow);
#endif // WIN32
    //    helpView_->setAlwaysOnTop(true);
    helpView_->toFront(false);


    // tool mode is perform, invoke asyncronously so that the main window is positioned first
    invokeDirectly(toolPerform, true);

    // tooltips
    lockButton_->setTooltip("Toggle canvas lock to allow/prevent edits");
    if(fullScreenButton2)
    {
        fullScreenButton2->setTooltip("Toggle fullscreen mode");
    }

    //--------------------------------------------------------------------------
    //[/Constructor]
}

MainComponent::~MainComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..

    // TODO: remove
    // save the tabs on exit
    //tabbedComponent->sendTabsToEigenD();

    //[/Destructor_pre]

    deleteAndZero (tabbedComponent);
    deleteAndZero (tabsFrameBox);
    deleteAndZero (toolbarHider);
    deleteAndZero (imageButton2);
    if(fullScreenButton2)
    {
        deleteAndZero (fullScreenButton2);
    }


    //[Destructor]. You can add your own custom destruction code here..
    deleteAndZero(toolbar_);
    deleteAndZero(lockButton_);
    deleteAndZero(agentView_);
    deleteAndZero(helpView_);
    deleteAndZero(animator_);

    deleteAndZero(XMLRPCManager_);
    deleteAndZero(OSCManager_);

    if (preferences_)
    {
        preferences_->saveIfNeeded();
        deleteAndZero(preferences_);
    }
    //[/Destructor]
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::black);

    g.setColour (Colours::black);
    g.drawImageWithin (cachedImage_logoText_2x_png,
            getWidth() - 31 - 133, 4, 133, 68,
            RectanglePlacement::stretchToFit,
            false);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainComponent::resized()
{
    int offset = -39;
    if(fullScreenButton2)
    {
        offset = 0;
        fullScreenButton2->setBounds (50, 16, 31, 31);
    }
    tabbedComponent->setBounds (16, 60, getWidth() - 32, getHeight() - 76);
    tabsFrameBox->setBounds (16, 84, getWidth() - 32, getHeight() - 100);
    toolbarHider->setBounds (offset, -6, 96, 64);
    imageButton2->setBounds (48, 24, 24, 24);
    //[UserResized] Add your own custom resize handling here..

    lockButton_->setBounds(24, 23, 18, 17);

    if (!animator_->isAnimating(toolbar_))
    {
        toolbar_->setBounds(getToolBarBounds(lockMode_));
    }

    // the Jucer sizing above is the default sizing and is overwritten below

    // resize tabbed component to fill window
    tabbedComponent->setBounds (16, 60, getWidth() - 32, getHeight() - 76);
    tabsFrameBox->setBounds (16, 84, getWidth() - 32, getHeight() - 100);


    //[/UserResized]
}

void MainComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == imageButton2)
    {
        //[UserButtonCode_imageButton2] -- add your button handler code here..
        //[/UserButtonCode_imageButton2]
    }
    else if (buttonThatWasClicked == fullScreenButton2)
    {
        //[UserButtonCode_fullScreenButton2] -- add your button handler code here..
        invokeDirectly(viewToggleFullScreen, false);
        //[/UserButtonCode_fullScreenButton2]
    }

    //[UserbuttonClicked_Post]
    if (buttonThatWasClicked == lockButton_)
    {
        invokeDirectly(editToggleLock, false);
    }
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

bool MainComponent::nativeFullScreenSupport()
{
    SystemStats::OperatingSystemType os = SystemStats::getOperatingSystemType();
    if((os & SystemStats::Windows) != 0 || os == SystemStats::Linux || os == SystemStats::MacOSX_10_4 || os == SystemStats::MacOSX_10_5 || os == SystemStats::MacOSX_10_6)
    {
        return false;
    }
    return true;
}

juce::Rectangle<int> MainComponent::getToolBarBounds(bool locked)
{
    int offset = -39;
    if(fullScreenButton2)
    {
        offset = 0;
    }
    if(!locked)
    {
        return juce::Rectangle<int>(96+offset,13,240,37);
    }
    else
    {
        return juce::Rectangle<int>(-240,13,240,37);
    }
}

void MainComponent::openAgentTree()
{
    int screenWidth = stageWindow_->getParentWidth();

    if (stageWindow_->getRight()+16+agentView_->getWidth() > screenWidth)
    {
        agentView_->setTopLeftPosition(screenWidth-agentView_->getWidth(), stageWindow_->getY()+86);
    }
    else
    {
        agentView_->setTopLeftPosition(stageWindow_->getRight()+16, stageWindow_->getY()+86);
    }
    agentView_->setSize(agentView_->getWidth(), tabbedComponent->getHeight()-20);
    agentView_->setVisible(true);

/*
    if(!treeOpen_)
    {
        treeViewWidth_=256;
        treeOpen_ = true;
        setSize(getWidth()+treeViewWidth_, getHeight());
    }
  */
}


void MainComponent::closeAgentTree()
{
    agentView_->setVisible(false);

    /*
    if(treeOpen_)
    {
        setSize(getWidth()-treeViewWidth_, getHeight());
        treeOpen_ = false;
        treeViewWidth_= -16;
    }
     */

}

void MainComponent::openHelp()
{
    int screenWidth = stageWindow_->getParentWidth();

    if (stageWindow_->getRight()+16+helpView_->getWidth() > screenWidth)
    {
        helpView_->setTopLeftPosition(screenWidth-helpView_->getWidth(), stageWindow_->getY()+86);
    }
    else
    {
        helpView_->setTopLeftPosition(stageWindow_->getRight()+16, stageWindow_->getY()+86);
    }
    helpView_->setSize(helpView_->getWidth(), tabbedComponent->getHeight()-20);

    helpView_->setVisible(true);
}

void MainComponent::closeHelp()
{
    helpView_->setVisible(false);
}

void MainComponent::setEditMode(bool editMode)
{
    editMode_ = editMode;

    if(editMode_)
    {
        // make lock unlocked

    }

    tabbedComponent->setEditMode(editMode_);

}

void MainComponent::toggleFullScreenMode()
{
    setFullScreenMode(getParentComponent() != Desktop::getInstance().getKioskModeComponent());

}

void MainComponent::setFullScreenMode(bool fullScreenMode)
{
    StageWindow* mainWindow = (StageWindow*)getParentComponent();

    if(fullScreenMode)
    {
        if(!nativeFullScreenSupport())
        {
            mainWindow->setUsingNativeTitleBar(false);
            mainWindow->setResizable(false, false);
        }
        if (Desktop::getInstance().getKioskModeComponent() == 0)
        {
            Desktop::getInstance().setKioskModeComponent(getParentComponent(), false);
        }

        if(fullScreenButton2)
        {
            fullScreenButton2->setImages (false, true, true,
                    ImageCache::getFromMemory (fullscreen_on_png, fullscreen_on_pngSize), 1.0000f, Colour (0x0),
                    Image(), 1.0000f, Colour (0x0),
                    Image(), 1.0000f, Colour (0x0));
        }
    }
    else
    {
        Desktop::getInstance().setKioskModeComponent (0);
        if(!nativeFullScreenSupport())
        {
            mainWindow->setUsingNativeTitleBar(true);
            mainWindow->setResizable(true, true);
        }

        if(fullScreenButton2)
        {
            fullScreenButton2->setImages (false, true, true,
                    ImageCache::getFromMemory (fullscreen_off_png, fullscreen_off_pngSize), 1.0000f, Colour (0x0),
                    Image(), 1.0000f, Colour (0x0),
                    Image(), 1.0000f, Colour (0x0));
        }
    }


    if(toolMode_==toolCreate)
        openAgentTree();

}


void MainComponent::setToolMode(CommandID toolMode)
{
    if(!lockMode_)
    {
        toolMode_ = toolMode;

        // if perform tool then exit edit mode
        setEditMode(!(toolMode_==toolPerform));

        toolManager_->setTool(toolMode_);

        tabbedComponent->setToolMode(toolMode_, toolManager_->getMouseCursor(toolMode_));

        if(toolMode==toolCreate || toolMode==toolMulti)
            openAgentTree();
        else
            closeAgentTree();

        if(toolMode==toolHelp)
            openHelp();
        else
            closeHelp();

        if(XMLRPCManager_)
        {
            if(toolMode==toolCreate || toolMode==toolHelp)
            {
                XMLRPCManager_->setAgentViewUpdate(true);
            }
            else
            {
                XMLRPCManager_->setAgentViewUpdate(false);
            }
        }
    }


}



bool MainComponent::keyPressed(const KeyPress& key)
{
    // TODO: what grabs keyboard focus in create mode?

    // handle esc out of full screen mode
    if(key.getKeyCode()==KeyPress::escapeKey)
    {
        setFullScreenMode(false);
            return true;
    }

    if(key.getKeyCode()=='e' && key.getModifiers().isAltDown() && key.getModifiers().isCtrlDown())
    {
        setToolMode(toolMulti);
        animator_->animateComponent((Component*)toolbar_, getToolBarBounds(true), 1.0f, 500, false, 0.0, 0.0);
    }


    // TODO: how the heck do you handle tab keys?
    if(key.getKeyCode()==KeyPress::tabKey) // && key.getModifiers().isCtrlDown())
    {
//        if(key.getModifiers().isCtrlDown())
//        {
            int numTabs = tabbedComponent->getNumTabs();
            int currTab = tabbedComponent->getCurrentTabIndex();
            if (currTab<(numTabs-1))
            {
                tabbedComponent->setCurrentTabIndex(0, true);
            }
            else
            {
                tabbedComponent->setCurrentTabIndex(currTab+1, true);
            }

//        }


    }

    if(key.getKeyCode()==KeyPress::spaceKey)
    {
        // TODO: open tool menu
    }

    return false;
}

void MainComponent::modifierKeysChanged(const ModifierKeys &modifiers)
{

}

//==============================================================================

StringArray MainComponent::getMenuBarNames()
{
    const wchar_t* const names[] = { L"File", L"Edit", L"Tool", L"View", L"Help", 0 };

    return StringArray ((const wchar_t**) names);
}

PopupMenu MainComponent::getMenuForIndex (int menuIndex, const String& menuName)
{
    ApplicationCommandManager* const commandManager = stageWindow_->commandManager;

    PopupMenu menu;

    if (menuIndex == 0)
    {
        menu.addCommandItem(commandManager, fileConnect);
        menu.addSeparator();
        menu.addCommandItem(commandManager, fileImportTabs);
        menu.addSeparator();
        menu.addCommandItem(commandManager, fileExportTab);
        menu.addCommandItem(commandManager, fileExportTabs);
        menu.addSeparator();
        menu.addCommandItem(commandManager, filePreferences);
#if WIN32
        menu.addSeparator();
        menu.addCommandItem(commandManager, fileQuit);
#endif // WIN32
    }
    else if (menuIndex == 1)
    {
        menu.addCommandItem(commandManager, editNewTab);
        menu.addCommandItem(commandManager, editTabProperties);
        menu.addSeparator();
        menu.addCommandItem(commandManager, editCloseTab);
        menu.addSeparator();
        menu.addCommandItem(commandManager, editToggleLock);

    }
    else if (menuIndex == 2)
    {
        menu.addCommandItem(commandManager, toolPerform);
        menu.addSeparator();
        menu.addCommandItem(commandManager, toolCreate);
        menu.addCommandItem(commandManager, toolEdit);
        menu.addCommandItem(commandManager, toolSize);
        menu.addSeparator();
        menu.addCommandItem(commandManager, toolDelete);
        menu.addSeparator();
        menu.addCommandItem(commandManager, toolHelp);

    }
    else if (menuIndex == 3)
    {
        menu.addCommandItem (commandManager, viewToggleFullScreen);
        menu.addSeparator();
        menu.addCommandItem (commandManager, viewGreyOutsideCanvas);

    }
    else if (menuIndex == 4)
    {
        menu.addCommandItem (commandManager, helpStageHelp);

    }

    return menu;
}

void MainComponent::menuItemSelected (int menuItemID,
                                      int topLevelMenuIndex)
{
    // most of our menu items are invoked automatically as commands, but we can handle the
    // other special cases here..
}





//==============================================================================

ApplicationCommandTarget* MainComponent::getNextCommandTarget()
{
    // this will return the next parent component that is an ApplicationCommandTarget (in this
    // case, there probably isn't one, but it's best to use this method in your own apps).
    return findFirstTargetParentComponent();
}

void MainComponent::getAllCommands (Array <CommandID>& commands)
{
    // this returns the set of all commands that this target can perform..
    const CommandID ids[] =
    {
        fileConnect,
        fileImportTabs,
        fileExportTab,
        fileExportTabs,
        filePreferences,
        fileQuit,

        editNewTab,
        editTabProperties,
        editCloseTab,
        editToggleLock,

        toolPerform,
        toolCreate,
        toolEdit,
        toolMove,
        toolSize,
        toolDelete,
        toolHelp,

        viewToggleFullScreen,
        viewGreyOutsideCanvas,

        helpStageHelp
    };

    commands.addArray (ids, numElementsInArray (ids));
}

void MainComponent::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    // This method is used when something needs to find out the details about one of the commands
    // that this object can perform..

    const String category1 ("File");
    const String category2 ("Edit");
    const String category3 ("Tool");
    const String category4 ("View");
    const String category5 ("Help");

    switch (commandID)
    {
        case fileConnect:
            result.setInfo ("Connect to eigenD...", "Connect to an eigenD server", category1, 0);
            break;

        case fileImportTabs:
            result.setInfo ("Import Tabs...", "Import tabs from another setup", category1, 0);
            break;

        case fileExportTab:
            result.setInfo ("Export Tab...", "Export the current tab from this setup", category1, 0);
            break;

        case fileExportTabs:
            result.setInfo ("Export All Tabs...", "Export tabs from this setup", category1, 0);
            break;

        case filePreferences:
            result.setInfo ("Preferences...", "Stage application preferences", category1, 0);
            break;

        case fileQuit:
            result.setInfo ("Quit", "Quit Stage", category1, 0);
            result.addDefaultKeypress ('q', ModifierKeys::commandModifier);
            break;


        case editNewTab:
            result.setInfo ("New Tab", "Create a New Tab", category2, 0);
            result.setActive (!getLockMode());
            result.addDefaultKeypress ('t', ModifierKeys::commandModifier);
            break;

        case editTabProperties:
            result.setInfo ("Tab Properties...", "Change the Current Tab Properties", category2, 0);
            result.setActive (!getLockMode());
            result.addDefaultKeypress ('n', ModifierKeys::commandModifier);
            break;

        case editCloseTab:
            result.setInfo ("Close Tab", "Close the Current Tab", category2, 0);
            result.setActive (!getLockMode());
            result.addDefaultKeypress ('w', ModifierKeys::commandModifier);
            break;

        case editToggleLock:
            // if locked
            if(lockMode_)
                result.setInfo ("Unlock Layout", "Unlock the Canvas to Edit the Widget Layout", category2, 0);
            else
                result.setInfo ("Lock Layout", "Lock the canvas to prevent changes to the widget layout", category2, 0);
            result.addDefaultKeypress ('l', ModifierKeys::commandModifier);
            break;


        case toolPerform:
            result.setInfo ("Perform", "Tool Mode for Using Widgets", category3, 0);
            result.setTicked (getToolMode()==toolPerform);
            result.setActive (!getLockMode());
            result.addDefaultKeypress ('p', ModifierKeys::commandModifier);
            break;

        case toolCreate:
            result.setInfo ("Create", "Tool Mode for Creating Widgets", category3, 0);
            result.setTicked (getToolMode()==toolCreate);
            result.setActive (!getLockMode());
            result.addDefaultKeypress ('c', ModifierKeys::commandModifier);
            break;

        case toolEdit:
            result.setInfo ("Edit", "Tool Mode for Editing Widget Properties", category3, 0);
            result.setTicked (getToolMode()==toolEdit);
            result.setActive (!getLockMode());
            result.addDefaultKeypress ('e', ModifierKeys::commandModifier);
            break;

        case toolMove:
            result.setInfo ("Move", "Tool Mode for Moving Widgets", category3, 0);
            result.setTicked (getToolMode()==toolMove);
            result.setActive (!getLockMode());
            result.addDefaultKeypress ('m', ModifierKeys::commandModifier);
            break;

        case toolSize:
            result.setInfo ("Size and Position", "Tool Mode for Moving and Sizing Widgets", category3, 0);
            result.setTicked (getToolMode()==toolSize);
            result.setActive (!getLockMode());
            result.addDefaultKeypress ('r', ModifierKeys::commandModifier);
            break;

        case toolDelete:
            result.setInfo ("Delete", "Tool Mode for Delete Widgets", category3, 0);
            result.setTicked (getToolMode()==toolDelete);
            result.setActive (!getLockMode());
            result.addDefaultKeypress ('d', ModifierKeys::commandModifier);
            break;

        case toolHelp:
            result.setInfo ("Help", "Tool Mode for Help", category3, 0);
            result.setTicked (getToolMode()==toolHelp);
            result.setActive (!getLockMode());
            result.addDefaultKeypress ('h', ModifierKeys::commandModifier);
            break;



        case viewToggleFullScreen:
            result.setInfo ("Toggle Full Screen Mode", "Enter or Leave Full Screen Mode", category4, 0);
            result.addDefaultKeypress ('f', ModifierKeys::commandModifier);
            break;

        case viewGreyOutsideCanvas:
            result.setInfo ("Grey Outside Canvas", "Grey the area outside the canvas", category4, 0);
            result.setTicked (tabbedComponent->getGreyOutsideCanvas());
            result.addDefaultKeypress ('g', ModifierKeys::commandModifier);
            break;



        // TODO: fix default keys
        case helpStageHelp:
            result.setInfo ("Stage Help", "Show Stage Documentation", category5, 0);
            //result.addDefaultKeypress (T('h'), ModifierKeys::commandModifier);
            break;

        default:
            break;
    };
}

bool MainComponent::perform (const InvocationInfo& info)
{
    bool newLockMode = false;

    // this is the ApplicationCommandTarget method that is used to actually perform one of our commands..
    switch (info.commandID)
    {
        case fileConnect:
            if(XMLRPCManager_)
                XMLRPCManager_->browseForEigenD();
            break;

        case fileImportTabs:
            tabbedComponent->importTabs();
            break;

        case fileExportTab:
            tabbedComponent->exportTab();
            break;

        case fileExportTabs:
            tabbedComponent->exportTabs();
            break;

        case filePreferences:
            userEditPreferences();
            break;

        case fileQuit:
            JUCEApplication::quit();
            break;


        case editNewTab:
            tabbedComponent->userAddTab("untitled");
            break;
        case editTabProperties:
            tabbedComponent->userSetTabProperties(tabbedComponent->getCurrentTabIndex());
            break;
        case editCloseTab:
            if(tabbedComponent->getNumTabs()>1)
                tabbedComponent->userRemoveTab(tabbedComponent->getCurrentTabIndex(), this);
            break;

        case editToggleLock:
            // toggle lock mode
            newLockMode = !lockMode_;
            // set button image to show lock state
            lockButton_->setToggleState(newLockMode, false);

            if(newLockMode)
            {
                animator_->animateComponent((Component*)toolbar_, getToolBarBounds(true), 1.0f, 500, false, 0.0, 0.0);
            }
            else
            {
                if(prevToolMode_!=toolMulti)
                    animator_->animateComponent((Component*)toolbar_, getToolBarBounds(false), 1.0f, 500, false, 0.0, 0.0);
            }


            if(newLockMode)
            {
                prevToolMode_ = toolMode_;
                setToolMode(toolPerform);
                lockMode_ = newLockMode;
            }
            else
            {
                lockMode_ = newLockMode;
                setToolMode(prevToolMode_);
            }

            tabbedComponent->getTabbedButtonBar().hideAddCloseButtons(lockMode_);
            tabbedComponent->updateShowAllTabs();
            tabbedComponent->ensureCurrentTabVisible();
            break;


        case toolPerform:
        case toolCreate:
        case toolMove:
        case toolSize:
        case toolEdit:
        case toolDelete:
        case toolHelp:
            setToolMode(info.commandID);
            animator_->animateComponent((Component*)toolbar_, getToolBarBounds(false), 1.0f, 500, false, 0.0, 0.0);
            break;


        case viewToggleFullScreen:
            toggleFullScreenMode();
            break;

        case viewGreyOutsideCanvas:
            tabbedComponent->toggleGreyOutsideCanvas();
            break;


        case helpStageHelp:
            juce::URL("http://www.eigenlabs.com/wiki/Stage/").launchInDefaultBrowser();
            break;

        default:
            return false;
    };

    return true;
}


//==============================================================================
/*
void MainComponent::mouseDown (const MouseEvent& e)
{
}


void MainComponent::mouseDrag (const MouseEvent& e)
{
}

void MainComponent::mouseDoubleClick (const MouseEvent& e)
{
}
*/

void MainComponent::bringAgentViewToFront()
{
    if(agentView_)
        agentView_->toFront(false);

    if(helpView_)
        helpView_->toFront(false);

    toBack();
}

void MainComponent::handleMessage(const Message& message)
{
    if(agentView_)
        agentView_->toFront(false);

    if(helpView_)
        helpView_->toFront(false);

    toBack();
}


//==============================================================================


void MainComponent::userEditPreferences()
{
    // open preferences dialog

    EigenDialogWindow window("Preferences Dialog Window");
    DialogFrameworkComponent framework(&window);
    PreferencesDialogComponent content(preferences_);

    content.initialize(&framework);
    window.setContentComponent(&framework, false, true);
    window.setSize(440, 640);
    window.centreAroundComponent(this, framework.getWidth(), framework.getHeight());
    window.setVisible(true);
    framework.setTitleText("Preferences");
    framework.setContentComponent(&content);
    framework.showResizer();
    framework.setMinimumSize(440, 640);
    content.setFocusOrder(&framework);
    framework.grabKeyboardFocus();

    // establish preference panel setting sources
    content.getTabViewPreferences()->setTabbedComponent(tabbedComponent);
    content.getViewPreferences()->setTabbedComponent(tabbedComponent);

    int result = framework.runModalLoop();

    // apply changes if ok pressed
    if(result)
    {
        content.getTabViewPreferences()->applyChanges();
        content.getNetworkPreferences()->applyChanges();
        content.getViewPreferences()->applyChanges();
    }

}





//==============================================================================

void MainComponent::initializeModel()
{
    // TODO: need a way to have the version in one source (e.g. plist file)
    std::cout << "Eigenlabs Stage version " << JUCEApplication::getInstance()->getApplicationVersion() << "\n";

    String defaultSans = Font::getDefaultSansSerifFontName();
    String defaultSerif = Font::getDefaultSerifFontName();
    String defaultFixed = Font::getDefaultMonospacedFontName();

    //std::cout << "Default Font: " << defaultSans << "\n";


    // preferences management

    preferences_ = new StagePreferences();

    // create default value if properties do not exist
    // tab view
    preferences_->setValue("monitor", preferences_->getBoolValue("monitor",true));
    preferences_->setValue("halfMonitorHorizontal", preferences_->getBoolValue("halfMonitorHorizontal",true));
    preferences_->setValue("halfMonitorVertical", preferences_->getBoolValue("halfMonitorVertical",true));
    preferences_->setValue("widescreenMonitor", preferences_->getBoolValue("widescreenMonitor",true));
    preferences_->setValue("halfWidescreenMonitorHorizontal", preferences_->getBoolValue("halfWidescreenMonitorHorizontal",true));
    preferences_->setValue("halfWidescreenMonitorVertical", preferences_->getBoolValue("halfWidescreenMonitorVertical",true));
    preferences_->setValue("iPhoneiPodTouchHorizontal", preferences_->getBoolValue("iPhoneiPodTouchHorizontal",false));
    preferences_->setValue("iPhoneiPodTouchVertical", preferences_->getBoolValue("iPhoneiPodTouchVertical",false));
    preferences_->setValue("iPhoneiPod4InchHorizontal", preferences_->getBoolValue("iPhoneiPod4InchHorizontal",false));
    preferences_->setValue("iPhoneiPod4InchVertical", preferences_->getBoolValue("iPhoneiPod4InchVertical",false));
    preferences_->setValue("iPadHorizontal", preferences_->getBoolValue("iPadHorizontal",false));
    preferences_->setValue("iPadVertical", preferences_->getBoolValue("iPadVertical",false));
    preferences_->setValue("custom", preferences_->getBoolValue("custom",true));
    preferences_->setValue("customShowTabs", preferences_->getBoolValue("customShowTabs", false));
    preferences_->setValue("allTabsEditMode", preferences_->getBoolValue("allTabsEditMode", true));
    // network
    preferences_->setValue("connectSingleHost", preferences_->getBoolValue("connectSingleHost", false));
    // view
    preferences_->setValue("greyOutsideCanvas", preferences_->getBoolValue("greyOutsideCanvas", true));
    preferences_->setValue("showCloseButtons", preferences_->getBoolValue("showCloseButtons", false));



    // save if a new set of preferences was created
    preferences_->saveIfNeeded();



    // set up server connections and get data from EigenD

    // XMLRPC client manager

    XMLRPCManager_ = new XMLRPCManager(this);

    // OSC manager

    OSCManager_ = new OSCManager(XMLRPCManager_);

    // create empty tab
    tabbedComponent->initTab("untitled");
    // set view preferences
    tabbedComponent->setGreyOutsideCanvas(preferences_->getBoolValue("greyOutsideCanvas", true));
    tabbedComponent->setShowCloseButtons(preferences_->getBoolValue("showCloseButtons", false));
    repaint();

    XMLRPCManager_->initialize();
    OSCManager_->initialize();

    // request the current tabs from EigenD immediately
    // this will complete when the EigenD connection is ready
    // TODO: should have receive/request function
    tabbedComponent->setXMLRPCManager(XMLRPCManager_);

    agentView_->setXMLRPCManager(XMLRPCManager_);
    agentView_->setTabbedComponent(tabbedComponent);

}

void MainComponent::shutdown()
{

    if(OSCManager_)
        OSCManager_->shutdown();
    // tell the XMLRPC manager that we're about to shutdown so it needs to abort
    // any blocked XMLRPC calls currently executing
    if(XMLRPCManager_)
        XMLRPCManager_->shutdown();


}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainComponent" componentName="Main"
                 parentClasses="public Component, public DragAndDropContainer, public MenuBarModel, public ApplicationCommandTarget, public MessageListener"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330000013" fixedSize="1" initialWidth="951"
                 initialHeight="758">
  <BACKGROUND backgroundColour="ff000000">
     <IMAGE pos="31Rr 4 133 68" resource="logoText_png" opacity="1" mode="1"/>
  </BACKGROUND>
  <TABBEDCOMPONENT name="widgetTabs" id="2916c2f0c924c39c" memberName="tabbedComponent"
                   virtualName="WidgetTabbedComponent" explicitFocusOrder="0" pos="16 60 32M 76M"
                   orientation="top" tabBarDepth="24" initialTab="-1"/>
  <JUCERCOMP name="" id="dc4f9b1f909b112d" memberName="tabsFrameBox" virtualName=""
             explicitFocusOrder="0" pos="16 84 32M 100M" sourceFile="TabsFrameBox.cpp"
             constructorParams=""/>
  <JUCERCOMP name="toolbar hider" id="26778dd8671bc74e" memberName="toolbarHider"
             virtualName="" explicitFocusOrder="0" pos="0 -6 96 64" sourceFile="BackgroundComponent.cpp"
             constructorParams=""/>
  <IMAGEBUTTON name="new button" id="8aa5fe4f3b2d51f0" memberName="imageButton2"
               virtualName="" explicitFocusOrder="0" pos="48 24 24 24" buttonText="new button"
               connectedEdges="0" needsCallback="1" radioGroupId="0" keepProportions="1"
               resourceNormal="" opacityNormal="1" colourNormal="0" resourceOver=""
               opacityOver="1" colourOver="0" resourceDown="" opacityDown="1"
               colourDown="0"/>
  <IMAGEBUTTON name="full screen button" id="c015d5f1ea242cea" memberName="fullScreenButton2"
               virtualName="" explicitFocusOrder="0" pos="50 16 31 31" buttonText="new button"
               connectedEdges="0" needsCallback="1" radioGroupId="0" keepProportions="1"
               resourceNormal="fullscreen_off_png" opacityNormal="1" colourNormal="0"
               resourceOver="" opacityOver="1" colourOver="0" resourceDown=""
               opacityDown="1" colourDown="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: lock_closed_png, 1489, "graphics/lock_closed.png"
static const unsigned char resource_MainComponent_lock_closed_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,36,0,0,0,34,8,6,0,0,0,55,89,123,133,0,0,0,25,116,69,88,116,83,111,102,116,119,
97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,177,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,99,107,
101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,116,97,
32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,54,48,
32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,34,104,
116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,112,116,
105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,82,105,103,104,116,115,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,
112,47,49,46,48,47,114,105,103,104,116,115,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,
109,47,34,32,120,109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,
99,101,82,101,102,35,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,32,120,109,112,82,105,103,104,116,
115,58,77,97,114,107,101,100,61,34,70,97,108,115,101,34,32,120,109,112,77,77,58,79,114,105,103,105,110,97,108,68,111,99,117,109,101,110,116,73,68,61,34,117,117,105,100,58,70,52,51,70,50,53,57,65,48,57,
55,65,68,70,49,49,66,70,51,50,57,56,65,65,56,68,51,50,48,69,56,57,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,54,51,68,49,52,70,53,50,70,65,54,54,
49,49,69,49,66,54,69,68,56,55,49,51,51,55,54,65,52,54,49,68,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,54,51,68,49,52,70,53,49,70,65,54,54,49,49,
69,49,66,54,69,68,56,55,49,51,51,55,54,65,52,54,49,68,34,32,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,46,49,32,77,
97,99,105,110,116,111,115,104,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,
68,50,68,51,67,70,50,57,48,69,50,51,54,56,49,49,57,49,48,57,69,53,51,67,55,65,52,66,56,68,51,49,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,117,117,105,100,58,70,52,51,70,50,
53,57,65,48,57,55,65,68,70,49,49,66,70,51,50,57,56,65,65,56,68,51,50,48,69,56,57,34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,
47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,110,100,61,34,114,34,63,62,56,228,148,13,0,0,1,182,73,68,65,84,120,218,98,250,255,255,63,3,22,44,13,196,43,128,248,201,
127,226,193,19,168,30,105,28,102,18,133,25,255,254,253,203,128,6,212,129,248,24,16,11,49,144,7,222,1,177,45,19,19,211,53,114,52,51,254,254,253,27,93,108,53,16,135,48,80,6,214,179,176,176,4,145,229,160,
159,63,127,162,139,125,0,98,126,10,29,244,133,141,141,141,151,28,141,76,160,40,67,195,252,104,252,131,64,108,14,196,140,56,176,57,84,13,178,30,30,114,125,194,248,249,243,103,116,177,255,104,124,73,32,
126,129,203,0,30,30,30,134,47,95,190,72,0,153,207,209,196,25,201,113,16,11,150,68,141,14,94,16,82,0,180,252,197,199,143,31,25,168,1,136,113,16,81,128,90,230,176,252,249,243,135,42,6,81,203,156,209,16,
26,117,208,176,115,16,227,165,75,151,64,180,52,16,247,2,177,13,148,77,13,240,20,136,143,0,113,177,174,174,238,83,162,29,116,238,220,57,74,107,119,162,106,127,67,67,195,107,196,70,89,11,13,29,195,0,53,
27,100,71,16,177,14,114,101,160,61,32,218,14,22,96,123,136,159,14,14,226,161,123,46,163,22,192,230,32,103,32,222,71,172,1,110,110,110,24,98,187,118,237,114,2,82,123,169,229,32,54,42,148,73,108,212,12,
161,237,164,24,176,105,211,38,154,71,217,208,78,67,97,97,97,24,98,171,86,173,26,70,105,8,75,195,138,164,52,180,120,241,226,97,158,134,176,132,16,73,105,40,37,37,5,67,108,206,156,57,228,167,33,44,14,162,
56,13,1,205,164,40,13,125,68,235,58,147,148,134,166,77,155,70,84,215,154,104,7,253,251,247,15,20,61,129,52,78,26,187,73,9,161,26,32,109,79,227,6,90,13,41,33,4,106,201,233,1,241,4,104,19,86,130,74,14,121,
1,109,194,22,20,22,22,18,221,132,5,8,48,0,79,90,119,50,49,242,45,2,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* MainComponent::lock_closed_png = (const char*) resource_MainComponent_lock_closed_png;
const int MainComponent::lock_closed_pngSize = 1489;

// JUCER_RESOURCE: lock_open_png, 1407, "graphics/lock_open.png"
static const unsigned char resource_MainComponent_lock_open_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,36,0,0,0,34,8,6,0,0,0,55,89,123,133,0,0,0,25,116,69,88,116,83,111,102,116,119,
97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,177,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,99,107,
101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,116,97,
32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,54,48,
32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,34,104,
116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,112,116,
105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,82,105,103,104,116,115,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,
112,47,49,46,48,47,114,105,103,104,116,115,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,
109,47,34,32,120,109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,
99,101,82,101,102,35,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,32,120,109,112,82,105,103,104,116,
115,58,77,97,114,107,101,100,61,34,70,97,108,115,101,34,32,120,109,112,77,77,58,79,114,105,103,105,110,97,108,68,111,99,117,109,101,110,116,73,68,61,34,117,117,105,100,58,70,52,51,70,50,53,57,65,48,57,
55,65,68,70,49,49,66,70,51,50,57,56,65,65,56,68,51,50,48,69,56,57,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,68,51,70,70,65,67,68,49,70,65,54,53,
49,49,69,49,66,54,69,68,56,55,49,51,51,55,54,65,52,54,49,68,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,68,51,70,70,65,67,68,48,70,65,54,53,49,49,
69,49,66,54,69,68,56,55,49,51,51,55,54,65,52,54,49,68,34,32,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,46,49,32,77,
97,99,105,110,116,111,115,104,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,
68,50,68,51,67,70,50,57,48,69,50,51,54,56,49,49,57,49,48,57,69,53,51,67,55,65,52,66,56,68,51,49,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,117,117,105,100,58,70,52,51,70,50,
53,57,65,48,57,55,65,68,70,49,49,66,70,51,50,57,56,65,65,56,68,51,50,48,69,56,57,34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,
47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,110,100,61,34,114,34,63,62,93,210,183,247,0,0,1,100,73,68,65,84,120,218,98,248,255,255,63,3,21,177,52,16,175,0,226,39,
255,137,7,79,160,122,64,122,25,24,255,254,253,203,64,37,160,14,196,199,128,88,136,76,253,239,128,216,150,241,247,239,223,212,114,208,106,32,14,161,208,140,245,140,63,127,254,164,150,131,62,0,49,63,133,
102,124,97,252,246,237,27,181,28,244,31,141,127,16,136,203,128,248,20,14,245,102,64,220,5,196,246,200,130,140,159,63,127,166,149,131,36,129,248,5,1,61,18,64,252,28,197,65,31,62,124,160,149,131,24,201,
209,199,248,246,237,219,193,229,160,151,47,95,14,46,7,61,123,246,108,112,57,232,209,163,71,3,234,32,89,89,89,20,62,203,159,63,127,24,6,19,24,117,16,177,14,146,6,226,94,32,182,129,178,169,1,158,0,241,17,
32,46,6,226,167,164,56,136,210,90,26,23,0,121,44,28,136,93,65,181,56,16,95,35,214,65,45,52,112,12,50,0,153,13,178,35,136,88,7,185,210,33,105,16,109,7,11,176,61,196,79,7,7,241,12,171,108,239,12,196,251,
40,52,215,9,136,247,82,203,65,108,84,240,40,27,53,67,104,251,104,73,61,154,134,72,113,16,150,142,226,104,26,34,20,66,3,155,134,176,56,104,192,211,208,71,180,46,48,45,210,208,23,162,29,244,239,223,63,80,
244,4,210,56,105,236,38,37,132,106,160,253,107,90,181,137,64,195,44,53,164,132,16,168,37,167,7,196,19,160,77,88,9,42,57,228,5,180,9,91,64,74,19,22,32,192,0,19,46,11,184,178,137,238,185,0,0,0,0,73,69,78,
68,174,66,96,130,0,0};

const char* MainComponent::lock_open_png = (const char*) resource_MainComponent_lock_open_png;
const int MainComponent::lock_open_pngSize = 1407;

// JUCER_RESOURCE: dots_png, 1137, "graphics/dots.png"
static const unsigned char resource_MainComponent_dots_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,20,0,0,0,60,8,2,0,0,0,233,20,13,1,0,0,0,25,116,69,88,116,83,111,102,116,119,97,
114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,177,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,99,107,101,
116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,116,97,32,120,
109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,54,48,32,54,49,
46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,34,104,116,116,
112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,112,116,105,111,
110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,82,105,103,104,116,115,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,
49,46,48,47,114,105,103,104,116,115,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,
34,32,120,109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,
82,101,102,35,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,32,120,109,112,82,105,103,104,116,115,58,
77,97,114,107,101,100,61,34,70,97,108,115,101,34,32,120,109,112,77,77,58,79,114,105,103,105,110,97,108,68,111,99,117,109,101,110,116,73,68,61,34,117,117,105,100,58,70,52,51,70,50,53,57,65,48,57,55,65,
68,70,49,49,66,70,51,50,57,56,65,65,56,68,51,50,48,69,56,57,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,54,51,68,49,52,70,53,54,70,65,54,54,49,49,
69,49,66,54,69,68,56,55,49,51,51,55,54,65,52,54,49,68,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,54,51,68,49,52,70,53,53,70,65,54,54,49,49,69,49,
66,54,69,68,56,55,49,51,51,55,54,65,52,54,49,68,34,32,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,46,49,32,77,97,99,
105,110,116,111,115,104,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,68,50,
68,51,67,70,50,57,48,69,50,51,54,56,49,49,57,49,48,57,69,53,51,67,55,65,52,66,56,68,51,49,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,117,117,105,100,58,70,52,51,70,50,53,57,
65,48,57,55,65,68,70,49,49,66,70,51,50,57,56,65,65,56,68,51,50,48,69,56,57,34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,
58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,110,100,61,34,114,34,63,62,193,246,220,58,0,0,0,86,73,68,65,84,120,218,236,212,161,14,128,48,12,69,209,137,125,243,28,22,141,
158,71,162,248,191,150,228,121,150,140,37,207,112,143,109,213,77,211,82,240,107,183,140,119,234,219,32,51,41,56,227,18,106,123,156,242,177,118,68,80,112,70,23,110,219,227,16,110,219,99,23,106,123,108,
194,39,241,104,66,109,44,122,4,24,0,135,80,51,240,209,7,102,33,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* MainComponent::dots_png = (const char*) resource_MainComponent_dots_png;
const int MainComponent::dots_pngSize = 1137;

// JUCER_RESOURCE: fullscreen_off_png, 2468, "graphics/toolbar_buttons/fullscreen_off.png"
static const unsigned char resource_MainComponent_fullscreen_off_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,62,0,0,0,62,8,6,0,0,0,115,193,168,68,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,177,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,82,105,103,104,116,115,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,
47,120,97,112,47,49,46,48,47,114,105,103,104,116,115,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,109,109,47,34,32,120,109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,
111,117,114,99,101,82,101,102,35,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,32,120,109,112,82,105,
103,104,116,115,58,77,97,114,107,101,100,61,34,70,97,108,115,101,34,32,120,109,112,77,77,58,79,114,105,103,105,110,97,108,68,111,99,117,109,101,110,116,73,68,61,34,117,117,105,100,58,70,52,51,70,50,53,
57,65,48,57,55,65,68,70,49,49,66,70,51,50,57,56,65,65,56,68,51,50,48,69,56,57,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,65,54,70,57,65,51,51,54,
70,65,54,53,49,49,69,49,66,54,69,68,56,55,49,51,51,55,54,65,52,54,49,68,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,65,54,70,57,65,51,51,53,70,65,
54,53,49,49,69,49,66,54,69,68,56,55,49,51,51,55,54,65,52,54,49,68,34,32,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,
46,49,32,77,97,99,105,110,116,111,115,104,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,
105,100,58,68,50,68,51,67,70,50,57,48,69,50,51,54,56,49,49,57,49,48,57,69,53,51,67,55,65,52,66,56,68,51,49,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,117,117,105,100,58,70,
52,51,70,50,53,57,65,48,57,55,65,68,70,49,49,66,70,51,50,57,56,65,65,56,68,51,50,48,69,56,57,34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,
70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,110,100,61,34,114,34,63,62,245,182,231,65,0,0,5,137,73,68,65,84,120,218,236,91,73,75,44,73,16,14,219,93,15,
238,226,14,34,30,251,246,24,120,238,11,42,45,130,11,131,131,119,153,243,252,3,245,39,12,14,60,152,155,7,193,101,112,151,193,131,184,194,240,96,230,244,78,30,220,119,197,125,111,183,169,47,233,108,210,
178,150,238,182,170,122,177,3,162,181,178,170,43,227,203,136,140,136,140,204,142,120,125,125,165,207,72,81,61,61,61,65,13,160,171,171,203,55,224,10,109,49,18,255,42,241,207,18,127,117,93,7,44,105,40,206,
41,241,63,18,255,37,241,159,210,0,57,197,155,54,217,195,185,18,127,151,184,87,226,202,64,7,173,67,49,46,12,192,242,93,26,160,92,241,102,68,119,119,183,248,224,191,18,219,31,30,30,232,226,226,130,110,111,
111,41,88,125,64,68,68,4,37,36,36,80,82,82,18,197,198,198,162,233,135,196,95,184,230,69,141,195,188,237,247,247,247,116,112,112,64,55,55,55,20,204,142,15,178,3,3,176,0,19,176,185,48,190,155,227,191,224,
227,242,242,146,158,159,159,67,198,123,3,11,48,197,197,197,113,140,127,200,129,255,132,143,235,235,107,10,181,16,7,76,153,153,153,110,140,114,224,204,145,189,188,188,132,92,204,22,20,25,163,26,206,62,
75,66,19,6,238,45,112,132,11,204,155,167,167,39,58,57,57,177,84,232,228,228,100,22,162,30,31,31,233,244,244,212,24,224,242,57,222,217,217,73,41,41,41,138,95,6,232,177,177,49,58,62,62,182,86,91,81,81,212,
214,214,198,226,180,18,33,124,245,246,246,106,190,195,166,164,113,145,161,89,121,27,216,233,116,210,232,232,40,173,173,173,41,222,55,147,15,15,15,105,96,96,192,157,107,200,89,9,135,215,166,30,29,29,173,
248,69,152,216,206,206,142,223,124,194,217,217,25,155,98,241,241,241,239,115,213,152,24,93,185,108,74,166,46,178,82,27,56,61,61,157,90,91,91,153,217,41,221,55,147,109,54,27,181,180,180,80,78,78,142,234,
51,176,84,57,14,175,76,29,26,87,123,57,58,6,120,110,21,86,112,100,100,36,235,51,63,63,95,115,112,240,156,150,169,107,2,87,211,182,28,60,28,141,21,224,1,166,185,185,153,242,242,242,116,229,130,37,122,53,
199,69,179,64,200,192,53,188,247,254,254,62,27,101,78,219,219,219,180,180,180,196,218,173,34,56,212,161,161,33,246,127,118,118,54,149,151,151,191,145,105,115,115,147,50,50,50,152,183,231,166,238,147,87,
23,67,22,58,92,89,89,113,143,232,212,212,20,237,237,237,89,238,209,57,163,111,200,192,229,89,93,93,165,145,145,17,38,39,188,253,135,52,142,81,195,203,54,54,54,216,245,196,196,4,53,53,53,81,113,113,49,
93,93,93,177,54,152,93,69,69,5,101,101,101,89,162,117,44,51,23,23,23,89,68,129,12,144,119,125,125,157,201,134,149,216,209,209,17,13,14,14,50,133,249,172,113,172,106,240,82,126,141,23,77,78,78,50,205,243,
54,135,195,193,50,56,171,60,58,250,66,159,188,127,104,122,124,124,156,1,229,109,72,168,16,238,124,214,184,26,1,60,127,25,226,168,213,43,58,177,79,128,246,165,126,224,83,174,46,62,227,175,101,172,232,131,
76,201,213,245,72,254,60,230,222,242,242,50,155,139,70,16,124,71,89,89,25,243,37,31,145,211,112,224,114,51,155,158,158,102,94,213,40,130,247,198,59,177,88,50,21,184,183,185,183,92,0,56,68,51,74,71,242,
126,62,186,70,48,92,227,102,205,121,163,251,49,124,142,155,5,220,232,126,62,108,234,114,77,152,181,76,53,186,159,176,198,63,253,28,255,168,87,55,203,212,195,94,61,108,234,97,83,15,12,175,142,149,147,145,
41,43,40,49,49,49,240,189,122,105,105,41,43,20,224,80,129,17,132,50,18,222,25,240,166,158,155,155,75,29,29,29,161,105,234,168,107,139,53,119,127,16,239,23,213,93,236,161,121,75,186,27,10,74,27,12,181,
181,181,238,107,126,130,194,74,70,159,188,255,250,250,122,86,118,246,68,110,143,107,110,56,62,1,211,21,247,209,0,26,181,116,222,134,18,51,156,153,85,53,55,244,133,62,121,255,216,209,105,104,104,120,83,
215,71,201,25,178,251,92,115,195,232,86,87,87,211,236,236,44,43,8,212,212,212,184,183,109,184,247,222,218,218,162,254,254,126,191,152,59,247,246,105,105,105,84,87,87,71,51,51,51,172,98,83,85,85,69,195,
195,195,154,211,80,211,185,1,56,230,51,192,163,106,137,98,61,191,95,82,82,194,70,254,238,238,206,47,160,49,240,144,129,203,3,205,163,250,138,227,93,176,76,108,62,104,57,64,77,141,227,203,0,143,23,165,
166,166,190,9,41,216,201,104,111,111,119,215,217,230,230,230,124,46,252,121,74,144,163,178,178,146,10,11,11,21,195,41,64,243,54,181,185,237,177,198,245,188,246,238,238,174,37,160,185,108,243,243,243,76,
38,17,188,90,126,225,149,198,229,15,3,16,188,166,191,65,139,242,45,44,44,176,191,90,224,245,66,156,110,28,7,40,152,152,156,176,85,3,167,103,37,104,145,160,121,120,114,56,91,79,18,30,195,52,142,115,49,
216,206,129,214,253,65,168,179,243,173,43,111,205,220,163,4,230,252,252,92,113,183,18,222,30,97,3,78,206,234,19,17,200,45,224,228,212,206,231,224,152,138,94,2,163,171,113,20,243,213,8,90,87,75,16,204,
36,228,14,125,125,125,254,91,164,248,122,198,44,16,40,124,178,209,236,210,81,160,145,232,220,216,201,125,181,152,29,204,132,176,231,162,107,37,224,255,225,163,160,160,192,111,231,90,204,98,97,139,249,
135,146,169,15,74,252,213,110,183,179,19,77,248,93,74,40,16,78,110,1,147,128,241,157,198,191,97,68,16,151,27,27,27,169,168,168,72,49,99,11,38,2,6,96,1,38,151,182,191,41,105,28,115,220,33,241,223,210,131,
118,215,195,161,66,0,237,16,127,123,38,207,220,144,127,126,145,248,55,137,23,66,0,240,178,11,11,126,118,181,171,25,206,92,154,255,221,197,1,79,190,254,196,210,70,159,148,254,23,96,0,129,248,176,53,3,197,
204,183,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* MainComponent::fullscreen_off_png = (const char*) resource_MainComponent_fullscreen_off_png;
const int MainComponent::fullscreen_off_pngSize = 2468;

// JUCER_RESOURCE: fullscreen_on_png, 3178, "graphics/toolbar_buttons/fullscreen_on.png"
static const unsigned char resource_MainComponent_fullscreen_on_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,62,0,0,0,62,8,6,0,0,0,115,193,168,68,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,177,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,82,105,103,104,116,115,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,
47,120,97,112,47,49,46,48,47,114,105,103,104,116,115,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,109,109,47,34,32,120,109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,
111,117,114,99,101,82,101,102,35,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,32,120,109,112,82,105,
103,104,116,115,58,77,97,114,107,101,100,61,34,70,97,108,115,101,34,32,120,109,112,77,77,58,79,114,105,103,105,110,97,108,68,111,99,117,109,101,110,116,73,68,61,34,117,117,105,100,58,70,52,51,70,50,53,
57,65,48,57,55,65,68,70,49,49,66,70,51,50,57,56,65,65,56,68,51,50,48,69,56,57,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,65,54,70,57,65,51,51,65,
70,65,54,53,49,49,69,49,66,54,69,68,56,55,49,51,51,55,54,65,52,54,49,68,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,65,54,70,57,65,51,51,57,70,65,
54,53,49,49,69,49,66,54,69,68,56,55,49,51,51,55,54,65,52,54,49,68,34,32,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,
46,49,32,77,97,99,105,110,116,111,115,104,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,
105,100,58,68,50,68,51,67,70,50,57,48,69,50,51,54,56,49,49,57,49,48,57,69,53,51,67,55,65,52,66,56,68,51,49,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,117,117,105,100,58,70,
52,51,70,50,53,57,65,48,57,55,65,68,70,49,49,66,70,51,50,57,56,65,65,56,68,51,50,48,69,56,57,34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,
70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,110,100,61,34,114,34,63,62,68,124,22,85,0,0,8,79,73,68,65,84,120,218,236,91,123,108,83,215,25,255,217,190,
190,215,175,56,78,226,4,18,8,121,192,70,88,40,227,161,192,32,44,213,160,82,91,117,155,6,109,215,72,155,214,85,45,173,52,237,33,117,93,197,166,106,170,186,23,180,42,162,27,130,177,72,101,172,237,186,81,
245,143,77,72,163,26,208,135,90,90,165,107,66,25,17,13,132,4,104,48,212,33,118,72,226,196,207,120,223,119,125,237,56,177,227,216,181,115,131,173,125,210,79,246,61,190,231,158,243,59,191,239,124,231,225,
123,52,225,112,24,100,90,130,72,48,17,108,132,50,66,9,161,136,96,32,232,8,26,228,135,49,161,16,193,75,24,33,184,9,131,132,33,194,24,193,79,152,16,20,210,6,133,112,21,161,158,176,148,176,132,80,78,176,
16,244,10,113,77,30,144,102,4,8,163,132,1,194,21,194,69,66,47,193,161,52,128,87,80,148,102,210,117,132,53,228,1,205,30,143,103,171,207,231,43,15,4,2,80,60,34,239,76,163,209,64,175,215,67,146,164,1,179,
217,124,130,174,223,163,228,78,66,31,193,37,40,238,205,74,175,9,6,131,219,134,134,134,182,48,225,124,55,22,204,239,247,51,202,189,94,111,171,205,102,171,16,4,166,11,95,84,113,86,187,158,149,118,187,221,
91,136,60,62,234,238,69,219,209,19,56,214,126,26,254,64,48,47,137,139,122,1,119,173,95,141,29,95,223,138,117,203,235,193,220,236,118,187,147,148,119,178,226,26,34,220,68,95,238,32,247,126,124,120,120,
216,222,126,174,7,15,238,218,143,155,163,99,40,4,43,182,152,112,120,231,15,176,126,197,50,88,173,214,27,228,246,123,40,249,184,86,137,222,75,200,29,236,124,227,139,199,222,198,77,207,56,119,146,130,0,
115,97,78,108,10,71,14,218,37,130,50,100,149,71,251,245,177,246,51,74,160,47,28,139,112,162,80,31,225,200,35,85,145,160,12,101,150,104,244,246,7,67,145,214,42,32,147,57,41,1,79,25,158,13,130,50,57,209,
199,141,3,40,112,99,174,58,33,97,98,162,209,22,58,113,153,175,16,119,145,145,226,58,173,22,141,181,139,224,245,7,112,190,255,186,170,53,175,93,104,135,213,100,132,199,235,195,69,135,243,243,16,135,144,
152,62,85,241,83,123,127,33,23,148,204,152,244,195,207,31,34,226,78,85,137,155,13,70,188,178,243,49,148,89,45,73,127,31,162,161,248,75,143,60,149,242,25,137,126,205,174,30,7,189,32,204,76,122,207,97,188,
121,230,124,66,158,185,70,215,229,107,248,246,175,15,98,112,120,52,57,41,109,146,124,211,76,72,50,201,157,114,105,49,74,242,188,119,186,245,94,191,129,246,243,151,230,45,24,246,57,7,229,58,216,139,139,
18,39,45,102,227,172,245,154,85,113,185,245,146,88,99,77,21,254,186,115,7,204,70,131,234,138,27,36,9,47,61,249,48,214,47,175,155,57,116,211,2,37,149,226,73,82,180,83,192,173,199,138,39,195,134,134,186,
8,121,147,33,33,223,92,193,96,144,240,50,145,254,234,202,47,204,88,47,134,145,238,155,146,55,19,197,117,58,97,86,151,219,176,188,22,127,87,73,121,86,250,240,79,31,196,230,198,165,105,4,64,67,166,125,124,
242,38,38,195,173,199,129,172,227,98,63,54,173,152,116,173,83,231,250,240,204,223,254,133,78,74,87,107,14,224,13,132,240,192,238,63,203,223,215,44,93,140,95,182,222,61,165,78,111,159,237,193,202,37,149,
20,237,205,147,174,158,118,31,143,115,15,157,160,67,32,52,129,135,126,255,42,238,219,125,8,71,63,236,138,185,210,99,251,143,160,179,207,161,154,139,79,7,151,205,117,136,214,231,248,199,231,241,221,61,
47,225,222,221,47,194,69,195,89,66,247,203,68,113,189,160,199,247,246,190,130,147,255,237,145,175,31,221,255,26,254,68,133,124,163,169,17,206,97,143,124,239,166,134,90,106,249,59,177,182,126,177,42,209,
188,163,183,159,60,237,13,156,250,228,82,164,14,100,255,62,221,45,139,227,15,133,113,174,127,0,219,119,29,146,189,35,149,226,137,196,181,186,216,87,231,200,56,78,158,237,141,165,241,84,255,209,3,175,163,
141,31,168,164,181,253,176,21,21,197,22,213,134,177,117,75,171,229,50,27,127,252,156,124,125,252,204,5,60,180,239,8,252,19,147,117,63,119,245,70,2,151,52,136,167,238,167,76,126,199,31,95,143,221,183,192,
86,164,250,24,46,151,169,148,255,125,153,116,120,214,122,103,164,120,42,242,233,220,55,167,166,148,31,175,116,150,196,51,107,185,233,179,186,247,186,47,227,233,215,78,160,131,3,95,14,108,109,93,21,158,
190,127,43,154,151,215,100,85,207,156,40,158,138,248,35,109,255,128,243,166,39,103,30,209,113,249,51,249,153,159,60,255,147,172,234,57,231,196,157,35,222,156,119,3,126,102,194,122,33,231,196,117,89,78,
66,116,42,109,100,232,110,49,87,159,171,160,167,130,226,89,18,215,169,68,92,247,127,226,185,33,174,77,99,69,150,170,66,153,230,159,175,114,18,114,11,217,182,164,78,157,137,141,144,107,197,121,69,150,141,
18,153,230,159,175,114,18,21,23,178,115,245,76,243,207,87,57,9,185,245,89,18,215,171,68,92,127,171,17,175,46,181,226,250,112,110,255,98,94,104,53,169,64,92,159,221,3,159,221,214,140,157,255,252,0,3,163,
227,57,33,93,110,49,98,215,55,191,130,92,215,51,231,138,223,190,108,17,222,127,252,222,57,143,234,57,87,92,212,235,103,47,148,230,201,188,23,151,116,98,161,146,69,235,105,18,5,140,249,51,127,93,69,155,
236,129,169,96,54,72,216,119,223,230,216,245,128,199,151,114,127,123,46,192,101,70,203,111,107,189,29,54,147,113,214,122,167,65,92,136,161,178,216,140,150,101,149,177,107,179,65,196,222,111,109,196,166,
218,5,177,180,223,157,252,24,131,99,62,213,148,230,178,184,204,104,249,171,42,75,113,224,254,102,148,154,13,177,180,173,95,92,36,215,61,158,75,70,174,174,163,126,180,251,158,38,60,245,70,39,58,250,93,
248,237,221,107,177,97,73,228,159,211,69,182,34,106,121,47,58,28,67,216,254,151,183,230,197,213,203,205,145,125,255,219,136,252,11,219,54,226,137,163,255,193,234,170,82,60,115,231,106,180,190,252,14,196,
192,68,250,125,92,138,35,174,165,165,159,129,90,235,55,119,173,67,159,123,20,13,229,214,216,111,63,223,178,10,207,189,211,5,215,152,127,94,250,120,169,73,196,207,90,26,99,49,102,229,194,18,28,216,190,
17,213,54,19,4,173,22,33,104,166,112,201,72,241,176,242,80,73,208,78,33,205,214,84,93,134,35,223,105,145,191,127,116,213,133,95,157,236,130,63,52,49,167,100,181,84,159,39,91,86,160,165,174,60,233,239,
117,165,150,41,60,2,97,77,6,138,139,66,234,229,224,244,61,49,135,27,207,190,219,13,13,69,122,73,133,221,151,23,62,184,64,43,82,45,54,215,216,83,55,18,215,71,20,62,255,112,198,42,138,51,16,58,125,125,8,
123,222,239,145,119,67,68,21,183,155,247,181,247,82,55,212,96,83,117,89,42,230,41,235,148,68,241,169,196,67,225,228,170,95,112,121,240,135,15,47,145,210,58,82,90,253,61,246,131,29,87,80,76,163,204,202,
10,107,210,223,167,243,200,40,184,69,136,39,127,123,185,166,216,136,219,22,216,112,118,96,100,94,130,219,234,5,86,52,216,147,255,139,195,94,42,233,51,37,62,173,165,156,99,1,88,165,196,135,72,180,30,254,
81,83,29,14,158,254,20,93,55,70,85,37,189,170,162,8,59,190,188,152,162,119,242,248,115,205,227,207,94,241,125,157,87,103,204,92,105,145,228,183,5,165,52,166,185,185,180,110,183,23,79,188,213,147,242,158,
116,21,15,167,155,33,222,92,190,137,140,243,220,2,22,142,18,15,199,19,23,245,2,10,220,100,190,204,146,255,252,12,100,178,58,203,79,139,77,174,152,107,136,137,243,10,195,195,67,22,247,87,35,13,250,161,
112,97,81,214,203,49,208,31,29,150,57,18,203,71,51,120,60,26,224,151,101,248,12,71,141,81,131,79,253,186,130,34,94,45,134,148,93,27,217,155,249,100,210,8,19,119,17,174,72,146,52,72,196,203,26,12,65,184,
194,34,252,225,194,120,125,91,212,132,209,96,136,44,155,137,35,191,35,194,199,177,220,81,226,61,102,179,249,248,248,248,248,3,118,4,209,108,26,71,111,80,194,181,144,136,124,246,250,42,157,31,245,130,15,
101,218,144,188,29,205,28,17,57,131,38,31,191,226,3,104,189,228,255,239,150,148,148,84,184,221,238,175,149,5,131,40,19,121,59,199,83,16,170,51,105,226,246,166,114,246,140,15,222,13,9,10,59,126,111,163,
147,111,176,219,237,159,121,60,158,59,124,62,159,157,251,124,94,187,185,40,178,123,15,178,210,44,44,34,7,238,28,114,48,167,72,158,236,136,229,50,76,30,177,52,35,114,156,33,95,142,46,76,40,67,150,7,147,
71,44,123,48,237,136,165,198,225,112,160,178,178,50,254,80,45,31,199,42,85,62,121,233,35,33,63,15,213,250,148,17,203,165,128,15,215,198,14,213,254,79,128,1,0,17,13,140,35,89,73,58,153,0,0,0,0,73,69,78,
68,174,66,96,130,0,0};

const char* MainComponent::fullscreen_on_png = (const char*) resource_MainComponent_fullscreen_on_png;
const int MainComponent::fullscreen_on_pngSize = 3178;

// JUCER_RESOURCE: logoText_2x_png, 3618, "graphics/LogoText@2x.png"
static const unsigned char resource_MainComponent_logoText_2x_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,1,10,0,0,0,136,8,2,0,0,0,144,166,50,249,0,0,0,25,116,69,88,116,83,111,102,116,
119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,99,
107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,116,
97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,54,
48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,34,
104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,112,
116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,
34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,116,82,
101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,58,67,
114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,
101,73,68,61,34,120,109,112,46,105,105,100,58,66,56,65,50,53,54,69,70,70,57,66,49,49,49,69,49,65,68,69,67,57,69,55,54,57,66,52,67,66,57,66,52,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,
68,61,34,120,109,112,46,100,105,100,58,66,56,65,50,53,54,70,48,70,57,66,49,49,49,69,49,65,68,69,67,57,69,55,54,57,66,52,67,66,57,66,52,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,
111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,66,56,65,50,53,54,69,68,70,57,66,49,49,49,69,49,65,68,69,67,57,69,55,54,57,66,52,67,66,57,66,52,
34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,66,56,65,50,53,54,69,69,70,57,66,49,49,49,69,49,65,68,69,67,57,69,55,54,57,66,52,67,66,57,66,52,34,
47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,110,
100,61,34,114,34,63,62,126,80,6,226,0,0,10,150,73,68,65,84,120,218,236,157,177,114,19,59,20,134,51,119,220,19,158,0,38,47,128,25,30,128,20,73,79,147,58,84,161,133,42,41,83,66,101,90,210,36,180,164,48,
125,50,147,208,147,9,60,0,118,120,129,96,158,32,247,191,214,189,186,66,218,149,142,180,187,142,180,251,127,69,198,24,105,189,182,207,191,231,28,157,163,245,218,26,33,132,16,66,8,33,132,16,66,8,33,132,
16,66,8,33,164,77,198,227,113,110,167,244,23,191,21,146,9,143,31,63,166,60,8,161,247,32,36,158,205,205,77,202,131,144,50,160,60,72,46,60,120,240,128,242,32,253,103,125,125,157,185,7,33,45,167,16,185,41,
132,242,32,237,147,108,229,105,110,135,242,32,37,241,252,249,115,166,230,132,100,17,146,17,82,18,119,119,119,105,179,166,211,41,189,7,97,250,193,220,131,12,152,220,12,157,242,32,5,251,141,12,139,30,148,
7,233,202,111,196,182,223,166,205,162,60,72,145,164,25,58,229,65,40,12,230,30,100,216,242,136,237,47,100,238,65,152,160,7,114,143,181,204,150,188,40,15,210,50,13,251,210,179,114,35,148,7,9,243,242,229,
203,210,195,36,202,131,116,152,78,196,54,68,197,142,127,244,232,17,229,65,138,228,201,147,39,93,167,4,121,174,119,81,30,68,100,187,242,144,169,97,112,149,85,223,46,229,65,68,22,15,7,34,28,172,253,76,15,
146,16,202,131,136,188,65,66,240,19,21,143,49,184,34,165,70,86,114,87,144,108,229,148,7,41,216,123,8,21,98,90,121,154,197,103,181,17,151,242,32,1,116,214,17,187,120,213,131,230,43,202,131,72,83,8,201,
154,82,218,250,111,182,123,167,40,15,18,64,171,66,82,185,51,3,48,121,119,73,182,107,92,148,7,233,42,151,232,193,182,65,202,131,72,229,177,154,130,29,59,118,73,49,88,215,242,160,237,202,171,135,76,205,
73,241,88,249,70,48,242,49,245,35,247,54,217,222,0,142,242,32,17,222,99,53,137,65,62,43,194,148,7,137,176,212,96,112,213,138,101,83,30,164,72,121,4,75,218,214,248,210,43,131,148,7,145,70,86,107,157,21,
206,179,77,232,41,15,34,202,179,59,205,61,172,23,202,103,109,151,242,32,181,84,46,40,121,28,130,59,190,244,95,81,11,203,99,99,99,99,127,127,255,195,135,15,103,103,103,119,127,114,123,123,139,39,159,61,
123,70,75,26,108,54,82,138,161,167,49,242,11,227,237,219,183,59,59,59,117,3,30,62,124,184,181,181,133,191,52,154,94,178,178,222,242,108,51,248,90,121,236,237,237,193,99,72,14,1,239,113,126,126,94,247,
95,74,60,191,126,253,186,186,186,162,193,149,197,205,205,205,106,172,188,48,121,184,218,56,95,50,155,205,96,232,150,0,60,118,15,231,3,247,162,166,111,111,111,211,224,202,226,227,199,143,214,29,174,32,
152,203,203,75,185,149,167,249,159,172,127,154,16,6,109,38,24,159,62,125,66,148,149,118,40,157,174,224,1,173,173,68,32,15,92,16,213,151,56,159,207,253,185,196,225,225,161,149,157,94,92,92,72,94,37,109,
214,253,120,15,211,111,28,29,29,189,122,245,138,86,50,88,78,150,108,110,110,46,22,139,111,223,190,117,241,18,57,255,142,212,200,205,22,180,175,64,40,69,109,16,224,9,168,252,65,145,36,169,200,121,117,107,
228,70,86,250,241,233,233,41,45,67,178,50,193,229,135,118,115,238,124,4,227,91,216,181,178,112,33,72,199,117,37,196,124,80,151,126,84,166,236,170,156,98,45,25,171,181,1,152,32,116,155,118,110,250,100,
112,21,216,88,98,93,17,244,41,213,173,197,105,118,254,195,122,30,231,134,185,8,74,169,141,30,134,91,251,251,251,102,82,222,36,29,23,34,201,213,92,144,32,197,214,91,48,30,239,238,199,143,31,193,131,187,
130,177,212,21,124,143,24,48,192,114,208,245,245,181,240,251,53,65,98,147,48,235,126,192,229,208,60,203,132,138,120,115,121,224,69,37,19,111,111,111,229,167,7,97,96,188,240,148,60,242,192,43,90,199,249,
250,245,235,217,18,235,32,120,126,104,10,169,252,48,131,145,146,187,222,149,175,60,240,141,38,155,160,231,243,138,90,216,173,44,198,227,52,84,53,198,58,189,160,9,98,0,44,213,154,5,199,136,163,153,175,
98,186,205,58,121,152,218,192,3,76,177,94,29,23,23,211,59,165,185,223,114,209,75,192,38,193,244,163,82,30,249,198,87,72,30,44,99,130,29,172,82,30,126,144,48,152,230,238,47,237,99,176,105,175,120,12,85,
84,42,42,40,15,204,210,135,242,92,53,44,53,250,227,180,158,113,124,124,108,89,249,124,62,15,206,170,148,71,182,219,107,43,46,183,42,84,72,248,166,187,144,135,101,169,160,174,106,105,189,17,127,186,18,
148,135,196,189,104,77,118,244,198,243,79,169,161,7,253,222,225,76,36,107,80,133,201,163,78,33,234,203,142,18,73,119,86,2,39,160,15,142,199,65,55,24,236,31,243,91,63,62,16,29,86,73,90,209,204,32,48,185,
231,160,80,133,188,126,253,26,22,143,191,194,0,201,245,57,174,60,16,161,29,46,137,250,25,183,110,21,98,69,89,166,173,11,19,146,238,228,97,94,161,43,99,63,115,0,92,77,48,69,241,203,195,84,163,196,220,205,
198,156,58,245,18,197,197,197,133,95,30,144,132,249,95,215,215,215,43,203,76,70,158,52,235,224,224,224,244,244,84,183,21,154,223,61,124,203,187,119,239,48,96,197,31,165,42,86,192,214,77,27,197,249,224,
100,220,37,56,253,248,232,232,168,73,157,100,205,40,224,204,102,51,93,48,145,95,104,168,129,88,116,66,15,37,76,38,19,243,191,16,176,65,48,239,223,191,207,229,92,97,127,149,203,181,193,213,253,230,222,
3,199,135,161,67,162,158,245,226,202,131,155,227,37,6,234,247,30,149,161,166,144,161,173,95,181,226,61,116,16,85,89,21,201,167,103,49,32,18,255,119,223,68,30,120,57,28,92,98,127,149,7,55,35,43,201,203,
249,229,113,215,0,118,43,251,65,150,226,89,14,190,95,121,140,228,67,213,150,15,92,203,205,37,32,252,19,177,117,187,61,20,56,56,132,225,218,232,213,213,149,106,42,65,164,132,7,66,179,195,200,22,207,77,
125,8,81,83,26,198,117,189,231,228,228,100,119,119,215,92,227,122,243,230,77,236,78,172,251,151,135,2,217,136,50,77,173,144,118,229,161,90,54,244,193,97,91,170,139,41,147,254,72,156,137,155,231,144,38,
44,22,139,167,79,159,34,154,82,30,227,114,73,91,203,104,56,44,254,226,37,32,66,252,93,209,91,178,122,79,234,130,251,216,0,195,42,104,248,43,21,254,131,235,117,216,86,130,43,125,86,76,36,86,12,140,219,
13,174,14,15,15,131,19,225,142,204,42,254,124,62,79,104,31,78,188,145,143,213,51,219,214,205,74,224,136,244,162,144,218,137,149,28,153,232,222,242,216,133,38,255,209,6,85,5,207,196,183,164,77,156,76,38,
230,250,47,180,97,173,128,117,40,15,211,98,218,117,74,58,166,106,184,106,108,102,8,158,155,173,196,190,89,120,51,214,49,138,192,173,187,191,120,241,98,117,242,136,202,62,133,107,255,218,11,169,252,187,
201,41,153,185,10,2,167,134,254,205,172,156,36,119,160,145,226,72,148,135,106,170,213,218,168,243,36,218,164,220,189,77,93,51,155,205,116,14,141,151,78,216,31,98,189,17,125,52,132,106,194,91,28,145,142,
226,171,180,117,173,213,165,230,102,79,145,199,86,204,194,133,228,162,107,86,223,130,214,28,204,251,173,68,31,143,61,153,67,84,199,174,176,243,138,180,130,91,55,148,52,44,222,79,181,68,93,137,205,118,
119,79,226,107,182,42,97,100,48,175,141,218,171,40,89,22,115,119,47,97,112,101,42,18,187,223,35,185,139,153,172,70,30,238,172,14,229,1,13,168,130,160,101,109,193,172,215,218,188,138,35,96,138,167,5,221,
60,190,191,65,88,184,106,12,155,118,91,66,212,221,129,33,9,181,41,10,152,154,143,218,45,136,55,168,218,210,172,244,198,140,63,73,31,228,161,250,76,245,238,80,77,221,94,86,201,26,142,107,79,254,109,147,
149,246,7,219,133,41,111,253,73,84,81,165,173,205,180,234,74,33,220,48,140,87,164,113,231,35,143,227,227,227,224,172,241,120,140,131,87,119,4,91,247,71,244,183,217,201,139,9,30,123,170,180,108,185,253,
197,214,28,33,18,73,115,161,228,170,95,215,166,201,110,171,214,113,247,75,73,102,77,167,211,168,98,34,132,97,222,74,66,245,68,254,209,84,18,44,101,96,128,186,75,77,84,35,19,6,111,111,111,171,219,231,152,
119,208,89,171,89,17,86,227,49,18,97,152,26,223,202,167,172,86,159,212,2,148,142,133,180,18,98,171,135,170,249,10,83,212,161,240,192,90,157,195,187,136,237,206,34,45,242,253,251,247,168,66,7,124,139,217,
247,133,127,46,22,139,145,101,64,176,75,107,55,133,122,94,247,2,38,159,238,213,18,121,195,146,110,254,83,134,235,22,46,212,89,165,221,124,77,31,92,159,15,28,11,82,8,143,104,235,148,207,91,90,245,128,241,
18,235,201,221,221,221,145,107,55,185,157,250,108,73,215,39,102,202,143,55,59,236,37,158,78,199,202,100,3,79,242,199,211,254,207,37,168,141,60,177,138,128,194,59,97,71,221,48,187,178,98,136,39,41,143,
127,35,43,157,54,240,206,194,153,203,67,88,252,142,170,145,87,106,9,217,11,229,241,79,88,165,43,250,106,123,9,63,147,94,226,247,39,149,173,43,67,151,199,222,222,158,185,251,10,153,122,187,187,11,73,38,
62,39,232,79,92,241,100,178,99,241,30,80,125,233,110,69,159,86,149,33,227,241,216,252,154,166,211,169,112,98,84,181,196,189,221,22,82,243,81,239,101,160,150,164,244,106,181,174,81,88,35,15,14,14,184,75,
182,136,160,8,41,65,194,65,130,169,200,207,159,63,155,100,47,165,230,21,146,162,59,127,154,61,115,98,119,210,186,179,130,13,87,214,45,81,212,248,158,123,15,207,42,109,90,7,0,41,136,203,203,75,249,189,
122,45,95,161,254,57,234,253,103,164,235,137,170,188,184,246,95,253,158,247,215,33,173,135,112,132,220,3,102,251,173,220,33,152,179,36,33,153,217,146,168,250,181,88,247,32,67,113,8,81,241,149,90,213,165,
60,72,111,249,253,251,119,212,248,47,95,190,88,210,162,60,200,32,144,212,248,244,24,253,128,242,32,133,133,73,242,144,201,13,150,40,15,210,243,48,73,94,173,139,205,61,116,199,187,142,178,40,15,50,8,132,
162,82,126,131,222,131,12,55,60,163,60,72,63,19,235,168,46,218,132,31,66,80,97,149,158,72,121,144,222,202,35,45,6,51,95,130,242,32,253,71,238,70,16,131,81,30,132,41,71,237,120,115,10,229,65,10,64,175,
59,197,238,193,72,24,111,54,35,82,30,164,36,39,144,214,72,27,149,177,156,156,156,80,30,100,64,57,189,187,19,80,8,229,65,250,76,178,48,40,15,50,184,212,133,242,32,253,68,45,206,166,253,228,121,236,250,
21,229,65,6,36,42,6,87,132,180,15,229,65,232,67,8,41,156,201,100,34,252,93,40,147,245,245,245,132,89,244,30,164,48,98,55,142,43,26,222,233,144,242,32,61,39,121,217,138,242,32,148,7,229,65,202,231,230,
230,38,205,208,155,20,206,41,15,82,140,60,210,18,137,38,59,168,40,15,210,115,62,127,254,76,121,16,82,77,147,197,43,202,131,20,99,229,171,255,53,51,202,131,148,1,242,242,134,221,233,148,7,33,148,7,33,148,
7,33,38,195,253,37,101,66,8,33,132,16,66,8,33,132,16,66,8,33,132,16,66,72,103,252,45,192,0,45,130,38,157,98,189,0,174,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* MainComponent::logoText_2x_png = (const char*) resource_MainComponent_logoText_2x_png;
const int MainComponent::logoText_2x_pngSize = 3618;
