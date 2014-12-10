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

#include "MusicalMappingComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MusicalMappingComponent::MusicalMappingComponent (Atom* atom, Component* editor)
    : atom_(atom), editor_(editor)
{
    addAndMakeVisible (viewport = new Viewport ("new viewport"));
    viewport->setScrollBarsShown (true, false);

    addAndMakeVisible (xInLabel = new Label ("xInLabel",
                                             TRANS("Course")));
    xInLabel->setFont (Font (15.00f, Font::plain));
    xInLabel->setJustificationType (Justification::centredLeft);
    xInLabel->setEditable (false, false, false);
    xInLabel->setColour (Label::textColourId, Colours::white);
    xInLabel->setColour (TextEditor::textColourId, Colours::black);
    xInLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (yInLabel = new Label ("yInLabel",
                                             TRANS("Key")));
    yInLabel->setFont (Font (15.00f, Font::plain));
    yInLabel->setJustificationType (Justification::centredLeft);
    yInLabel->setEditable (false, false, false);
    yInLabel->setColour (Label::textColourId, Colours::white);
    yInLabel->setColour (TextEditor::textColourId, Colours::black);
    yInLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (yOutLabel = new Label ("yOutLabel",
                                              TRANS("Key")));
    yOutLabel->setFont (Font (15.00f, Font::plain));
    yOutLabel->setJustificationType (Justification::centredLeft);
    yOutLabel->setEditable (false, false, false);
    yOutLabel->setColour (Label::textColourId, Colours::white);
    yOutLabel->setColour (TextEditor::textColourId, Colours::black);
    yOutLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (addElementButton = new TextButton ("addElementButton"));
    addElementButton->setTooltip (TRANS("Add a key mapping"));
    addElementButton->setButtonText (TRANS("+"));
    addElementButton->addListener (this);

    addAndMakeVisible (xOutLabel = new Label ("xOutLabel",
                                              TRANS("Course")));
    xOutLabel->setFont (Font (15.00f, Font::plain));
    xOutLabel->setJustificationType (Justification::centredLeft);
    xOutLabel->setEditable (false, false, false);
    xOutLabel->setColour (Label::textColourId, Colours::white);
    xOutLabel->setColour (TextEditor::textColourId, Colours::black);
    xOutLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    mappingPanel_=new MappingPanel(atom_,getMapping(),editor_);
    if(atom_->get_desc().containsIgnoreCase("physical"))
    {
        xInLabel->setText("Row",dontSendNotification);
        yInLabel->setText("Column",dontSendNotification);
        xOutLabel->setText("Row",dontSendNotification);
        yOutLabel->setText("Column",dontSendNotification);

    }
    viewport->setViewedComponent(mappingPanel_);
    //[/UserPreSize]

    setSize (370, 250);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

MusicalMappingComponent::~MusicalMappingComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    viewport = nullptr;
    xInLabel = nullptr;
    yInLabel = nullptr;
    yOutLabel = nullptr;
    addElementButton = nullptr;
    xOutLabel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MusicalMappingComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MusicalMappingComponent::resized()
{
    viewport->setBounds (8, 24, 360, proportionOfHeight (0.9002f));
    xInLabel->setBounds (13, 8, 56, 16);
    yInLabel->setBounds (71, 8, 56, 16);
    yOutLabel->setBounds (275, 8, 56, 16);
    addElementButton->setBounds (328, getHeight() - 9 - 16, 24, 16);
    xOutLabel->setBounds (226, 8, 50, 16);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void MusicalMappingComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == addElementButton)
    {
        //[UserButtonCode_addElementButton] -- add your button handler code here..
        mappingPanel_->addElement();
        //[/UserButtonCode_addElementButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
String MusicalMappingComponent::getMapping()
{
    return atom_->get_value();
}


void MusicalMappingComponent::setUpstreamKeys(String upstream)
{
    mappingPanel_->setUpstreamKeys(upstream);
}

void MusicalMappingComponent::changed()
{
    mappingPanel_->displayMapping(getMapping());
}

void MusicalMappingComponent::revert()
{
    mappingPanel_->revertMapping(getMapping());
}


void MusicalMappingComponent::clear()
{
    mappingPanel_->clearMapping();
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MusicalMappingComponent"
                 componentName="" parentClasses="public Component" constructorParams="Atom* atom, Component* editor"
                 variableInitialisers="atom_(atom), editor_(editor)" snapPixels="2"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="0"
                 initialWidth="370" initialHeight="250">
  <BACKGROUND backgroundColour="ffffff"/>
  <VIEWPORT name="new viewport" id="be831c399c5fc916" memberName="viewport"
            virtualName="" explicitFocusOrder="0" pos="8 24 360 90.023%"
            vscroll="1" hscroll="0" scrollbarThickness="18" contentType="0"
            jucerFile="" contentClass="" constructorParams=""/>
  <LABEL name="xInLabel" id="a9c977bd0212ff4a" memberName="xInLabel" virtualName=""
         explicitFocusOrder="0" pos="13 8 56 16" textCol="ffffffff" edTextCol="ff000000"
         edBkgCol="0" labelText="Course" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <LABEL name="yInLabel" id="f153c9b5247e07bf" memberName="yInLabel" virtualName=""
         explicitFocusOrder="0" pos="71 8 56 16" textCol="ffffffff" edTextCol="ff000000"
         edBkgCol="0" labelText="Key" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <LABEL name="yOutLabel" id="c4f22b7a9dd3c475" memberName="yOutLabel"
         virtualName="" explicitFocusOrder="0" pos="275 8 56 16" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Key" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="addElementButton" id="3a5d2db86c0cd49" memberName="addElementButton"
              virtualName="" explicitFocusOrder="0" pos="328 9Rr 24 16" tooltip="Add a key mapping"
              buttonText="+" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="xOutLabel" id="d4c93acf7b57bbbb" memberName="xOutLabel"
         virtualName="" explicitFocusOrder="0" pos="226 8 50 16" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Course" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
