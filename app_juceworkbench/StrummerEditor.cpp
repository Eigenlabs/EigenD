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

#include "StrummerEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
StrummerEditor::StrummerEditor (Atom* atom, String name, ToolManager* tm)
    : atom_(atom), name_(name), tm_(tm)
{
    addAndMakeVisible (viewport = new Viewport ("new viewport"));
    viewport->setScrollBarsShown (true, false);

    addAndMakeVisible (addCourse = new TextButton ("addCourse"));
    addCourse->setTooltip (TRANS("Add a course"));
    addCourse->setButtonText (TRANS("Add course"));
    addCourse->addListener (this);

    addAndMakeVisible (revertButton = new TextButton ("revertButton"));
    revertButton->setButtonText (TRANS("Revert"));
    revertButton->addListener (this);

    addAndMakeVisible (clearButton = new TextButton ("clearButton"));
    clearButton->setButtonText (TRANS("Clear"));
    clearButton->addListener (this);

    addAndMakeVisible (okButton = new TextButton ("okButton"));
    okButton->setButtonText (TRANS("OK"));
    okButton->addListener (this);


    //[UserPreSize]
    // not implemented
    revertButton->setVisible(false);
    clearButton->setVisible(false);
    String mapping=atom_->get_value();
    //test string
    //String mapping=T("[[1,true,0,1,1,[[1,2],[2,2]]],[2,false,0,1,1,[[1,3]]],[3,false,0,1,1,[[1,4]]]]");
    courseEditor_=new CoursePanelContainer(atom_,mapping,this);
    viewport->setViewedComponent(courseEditor_);
    //[/UserPreSize]

    setSize (450, 630);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

StrummerEditor::~StrummerEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    viewport = nullptr;
    addCourse = nullptr;
    revertButton = nullptr;
    clearButton = nullptr;
    okButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void StrummerEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff303030));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void StrummerEditor::resized()
{
    viewport->setBounds (20, 18, 408, 526);
    addCourse->setBounds (334, 558, 90, 24);
    revertButton->setBounds (168, 558, 64, 24);
    clearButton->setBounds (254, 558, 64, 24);
    okButton->setBounds (334, 592, 90, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void StrummerEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == addCourse)
    {
        //[UserButtonCode_addCourse] -- add your button handler code here..
        courseEditor_->addElement();
        //[/UserButtonCode_addCourse]
    }
    else if (buttonThatWasClicked == revertButton)
    {
        //[UserButtonCode_revertButton] -- add your button handler code here..
        courseEditor_->revertMapping(atom_->get_value());
        //[/UserButtonCode_revertButton]
    }
    else if (buttonThatWasClicked == clearButton)
    {
        //[UserButtonCode_clearButton] -- add your button handler code here..
// clearButton not implemented yet
//        bool doDelete=true;
//
//        if(!tm_->getPropertyValue(String("KeyCourseMapDelete"),false))
//        {
//            DeleteMapConfirmation* da=new DeleteMapConfirmation();
//            DialogWindow::showModalDialog(T("Clear key mapping"),da,this,Colour(0xffababab),true);
//            if(da->dontShowAgain_&& da->okPressed_)
//            {
//               tm_->setPropertyValue(String("KeyCourseMapDelete"),true);
//            }
//
//            doDelete=da->okPressed_;
//            delete da;
//        }
//
//        if(doDelete)
//        {
//            courseEditor_->clearMapping();
//        }
//

        //[/UserButtonCode_clearButton]
    }
    else if (buttonThatWasClicked == okButton)
    {
        //[UserButtonCode_okButton] -- add your button handler code here..
        DialogWindow* dw =findParentComponentOfClass<DialogWindow>();
        if(dw!=0)
        {
            dw->exitModalState(0);
        }


        //[/UserButtonCode_okButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void StrummerEditor::refreshValue()
{
    courseEditor_->changed();
}

void StrummerEditor::handleCommandMessage(int commandId)
{
    //std::cout<<"StrummerEditor:handleCommandMessage "<<commandId<<std::endl;
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

<JUCER_COMPONENT documentType="Component" className="StrummerEditor" componentName=""
                 parentClasses="public Component" constructorParams="Atom* atom, String name, ToolManager* tm"
                 variableInitialisers="atom_(atom), name_(name), tm_(tm)" snapPixels="2"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="450" initialHeight="630">
  <BACKGROUND backgroundColour="ff303030"/>
  <VIEWPORT name="new viewport" id="872fac1e89710e7f" memberName="viewport"
            virtualName="" explicitFocusOrder="0" pos="20 18 408 526" vscroll="1"
            hscroll="0" scrollbarThickness="18" contentType="0" jucerFile=""
            contentClass="" constructorParams=""/>
  <TEXTBUTTON name="addCourse" id="edc8ef5048e47de2" memberName="addCourse"
              virtualName="" explicitFocusOrder="0" pos="334 558 90 24" tooltip="Add a course"
              buttonText="Add course" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
  <TEXTBUTTON name="revertButton" id="9ea2843492452310" memberName="revertButton"
              virtualName="" explicitFocusOrder="0" pos="168 558 64 24" buttonText="Revert"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="clearButton" id="5687f39d2ca0e74e" memberName="clearButton"
              virtualName="" explicitFocusOrder="0" pos="254 558 64 24" buttonText="Clear"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="okButton" id="6885a23a92d1ebd2" memberName="okButton" virtualName=""
              explicitFocusOrder="0" pos="334 592 90 24" buttonText="OK" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
