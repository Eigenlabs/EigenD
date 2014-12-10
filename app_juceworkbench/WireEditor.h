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

#ifndef __JUCE_HEADER_BFE6D65EAF8A9B5C__
#define __JUCE_HEADER_BFE6D65EAF8A9B5C__

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
#include "Wire.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class WireEditor  : public Component,
                    public TextEditor::Listener,
                    public ButtonListener
{
public:
    //==============================================================================
    WireEditor (Wire* w);
    ~WireEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    virtual void textEditorTextChanged(TextEditor &editor);
    virtual void textEditorReturnKeyPressed(TextEditor &editor){};
    virtual void textEditorEscapeKeyPressed(TextEditor &editor){};
    virtual void textEditorFocusLost(TextEditor &editor){};

    bool changed();
    Wire* getWire();
    String getUsing();
    String getFilter();
    String getControl();
    void enableUsing(bool enable);
    void enableFilter(bool filter);
    void setUsingController(bool controller);
    void setFilterController(bool controller);
    void setControlController(bool controller);
    void setUsingText(int u);
    void setFilterText(String f);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    Wire* w_;
    int u_;
    int initial_u_;
    String f_;
    String initial_f_;
    bool c_;
    bool initial_c_;

    bool usingController_;
    bool filterController_;
    bool controlController_;

    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> routeLabel;
    ScopedPointer<Label> usingLabel;
    ScopedPointer<TextEditor> txtUsing;
    ScopedPointer<Label> filterLabel;
    ScopedPointer<TextEditor> textEditor;
    ScopedPointer<TextButton> incButton;
    ScopedPointer<TextButton> decButton;
    ScopedPointer<ToggleButton> ctlButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WireEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_BFE6D65EAF8A9B5C__
