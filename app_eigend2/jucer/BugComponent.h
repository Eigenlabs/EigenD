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

#ifndef __JUCE_HEADER_77EFB677E196AFFA__
#define __JUCE_HEADER_77EFB677E196AFFA__

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
class BugComponent  : public Component,
                      public ButtonListener
{
public:
    //==============================================================================
    BugComponent ();
    ~BugComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    TextEditor *name_editor() { return name; }
    TextEditor *email_editor() { return email; }
    TextEditor *subject_editor() { return subject; }
    TextEditor *description_editor() { return description; }
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;

    // Binary resources:
    static const char* backgroundBoxT_png;
    static const int backgroundBoxT_pngSize;
    static const char* backgroundBoxL_png;
    static const int backgroundBoxL_pngSize;
    static const char* backgroundBoxR_png;
    static const int backgroundBoxR_pngSize;
    static const char* backgroundBoxB_png;
    static const int backgroundBoxB_pngSize;
    static const char* backgroundBoxTl_png;
    static const int backgroundBoxTl_pngSize;
    static const char* backgroundBoxTr_png;
    static const int backgroundBoxTr_pngSize;
    static const char* backgroundBoxBl_png;
    static const int backgroundBoxBl_pngSize;
    static const char* backgroundBoxBr_png;
    static const int backgroundBoxBr_pngSize;
    static const char* backgroundBoxInner_png;
    static const int backgroundBoxInner_pngSize;
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
    static const char* eigenD_png;
    static const int eigenD_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<TextButton> textButton;
    ScopedPointer<TextEditor> name;
    ScopedPointer<TextEditor> email;
    ScopedPointer<TextEditor> subject;
    ScopedPointer<TextEditor> description;
    Image cachedImage_backgroundBoxInner_png_1;
    Image cachedImage_backgroundBoxT_png_2;
    Image cachedImage_backgroundBoxL_png_3;
    Image cachedImage_backgroundBoxR_png_4;
    Image cachedImage_backgroundBoxTl_png_5;
    Image cachedImage_backgroundBoxTr_png_6;
    Image cachedImage_backgroundBoxB_png_7;
    Image cachedImage_backgroundBoxBr_png_8;
    Image cachedImage_backgroundBoxBl_png_9;
    Image cachedImage_textBoxTl_png_10;
    Image cachedImage_textBoxTr_png_11;
    Image cachedImage_textBoxBr_png_12;
    Image cachedImage_textBoxBl_png_13;
    Image cachedImage_textBoxL_png_14;
    Image cachedImage_textBoxT_png_15;
    Image cachedImage_textBoxR_png_16;
    Image cachedImage_textBoxB_png_17;
    Image cachedImage_textBoxInner_png_18;
    Image cachedImage_textBoxTl_png_19;
    Image cachedImage_textBoxTr_png_20;
    Image cachedImage_textBoxBr_png_21;
    Image cachedImage_textBoxBl_png_22;
    Image cachedImage_textBoxL_png_23;
    Image cachedImage_textBoxT_png_24;
    Image cachedImage_textBoxR_png_25;
    Image cachedImage_textBoxB_png_26;
    Image cachedImage_textBoxInner_png_27;
    Image cachedImage_textBoxTl_png_28;
    Image cachedImage_textBoxTr_png_29;
    Image cachedImage_textBoxBr_png_30;
    Image cachedImage_textBoxBl_png_31;
    Image cachedImage_textBoxL_png_32;
    Image cachedImage_textBoxT_png_33;
    Image cachedImage_textBoxR_png_34;
    Image cachedImage_textBoxB_png_35;
    Image cachedImage_textBoxInner_png_36;
    Image cachedImage_textBoxTl_png_37;
    Image cachedImage_textBoxTr_png_38;
    Image cachedImage_textBoxBr_png_39;
    Image cachedImage_textBoxBl_png_40;
    Image cachedImage_textBoxL_png_41;
    Image cachedImage_textBoxT_png_42;
    Image cachedImage_textBoxR_png_43;
    Image cachedImage_textBoxB_png_44;
    Image cachedImage_textBoxInner_png_45;
    Image cachedImage_eigenD_png_46;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BugComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_77EFB677E196AFFA__
