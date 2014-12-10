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

#include "WireEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
WireEditor::WireEditor (Wire* w)
    : w_(w),
      usingController_(false),
      filterController_(false),
      controlController_(false)
{
    addAndMakeVisible (routeLabel = new Label ("routeLabel",
                                               String::empty));
    routeLabel->setFont (Font (15.00f, Font::plain));
    routeLabel->setJustificationType (Justification::centredLeft);
    routeLabel->setEditable (false, false, false);
    routeLabel->setColour (Label::textColourId, Colours::white);
    routeLabel->setColour (TextEditor::textColourId, Colours::black);
    routeLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (usingLabel = new Label ("usingLabel",
                                               TRANS("channel")));
    usingLabel->setFont (Font (15.00f, Font::plain));
    usingLabel->setJustificationType (Justification::centredLeft);
    usingLabel->setEditable (false, false, false);
    usingLabel->setColour (Label::textColourId, Colours::white);
    usingLabel->setColour (TextEditor::textColourId, Colours::black);
    usingLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (txtUsing = new TextEditor ("txtUsing"));
    txtUsing->setMultiLine (false);
    txtUsing->setReturnKeyStartsNewLine (false);
    txtUsing->setReadOnly (false);
    txtUsing->setScrollbarsShown (true);
    txtUsing->setCaretVisible (true);
    txtUsing->setPopupMenuEnabled (true);
    txtUsing->setText (String::empty);

    addAndMakeVisible (filterLabel = new Label ("filterLabel",
                                                TRANS("input filter")));
    filterLabel->setFont (Font (15.00f, Font::plain));
    filterLabel->setJustificationType (Justification::centredLeft);
    filterLabel->setEditable (false, false, false);
    filterLabel->setColour (Label::textColourId, Colours::white);
    filterLabel->setColour (TextEditor::textColourId, Colours::black);
    filterLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (textEditor = new TextEditor ("new text editor"));
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (String::empty);

    addAndMakeVisible (incButton = new TextButton ("incButton"));
    incButton->setButtonText (TRANS("+"));
    incButton->addListener (this);

    addAndMakeVisible (decButton = new TextButton ("decButton"));
    decButton->setButtonText (TRANS("-"));
    decButton->addListener (this);

    addAndMakeVisible (ctlButton = new ToggleButton ("ctl"));
    ctlButton->addListener (this);
    ctlButton->setColour (ToggleButton::textColourId, Colours::white);


    //[UserPreSize]
    pic::logmsg()<<"setup wire editor "<<std::string(w_->getId().toUTF8())<<" routehash="<<w_->getRouteHash();
    routeLabel->setText(w->getDescription(),dontSendNotification);
    routeLabel->setTooltip(w->getDescription());
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
    txtUsing->setInputRestrictions(0,"0123456789");
    txtUsing->addListener(this);
    initial_u_=u_;

    f_=w_->get_srcFilter();
    setFilterText(f_);
    textEditor->setInputRestrictions(0,"0123456789.");
    textEditor->addListener(this);
    initial_f_=f_;

    if(w_->get_control().equalsIgnoreCase("ctl"))
    {
        c_=true;
        ctlButton->setToggleState(true,false);
    }
    else
    {
        c_=false;
        ctlButton->setToggleState(false,false);
    }
    initial_c_=c_;

    //[/UserPreSize]

    setSize (550, 28);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

WireEditor::~WireEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    routeLabel = nullptr;
    usingLabel = nullptr;
    txtUsing = nullptr;
    filterLabel = nullptr;
    textEditor = nullptr;
    incButton = nullptr;
    decButton = nullptr;
    ctlButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void WireEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void WireEditor::resized()
{
    routeLabel->setBounds (8, 8, 296, 16);
    usingLabel->setBounds (310, 6, 54, 16);
    txtUsing->setBounds (362, 6, 24, 16);
    filterLabel->setBounds (404, 8, 78, 14);
    textEditor->setBounds (478, 6, 32, 16);
    incButton->setBounds (388, 6, 16, 8);
    decButton->setBounds (388, 16, 16, 8);
    ctlButton->setBounds (510, 2, 36, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void WireEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == incButton)
    {
        //[UserButtonCode_incButton] -- add your button handler code here..
        u_=u_+1;
        setUsingText(u_);
        //[/UserButtonCode_incButton]
    }
    else if (buttonThatWasClicked == decButton)
    {
        //[UserButtonCode_decButton] -- add your button handler code here..
        if(u_>=1)
        {
            u_=u_-1;
            setUsingText(u_);
        }

        //[/UserButtonCode_decButton]
    }
    else if (buttonThatWasClicked == ctlButton)
    {
        //[UserButtonCode_ctlButton] -- add your button handler code here..
        c_=ctlButton->getToggleState();
        if(controlController_)
        {
            getParentComponent()->postCommandMessage(6);
        }


        //[/UserButtonCode_ctlButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void WireEditor::setUsingController(bool controller)
{
    usingController_=controller;
}

void WireEditor::setFilterController(bool controller)
{
    filterController_=controller;
}

void WireEditor::setControlController(bool controller)
{
    controlController_=controller;
}


void WireEditor:: textEditorTextChanged(TextEditor &editor)
{
    if(&editor==txtUsing)
    {
        String u=txtUsing->getText();
        if(u.isEmpty())
        {
            u_=0;
        }
        else
        {
            u_=u.getIntValue();
        }
        if(usingController_)
        {
            getParentComponent()->postCommandMessage(4);
        }

    }
    else if(&editor==textEditor)
    {
        f_=textEditor->getText();
        if(filterController_)
        {
            getParentComponent()->postCommandMessage(5);
        }
    }
}

bool WireEditor::changed()
{
    return (u_!=initial_u_)||(f_!=initial_f_)||(c_!=initial_c_);
}

Wire* WireEditor::getWire()
{
    return w_;
}

String WireEditor::getUsing()
{
    if(u_>0)
    {
        return String(u_);
    }
    return String::empty;
}

String WireEditor::getFilter()
{
    return f_;
}

String WireEditor::getControl()
{
    String c=String::empty;
    if(c_)
    {
        c="ctl";
    }
    return c;
}

void WireEditor::setUsingText(int u)
{
    if(u<1)
    {
        txtUsing->setText (String::empty,false);
    }
    else
    {
        txtUsing->setText (String(u),false);
    }
    String utext=txtUsing->getText();
    if(utext.isEmpty())
    {
        u_=0;
    }
    else
    {
        u_=utext.getIntValue();
    }

    if(usingController_)
    {
        getParentComponent()->postCommandMessage(4);
    }
}

void WireEditor::setFilterText(String f)
{
    textEditor->setText(f,false);
    f_=textEditor->getText();

    if(filterController_)
    {
        getParentComponent()->postCommandMessage(5);
    }
}

void WireEditor::enableUsing(bool enable)
{
    txtUsing->setEnabled(enable);
    if(enable)
    {
        txtUsing->setColour(TextEditor::backgroundColourId,Colour(0xffffffff));
    }
    else
    {
        txtUsing->setColour(TextEditor::backgroundColourId,Colour(0xff8a8a8a));
    }
}

void WireEditor::enableFilter(bool enable)
{
    textEditor->setEnabled(enable);
    if(enable)
    {
        textEditor->setColour(TextEditor::backgroundColourId,Colour(0xffffffff));
    }
    else
    {
        textEditor->setColour(TextEditor::backgroundColourId,Colour(0xff8a8a8a));
    }

}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="WireEditor" componentName=""
                 parentClasses="public Component, public TextEditor::Listener, public ButtonListener"
                 constructorParams="Wire* w" variableInitialisers="w_(w),&#10;usingController_(false),&#10;filterController_(false),&#10;controlController_(false)"
                 snapPixels="2" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="550" initialHeight="28">
  <BACKGROUND backgroundColour="ffffff"/>
  <LABEL name="routeLabel" id="75ed7d83ad7fc4c4" memberName="routeLabel"
         virtualName="" explicitFocusOrder="0" pos="8 8 296 16" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="usingLabel" id="46f8f53fb4e8568f" memberName="usingLabel"
         virtualName="" explicitFocusOrder="0" pos="310 6 54 16" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="channel" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="txtUsing" id="49ad1e90c8440022" memberName="txtUsing" virtualName=""
              explicitFocusOrder="0" pos="362 6 24 16" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="filterLabel" id="c251fc33af6b558a" memberName="filterLabel"
         virtualName="" explicitFocusOrder="0" pos="404 8 78 14" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="input filter" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="new text editor" id="3452f8117e338097" memberName="textEditor"
              virtualName="" explicitFocusOrder="0" pos="478 6 32 16" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <TEXTBUTTON name="incButton" id="f45426b12aa7b554" memberName="incButton"
              virtualName="" explicitFocusOrder="0" pos="388 6 16 8" buttonText="+"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="decButton" id="889712e985fe405f" memberName="decButton"
              virtualName="" explicitFocusOrder="0" pos="388 16 16 8" buttonText="-"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TOGGLEBUTTON name="ctl" id="5896a3c9e16cc50" memberName="ctlButton" virtualName=""
                explicitFocusOrder="0" pos="510 2 36 24" txtcol="ffffffff" buttonText="ctl"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
