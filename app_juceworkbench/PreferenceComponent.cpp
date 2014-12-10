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
 Copyright 2012-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include "PreferenceComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
PreferenceComponent::PreferenceComponent (PreferenceManager* manager)
{
    addAndMakeVisible (okButton = new TextButton ("okButton"));
    okButton->setButtonText (TRANS("OK"));
    okButton->addListener (this);
    okButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (cancelButton = new TextButton ("cancelButton"));
    cancelButton->setButtonText (TRANS("Cancel"));
    cancelButton->addListener (this);
    cancelButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (selectOnExpand = new ToggleButton ("selectOnExpand"));
    selectOnExpand->setButtonText (TRANS("select agent when opened"));
    selectOnExpand->addListener (this);
    selectOnExpand->setColour (ToggleButton::textColourId, Colour (0xfffdfdfd));

    addAndMakeVisible (enableMouseWheelZoom = new ToggleButton ("enableMouseWheelZoom"));
    enableMouseWheelZoom->setButtonText (TRANS("enable mouse wheel zooming"));
    enableMouseWheelZoom->addListener (this);
    enableMouseWheelZoom->setColour (ToggleButton::textColourId, Colours::white);


    //[UserPreSize]
    okPressed=false;
    manager_=manager;
    selectOnExpand->setToggleState(manager_->getProperty("selectOnExpand",true),false);
    enableMouseWheelZoom->setToggleState(manager_->getProperty("enableMouseWheelZoom",true),false);
    //[/UserPreSize]

    setSize (300, 200);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

PreferenceComponent::~PreferenceComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    okButton = nullptr;
    cancelButton = nullptr;
    selectOnExpand = nullptr;
    enableMouseWheelZoom = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void PreferenceComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff161616),
                                       96.0f, 0.0f,
                                       Colour (0xff898989),
                                       96.0f, 200.0f,
                                       false));
    g.fillRect (-8, -8, 312, 208);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void PreferenceComponent::resized()
{
    okButton->setBounds (56, 144, 80, 24);
    cancelButton->setBounds (176, 144, 70, 24);
    selectOnExpand->setBounds (56, 16, 192, 24);
    enableMouseWheelZoom->setBounds (56, 48, 216, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void PreferenceComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == okButton)
    {
        //[UserButtonCode_okButton] -- add your button handler code here..
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            okPressed=true;
            dw->exitModalState(0);
        }

        //[/UserButtonCode_okButton]
    }
    else if (buttonThatWasClicked == cancelButton)
    {
        //[UserButtonCode_cancelButton] -- add your button handler code here..
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            dw->exitModalState(0);
        }

        //[/UserButtonCode_cancelButton]
    }
    else if (buttonThatWasClicked == selectOnExpand)
    {
        //[UserButtonCode_selectOnExpand] -- add your button handler code here..
        //[/UserButtonCode_selectOnExpand]
    }
    else if (buttonThatWasClicked == enableMouseWheelZoom)
    {
        //[UserButtonCode_enableMouseWheelZoom] -- add your button handler code here..
        //[/UserButtonCode_enableMouseWheelZoom]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

bool PreferenceComponent::keyPressed (const KeyPress& key)
{
    //[UserCode_keyPressed] -- Add your code here...
    return false;  // Return true if your handler uses this key event, or false to allow it to be passed-on.
    //[/UserCode_keyPressed]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
bool PreferenceComponent::getValue(String preferenceName)
{
    for (int i=getNumChildComponents();--i>=0;)
    {
          if(getChildComponent(i)->getName()==preferenceName)
          {
                Button* b=dynamic_cast<Button*>(getChildComponent(i));
                if(b!=0)
                {
                    return b->getToggleState();
                }
          }
    }
    return false;
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="PreferenceComponent" componentName=""
                 parentClasses="public Component" constructorParams="PreferenceManager* manager"
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="300" initialHeight="200">
  <METHODS>
    <METHOD name="keyPressed (const KeyPress&amp; key)"/>
  </METHODS>
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="-8 -8 312 208" fill="linear: 96 0, 96 200, 0=ff161616, 1=ff898989"
          hasStroke="0"/>
  </BACKGROUND>
  <TEXTBUTTON name="okButton" id="db06151ac7f364db" memberName="okButton" virtualName=""
              explicitFocusOrder="0" pos="56 144 80 24" bgColOff="ffaeaeae"
              buttonText="OK" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="cancelButton" id="64a84dc445c9601" memberName="cancelButton"
              virtualName="" explicitFocusOrder="0" pos="176 144 70 24" bgColOff="ffaeaeae"
              buttonText="Cancel" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TOGGLEBUTTON name="selectOnExpand" id="15e112411352b297" memberName="selectOnExpand"
                virtualName="" explicitFocusOrder="0" pos="56 16 192 24" txtcol="fffdfdfd"
                buttonText="select agent when opened" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="enableMouseWheelZoom" id="c8507e8ddf26e5e9" memberName="enableMouseWheelZoom"
                virtualName="" explicitFocusOrder="0" pos="56 48 216 24" txtcol="ffffffff"
                buttonText="enable mouse wheel zooming" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
