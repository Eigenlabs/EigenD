/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_D79FC93E6051AA44__
#define __JUCE_HEADER_D79FC93E6051AA44__

//[Headers]     -- You can add your own extra header files here --
#include "juce.h"
#include "Conductor.h"
#include "ClipPoolList.h"
#include "ClipPoolFactory.h"
#include "ClipSelector.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ClipPoolWidget  : public Component,
                        public ClipSelector,
                        public ButtonListener
{
public:
    //==============================================================================
    ClipPoolWidget (ClipPoolBackend * backend);
    ~ClipPoolWidget();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void clipSelected(Clip*);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    ClipPoolBackend* backend_;
    Toolbar* toolbar_;
    ClipPoolFactory* tm_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<TextButton> addToArrangementButton;
    ScopedPointer<TextButton> addToSceneButton;
    ScopedPointer<TextButton> removeButton;
    ScopedPointer<Label> clipPoolLabel;
    ScopedPointer<ClipPoolList> clipPoolList;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipPoolWidget)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_D79FC93E6051AA44__
