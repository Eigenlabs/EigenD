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

#include "DeleteDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
#include "DialogFrameworkComponent.h"
//[/MiscUserDefs]

//==============================================================================
DeleteDialogComponent::DeleteDialogComponent ()
{
    setName ("DeleteDialog");
    addAndMakeVisible (doNotShowAgain = new ToggleButton ("do not show again"));
    doNotShowAgain->setButtonText (TRANS("do not show this dialog again"));
    doNotShowAgain->addListener (this);
    doNotShowAgain->setColour (ToggleButton::textColourId, Colour (0xff999999));

    addAndMakeVisible (confirm = new Label ("confirm",
                                            TRANS("Are you sure?")));
    confirm->setFont (Font (15.00f, Font::plain));
    confirm->setJustificationType (Justification::centredLeft);
    confirm->setEditable (false, false, false);
    confirm->setColour (Label::textColourId, Colour (0xffeeeeee));
    confirm->setColour (TextEditor::textColourId, Colours::black);
    confirm->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (288, 72);


    //[Constructor] You can add your own custom stuff here..

    doNotShowAgain->setExplicitFocusOrder(3);
    //[/Constructor]
}

DeleteDialogComponent::~DeleteDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    doNotShowAgain = nullptr;
    confirm = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void DeleteDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void DeleteDialogComponent::resized()
{
    doNotShowAgain->setBounds (0, 48, getWidth() - 0, 24);
    confirm->setBounds (0, 8, getWidth() - 0, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void DeleteDialogComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == doNotShowAgain)
    {
        //[UserButtonCode_doNotShowAgain] -- add your button handler code here..
        //[/UserButtonCode_doNotShowAgain]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void DeleteDialogComponent::setText(const String& text)
{
    confirm->setText(text, dontSendNotification);
}

bool DeleteDialogComponent::getDoNotShowAgain()
{
    return doNotShowAgain->getToggleState();
}

void DeleteDialogComponent::setFocusOrder(DialogFrameworkComponent* framework)
{
    // only OK button can be triggered with return
    framework->getOKButton()->setExplicitFocusOrder(1);
    framework->getCancelButton()->setWantsKeyboardFocus(false);
    doNotShowAgain->setWantsKeyboardFocus(false);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="DeleteDialogComponent" componentName="DeleteDialog"
                 parentClasses="public Component, public ComponentBoundsConstrainer"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="288"
                 initialHeight="72">
  <BACKGROUND backgroundColour="0"/>
  <TOGGLEBUTTON name="do not show again" id="f9c0ae2557dae5b7" memberName="doNotShowAgain"
                virtualName="" explicitFocusOrder="0" pos="0 48 0M 24" txtcol="ff999999"
                buttonText="do not show this dialog again" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <LABEL name="confirm" id="5649230afc55a8d8" memberName="confirm" virtualName=""
         explicitFocusOrder="0" pos="0 8 0M 24" textCol="ffeeeeee" edTextCol="ff000000"
         edBkgCol="0" labelText="Are you sure?" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
