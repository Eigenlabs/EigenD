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

#include "SingleInputEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
SingleInputEditor::SingleInputEditor (Wire* w, String srcName)
{
    setName ("SingleInputEditor");
    addAndMakeVisible (usingLabel = new Label ("usingLabel",
                                               TRANS("Channel:")));
    usingLabel->setFont (Font (15.00f, Font::plain));
    usingLabel->setJustificationType (Justification::centredLeft);
    usingLabel->setEditable (false, false, false);
    usingLabel->setColour (Label::textColourId, Colours::white);
    usingLabel->setColour (TextEditor::textColourId, Colours::black);
    usingLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

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

    addAndMakeVisible (incrementButton = new TextButton ("incrementButton"));
    incrementButton->setButtonText (TRANS("+"));
    incrementButton->addListener (this);
    incrementButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (decrementButton = new TextButton ("decrementButton"));
    decrementButton->setButtonText (TRANS("-"));
    decrementButton->addListener (this);
    decrementButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (fromLabel = new Label ("fromLabel",
                                              TRANS("Connected from:")));
    fromLabel->setFont (Font (15.00f, Font::plain));
    fromLabel->setJustificationType (Justification::centredLeft);
    fromLabel->setEditable (false, false, false);
    fromLabel->setColour (Label::textColourId, Colours::white);
    fromLabel->setColour (TextEditor::textColourId, Colours::black);
    fromLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (srcLabel = new Label ("srcLabel",
                                             String::empty));
    srcLabel->setFont (Font (15.00f, Font::plain));
    srcLabel->setJustificationType (Justification::centredLeft);
    srcLabel->setEditable (false, false, false);
    srcLabel->setColour (Label::textColourId, Colours::whitesmoke);
    srcLabel->setColour (TextEditor::textColourId, Colours::black);
    srcLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (textEditor2 = new TextEditor ("new text editor"));
    textEditor2->setMultiLine (false);
    textEditor2->setReturnKeyStartsNewLine (false);
    textEditor2->setReadOnly (false);
    textEditor2->setScrollbarsShown (true);
    textEditor2->setCaretVisible (true);
    textEditor2->setPopupMenuEnabled (true);
    textEditor2->setText (String::empty);

    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("Filter on input:")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (controlButton = new ToggleButton ("controlButton"));
    controlButton->setButtonText (TRANS("Control connection"));
    controlButton->addListener (this);
    controlButton->setColour (ToggleButton::textColourId, Colours::white);


    //[UserPreSize]
    w_=w;
    srcName_=srcName;
    String u=w_->get_dstUsing();
    if(u.isNotEmpty())
    {
        u_=u.getIntValue();
    }
    else
    {
        u_=0;
    }
    setUsingText(u_);
    textEditor->setInputRestrictions(0,"0123456789");
    textEditor->addListener(this);
    initial_u_=u_;

    String f_=w_->get_srcFilter();
    setFilterText(f_);
    textEditor2->setInputRestrictions(0,"0123456789.");
    textEditor2->addListener(this);
    initial_f_=f_;

    if(w_->get_control().equalsIgnoreCase("ctl"))
    {
        c_=true;
        controlButton->setToggleState(true,false);
    }
    else
    {
        c_=false;
        controlButton->setToggleState(false,false);
    }
    initial_c_=c_;


    srcLabel->setText(srcName_,dontSendNotification);
    okPressed_=false;
    //[/UserPreSize]

    setSize (400, 200);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

SingleInputEditor::~SingleInputEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    usingLabel = nullptr;
    cancelButton = nullptr;
    okButton = nullptr;
    textEditor = nullptr;
    incrementButton = nullptr;
    decrementButton = nullptr;
    fromLabel = nullptr;
    srcLabel = nullptr;
    textEditor2 = nullptr;
    label = nullptr;
    controlButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SingleInputEditor::paint (Graphics& g)
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

void SingleInputEditor::resized()
{
    usingLabel->setBounds (22, 56, 100, 24);
    cancelButton->setBounds (232, 152, 64, 24);
    okButton->setBounds (312, 152, 64, 24);
    textEditor->setBounds (96, 58, 32, 24);
    incrementButton->setBounds (140, 58, 32, 24);
    decrementButton->setBounds (182, 58, 32, 24);
    fromLabel->setBounds (22, 24, 112, 24);
    srcLabel->setBounds (136, 24, 248, 24);
    textEditor2->setBounds (136, 96, 72, 24);
    label->setBounds (22, 96, 106, 24);
    controlButton->setBounds (22, 128, 150, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void SingleInputEditor::buttonClicked (Button* buttonThatWasClicked)
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
    else if (buttonThatWasClicked == incrementButton)
    {
        //[UserButtonCode_incrementButton] -- add your button handler code here..
        u_=u_+1;
        setUsingText(u_);
        //[/UserButtonCode_incrementButton]
    }
    else if (buttonThatWasClicked == decrementButton)
    {
        //[UserButtonCode_decrementButton] -- add your button handler code here..
        if(u_>=1)
        {
            u_=u_-1;
            setUsingText(u_);
        }
        //[/UserButtonCode_decrementButton]
    }
    else if (buttonThatWasClicked == controlButton)
    {
        //[UserButtonCode_controlButton] -- add your button handler code here..
        c_=controlButton->getToggleState();
        //[/UserButtonCode_controlButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
bool SingleInputEditor::changed()
{
    return (okPressed_ && ((u_!=initial_u_)||(f_!=initial_f_)||(c_!=initial_c_)));
}

String SingleInputEditor::getUsing()
{
    if(u_>0)
    {
        return String(u_);
    }
    return String::empty;
}

String SingleInputEditor::getFilter()
{
    return f_;
}

String SingleInputEditor::getControl()
{
    if(c_)
    {
        return String("ctl");
    }
    else
    {
        return String::empty;
    }
}

void SingleInputEditor::setUsingText(int u)
{
    if(u<1)
    {
        textEditor->setText (String::empty,false);
    }
    else
    {
        textEditor->setText (String(u),false);
    }
}

void SingleInputEditor::setFilterText(String f)
{
    textEditor2->setText(f,false);
}

void SingleInputEditor:: textEditorTextChanged(TextEditor &editor)
{
    if(&editor==textEditor)
    {
        String u=textEditor->getText();
        if(u.isEmpty())
        {
            u_=0;
        }
        else
        {
            u_=u.getIntValue();
        }
    }
    else if(&editor==textEditor2)
    {
        f_=textEditor2->getText();
    }
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="SingleInputEditor" componentName="SingleInputEditor"
                 parentClasses="public Component, public TextEditor::Listener"
                 constructorParams="Wire* w, String srcName" variableInitialisers=""
                 snapPixels="2" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="400" initialHeight="200">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 400 200" fill="linear: 104 48, 104 184, 0=ff161616, 1=ff898989"
          hasStroke="0"/>
  </BACKGROUND>
  <LABEL name="usingLabel" id="d1103aa81b503cfc" memberName="usingLabel"
         virtualName="" explicitFocusOrder="0" pos="22 56 100 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Channel:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="cancelButton" id="a04c352052413389" memberName="cancelButton"
              virtualName="" explicitFocusOrder="0" pos="232 152 64 24" bgColOff="ffaeaeae"
              buttonText="Cancel" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="okButton" id="834f229b0ba10346" memberName="okButton" virtualName=""
              explicitFocusOrder="0" pos="312 152 64 24" bgColOff="ffaeaeae"
              buttonText="OK" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="new text editor" id="6f0ab1eecb0ac9bd" memberName="textEditor"
              virtualName="" explicitFocusOrder="0" pos="96 58 32 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <TEXTBUTTON name="incrementButton" id="bab0bfd0f818f32e" memberName="incrementButton"
              virtualName="" explicitFocusOrder="0" pos="140 58 32 24" bgColOff="ffaeaeae"
              buttonText="+" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="decrementButton" id="8f824f64ed4a0991" memberName="decrementButton"
              virtualName="" explicitFocusOrder="0" pos="182 58 32 24" bgColOff="ffaeaeae"
              buttonText="-" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="fromLabel" id="a93cf8174eb4efdb" memberName="fromLabel"
         virtualName="" explicitFocusOrder="0" pos="22 24 112 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Connected from:"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="srcLabel" id="2590dcac79af0d80" memberName="srcLabel" virtualName=""
         explicitFocusOrder="0" pos="136 24 248 24" textCol="fff5f5f5"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="new text editor" id="1528ccd1587c5496" memberName="textEditor2"
              virtualName="" explicitFocusOrder="0" pos="136 96 72 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <LABEL name="new label" id="a1632146c9f93bc9" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="22 96 106 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Filter on input:"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="controlButton" id="711bfd5d142b15fd" memberName="controlButton"
                virtualName="" explicitFocusOrder="0" pos="22 128 150 24" txtcol="ffffffff"
                buttonText="Control connection" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
