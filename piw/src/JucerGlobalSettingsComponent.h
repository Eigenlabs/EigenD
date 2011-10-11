/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  28 Jan 2011 3:50:17pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_GLOBALSETTINGSCOMPONENT_JUCERGLOBALSETTINGSCOMPONENT_B7EB751B__
#define __JUCER_HEADER_GLOBALSETTINGSCOMPONENT_JUCERGLOBALSETTINGSCOMPONENT_B7EB751B__

//[Headers]     -- You can add your own extra header files here --
#include <piw/piw_gui_mapper.h>

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
    void initialize(piw::mapping_delegate_t *mapping_delegate);
    void updateComponent();
    void updateComponent(piw::mapping_delegate_t *mapping_delegate);
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
    piw::mapping_delegate_t *mapping_delegate_;
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

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    GlobalSettingsComponent (const GlobalSettingsComponent&);
    const GlobalSettingsComponent& operator= (const GlobalSettingsComponent&);
};


#endif   // __JUCER_HEADER_GLOBALSETTINGSCOMPONENT_JUCERGLOBALSETTINGSCOMPONENT_B7EB751B__
