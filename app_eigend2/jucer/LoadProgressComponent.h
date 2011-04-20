/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  28 Jan 2011 3:52:00pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_LOADPROGRESSCOMPONENT_LOADPROGRESSCOMPONENT_CA1A4964__
#define __JUCER_HEADER_LOADPROGRESSCOMPONENT_LOADPROGRESSCOMPONENT_CA1A4964__

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
class LoadProgressComponent  : public Component,
                               public SliderListener
{
public:
    //==============================================================================
    LoadProgressComponent ();
    ~LoadProgressComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    Slider *getProgressSlider() { return progress_slider; }
    Label *getMessageLabel() { return message_label; }
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);

    // Binary resources:
    static const char* textBoxTl_png;
    static const int textBoxTl_pngSize;
    static const char* textBoxTr_png;
    static const int textBoxTr_pngSize;
    static const char* textBoxBr_png;
    static const int textBoxBr_pngSize;
    static const char* textBoxBl_png;
    static const int textBoxBl_pngSize;
    static const char* textBoxL_png;
    static const int textBoxL_pngSize;
    static const char* textBoxR_png;
    static const int textBoxR_pngSize;
    static const char* textBoxB_png;
    static const int textBoxB_pngSize;
    static const char* textBoxT_png;
    static const int textBoxT_pngSize;
    static const char* textBoxInner_png;
    static const int textBoxInner_pngSize;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    Slider* progress_slider;
    Label* message_label;
    Image cachedImage_textBoxTl_png;
    Image cachedImage_textBoxTr_png;
    Image cachedImage_textBoxBr_png;
    Image cachedImage_textBoxBl_png;
    Image cachedImage_textBoxL_png;
    Image cachedImage_textBoxR_png;
    Image cachedImage_textBoxB_png;
    Image cachedImage_textBoxT_png;
    Image cachedImage_textBoxInner_png;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    LoadProgressComponent (const LoadProgressComponent&);
    const LoadProgressComponent& operator= (const LoadProgressComponent&);
};


#endif   // __JUCER_HEADER_LOADPROGRESSCOMPONENT_LOADPROGRESSCOMPONENT_CA1A4964__
