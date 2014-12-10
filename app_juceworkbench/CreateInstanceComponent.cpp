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

#include "CreateInstanceComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
CreateInstanceComponent::CreateInstanceComponent (const std::set<std::string> & ords)
    : ordinal_(1)
{
    addAndMakeVisible (textButton = new TextButton ("okButton"));
    textButton->setButtonText (TRANS("OK"));
    textButton->addListener (this);
    textButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (textButton2 = new TextButton ("cancelButton"));
    textButton2->setButtonText (TRANS("Cancel"));
    textButton2->addListener (this);
    textButton2->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (txtOrdinal = new TextEditor ("txtOrdinal"));
    txtOrdinal->setMultiLine (false);
    txtOrdinal->setReturnKeyStartsNewLine (false);
    txtOrdinal->setReadOnly (false);
    txtOrdinal->setScrollbarsShown (false);
    txtOrdinal->setCaretVisible (true);
    txtOrdinal->setPopupMenuEnabled (false);
    txtOrdinal->setText (TRANS("1"));

    addAndMakeVisible (upButton = new TextButton ("upButton"));
    upButton->setTooltip (TRANS("Increment No."));
    upButton->setButtonText (TRANS("+"));
    upButton->addListener (this);
    upButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (downButton = new TextButton ("downButton"));
    downButton->setTooltip (TRANS("Decrement No."));
    downButton->setButtonText (TRANS("-"));
    downButton->addListener (this);
    downButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("No.")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (errorLabel = new Label ("errorLabel",
                                               TRANS("Number already in use!")));
    errorLabel->setFont (Font (15.00f, Font::plain));
    errorLabel->setJustificationType (Justification::centredLeft);
    errorLabel->setEditable (false, false, false);
    errorLabel->setColour (Label::textColourId, Colour (0xffe76a3d));
    errorLabel->setColour (TextEditor::textColourId, Colours::black);
    errorLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    setupOrdinals(ords);
    okPressed_=false;
    txtOrdinal->addListener(this);
    errorLabel->setVisible(false);
    txtOrdinal->setInputRestrictions(0,"0123456789");
    //[/UserPreSize]

    setSize (300, 100);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

CreateInstanceComponent::~CreateInstanceComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    textButton = nullptr;
    textButton2 = nullptr;
    txtOrdinal = nullptr;
    upButton = nullptr;
    downButton = nullptr;
    label = nullptr;
    errorLabel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void CreateInstanceComponent::paint (Graphics& g)
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

void CreateInstanceComponent::resized()
{
    textButton->setBounds (200, 72, 71, 24);
    textButton2->setBounds (112, 72, 71, 24);
    txtOrdinal->setBounds (128, 40, 56, 24);
    upButton->setBounds (240, 44, 32, 16);
    downButton->setBounds (200, 44, 32, 16);
    label->setBounds (96, 40, 32, 24);
    errorLabel->setBounds (56, 16, 216, 16);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void CreateInstanceComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == textButton)
    {
        //[UserButtonCode_textButton] -- add your button handler code here..
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            okPressed_=true;
            dw->exitModalState(0);
        }
        //[/UserButtonCode_textButton]
    }
    else if (buttonThatWasClicked == textButton2)
    {
        //[UserButtonCode_textButton2] -- add your button handler code here..
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            dw->exitModalState(0);
        }
        //[/UserButtonCode_textButton2]
    }
    else if (buttonThatWasClicked == upButton)
    {
        //[UserButtonCode_upButton] -- add your button handler code here..
        setNextAvailableOrdinal();
        txtOrdinal->setText (String(ordinal_));
        //[/UserButtonCode_upButton]
    }
    else if (buttonThatWasClicked == downButton)
    {
        //[UserButtonCode_downButton] -- add your button handler code here..
        setPreviousAvailableOrdinal();
        txtOrdinal->setText (String(ordinal_));
        //[/UserButtonCode_downButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void CreateInstanceComponent::setupOrdinals(const std::set<std::string>& ords)
{
    ordinals_.clear();
    std::set<std::string>::const_iterator i=ords.begin();
    while(i!=ords.end())
    {
       String s=String::fromUTF8(i->c_str());
       ordinals_.insert(s.getIntValue());
       i++;
    }

    setLowestAvailableOrdinal();
    txtOrdinal->setText (String(ordinal_));
}

void CreateInstanceComponent::setNextAvailableOrdinal()
{
    int test=ordinal_+1;
    std::set<int>::iterator i=ordinals_.find(test);
    while(i!=ordinals_.end())
    {
        test=test+1;
        i=ordinals_.find(test);
    }
    ordinal_=test;
}

void CreateInstanceComponent::setPreviousAvailableOrdinal()
{
    if (ordinal_>1)
    {
        int test=ordinal_-1;
        std::set<int>::iterator i=ordinals_.find(test);
        while(i!=ordinals_.end())
        {
            test=test-1;

            i=ordinals_.find(test);
        }
        if (test>=1)
        {
            ordinal_=test;
        }
    }
}

void CreateInstanceComponent::setLowestAvailableOrdinal()
{
    int test=1;
    std::set<int>::iterator i=ordinals_.find(test);
    while(i!=ordinals_.end())
    {
        test=test+1;
        i=ordinals_.find(test);
    }
    ordinal_=test;
}

bool CreateInstanceComponent::isValidOrdinal(String ord)
{
    int test=ord.getIntValue();
    std::set<int>::iterator i=ordinals_.find(test);
    if(i==ordinals_.end())
    {
        return true;
    }
    return false;
}

void CreateInstanceComponent:: textEditorTextChanged(TextEditor &editor)
{
    if(&editor==txtOrdinal)
    {
        if(!isValidOrdinal(txtOrdinal->getText()))
        {
            textButton->setEnabled(false);
            errorLabel->setVisible(true);
        }
        else
        {
            ordinal_=txtOrdinal->getText().getIntValue();
            textButton->setEnabled(true);
            errorLabel->setVisible(false);
        }

    }
}

int CreateInstanceComponent::getOrdinal()
{
    return ordinal_;
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CreateInstanceComponent"
                 componentName="" parentClasses="public Component, public TextEditor::Listener"
                 constructorParams="const std::set&lt;std::string&gt; &amp; ords"
                 variableInitialisers="ordinal_(1)" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="300"
                 initialHeight="100">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 100% 100%" fill="linear: 96 24, 100 100, 0=ff060505, 1=ff757775"
          hasStroke="0"/>
  </BACKGROUND>
  <TEXTBUTTON name="okButton" id="e2aadca74a756fb3" memberName="textButton"
              virtualName="" explicitFocusOrder="0" pos="200 72 71 24" bgColOff="ffaeaeae"
              buttonText="OK" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="cancelButton" id="e058ee11d9e251b0" memberName="textButton2"
              virtualName="" explicitFocusOrder="0" pos="112 72 71 24" bgColOff="ffaeaeae"
              buttonText="Cancel" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="txtOrdinal" id="614368bc45612b55" memberName="txtOrdinal"
              virtualName="" explicitFocusOrder="0" pos="128 40 56 24" initialText="1"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="0"
              caret="1" popupmenu="0"/>
  <TEXTBUTTON name="upButton" id="b4835983d8abde78" memberName="upButton" virtualName=""
              explicitFocusOrder="0" pos="240 44 32 16" tooltip="Increment No."
              bgColOff="ffaeaeae" buttonText="+" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
  <TEXTBUTTON name="downButton" id="81b7e7e3e1250b2f" memberName="downButton"
              virtualName="" explicitFocusOrder="0" pos="200 44 32 16" tooltip="Decrement No."
              bgColOff="ffaeaeae" buttonText="-" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
  <LABEL name="new label" id="43e9cffc1c851c85" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="96 40 32 24" textCol="ffffffff" edTextCol="ff000000"
         edBkgCol="0" labelText="No." editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <LABEL name="errorLabel" id="c822ad63e65730be" memberName="errorLabel"
         virtualName="" explicitFocusOrder="0" pos="56 16 216 16" textCol="ffe76a3d"
         edTextCol="ff000000" edBkgCol="0" labelText="Number already in use!"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
