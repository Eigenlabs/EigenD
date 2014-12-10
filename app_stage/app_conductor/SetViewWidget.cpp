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

#include "SetViewWidget.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
SetViewWidget::SetViewWidget (SetBackend* backend)
    : backend_(backend)
{
    addAndMakeVisible (removeButton2 = new TextButton ("removeButton"));
    removeButton2->setButtonText (TRANS("Remove"));
    removeButton2->addListener (this);
    removeButton2->setColour (TextButton::buttonColourId, Colour (0xffbababa));

    addAndMakeVisible (playButton = new TextButton ("playButton"));
    playButton->setButtonText (TRANS("Play"));
    playButton->addListener (this);
    playButton->setColour (TextButton::buttonColourId, Colour (0xffababab));

    addAndMakeVisible (arrangementLabel = new Label ("arrangementLabel",
                                                     String::empty));
    arrangementLabel->setFont (Font (15.00f, Font::plain));
    arrangementLabel->setJustificationType (Justification::centredLeft);
    arrangementLabel->setEditable (false, false, false);
    arrangementLabel->setColour (TextEditor::textColourId, Colours::black);
    arrangementLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (setList = new SetList (backend_));
    setList->setName ("setList");


    //[UserPreSize]
    playButton->setEnabled(false);
    removeButton2->setEnabled(false);

    //[/UserPreSize]

    setSize (700, 300);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

SetViewWidget::~SetViewWidget()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    removeButton2 = nullptr;
    playButton = nullptr;
    arrangementLabel = nullptr;
    setList = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SetViewWidget::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setColour (Colour (0xffababab));
    g.drawRoundedRectangle (4.0f, 4.0f, static_cast<float> (getWidth() - 8), static_cast<float> (getHeight() - 8), 10.000f, 1.000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SetViewWidget::resized()
{
    removeButton2->setBounds (16, getHeight() - 16 - 24, 104, 24);
    playButton->setBounds (getWidth() - 16 - 90, getHeight() - 16 - 24, 90, 24);
    arrangementLabel->setBounds (68, 12, 112, 16);
    setList->setBounds (8, 8, getWidth() - 20, getHeight() - 64);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void SetViewWidget::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == removeButton2)
    {
        //[UserButtonCode_removeButton2] -- add your button handler code here..
        setList->removeFromSet();
        //[/UserButtonCode_removeButton2]
    }
    else if (buttonThatWasClicked == playButton)
    {
        //[UserButtonCode_playButton] -- add your button handler code here..
        setList->play();
        //[/UserButtonCode_playButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void SetViewWidget::clipSelected(Clip* clip)
{

    if(clip==0)
    {
        playButton->setEnabled(false);
        removeButton2->setEnabled(false);
    }
    else
    {
        playButton->setEnabled(true);
        removeButton2->setEnabled(true);
    }
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="SetViewWidget" componentName=""
                 parentClasses="public Component, public ClipSelector" constructorParams="SetBackend* backend"
                 variableInitialisers="backend_(backend)" snapPixels="4" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="0" initialWidth="700"
                 initialHeight="300">
  <BACKGROUND backgroundColour="ffffffff">
    <ROUNDRECT pos="4 4 8M 8M" cornerSize="10" fill="solid: ffffff" hasStroke="1"
               stroke="1, mitered, butt" strokeColour="solid: ffababab"/>
  </BACKGROUND>
  <TEXTBUTTON name="removeButton" id="cd407349dd11df4d" memberName="removeButton2"
              virtualName="" explicitFocusOrder="0" pos="16 16Rr 104 24" bgColOff="ffbababa"
              buttonText="Remove" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="playButton" id="d16be70d6ab8561a" memberName="playButton"
              virtualName="" explicitFocusOrder="0" pos="16Rr 16Rr 90 24" bgColOff="ffababab"
              buttonText="Play" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="arrangementLabel" id="d9d07e59188b630b" memberName="arrangementLabel"
         virtualName="" explicitFocusOrder="0" pos="68 12 112 16" edTextCol="ff000000"
         edBkgCol="0" labelText="" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="setList" id="8fad701adec899a7" memberName="setList" virtualName=""
                    explicitFocusOrder="0" pos="8 8 20M 64M" class="SetList" params="backend_"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
