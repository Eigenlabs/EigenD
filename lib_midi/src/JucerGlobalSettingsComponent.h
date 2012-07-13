/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  13 Jul 2012 6:13:26pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_GLOBALSETTINGSCOMPONENT_JUCERGLOBALSETTINGSCOMPONENT_E1544CC5__
#define __JUCER_HEADER_GLOBALSETTINGSCOMPONENT_JUCERGLOBALSETTINGSCOMPONENT_E1544CC5__

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
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);



    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    DialogWindow* window_;
    midi::mapping_delegate_t *mapping_delegate_;
    //[/UserVariables]

    //==============================================================================
    GroupComponent* midi_group;
    TextButton* ok;
    ToggleButton* midi_notes;
    ToggleButton* midi_pitchbend;
    GroupComponent* min_data_decimation_group;
    Slider* data_decimation;
    Label* active_channel_label;
    ComboBox* active_channel;
    Label* min_channel_label;
    ComboBox* min_channel;
    Label* max_channel_label;
    ComboBox* max_channel;
    ToggleButton* midi_hires_velocity;
    Label* pitchbend_up_label;
    Label* pitchbend_down_label;
    Slider* pitchbend_up;
    Slider* pitchbend_down;


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    GlobalSettingsComponent (const GlobalSettingsComponent&);
    const GlobalSettingsComponent& operator= (const GlobalSettingsComponent&);
};


#endif   // __JUCER_HEADER_GLOBALSETTINGSCOMPONENT_JUCERGLOBALSETTINGSCOMPONENT_E1544CC5__
