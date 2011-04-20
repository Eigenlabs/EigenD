/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  23 Nov 2010 3:30:31pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_POPUPDIALOGCOMPONENT_JUCERPOPUPDIALOGCOMPONENT_BE3BE226__
#define __JUCER_HEADER_POPUPDIALOGCOMPONENT_JUCERPOPUPDIALOGCOMPONENT_BE3BE226__

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
class PopupDialogComponent  : public Component
{
public:
    //==============================================================================
    PopupDialogComponent ();
    ~PopupDialogComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void setContentComponent(Component* content);
    void setContentComponent(Component* content, int x_offset, int y_offset);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    Component* content_;
    int x_offset_;
    int y_offset_;
    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    PopupDialogComponent (const PopupDialogComponent&);
    const PopupDialogComponent& operator= (const PopupDialogComponent&);
};


#endif   // __JUCER_HEADER_POPUPDIALOGCOMPONENT_JUCERPOPUPDIALOGCOMPONENT_BE3BE226__
