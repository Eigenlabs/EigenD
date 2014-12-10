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

#include "IntPropertyEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
IntPropertyEditor::IntPropertyEditor ()
{
    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("label text")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colour (0xfff8f4f4));
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (slider = new Slider ("new slider"));
    slider->setRange (0, 10, 0);
    slider->setSliderStyle (Slider::IncDecButtons);
    slider->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    slider->setColour (Slider::thumbColourId, Colour (0xffaeaeae));
    slider->addListener (this);


    //[UserPreSize]
    disabled_=false;
    atom_=0;
    //[/UserPreSize]

    setSize (400, 28);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

IntPropertyEditor::~IntPropertyEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;
    slider = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void IntPropertyEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void IntPropertyEditor::resized()
{
    label->setBounds (8, 8, proportionOfWidth (0.3725f), 16);
    slider->setBounds (getWidth() - 32 - 150, 8, 150, 16);
    //[UserResized] Add your own custom resize handling here..
    label->setBounds (getX(), 8, proportionOfWidth (0.3725f), 16);
    //[/UserResized]
}

void IntPropertyEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == slider)
    {
        //[UserSliderCode_slider] -- add your slider handling code here..
        atom_->set_value((int)slider->getValue());
        //[/UserSliderCode_slider]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void IntPropertyEditor::refreshValue()
{
    //std::cout<<"IntPropertyEditor:refreshValue"<<std::endl;
    slider->setValue(atom_->get_value().getIntValue(),dontSendNotification);
}


void IntPropertyEditor::setItem(String name, int value)
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

    slider->setValue(value,dontSendNotification);
}

void IntPropertyEditor::setRange(int min, int max)
{
    pic::logmsg()<<"IntPropertyEditor: setRange";
    slider->setRange(min,max,1);
    slider->setTooltip (L"Valid range = "+ String(min) + L" to " + String(max));
}

void IntPropertyEditor::setDisabled(bool disabled)
{
    disabled_=disabled;
    slider->setEnabled(!disabled);
}

void IntPropertyEditor::setAtom(Atom* atom)
{
    atom_=atom;
}

void IntPropertyEditor::setIndent(int indent)
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

<JUCER_COMPONENT documentType="Component" className="IntPropertyEditor" componentName=""
                 parentClasses="public Component, public AtomEditor" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="400" initialHeight="28">
  <BACKGROUND backgroundColour="ffffff"/>
  <LABEL name="new label" id="d295b9c350a86ef6" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="8 8 37.25% 16" textCol="fff8f4f4"
         edTextCol="ff000000" edBkgCol="0" labelText="label text" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <SLIDER name="new slider" id="2b8dd39d614c572b" memberName="slider" virtualName=""
          explicitFocusOrder="0" pos="32Rr 8 150 16" thumbcol="ffaeaeae"
          min="0" max="10" int="0" style="IncDecButtons" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
