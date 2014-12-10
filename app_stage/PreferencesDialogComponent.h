/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_7D3C02650FD1C6FA__
#define __JUCE_HEADER_7D3C02650FD1C6FA__

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

#include "WidgetComponent.h"
#include "juce.h"
#include "DialogFrameworkComponent.h"

#include "TabViewPreferencesComponent.h"
#include "NetworkPreferencesComponent.h"
#include "ViewPreferencesComponent.h"

class TabViewPreferencesComponent;
class NetworkPreferencesComponent;
class ViewPreferencesComponent;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class PreferencesDialogComponent  : public Component,
                                    public TextEditorListener,
                                    public ButtonListener
{
public:
    //==============================================================================
    PreferencesDialogComponent (StagePreferences* preferences);
    ~PreferencesDialogComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void initialize(DialogFrameworkComponent* framework);
    void setFocusOrder(DialogFrameworkComponent* framework);

    String getName();
    String getTextPosition();
    bool getUserEnabled();

    void setDoubleEditorValue(TextEditor* editor, double value);
    //double getUserMin();
    double getUserStep();
    //double getUserMax();

    // text editor listener virtual functions
    void textEditorTextChanged (TextEditor& editor) {}
    void textEditorReturnKeyPressed(TextEditor& editor);
    void textEditorEscapeKeyPressed(TextEditor& editor);
    void textEditorFocusLost(TextEditor& editor) {}

    TabViewPreferencesComponent* getTabViewPreferences();
    NetworkPreferencesComponent* getNetworkPreferences();
    ViewPreferencesComponent* getViewPreferences();

    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    DialogFrameworkComponent* framework_;
    StagePreferences* preferences_;

    // the preferences panels
    TabViewPreferencesComponent* tabView_;
    NetworkPreferencesComponent* network_;
    ViewPreferencesComponent* view_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<TextButton> tabsButton;
    ScopedPointer<TextButton> networkButton;
    ScopedPointer<TextButton> viewButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreferencesDialogComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_7D3C02650FD1C6FA__
