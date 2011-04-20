/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  28 Jan 2011 3:49:45pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_CELLPOPUPCOMPONENT_JUCERCELLPOPUPCOMPONENT_74F99BDF__
#define __JUCER_HEADER_CELLPOPUPCOMPONENT_JUCERCELLPOPUPCOMPONENT_74F99BDF__

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
class CellPopupComponent  : public Component,
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
    void initialize(piw::mapper_cell_editor_t *cell_editor);
    void updateComponent();
    void updateComponent(piw::mapper_cell_editor_t *cell_editor);
    void setFocusOrder();
    void updateMapping();
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void buttonClicked (Button* buttonThatWasClicked);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    piw::mapper_cell_editor_t *cell_editor_;
    unsigned curve_;
    //[/UserVariables]

    //==============================================================================
    GroupComponent* bounds_group;
    Label* lo_label;
    Label* base_label;
    GroupComponent* scale_group;
    GroupComponent* control_scope_group;
    Slider* scale_factor;
    ToggleButton* mapping_enabled;
    ToggleButton* control_scope_global;
    ToggleButton* control_scope_pernote;
    TextButton* clear_mapping;
    GroupComponent* resolution_group;
    ToggleButton* resolution_7bit;
    ToggleButton* resolution_14bit;
    ComboBox* secondary_cc;
    Label* lo;
    Label* base;
    Label* hi_label;
    Label* hi;
    TextButton* ok;
    Slider* boundslo;
    Slider* boundsbase;
    Slider* boundshi;
    ToggleButton* scale_factor_invert;
    GroupComponent* data_decimation_group;
    Slider* data_decimation;
    ToggleButton* control_scope_channel;
    ComboBox* control_scope_channel_number;
    ToggleButton* bounds_origin_return;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    CellPopupComponent (const CellPopupComponent&);
    const CellPopupComponent& operator= (const CellPopupComponent&);
};


#endif   // __JUCER_HEADER_CELLPOPUPCOMPONENT_JUCERCELLPOPUPCOMPONENT_74F99BDF__
