/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.1

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_152173484D4E5DB7__
#define __JUCE_HEADER_152173484D4E5DB7__

//[Headers]     -- You can add your own extra header files here --
#include <lib_midi/control_mapper_gui.h>

#include "juce.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class GlobalSettingsComponent  : public Component,
                                 public MessageListener,
                                 public ButtonListener,
                                 public SliderListener,
                                 public ComboBoxListener
{
public:
    //==============================================================================
    GlobalSettingsComponent ();
    ~GlobalSettingsComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void initialize(midi::mapping_delegate_t *mapping_delegate);
    void updateComponent();
    void updateComponent(midi::mapping_delegate_t *mapping_delegate);
    void setDialogWindow(DialogWindow *window);
    void setFocusOrder();
    void updateSettings();
    void handleMessage(const Message &);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    DialogWindow* window_;
    midi::mapping_delegate_t *mapping_delegate_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<GroupComponent> midi_group;
    ScopedPointer<TextButton> ok;
    ScopedPointer<ToggleButton> midi_notes;
    ScopedPointer<ToggleButton> midi_pitchbend;
    ScopedPointer<GroupComponent> min_data_decimation_group;
    ScopedPointer<Slider> data_decimation;
    ScopedPointer<Label> active_channel_label;
    ScopedPointer<ComboBox> active_channel;
    ScopedPointer<Label> min_channel_label;
    ScopedPointer<ComboBox> min_channel;
    ScopedPointer<Label> max_channel_label;
    ScopedPointer<ComboBox> max_channel;
    ScopedPointer<ToggleButton> midi_hires_velocity;
    ScopedPointer<Label> pitchbend_up_label;
    ScopedPointer<Label> pitchbend_down_label;
    ScopedPointer<Slider> pitchbend_up;
    ScopedPointer<Slider> pitchbend_down;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalSettingsComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_152173484D4E5DB7__
