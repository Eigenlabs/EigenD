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

#include "BoolPropertyEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
BoolPropertyEditor::BoolPropertyEditor ()
{
    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("label text")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (toggleButton = new ToggleButton ("new toggle button"));
    toggleButton->setButtonText (TRANS("Off"));
    toggleButton->addListener (this);
    toggleButton->setColour (ToggleButton::textColourId, Colours::white);


    //[UserPreSize]
    LookAndFeel::getDefaultLookAndFeel().setColour(TextButton::buttonColourId,Colour(0xffaeaeae));
    disabled_=false;
    warning_=String::empty;
    atom_=0;
    //[/UserPreSize]

    setSize (400, 28);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

BoolPropertyEditor::~BoolPropertyEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;
    toggleButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void BoolPropertyEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void BoolPropertyEditor::resized()
{
    label->setBounds (8, 8, proportionOfWidth (0.3725f), 16);
    toggleButton->setBounds (getWidth() - 25 - 47, 8, 47, 16);
    //[UserResized] Add your own custom resize handling here..
    label->setBounds (getX(), 8, proportionOfWidth (0.3725f), 16);
    //[/UserResized]
}

void BoolPropertyEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    pic::logmsg()<<"BoolPropertyEditor buttonClicked";
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == toggleButton)
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        if(toggleButton->getToggleState())
        {
            toggleButton->setButtonText ("On");
            if(atom_!=0)
            {
                atom_->set_value(true);
            }
        }
        else
        {
            if(warning_.isNotEmpty())
            {
                HeadPhoneWarningComponent* e=new HeadPhoneWarningComponent(warning_);

                DialogWindow::showModalDialog(String("Warning"),e,this,Colour (0xffababab),true);
                if(e->okPressed())
                {
                    setOff();
                }
                else
                {
                    toggleButton->setToggleState(true,false);
                    toggleButton->setButtonText ("On");
                }
                delete e;
            }
            else
            {
                setOff();
            }
        }
        //[/UserButtonCode_toggleButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void BoolPropertyEditor::setOff()
{
    toggleButton->setButtonText ("Off");
    if(atom_!=0)
    {
        atom_->set_value(false);
    }

}

void BoolPropertyEditor::refreshValue()
{
    bool value=atom_->get_value().equalsIgnoreCase(String("true"));
    toggleButton->setToggleState(value,false);

    if (value)
    {
        toggleButton->setButtonText ("On");
    }
    else
    {
        toggleButton->setButtonText ("Off");
    }

}

void BoolPropertyEditor::setItem(String name, bool value)
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

    toggleButton->setToggleState(value,false);

    if (value)
    {
        toggleButton->setButtonText ("On");
    }
    else
    {
        toggleButton->setButtonText ("Off");
    }
}

void BoolPropertyEditor::setDisabled(bool disabled)
{
    disabled_=disabled;
    toggleButton->setEnabled(!disabled);
}

void BoolPropertyEditor::setAtom(Atom* atom)
{
    atom_=atom;
    warning_=atom_->getProperty("show_warning");
}

void BoolPropertyEditor::setIndent(int indent)
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

<JUCER_COMPONENT documentType="Component" className="BoolPropertyEditor" componentName=""
                 parentClasses="public Component, public AtomEditor" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="400" initialHeight="28">
  <BACKGROUND backgroundColour="ffffff"/>
  <LABEL name="new label" id="d295b9c350a86ef6" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="8 8 37.25% 16" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="label text" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="857f7c475b853b9a" memberName="toggleButton"
                virtualName="" explicitFocusOrder="0" pos="25Rr 8 47 16" txtcol="ffffffff"
                buttonText="Off" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
