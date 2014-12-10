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

#include "EditMapConfirmation.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
EditMapConfirmation::EditMapConfirmation ()
{
    addAndMakeVisible (okButton = new TextButton ("okButton"));
    okButton->setButtonText (TRANS("OK"));
    okButton->addListener (this);
    okButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("It\'s easier to edit this mapping with the Mapping Editor displayed by clicking the \'Edit\' button")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centred);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (toggleButton = new ToggleButton ("new toggle button"));
    toggleButton->setButtonText (TRANS("don\'t show this again"));
    toggleButton->addListener (this);
    toggleButton->setColour (ToggleButton::textColourId, Colours::white);


    //[UserPreSize]
    okPressed_=false;
    dontShowAgain_=false;
    //[/UserPreSize]

    setSize (300, 140);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

EditMapConfirmation::~EditMapConfirmation()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    okButton = nullptr;
    label = nullptr;
    toggleButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void EditMapConfirmation::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff060505),
                                       96.0f, 24.0f,
                                       Colour (0xff757775),
                                       100.0f, 100.0f,
                                       false));
    g.fillRect (0, 0, proportionOfWidth (1.0000f), proportionOfHeight (1.0000f));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void EditMapConfirmation::resized()
{
    okButton->setBounds (120, 104, 71, 24);
    label->setBounds (16, 16, 272, 56);
    toggleButton->setBounds (8, 72, 150, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void EditMapConfirmation::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == okButton)
    {
        //[UserButtonCode_okButton] -- add your button handler code here..
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            okPressed_=true;
            dw->exitModalState(0);
        }
        //[/UserButtonCode_okButton]
    }
    else if (buttonThatWasClicked == toggleButton)
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        dontShowAgain_=toggleButton->getToggleState();
        //[/UserButtonCode_toggleButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="EditMapConfirmation" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="300" initialHeight="140">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 100% 100%" fill="linear: 96 24, 100 100, 0=ff060505, 1=ff757775"
          hasStroke="0"/>
  </BACKGROUND>
  <TEXTBUTTON name="okButton" id="e2aadca74a756fb3" memberName="okButton" virtualName=""
              explicitFocusOrder="0" pos="120 104 71 24" bgColOff="ffaeaeae"
              buttonText="OK" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="new label" id="cf632a1aa060f6f0" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="16 16 272 56" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="It's easier to edit this mapping with the Mapping Editor displayed by clicking the 'Edit' button"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
  <TOGGLEBUTTON name="new toggle button" id="ebc24f529e7bdcaa" memberName="toggleButton"
                virtualName="" explicitFocusOrder="0" pos="8 72 150 24" txtcol="ffffffff"
                buttonText="don't show this again" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
