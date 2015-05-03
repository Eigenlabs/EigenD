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

    data_decimation->setValue(settings.minimum_decimation_, sendNotificationSync);
    midi_notes->setToggleState(settings.send_notes_, false);
    midi_pitchbend->setToggleState(settings.send_pitchbend_, false);
    midi_hires_velocity->setToggleState(settings.send_hires_velocity_, false);

    unsigned midi_channel = settings.midi_channel_;
    if(0==midi_channel) { midi_channel = 17; }

    active_channel->setSelectedId(midi_channel);
    min_channel->setSelectedId(settings.minimum_midi_channel_);
    max_channel->setSelectedId(settings.maximum_midi_channel_);
    pitchbend_up->setValue((int)settings.pitchbend_semitones_up_,sendNotificationSync);
    pitchbend_down->setValue((int)settings.pitchbend_semitones_down_,sendNotificationSync);
}

void GlobalSettingsComponent::setFocusOrder()
{
    midi_notes->setWantsKeyboardFocus(false);
    midi_pitchbend->setWantsKeyboardFocus(false);
    ok->setWantsKeyboardFocus(false);
}

void GlobalSettingsComponent::handleMessage(const Message &)
{
    if(!mapping_delegate_) return;

    unsigned midi_channel = active_channel->getSelectedId();
    if(midi_channel>0)
    {
        if(17==midi_channel)
        {
            midi_channel = 0;
        }
    }
    bool send_notes = midi_notes->getToggleState();
    bool send_pitchbend = midi_pitchbend->getToggleState();
    bool send_hires_velocity = midi_hires_velocity->getToggleState();
    unsigned pb_up = pitchbend_up->getValue();
    unsigned pb_down = pitchbend_down->getValue();

    mapping_delegate_->change_settings(midi::global_settings_t(midi_channel, min_channel->getSelectedId(), max_channel->getSelectedId(), data_decimation->getValue(), send_notes, send_pitchbend, send_hires_velocity, pb_up, pb_down));
}

void GlobalSettingsComponent::updateSettings()
{
    postMessage(new Message());
}
//[/MiscUserDefs]

//==============================================================================
GlobalSettingsComponent::GlobalSettingsComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    setName ("GlobalSettings");
    addAndMakeVisible (midi_group = new GroupComponent ("midi group",
                                                        TRANS("MIDI")));
    midi_group->setColour (GroupComponent::outlineColourId, Colour (0x66eeeeee));
    midi_group->setColour (GroupComponent::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (ok = new TextButton ("ok button"));
    ok->setButtonText (TRANS("Ok"));
    ok->addListener (this);
    ok->setColour (TextButton::buttonColourId, Colour (0xffc1c1c1));

    addAndMakeVisible (midi_notes = new ToggleButton ("midi notes toggle button"));
    midi_notes->setButtonText (TRANS("send notes"));
    midi_notes->addListener (this);
    midi_notes->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (midi_pitchbend = new ToggleButton ("midi pitchbend toggle button"));
    midi_pitchbend->setButtonText (TRANS("send pitchbend"));
    midi_pitchbend->addListener (this);
    midi_pitchbend->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (min_data_decimation_group = new GroupComponent ("min data decimation group",
                                                                       TRANS("Minimum data decimation (in ms)")));
    min_data_decimation_group->setColour (GroupComponent::outlineColourId, Colour (0x66eeeeee));
    min_data_decimation_group->setColour (GroupComponent::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (data_decimation = new Slider ("data decimation slider"));
    data_decimation->setRange (0, 100, 1);
    data_decimation->setSliderStyle (Slider::LinearBar);
    data_decimation->setTextBoxStyle (Slider::TextBoxLeft, true, 45, 20);
    data_decimation->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    data_decimation->setColour (Slider::rotarySliderFillColourId, Colours::azure);
    data_decimation->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    data_decimation->setColour (Slider::textBoxBackgroundColourId, Colour (0x00ffffff));
    data_decimation->setColour (Slider::textBoxHighlightColourId, Colour (0x40000000));
    data_decimation->setColour (Slider::textBoxOutlineColourId, Colour (0x66ffffff));
    data_decimation->addListener (this);

    addAndMakeVisible (active_channel_label = new Label ("active channel label",
                                                         TRANS("Active midi channel")));
    active_channel_label->setFont (Font (15.00f, Font::plain));
    active_channel_label->setJustificationType (Justification::centredLeft);
    active_channel_label->setEditable (false, false, false);
    active_channel_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    active_channel_label->setColour (TextEditor::textColourId, Colours::black);
    active_channel_label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (active_channel = new ComboBox ("active channel combo box"));
    active_channel->setEditableText (false);
    active_channel->setJustificationType (Justification::centredRight);
    active_channel->setTextWhenNothingSelected (String::empty);
    active_channel->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    active_channel->addItem (TRANS("1"), 1);
    active_channel->addItem (TRANS("2"), 2);
    active_channel->addItem (TRANS("3"), 3);
    active_channel->addItem (TRANS("4"), 4);
    active_channel->addItem (TRANS("5"), 5);
    active_channel->addItem (TRANS("6"), 6);
    active_channel->addItem (TRANS("7"), 7);
    active_channel->addItem (TRANS("8"), 8);
    active_channel->addItem (TRANS("9"), 9);
    active_channel->addItem (TRANS("10"), 10);
    active_channel->addItem (TRANS("11"), 11);
    active_channel->addItem (TRANS("12"), 12);
    active_channel->addItem (TRANS("13"), 13);
    active_channel->addItem (TRANS("14"), 14);
    active_channel->addItem (TRANS("15"), 15);
    active_channel->addItem (TRANS("16"), 16);
    active_channel->addItem (TRANS("Poly"), 17);
    active_channel->addListener (this);

    addAndMakeVisible (min_channel_label = new Label ("min channel label",
                                                      TRANS("Minimum poly channel")));
    min_channel_label->setFont (Font (15.00f, Font::plain));
    min_channel_label->setJustificationType (Justification::centredLeft);
    min_channel_label->setEditable (false, false, false);
    min_channel_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    min_channel_label->setColour (TextEditor::textColourId, Colours::black);
    min_channel_label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (min_channel = new ComboBox ("min channel combo box"));
    min_channel->setEditableText (false);
    min_channel->setJustificationType (Justification::centredRight);
    min_channel->setTextWhenNothingSelected (String::empty);
    min_channel->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    min_channel->addItem (TRANS("1"), 1);
    min_channel->addItem (TRANS("2"), 2);
    min_channel->addItem (TRANS("3"), 3);
    min_channel->addItem (TRANS("4"), 4);
    min_channel->addItem (TRANS("5"), 5);
    min_channel->addItem (TRANS("6"), 6);
    min_channel->addItem (TRANS("7"), 7);
    min_channel->addItem (TRANS("8"), 8);
    min_channel->addItem (TRANS("9"), 9);
    min_channel->addItem (TRANS("10"), 10);
    min_channel->addItem (TRANS("11"), 11);
    min_channel->addItem (TRANS("12"), 12);
    min_channel->addItem (TRANS("13"), 13);
    min_channel->addItem (TRANS("14"), 14);
    min_channel->addItem (TRANS("15"), 15);
    min_channel->addItem (TRANS("16"), 16);
    min_channel->addListener (this);

    addAndMakeVisible (max_channel_label = new Label ("max channel label",
                                                      TRANS("Maximum poly channel")));
    max_channel_label->setFont (Font (15.00f, Font::plain));
    max_channel_label->setJustificationType (Justification::centredLeft);
    max_channel_label->setEditable (false, false, false);
    max_channel_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    max_channel_label->setColour (TextEditor::textColourId, Colours::black);
    max_channel_label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (max_channel = new ComboBox ("max channel combo box"));
    max_channel->setEditableText (false);
    max_channel->setJustificationType (Justification::centredRight);
    max_channel->setTextWhenNothingSelected (String::empty);
    max_channel->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    max_channel->addItem (TRANS("1"), 1);
    max_channel->addItem (TRANS("2"), 2);
    max_channel->addItem (TRANS("3"), 3);
    max_channel->addItem (TRANS("4"), 4);
    max_channel->addItem (TRANS("5"), 5);
    max_channel->addItem (TRANS("6"), 6);
    max_channel->addItem (TRANS("7"), 7);
    max_channel->addItem (TRANS("8"), 8);
    max_channel->addItem (TRANS("9"), 9);
    max_channel->addItem (TRANS("10"), 10);
    max_channel->addItem (TRANS("11"), 11);
    max_channel->addItem (TRANS("12"), 12);
    max_channel->addItem (TRANS("13"), 13);
    max_channel->addItem (TRANS("14"), 14);
    max_channel->addItem (TRANS("15"), 15);
    max_channel->addItem (TRANS("16"), 16);
    max_channel->addListener (this);

    addAndMakeVisible (midi_hires_velocity = new ToggleButton ("midi hires velocity toggle button"));
    midi_hires_velocity->setButtonText (TRANS("send high resolution velocity"));
    midi_hires_velocity->addListener (this);
    midi_hires_velocity->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (pitchbend_up_label = new Label ("pitchbend up label",
                                                       TRANS("Pitch bend up range (semis)")));
    pitchbend_up_label->setFont (Font (15.00f, Font::plain));
    pitchbend_up_label->setJustificationType (Justification::centred);
    pitchbend_up_label->setEditable (false, false, false);
    pitchbend_up_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    pitchbend_up_label->setColour (TextEditor::textColourId, Colours::black);
    pitchbend_up_label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (pitchbend_down_label = new Label ("pitchbend down label",
                                                         TRANS("Pitch bend down range (semis)")));
    pitchbend_down_label->setFont (Font (15.00f, Font::plain));
    pitchbend_down_label->setJustificationType (Justification::centred);
    pitchbend_down_label->setEditable (false, false, false);
    pitchbend_down_label->setColour (Label::textColourId, Colour (0xffeeeeee));
    pitchbend_down_label->setColour (TextEditor::textColourId, Colours::black);
    pitchbend_down_label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (pitchbend_up = new Slider ("pitchbend up slider"));
    pitchbend_up->setRange (0, 48, 0.1);
    pitchbend_up->setSliderStyle (Slider::LinearBar);
    pitchbend_up->setTextBoxStyle (Slider::TextBoxLeft, true, 80, 20);
    pitchbend_up->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    pitchbend_up->setColour (Slider::rotarySliderFillColourId, Colours::azure);
    pitchbend_up->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    pitchbend_up->setColour (Slider::textBoxBackgroundColourId, Colour (0x00ffffff));
    pitchbend_up->setColour (Slider::textBoxHighlightColourId, Colour (0x40000000));
    pitchbend_up->setColour (Slider::textBoxOutlineColourId, Colour (0x66ffffff));
    pitchbend_up->addListener (this);

    addAndMakeVisible (pitchbend_down = new Slider ("pitchbend down slider"));
    pitchbend_down->setRange (0, 48, 0.1);
    pitchbend_down->setSliderStyle (Slider::LinearBar);
    pitchbend_down->setTextBoxStyle (Slider::TextBoxLeft, true, 80, 20);
    pitchbend_down->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    pitchbend_down->setColour (Slider::rotarySliderFillColourId, Colours::azure);
    pitchbend_down->setColour (Slider::textBoxTextColourId, Colour (0xffeeeeee));
    pitchbend_down->setColour (Slider::textBoxBackgroundColourId, Colour (0x00ffffff));
    pitchbend_down->setColour (Slider::textBoxHighlightColourId, Colour (0x40000000));
    pitchbend_down->setColour (Slider::textBoxOutlineColourId, Colour (0x66ffffff));
    pitchbend_down->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (256, 410);


    //[Constructor] You can add your own custom stuff here..
    window_ = 0;
    mapping_delegate_ = 0;
    //[/Constructor]
}

GlobalSettingsComponent::~GlobalSettingsComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    midi_group = nullptr;
    ok = nullptr;
    midi_notes = nullptr;
    midi_pitchbend = nullptr;
    min_data_decimation_group = nullptr;
    data_decimation = nullptr;
    active_channel_label = nullptr;
    active_channel = nullptr;
    min_channel_label = nullptr;
    min_channel = nullptr;
    max_channel_label = nullptr;
    max_channel = nullptr;
    midi_hires_velocity = nullptr;
    pitchbend_up_label = nullptr;
    pitchbend_down_label = nullptr;
    pitchbend_up = nullptr;
    pitchbend_down = nullptr;


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
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

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
    pitchbend_up_label->setBounds (16, 247, 224, 24);
    pitchbend_down_label->setBounds (16, 303, 224, 24);
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
        updateSettings();
        //[/UserComboBoxCode_active_channel]
    }
    else if (comboBoxThatHasChanged == min_channel)
    {
        //[UserComboBoxCode_min_channel] -- add your combo box handling code here..
        updateSettings();
        //[/UserComboBoxCode_min_channel]
    }
    else if (comboBoxThatHasChanged == max_channel)
    {
        //[UserComboBoxCode_max_channel] -- add your combo box handling code here..
        updateSettings();
        //[/UserComboBoxCode_max_channel]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="GlobalSettingsComponent"
                 componentName="GlobalSettings" parentClasses="public Component, public MessageListener"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="256"
                 initialHeight="410">
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
         virtualName="" explicitFocusOrder="0" pos="16 247 224 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Pitch bend up range (semis)"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
  <LABEL name="pitchbend down label" id="c0c5425a49b2d366" memberName="pitchbend_down_label"
         virtualName="" explicitFocusOrder="0" pos="16 303 224 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Pitch bend down range (semis)"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
  <SLIDER name="pitchbend up slider" id="ad6f6ba17280bd77" memberName="pitchbend_up"
          virtualName="" explicitFocusOrder="0" pos="16 272 224 24" thumbcol="ff8a8a8a"
          rotarysliderfill="fff0ffff" textboxtext="ffeeeeee" textboxbkgd="ffffff"
          textboxhighlight="40000000" textboxoutline="66ffffff" min="0"
          max="48" int="0.10000000000000000555" style="LinearBar" textBoxPos="TextBoxLeft"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="pitchbend down slider" id="44141a9a8275e444" memberName="pitchbend_down"
          virtualName="" explicitFocusOrder="0" pos="16 328 224 24" thumbcol="ff8a8a8a"
          rotarysliderfill="fff0ffff" textboxtext="ffeeeeee" textboxbkgd="ffffff"
          textboxhighlight="40000000" textboxoutline="66ffffff" min="0"
          max="48" int="0.10000000000000000555" style="LinearBar" textBoxPos="TextBoxLeft"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
