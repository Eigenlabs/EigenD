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

#include "ScenePanel.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ScenePanel::ScenePanel ()
{
    setName ("scenePanel");
    addAndMakeVisible (clipLabel = new Label ("clipLabel",
                                              String::empty));
    clipLabel->setFont (Font (15.00f, Font::plain));
    clipLabel->setJustificationType (Justification::centredLeft);
    clipLabel->setEditable (false, false, false);
    clipLabel->setColour (TextEditor::textColourId, Colours::black);
    clipLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (activeButton = new ToggleButton ("activeButton"));
    activeButton->setButtonText (TRANS("Active"));
    activeButton->addListener (this);

    addAndMakeVisible (playLabel = new Label ("playLabel",
                                              TRANS("Play for")));
    playLabel->setFont (Font (15.00f, Font::plain));
    playLabel->setJustificationType (Justification::centredLeft);
    playLabel->setEditable (false, false, false);
    playLabel->setColour (TextEditor::textColourId, Colours::black);
    playLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (playNumCombo = new ComboBox ("playNumCombo"));
    playNumCombo->setEditableText (true);
    playNumCombo->setJustificationType (Justification::centredLeft);
    playNumCombo->setTextWhenNothingSelected (TRANS("1"));
    playNumCombo->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    playNumCombo->addItem (TRANS("1"), 1);
    playNumCombo->addItem (TRANS("2"), 2);
    playNumCombo->addItem (TRANS("3"), 3);
    playNumCombo->addItem (TRANS("4"), 4);
    playNumCombo->addItem (TRANS("5"), 5);
    playNumCombo->addItem (TRANS("6"), 6);
    playNumCombo->addItem (TRANS("7"), 7);
    playNumCombo->addItem (TRANS("8"), 8);
    playNumCombo->addListener (this);

    addAndMakeVisible (playCombo = new ComboBox ("playCombo"));
    playCombo->setEditableText (false);
    playCombo->setJustificationType (Justification::centredLeft);
    playCombo->setTextWhenNothingSelected (TRANS("Loops"));
    playCombo->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    playCombo->addItem (TRANS("Loops"), 1);
    playCombo->addListener (this);


    //[UserPreSize]
    activeButton->setEnabled(false);
    playLabel->setEnabled(false);
    playNumCombo->setEnabled(false);
    playCombo->setEnabled(false);

    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ScenePanel::~ScenePanel()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    clipLabel = nullptr;
    activeButton = nullptr;
    playLabel = nullptr;
    playNumCombo = nullptr;
    playCombo = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ScenePanel::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ScenePanel::resized()
{
    clipLabel->setBounds (proportionOfWidth (-0.0029f), 4, 150, 24);
    activeButton->setBounds (proportionOfWidth (0.0000f), 24, 150, 24);
    playLabel->setBounds (proportionOfWidth (0.0000f), 48, 60, 20);
    playNumCombo->setBounds ((proportionOfWidth (0.0000f)) + 52, (48) + 0, 44, 20);
    playCombo->setBounds ((proportionOfWidth (0.0000f)) + 100, (48) + 0, 67, 20);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ScenePanel::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == activeButton)
    {
        //[UserButtonCode_activeButton] -- add your button handler code here..
        //[/UserButtonCode_activeButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void ScenePanel::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == playNumCombo)
    {
        //[UserComboBoxCode_playNumCombo] -- add your combo box handling code here..
        //[/UserComboBoxCode_playNumCombo]
    }
    else if (comboBoxThatHasChanged == playCombo)
    {
        //[UserComboBoxCode_playCombo] -- add your combo box handling code here..
        //[/UserComboBoxCode_playCombo]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ScenePanel::setClip(Clip* clip)
{
   // set the values of all controls from the clip
    clip_=clip;
    if(clip!=0)
    {
        clipLabel->setText(clip->getName(),dontSendNotification);
        // XXX and the others
        activeButton->setEnabled(true);
        playLabel->setEnabled(true);
        playNumCombo->setEnabled(true);
        playCombo->setEnabled(true);
    }
    else
    {
        clipLabel->setText(String::empty,dontSendNotification);
        // XXX and the others
        activeButton->setEnabled(false);
        playLabel->setEnabled(false);
        playNumCombo->setEnabled(false);
        playCombo->setEnabled(false);
    }
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ScenePanel" componentName="scenePanel"
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffffff"/>
  <LABEL name="clipLabel" id="5f64023da542303" memberName="clipLabel"
         virtualName="" explicitFocusOrder="0" pos="-0.236% 4 150 24"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="activeButton" id="49fdb5bca134f89d" memberName="activeButton"
                virtualName="" explicitFocusOrder="0" pos="0% 24 150 24" buttonText="Active"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <LABEL name="playLabel" id="81c490038fa8edbc" memberName="playLabel"
         virtualName="" explicitFocusOrder="0" pos="0% 48 60 20" edTextCol="ff000000"
         edBkgCol="0" labelText="Play for" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <COMBOBOX name="playNumCombo" id="6351eac24852bd5e" memberName="playNumCombo"
            virtualName="" explicitFocusOrder="0" pos="52 0 44 20" posRelativeX="81c490038fa8edbc"
            posRelativeY="81c490038fa8edbc" editable="1" layout="33" items="1&#10;2&#10;3&#10;4&#10;5&#10;6&#10;7&#10;8"
            textWhenNonSelected="1" textWhenNoItems="(no choices)"/>
  <COMBOBOX name="playCombo" id="3a9ede16afde5ca3" memberName="playCombo"
            virtualName="" explicitFocusOrder="0" pos="100 0 67 20" posRelativeX="81c490038fa8edbc"
            posRelativeY="81c490038fa8edbc" editable="0" layout="33" items="Loops"
            textWhenNonSelected="Loops" textWhenNoItems="(no choices)"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
