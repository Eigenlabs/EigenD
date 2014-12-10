/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  11 Sep 2012 5:12:22pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_MAINCOMPONENT_IOSMAINCOMPONENT_4AFE8E85__
#define __JUCER_HEADER_MAINCOMPONENT_IOSMAINCOMPONENT_4AFE8E85__

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

#include "juce.h"
#include "WidgetTabbedComponent.h"
#include "EigenLookAndFeel.h"
#include "Network.h"
#include "Main.h"
#include "StagePreferences.h"

class StageWindow;
class WidgetTabbedComponent;
class OSCManager;
class XMLRPCManager;
class EigenLookAndFeel;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class MainComponent  : public Component
{
public:
    //==============================================================================
    MainComponent ();
    ~MainComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.

    // commands
    enum CommandIDs
    {
        // edit menu
        editNewTab              = 0x1000,
        editRenameTab           = 0x1001,
        editCloseTab            = 0x1002,
        editToggleLock          = 0x1003,

        // tool menu
        toolPerform             = 0x2000,
        toolCreate              = 0x2001,
        toolEdit                = 0x2002,
        toolMove                = 0x2003,
        toolSize                = 0x2004,
        toolDelete              = 0x2005,
        toolHelp                = 0x2006,
        toolMulti               = 0x2007,

        // view menu
        viewToggleFullScreen    = 0x3000,
        viewGreyOutsideCanvas   = 0x3001,

        // help menu
        helpStageHelp           = 0x4000,
        helpStageQuickStart     = 0x4001,

    };

    bool getEditMode() { return editMode_; }
    bool getLockMode() { return lockMode_; }

    void setMainWindow(StageWindow* stageWindow) { stageWindow_ = stageWindow; }
    StageWindow* getMainWindow() { return stageWindow_; }

    OSCManager* getOSCManager() { return OSCManager_; }
    XMLRPCManager* getXMLRPCManager() { return XMLRPCManager_; }
    WidgetTabbedComponent* getTabbedComponent() { return tabbedComponent; }

    CommandID getToolMode() {
        return toolMode_;
    }

	void userEditPreferences();
    void initializeModel();
    void shutdown();
    void preferencesChanged();

    //[/UserMethods]

    void paint (Graphics& g);
    void resized();

    // Binary resources:
    static const char* logoText_png;
    static const int logoText_pngSize;
    static const char* preferences_png;
    static const int preferences_pngSize;
    static const char* preferences_2x_png;
    static const int preferences_2x_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.

    bool editMode_;
    bool lockMode_;
    CommandID toolMode_;

	DrawableButton* prefsButton_;

    StageWindow* stageWindow_;

    // look and feel
    EigenLookAndFeel* eigenLookAndFeel_;

    // xml rpc manager
    XMLRPCManager* XMLRPCManager_;

    // osc manager
    OSCManager* OSCManager_;

	// ObjC delegate
	void* appDelegate_;

public:
    // Stage preferences
    StagePreferences* preferences_;

	Button::Listener* prefsButtonListener_;

private:
    //[/UserVariables]

    //==============================================================================
    WidgetTabbedComponent* tabbedComponent;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent);
};


#endif   // __JUCER_HEADER_MAINCOMPONENT_IOSMAINCOMPONENT_4AFE8E85__
