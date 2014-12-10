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

#include "NameEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
NameEditor::NameEditor (String name)
{
    setName ("NameEditor");
    addAndMakeVisible (nameLabel = new Label ("nameLabel",
                                              TRANS("Name:")));
    nameLabel->setFont (Font (15.00f, Font::plain));
    nameLabel->setJustificationType (Justification::centredLeft);
    nameLabel->setEditable (false, false, false);
    nameLabel->setColour (Label::textColourId, Colours::white);
    nameLabel->setColour (TextEditor::textColourId, Colours::black);
    nameLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (cancelButton = new TextButton ("cancelButton"));
    cancelButton->setButtonText (TRANS("Cancel"));
    cancelButton->addListener (this);
    cancelButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (okButton = new TextButton ("okButton"));
    okButton->setButtonText (TRANS("OK"));
    okButton->addListener (this);
    okButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (textEditor = new TextEditor ("new text editor"));
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (String::empty);

    addAndMakeVisible (srcLabel = new Label ("srcLabel",
                                             String::empty));
    srcLabel->setFont (Font (15.00f, Font::plain));
    srcLabel->setJustificationType (Justification::centredLeft);
    srcLabel->setEditable (false, false, false);
    srcLabel->setColour (Label::textColourId, Colours::whitesmoke);
    srcLabel->setColour (TextEditor::textColourId, Colours::black);
    srcLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (400, 200);


    //[Constructor] You can add your own custom stuff here..
    textEditor->setText(name);
    okPressed_=false;
    srcLabel->setVisible(false);
    //[/Constructor]
}

NameEditor::~NameEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    nameLabel = nullptr;
    cancelButton = nullptr;
    okButton = nullptr;
    textEditor = nullptr;
    srcLabel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void NameEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff161616),
                                       104.0f, 48.0f,
                                       Colour (0xff898989),
                                       104.0f, 184.0f,
                                       false));
    g.fillRect (0, 0, 400, 200);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void NameEditor::resized()
{
    nameLabel->setBounds (24, 56, 47, 24);
    cancelButton->setBounds (232, 152, 64, 24);
    okButton->setBounds (312, 152, 64, 24);
    textEditor->setBounds (104, 56, 264, 24);
    srcLabel->setBounds (136, 24, 248, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void NameEditor::buttonClicked (Button* buttonThatWasClicked)
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
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            okPressed_=true;
            dw->exitModalState(0);
        }

        //[/UserButtonCode_okButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
bool NameEditor::changed()
{
    // XXX
    return okPressed_;
}

String NameEditor::getName()
{
    return textEditor->getText();
}

void NameEditor:: textEditorTextChanged(TextEditor &editor)
{
    if(&editor==textEditor)
    {
    }
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="NameEditor" componentName="NameEditor"
                 parentClasses="public Component, public TextEditor::Listener"
                 constructorParams="String name" variableInitialisers="" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="400" initialHeight="200">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 400 200" fill="linear: 104 48, 104 184, 0=ff161616, 1=ff898989"
          hasStroke="0"/>
  </BACKGROUND>
  <LABEL name="nameLabel" id="d1103aa81b503cfc" memberName="nameLabel"
         virtualName="" explicitFocusOrder="0" pos="24 56 47 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Name:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="cancelButton" id="a04c352052413389" memberName="cancelButton"
              virtualName="" explicitFocusOrder="0" pos="232 152 64 24" bgColOff="ffaeaeae"
              buttonText="Cancel" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="okButton" id="834f229b0ba10346" memberName="okButton" virtualName=""
              explicitFocusOrder="0" pos="312 152 64 24" bgColOff="ffaeaeae"
              buttonText="OK" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="new text editor" id="6f0ab1eecb0ac9bd" memberName="textEditor"
              virtualName="" explicitFocusOrder="0" pos="104 56 264 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <LABEL name="srcLabel" id="2590dcac79af0d80" memberName="srcLabel" virtualName=""
         explicitFocusOrder="0" pos="136 24 248 24" textCol="fff5f5f5"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
