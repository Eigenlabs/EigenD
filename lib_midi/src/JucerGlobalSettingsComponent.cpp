/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  13 Jul 2012 3:26:59pm

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
//[/Headers]

#include "JucerGlobalSettingsComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
void GlobalSettingsComponent::initialize(midi::mapping_delegate_t *mapping_delegate)
{
    mapping_delegate_ = 0;

    updateComponent(mapping_delegate);

    mapping_delegate_ = mapping_delegate;
}

void GlobalSettingsComponent::setDialogWindow(DialogWindow *window)
{
    window_ = window;
}

void GlobalSettingsComponent::updateComponent()
{
    updateComponent(mapping_delegate_);
}

void GlobalSettingsComponent::updateComponent(midi::mapping_delegate_t *mapping_delegate)
{
    if(!mapping_delegate) return;

    midi::global_settings_t settings = mapping_delegate->get_settings();

    data_decimation->setValue(settings.minimum_decimation_, true, true);
    midi_notes->setToggleState(settings.send_notes_, false);
    midi_pitchbend->setToggleState(settings.send_pitchbend_, false);
    midi_hires_velocity->setToggleState(settings.send_hires_velocity_, false);

    unsigned midi_channel = mapping_delegate->get_midi_channel();
    if(0==midi_channel)
    {
        midi_channel = 17;
    }

    active_channel->setSelectedId(midi_channel);
    min_channel->setSelectedId(mapping_delegate->get_min_channel());
    max_channel->setSelectedId(mapping_delegate->get_max_channel());
    pitchbend_up->setValue((int)settings.pitchbend_semitones_up_,true,true);
    pitchbend_down->setValue((int)settings.pitchbend_semitones_down_,true,true);
}

void GlobalSettingsComponent::setFocusOrder()
{
    midi_notes->setWantsKeyboardFocus(false);
    midi_pitchbend->setWantsKeyboardFocus(false);
    ok->setWantsKeyboardFocus(false);
}

void GlobalSettingsComponent::updateSettings()
{
    if(!mapping_delegate_) return;

    bool send_notes = midi_notes->getToggleState();
    bool send_pitchbend = midi_pitchbend->getToggleState();
    bool send_hires_velocity = midi_hires_velocity->getToggleState();
    unsigned pb_up = pitchbend_up->getValue();
    unsigned pb_down = pitchbend_down->getValue();

    mapping_delegate_->change_settings(midi::global_settings_t(data_decimation->getValue(), send_notes, send_pitchbend, send_hires_velocity, pb_up, pb_down));
}
//[/MiscUserDefs]

//==============================================================================
GlobalSettingsComponent::GlobalSettingsComponent ()
    : Component (L"GlobalSettings"),
      midi_group (0),
      ok (0),
      midi_notes (0),
      midi_pitchbend (0),
      min_data_decimation_group (0),
      data_decimation (0),
      active_channel_label (0),
      active_channel (0),
      min_channel_label (0),
      min_channel (0),
      max_channel_label (0),
      max_channel (0),
      midi_hires_velocity (0),
      pitchbend_up_label (0),
      pitchbend_down_label (0),
      pitchbend_up (0),
      pitchbend_down (0)
{
    addAndMakeVisible (midi_group = new GroupComponent (L"midi group",
                                                        L"MIDI"));
    midi_group->setColour (GroupComponent::outlineColourId, Colour (0x66eeeeee));
    midi_group->setColour (GroupComponent::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (ok = new TextButton (L"ok button"));
    ok->setButtonText (L"Ok");
    ok->addListener (this);
    ok->setColour (TextButton::buttonColourId, Colour (0xffc1c1c1));

    addAndMakeVisible (midi_notes = new ToggleButton (L"midi notes toggle button"));
    midi_notes->setButtonText (L"send notes");
    midi_notes->addListener (this);
    midi_notes->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (midi_pitchbend = new ToggleButton (L"midi pitchbend toggle button"));
    midi_pitchbend->setButtonText (L"send pitchbend");
    midi_pitchbend->addListener (this);
    midi_pitchbend->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (min_data_decimation_group = new GroupComponent (L"min data decimation group",
                                                                       L"Minimum data decimation (in ms)"));
    min_data_decimation_group->setColour (GroupComponent::outlineColourId, Colour (0x66eeeeee));
    min_data_decimation_group->setColour (GroupComponent::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (data_decimation = new Slider (L"data decimation slider"));
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

    addAndMakeVisible (active_channel_label = new Label (L"active channel label",
                                                         L"Active midi channel"));
    active_channel_label->setFont (Font (15.0000f, Font::plain));
    active_channel_label->setJustificationType (Justification::centredLeft);
    active_channel_label->setEditable (false, false, false);
    active_channel_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    active_channel_label->setColour (TextEditor::textColourId, Colours::black);
    active_channel_label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (active_channel = new ComboBox (L"active channel combo box"));
    active_channel->setEditableText (false);
    active_channel->setJustificationType (Justification::centredRight);
    active_channel->setTextWhenNothingSelected (String::empty);
    active_channel->setTextWhenNoChoicesAvailable (L"(no choices)");
    active_channel->addItem (L"1", 1);
    active_channel->addItem (L"2", 2);
    active_channel->addItem (L"3", 3);
    active_channel->addItem (L"4", 4);
    active_channel->addItem (L"5", 5);
    active_channel->addItem (L"6", 6);
    active_channel->addItem (L"7", 7);
    active_channel->addItem (L"8", 8);
    active_channel->addItem (L"9", 9);
    active_channel->addItem (L"10", 10);
    active_channel->addItem (L"11", 11);
    active_channel->addItem (L"12", 12);
    active_channel->addItem (L"13", 13);
    active_channel->addItem (L"14", 14);
    active_channel->addItem (L"15", 15);
    active_channel->addItem (L"16", 16);
    active_channel->addItem (L"Poly", 17);
    active_channel->addListener (this);

    addAndMakeVisible (min_channel_label = new Label (L"min channel label",
                                                      L"Minimum poly channel"));
    min_channel_label->setFont (Font (15.0000f, Font::plain));
    min_channel_label->setJustificationType (Justification::centredLeft);
    min_channel_label->setEditable (false, false, false);
    min_channel_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    min_channel_label->setColour (TextEditor::textColourId, Colours::black);
    min_channel_label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (min_channel = new ComboBox (L"min channel combo box"));
    min_channel->setEditableText (false);
    min_channel->setJustificationType (Justification::centredRight);
    min_channel->setTextWhenNothingSelected (String::empty);
    min_channel->setTextWhenNoChoicesAvailable (L"(no choices)");
    min_channel->addItem (L"1", 1);
    min_channel->addItem (L"2", 2);
    min_channel->addItem (L"3", 3);
    min_channel->addItem (L"4", 4);
    min_channel->addItem (L"5", 5);
    min_channel->addItem (L"6", 6);
    min_channel->addItem (L"7", 7);
    min_channel->addItem (L"8", 8);
    min_channel->addItem (L"9", 9);
    min_channel->addItem (L"10", 10);
    min_channel->addItem (L"11", 11);
    min_channel->addItem (L"12", 12);
    min_channel->addItem (L"13", 13);
    min_channel->addItem (L"14", 14);
    min_channel->addItem (L"15", 15);
    min_channel->addItem (L"16", 16);
    min_channel->addListener (this);

    addAndMakeVisible (max_channel_label = new Label (L"max channel label",
                                                      L"Maximum poly channel"));
    max_channel_label->setFont (Font (15.0000f, Font::plain));
    max_channel_label->setJustificationType (Justification::centredLeft);
    max_channel_label->setEditable (false, false, false);
    max_channel_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    max_channel_label->setColour (TextEditor::textColourId, Colours::black);
    max_channel_label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (max_channel = new ComboBox (L"max channel combo box"));
    max_channel->setEditableText (false);
    max_channel->setJustificationType (Justification::centredRight);
    max_channel->setTextWhenNothingSelected (String::empty);
    max_channel->setTextWhenNoChoicesAvailable (L"(no choices)");
    max_channel->addItem (L"1", 1);
    max_channel->addItem (L"2", 2);
    max_channel->addItem (L"3", 3);
    max_channel->addItem (L"4", 4);
    max_channel->addItem (L"5", 5);
    max_channel->addItem (L"6", 6);
    max_channel->addItem (L"7", 7);
    max_channel->addItem (L"8", 8);
    max_channel->addItem (L"9", 9);
    max_channel->addItem (L"10", 10);
    max_channel->addItem (L"11", 11);
    max_channel->addItem (L"12", 12);
    max_channel->addItem (L"13", 13);
    max_channel->addItem (L"14", 14);
    max_channel->addItem (L"15", 15);
    max_channel->addItem (L"16", 16);
    max_channel->addListener (this);

    addAndMakeVisible (midi_hires_velocity = new ToggleButton (L"midi hires velocity toggle button"));
    midi_hires_velocity->setButtonText (L"send high resolution velocity");
    midi_hires_velocity->addListener (this);
    midi_hires_velocity->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (pitchbend_up_label = new Label (L"pitchbend up label",
                                                       L"Pitch bend up range (semis)"));
    pitchbend_up_label->setFont (Font (15.0000f, Font::plain));
    pitchbend_up_label->setJustificationType (Justification::centred);
    pitchbend_up_label->setEditable (false, false, false);
    pitchbend_up_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    pitchbend_up_label->setColour (TextEditor::textColourId, Colours::black);
    pitchbend_up_label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (pitchbend_down_label = new Label (L"pitchbend down label",
                                                         L"Pitch bend down range (semis)"));
    pitchbend_down_label->setFont (Font (15.0000f, Font::plain));
    pitchbend_down_label->setJustificationType (Justification::centred);
    pitchbend_down_label->setEditable (false, false, false);
    pitchbend_down_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    pitchbend_down_label->setColour (TextEditor::textColourId, Colours::black);
    pitchbend_down_label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (pitchbend_up = new Slider (L"pitchbend up slider"));
    pitchbend_up->setRange (0, 48, 0.1);
    pitchbend_up->setSliderStyle (Slider::LinearBar);
    pitchbend_up->setTextBoxStyle (Slider::TextBoxLeft, true, 80, 20);
    pitchbend_up->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    pitchbend_up->setColour (Slider::rotarySliderFillColourId, Colours::azure);
    pitchbend_up->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    pitchbend_up->setColour (Slider::textBoxBackgroundColourId, Colour (0xffffff));
    pitchbend_up->setColour (Slider::textBoxHighlightColourId, Colour (0x40000000));
    pitchbend_up->setColour (Slider::textBoxOutlineColourId, Colour (0x66ffffff));
    pitchbend_up->addListener (this);

    addAndMakeVisible (pitchbend_down = new Slider (L"pitchbend down slider"));
    pitchbend_down->setRange (0, 48, 0.1);
    pitchbend_down->setSliderStyle (Slider::LinearBar);
    pitchbend_down->setTextBoxStyle (Slider::TextBoxLeft, true, 80, 20);
    pitchbend_down->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    pitchbend_down->setColour (Slider::rotarySliderFillColourId, Colours::azure);
    pitchbend_down->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    pitchbend_down->setColour (Slider::textBoxBackgroundColourId, Colour (0xffffff));
    pitchbend_down->setColour (Slider::textBoxHighlightColourId, Colour (0x40000000));
    pitchbend_down->setColour (Slider::textBoxOutlineColourId, Colour (0x66ffffff));
    pitchbend_down->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (256, 400);


    //[Constructor] You can add your own custom stuff here..
    window_ = 0;
    mapping_delegate_ = 0;
    //[/Constructor]
}

GlobalSettingsComponent::~GlobalSettingsComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (midi_group);
    deleteAndZero (ok);
    deleteAndZero (midi_notes);
    deleteAndZero (midi_pitchbend);
    deleteAndZero (min_data_decimation_group);
    deleteAndZero (data_decimation);
    deleteAndZero (active_channel_label);
    deleteAndZero (active_channel);
    deleteAndZero (min_channel_label);
    deleteAndZero (min_channel);
    deleteAndZero (max_channel_label);
    deleteAndZero (max_channel);
    deleteAndZero (midi_hires_velocity);
    deleteAndZero (pitchbend_up_label);
    deleteAndZero (pitchbend_down_label);
    deleteAndZero (pitchbend_up);
    deleteAndZero (pitchbend_down);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void GlobalSettingsComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void GlobalSettingsComponent::resized()
{
    midi_group->setBounds (8, 72, 240, 296);
    ok->setBounds (200, getHeight() - 24, 48, 24);
    midi_notes->setBounds (16, 88, 104, 24);
    midi_pitchbend->setBounds (118, 88, 128, 24);
    min_data_decimation_group->setBounds (8, 8, 240, 56);
    data_decimation->setBounds (21, 30, 214, 24);
    active_channel_label->setBounds (16, 144, 136, 24);
    active_channel->setBounds (176, 144, 56, 24);
    min_channel_label->setBounds (16, 176, 152, 24);
    min_channel->setBounds (176, 176, 56, 24);
    max_channel_label->setBounds (16, 208, 160, 24);
    max_channel->setBounds (176, 208, 56, 24);
    midi_hires_velocity->setBounds (16, 112, 216, 24);
    pitchbend_up_label->setBounds (16, 240, 224, 24);
    pitchbend_down_label->setBounds (16, 296, 224, 24);
    pitchbend_up->setBounds (16, 272, 224, 24);
    pitchbend_down->setBounds (16, 328, 224, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void GlobalSettingsComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == ok)
    {
        //[UserButtonCode_ok] -- add your button handler code here..
        if(window_)
        {
            window_->closeButtonPressed();
        }
        //[/UserButtonCode_ok]
    }
    else if (buttonThatWasClicked == midi_notes)
    {
        //[UserButtonCode_midi_notes] -- add your button handler code here..
        updateSettings();
        //[/UserButtonCode_midi_notes]
    }
    else if (buttonThatWasClicked == midi_pitchbend)
    {
        //[UserButtonCode_midi_pitchbend] -- add your button handler code here..
        updateSettings();
        //[/UserButtonCode_midi_pitchbend]
    }
    else if (buttonThatWasClicked == midi_hires_velocity)
    {
        //[UserButtonCode_midi_hires_velocity] -- add your button handler code here..
        updateSettings();
        //[/UserButtonCode_midi_hires_velocity]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void GlobalSettingsComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == data_decimation)
    {
        //[UserSliderCode_data_decimation] -- add your slider handling code here..
        updateSettings();
        //[/UserSliderCode_data_decimation]
    }
    else if (sliderThatWasMoved == pitchbend_up)
    {
        //[UserSliderCode_pitchbend_up] -- add your slider handling code here..
        updateSettings();
        //[/UserSliderCode_pitchbend_up]
    }
    else if (sliderThatWasMoved == pitchbend_down)
    {
        //[UserSliderCode_pitchbend_down] -- add your slider handling code here..
        updateSettings();
        //[/UserSliderCode_pitchbend_down]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void GlobalSettingsComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == active_channel)
    {
        //[UserComboBoxCode_active_channel] -- add your combo box handling code here..
        unsigned midi_channel = active_channel->getSelectedId();
        if(midi_channel>0)
        {
            if(17==midi_channel)
            {
                midi_channel = 0;
            }
            mapping_delegate_->set_midi_channel(midi_channel);
        }
        //[/UserComboBoxCode_active_channel]
    }
    else if (comboBoxThatHasChanged == min_channel)
    {
        //[UserComboBoxCode_min_channel] -- add your combo box handling code here..
        mapping_delegate_->set_min_channel(min_channel->getSelectedId());
        //[/UserComboBoxCode_min_channel]
    }
    else if (comboBoxThatHasChanged == max_channel)
    {
        //[UserComboBoxCode_max_channel] -- add your combo box handling code here..
        mapping_delegate_->set_max_channel(max_channel->getSelectedId());
        //[/UserComboBoxCode_max_channel]
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

<JUCER_COMPONENT documentType="Component" className="GlobalSettingsComponent"
                 componentName="GlobalSettings" parentClasses="public Component"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330000013" fixedSize="1" initialWidth="256"
                 initialHeight="400">
  <BACKGROUND backgroundColour="0"/>
  <GROUPCOMPONENT name="midi group" id="237080525f93b812" memberName="midi_group"
                  virtualName="" explicitFocusOrder="0" pos="8 72 240 296" outlinecol="66eeeeee"
                  textcol="ffeeeeee" title="MIDI"/>
  <TEXTBUTTON name="ok button" id="489abe6be4232158" memberName="ok" virtualName=""
              explicitFocusOrder="0" pos="200 24R 48 24" bgColOff="ffc1c1c1"
              buttonText="Ok" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TOGGLEBUTTON name="midi notes toggle button" id="a30c500f267f6636" memberName="midi_notes"
                virtualName="" explicitFocusOrder="0" pos="16 88 104 24" txtcol="ffeeeeee"
                buttonText="send notes" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="midi pitchbend toggle button" id="a4e70937ebc61f01" memberName="midi_pitchbend"
                virtualName="" explicitFocusOrder="0" pos="118 88 128 24" txtcol="ffeeeeee"
                buttonText="send pitchbend" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
  <GROUPCOMPONENT name="min data decimation group" id="712092c3774a0aa8" memberName="min_data_decimation_group"
                  virtualName="" explicitFocusOrder="0" pos="8 8 240 56" outlinecol="66eeeeee"
                  textcol="ffeeeeee" title="Minimum data decimation (in ms)"/>
  <SLIDER name="data decimation slider" id="8b94f735d1a426e9" memberName="data_decimation"
          virtualName="" explicitFocusOrder="0" pos="21 30 214 24" thumbcol="ff8a8a8a"
          rotarysliderfill="fff0ffff" textboxtext="ffeeeeee" textboxbkgd="ffffff"
          textboxhighlight="40000000" textboxoutline="66ffffff" min="0"
          max="100" int="1" style="LinearBar" textBoxPos="TextBoxLeft"
          textBoxEditable="0" textBoxWidth="45" textBoxHeight="20" skewFactor="1"/>
  <LABEL name="active channel label" id="d74fb4f0a0a61833" memberName="active_channel_label"
         virtualName="" explicitFocusOrder="0" pos="16 144 136 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Active midi channel"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <COMBOBOX name="active channel combo box" id="cf5597acdc45560c" memberName="active_channel"
            virtualName="" explicitFocusOrder="0" pos="176 144 56 24" editable="0"
            layout="34" items="1&#10;2&#10;3&#10;4&#10;5&#10;6&#10;7&#10;8&#10;9&#10;10&#10;11&#10;12&#10;13&#10;14&#10;15&#10;16&#10;Poly"
            textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <LABEL name="min channel label" id="1b3bcc844c3b5b67" memberName="min_channel_label"
         virtualName="" explicitFocusOrder="0" pos="16 176 152 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Minimum poly channel"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <COMBOBOX name="min channel combo box" id="74cbf8d0e6dce2b4" memberName="min_channel"
            virtualName="" explicitFocusOrder="0" pos="176 176 56 24" editable="0"
            layout="34" items="1&#10;2&#10;3&#10;4&#10;5&#10;6&#10;7&#10;8&#10;9&#10;10&#10;11&#10;12&#10;13&#10;14&#10;15&#10;16"
            textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <LABEL name="max channel label" id="4ff09523d24f844" memberName="max_channel_label"
         virtualName="" explicitFocusOrder="0" pos="16 208 160 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Maximum poly channel"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <COMBOBOX name="max channel combo box" id="1531558de1c1a2c1" memberName="max_channel"
            virtualName="" explicitFocusOrder="0" pos="176 208 56 24" editable="0"
            layout="34" items="1&#10;2&#10;3&#10;4&#10;5&#10;6&#10;7&#10;8&#10;9&#10;10&#10;11&#10;12&#10;13&#10;14&#10;15&#10;16"
            textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <TOGGLEBUTTON name="midi hires velocity toggle button" id="127457ac3bc55b74"
                memberName="midi_hires_velocity" virtualName="" explicitFocusOrder="0"
                pos="16 112 216 24" txtcol="ffeeeeee" buttonText="send high resolution velocity"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <LABEL name="pitchbend up label" id="a560c9a5ba59aa86" memberName="pitchbend_up_label"
         virtualName="" explicitFocusOrder="0" pos="16 240 224 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Pitch bend up range (semis)"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
  <LABEL name="pitchbend down label" id="c0c5425a49b2d366" memberName="pitchbend_down_label"
         virtualName="" explicitFocusOrder="0" pos="16 296 224 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Pitch bend down range (semis)"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
  <SLIDER name="pitchbend up slider" id="ad6f6ba17280bd77" memberName="pitchbend_up"
          virtualName="" explicitFocusOrder="0" pos="16 272 224 24" thumbcol="ff8a8a8a"
          rotarysliderfill="fff0ffff" textboxtext="ffeeeeee" textboxbkgd="ffffff"
          textboxhighlight="40000000" textboxoutline="66ffffff" min="0"
          max="48" int="0.1" style="LinearBar" textBoxPos="TextBoxLeft"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="pitchbend down slider" id="44141a9a8275e444" memberName="pitchbend_down"
          virtualName="" explicitFocusOrder="0" pos="16 328 224 24" thumbcol="ff8a8a8a"
          rotarysliderfill="fff0ffff" textboxtext="ffeeeeee" textboxbkgd="ffffff"
          textboxhighlight="40000000" textboxoutline="66ffffff" min="0"
          max="48" int="0.1" style="LinearBar" textBoxPos="TextBoxLeft"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
