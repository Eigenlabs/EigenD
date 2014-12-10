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

#include "StringMappingPropertyEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
KeyMapTextEditor::KeyMapTextEditor(String name):TextEditor(name)
{
}

void KeyMapTextEditor::focusGained(FocusChangeType cause)
{
    TextEditor::focusGained(cause);
    //std::cout<<"FocusGained "<<cause<<std::endl;
    if(cause!=2)
    {
        StringMappingPropertyEditor* c=dynamic_cast<StringMappingPropertyEditor*>(getParentComponent());
        if(c!=0)
        {
            c->showWarning();
        }
    }
}

//[/MiscUserDefs]

//==============================================================================
StringMappingPropertyEditor::StringMappingPropertyEditor (Atom* atom, ToolManager* tm)
    : atom_(atom), tm_(tm)
{
    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("label text")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (textEditor = new KeyMapTextEditor ("new text editor"));
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (String::empty);

    addAndMakeVisible (editButton = new TextButton ("editButton"));
    editButton->setButtonText (TRANS("Edit"));
    editButton->addListener (this);


    //[UserPreSize]
    mappingEditor_=0;
    setItem(atom_->get_desc(),atom->get_value());
    textEditor->addListener(this);
    //[/UserPreSize]

    setSize (400, 28);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

StringMappingPropertyEditor::~StringMappingPropertyEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;
    textEditor = nullptr;
    editButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void StringMappingPropertyEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void StringMappingPropertyEditor::resized()
{
    label->setBounds (8, 8, proportionOfWidth (0.2200f), 16);
    textEditor->setBounds (getWidth() - 296, 8, 216, 16);
    editButton->setBounds (getWidth() - 32 - 40, 8, 40, 16);
    //[UserResized] Add your own custom resize handling here..
    label->setBounds (getX(), 8, proportionOfWidth (0.1800f), 16);
    //[/UserResized]
}

void StringMappingPropertyEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == editButton)
    {
        //[UserButtonCode_editButton] -- add your button handler code here..
        String title="Edit " + atom_->get_fulldesc();
        mappingEditor_=new MappingEditor(atom_,name_,tm_);
        DialogWindow::showModalDialog(title,mappingEditor_,this,Colour(0xffababab),true);
        getTopLevelComponent()->toFront(true);
        delete mappingEditor_;
        mappingEditor_=0;

        //[/UserButtonCode_editButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void StringMappingPropertyEditor::refreshValue()
{
    //std::cout<<"StringMappingPropertyEditor:refreshValue"<<std::endl;
    textEditor->setText(atom_->get_value());
    textEditor->setTooltip(formatTooltip(textEditor->getText()));
    if(mappingEditor_!=0)
    {
        //std::cout<<"call refreshvalue on mappingEditor"<<std::endl;
        mappingEditor_->refreshValue();
    }
}

String StringMappingPropertyEditor::formatTooltip(String s)
{
    int endIndex=100;
    int startIndex=0;
    String formatedTooltip=String();
    while (startIndex<s.length())
    {
        formatedTooltip=formatedTooltip+s.substring(startIndex,endIndex)+"\n";
        endIndex=endIndex+100;
        startIndex=startIndex+100;
    }
    return formatedTooltip.upToLastOccurrenceOf("\n",false,true);
}

void StringMappingPropertyEditor::setItem(String name, String value)
{
    name_=name;
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
    textEditor->setTooltip(formatTooltip(value));
}

void StringMappingPropertyEditor::setAtom(Atom* atom)
{
    atom_=atom;
}

void StringMappingPropertyEditor::textEditorTextChanged(TextEditor &editor)
{
//    if(ready_)
//    {
//        if(textEditor->getText()!=atom_->get_value())
//        {
//            setButton->setEnabled(true);
//        }
//        else
//        {
//            setButton->setEnabled(false);
//        }
//
//    }
//    ready_=true;
}


void StringMappingPropertyEditor::textEditorReturnKeyPressed(TextEditor &editor)
{
    //std::cout<<"StringMappingPropertyEditor: textEditorReturnKeyPressed"<<std::endl;
    String val=textEditor->getText();

    if(val!=atom_->get_value())
    {
        atom_->set_value(val);
        textEditor->setTooltip(formatTooltip(val));
//        setButton->setEnabled(false);
    }
}

void StringMappingPropertyEditor::showWarning()
{
    if(!tm_->getPropertyValue(String("EditMapDirectly"),false))
    {
            EditMapConfirmation* da=new EditMapConfirmation();
            DialogWindow::showModalDialog("Edit mapping directly",da,this,Colour(0xffababab),true);

            if(da->dontShowAgain_&& da->okPressed_)
            {
               tm_->setPropertyValue(String("EditMapDirectly"),true);
            }

            delete da;
    }
}

void StringMappingPropertyEditor::sourcekeys_updated(String keys)
{
    if(mappingEditor_!=0)
    {
        mappingEditor_->setUpstreamKeys(keys);
    }
}

void StringMappingPropertyEditor::setIndent(int indent)
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

<JUCER_COMPONENT documentType="Component" className="StringMappingPropertyEditor"
                 componentName="" parentClasses="public Component, public TextEditorListener, public AtomEditor"
                 constructorParams="Atom* atom, ToolManager* tm" variableInitialisers="atom_(atom), tm_(tm)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="400" initialHeight="28">
  <BACKGROUND backgroundColour="ffffff"/>
  <LABEL name="new label" id="d295b9c350a86ef6" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="8 8 22% 16" textCol="ffffffff" edTextCol="ff000000"
         edBkgCol="0" labelText="label text" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="new text editor" id="47f2e7b2f1fb4881" memberName="textEditor"
              virtualName="KeyMapTextEditor" explicitFocusOrder="0" pos="296R 8 216 16"
              initialText="" multiline="0" retKeyStartsLine="0" readonly="0"
              scrollbars="1" caret="1" popupmenu="1"/>
  <TEXTBUTTON name="editButton" id="b2652457bc0147f9" memberName="editButton"
              virtualName="" explicitFocusOrder="0" pos="32Rr 8 40 16" buttonText="Edit"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
