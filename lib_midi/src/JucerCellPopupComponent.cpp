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

//[Headers] You can add your own extra header files here...
#include <lib_midi/control_mapping.h>
#include <lib_midi/midi_gm.h>
//[/Headers]

#include "JucerCellPopupComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
void CellPopupComponent::initialize(midi::mapper_cell_editor_t *cell_editor)
{
    cell_editor_ = 0;

    updateComponent(cell_editor);

    int height = 480;

    if(!cell_editor->edit_control_scope())
    {
        control_scope_group->setVisible(false);
        control_scope_global->setVisible(false);
        control_scope_pernote->setVisible(false);
        control_scope_channel->setVisible(false);
        control_scope_channel_number->setVisible(false);
        removeChildComponent(control_scope_group);
        removeChildComponent(control_scope_global);
        removeChildComponent(control_scope_pernote);
        removeChildComponent(control_scope_channel);
        removeChildComponent(control_scope_channel_number);
        height -= 90;
    }
    else if(!cell_editor->edit_fixed_channel())
    {
        control_scope_channel->setVisible(false);
        control_scope_channel_number->setVisible(false);
        removeChildComponent(control_scope_channel);
        removeChildComponent(control_scope_channel_number);
        height -= 32;
    }

    if(!cell_editor->edit_resolution())
    {
        resolution_group->setVisible(false);
        resolution_7bit->setVisible(false);
        resolution_14bit->setVisible(false);
        secondary_cc->setVisible(false);
        removeChildComponent(resolution_group);
        removeChildComponent(resolution_7bit);
        removeChildComponent(resolution_14bit);
        removeChildComponent(secondary_cc);
        height -= 100;
    }

    setSize(208, height);

    scale_factor->setRange(0, 10, 0.1);

    cell_editor_ = cell_editor;
}

void CellPopupComponent::updateComponent()
{
    updateComponent(cell_editor_);
}

void CellPopupComponent::updateComponent(midi::mapper_cell_editor_t *cell_editor)
{
    if(!cell_editor->is_mapped())
    {
        cell_editor->close_popup();
        return;
    }

    midi::mapping_info_t info = cell_editor->get_info();
    if(info.scale_<0)
    {
        scale_factor_invert->setToggleState(true,false);
        scale_factor->setValue(info.scale_*-1.0, true, true);
    }
    else
    {
        scale_factor_invert->setToggleState(false,false);
        scale_factor->setValue(info.scale_, true, true);
    }

    boundshi->setValue(info.hi_*100, true, true);
    boundsbase->setValue(info.base_*100, true);
    boundslo->setValue(info.lo_*100, true, true);

    bounds_origin_return->setToggleState(info.origin_return_, false);

    if(cell_editor->get_settings().minimum_decimation_ > info.decimation_)
    {
        data_decimation->setColour (Slider::textBoxTextColourId, Colour (0xffff3a3a));
    }
    else
    {
        data_decimation->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    }
    data_decimation->setValue(info.decimation_, true, true);

    mapping_enabled->setToggleState(info.enabled_, false);

    if(cell_editor->edit_control_scope())
    {
        switch(info.scope_)
        {
            case GLOBAL_SCOPE:
                control_scope_global->setToggleState(true,false);
                control_scope_pernote->setToggleState(false,false);
                control_scope_channel->setToggleState(false,false);
                control_scope_channel_number->setEnabled(false);
                break;
            case PERNOTE_SCOPE:
                control_scope_global->setToggleState(false,false);
                control_scope_pernote->setToggleState(true,false);
                control_scope_channel->setToggleState(false,false);
                control_scope_channel_number->setEnabled(false);
                break;
            case CHANNEL_SCOPE:
                control_scope_global->setToggleState(false,false);
                control_scope_pernote->setToggleState(true,false);
                control_scope_channel->setToggleState(true,false);
                control_scope_channel_number->setSelectedId(info.channel_);
                control_scope_channel_number->setEnabled(true);
                break;
        }

        switch(info.resolution_)
        {
            case BITS_7:
                resolution_7bit->setToggleState(true,false);
                resolution_14bit->setToggleState(false,false);
                secondary_cc->setEnabled(false);
                break;
            case BITS_14:
                resolution_7bit->setToggleState(false,false);
                resolution_14bit->setToggleState(true,false);
                secondary_cc->setEnabled(true);
                break;
        }

        int oparam = info.oparam_;

        int secondary_cc_ = info.secondary_cc_;
        if(secondary_cc_<0)
        {
            // default 14 bit mapping for first 32 CCs
            if(oparam<32)
            {
                secondary_cc_= oparam+32;
            }
            // two other standard LSBs
            else if(oparam==98)
            {
                secondary_cc_ = 99;
            }
            else if(oparam==100)
            {
                secondary_cc_ = 101;
            }
        }
        if(secondary_cc_>=0 && secondary_cc_<MIDI_CC_MAX)
        {
            secondary_cc->setSelectedId(secondary_cc_);
        }
    }

    curve_ = info.curve_;
}

void CellPopupComponent::setFocusOrder()
{
    scale_factor->setWantsKeyboardFocus(false);
    control_scope_global->setWantsKeyboardFocus(false);
    control_scope_pernote->setWantsKeyboardFocus(false);
    control_scope_channel->setWantsKeyboardFocus(false);
    control_scope_channel_number->setWantsKeyboardFocus(false);
    mapping_enabled->setWantsKeyboardFocus(false);
    clear_mapping->setWantsKeyboardFocus(false);
    ok->setWantsKeyboardFocus(false);
}

void CellPopupComponent::updateMapping()
{
    if(!cell_editor_) return;

    unsigned scope = GLOBAL_SCOPE;
    unsigned channel = 0;
    unsigned resolution = BITS_7;

    if(cell_editor_->edit_control_scope())
    {
        if(control_scope_global->getToggleState())
        {
            scope = GLOBAL_SCOPE;
        }
        else if(control_scope_pernote->getToggleState())
        {
            scope = PERNOTE_SCOPE;
        }
        else if(control_scope_channel->getToggleState())
        {
            scope = CHANNEL_SCOPE;
            channel = control_scope_channel_number->getSelectedId();
        }

        if(resolution_7bit->getToggleState())
        {
            resolution = BITS_7;
        }
        else if(resolution_14bit->getToggleState())
        {
            resolution = BITS_14;
        }
    }

    double scale=scale_factor->getValue();
    if(scale_factor_invert->getToggleState())
    {
        scale *= -1.0;
    }

    cell_editor_->map(mapping_enabled->getToggleState(), scale,
        boundslo->getValue()/100.f, boundsbase->getValue()/100.f, boundshi->getValue()/100.f,
        bounds_origin_return->getToggleState(), data_decimation->getValue(), scope, channel,
        resolution, secondary_cc->getSelectedId(), curve_);
}
//[/MiscUserDefs]

//==============================================================================
CellPopupComponent::CellPopupComponent ()
    : Component ("CellPopup"),
      bounds_group (0),
      lo_label (0),
      base_label (0),
      scale_group (0),
      control_scope_group (0),
      scale_factor (0),
      mapping_enabled (0),
      control_scope_global (0),
      control_scope_pernote (0),
      clear_mapping (0),
      resolution_group (0),
      resolution_7bit (0),
      resolution_14bit (0),
      secondary_cc (0),
      lo (0),
      base (0),
      hi_label (0),
      hi (0),
      ok (0),
      boundslo (0),
      boundsbase (0),
      boundshi (0),
      scale_factor_invert (0),
      data_decimation_group (0),
      data_decimation (0),
      control_scope_channel (0),
      control_scope_channel_number (0),
      bounds_origin_return (0)
{
    addAndMakeVisible (bounds_group = new GroupComponent ("bounds group",
                                                          "Bounds (in %)"));
    bounds_group->setColour (GroupComponent::outlineColourId, Colour (0x66eeeeee));
    bounds_group->setColour (GroupComponent::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (lo_label = new Label ("lo label",
                                             "Lo:"));
    lo_label->setFont (Font (15.0000f, Font::plain));
    lo_label->setJustificationType (Justification::centredLeft);
    lo_label->setEditable (false, false, false);
    lo_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    lo_label->setColour (TextEditor::textColourId, Colours::black);
    lo_label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (base_label = new Label ("base label",
                                               "Base:"));
    base_label->setFont (Font (15.0000f, Font::plain));
    base_label->setJustificationType (Justification::centredLeft);
    base_label->setEditable (false, false, false);
    base_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    base_label->setColour (TextEditor::textColourId, Colours::black);
    base_label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (scale_group = new GroupComponent ("scale group",
                                                         "Scale factor"));
    scale_group->setColour (GroupComponent::outlineColourId, Colour (0x66eeeeee));
    scale_group->setColour (GroupComponent::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (control_scope_group = new GroupComponent ("control scope group",
                                                                 "Control scope"));
    control_scope_group->setColour (GroupComponent::outlineColourId, Colour (0x66eeeeee));
    control_scope_group->setColour (GroupComponent::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (scale_factor = new Slider ("scale slider"));
    scale_factor->setRange (0, 10, 0.1);
    scale_factor->setSliderStyle (Slider::LinearBar);
    scale_factor->setTextBoxStyle (Slider::TextBoxLeft, true, 45, 20);
    scale_factor->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    scale_factor->setColour (Slider::rotarySliderFillColourId, Colours::azure);
    scale_factor->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    scale_factor->setColour (Slider::textBoxBackgroundColourId, Colour (0xffffff));
    scale_factor->setColour (Slider::textBoxHighlightColourId, Colour (0x40000000));
    scale_factor->setColour (Slider::textBoxOutlineColourId, Colour (0x66ffffff));
    scale_factor->addListener (this);

    addAndMakeVisible (mapping_enabled = new ToggleButton ("enabled toggle button"));
    mapping_enabled->setButtonText ("Enabled");
    mapping_enabled->addListener (this);
    mapping_enabled->setToggleState (true, false);
    mapping_enabled->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (control_scope_global = new ToggleButton ("global scope toggle button"));
    control_scope_global->setButtonText ("Global");
    control_scope_global->setRadioGroupId (1);
    control_scope_global->addListener (this);
    control_scope_global->setToggleState (true, false);
    control_scope_global->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (control_scope_pernote = new ToggleButton ("pernote scope toggle button"));
    control_scope_pernote->setButtonText ("Per-note");
    control_scope_pernote->setRadioGroupId (1);
    control_scope_pernote->addListener (this);
    control_scope_pernote->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (clear_mapping = new TextButton ("clear mapping button"));
    clear_mapping->setButtonText ("Clear");
    clear_mapping->addListener (this);
    clear_mapping->setColour (TextButton::buttonColourId, Colour (0xffc1c1c1));

    addAndMakeVisible (resolution_group = new GroupComponent ("resolution group",
                                                              "Resolution"));
    resolution_group->setColour (GroupComponent::outlineColourId, Colour (0x66eeeeee));
    resolution_group->setColour (GroupComponent::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (resolution_7bit = new ToggleButton ("7bit resolution toggle button"));
    resolution_7bit->setButtonText ("7 bit");
    resolution_7bit->setRadioGroupId (2);
    resolution_7bit->addListener (this);
    resolution_7bit->setToggleState (true, false);
    resolution_7bit->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (resolution_14bit = new ToggleButton ("14 bit resolution toggle button"));
    resolution_14bit->setButtonText ("14 bit");
    resolution_14bit->setRadioGroupId (2);
    resolution_14bit->addListener (this);
    resolution_14bit->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (secondary_cc = new ComboBox ("secondary cc combo box"));
    secondary_cc->setEditableText (false);
    secondary_cc->setJustificationType (Justification::centredLeft);
    secondary_cc->setTextWhenNothingSelected (String::empty);
    secondary_cc->setTextWhenNoChoicesAvailable ("(no choices)");
    secondary_cc->addListener (this);

    addAndMakeVisible (lo = new Label ("lo",
                                       "0"));
    lo->setFont (Font (15.0000f, Font::plain));
    lo->setJustificationType (Justification::centredRight);
    lo->setEditable (false, false, false);
    lo->setColour (Label::textColourId, Colour (0xffeeeeee));
    lo->setColour (TextEditor::textColourId, Colours::black);
    lo->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (base = new Label ("base",
                                         "0"));
    base->setFont (Font (15.0000f, Font::plain));
    base->setJustificationType (Justification::centredRight);
    base->setEditable (false, false, false);
    base->setColour (Label::textColourId, Colour (0xffeeeeee));
    base->setColour (TextEditor::textColourId, Colours::black);
    base->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (hi_label = new Label ("hi label",
                                             "Hi:"));
    hi_label->setFont (Font (15.0000f, Font::plain));
    hi_label->setJustificationType (Justification::centredLeft);
    hi_label->setEditable (false, false, false);
    hi_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    hi_label->setColour (TextEditor::textColourId, Colours::black);
    hi_label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (hi = new Label ("hi",
                                       "100"));
    hi->setFont (Font (15.0000f, Font::plain));
    hi->setJustificationType (Justification::centredRight);
    hi->setEditable (false, false, false);
    hi->setColour (Label::textColourId, Colour (0xffeeeeee));
    hi->setColour (TextEditor::textColourId, Colours::black);
    hi->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (ok = new TextButton ("ok button"));
    ok->setButtonText ("Ok");
    ok->addListener (this);
    ok->setColour (TextButton::buttonColourId, Colour (0xffc1c1c1));

    addAndMakeVisible (boundslo = new Slider ("boundslo dial"));
    boundslo->setRange (0, 100, 1);
    boundslo->setSliderStyle (Slider::Rotary);
    boundslo->setTextBoxStyle (Slider::NoTextBox, true, 45, 14);
    boundslo->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    boundslo->setColour (Slider::trackColourId, Colour (0x7fffffff));
    boundslo->setColour (Slider::rotarySliderFillColourId, Colour (0xddffffff));
    boundslo->setColour (Slider::rotarySliderOutlineColourId, Colour (0x66ffffff));
    boundslo->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    boundslo->setColour (Slider::textBoxBackgroundColourId, Colour (0xffffff));
    boundslo->setColour (Slider::textBoxHighlightColourId, Colour (0x40000000));
    boundslo->setColour (Slider::textBoxOutlineColourId, Colour (0x0));
    boundslo->addListener (this);

    addAndMakeVisible (boundsbase = new Slider ("boundsbase dial"));
    boundsbase->setRange (0, 100, 1);
    boundsbase->setSliderStyle (Slider::Rotary);
    boundsbase->setTextBoxStyle (Slider::NoTextBox, true, 45, 14);
    boundsbase->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    boundsbase->setColour (Slider::rotarySliderFillColourId, Colour (0xddffffff));
    boundsbase->setColour (Slider::rotarySliderOutlineColourId, Colour (0x66ffffff));
    boundsbase->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    boundsbase->setColour (Slider::textBoxBackgroundColourId, Colour (0xffffff));
    boundsbase->setColour (Slider::textBoxHighlightColourId, Colour (0x40000000));
    boundsbase->setColour (Slider::textBoxOutlineColourId, Colour (0x0));
    boundsbase->addListener (this);

    addAndMakeVisible (boundshi = new Slider ("boundshi dial"));
    boundshi->setRange (0, 100, 1);
    boundshi->setSliderStyle (Slider::Rotary);
    boundshi->setTextBoxStyle (Slider::NoTextBox, true, 45, 14);
    boundshi->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    boundshi->setColour (Slider::rotarySliderFillColourId, Colour (0xddffffff));
    boundshi->setColour (Slider::rotarySliderOutlineColourId, Colour (0x66ffffff));
    boundshi->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    boundshi->setColour (Slider::textBoxBackgroundColourId, Colour (0xffffff));
    boundshi->setColour (Slider::textBoxHighlightColourId, Colour (0x40000000));
    boundshi->setColour (Slider::textBoxOutlineColourId, Colour (0x0));
    boundshi->addListener (this);

    addAndMakeVisible (scale_factor_invert = new ToggleButton ("scale factor invert toggle button"));
    scale_factor_invert->setButtonText ("Invert");
    scale_factor_invert->addListener (this);
    scale_factor_invert->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (data_decimation_group = new GroupComponent ("data decimation group",
                                                                   "Data decimation (in ms)"));
    data_decimation_group->setColour (GroupComponent::outlineColourId, Colour (0x66eeeeee));
    data_decimation_group->setColour (GroupComponent::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (data_decimation = new Slider ("data decimation slider"));
    data_decimation->setRange (0, 100, 1);
    data_decimation->setSliderStyle (Slider::LinearBar);
    data_decimation->setTextBoxStyle (Slider::TextBoxLeft, true, 45, 20);
    data_decimation->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    data_decimation->setColour (Slider::rotarySliderFillColourId, Colours::azure);
    data_decimation->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    data_decimation->setColour (Slider::textBoxBackgroundColourId, Colour (0xffffff));
    data_decimation->setColour (Slider::textBoxHighlightColourId, Colour (0x40000000));
    data_decimation->setColour (Slider::textBoxOutlineColourId, Colour (0x66ffffff));
    data_decimation->addListener (this);

    addAndMakeVisible (control_scope_channel = new ToggleButton ("channel scope toggle button"));
    control_scope_channel->setButtonText ("Fixed channel");
    control_scope_channel->setRadioGroupId (1);
    control_scope_channel->addListener (this);
    control_scope_channel->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (control_scope_channel_number = new ComboBox ("fixed channel combo box"));
    control_scope_channel_number->setEditableText (false);
    control_scope_channel_number->setJustificationType (Justification::centredRight);
    control_scope_channel_number->setTextWhenNothingSelected (String::empty);
    control_scope_channel_number->setTextWhenNoChoicesAvailable ("(no choices)");
    control_scope_channel_number->addItem ("1", 1);
    control_scope_channel_number->addItem ("2", 2);
    control_scope_channel_number->addItem ("3", 3);
    control_scope_channel_number->addItem ("4", 4);
    control_scope_channel_number->addItem ("5", 5);
    control_scope_channel_number->addItem ("6", 6);
    control_scope_channel_number->addItem ("7", 7);
    control_scope_channel_number->addItem ("8", 8);
    control_scope_channel_number->addItem ("9", 9);
    control_scope_channel_number->addItem ("10", 10);
    control_scope_channel_number->addItem ("11", 11);
    control_scope_channel_number->addItem ("12", 12);
    control_scope_channel_number->addItem ("13", 13);
    control_scope_channel_number->addItem ("14", 14);
    control_scope_channel_number->addItem ("15", 15);
    control_scope_channel_number->addItem ("16", 16);
    control_scope_channel_number->addListener (this);

    addAndMakeVisible (bounds_origin_return = new ToggleButton ("bounds return to origin"));
    bounds_origin_return->setButtonText ("Always return to origin");
    bounds_origin_return->addListener (this);
    bounds_origin_return->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));


    //[UserPreSize]
    cell_editor_ = 0;

    scale_factor->setScrollWheelEnabled(true);

    boundslo->setScrollWheelEnabled(true);

    boundsbase->setScrollWheelEnabled(true);

    boundshi->setScrollWheelEnabled(true);

    for(int cc=0; cc<MIDI_CC_MAX; cc++)
    {
        std::stringstream oss;
        oss << "CC# ";
        oss << cc;
        oss << " : ";
        oss << midi::midi_cc[cc];
        secondary_cc->addItem(juce::String(oss.str().c_str()), cc);
        secondary_cc->setEnabled(false);
    }
    secondary_cc->setEnabled(false);
    control_scope_channel_number->setEnabled(false);
    //[/UserPreSize]

    setSize (208, 480);

    //[Constructor] You can add your own custom stuff here..
    curve_ = CURVE_LINEAR;
    //[/Constructor]
}

CellPopupComponent::~CellPopupComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (bounds_group);
    deleteAndZero (lo_label);
    deleteAndZero (base_label);
    deleteAndZero (scale_group);
    deleteAndZero (control_scope_group);
    deleteAndZero (scale_factor);
    deleteAndZero (mapping_enabled);
    deleteAndZero (control_scope_global);
    deleteAndZero (control_scope_pernote);
    deleteAndZero (clear_mapping);
    deleteAndZero (resolution_group);
    deleteAndZero (resolution_7bit);
    deleteAndZero (resolution_14bit);
    deleteAndZero (secondary_cc);
    deleteAndZero (lo);
    deleteAndZero (base);
    deleteAndZero (hi_label);
    deleteAndZero (hi);
    deleteAndZero (ok);
    deleteAndZero (boundslo);
    deleteAndZero (boundsbase);
    deleteAndZero (boundshi);
    deleteAndZero (scale_factor_invert);
    deleteAndZero (data_decimation_group);
    deleteAndZero (data_decimation);
    deleteAndZero (control_scope_channel);
    deleteAndZero (control_scope_channel_number);
    deleteAndZero (bounds_origin_return);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void CellPopupComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void CellPopupComponent::resized()
{
    bounds_group->setBounds (8, 72, 192, 112);
    lo_label->setBounds (16, 125, 30, 24);
    base_label->setBounds (74, 125, 40, 24);
    scale_group->setBounds (8, 8, 192, 56);
    control_scope_group->setBounds (8, 254, 192, 82);
    scale_factor->setBounds (21, 30, 107, 24);
    mapping_enabled->setBounds (64, getHeight() - 24, 72, 24);
    control_scope_global->setBounds (16, 272, 72, 24);
    control_scope_pernote->setBounds (104, 272, 80, 24);
    clear_mapping->setBounds (8, getHeight() - 24, 48, 24);
    resolution_group->setBounds (8, 342, 192, 96);
    resolution_7bit->setBounds (24, 366, 72, 24);
    resolution_14bit->setBounds (104, 366, 80, 24);
    secondary_cc->setBounds (24, 398, 160, 24);
    lo->setBounds (33, 125, 36, 24);
    base->setBounds ((getWidth() / 2) + 16 - ((36) / 2), 125, 36, 24);
    hi_label->setBounds (141, 125, 32, 24);
    hi->setBounds (157, 125, 36, 24);
    ok->setBounds (152, getHeight() - 24, 48, 24);
    boundslo->setBounds (21, 90, 40, 40);
    boundsbase->setBounds (85, 90, 40, 40);
    boundshi->setBounds (149, 90, 40, 40);
    scale_factor_invert->setBounds (130, 32, 64, 24);
    data_decimation_group->setBounds (8, 192, 192, 56);
    data_decimation->setBounds (21, 214, 166, 24);
    control_scope_channel->setBounds (16, 304, 112, 24);
    control_scope_channel_number->setBounds (136, 301, 48, 24);
    bounds_origin_return->setBounds (16, 152, 168, 24);
    //[UserResized] Add your own custom resize handling here..
    if(!control_scope_channel->isVisible())
    {
        control_scope_group->setBounds (8, 254, 192, 50);
    }
    //[/UserResized]
}

void CellPopupComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == scale_factor)
    {
        //[UserSliderCode_scale_factor] -- add your slider handling code here..
        updateMapping();
        //[/UserSliderCode_scale_factor]
    }
    else if (sliderThatWasMoved == boundslo)
    {
        //[UserSliderCode_boundslo] -- add your slider handling code here..
        lo->setText(juce::String((unsigned)boundslo->getValue()), false);
        updateMapping();
        //[/UserSliderCode_boundslo]
    }
    else if (sliderThatWasMoved == boundsbase)
    {
        //[UserSliderCode_boundsbase] -- add your slider handling code here..
        base->setText(juce::String((unsigned)boundsbase->getValue()), false);
        updateMapping();
        //[/UserSliderCode_boundsbase]
    }
    else if (sliderThatWasMoved == boundshi)
    {
        //[UserSliderCode_boundshi] -- add your slider handling code here..
        hi->setText(juce::String((unsigned)boundshi->getValue()), false);
        updateMapping();
        //[/UserSliderCode_boundshi]
    }
    else if (sliderThatWasMoved == data_decimation)
    {
        //[UserSliderCode_data_decimation] -- add your slider handling code here..
        updateMapping();
        //[/UserSliderCode_data_decimation]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void CellPopupComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == mapping_enabled)
    {
        //[UserButtonCode_mapping_enabled] -- add your button handler code here..
        updateMapping();
        //[/UserButtonCode_mapping_enabled]
    }
    else if (buttonThatWasClicked == control_scope_global)
    {
        //[UserButtonCode_control_scope_global] -- add your button handler code here..
        control_scope_channel_number->setEnabled(false);
        updateMapping();
        //[/UserButtonCode_control_scope_global]
    }
    else if (buttonThatWasClicked == control_scope_pernote)
    {
        //[UserButtonCode_control_scope_pernote] -- add your button handler code here..
        control_scope_channel_number->setEnabled(false);
        updateMapping();
        //[/UserButtonCode_control_scope_pernote]
    }
    else if (buttonThatWasClicked == clear_mapping)
    {
        //[UserButtonCode_clear_mapping] -- add your button handler code here..
        cell_editor_->unmap();
        //[/UserButtonCode_clear_mapping]
    }
    else if (buttonThatWasClicked == resolution_7bit)
    {
        //[UserButtonCode_resolution_7bit] -- add your button handler code here..
        secondary_cc->setEnabled(false);
        updateMapping();
        //[/UserButtonCode_resolution_7bit]
    }
    else if (buttonThatWasClicked == resolution_14bit)
    {
        //[UserButtonCode_resolution_14bit] -- add your button handler code here..
        secondary_cc->setEnabled(true);
        updateMapping();
        //[/UserButtonCode_resolution_14bit]
    }
    else if (buttonThatWasClicked == ok)
    {
        //[UserButtonCode_ok] -- add your button handler code here..
        if(cell_editor_)
        {
            cell_editor_->close_popup();
        }
        //[/UserButtonCode_ok]
    }
    else if (buttonThatWasClicked == scale_factor_invert)
    {
        //[UserButtonCode_scale_factor_invert] -- add your button handler code here..
        updateMapping();
        //[/UserButtonCode_scale_factor_invert]
    }
    else if (buttonThatWasClicked == control_scope_channel)
    {
        //[UserButtonCode_control_scope_channel] -- add your button handler code here..
        control_scope_channel_number->setEnabled(true);
        updateMapping();
        //[/UserButtonCode_control_scope_channel]
    }
    else if (buttonThatWasClicked == bounds_origin_return)
    {
        //[UserButtonCode_bounds_origin_return] -- add your button handler code here..
        updateMapping();
        //[/UserButtonCode_bounds_origin_return]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void CellPopupComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == secondary_cc)
    {
        //[UserComboBoxCode_secondary_cc] -- add your combo box handling code here..
        updateMapping();
        //[/UserComboBoxCode_secondary_cc]
    }
    else if (comboBoxThatHasChanged == control_scope_channel_number)
    {
        //[UserComboBoxCode_control_scope_channel_number] -- add your combo box handling code here..
        updateMapping();
        //[/UserComboBoxCode_control_scope_channel_number]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CellPopupComponent" componentName="CellPopup"
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="208" initialHeight="480">
  <BACKGROUND backgroundColour="0"/>
  <GROUPCOMPONENT name="bounds group" id="6d888bc9426a2e47" memberName="bounds_group"
                  virtualName="" explicitFocusOrder="0" pos="8 72 192 112" outlinecol="66eeeeee"
                  textcol="ffeeeeee" title="Bounds (in %)"/>
  <LABEL name="lo label" id="7aa5189d547c63f1" memberName="lo_label" virtualName=""
         explicitFocusOrder="0" pos="16 125 30 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Lo:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="base label" id="d74fb4f0a0a61833" memberName="base_label"
         virtualName="" explicitFocusOrder="0" pos="74 125 40 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Base:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <GROUPCOMPONENT name="scale group" id="237080525f93b812" memberName="scale_group"
                  virtualName="" explicitFocusOrder="0" pos="8 8 192 56" outlinecol="66eeeeee"
                  textcol="ffeeeeee" title="Scale factor"/>
  <GROUPCOMPONENT name="control scope group" id="130d1256c3688b43" memberName="control_scope_group"
                  virtualName="" explicitFocusOrder="0" pos="8 254 192 82" outlinecol="66eeeeee"
                  textcol="ffeeeeee" title="Control scope"/>
  <SLIDER name="scale slider" id="c62d7549b8b11f7d" memberName="scale_factor"
          virtualName="" explicitFocusOrder="0" pos="21 30 107 24" thumbcol="ff8a8a8a"
          rotarysliderfill="fff0ffff" textboxtext="ffeeeeee" textboxbkgd="ffffff"
          textboxhighlight="40000000" textboxoutline="66ffffff" min="0"
          max="10" int="0.1" style="LinearBar" textBoxPos="TextBoxLeft"
          textBoxEditable="0" textBoxWidth="45" textBoxHeight="20" skewFactor="1"/>
  <TOGGLEBUTTON name="enabled toggle button" id="9699df892a5ec5b4" memberName="mapping_enabled"
                virtualName="" explicitFocusOrder="0" pos="64 24R 72 24" txtcol="ffeeeeee"
                buttonText="Enabled" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="1"/>
  <TOGGLEBUTTON name="global scope toggle button" id="597f3f0704f60862" memberName="control_scope_global"
                virtualName="" explicitFocusOrder="0" pos="16 272 72 24" txtcol="ffeeeeee"
                buttonText="Global" connectedEdges="0" needsCallback="1" radioGroupId="1"
                state="1"/>
  <TOGGLEBUTTON name="pernote scope toggle button" id="9a5b8941fc384f38" memberName="control_scope_pernote"
                virtualName="" explicitFocusOrder="0" pos="104 272 80 24" txtcol="ffeeeeee"
                buttonText="Per-note" connectedEdges="0" needsCallback="1" radioGroupId="1"
                state="0"/>
  <TEXTBUTTON name="clear mapping button" id="ed011a91665c322b" memberName="clear_mapping"
              virtualName="" explicitFocusOrder="0" pos="8 24R 48 24" bgColOff="ffc1c1c1"
              buttonText="Clear" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <GROUPCOMPONENT name="resolution group" id="b0c6d411483e371d" memberName="resolution_group"
                  virtualName="" explicitFocusOrder="0" pos="8 342 192 96" outlinecol="66eeeeee"
                  textcol="ffeeeeee" title="Resolution"/>
  <TOGGLEBUTTON name="7bit resolution toggle button" id="f6462456a441a00f" memberName="resolution_7bit"
                virtualName="" explicitFocusOrder="0" pos="24 366 72 24" txtcol="ffeeeeee"
                buttonText="7 bit" connectedEdges="0" needsCallback="1" radioGroupId="2"
                state="1"/>
  <TOGGLEBUTTON name="14 bit resolution toggle button" id="d12409e4a3754609"
                memberName="resolution_14bit" virtualName="" explicitFocusOrder="0"
                pos="104 366 80 24" txtcol="ffeeeeee" buttonText="14 bit" connectedEdges="0"
                needsCallback="1" radioGroupId="2" state="0"/>
  <COMBOBOX name="secondary cc combo box" id="af7acf92d99991de" memberName="secondary_cc"
            virtualName="" explicitFocusOrder="0" pos="24 398 160 24" editable="0"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <LABEL name="lo" id="471687e1a09eef00" memberName="lo" virtualName=""
         explicitFocusOrder="0" pos="33 125 36 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="0" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="base" id="b15f2763d2b38be6" memberName="base" virtualName=""
         explicitFocusOrder="0" pos="16Cc 125 36 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="0" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="hi label" id="800da5e563ce405" memberName="hi_label" virtualName=""
         explicitFocusOrder="0" pos="141 125 32 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Hi:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="hi" id="edb873f4838f0da2" memberName="hi" virtualName=""
         explicitFocusOrder="0" pos="157 125 36 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="100" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <TEXTBUTTON name="ok button" id="489abe6be4232158" memberName="ok" virtualName=""
              explicitFocusOrder="0" pos="152 24R 48 24" bgColOff="ffc1c1c1"
              buttonText="Ok" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <SLIDER name="boundslo dial" id="7b019cec9fcb7ea1" memberName="boundslo"
          virtualName="" explicitFocusOrder="0" pos="21 90 40 40" thumbcol="ff8a8a8a"
          trackcol="7fffffff" rotarysliderfill="ddffffff" rotaryslideroutline="66ffffff"
          textboxtext="ffeeeeee" textboxbkgd="ffffff" textboxhighlight="40000000"
          textboxoutline="0" min="0" max="100" int="1" style="Rotary" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="45" textBoxHeight="14" skewFactor="1"/>
  <SLIDER name="boundsbase dial" id="2db1da6e8c60e90c" memberName="boundsbase"
          virtualName="" explicitFocusOrder="0" pos="85 90 40 40" thumbcol="ff8a8a8a"
          rotarysliderfill="ddffffff" rotaryslideroutline="66ffffff" textboxtext="ffeeeeee"
          textboxbkgd="ffffff" textboxhighlight="40000000" textboxoutline="0"
          min="0" max="100" int="1" style="Rotary" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="45" textBoxHeight="14" skewFactor="1"/>
  <SLIDER name="boundshi dial" id="1c37d364908c9764" memberName="boundshi"
          virtualName="" explicitFocusOrder="0" pos="149 90 40 40" thumbcol="ff8a8a8a"
          rotarysliderfill="ddffffff" rotaryslideroutline="66ffffff" textboxtext="ffeeeeee"
          textboxbkgd="ffffff" textboxhighlight="40000000" textboxoutline="0"
          min="0" max="100" int="1" style="Rotary" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="45" textBoxHeight="14" skewFactor="1"/>
  <TOGGLEBUTTON name="scale factor invert toggle button" id="a30c500f267f6636"
                memberName="scale_factor_invert" virtualName="" explicitFocusOrder="0"
                pos="130 32 64 24" txtcol="ffeeeeee" buttonText="Invert" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <GROUPCOMPONENT name="data decimation group" id="712092c3774a0aa8" memberName="data_decimation_group"
                  virtualName="" explicitFocusOrder="0" pos="8 192 192 56" outlinecol="66eeeeee"
                  textcol="ffeeeeee" title="Data decimation (in ms)"/>
  <SLIDER name="data decimation slider" id="8b94f735d1a426e9" memberName="data_decimation"
          virtualName="" explicitFocusOrder="0" pos="21 214 166 24" thumbcol="ff8a8a8a"
          rotarysliderfill="fff0ffff" textboxtext="ffeeeeee" textboxbkgd="ffffff"
          textboxhighlight="40000000" textboxoutline="66ffffff" min="0"
          max="100" int="1" style="LinearBar" textBoxPos="TextBoxLeft"
          textBoxEditable="0" textBoxWidth="45" textBoxHeight="20" skewFactor="1"/>
  <TOGGLEBUTTON name="channel scope toggle button" id="601cb1dfa2e060cf" memberName="control_scope_channel"
                virtualName="" explicitFocusOrder="0" pos="16 304 112 24" txtcol="ffeeeeee"
                buttonText="Fixed channel" connectedEdges="0" needsCallback="1"
                radioGroupId="1" state="0"/>
  <COMBOBOX name="fixed channel combo box" id="cf5597acdc45560c" memberName="control_scope_channel_number"
            virtualName="" explicitFocusOrder="0" pos="136 301 48 24" editable="0"
            layout="34" items="1&#10;2&#10;3&#10;4&#10;5&#10;6&#10;7&#10;8&#10;9&#10;10&#10;11&#10;12&#10;13&#10;14&#10;15&#10;16"
            textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <TOGGLEBUTTON name="bounds return to origin" id="8d7dccf06a79a45b" memberName="bounds_origin_return"
                virtualName="" explicitFocusOrder="0" pos="16 152 168 24" txtcol="ffeeeeee"
                buttonText="Always return to origin" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
