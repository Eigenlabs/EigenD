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

#include "XYMapComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
MappingTextEditor::MappingTextEditor(String name):TextEditor(name)
{
}

void MappingTextEditor::focusLost(FocusChangeType cause)
{
    TextEditor::focusLost(cause);
    std::cout<<"FocusLost: cause="<<cause<<"  "<<std::string(getName().toUTF8())<<std::endl;
    XYMapComponent* c=dynamic_cast<XYMapComponent*>(getParentComponent());
    if(c!=0)
    {
        if (!c->removing)
        {
            c->updateMapping();
        }
    }

}
//[/MiscUserDefs]

//==============================================================================
XYMapComponent::XYMapComponent (int x, int y)
{
    addAndMakeVisible (xIn = new MappingTextEditor ("xIn"));
    xIn->setExplicitFocusOrder (1);
    xIn->setMultiLine (false);
    xIn->setReturnKeyStartsNewLine (false);
    xIn->setReadOnly (false);
    xIn->setScrollbarsShown (false);
    xIn->setCaretVisible (true);
    xIn->setPopupMenuEnabled (true);
    xIn->setText (String::empty);

    addAndMakeVisible (yIn = new MappingTextEditor ("yIn"));
    yIn->setExplicitFocusOrder (2);
    yIn->setMultiLine (false);
    yIn->setReturnKeyStartsNewLine (false);
    yIn->setReadOnly (false);
    yIn->setScrollbarsShown (false);
    yIn->setCaretVisible (true);
    yIn->setPopupMenuEnabled (true);
    yIn->setText (String::empty);

    addAndMakeVisible (yOut = new MappingTextEditor ("yOut"));
    yOut->setExplicitFocusOrder (4);
    yOut->setMultiLine (false);
    yOut->setReturnKeyStartsNewLine (false);
    yOut->setReadOnly (false);
    yOut->setScrollbarsShown (false);
    yOut->setCaretVisible (true);
    yOut->setPopupMenuEnabled (true);
    yOut->setText (String::empty);

    addAndMakeVisible (xOut = new MappingTextEditor ("xOut"));
    xOut->setExplicitFocusOrder (3);
    xOut->setMultiLine (false);
    xOut->setReturnKeyStartsNewLine (false);
    xOut->setReadOnly (false);
    xOut->setScrollbarsShown (false);
    xOut->setCaretVisible (true);
    xOut->setPopupMenuEnabled (true);
    xOut->setText (String::empty);

    addAndMakeVisible (removeElementButton = new TextButton ("removeElementButton"));
    removeElementButton->setTooltip (TRANS("Remove this key mapping"));
    removeElementButton->setButtonText (TRANS("-"));
    removeElementButton->addListener (this);


    //[UserPreSize]
    removing=false;
    xIn->setInputRestrictions(0,"0123456789");
    yIn->setInputRestrictions(0,"0123456789");
    xOut->setInputRestrictions(0,"0123456789");
    yOut->setInputRestrictions(0,"0123456789");
    //[/UserPreSize]

    setSize (400, 24);


    //[Constructor] You can add your own custom stuff here..
    setTopLeftPosition(x,y);
    //[/Constructor]
}

XYMapComponent::~XYMapComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    xIn = nullptr;
    yIn = nullptr;
    yOut = nullptr;
    xOut = nullptr;
    removeElementButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void XYMapComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    g.setColour(Colours::white);
    g.drawArrow(Line<float>(118,12,212,12),1.0,5.0,5.0);
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void XYMapComponent::resized()
{
    xIn->setBounds (10, 4, 40, 16);
    yIn->setBounds (68, 4, 40, 16);
    yOut->setBounds (272, 4, 40, 16);
    xOut->setBounds (222, 4, 40, 16);
    removeElementButton->setBounds (318, 4, 19, 16);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void XYMapComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == removeElementButton)
    {
        //[UserButtonCode_removeElementButton] -- add your button handler code here..
        MappingPanel* mp=dynamic_cast<MappingPanel*>(getParentComponent());
        if(mp!=0)
        {
            mp->removeElement(this);
        }
        //[/UserButtonCode_removeElementButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
String XYMapComponent::makeMappingString()
{
    String src="[";
    src=src+xIn->getText()+","+yIn->getText();
    src=src+"]";

    String dst="[";
    dst=dst+xOut->getText()+","+yOut->getText();
    dst=dst+"]";

    String mapping="[";
    mapping=mapping+src+","+dst;
    mapping=mapping+"]";
    return mapping;
}
void XYMapComponent::setInputValues(String x, String y)
{
    xIn->setText(x,false);
    yIn->setText(y,false);
}

void XYMapComponent::setOutputValues(String x, String y)
{
    xOut->setText(x,false);
    yOut->setText(y,false);
}

bool XYMapComponent::isValid()
{
    return xIn->getText().isNotEmpty() &&
        yIn->getText().isNotEmpty() &&
        xOut->getText().isNotEmpty() &&
        yOut->getText().isNotEmpty();
}

bool XYMapComponent::isEmpty()
{
    return xIn->getText().isEmpty() &&
        yIn->getText().isEmpty() &&
        xOut->getText().isEmpty() &&
        yOut->getText().isEmpty();
}

void XYMapComponent::updateMapping()
{
    MappingPanel* mp=dynamic_cast<MappingPanel*>(getParentComponent());
    if(isValid())
    {
        if(mp!=0)
        {
            mp->updateMapping();
        }
    }
    mp->checkRevertable();
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="XYMapComponent" componentName=""
                 parentClasses="public Component" constructorParams="int x, int y"
                 variableInitialisers="" snapPixels="2" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="400" initialHeight="24">
  <BACKGROUND backgroundColour="ffffff"/>
  <TEXTEDITOR name="xIn" id="25daa0004c9d3328" memberName="xIn" virtualName="MappingTextEditor"
              explicitFocusOrder="1" pos="10 4 40 16" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <TEXTEDITOR name="yIn" id="67c0056dad39c7a" memberName="yIn" virtualName="MappingTextEditor"
              explicitFocusOrder="2" pos="68 4 40 16" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <TEXTEDITOR name="yOut" id="e64381adb132d48c" memberName="yOut" virtualName="MappingTextEditor"
              explicitFocusOrder="4" pos="272 4 40 16" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <TEXTEDITOR name="xOut" id="1d57e4d5b4b98d53" memberName="xOut" virtualName="MappingTextEditor"
              explicitFocusOrder="3" pos="222 4 40 16" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <TEXTBUTTON name="removeElementButton" id="dbab43aced73afd5" memberName="removeElementButton"
              virtualName="" explicitFocusOrder="0" pos="318 4 19 16" tooltip="Remove this key mapping"
              buttonText="-" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
