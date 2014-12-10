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

#include "NamePropertyEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
NamePropertyTextEditor::NamePropertyTextEditor(String name):TextEditor(name)
{
}

void NamePropertyTextEditor::focusLost(FocusChangeType cause)
{
    TextEditor::focusLost(cause);
    NamePropertyEditor* c=dynamic_cast<NamePropertyEditor*>(getParentComponent());
    if(c!=0)
    {
        if(!(c->removing()))
        {
            std::cout<<"FocusLost: cause="<<cause<<" set value of atom to "<<std::string(getText().toUTF8())<<std::endl;
            c->set_value(getText());
        }
        else
        {
            std::cout<<"Focus lost on now invalid NamePropertyEditor"<<std::endl;
        }

    }
}

//[/MiscUserDefs]

//==============================================================================
NamePropertyEditor::NamePropertyEditor (Atom* atom)
    : atom_(atom)
{
    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("name")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (textEditor = new NamePropertyTextEditor ("new text editor"));
    textEditor->setTooltip (TRANS("name"));
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (String::empty);


    //[UserPreSize]
    setting_=false;
    originalName_=atom_->get_name();
    textEditor->setInputRestrictions(0,"0123456789 abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    textEditor->setText(originalName_);
    textEditor->setTooltip(originalName_);
    textEditor->addListener(this);
    //[/UserPreSize]

    setSize (400, 28);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

NamePropertyEditor::~NamePropertyEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;
    textEditor = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void NamePropertyEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void NamePropertyEditor::resized()
{
    label->setBounds (8, 8, 96, 16);
    textEditor->setBounds (getWidth() - 296, 8, 260, 16);
    //[UserResized] Add your own custom resize handling here..
    label->setBounds (getX(), 8, 72, 16);
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void NamePropertyEditor:: textEditorTextChanged(TextEditor &editor)
{
}

void NamePropertyEditor::textEditorReturnKeyPressed(TextEditor &editor)
{
    //std::cout<<"NamePropertyEditor: textEditorReturnKeyPressed"<<std::endl;
    set_value(textEditor->getText());
}

void NamePropertyEditor::setIndent(int indent)
{
    label->setTopLeftPosition(indent,8);
}

void NamePropertyEditor::set_value(String newVal)
{
    if(!setting_)
    {
        setting_=true;
        atom_->setName(newVal);
        textEditor->setTooltip(newVal);
        setting_=false;
    }
}





//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="NamePropertyEditor" componentName=""
                 parentClasses="public Component, public TextEditor::Listener, public AtomEditor"
                 constructorParams="Atom* atom" variableInitialisers="atom_(atom)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="400" initialHeight="28">
  <BACKGROUND backgroundColour="ffffff"/>
  <LABEL name="new label" id="7aa4ef07aa2c057b" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="8 8 96 16" textCol="ffffffff" edTextCol="ff000000"
         edBkgCol="0" labelText="name" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="new text editor" id="cb8cab162cf44545" memberName="textEditor"
              virtualName="NamePropertyTextEditor" explicitFocusOrder="0" pos="296R 8 260 16"
              tooltip="name" initialText="" multiline="0" retKeyStartsLine="0"
              readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
