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

#include "MappingEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MappingEditor::MappingEditor (Atom* atom, String name, ToolManager* tm)
    : atom_(atom), name_(name), tm_(tm)
{
    setName ("MappingEditor");
    addAndMakeVisible (groupComponent = new GroupComponent ("new group",
                                                            String::empty));
    groupComponent->setColour (GroupComponent::outlineColourId, Colour (0x66bcbbbb));

    addAndMakeVisible (mmc_ = new MusicalMappingComponent (atom_, this));
    mmc_->setName ("mmc_");

    addAndMakeVisible (chooseButton = new TextButton ("chooseButton"));
    chooseButton->setTooltip (TRANS("Switch the keyboard into choose mode "));
    chooseButton->setButtonText (TRANS("Choose using keyboard"));
    chooseButton->addListener (this);

    addAndMakeVisible (toggleButton = new ToggleButton ("boundingBoxButton"));
    toggleButton->setButtonText (TRANS("Physical mapping follows bounding box"));
    toggleButton->addListener (this);
    toggleButton->setToggleState (true, dontSendNotification);
    toggleButton->setColour (ToggleButton::textColourId, Colours::azure);

    addAndMakeVisible (slider = new Slider ("new slider"));
    slider->setTooltip (TRANS("Select the course for keyboard choose mode"));
    slider->setRange (1, 100, 1);
    slider->setSliderStyle (Slider::IncDecButtons);
    slider->setTextBoxStyle (Slider::TextBoxLeft, false, 40, 20);
    slider->addListener (this);

    addAndMakeVisible (label = new Label ("courseLabel",
                                          TRANS("Course number")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::azure);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (fetchButton = new TextButton ("fetchButton"));
    fetchButton->setTooltip (TRANS("Get the keys from upstream"));
    fetchButton->setButtonText (TRANS("Fetch"));
    fetchButton->addListener (this);

    addAndMakeVisible (revertButton = new TextButton ("revertButton"));
    revertButton->setTooltip (TRANS("Reset this editor to show the current stored values"));
    revertButton->setButtonText (TRANS("Revert"));
    revertButton->addListener (this);

    addAndMakeVisible (clearButton = new TextButton ("clearButton"));
    clearButton->setTooltip (TRANS("Reset this editor to an empty mapping"));
    clearButton->setButtonText (TRANS("Clear"));
    clearButton->addListener (this);


    //[UserPreSize]
    toggleButton->setVisible(false);
    mode_=0;

    //[/UserPreSize]

    setSize (400, 700);


    //[Constructor] You can add your own custom stuff here..
    if(atom_->get_name().contains("physical"))
    {
        groupComponent->setVisible(false);
        chooseButton->setVisible(false);
        slider->setVisible(false);
        label->setVisible(false);
        setSize (400, 660);
    }
    //[/Constructor]
}

MappingEditor::~MappingEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    groupComponent = nullptr;
    mmc_ = nullptr;
    chooseButton = nullptr;
    toggleButton = nullptr;
    slider = nullptr;
    label = nullptr;
    fetchButton = nullptr;
    revertButton = nullptr;
    clearButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MappingEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff303030));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MappingEditor::resized()
{
    groupComponent->setBounds (10, 648, 380, 48);
    mmc_->setBounds (16, 17, 368, getHeight() - 100);
    chooseButton->setBounds (228, 664, 150, 22);
    toggleButton->setBounds (14, 628, 248, 24);
    slider->setBounds (112, 664, 80, 22);
    label->setBounds (18, 668, 87, 16);
    fetchButton->setBounds (14, 628, 78, 22);
    revertButton->setBounds (296, 628, 88, 22);
    clearButton->setBounds (152, 628, 86, 22);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void MappingEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == chooseButton)
    {
        //[UserButtonCode_chooseButton] -- add your button handler code here..
        Atom* parent=atom_->get_parent();
        if(parent!=0)
        {
            if(mode_==0)
            {
                parent->invoke("choose",(int)slider->getValue());
                setToKeyboardMode();
            }
            else
            {
                int dummy=0;
                parent->invoke("unchoose",dummy);
                setToEditorMode();
            }
            delete parent;
        }

        //[/UserButtonCode_chooseButton]
    }
    else if (buttonThatWasClicked == toggleButton)
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == fetchButton)
    {
        //[UserButtonCode_fetchButton] -- add your button handler code here..
        Atom* parent=atom_->get_parent();
        if(parent!=0)
        {
            parent->getSourceKeys(atom_->get_id());
        }
        //[/UserButtonCode_fetchButton]
    }
    else if (buttonThatWasClicked == revertButton)
    {
        //[UserButtonCode_revertButton] -- add your button handler code here..
        mmc_->revert();
        //[/UserButtonCode_revertButton]
    }
    else if (buttonThatWasClicked == clearButton)
    {
        //[UserButtonCode_clearButton] -- add your button handler code here..
        bool doDelete=true;
        if(!tm_->getPropertyValue(String("MappingDelete"),false))
        {
            DeleteMapConfirmation* da=new DeleteMapConfirmation();
            DialogWindow::showModalDialog("Clear key mapping",da,this,Colour(0xffababab),true);
            if(da->dontShowAgain_&& da->okPressed_)
            {
               tm_->setPropertyValue(String("MappingDelete"),true);
            }

            doDelete=da->okPressed_;
            delete da;
        }

        if(doDelete)
        {
            mmc_->clear();
        }

        //[/UserButtonCode_clearButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void MappingEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == slider)
    {
        //[UserSliderCode_slider] -- add your slider handling code here..
        //[/UserSliderCode_slider]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void MappingEditor::handleCommandMessage(int commandId)
{
    //std::cout<<"MappingEditor:handleCommandMessage "<<commandId<<std::endl;
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

void MappingEditor::setUpstreamKeys(String keys)
{
    mmc_->setUpstreamKeys(keys);
}

void MappingEditor::refreshValue()
{
    if(mode_==1)
    {
        setToEditorMode();
    }
    mmc_->changed();
}

void MappingEditor::setToEditorMode()
{
    mode_=0;
    chooseButton->setButtonText ("Choose using keyboard");
    chooseButton->setTooltip ("Switch the keyboard into choose mode ");
    revertButton->setEnabled(true);
    fetchButton->setEnabled(true);
    clearButton->setEnabled(true);
    mmc_->setEnabled(true);
}

void MappingEditor::setToKeyboardMode()
{
    mode_=1;
    chooseButton->setButtonText ("End keyboard choosing");
    chooseButton->setTooltip ("Switch the keyboard out of choose mode");
    revertButton->setEnabled(false);
    fetchButton->setEnabled(false);
    clearButton->setEnabled(false);
    mmc_->setEnabled(false);
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MappingEditor" componentName="MappingEditor"
                 parentClasses="public Component" constructorParams="Atom* atom, String name, ToolManager* tm"
                 variableInitialisers="atom_(atom), name_(name), tm_(tm)" snapPixels="2"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="400" initialHeight="700">
  <BACKGROUND backgroundColour="ff303030"/>
  <GROUPCOMPONENT name="new group" id="e86044cfa2df87f1" memberName="groupComponent"
                  virtualName="" explicitFocusOrder="0" pos="10 648 380 48" outlinecol="66bcbbbb"
                  title=""/>
  <GENERICCOMPONENT name="mmc_" id="d8f171919ce5f456" memberName="mmc_" virtualName="MusicalMappingComponent"
                    explicitFocusOrder="0" pos="16 17 368 100M" class="Component"
                    params="atom_, this"/>
  <TEXTBUTTON name="chooseButton" id="2c27df547bb5a373" memberName="chooseButton"
              virtualName="" explicitFocusOrder="0" pos="228 664 150 22" tooltip="Switch the keyboard into choose mode "
              buttonText="Choose using keyboard" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
  <TOGGLEBUTTON name="boundingBoxButton" id="6bbaebe4a244b53" memberName="toggleButton"
                virtualName="" explicitFocusOrder="0" pos="14 628 248 24" txtcol="fff0ffff"
                buttonText="Physical mapping follows bounding box" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="1"/>
  <SLIDER name="new slider" id="45759a02aaab0045" memberName="slider" virtualName=""
          explicitFocusOrder="0" pos="112 664 80 22" tooltip="Select the course for keyboard choose mode"
          min="1" max="100" int="1" style="IncDecButtons" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="40" textBoxHeight="20" skewFactor="1"/>
  <LABEL name="courseLabel" id="4e6c2066ad60bdd7" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="18 668 87 16" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Course number" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="fetchButton" id="6be4a2e9422cbe3f" memberName="fetchButton"
              virtualName="" explicitFocusOrder="0" pos="14 628 78 22" tooltip="Get the keys from upstream"
              buttonText="Fetch" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="revertButton" id="bcb156508b77109b" memberName="revertButton"
              virtualName="" explicitFocusOrder="0" pos="296 628 88 22" tooltip="Reset this editor to show the current stored values"
              buttonText="Revert" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="clearButton" id="bdda765db625519e" memberName="clearButton"
              virtualName="" explicitFocusOrder="0" pos="152 628 86 22" tooltip="Reset this editor to an empty mapping"
              buttonText="Clear" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
