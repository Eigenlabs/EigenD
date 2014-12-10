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

#ifndef __JUCE_HEADER_3C1514AC00827228__
#define __JUCE_HEADER_3C1514AC00827228__

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
#include "Workspace.h"
#include <vector>
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class FindComponent  : public Component,
                       public ListBoxModel,
                       public ButtonListener
{
public:
    //==============================================================================
    FindComponent (std::vector<String> names);
    ~FindComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    bool okPressed_;
    String getSelection();
    void setFocusOrder();
//    virtual void textEditorTextChanged(TextEditor &editor){};
//    virtual void textEditorReturnKeyPressed(TextEditor &editor);
//    virtual void textEditorEscapeKeyPressed(TextEditor &editor){};
//    virtual void textEditorFocusLost(TextEditor &editor){};
    virtual int getNumRows();
    virtual void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);
    virtual void selectedRowsChanged(int lastRowSelected);
    virtual String getTooltipForRow(int row);
    virtual void listBoxItemDoubleClicked(int row, const MouseEvent& e);

    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);
    bool keyPressed (const KeyPress& key);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    bool initialised_;
    std::vector<String> names_;
    StringArray displayNames_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<TextButton> findButton;
    ScopedPointer<TextButton> cancelButton;
    ScopedPointer<ListBox> listbox;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FindComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_3C1514AC00827228__
