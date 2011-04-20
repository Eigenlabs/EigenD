/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  28 Jan 2011 3:52:06pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "StatusComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
StatusComponent::StatusComponent ()
    : cpu_meter (0)
{
    addAndMakeVisible (cpu_meter = new Slider (T("new slider")));
    cpu_meter->setRange (0, 100, 1);
    cpu_meter->setSliderStyle (Slider::LinearBar);
    cpu_meter->setTextBoxStyle (Slider::TextBoxLeft, true, 80, 20);
    cpu_meter->setColour (Slider::trackColourId, Colour (0xffffff));
    cpu_meter->setColour (Slider::textBoxOutlineColourId, Colour (0xb2000000));
    cpu_meter->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (200, 16);

    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

StatusComponent::~StatusComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (cpu_meter);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void StatusComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::black);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void StatusComponent::resized()
{
    cpu_meter->setBounds (0, 0, getWidth() - 0, getHeight() - 0);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void StatusComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == cpu_meter)
    {
        //[UserSliderCode_cpu_meter] -- add your slider handling code here..
        //[/UserSliderCode_cpu_meter]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="StatusComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="0" initialWidth="200" initialHeight="16">
  <BACKGROUND backgroundColour="ff000000"/>
  <SLIDER name="new slider" id="a194bab820fc1ad0" memberName="cpu_meter"
          virtualName="" explicitFocusOrder="0" pos="0 0 0M 0M" trackcol="ffffff"
          textboxoutline="b2000000" min="0" max="100" int="1" style="LinearBar"
          textBoxPos="TextBoxLeft" textBoxEditable="0" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
