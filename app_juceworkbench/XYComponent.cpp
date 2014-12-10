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

#include "XYComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
XYTextEditor::XYTextEditor(String name):TextEditor(name)
{
}

void XYTextEditor::focusLost(FocusChangeType cause)
{
    TextEditor::focusLost(cause);
    //std::cout<<"FocusLost: cause="<<cause<<"  "<<std::string(getName().toUTF8())<<std::endl;
    XYComponent* c=dynamic_cast<XYComponent*>(getParentComponent());
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
XYComponent::XYComponent (int x, int y)
{
    addAndMakeVisible (xIn = new XYTextEditor ("xIn"));
    xIn->setExplicitFocusOrder (1);
    xIn->setMultiLine (false);
    xIn->setReturnKeyStartsNewLine (false);
    xIn->setReadOnly (false);
    xIn->setScrollbarsShown (false);
    xIn->setCaretVisible (true);
    xIn->setPopupMenuEnabled (true);
    xIn->setText (String::empty);

    addAndMakeVisible (yIn = new XYTextEditor ("yIn"));
    yIn->setExplicitFocusOrder (2);
    yIn->setMultiLine (false);
    yIn->setReturnKeyStartsNewLine (false);
    yIn->setReadOnly (false);
    yIn->setScrollbarsShown (false);
    yIn->setCaretVisible (true);
    yIn->setPopupMenuEnabled (true);
    yIn->setText (String::empty);

    addAndMakeVisible (removeElementButton = new TextButton ("removeElementButton"));
    removeElementButton->setTooltip (TRANS("Remove this key mapping"));
    removeElementButton->setButtonText (TRANS("Remove key"));
    removeElementButton->addListener (this);


    //[UserPreSize]
    removing=false;
    xIn->setInputRestrictions(0,"0123456789");
    yIn->setInputRestrictions(0,"0123456789");
    //[/UserPreSize]

    setSize (288, 24);


    //[Constructor] You can add your own custom stuff here..
    setTopLeftPosition(x,y);
    //[/Constructor]
}

XYComponent::~XYComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    xIn = nullptr;
    yIn = nullptr;
    removeElementButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void XYComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void XYComponent::resized()
{
    xIn->setBounds (142, 2, 46, 16);
    yIn->setBounds (80, 2, 46, 16);
    removeElementButton->setBounds (200, 4, 68, 16);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void XYComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == removeElementButton)
    {
        //[UserButtonCode_removeElementButton] -- add your button handler code here..
        // XXX
        StrumKeyPanel* mp=dynamic_cast<StrumKeyPanel*>(getParentComponent());
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
String XYComponent::makeMappingString()
{
    String s="";
    s=s+yIn->getText()+","+xIn->getText();

    String mapping="[";
    mapping=mapping+s;
    mapping=mapping+"]";
    return mapping;
}

void XYComponent::setInputValues(String x, String y)
{
    xIn->setText(x,false);
    yIn->setText(y,false);
}

bool XYComponent::isValid()
{
    return xIn->getText().isNotEmpty() &&
        yIn->getText().isNotEmpty();
}

bool XYComponent::isEmpty()
{
    return xIn->getText().isEmpty() &&
        yIn->getText().isEmpty();
}

void XYComponent::updateMapping()
{
    StrumKeyPanel* mp=dynamic_cast<StrumKeyPanel*>(getParentComponent());
    if(isValid())
    {
        if(mp!=0)
        {
            mp->updateMapping();
        }
    }
//    mp->checkRevertable();
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="XYComponent" componentName=""
                 parentClasses="public Component" constructorParams="int x, int y"
                 variableInitialisers="" snapPixels="2" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="288" initialHeight="24">
  <BACKGROUND backgroundColour="60505"/>
  <TEXTEDITOR name="xIn" id="25daa0004c9d3328" memberName="xIn" virtualName="XYTextEditor"
              explicitFocusOrder="1" pos="142 2 46 16" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <TEXTEDITOR name="yIn" id="67c0056dad39c7a" memberName="yIn" virtualName="XYTextEditor"
              explicitFocusOrder="2" pos="80 2 46 16" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <TEXTBUTTON name="removeElementButton" id="dbab43aced73afd5" memberName="removeElementButton"
              virtualName="" explicitFocusOrder="0" pos="200 4 68 16" tooltip="Remove this key mapping"
              buttonText="Remove key" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
