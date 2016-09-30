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

#ifndef __JUCE_HEADER_4AF8C7408AAAFAEC__
#define __JUCE_HEADER_4AF8C7408AAAFAEC__

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
class JucerScanPrgComponent  : public Component,
                               public SliderListener
{
public:
    //==============================================================================
    JucerScanPrgComponent ();
    ~JucerScanPrgComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    Label *getPluginLabel() { return plugin_label; }
    Label *getFormatLabel() { return format_label; }
    Slider *getProgressBar() { return slider; }
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

    // Binary resources:
    static const char* textBoxB_png;
    static const int textBoxB_pngSize;
    static const char* textBoxBl_png;
    static const int textBoxBl_pngSize;
    static const char* textBoxBr_png;
    static const int textBoxBr_pngSize;
    static const char* textBoxInner_png;
    static const int textBoxInner_pngSize;
    static const char* textBoxL_png;
    static const int textBoxL_pngSize;
    static const char* textBoxR_png;
    static const int textBoxR_pngSize;
    static const char* textBoxT_png;
    static const int textBoxT_pngSize;
    static const char* textBoxTl_png;
    static const int textBoxTl_pngSize;
    static const char* textBoxTr_png;
    static const int textBoxTr_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Slider> slider;
    ScopedPointer<Label> plugin_label;
    ScopedPointer<Label> format_label;
    Image cachedImage_textBoxTl_png_1;
    Image cachedImage_textBoxTr_png_2;
    Image cachedImage_textBoxBr_png_3;
    Image cachedImage_textBoxBl_png_4;
    Image cachedImage_textBoxL_png_5;
    Image cachedImage_textBoxR_png_6;
    Image cachedImage_textBoxB_png_7;
    Image cachedImage_textBoxT_png_8;
    Image cachedImage_textBoxInner_png_9;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucerScanPrgComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_4AF8C7408AAAFAEC__
