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

#include "FloatPropertyEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
FloatPropertyTextEditor::FloatPropertyTextEditor(String name):TextEditor(name)
{
}

void FloatPropertyTextEditor::focusLost(FocusChangeType cause)
{
    TextEditor::focusLost(cause);
//    std::cout<<"FocusLost: cause="<<cause<<std::endl;
    FloatPropertyEditor* c=dynamic_cast<FloatPropertyEditor*>(getParentComponent());
    if(c!=0)
    {
        if(!(c->removing()))
        {
            //std::cout<<"FocusLost: cause="<<cause<<" set value of atom to "<<std::string(getText().toUTF8())<<std::endl;
            c->set_value(getText());
        }
        else
        {
            std::cout<<"Focus lost on now invalid FloatPropertyEditor"<<std::endl;
        }
    }
}

//[/MiscUserDefs]

//==============================================================================
FloatPropertyEditor::FloatPropertyEditor ()
{
    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("label text")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (slider = new Slider ("new slider"));
    slider->setRange (0, 10, 0);
    slider->setSliderStyle (Slider::LinearHorizontal);
    slider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider->setColour (Slider::thumbColourId, Colour (0xffaeaeae));
    slider->addListener (this);

    addAndMakeVisible (valueBox = new FloatPropertyTextEditor ("valueBox"));
    valueBox->setMultiLine (false);
    valueBox->setReturnKeyStartsNewLine (false);
    valueBox->setReadOnly (false);
    valueBox->setScrollbarsShown (true);
    valueBox->setCaretVisible (true);
    valueBox->setPopupMenuEnabled (true);
    valueBox->setText (String::empty);


    //[UserPreSize]
    disabled_=false;
    atom_=0;
    valueBox->setInputRestrictions(0,"-.0123456789");
    valueBox->addListener(this);
    //[/UserPreSize]

    setSize (400, 28);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

FloatPropertyEditor::~FloatPropertyEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;
    slider = nullptr;
    valueBox = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void FloatPropertyEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void FloatPropertyEditor::resized()
{
    label->setBounds (8, 8, proportionOfWidth (0.3725f), 16);
    slider->setBounds (getWidth() - 32 - 66, 8, 66, 16);
    valueBox->setBounds (222, 8, 72, 16);
    //[UserResized] Add your own custom resize handling here..
    label->setBounds (getX(), 8, proportionOfWidth (0.3725f), 16);
    //[/UserResized]
}

void FloatPropertyEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == slider)
    {
        //[UserSliderCode_slider] -- add your slider handling code here..
        atom_->set_value((float)slider->getValue());
        //[/UserSliderCode_slider]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void FloatPropertyEditor::refreshValue()
{
    float val=atom_->get_value().getFloatValue();
    valueBox->setText(getTextFromValue(val));
    slider->setValue(val,dontSendNotification);
}

void FloatPropertyEditor::setItem(String name, float value)
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
    valueBox->setText(getTextFromValue(value));
}

String FloatPropertyEditor::getTextFromValue(float value)
{
    String val=String(value,5).trimCharactersAtEnd("0");
    if(val.endsWith("."))
    {
        val=val+"0";
    }
    return val;
}

void FloatPropertyEditor::setRange(float min, float max)
{
    max_=max;
    min_=min;
    slider->setRange(min,max,0);
    float interval;
    int range=max-min;
    if(range>=1000)
    {
        //interval=1;
        interval=50;
    }
    else if(range>=100)
    {
        //interval=0.1;
        interval=5;
    }
    else if(range>=10)
    {
        //interval=0.1;
        interval=1;
    }
    else if(range>=1)
    {
        interval=0.1;
    }
    else
    {
        interval=0;
    }
    slider->setRange (min, max, interval);
    valueBox->setTooltip ("Valid range = "+ getTextFromValue(min_) + " to "+ getTextFromValue(max_));
    if(min_<0.0f)
    {
        valueBox->setInputRestrictions(0,"-.0123456789");
    }
    else
    {
        valueBox->setInputRestrictions(0,".0123456789");
    }
    slider->setTooltip ("Valid range = "+ getTextFromValue(min_) + " to "+ getTextFromValue(max_));
}

void FloatPropertyEditor::setDisabled(bool disabled)
{
    disabled_=disabled;
    slider->setEnabled(!disabled);
}

void FloatPropertyEditor::setAtom(Atom* atom)
{
    atom_=atom;
}

void FloatPropertyEditor::setIndent(int indent)
{
    label->setTopLeftPosition(indent,8);
}

void FloatPropertyEditor::textEditorTextChanged(TextEditor &editor)
{
//    std::cout<<"FloatPropertyEditor: textEditorTextChanged"<<std::endl;
    String r=String("0123456789");
    if(!(valueBox->getText().contains(String("."))))
    {
       r=r+".";
    }

    if((!(valueBox->getText().contains(String("-")))) && (min_<0.0f))
    {
       r=r+"-";
    }

    valueBox->setInputRestrictions(0,r);
}

void FloatPropertyEditor::textEditorReturnKeyPressed(TextEditor &editor)
{
//    std::cout<<"FlaotPropertyEditor: textEditorReturnKeyPressed"<<std::endl;
    String txt=valueBox->getText();
    set_value(txt);
}

void FloatPropertyEditor::set_value(String value)
{
    float val=value.getFloatValue();
    if(val>max_)
    {
        val=max_;
        valueBox->setText(getTextFromValue(max_),false);
    }
    else if(val<min_)
    {
        val=min_;
        valueBox->setText(getTextFromValue(min_),false);
    }

    slider->setValue(val,dontSendNotification);
    setAtomValue();
}

void FloatPropertyEditor::setAtomValue()
{
    float val=valueBox->getText().getFloatValue();
    atom_->set_value(val);
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="FloatPropertyEditor" componentName=""
                 parentClasses="public Component, public TextEditorListener, public AtomEditor"
                 constructorParams="" variableInitialisers="" snapPixels="2" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="400"
                 initialHeight="28">
  <BACKGROUND backgroundColour="ffffff"/>
  <LABEL name="new label" id="d295b9c350a86ef6" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="8 8 37.25% 16" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="label text" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <SLIDER name="new slider" id="2b8dd39d614c572b" memberName="slider" virtualName=""
          explicitFocusOrder="0" pos="32Rr 8 66 16" thumbcol="ffaeaeae"
          min="0" max="10" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <TEXTEDITOR name="valueBox" id="68348c48dcb5e6f0" memberName="valueBox" virtualName="FloatPropertyTextEditor"
              explicitFocusOrder="0" pos="222 8 72 16" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
