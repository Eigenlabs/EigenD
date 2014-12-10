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

#include "DuplicatePanel.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
DuplicatePanel::DuplicatePanel ()
{
    addAndMakeVisible (usingButton = new ToggleButton ("usingButton"));
    usingButton->setButtonText (String::empty);
    usingButton->addListener (this);

    addAndMakeVisible (filterButton = new ToggleButton ("filterButton"));
    filterButton->setButtonText (String::empty);
    filterButton->addListener (this);

    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("Use same values for all")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (550, 35);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

DuplicatePanel::~DuplicatePanel()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    usingButton = nullptr;
    filterButton = nullptr;
    label = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void DuplicatePanel::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void DuplicatePanel::resized()
{
    usingButton->setBounds (376, 8, 24, 16);
    filterButton->setBounds (496, 8, 24, 16);
    label->setBounds (8, 8, 296, 16);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void DuplicatePanel::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == usingButton)
    {
        //[UserButtonCode_usingButton] -- add your button handler code here..
        if (usingButton->getToggleState()==true)
        {
            getParentComponent()->postCommandMessage(0);
        }
        else
        {
            getParentComponent()->postCommandMessage(1);
        }
        //[/UserButtonCode_usingButton]
    }
    else if (buttonThatWasClicked == filterButton)
    {
        //[UserButtonCode_filterButton] -- add your button handler code here..
        if (filterButton->getToggleState()==true)
        {
            getParentComponent()->postCommandMessage(2);
        }
        else
        {
            getParentComponent()->postCommandMessage(3);
        }
        //[/UserButtonCode_filterButton]
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

<JUCER_COMPONENT documentType="Component" className="DuplicatePanel" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="550" initialHeight="35">
  <BACKGROUND backgroundColour="ffffff"/>
  <TOGGLEBUTTON name="usingButton" id="902daf318bac9064" memberName="usingButton"
                virtualName="" explicitFocusOrder="0" pos="376 8 24 16" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="filterButton" id="c6f930213944cac9" memberName="filterButton"
                virtualName="" explicitFocusOrder="0" pos="496 8 24 16" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <LABEL name="new label" id="934e19f097e22706" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="8 8 296 16" textCol="ffffffff" edTextCol="ff000000"
         edBkgCol="0" labelText="Use same values for all" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
