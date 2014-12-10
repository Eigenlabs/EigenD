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

#include "CoursePanel.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
CourseTextEditor::CourseTextEditor(String name):TextEditor(name)
{
}

void CourseTextEditor::focusLost(FocusChangeType cause)
{
    TextEditor::focusLost(cause);
    //std::cout<<"FocusLost: cause="<<cause<<"  "<<std::string(getName().toUTF8())<<std::endl;
    CoursePanel* c=dynamic_cast<CoursePanel*>(getParentComponent());
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
CoursePanel::CoursePanel (int x, int y)
{
    setName ("CoursePanel");
    addAndMakeVisible (courseLabel = new Label ("courseLabel",
                                                TRANS("Course")));
    courseLabel->setFont (Font (15.00f, Font::plain));
    courseLabel->setJustificationType (Justification::centredLeft);
    courseLabel->setEditable (false, false, false);
    courseLabel->setColour (Label::textColourId, Colours::azure);
    courseLabel->setColour (TextEditor::textColourId, Colours::black);
    courseLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (courseBox = new CourseTextEditor ("courseBox"));
    courseBox->setMultiLine (false);
    courseBox->setReturnKeyStartsNewLine (false);
    courseBox->setReadOnly (false);
    courseBox->setScrollbarsShown (true);
    courseBox->setCaretVisible (true);
    courseBox->setPopupMenuEnabled (true);
    courseBox->setText (String::empty);

    addAndMakeVisible (removeButton = new TextButton ("removebutton"));
    removeButton->setButtonText (TRANS("Remove course"));
    removeButton->addListener (this);
    removeButton->setColour (TextButton::buttonColourId, Colour (0xffcdcdcd));

    addAndMakeVisible (breathButton = new ToggleButton ("breathButton"));
    breathButton->setButtonText (String::empty);
    breathButton->addListener (this);
    breathButton->setColour (ToggleButton::textColourId, Colour (0xfff0efef));

    addAndMakeVisible (breathLabel = new Label ("breathLabel",
                                                TRANS("Breath modulate")));
    breathLabel->setFont (Font (15.00f, Font::plain));
    breathLabel->setJustificationType (Justification::centredLeft);
    breathLabel->setEditable (false, false, false);
    breathLabel->setColour (Label::textColourId, Colours::azure);
    breathLabel->setColour (TextEditor::textColourId, Colours::black);
    breathLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (openKeyMusicLabel = new Label ("openKeyMusicLabel",
                                                      TRANS("Open key (musical)")));
    openKeyMusicLabel->setFont (Font (15.00f, Font::plain));
    openKeyMusicLabel->setJustificationType (Justification::centredLeft);
    openKeyMusicLabel->setEditable (false, false, false);
    openKeyMusicLabel->setColour (Label::textColourId, Colours::azure);
    openKeyMusicLabel->setColour (TextEditor::textColourId, Colours::black);
    openKeyMusicLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (openKeyPhysLabel = new Label ("openKeyPhyLabel",
                                                     TRANS("Open key (physical)")));
    openKeyPhysLabel->setFont (Font (15.00f, Font::plain));
    openKeyPhysLabel->setJustificationType (Justification::centredLeft);
    openKeyPhysLabel->setEditable (false, false, false);
    openKeyPhysLabel->setColour (Label::textColourId, Colours::azure);
    openKeyPhysLabel->setColour (TextEditor::textColourId, Colours::black);
    openKeyPhysLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (keyBox = new CourseTextEditor ("keyBox"));
    keyBox->setMultiLine (false);
    keyBox->setReturnKeyStartsNewLine (false);
    keyBox->setReadOnly (false);
    keyBox->setScrollbarsShown (true);
    keyBox->setCaretVisible (true);
    keyBox->setPopupMenuEnabled (true);
    keyBox->setText (TRANS("0"));

    addAndMakeVisible (rowLabel = new Label ("rowLabel",
                                             TRANS("Row")));
    rowLabel->setFont (Font (15.00f, Font::plain));
    rowLabel->setJustificationType (Justification::centredLeft);
    rowLabel->setEditable (false, false, false);
    rowLabel->setColour (Label::textColourId, Colours::azure);
    rowLabel->setColour (TextEditor::textColourId, Colours::black);
    rowLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (columnLabel = new Label ("columnLabel",
                                                TRANS("Column")));
    columnLabel->setFont (Font (15.00f, Font::plain));
    columnLabel->setJustificationType (Justification::centredLeft);
    columnLabel->setEditable (false, false, false);
    columnLabel->setColour (Label::textColourId, Colours::azure);
    columnLabel->setColour (TextEditor::textColourId, Colours::black);
    columnLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (rowBox = new CourseTextEditor ("rowBox"));
    rowBox->setMultiLine (false);
    rowBox->setReturnKeyStartsNewLine (false);
    rowBox->setReadOnly (false);
    rowBox->setScrollbarsShown (true);
    rowBox->setCaretVisible (true);
    rowBox->setPopupMenuEnabled (true);
    rowBox->setText (TRANS("0"));

    addAndMakeVisible (columnBox = new CourseTextEditor ("columnBox"));
    columnBox->setMultiLine (false);
    columnBox->setReturnKeyStartsNewLine (false);
    columnBox->setReadOnly (false);
    columnBox->setScrollbarsShown (true);
    columnBox->setCaretVisible (true);
    columnBox->setPopupMenuEnabled (true);
    columnBox->setText (TRANS("0"));

    addAndMakeVisible (groupComponent = new GroupComponent ("new group",
                                                            TRANS("Strum keys")));
    groupComponent->setColour (GroupComponent::outlineColourId, Colour (0x66e9e9e9));
    groupComponent->setColour (GroupComponent::textColourId, Colours::azure);

    addAndMakeVisible (rowLabel2 = new Label ("rowLabel",
                                              TRANS("Row")));
    rowLabel2->setFont (Font (15.00f, Font::plain));
    rowLabel2->setJustificationType (Justification::centredLeft);
    rowLabel2->setEditable (false, false, false);
    rowLabel2->setColour (Label::textColourId, Colours::azure);
    rowLabel2->setColour (TextEditor::textColourId, Colours::black);
    rowLabel2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (columnLabel2 = new Label ("columnLabel",
                                                 TRANS("Column")));
    columnLabel2->setFont (Font (15.00f, Font::plain));
    columnLabel2->setJustificationType (Justification::centredLeft);
    columnLabel2->setEditable (false, false, false);
    columnLabel2->setColour (Label::textColourId, Colours::azure);
    columnLabel2->setColour (TextEditor::textColourId, Colours::black);
    columnLabel2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (addKeyButton = new TextButton ("addKeyButton"));
    addKeyButton->setButtonText (TRANS("Add key"));
    addKeyButton->addListener (this);
    addKeyButton->setColour (TextButton::buttonColourId, Colour (0xffcdcdcd));

    addAndMakeVisible (viewport = new Viewport ("new viewport"));
    viewport->setScrollBarsShown (true, false);


    //[UserPreSize]
    removing=false;
    String keyMapping="[]";
    keyEditor_=new StrumKeyPanel(keyMapping,this);
    viewport->setViewedComponent(keyEditor_);
    //[/UserPreSize]

    setSize (450, 200);


    //[Constructor] You can add your own custom stuff here..
    setTopLeftPosition(x,y);
    //[/Constructor]
}

CoursePanel::~CoursePanel()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    courseLabel = nullptr;
    courseBox = nullptr;
    removeButton = nullptr;
    breathButton = nullptr;
    breathLabel = nullptr;
    openKeyMusicLabel = nullptr;
    openKeyPhysLabel = nullptr;
    keyBox = nullptr;
    rowLabel = nullptr;
    columnLabel = nullptr;
    rowBox = nullptr;
    columnBox = nullptr;
    groupComponent = nullptr;
    rowLabel2 = nullptr;
    columnLabel2 = nullptr;
    addKeyButton = nullptr;
    viewport = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void CoursePanel::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff303030));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void CoursePanel::resized()
{
    courseLabel->setBounds (4, 12, 56, 16);
    courseBox->setBounds (60, 12, 24, 16);
    removeButton->setBounds (272, 12, 100, 20);
    breathButton->setBounds (164, 138, 20, 20);
    breathLabel->setBounds (54, 140, 116, 16);
    openKeyMusicLabel->setBounds (54, 160, 128, 16);
    openKeyPhysLabel->setBounds (54, 180, 144, 16);
    keyBox->setBounds (184, 160, 27, 16);
    rowLabel->setBounds (308, 180, 36, 16);
    columnLabel->setBounds (200, 180, 60, 16);
    rowBox->setBounds (346, 180, 27, 16);
    columnBox->setBounds (260, 180, 27, 16);
    groupComponent->setBounds (58, 32, 318, 108);
    rowLabel2->setBounds (212, 40, 40, 20);
    columnLabel2->setBounds (150, 40, 64, 20);
    addKeyButton->setBounds (276, 112, 68, 16);
    viewport->setBounds (76, 60, 288, 44);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void CoursePanel::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == removeButton)
    {
        //[UserButtonCode_removeButton] -- add your button handler code here..
        CoursePanelContainer* mp=dynamic_cast<CoursePanelContainer*>(getParentComponent());
        if(mp!=0)
        {
            mp->removeElement(this);
        }

        //[/UserButtonCode_removeButton]
    }
    else if (buttonThatWasClicked == breathButton)
    {
        //[UserButtonCode_breathButton] -- add your button handler code here..
        updateMapping();
        //[/UserButtonCode_breathButton]
    }
    else if (buttonThatWasClicked == addKeyButton)
    {
        //[UserButtonCode_addKeyButton] -- add your button handler code here..
        keyEditor_->addElement();
        //[/UserButtonCode_addKeyButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
// XXX
void CoursePanel::updateMapping()
{
    CoursePanelContainer* mp=dynamic_cast<CoursePanelContainer*>(getParentComponent());
    if(mp!=0)
    {
        mp->updateMapping();
    }

}
String CoursePanel::makeMappingString()
{
    String s="[";
    String breathMod="false";
    if(breathButton->getToggleState())
    {
        breathMod="true";
    }
    s=s+courseBox->getText()+","+breathMod+","+keyBox->getText()+","+columnBox->getText()+","+rowBox->getText();
    s=s+","+keyEditor_->makeMappingString();
    s=s+"]";
    return s;
}

void CoursePanel::setValues(String val)
{
//  val is the course part of the full strummer mapping string
// parse the string
     String s=val.fromFirstOccurrenceOf("[",false,false);
     s=s.upToLastOccurrenceOf("]",false,false);
//  split into the data for the keyeditor and the data for the other various controls
     String courseData=s.upToFirstOccurrenceOf("[",false,false);
     String keyData=s.fromFirstOccurrenceOf("[",true,false);
//  set the values
    courseBox->setText(courseData.upToFirstOccurrenceOf(",",false,false),false);
    courseData=courseData.fromFirstOccurrenceOf(",",false,false);
    String breathMod=courseData.upToFirstOccurrenceOf(",",false,false);
    breathButton->setToggleState(breathMod.equalsIgnoreCase("true"),false);
    courseData=courseData.fromFirstOccurrenceOf(",",false,false);
    keyBox->setText(courseData.upToFirstOccurrenceOf(",",false,false),false);
    courseData=courseData.fromFirstOccurrenceOf(",",false,false);
    columnBox->setText(courseData.upToFirstOccurrenceOf(",",false,false),false);
    courseData=courseData.fromFirstOccurrenceOf(",",false,false);
    rowBox->setText(courseData.upToFirstOccurrenceOf(",",false,false),false);

//   set the keyeditor values
    keyEditor_->displayMapping(keyData);
}

bool CoursePanel::isValid()
{
    return courseBox->getText().isNotEmpty() &&
        keyBox->getText().isNotEmpty() &&
        rowBox->getText().isNotEmpty() &&
        columnBox->getText().isNotEmpty();
}

bool CoursePanel:: isEmpty()
{
    return courseBox->getText().isEmpty() &&
        keyBox->getText().isEmpty() &&
        rowBox->getText().isEmpty() &&
        columnBox->getText().isEmpty();
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CoursePanel" componentName="CoursePanel"
                 parentClasses="public Component" constructorParams="int x, int y"
                 variableInitialisers="" snapPixels="2" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="450" initialHeight="200">
  <BACKGROUND backgroundColour="ff303030"/>
  <LABEL name="courseLabel" id="b0b9126703e6241e" memberName="courseLabel"
         virtualName="" explicitFocusOrder="0" pos="4 12 56 16" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Course" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="courseBox" id="946d54030fe62ead" memberName="courseBox"
              virtualName="CourseTextEditor" explicitFocusOrder="0" pos="60 12 24 16"
              initialText="" multiline="0" retKeyStartsLine="0" readonly="0"
              scrollbars="1" caret="1" popupmenu="1"/>
  <TEXTBUTTON name="removebutton" id="6a67602b4bfff8c" memberName="removeButton"
              virtualName="" explicitFocusOrder="0" pos="272 12 100 20" bgColOff="ffcdcdcd"
              buttonText="Remove course" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
  <TOGGLEBUTTON name="breathButton" id="447a1cbc27034ce0" memberName="breathButton"
                virtualName="" explicitFocusOrder="0" pos="164 138 20 20" txtcol="fff0efef"
                buttonText="" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <LABEL name="breathLabel" id="8a51a9cbff05a719" memberName="breathLabel"
         virtualName="" explicitFocusOrder="0" pos="54 140 116 16" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Breath modulate"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="openKeyMusicLabel" id="2d42bb093d1ec4f8" memberName="openKeyMusicLabel"
         virtualName="" explicitFocusOrder="0" pos="54 160 128 16" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Open key (musical)"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="openKeyPhyLabel" id="a06d2b01bc58637c" memberName="openKeyPhysLabel"
         virtualName="" explicitFocusOrder="0" pos="54 180 144 16" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Open key (physical)"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="keyBox" id="274fd5bc902b8e3a" memberName="keyBox" virtualName="CourseTextEditor"
              explicitFocusOrder="0" pos="184 160 27 16" initialText="0" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="rowLabel" id="8a084615004b456a" memberName="rowLabel" virtualName=""
         explicitFocusOrder="0" pos="308 180 36 16" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Row" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="columnLabel" id="3a8d11b89ea23d1b" memberName="columnLabel"
         virtualName="" explicitFocusOrder="0" pos="200 180 60 16" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Column" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="rowBox" id="59cfef45517e3429" memberName="rowBox" virtualName="CourseTextEditor"
              explicitFocusOrder="0" pos="346 180 27 16" initialText="0" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
  <TEXTEDITOR name="columnBox" id="ed579e4d840a59ba" memberName="columnBox"
              virtualName="CourseTextEditor" explicitFocusOrder="0" pos="260 180 27 16"
              initialText="0" multiline="0" retKeyStartsLine="0" readonly="0"
              scrollbars="1" caret="1" popupmenu="1"/>
  <GROUPCOMPONENT name="new group" id="3c1eaf8694a9af6b" memberName="groupComponent"
                  virtualName="" explicitFocusOrder="0" pos="58 32 318 108" outlinecol="66e9e9e9"
                  textcol="fff0ffff" title="Strum keys"/>
  <LABEL name="rowLabel" id="121bbee4d6218427" memberName="rowLabel2"
         virtualName="" explicitFocusOrder="0" pos="212 40 40 20" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Row" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="columnLabel" id="948c338d66c7deb0" memberName="columnLabel2"
         virtualName="" explicitFocusOrder="0" pos="150 40 64 20" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Column" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="addKeyButton" id="2fb2993b78e12903" memberName="addKeyButton"
              virtualName="" explicitFocusOrder="0" pos="276 112 68 16" bgColOff="ffcdcdcd"
              buttonText="Add key" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <VIEWPORT name="new viewport" id="95027dc99bc54ab9" memberName="viewport"
            virtualName="" explicitFocusOrder="0" pos="76 60 288 44" vscroll="1"
            hscroll="0" scrollbarThickness="18" contentType="0" jucerFile=""
            contentClass="" constructorParams=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
