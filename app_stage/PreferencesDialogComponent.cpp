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

#include "PreferencesDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
PreferencesDialogComponent::PreferencesDialogComponent (StagePreferences* preferences)
    : preferences_(preferences)
{
    setName ("PreferencesDialog");
    addAndMakeVisible (tabsButton = new TextButton ("tabs button"));
    tabsButton->setButtonText (TRANS("Tabs"));
    tabsButton->setConnectedEdges (Button::ConnectedOnRight);
    tabsButton->setRadioGroupId (1);
    tabsButton->addListener (this);
    tabsButton->setColour (TextButton::buttonColourId, Colour (0xffdddddd));
    tabsButton->setColour (TextButton::buttonOnColourId, Colour (0xff666666));
    tabsButton->setColour (TextButton::textColourOnId, Colours::white);

    addAndMakeVisible (networkButton = new TextButton ("network button"));
    networkButton->setButtonText (TRANS("Network"));
    networkButton->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight);
    networkButton->setRadioGroupId (1);
    networkButton->addListener (this);
    networkButton->setColour (TextButton::buttonColourId, Colour (0xffdddddd));
    networkButton->setColour (TextButton::buttonOnColourId, Colour (0xff666666));

    addAndMakeVisible (viewButton = new TextButton ("view button"));
    viewButton->setButtonText (TRANS("View"));
    viewButton->setConnectedEdges (Button::ConnectedOnLeft);
    viewButton->setRadioGroupId (1);
    viewButton->addListener (this);
    viewButton->setColour (TextButton::buttonColourId, Colour (0xffdddddd));
    viewButton->setColour (TextButton::buttonOnColourId, Colour (0xff666666));
    viewButton->setColour (TextButton::textColourOnId, Colours::black);
    viewButton->setColour (TextButton::textColourOffId, Colours::black);


    //[UserPreSize]

    // create the preferences panels
    addChildComponent (tabView_ = new TabViewPreferencesComponent(preferences_));
    addChildComponent (network_ = new NetworkPreferencesComponent(preferences_));
    addChildComponent (view_ = new ViewPreferencesComponent(preferences_));
    //[/UserPreSize]

    setSize (500, 280);


    //[Constructor] You can add your own custom stuff here..

    framework_ = 0;

    tabView_->setVisible(true);
    //[/Constructor]
}

PreferencesDialogComponent::~PreferencesDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    tabsButton = nullptr;
    networkButton = nullptr;
    viewButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    deleteAndZero(tabView_);
    deleteAndZero(network_);
    deleteAndZero(view_);
    //[/Destructor]
}

//==============================================================================
void PreferencesDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void PreferencesDialogComponent::resized()
{
    tabsButton->setBounds (0, 0, 72, 24);
    networkButton->setBounds (72, 0, 72, 24);
    viewButton->setBounds (144, 0, 72, 24);
    //[UserResized] Add your own custom resize handling here..
    tabView_->setBounds(0, 30, getWidth(), getHeight()-30);
    network_->setBounds(0, 30, getWidth(), getHeight()-30);
    view_->setBounds(0, 30, getWidth(), getHeight()-30);
    //[/UserResized]
}

void PreferencesDialogComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == tabsButton)
    {
        //[UserButtonCode_tabsButton] -- add your button handler code here..
        tabView_->setVisible(true);
        network_->setVisible(false);
        view_->setVisible(false);
        //[/UserButtonCode_tabsButton]
    }
    else if (buttonThatWasClicked == networkButton)
    {
        //[UserButtonCode_networkButton] -- add your button handler code here..
        tabView_->setVisible(false);
        network_->setVisible(true);
        view_->setVisible(false);
        //[/UserButtonCode_networkButton]
    }
    else if (buttonThatWasClicked == viewButton)
    {
        //[UserButtonCode_viewButton] -- add your button handler code here..
        tabView_->setVisible(false);
        network_->setVisible(false);
        view_->setVisible(true);
        //[/UserButtonCode_viewButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void PreferencesDialogComponent::initialize(DialogFrameworkComponent* framework)
{
    framework_ = framework;
}

void PreferencesDialogComponent::setFocusOrder(DialogFrameworkComponent* framework)
{
    // only OK button can be triggered with return
    framework->getOKButton()->setWantsKeyboardFocus(false);
    framework->getCancelButton()->setWantsKeyboardFocus(false);
}

void PreferencesDialogComponent::textEditorReturnKeyPressed(TextEditor& editor)
{
    if(framework_)
        framework_->exitModalState(1);
}

void PreferencesDialogComponent::textEditorEscapeKeyPressed(TextEditor& editor)
{
    if(framework_)
        framework_->exitModalState(0);
}


TabViewPreferencesComponent* PreferencesDialogComponent::getTabViewPreferences() { return tabView_; }
NetworkPreferencesComponent* PreferencesDialogComponent::getNetworkPreferences() { return network_; }
ViewPreferencesComponent* PreferencesDialogComponent::getViewPreferences() { return view_; }

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="PreferencesDialogComponent"
                 componentName="PreferencesDialog" parentClasses="public Component, public TextEditorListener"
                 constructorParams="StagePreferences* preferences" variableInitialisers="preferences_(preferences)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="500" initialHeight="280">
  <BACKGROUND backgroundColour="0"/>
  <TEXTBUTTON name="tabs button" id="bfd11dd78a611de0" memberName="tabsButton"
              virtualName="" explicitFocusOrder="0" pos="0 0 72 24" bgColOff="ffdddddd"
              bgColOn="ff666666" textCol="ffffffff" buttonText="Tabs" connectedEdges="2"
              needsCallback="1" radioGroupId="1"/>
  <TEXTBUTTON name="network button" id="ad4db38b89bdec3d" memberName="networkButton"
              virtualName="" explicitFocusOrder="0" pos="72 0 72 24" bgColOff="ffdddddd"
              bgColOn="ff666666" buttonText="Network" connectedEdges="3" needsCallback="1"
              radioGroupId="1"/>
  <TEXTBUTTON name="view button" id="feaddaaefbc91203" memberName="viewButton"
              virtualName="" explicitFocusOrder="0" pos="144 0 72 24" bgColOff="ffdddddd"
              bgColOn="ff666666" textCol="ff000000" textColOn="ff000000" buttonText="View"
              connectedEdges="1" needsCallback="1" radioGroupId="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
