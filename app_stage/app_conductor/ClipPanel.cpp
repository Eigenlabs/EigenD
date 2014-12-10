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

//[Headers] You can add your own extra header files here...
/*
 Copyright 2010-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/
//[/Headers]

#include "ClipPanel.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ClipPanel::ClipPanel ()
{
    setName ("clipPanel");
    addAndMakeVisible (activeButton = new ToggleButton ("activeButton"));
    activeButton->setButtonText (TRANS("Active"));
    activeButton->addListener (this);

    addAndMakeVisible (playModeGroup = new GroupComponent ("playModeGroup",
                                                           TRANS("Play mode")));

    addAndMakeVisible (loopButton = new ToggleButton ("loopButton"));
    loopButton->setButtonText (TRANS("loop with scene"));
    loopButton->addListener (this);

    addAndMakeVisible (playOnceButton = new ToggleButton ("playOnceButton"));
    playOnceButton->setButtonText (TRANS("play once"));
    playOnceButton->addListener (this);

    addAndMakeVisible (clipLabel = new Label ("clipLabel",
                                              String::empty));
    clipLabel->setFont (Font (15.00f, Font::plain));
    clipLabel->setJustificationType (Justification::centredLeft);
    clipLabel->setEditable (false, false, false);
    clipLabel->setColour (TextEditor::textColourId, Colours::black);
    clipLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (timingUnitCombo = new ComboBox ("timingUnitCombo"));
    timingUnitCombo->setEditableText (false);
    timingUnitCombo->setJustificationType (Justification::centredLeft);
    timingUnitCombo->setTextWhenNothingSelected (TRANS("Beats"));
    timingUnitCombo->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    timingUnitCombo->addItem (TRANS("Beats"), 1);
    timingUnitCombo->addItem (TRANS("Bars"), 2);
    timingUnitCombo->addListener (this);

    addAndMakeVisible (timingCombo = new ComboBox ("timingCombo"));
    timingCombo->setEditableText (true);
    timingCombo->setJustificationType (Justification::centredLeft);
    timingCombo->setTextWhenNothingSelected (TRANS("2"));
    timingCombo->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    timingCombo->addItem (TRANS("0"), 1);
    timingCombo->addItem (TRANS("1"), 2);
    timingCombo->addItem (TRANS("2"), 3);
    timingCombo->addItem (TRANS("3"), 4);
    timingCombo->addItem (TRANS("4"), 5);
    timingCombo->addItem (TRANS("5"), 6);
    timingCombo->addItem (TRANS("6"), 7);
    timingCombo->addItem (TRANS("7"), 8);
    timingCombo->addItem (TRANS("8"), 9);
    timingCombo->addListener (this);

    addAndMakeVisible (startLabel = new Label ("startLabel",
                                               TRANS("Start after")));
    startLabel->setFont (Font (15.00f, Font::plain));
    startLabel->setJustificationType (Justification::centredLeft);
    startLabel->setEditable (false, false, false);
    startLabel->setColour (TextEditor::textColourId, Colours::black);
    startLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (timingCombo2 = new ComboBox ("timingCombo"));
    timingCombo2->setEditableText (true);
    timingCombo2->setJustificationType (Justification::centredLeft);
    timingCombo2->setTextWhenNothingSelected (TRANS("2"));
    timingCombo2->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    timingCombo2->addItem (TRANS("0"), 1);
    timingCombo2->addItem (TRANS("1"), 2);
    timingCombo2->addItem (TRANS("2"), 3);
    timingCombo2->addItem (TRANS("3"), 4);
    timingCombo2->addItem (TRANS("4"), 5);
    timingCombo2->addItem (TRANS("5"), 6);
    timingCombo2->addItem (TRANS("6"), 7);
    timingCombo2->addItem (TRANS("7"), 8);
    timingCombo2->addItem (TRANS("8"), 9);
    timingCombo2->addListener (this);

    addAndMakeVisible (timingUnitCombo2 = new ComboBox ("timingUnitCombo"));
    timingUnitCombo2->setEditableText (false);
    timingUnitCombo2->setJustificationType (Justification::centredLeft);
    timingUnitCombo2->setTextWhenNothingSelected (TRANS("Beats"));
    timingUnitCombo2->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    timingUnitCombo2->addItem (TRANS("Beats"), 1);
    timingUnitCombo2->addItem (TRANS("Bars"), 2);
    timingUnitCombo2->addListener (this);


    //[UserPreSize]
    clip_=0;
    activeButton->setEnabled(false);
    playOnceButton->setEnabled(false);
    loopButton->setEnabled(false);
    timingCombo->setEnabled(false);
    timingCombo2->setEnabled(false);
    timingUnitCombo2->setEnabled(false);
    playModeGroup->setEnabled(false);
    startLabel->setEnabled(false);
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ClipPanel::~ClipPanel()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    activeButton = nullptr;
    playModeGroup = nullptr;
    loopButton = nullptr;
    playOnceButton = nullptr;
    clipLabel = nullptr;
    timingUnitCombo = nullptr;
    timingCombo = nullptr;
    startLabel = nullptr;
    timingCombo2 = nullptr;
    timingUnitCombo2 = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ClipPanel::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ClipPanel::resized()
{
    activeButton->setBounds (4, 24, 150, 24);
    playModeGroup->setBounds (4, 48, 146, 76);
    loopButton->setBounds ((4) + 12, (48) + 20, 150, 24);
    playOnceButton->setBounds ((4) + 12, (48) + 44, 150, 24);
    clipLabel->setBounds (4, 0, 150, 24);
    timingUnitCombo->setBounds ((4) + -826, (128) + -168, 64, 20);
    timingCombo->setBounds ((4) + -878, (128) + -168, 44, 20);
    startLabel->setBounds (4, 128, 71, 16);
    timingCombo2->setBounds ((4) + 68, (128) + 0, 44, 20);
    timingUnitCombo2->setBounds ((4) + 116, (128) + 0, 64, 20);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ClipPanel::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == activeButton)
    {
        //[UserButtonCode_activeButton] -- add your button handler code here..
        //[/UserButtonCode_activeButton]
    }
    else if (buttonThatWasClicked == loopButton)
    {
        //[UserButtonCode_loopButton] -- add your button handler code here..
        //[/UserButtonCode_loopButton]
    }
    else if (buttonThatWasClicked == playOnceButton)
    {
        //[UserButtonCode_playOnceButton] -- add your button handler code here..
        //[/UserButtonCode_playOnceButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void ClipPanel::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == timingUnitCombo)
    {
        //[UserComboBoxCode_timingUnitCombo] -- add your combo box handling code here..
        //[/UserComboBoxCode_timingUnitCombo]
    }
    else if (comboBoxThatHasChanged == timingCombo)
    {
        //[UserComboBoxCode_timingCombo] -- add your combo box handling code here..
        //[/UserComboBoxCode_timingCombo]
    }
    else if (comboBoxThatHasChanged == timingCombo2)
    {
        //[UserComboBoxCode_timingCombo2] -- add your combo box handling code here..
        //[/UserComboBoxCode_timingCombo2]
    }
    else if (comboBoxThatHasChanged == timingUnitCombo2)
    {
        //[UserComboBoxCode_timingUnitCombo2] -- add your combo box handling code here..
        //[/UserComboBoxCode_timingUnitCombo2]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ClipPanel::setClip(Clip* clip)
{
   // set the values of all controls from the clip
    clip_=clip;
    if(clip!=0)
    {
        clipLabel->setText(clip->getName(),dontSendNotification);
        // XXX and the others
        activeButton->setEnabled(true);
        playOnceButton->setEnabled(true);
        loopButton->setEnabled(true);
        timingCombo->setEnabled(true);
        timingCombo2->setEnabled(true);
        timingUnitCombo2->setEnabled(true);
        playModeGroup->setEnabled(true);
        startLabel->setEnabled(true);

    }
    else
    {
        clipLabel->setText(String::empty,dontSendNotification);
        // XXX and the others
        activeButton->setEnabled(false);
        playOnceButton->setEnabled(false);
        loopButton->setEnabled(false);
        timingCombo->setEnabled(false);
        timingCombo2->setEnabled(false);
        timingUnitCombo2->setEnabled(false);
        playModeGroup->setEnabled(false);
        startLabel->setEnabled(false);

    }
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ClipPanel" componentName="clipPanel"
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="4" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffffff"/>
  <TOGGLEBUTTON name="activeButton" id="f7aea63fada448eb" memberName="activeButton"
                virtualName="" explicitFocusOrder="0" pos="4 24 150 24" buttonText="Active"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <GROUPCOMPONENT name="playModeGroup" id="e68cc6a5eb59fd7d" memberName="playModeGroup"
                  virtualName="" explicitFocusOrder="0" pos="4 48 146 76" title="Play mode"/>
  <TOGGLEBUTTON name="loopButton" id="90c490f2eb2ffc13" memberName="loopButton"
                virtualName="" explicitFocusOrder="0" pos="12 20 150 24" posRelativeX="e68cc6a5eb59fd7d"
                posRelativeY="e68cc6a5eb59fd7d" buttonText="loop with scene"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="playOnceButton" id="1b51a8be36e4fff1" memberName="playOnceButton"
                virtualName="" explicitFocusOrder="0" pos="12 44 150 24" posRelativeX="e68cc6a5eb59fd7d"
                posRelativeY="e68cc6a5eb59fd7d" buttonText="play once" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <LABEL name="clipLabel" id="5128d25c4198a2de" memberName="clipLabel"
         virtualName="" explicitFocusOrder="0" pos="4 0 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <COMBOBOX name="timingUnitCombo" id="4663988403623bf7" memberName="timingUnitCombo"
            virtualName="" explicitFocusOrder="0" pos="-826 -168 64 20" posRelativeX="697c1d16a81c063d"
            posRelativeY="697c1d16a81c063d" editable="0" layout="33" items="Beats&#10;Bars"
            textWhenNonSelected="Beats" textWhenNoItems="(no choices)"/>
  <COMBOBOX name="timingCombo" id="cd1c86ce9ba31d7b" memberName="timingCombo"
            virtualName="" explicitFocusOrder="0" pos="-878 -168 44 20" posRelativeX="697c1d16a81c063d"
            posRelativeY="697c1d16a81c063d" editable="1" layout="33" items="0&#10;1&#10;2&#10;3&#10;4&#10;5&#10;6&#10;7&#10;8"
            textWhenNonSelected="2" textWhenNoItems="(no choices)"/>
  <LABEL name="startLabel" id="697c1d16a81c063d" memberName="startLabel"
         virtualName="" explicitFocusOrder="0" pos="4 128 71 16" edTextCol="ff000000"
         edBkgCol="0" labelText="Start after" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <COMBOBOX name="timingCombo" id="a82c2755948fc476" memberName="timingCombo2"
            virtualName="" explicitFocusOrder="0" pos="68 0 44 20" posRelativeX="697c1d16a81c063d"
            posRelativeY="697c1d16a81c063d" editable="1" layout="33" items="0&#10;1&#10;2&#10;3&#10;4&#10;5&#10;6&#10;7&#10;8"
            textWhenNonSelected="2" textWhenNoItems="(no choices)"/>
  <COMBOBOX name="timingUnitCombo" id="417cec92b1b39854" memberName="timingUnitCombo2"
            virtualName="" explicitFocusOrder="0" pos="116 0 64 20" posRelativeX="697c1d16a81c063d"
            posRelativeY="697c1d16a81c063d" editable="0" layout="33" items="Beats&#10;Bars"
            textWhenNonSelected="Beats" textWhenNoItems="(no choices)"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
