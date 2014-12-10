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

//[Headers] You can add your own extra header files here...
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
//[/Headers]

#include "StringPropertyEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
StringPropertyTextEditor::StringPropertyTextEditor(String name):TextEditor(name)
{
}

void StringPropertyTextEditor::focusLost(FocusChangeType cause)
{
    TextEditor::focusLost(cause);
//    std::cout<<"FocusLost: cause="<<cause<<std::endl;
    StringPropertyEditor* c=dynamic_cast<StringPropertyEditor*>(getParentComponent());
    if(c!=0)
    {
        if(!(c->removing()))
        {
            std::cout<<"FocusLost: cause="<<cause<<" set value of atom to "<<std::string(getText().toUTF8())<<std::endl;
            c->set_value(getText());
        }
        else
        {
            std::cout<<"Focus lost on now invalid StringPropertyEditor"<<std::endl;
        }
    }
}
//[/MiscUserDefs]

//==============================================================================
StringPropertyEditor::StringPropertyEditor ()
{
    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("label text")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (textEditor = new StringPropertyTextEditor ("new text editor"));
    textEditor->setTooltip (TRANS("test"));
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (String::empty);


    //[UserPreSize]
    disabled_=false;
    atom_=0;
    textEditor->addListener(this);
    //[/UserPreSize]

    setSize (400, 28);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

StringPropertyEditor::~StringPropertyEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;
    textEditor = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void StringPropertyEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void StringPropertyEditor::resized()
{
    label->setBounds (8, 8, proportionOfWidth (0.2200f), 16);
    textEditor->setBounds (getWidth() - 296, 8, 260, 16);
    //[UserResized] Add your own custom resize handling here..
    label->setBounds (getX(), 8, proportionOfWidth (0.1800f), 16);
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void StringPropertyEditor::refreshValue()
{
    //std::cout<<"StringPropertyEditor:refreshValue"<<std::endl;
    textEditor->setText(atom_->get_value());
}


void StringPropertyEditor::setItem(String name, String value)
{
    label->setText(name,dontSendNotification);
    String tooltip= atom_->get_tooltip();
    if(tooltip.isNotEmpty())
    {
        label->setTooltip(name+ ": "+tooltip);
    }
    else
    {
        label->setTooltip(name);
    }

    textEditor->setText (value);
    textEditor->setTooltip (value);
}

void StringPropertyEditor::setDisabled(bool disabled)
{
    disabled_=disabled;
    textEditor->setEnabled(!disabled);
}

void StringPropertyEditor::setAtom(Atom* atom)
{
    atom_=atom;
}

void StringPropertyEditor::textEditorTextChanged(TextEditor &editor)
{
//    std::cout<<"StringPropertyEditor: textEditorTextChanged"<<std::endl;
    textEditor->setTooltip (textEditor->getText());
}

void StringPropertyEditor::textEditorReturnKeyPressed(TextEditor &editor)
{
    //std::cout<<"StringPropertyEditor: textEditorReturnKeyPressed"<<std::endl;
    set_value(textEditor->getText());
}

void StringPropertyEditor::set_value(String newVal)
{
    if(newVal!=atom_->get_value())
    {
        atom_->set_value(newVal);
        textEditor->setTooltip(newVal);
    }
}

void StringPropertyEditor::setIndent(int indent)
{
    label->setTopLeftPosition(indent,8);
}



//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="StringPropertyEditor" componentName=""
                 parentClasses="public Component, public TextEditorListener, public AtomEditor"
                 constructorParams="" variableInitialisers="" snapPixels="4" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="400"
                 initialHeight="28">
  <BACKGROUND backgroundColour="ffffff"/>
  <LABEL name="new label" id="d295b9c350a86ef6" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="8 8 22% 16" textCol="ffffffff" edTextCol="ff000000"
         edBkgCol="0" labelText="label text" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="new text editor" id="47f2e7b2f1fb4881" memberName="textEditor"
              virtualName="StringPropertyTextEditor" explicitFocusOrder="0"
              pos="296R 8 260 16" tooltip="test" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
