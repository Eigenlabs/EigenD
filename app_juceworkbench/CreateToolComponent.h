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

#ifndef __JUCE_HEADER_306A476631AC3FBE__
#define __JUCE_HEADER_306A476631AC3FBE__

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
//#include "Workspace.h"
#include <set>
#include <map>
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class CreateToolComponent  : public Component,
                             public TextEditor::Listener,
                             public ComboBoxListener,
                             public ButtonListener
{
public:
    //==============================================================================
    CreateToolComponent (const std::set<std::string>& agents);
    ~CreateToolComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
//    CreateToolComponent (const std::set<std::string>& agentNames);
    String getSelection();
    void setSelection(String selection);
    int getOrdinal();
    bool okPressed_;
    virtual void textEditorTextChanged(TextEditor &editor);
    virtual void textEditorReturnKeyPressed(TextEditor &editor){};
    virtual void textEditorEscapeKeyPressed(TextEditor &editor){};
    virtual void textEditorFocusLost(TextEditor &editor){};

    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    std::set<std::string> agents_;
    void setupAgentCombo();
    void setNextAvailableOrdinal();
    void setPreviousAvailableOrdinal();
    void setLowestAvailableOrdinal();
    void setupOrdinals();
    std::map<String,std::set<int> > ordinalMap_;
    bool isValidOrdinal(String ord);
    int ordinal_;

    //[/UserVariables]

    //==============================================================================
    ScopedPointer<ComboBox> comboBox;
    ScopedPointer<TextButton> textButton;
    ScopedPointer<TextButton> textButton2;
    ScopedPointer<TextEditor> txtOrdinal;
    ScopedPointer<TextButton> upButton;
    ScopedPointer<TextButton> downButton;
    ScopedPointer<Label> label;
    ScopedPointer<Label> errorLabel;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CreateToolComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_306A476631AC3FBE__
