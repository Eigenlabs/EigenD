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

#include "KeyToCourseEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
KeyToCourseEditor::KeyToCourseEditor (Atom* atom, String name, ToolManager* tm)
    : atom_(atom), name_(name), tm_(tm)
{
    addAndMakeVisible (viewport = new Viewport ("new viewport"));
    viewport->setScrollBarsShown (true, false);

    addAndMakeVisible (addRow = new TextButton ("addRow"));
    addRow->setTooltip (TRANS("Add a new key "));
    addRow->setButtonText (TRANS("+"));
    addRow->addListener (this);

    addAndMakeVisible (revertButton = new TextButton ("revertButton"));
    revertButton->setButtonText (TRANS("Revert"));
    revertButton->addListener (this);

    addAndMakeVisible (clearButton = new TextButton ("clearButton"));
    clearButton->setButtonText (TRANS("Clear"));
    clearButton->addListener (this);

    addAndMakeVisible (rowLabel = new Label ("rowLabel",
                                             TRANS("Column")));
    rowLabel->setFont (Font (15.00f, Font::plain));
    rowLabel->setJustificationType (Justification::centredLeft);
    rowLabel->setEditable (false, false, false);
    rowLabel->setColour (Label::textColourId, Colours::gainsboro);
    rowLabel->setColour (TextEditor::textColourId, Colours::black);
    rowLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (CourseLabel = new Label ("CourseLabel",
                                                TRANS("Course")));
    CourseLabel->setFont (Font (15.00f, Font::plain));
    CourseLabel->setJustificationType (Justification::centredLeft);
    CourseLabel->setEditable (false, false, false);
    CourseLabel->setColour (Label::textColourId, Colours::gainsboro);
    CourseLabel->setColour (TextEditor::textColourId, Colours::black);
    CourseLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (columnLabel = new Label ("columnLabel",
                                                TRANS("Row")));
    columnLabel->setFont (Font (15.00f, Font::plain));
    columnLabel->setJustificationType (Justification::centredLeft);
    columnLabel->setEditable (false, false, false);
    columnLabel->setColour (Label::textColourId, Colours::gainsboro);
    columnLabel->setColour (TextEditor::textColourId, Colours::black);
    columnLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    // XXX
    String mapping=atom_->get_value();
    keyToCoursePanel_=new KeyToCoursePanel(atom_,mapping,this);
    viewport->setViewedComponent(keyToCoursePanel_);
    //[/UserPreSize]

    setSize (400, 600);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

KeyToCourseEditor::~KeyToCourseEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    viewport = nullptr;
    addRow = nullptr;
    revertButton = nullptr;
    clearButton = nullptr;
    rowLabel = nullptr;
    CourseLabel = nullptr;
    columnLabel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void KeyToCourseEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff303030));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void KeyToCourseEditor::resized()
{
    viewport->setBounds (24, 48, 350, 440);
    addRow->setBounds (314, 500, 24, 16);
    revertButton->setBounds (274, 528, 64, 24);
    clearButton->setBounds (176, 528, 64, 24);
    rowLabel->setBounds (56, 24, 58, 16);
    CourseLabel->setBounds (238, 24, 64, 16);
    columnLabel->setBounds (114, 24, 64, 16);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void KeyToCourseEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == addRow)
    {
        //[UserButtonCode_addRow] -- add your button handler code here..
        keyToCoursePanel_->addElement();
        //[/UserButtonCode_addRow]
    }
    else if (buttonThatWasClicked == revertButton)
    {
        //[UserButtonCode_revertButton] -- add your button handler code here..
        keyToCoursePanel_->revertMapping(atom_->get_value());
        //[/UserButtonCode_revertButton]
    }
    else if (buttonThatWasClicked == clearButton)
    {
        //[UserButtonCode_clearButton] -- add your button handler code here..
        bool doDelete=true;

        if(!tm_->getPropertyValue(String("KeyCourseMapDelete"),false))
        {
            DeleteMapConfirmation* da=new DeleteMapConfirmation();
            DialogWindow::showModalDialog("Clear key mapping",da,this,Colour(0xffababab),true);
            if(da->dontShowAgain_&& da->okPressed_)
            {
               tm_->setPropertyValue(String("KeyCourseMapDelete"),true);
            }

            doDelete=da->okPressed_;
            delete da;
        }

        if(doDelete)
        {
            keyToCoursePanel_->clearMapping();
        }


        //[/UserButtonCode_clearButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void KeyToCourseEditor::refreshValue()
{
}

void KeyToCourseEditor::handleCommandMessage(int commandId)
{
    //std::cout<<"KeyToCourseEditor:handleCommandMessage "<<commandId<<std::endl;
    if(commandId==1)
    {
    //revertable
        revertButton->setEnabled(true);
    }
    else if (commandId==0)
    {
    //not revertable
        revertButton->setEnabled(false);
    }
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="KeyToCourseEditor" componentName=""
                 parentClasses="public Component" constructorParams="Atom* atom, String name, ToolManager* tm"
                 variableInitialisers="atom_(atom), name_(name), tm_(tm)" snapPixels="2"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="400" initialHeight="600">
  <BACKGROUND backgroundColour="ff303030"/>
  <VIEWPORT name="new viewport" id="872fac1e89710e7f" memberName="viewport"
            virtualName="" explicitFocusOrder="0" pos="24 48 350 440" vscroll="1"
            hscroll="0" scrollbarThickness="18" contentType="0" jucerFile=""
            contentClass="" constructorParams=""/>
  <TEXTBUTTON name="addRow" id="edc8ef5048e47de2" memberName="addRow" virtualName=""
              explicitFocusOrder="0" pos="314 500 24 16" tooltip="Add a new key "
              buttonText="+" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="revertButton" id="9ea2843492452310" memberName="revertButton"
              virtualName="" explicitFocusOrder="0" pos="274 528 64 24" buttonText="Revert"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="clearButton" id="5687f39d2ca0e74e" memberName="clearButton"
              virtualName="" explicitFocusOrder="0" pos="176 528 64 24" buttonText="Clear"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="rowLabel" id="6ce85d06f902a233" memberName="rowLabel" virtualName=""
         explicitFocusOrder="0" pos="56 24 58 16" textCol="ffdcdcdc" edTextCol="ff000000"
         edBkgCol="0" labelText="Column" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <LABEL name="CourseLabel" id="d1165f8a484fea70" memberName="CourseLabel"
         virtualName="" explicitFocusOrder="0" pos="238 24 64 16" textCol="ffdcdcdc"
         edTextCol="ff000000" edBkgCol="0" labelText="Course" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="columnLabel" id="7a4cde037f733d98" memberName="columnLabel"
         virtualName="" explicitFocusOrder="0" pos="114 24 64 16" textCol="ffdcdcdc"
         edTextCol="ff000000" edBkgCol="0" labelText="Row" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
