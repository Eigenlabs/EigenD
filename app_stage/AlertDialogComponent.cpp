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

#include "AlertDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
AlertDialogComponent::AlertDialogComponent ()
{
    setName ("AlertDialog");
    addAndMakeVisible (messageLabel = new Label ("message label",
                                                 String::empty));
    messageLabel->setFont (Font (15.00f, Font::plain));
    messageLabel->setJustificationType (Justification::centred);
    messageLabel->setEditable (false, false, false);
    messageLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    messageLabel->setColour (TextEditor::textColourId, Colours::black);
    messageLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (288, 72);


    //[Constructor] You can add your own custom stuff here..

    //[/Constructor]
}

AlertDialogComponent::~AlertDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    messageLabel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void AlertDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AlertDialogComponent::resized()
{
    messageLabel->setBounds (8, 8, getWidth() - 16, getHeight() - 16);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void AlertDialogComponent::setText(const String& text)
{
    messageLabel->setText(text, dontSendNotification);
}

void AlertDialogComponent::setFocusOrder(DialogFrameworkComponent* framework)
{
    // only OK button can be triggered with return
    framework->getOKButton()->setExplicitFocusOrder(1);
    framework->getCancelButton()->setVisible(false);
    framework->getCancelButton()->setEnabled(false);

}

void AlertDialogComponent::openAlertDialog(const String& titleText, const String& messageText, Component* componentToCentreAround)
{
    EigenDialogWindow window("Alert Dialog Window");
    DialogFrameworkComponent framework(&window);
    AlertDialogComponent content;

    content.setText(messageText);
    window.setContentNonOwned(&framework, true);
    window.centreAroundComponent(componentToCentreAround, framework.getWidth(), framework.getHeight());
    window.setVisible(true);
    framework.setTitleText(titleText);
    framework.setContentComponent(&content);
    content.setFocusOrder(&framework);

    framework.runModalLoop();

}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AlertDialogComponent" componentName="AlertDialog"
                 parentClasses="public Component, public ComponentBoundsConstrainer"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="288"
                 initialHeight="72">
  <BACKGROUND backgroundColour="0"/>
  <LABEL name="message label" id="5649230afc55a8d8" memberName="messageLabel"
         virtualName="" explicitFocusOrder="0" pos="8 8 16M 16M" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
