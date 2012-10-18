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

#ifndef __JUCER_HEADER_STATUSCOMPONENT_STATUSCOMPONENT_D1DC6B22__
#define __JUCER_HEADER_STATUSCOMPONENT_STATUSCOMPONENT_D1DC6B22__

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

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    Slider* cpu_meter;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    StatusComponent (const StatusComponent&);
    const StatusComponent& operator= (const StatusComponent&);
};


#endif   // __JUCER_HEADER_STATUSCOMPONENT_STATUSCOMPONENT_D1DC6B22__
