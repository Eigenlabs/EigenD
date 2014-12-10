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

#include "ViewPreferencesComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ViewPreferencesComponent::ViewPreferencesComponent (StagePreferences* preferences)
    : preferences_(preferences)
{
    setName ("ViewPreferences");
    addAndMakeVisible (orientationGroup = new GroupComponent ("orientation group",
                                                              TRANS("display orientation (restart application to activate)")));
    orientationGroup->setColour (GroupComponent::outlineColourId, Colour (0x66ffffff));
    orientationGroup->setColour (GroupComponent::textColourId, Colours::white);

    addAndMakeVisible (greyToggleButton = new ToggleButton ("grey toggle button"));
    greyToggleButton->setButtonText (TRANS("grey outside canvas"));
    greyToggleButton->addListener (this);
    greyToggleButton->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (tabDeleteToggleButton = new ToggleButton ("tab delete toggle button"));
    tabDeleteToggleButton->setButtonText (TRANS("tab delete buttons"));
    tabDeleteToggleButton->addListener (this);
    tabDeleteToggleButton->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (allTabsToggleButton = new ToggleButton ("all tabs toggle button"));
    allTabsToggleButton->setButtonText (TRANS("show all tabs when editing"));
    allTabsToggleButton->addListener (this);
    allTabsToggleButton->setColour (ToggleButton::textColourId, Colour (0xffcccccc));

    addAndMakeVisible (uprightButton = new ToggleButton ("upright button"));
    uprightButton->setButtonText (TRANS("upright"));
    uprightButton->setRadioGroupId (1);
    uprightButton->addListener (this);
    uprightButton->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (rotatedAntiClockwiseButton = new ToggleButton ("rotatedAntiClockwise button"));
    rotatedAntiClockwiseButton->setButtonText (TRANS("rotated anti-clockwise"));
    rotatedAntiClockwiseButton->setRadioGroupId (1);
    rotatedAntiClockwiseButton->addListener (this);
    rotatedAntiClockwiseButton->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (upsideDownButton = new ToggleButton ("upsideDown button"));
    upsideDownButton->setButtonText (TRANS("upside down"));
    upsideDownButton->setRadioGroupId (1);
    upsideDownButton->addListener (this);
    upsideDownButton->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (rotatedClockwiseButton = new ToggleButton ("rotatedClockwise button"));
    rotatedClockwiseButton->setButtonText (TRANS("rotated clockwise"));
    rotatedClockwiseButton->setRadioGroupId (1);
    rotatedClockwiseButton->addListener (this);
    rotatedClockwiseButton->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (autoDetectButton = new ToggleButton ("autoDetect button"));
    autoDetectButton->setButtonText (TRANS("auto detect"));
    autoDetectButton->setRadioGroupId (1);
    autoDetectButton->addListener (this);
    autoDetectButton->setToggleState (true, dontSendNotification);
    autoDetectButton->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));


    //[UserPreSize]
    greyToggleButton->setWantsKeyboardFocus(false);
    tabDeleteToggleButton->setWantsKeyboardFocus(false);
    allTabsToggleButton->setWantsKeyboardFocus(false);
    uprightButton->setWantsKeyboardFocus(false);
    rotatedAntiClockwiseButton->setWantsKeyboardFocus(false);
    upsideDownButton->setWantsKeyboardFocus(false);
    rotatedClockwiseButton->setWantsKeyboardFocus(false);
    autoDetectButton->setWantsKeyboardFocus(false);
#if STAGE_BUILD==IOS
	tabDeleteToggleButton->setVisible(false);
	allTabsToggleButton->setVisible(false);
#endif
#if STAGE_BUILD==DESKTOP
    orientationGroup->setVisible(false);
	uprightButton->setVisible(false);
	rotatedAntiClockwiseButton->setVisible(false);
	upsideDownButton->setVisible(false);
	rotatedClockwiseButton->setVisible(false);
	autoDetectButton->setVisible(false);
#endif
    //[/UserPreSize]

    setSize (288, 240);


    //[Constructor] You can add your own custom stuff here..
    allTabsToggleButton->setWantsKeyboardFocus(false);
    allTabsToggleButton->setToggleState(preferences_->getBoolValue("allTabsEditMode", true), dontSendNotification);
    //[/Constructor]
}

ViewPreferencesComponent::~ViewPreferencesComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    orientationGroup = nullptr;
    greyToggleButton = nullptr;
    tabDeleteToggleButton = nullptr;
    allTabsToggleButton = nullptr;
    uprightButton = nullptr;
    rotatedAntiClockwiseButton = nullptr;
    upsideDownButton = nullptr;
    rotatedClockwiseButton = nullptr;
    autoDetectButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ViewPreferencesComponent::paint (Graphics& g)
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

void ViewPreferencesComponent::resized()
{
    orientationGroup->setBounds (0, 37, getWidth() - 0, 160);
    greyToggleButton->setBounds (0, 0, 240, 24);
    tabDeleteToggleButton->setBounds (0, 32, 240, 24);
    allTabsToggleButton->setBounds (0, 64, 192, 24);
    uprightButton->setBounds (16, 85, 240, 24);
    rotatedAntiClockwiseButton->setBounds (16, 157, 240, 24);
    upsideDownButton->setBounds (16, 109, 240, 24);
    rotatedClockwiseButton->setBounds (16, 133, 240, 24);
    autoDetectButton->setBounds (16, 61, 240, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ViewPreferencesComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == greyToggleButton)
    {
        //[UserButtonCode_greyToggleButton] -- add your button handler code here..
        //[/UserButtonCode_greyToggleButton]
    }
    else if (buttonThatWasClicked == tabDeleteToggleButton)
    {
        //[UserButtonCode_tabDeleteToggleButton] -- add your button handler code here..
        //[/UserButtonCode_tabDeleteToggleButton]
    }
    else if (buttonThatWasClicked == allTabsToggleButton)
    {
        //[UserButtonCode_allTabsToggleButton] -- add your button handler code here..
        //[/UserButtonCode_allTabsToggleButton]
    }
    else if (buttonThatWasClicked == uprightButton)
    {
        //[UserButtonCode_uprightButton] -- add your button handler code here..
        //[/UserButtonCode_uprightButton]
    }
    else if (buttonThatWasClicked == rotatedAntiClockwiseButton)
    {
        //[UserButtonCode_rotatedAntiClockwiseButton] -- add your button handler code here..
        //[/UserButtonCode_rotatedAntiClockwiseButton]
    }
    else if (buttonThatWasClicked == upsideDownButton)
    {
        //[UserButtonCode_upsideDownButton] -- add your button handler code here..
        //[/UserButtonCode_upsideDownButton]
    }
    else if (buttonThatWasClicked == rotatedClockwiseButton)
    {
        //[UserButtonCode_rotatedClockwiseButton] -- add your button handler code here..
        //[/UserButtonCode_rotatedClockwiseButton]
    }
    else if (buttonThatWasClicked == autoDetectButton)
    {
        //[UserButtonCode_autoDetectButton] -- add your button handler code here..
        //[/UserButtonCode_autoDetectButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ViewPreferencesComponent::setTabbedComponent (WidgetTabbedComponent* tabbedComponent)
{
    tabbedComponent_ = tabbedComponent;
    greyToggleButton->setToggleState(tabbedComponent_->getGreyOutsideCanvas(), dontSendNotification);
    tabDeleteToggleButton->setToggleState(tabbedComponent_->getShowCloseButtons(), dontSendNotification);

    switch(preferences_->getIntValue("orientation", Desktop::allOrientations))
    {
        case Desktop::allOrientations:
            autoDetectButton->setToggleState(true, dontSendNotification);
            break;
        case Desktop::upright:
            uprightButton->setToggleState(true, dontSendNotification);
            break;
        case Desktop::upsideDown:
            upsideDownButton->setToggleState(true, dontSendNotification);
            break;
        case Desktop::rotatedClockwise:
            rotatedClockwiseButton->setToggleState(true, dontSendNotification);
            break;
        case Desktop::rotatedAntiClockwise:
            rotatedAntiClockwiseButton->setToggleState(true, dontSendNotification);
            break;
    }
}

void ViewPreferencesComponent::applyChanges()
{
    tabbedComponent_->setGreyOutsideCanvas(greyToggleButton->getToggleState());
    tabbedComponent_->setShowCloseButtons(tabDeleteToggleButton->getToggleState());

    bool allTabsEditMode = allTabsToggleButton->getToggleState();
    preferences_->setValue("allTabsEditMode", allTabsEditMode);
    preferences_->saveIfNeeded();
    tabbedComponent_->updateShowAllTabs();

    tabbedComponent_->updateAddCloseButtons();

    if(autoDetectButton->getToggleState())
    {
        tabbedComponent_->setDisplayOrientation(Desktop::allOrientations);
    }
    if(uprightButton->getToggleState())
    {
        tabbedComponent_->setDisplayOrientation(Desktop::upright);
    }
    if(upsideDownButton->getToggleState())
    {
        tabbedComponent_->setDisplayOrientation(Desktop::upsideDown);
    }
    if(rotatedClockwiseButton->getToggleState())
    {
        tabbedComponent_->setDisplayOrientation(Desktop::rotatedClockwise);
    }
    if(rotatedAntiClockwiseButton->getToggleState())
    {
        tabbedComponent_->setDisplayOrientation(Desktop::rotatedAntiClockwise);
    }
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ViewPreferencesComponent"
                 componentName="ViewPreferences" parentClasses="public Component"
                 constructorParams="StagePreferences* preferences" variableInitialisers="preferences_(preferences)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="288" initialHeight="240">
  <BACKGROUND backgroundColour="ff000000"/>
  <GROUPCOMPONENT name="orientation group" id="e2750625731db8e0" memberName="orientationGroup"
                  virtualName="" explicitFocusOrder="0" pos="0 37 0M 160" outlinecol="66ffffff"
                  textcol="ffffffff" title="display orientation (restart application to activate)"/>
  <TOGGLEBUTTON name="grey toggle button" id="47a03168a3b8035" memberName="greyToggleButton"
                virtualName="" explicitFocusOrder="0" pos="0 0 240 24" txtcol="ffeeeeee"
                buttonText="grey outside canvas" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="tab delete toggle button" id="d85cd5cb71fbb3bd" memberName="tabDeleteToggleButton"
                virtualName="" explicitFocusOrder="0" pos="0 32 240 24" txtcol="ffeeeeee"
                buttonText="tab delete buttons" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="all tabs toggle button" id="c2eb01a76c4f1890" memberName="allTabsToggleButton"
                virtualName="" explicitFocusOrder="0" pos="0 64 192 24" posRelativeY="26e7bc7cc27b72cf"
                txtcol="ffcccccc" buttonText="show all tabs when editing" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="upright button" id="6276f1a1e3fe91d9" memberName="uprightButton"
                virtualName="" explicitFocusOrder="0" pos="16 85 240 24" txtcol="ffeeeeee"
                buttonText="upright" connectedEdges="0" needsCallback="1" radioGroupId="1"
                state="0"/>
  <TOGGLEBUTTON name="rotatedAntiClockwise button" id="391e86a00a8b8d6f" memberName="rotatedAntiClockwiseButton"
                virtualName="" explicitFocusOrder="0" pos="16 157 240 24" txtcol="ffeeeeee"
                buttonText="rotated anti-clockwise" connectedEdges="0" needsCallback="1"
                radioGroupId="1" state="0"/>
  <TOGGLEBUTTON name="upsideDown button" id="fa03b9bb312e77b" memberName="upsideDownButton"
                virtualName="" explicitFocusOrder="0" pos="16 109 240 24" txtcol="ffeeeeee"
                buttonText="upside down" connectedEdges="0" needsCallback="1"
                radioGroupId="1" state="0"/>
  <TOGGLEBUTTON name="rotatedClockwise button" id="705b591cfa51ac8" memberName="rotatedClockwiseButton"
                virtualName="" explicitFocusOrder="0" pos="16 133 240 24" txtcol="ffeeeeee"
                buttonText="rotated clockwise" connectedEdges="0" needsCallback="1"
                radioGroupId="1" state="0"/>
  <TOGGLEBUTTON name="autoDetect button" id="2dacb492b79caf2" memberName="autoDetectButton"
                virtualName="" explicitFocusOrder="0" pos="16 61 240 24" txtcol="ffeeeeee"
                buttonText="auto detect" connectedEdges="0" needsCallback="1"
                radioGroupId="1" state="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
