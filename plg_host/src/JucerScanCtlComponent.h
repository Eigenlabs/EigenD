/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  24 Sep 2012 6:42:56pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_JUCERSCANCTLCOMPONENT_JUCERSCANCTLCOMPONENT_DB6952F2__
#define __JUCER_HEADER_JUCERSCANCTLCOMPONENT_JUCERSCANCTLCOMPONENT_DB6952F2__

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
class JucerScanCtlComponent  : public Component,
                               public ButtonListener
{
public:
    //==============================================================================
    JucerScanCtlComponent ();
    ~JucerScanCtlComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    ToggleButton *full_scan_button() { return toggleButton; }
    TextButton *cancel_button() { return textButton; }
    TextButton *scan_button() { return textButton2; }
    TextButton *add_button() { return add_text_button; }
    TextButton *del_button() { return del_text_button; }
    Label *plugin_label() { return label2; }
    TableListBox *table() { return component; }
    ListBox *paths() { return path_box; }
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);

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
    TableListBox* component;
    Label* label;
    TextButton* textButton;
    TextButton* textButton2;
    ToggleButton* toggleButton;
    Label* label2;
    Label* label3;
    TextButton* add_text_button;
    TextButton* del_text_button;
    ListBox* path_box;
    Image cachedImage_backgroundBoxInner_png;
    Image cachedImage_backgroundBoxT_png;
    Image cachedImage_backgroundBoxL_png;
    Image cachedImage_backgroundBoxR_png;
    Image cachedImage_backgroundBoxB_png;
    Image cachedImage_backgroundBoxBr_png;
    Image cachedImage_backgroundBoxBl_png;
    Image cachedImage_eigenD_png;
    Image cachedImage_backgroundBoxTl_png;
    Image cachedImage_backgroundBoxTr_png;
    Image cachedImage_innerBoxTl_png;
    Image cachedImage_innerBoxTr_png;
    Image cachedImage_innerBoxBl_png;
    Image cachedImage_innerBoxBr_png;
    Image cachedImage_innerBoxB_png;
    Image cachedImage_innerBoxL_png;
    Image cachedImage_innerBoxR_png;
    Image cachedImage_innerBoxInner_png;
    Image cachedImage_innerBoxT_png;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucerScanCtlComponent);
};


#endif   // __JUCER_HEADER_JUCERSCANCTLCOMPONENT_JUCERSCANCTLCOMPONENT_DB6952F2__
