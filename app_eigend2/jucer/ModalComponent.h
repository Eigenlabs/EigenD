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

#ifndef __JUCE_HEADER_2CFBF8303D5624F0__
#define __JUCE_HEADER_2CFBF8303D5624F0__

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
class ModalComponent  : public Component,
                        public ButtonListener
{
public:
    //==============================================================================
    ModalComponent ();
    ~ModalComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    Label *getTitle() { return title; }
    Label *getText() { return text; }
    void closeButtonPressed() { buttonClicked(ok_button); }
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
    static const char* eigenD_png;
    static const int eigenD_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> title;
    ScopedPointer<TextButton> ok_button;
    ScopedPointer<Label> text;
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


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModalComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_2CFBF8303D5624F0__
