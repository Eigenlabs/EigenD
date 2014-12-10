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

#include "DialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
DialogComponent::DialogComponent (Component* propPanel, int mode)
    : mode_(mode)
{
    addAndMakeVisible (viewport = new Viewport ("new viewport"));

    addAndMakeVisible (cancelButton = new TextButton ("cancelButton"));
    cancelButton->setButtonText (TRANS("Cancel"));
    cancelButton->addListener (this);

    addAndMakeVisible (okButton = new TextButton ("okbutton"));
    okButton->setButtonText (TRANS("OK"));
    okButton->addListener (this);


    //[UserPreSize]
    id_=String::empty;
    if (mode_==0)
    {
        okButton->setVisible(false);
    }
    else if (mode==2)
    {
        okButton->setVisible(false);
        cancelButton->setButtonText("OK");
    }

    viewport->setScrollBarThickness(16);
    viewport->setViewedComponent(propPanel);
    okPressed_=false;
    //[/UserPreSize]

    setSize (450, 300);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

DialogComponent::~DialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    viewport = nullptr;
    cancelButton = nullptr;
    okButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void DialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff161616),
                                       96.0f, static_cast<float> (proportionOfHeight (0.4267f)),
                                       Colour (0xff969696),
                                       96.0f, static_cast<float> (proportionOfHeight (1.2800f)),
                                       false));
    g.fillRect (-8, -8, proportionOfWidth (1.1000f), proportionOfHeight (1.1000f));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void DialogComponent::resized()
{
    viewport->setBounds (20, 20, getWidth() - 40, getHeight() - 64);
    cancelButton->setBounds (getWidth() - 82, getHeight() - 36, 64, 24);
    okButton->setBounds (getWidth() - 162, getHeight() - 36, 64, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void DialogComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == cancelButton)
    {
        //[UserButtonCode_cancelButton] -- add your button handler code here..
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            dw->exitModalState(0);
        }

        //[/UserButtonCode_cancelButton]
    }
    else if (buttonThatWasClicked == okButton)
    {
        //[UserButtonCode_okButton] -- add your button handler code here..
        okPressed_=true;
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            dw->exitModalState(0);
        }


        //[/UserButtonCode_okButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
bool DialogComponent::okPressed()
{
    return okPressed_;
}

void DialogComponent::setId(String id)
{
    id_=id;
}

String DialogComponent::getId()
{
    return id_;
}
Component* DialogComponent::getViewedComponent()
{
    return viewport->getViewedComponent();
}

void DialogComponent::updateComponent(Component* propPanel)
{
    // XXX
    // delete the old one
    viewport->setViewedComponent(propPanel);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="DialogComponent" componentName=""
                 parentClasses="public Component" constructorParams="Component* propPanel, int mode"
                 variableInitialisers="mode_(mode)" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="450"
                 initialHeight="300">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="-8 -8 110% 110%" fill="linear: 96 42.667%, 96 128%, 0=ff161616, 1=ff969696"
          hasStroke="0"/>
  </BACKGROUND>
  <VIEWPORT name="new viewport" id="b87e30c574490959" memberName="viewport"
            virtualName="" explicitFocusOrder="0" pos="20 20 40M 64M" vscroll="1"
            hscroll="1" scrollbarThickness="18" contentType="0" jucerFile=""
            contentClass="" constructorParams=""/>
  <TEXTBUTTON name="cancelButton" id="e2fa13339d32a919" memberName="cancelButton"
              virtualName="" explicitFocusOrder="0" pos="82R 36R 64 24" buttonText="Cancel"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="okbutton" id="4fc7d5266f6b1b35" memberName="okButton" virtualName=""
              explicitFocusOrder="0" pos="162R 36R 64 24" buttonText="OK" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
