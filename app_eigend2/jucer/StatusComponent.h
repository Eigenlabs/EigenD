/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 4.2.4

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_414CFFB9475E295A__
#define __JUCE_HEADER_414CFFB9475E295A__

//[Headers]     -- You can add your own extra header files here --
#include "juce.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class StatusComponent  : public Component,
                         public SliderListener
{
public:
    //==============================================================================
    StatusComponent ();
    ~StatusComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.

    void set_cpu(unsigned cpu)
    {
        Colour colour;

        colour = Colours::lightgreen;
        if(cpu > 30) colour=Colours::orange;
        if(cpu > 60) colour=Colours::red;

        cpu_meter->setColour (Slider::thumbColourId,colour);
        cpu_meter->setValue((float)cpu,dontSendNotification);
        getPeer()->performAnyPendingRepaintsNow();
    }

    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Slider> cpu_meter;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_414CFFB9475E295A__
