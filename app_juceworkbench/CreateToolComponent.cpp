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

#include "CreateToolComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
CreateToolComponent::CreateToolComponent (const std::set<std::string>& agents)
    : agents_(agents),
      ordinal_(1)
{
    addAndMakeVisible (comboBox = new ComboBox ("categoryBox"));
    comboBox->setEditableText (false);
    comboBox->setJustificationType (Justification::centredLeft);
    comboBox->setTextWhenNothingSelected (String::empty);
    comboBox->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    comboBox->addListener (this);

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
    okPressed_=false;
    comboBox->setColour(ComboBox::buttonColourId, Colour(0xffaeaeae));
    setupOrdinals();
    setupAgentCombo();
    errorLabel->setVisible(false);
    txtOrdinal->addListener(this);
    //[/UserPreSize]

    setSize (300, 140);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

CreateToolComponent::~CreateToolComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    comboBox = nullptr;
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
void CreateToolComponent::paint (Graphics& g)
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

void CreateToolComponent::resized()
{
    comboBox->setBounds (24, 40, 248, 24);
    textButton->setBounds (200, 104, 71, 24);
    textButton2->setBounds (112, 104, 71, 24);
    txtOrdinal->setBounds (128, 72, 56, 24);
    upButton->setBounds (240, 72, 32, 16);
    downButton->setBounds (200, 72, 32, 16);
    label->setBounds (96, 72, 32, 24);
    errorLabel->setBounds (32, 16, 232, 16);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void CreateToolComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == comboBox)
    {
        //[UserComboBoxCode_comboBox] -- add your combo box handling code here..
        setLowestAvailableOrdinal();
        txtOrdinal->setText (String(ordinal_));
        //[/UserComboBoxCode_comboBox]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void CreateToolComponent::buttonClicked (Button* buttonThatWasClicked)
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
void CreateToolComponent::setupOrdinals()
{
    std::set<std::string>::const_iterator i = agents_.begin();

    String testKey;
    String ordinals;
    String o;

    while(i!=agents_.end())
    {
        String s = String::fromUTF8(i->c_str());

        testKey=(s.upToFirstOccurrenceOf(",",false,true));
        ordinals=s.fromFirstOccurrenceOf(",",false,true);

        std::set<int> testOrdinals;
        while(ordinals.isNotEmpty())
        {
            o=ordinals.upToFirstOccurrenceOf(",",false,true);
            ordinals=ordinals.fromFirstOccurrenceOf(",",false,true);
            testOrdinals.insert(o.getIntValue());
        }

        ordinalMap_.insert(std::pair<String,std::set<int> >(testKey,testOrdinals));
        i++;
    }
}


void CreateToolComponent::setupAgentCombo()
{
    comboBox->clear(true);

    std::set<std::string>::const_iterator i = agents_.begin();
    StringArray agentNames=StringArray();

    while(i!=agents_.end())
    {
        String s = String::fromUTF8(i->c_str());
        agentNames.add(s.upToFirstOccurrenceOf(",",false,true));
        i++;
    }


    for (int i =0;i<agentNames.size();++i)
    {
        comboBox->addItem(agentNames[i],i+1);
    }

    comboBox->setEnabled(true);
    comboBox->setSelectedId(1);
}

void CreateToolComponent::setSelection(String selection)
{

    std::set<std::string>::const_iterator i = agents_.begin();
    while(i!=agents_.end())
    {
        String s = String::fromUTF8(i->c_str());
        if(s.upToFirstOccurrenceOf(",",false,true)==selection)
        {
            comboBox->setText(selection);
            break;
        }
        i++;
    }

}

String CreateToolComponent::getSelection()
{
    return comboBox->getText();
}


int CreateToolComponent::getOrdinal()
{
    return ordinal_;
}

void CreateToolComponent::setNextAvailableOrdinal()
{

    std::map<String,std::set<int> >::iterator i=ordinalMap_.find(getSelection());
    if(i!=ordinalMap_.end())
    {
        int test=ordinal_+1;
        std::set<int> in_use= (i->second);
        std::set<int>::iterator iter;
        iter=in_use.find(test);

        while(iter!=in_use.end())
        {
            test=test+1;
            iter=in_use.find(test);
        }

        ordinal_=test;
    }
    else
    {
        pic::logmsg()<<"cant find key in ordinalMap"<<std::string(getSelection().toUTF8());
    }
}

void CreateToolComponent::setPreviousAvailableOrdinal()
{
    if (ordinal_>1)
    {
        std::map<String,std::set<int> >::iterator i=ordinalMap_.find(getSelection());
        if(i!=ordinalMap_.end())
        {
            int test=ordinal_-1;
            std::set<int> in_use= (i->second);
            std::set<int>::iterator iter;
            iter=in_use.find(test);

            while(iter!=in_use.end() && (test!=0))
            {
                test=test-1;
                iter=in_use.find(test);
            }

            if (test>0)
            {
                ordinal_=test;
            }
        }
        else
        {
            pic::logmsg()<<"cant find key in ordinalMap"<<std::string(getSelection().toUTF8());
        }

    }
}

void CreateToolComponent::setLowestAvailableOrdinal()
{
    std::map<String,std::set<int> >::iterator i=ordinalMap_.find(getSelection());
    if(i!=ordinalMap_.end())
    {
        int test=1;
        std::set<int> in_use= (i->second);
        std::set<int>::iterator iter;
        iter=in_use.find(test);

        while(iter!=in_use.end())
        {
            test=test+1;
            iter=in_use.find(test);
        }

        ordinal_=test;
    }
    else
    {
        pic::logmsg()<<"cant find key in ordinalMap"<<std::string(getSelection().toUTF8());
    }
}

bool CreateToolComponent::isValidOrdinal(String ord)
{
    int test=ord.getIntValue();
    std::map<String,std::set<int> >::iterator i=ordinalMap_.find(getSelection());
    if(i!=ordinalMap_.end())
    {
        std::set<int> in_use= (i->second);
        std::set<int>::iterator iter;
        iter=in_use.find(test);
        if(iter!=in_use.end())
        {
            return false;
        }
    }
    return true;
}

void CreateToolComponent:: textEditorTextChanged(TextEditor &editor)
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
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CreateToolComponent" componentName=""
                 parentClasses="public Component, public TextEditor::Listener"
                 constructorParams="const std::set&lt;std::string&gt;&amp; agents"
                 variableInitialisers="agents_(agents),&#10;ordinal_(1)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="300" initialHeight="140">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 100% 100%" fill="linear: 96 24, 100 100, 0=ff060505, 1=ff757775"
          hasStroke="0"/>
  </BACKGROUND>
  <COMBOBOX name="categoryBox" id="e497a5b863154e90" memberName="comboBox"
            virtualName="" explicitFocusOrder="0" pos="24 40 248 24" editable="0"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <TEXTBUTTON name="okButton" id="e2aadca74a756fb3" memberName="textButton"
              virtualName="" explicitFocusOrder="0" pos="200 104 71 24" bgColOff="ffaeaeae"
              buttonText="OK" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="cancelButton" id="e058ee11d9e251b0" memberName="textButton2"
              virtualName="" explicitFocusOrder="0" pos="112 104 71 24" bgColOff="ffaeaeae"
              buttonText="Cancel" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="txtOrdinal" id="614368bc45612b55" memberName="txtOrdinal"
              virtualName="" explicitFocusOrder="0" pos="128 72 56 24" initialText="1"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="0"
              caret="1" popupmenu="0"/>
  <TEXTBUTTON name="upButton" id="b4835983d8abde78" memberName="upButton" virtualName=""
              explicitFocusOrder="0" pos="240 72 32 16" tooltip="Increment No."
              bgColOff="ffaeaeae" buttonText="+" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
  <TEXTBUTTON name="downButton" id="81b7e7e3e1250b2f" memberName="downButton"
              virtualName="" explicitFocusOrder="0" pos="200 72 32 16" tooltip="Decrement No."
              bgColOff="ffaeaeae" buttonText="-" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
  <LABEL name="new label" id="43e9cffc1c851c85" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="96 72 32 24" textCol="ffffffff" edTextCol="ff000000"
         edBkgCol="0" labelText="No." editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <LABEL name="errorLabel" id="186747d445425ebe" memberName="errorLabel"
         virtualName="" explicitFocusOrder="0" pos="32 16 232 16" textCol="ffe76a3d"
         edTextCol="ff000000" edBkgCol="0" labelText="Number already in use!"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
