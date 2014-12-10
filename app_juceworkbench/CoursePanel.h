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

#ifndef __JUCE_HEADER_A0041B6E2B33EB96__
#define __JUCE_HEADER_A0041B6E2B33EB96__

//[Headers]     -- You can add your own extra header files here --
/*
 Copyright 2012-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include <piw/piw_tsd.h>
#include "juce.h"
#include "StrumKeyPanel.h"
#include "CoursePanelContainer.h"
class StrumKeyPanel;

class CourseTextEditor: public TextEditor
{
public:

    CourseTextEditor(String name);
    virtual void focusLost(FocusChangeType);
};

//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class CoursePanel  : public Component,
                     public ButtonListener
{
public:
    //==============================================================================
    CoursePanel (int x, int y);
    ~CoursePanel();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    String makeMappingString();
    bool isValid();
    bool isEmpty();
    void updateMapping();
    void setValues(String);
//    void setInputValues(String,String);
//    void setOutputValue(String);

    bool removing;

    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    StrumKeyPanel* keyEditor_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> courseLabel;
    ScopedPointer<CourseTextEditor> courseBox;
    ScopedPointer<TextButton> removeButton;
    ScopedPointer<ToggleButton> breathButton;
    ScopedPointer<Label> breathLabel;
    ScopedPointer<Label> openKeyMusicLabel;
    ScopedPointer<Label> openKeyPhysLabel;
    ScopedPointer<CourseTextEditor> keyBox;
    ScopedPointer<Label> rowLabel;
    ScopedPointer<Label> columnLabel;
    ScopedPointer<CourseTextEditor> rowBox;
    ScopedPointer<CourseTextEditor> columnBox;
    ScopedPointer<GroupComponent> groupComponent;
    ScopedPointer<Label> rowLabel2;
    ScopedPointer<Label> columnLabel2;
    ScopedPointer<TextButton> addKeyButton;
    ScopedPointer<Viewport> viewport;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoursePanel)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_A0041B6E2B33EB96__
