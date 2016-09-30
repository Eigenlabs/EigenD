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

#ifndef __JUCE_HEADER_30E7AB2CC343A7A4__
#define __JUCE_HEADER_30E7AB2CC343A7A4__

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
class JucerScanCompleteComponent  : public Component,
                                    public ButtonListener
{
public:
    //==============================================================================
    JucerScanCompleteComponent ();
    ~JucerScanCompleteComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    Label *text() { return label; }
    ListBox *good_list_box() { return good_list; }
    ListBox *failed_list_box() { return failed_list; }
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;

    // Binary resources:
    static const char* backgroundBoxB_png;
    static const int backgroundBoxB_pngSize;
    static const char* backgroundBoxBl_png;
    static const int backgroundBoxBl_pngSize;
    static const char* backgroundBoxBr_png;
    static const int backgroundBoxBr_pngSize;
    static const char* backgroundBoxInner_png;
    static const int backgroundBoxInner_pngSize;
    static const char* backgroundBoxL_png;
    static const int backgroundBoxL_pngSize;
    static const char* backgroundBoxR_png;
    static const int backgroundBoxR_pngSize;
    static const char* backgroundBoxT_png;
    static const int backgroundBoxT_pngSize;
    static const char* backgroundBoxTl_png;
    static const int backgroundBoxTl_pngSize;
    static const char* backgroundBoxTr_png;
    static const int backgroundBoxTr_pngSize;
    static const char* innerBoxB_png;
    static const int innerBoxB_pngSize;
    static const char* innerBoxBl_png;
    static const int innerBoxBl_pngSize;
    static const char* innerBoxBr_png;
    static const int innerBoxBr_pngSize;
    static const char* innerBoxInner_png;
    static const int innerBoxInner_pngSize;
    static const char* innerBoxL_png;
    static const int innerBoxL_pngSize;
    static const char* innerBoxR_png;
    static const int innerBoxR_pngSize;
    static const char* innerBoxT_png;
    static const int innerBoxT_pngSize;
    static const char* innerBoxTl_png;
    static const int innerBoxTl_pngSize;
    static const char* innerBoxTr_png;
    static const int innerBoxTr_pngSize;
    static const char* eigenD_png;
    static const int eigenD_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> label;
    ScopedPointer<TextButton> textButton;
    ScopedPointer<Label> label2;
    ScopedPointer<Label> label3;
    ScopedPointer<ListBox> failed_list;
    ScopedPointer<ListBox> good_list;
    Image cachedImage_backgroundBoxInner_png_1;
    Image cachedImage_backgroundBoxT_png_2;
    Image cachedImage_backgroundBoxL_png_3;
    Image cachedImage_backgroundBoxR_png_4;
    Image cachedImage_backgroundBoxTl_png_5;
    Image cachedImage_backgroundBoxTr_png_6;
    Image cachedImage_backgroundBoxB_png_7;
    Image cachedImage_backgroundBoxBr_png_8;
    Image cachedImage_backgroundBoxBl_png_9;
    Image cachedImage_eigenD_png_10;
    Image cachedImage_innerBoxBr_png_11;
    Image cachedImage_innerBoxInner_png_12;
    Image cachedImage_innerBoxTl_png_13;
    Image cachedImage_innerBoxTr_png_14;
    Image cachedImage_innerBoxBl_png_15;
    Image cachedImage_innerBoxT_png_16;
    Image cachedImage_innerBoxB_png_17;
    Image cachedImage_innerBoxL_png_18;
    Image cachedImage_innerBoxR_png_19;
    Image cachedImage_innerBoxBr_png_20;
    Image cachedImage_innerBoxR_png_21;
    Image cachedImage_innerBoxTl_png_22;
    Image cachedImage_innerBoxTr_png_23;
    Image cachedImage_innerBoxBl_png_24;
    Image cachedImage_innerBoxT_png_25;
    Image cachedImage_innerBoxB_png_26;
    Image cachedImage_innerBoxL_png_27;
    Image cachedImage_innerBoxInner_png_28;
    Image cachedImage_innerBoxBr_png_29;
    Image cachedImage_innerBoxInner_png_30;
    Image cachedImage_innerBoxTl_png_31;
    Image cachedImage_innerBoxTr_png_32;
    Image cachedImage_innerBoxBl_png_33;
    Image cachedImage_innerBoxT_png_34;
    Image cachedImage_innerBoxB_png_35;
    Image cachedImage_innerBoxL_png_36;
    Image cachedImage_innerBoxR_png_37;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucerScanCompleteComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_30E7AB2CC343A7A4__
