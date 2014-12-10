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

#include "NetworkPreferencesComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
NetworkPreferencesComponent::NetworkPreferencesComponent (StagePreferences* preferences)
    : preferences_(preferences)
{
    setName ("NetworkPreferences");
    addAndMakeVisible (autoToggleButton = new ToggleButton ("auto toggle button"));
    autoToggleButton->setButtonText (TRANS("auto connect when single eigenD"));
    autoToggleButton->addListener (this);
    autoToggleButton->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (288, 240);


    //[Constructor] You can add your own custom stuff here..
    autoToggleButton->setWantsKeyboardFocus(false);
    autoToggleButton->setToggleState(preferences_->getBoolValue("connectSingleHost", true), dontSendNotification);
    //[/Constructor]
}

NetworkPreferencesComponent::~NetworkPreferencesComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    autoToggleButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void NetworkPreferencesComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
// black background just used in jucer
#if 0
    //[/UserPrePaint]

    g.fillAll (Colours::black);

    //[UserPaint] Add your own custom painting code here..
#endif
    g.fillAll (Colour(0x0));
    //[/UserPaint]
}

void NetworkPreferencesComponent::resized()
{
    autoToggleButton->setBounds (0, 0, 240, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void NetworkPreferencesComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == autoToggleButton)
    {
        //[UserButtonCode_autoToggleButton] -- add your button handler code here..
        //[/UserButtonCode_autoToggleButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void NetworkPreferencesComponent::applyChanges()
{
    bool autoConnect = autoToggleButton->getToggleState();
    preferences_->setValue("connectSingleHost", autoConnect);
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="NetworkPreferencesComponent"
                 componentName="NetworkPreferences" parentClasses="public Component"
                 constructorParams="StagePreferences* preferences" variableInitialisers="preferences_(preferences)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="288" initialHeight="240">
  <BACKGROUND backgroundColour="ff000000"/>
  <TOGGLEBUTTON name="auto toggle button" id="47a03168a3b8035" memberName="autoToggleButton"
                virtualName="" explicitFocusOrder="0" pos="0 0 240 24" txtcol="ffeeeeee"
                buttonText="auto connect when single eigenD" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
