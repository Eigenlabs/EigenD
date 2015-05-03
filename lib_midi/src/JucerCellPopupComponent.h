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

#ifndef __JUCE_HEADER_7F7EC30B14DA7E63__
#define __JUCE_HEADER_7F7EC30B14DA7E63__

//[Headers]     -- You can add your own extra header files here --
#include <lib_midi/control_mapper_gui.h>

#include "juce.h"

#include "SliderEx.h"

//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class CellPopupComponent  : public Component,
                            public MessageListener,
                            public SliderListener,
                            public ButtonListener,
                            public ComboBoxListener
{
public:
    //==============================================================================
    CellPopupComponent ();
    ~CellPopupComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void initialize(midi::mapper_cell_editor_t *cell_editor);
    void updateComponent();
    void updateComponent(midi::mapper_cell_editor_t *cell_editor);
    void setFocusOrder();
    void updateMapping();
    void handleMessage(const Message &);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void buttonClicked (Button* buttonThatWasClicked);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    midi::mapper_cell_editor_t *cell_editor_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<GroupComponent> bounds_group;
    ScopedPointer<Label> lo_label;
    ScopedPointer<Label> base_label;
    ScopedPointer<GroupComponent> scale_group;
    ScopedPointer<GroupComponent> control_scope_group;
    ScopedPointer<Slider> scale_factor;
    ScopedPointer<ToggleButton> mapping_enabled;
    ScopedPointer<ToggleButton> control_scope_global;
    ScopedPointer<ToggleButton> control_scope_pernote;
    ScopedPointer<TextButton> clear_mapping;
    ScopedPointer<GroupComponent> resolution_group;
    ScopedPointer<ToggleButton> resolution_7bit;
    ScopedPointer<ToggleButton> resolution_14bit;
    ScopedPointer<ComboBox> secondary_cc;
    ScopedPointer<Label> lo;
    ScopedPointer<Label> base;
    ScopedPointer<Label> hi_label;
    ScopedPointer<Label> hi;
    ScopedPointer<TextButton> ok;
    ScopedPointer<SliderEx> boundslo;
    ScopedPointer<SliderEx> boundsbase;
    ScopedPointer<SliderEx> boundshi;
    ScopedPointer<ToggleButton> scale_factor_invert;
    ScopedPointer<GroupComponent> data_decimation_group;
    ScopedPointer<Slider> data_decimation;
    ScopedPointer<ToggleButton> control_scope_channel;
    ScopedPointer<ComboBox> control_scope_channel_number;
    ScopedPointer<ToggleButton> bounds_origin_return;
    ScopedPointer<GroupComponent> curve_group;
    ScopedPointer<ComboBox> curve;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CellPopupComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_7F7EC30B14DA7E63__
