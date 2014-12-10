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

#ifndef __JUCE_HEADER_457F9E02DC4459CA__
#define __JUCE_HEADER_457F9E02DC4459CA__

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
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class WidgetDialogComponent  : public Component,
                               public TextEditorListener,
                               public ComboBoxListener,
                               public ButtonListener
{
public:
    //==============================================================================
    WidgetDialogComponent ();
    ~WidgetDialogComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void initialize(DialogFrameworkComponent* framework, WidgetComponent* widget);
    void setFocusOrder(DialogFrameworkComponent* framework);

    String getName();
    String getWidgetType();
    String getTextPosition();
    bool getUserEnabled();

    void setDoubleEditorValue(TextEditor* editor, double value);
    double getUserMin();
    double getUserStep();
    double getUserMax();

    // text editor listener virtual functions
    void textEditorTextChanged (TextEditor& editor) {}
    void textEditorReturnKeyPressed(TextEditor& editor);
    void textEditorEscapeKeyPressed(TextEditor& editor);
    void textEditorFocusLost(TextEditor& editor) {}
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    DialogFrameworkComponent* framework_;
    String textPosition_;
    WidgetComponent* widget_;
    bool userEnabled_;
    double userMin_;
    double userStep_;
    Array<String> comboData_;
    double userMax_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<TextEditor> nameEditor;
    ScopedPointer<ComboBox> typeCombo;
    ScopedPointer<ToggleButton> leftToggleButton;
    ScopedPointer<ToggleButton> rightToggleButton;
    ScopedPointer<ToggleButton> bottomToggleButton;
    ScopedPointer<ToggleButton> topToggleButton;
    ScopedPointer<Label> typeLabel;
    ScopedPointer<Label> nameLabel;
    ScopedPointer<Label> positionLabel;
    ScopedPointer<ToggleButton> enableToggleButton;
    ScopedPointer<Label> enabledLabel;
    ScopedPointer<Label> minLabel;
    ScopedPointer<TextEditor> minEditor;
    ScopedPointer<Label> maxLabel;
    ScopedPointer<TextEditor> maxEditor;
    ScopedPointer<Label> stepLabel;
    ScopedPointer<TextEditor> stepEditor;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WidgetDialogComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_457F9E02DC4459CA__
