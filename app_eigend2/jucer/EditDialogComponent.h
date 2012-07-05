/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  5 Jul 2012 5:18:59pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_EDITDIALOGCOMPONENT_EDITDIALOGCOMPONENT_5759BEAC__
#define __JUCER_HEADER_EDITDIALOGCOMPONENT_EDITDIALOGCOMPONENT_5759BEAC__

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
class EditDialogComponent  : public Component,
                             public ButtonListener
{
public:
    //==============================================================================
    EditDialogComponent ();
    ~EditDialogComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    Label *getSlot() { return slot; }
    TextEditor *getDescription() { return description; }
    TextEditor *getSummary() { return summary; }
    TextButton *getButton() { return save_button; }
    TextButton *helpButton() { return help_button; }
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);

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
    static const char* logoText_png;
    static const int logoText_pngSize;
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
    static const char* textBoxT_png;
    static const int textBoxT_pngSize;
    static const char* textBoxR_png;
    static const int textBoxR_pngSize;
    static const char* textBoxB_png;
    static const int textBoxB_pngSize;
    static const char* textBoxInner_png;
    static const int textBoxInner_pngSize;
    static const char* eigenD_png;
    static const int eigenD_pngSize;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    TextEditor* description;
    TextButton* save_button;
    TextEditor* summary;
    Label* label;
    Label* label2;
    TextButton* help_button;
    Label* slot;
    Image cachedImage_backgroundBoxInner_png;
    Image cachedImage_backgroundBoxT_png;
    Image cachedImage_backgroundBoxL_png;
    Image cachedImage_backgroundBoxR_png;
    Image cachedImage_backgroundBoxTl_png;
    Image cachedImage_backgroundBoxTr_png;
    Image cachedImage_backgroundBoxB_png;
    Image cachedImage_backgroundBoxBr_png;
    Image cachedImage_backgroundBoxBl_png;
    Image cachedImage_textBoxTl_png;
    Image cachedImage_textBoxTr_png;
    Image cachedImage_textBoxBr_png;
    Image cachedImage_textBoxBl_png;
    Image cachedImage_textBoxL_png;
    Image cachedImage_textBoxT_png;
    Image cachedImage_textBoxR_png;
    Image cachedImage_textBoxB_png;
    Image cachedImage_textBoxInner_png;
    Image cachedImage_eigenD_png;


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    EditDialogComponent (const EditDialogComponent&);
    const EditDialogComponent& operator= (const EditDialogComponent&);
};


#endif   // __JUCER_HEADER_EDITDIALOGCOMPONENT_EDITDIALOGCOMPONENT_5759BEAC__
