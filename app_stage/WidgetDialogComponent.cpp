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
 Copyright 2010-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include "WidgetDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
WidgetDialogComponent::WidgetDialogComponent ()
{
    setName ("WidgetDialog");
    addAndMakeVisible (nameEditor = new TextEditor ("name editor"));
    nameEditor->setMultiLine (false);
    nameEditor->setReturnKeyStartsNewLine (false);
    nameEditor->setReadOnly (false);
    nameEditor->setScrollbarsShown (true);
    nameEditor->setCaretVisible (true);
    nameEditor->setPopupMenuEnabled (true);
    nameEditor->setText (String::empty);

    addAndMakeVisible (typeCombo = new ComboBox ("type combo"));
    typeCombo->setEditableText (false);
    typeCombo->setJustificationType (Justification::centredLeft);
    typeCombo->setTextWhenNothingSelected (String::empty);
    typeCombo->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    typeCombo->addListener (this);

    addAndMakeVisible (leftToggleButton = new ToggleButton ("left toggle button"));
    leftToggleButton->setButtonText (TRANS("left"));
    leftToggleButton->setRadioGroupId (1);
    leftToggleButton->addListener (this);
    leftToggleButton->setColour (ToggleButton::textColourId, Colour (0xff999999));

    addAndMakeVisible (rightToggleButton = new ToggleButton ("right toggle button"));
    rightToggleButton->setButtonText (TRANS("right"));
    rightToggleButton->setRadioGroupId (1);
    rightToggleButton->addListener (this);
    rightToggleButton->setColour (ToggleButton::textColourId, Colour (0xff999999));

    addAndMakeVisible (bottomToggleButton = new ToggleButton ("bottom toggle button"));
    bottomToggleButton->setButtonText (TRANS("bottom"));
    bottomToggleButton->setRadioGroupId (1);
    bottomToggleButton->addListener (this);
    bottomToggleButton->setColour (ToggleButton::textColourId, Colour (0xff999999));

    addAndMakeVisible (topToggleButton = new ToggleButton ("top toggle button"));
    topToggleButton->setButtonText (TRANS("top"));
    topToggleButton->setRadioGroupId (1);
    topToggleButton->addListener (this);
    topToggleButton->setToggleState (true, dontSendNotification);
    topToggleButton->setColour (ToggleButton::textColourId, Colour (0xff999999));

    addAndMakeVisible (typeLabel = new Label ("type label",
                                              TRANS("Type")));
    typeLabel->setFont (Font (15.00f, Font::plain));
    typeLabel->setJustificationType (Justification::centredLeft);
    typeLabel->setEditable (false, false, false);
    typeLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    typeLabel->setColour (TextEditor::textColourId, Colours::black);
    typeLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (nameLabel = new Label ("name label",
                                              TRANS("Name")));
    nameLabel->setFont (Font (15.00f, Font::plain));
    nameLabel->setJustificationType (Justification::centredLeft);
    nameLabel->setEditable (false, false, false);
    nameLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    nameLabel->setColour (TextEditor::textColourId, Colours::black);
    nameLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (positionLabel = new Label ("position label",
                                                  TRANS("Name position")));
    positionLabel->setFont (Font (15.00f, Font::plain));
    positionLabel->setJustificationType (Justification::centredLeft);
    positionLabel->setEditable (false, false, false);
    positionLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    positionLabel->setColour (TextEditor::textColourId, Colours::black);
    positionLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (enableToggleButton = new ToggleButton ("enable toggle button"));
    enableToggleButton->setButtonText (String::empty);
    enableToggleButton->addListener (this);
    enableToggleButton->setColour (ToggleButton::textColourId, Colour (0xff999999));

    addAndMakeVisible (enabledLabel = new Label ("enabled label",
                                                 TRANS("Enabled")));
    enabledLabel->setFont (Font (15.00f, Font::plain));
    enabledLabel->setJustificationType (Justification::centredLeft);
    enabledLabel->setEditable (false, false, false);
    enabledLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    enabledLabel->setColour (TextEditor::textColourId, Colours::black);
    enabledLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (minLabel = new Label ("min label",
                                             TRANS("Minimum")));
    minLabel->setFont (Font (15.00f, Font::plain));
    minLabel->setJustificationType (Justification::centredLeft);
    minLabel->setEditable (false, false, false);
    minLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    minLabel->setColour (TextEditor::textColourId, Colours::black);
    minLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (minEditor = new TextEditor ("min editor"));
    minEditor->setMultiLine (false);
    minEditor->setReturnKeyStartsNewLine (false);
    minEditor->setReadOnly (false);
    minEditor->setScrollbarsShown (false);
    minEditor->setCaretVisible (true);
    minEditor->setPopupMenuEnabled (true);
    minEditor->setText (String::empty);

    addAndMakeVisible (maxLabel = new Label ("max label",
                                             TRANS("Maximum")));
    maxLabel->setFont (Font (15.00f, Font::plain));
    maxLabel->setJustificationType (Justification::centredLeft);
    maxLabel->setEditable (false, false, false);
    maxLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    maxLabel->setColour (TextEditor::textColourId, Colours::black);
    maxLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (maxEditor = new TextEditor ("max editor"));
    maxEditor->setMultiLine (false);
    maxEditor->setReturnKeyStartsNewLine (false);
    maxEditor->setReadOnly (false);
    maxEditor->setScrollbarsShown (false);
    maxEditor->setCaretVisible (true);
    maxEditor->setPopupMenuEnabled (true);
    maxEditor->setText (String::empty);

    addAndMakeVisible (stepLabel = new Label ("step label",
                                              TRANS("Step")));
    stepLabel->setFont (Font (15.00f, Font::plain));
    stepLabel->setJustificationType (Justification::centredLeft);
    stepLabel->setEditable (false, false, false);
    stepLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    stepLabel->setColour (TextEditor::textColourId, Colours::black);
    stepLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (stepEditor = new TextEditor ("step editor"));
    stepEditor->setMultiLine (false);
    stepEditor->setReturnKeyStartsNewLine (false);
    stepEditor->setReadOnly (false);
    stepEditor->setScrollbarsShown (false);
    stepEditor->setCaretVisible (true);
    stepEditor->setPopupMenuEnabled (true);
    stepEditor->setText (String::empty);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (288, 315);


    //[Constructor] You can add your own custom stuff here..

    framework_ = 0;
    textPosition_ = "top";

    // the text editor should have explicit focus first
    nameEditor->setSelectAllWhenFocused(true);
    // text editor listener to capture return and escape
    nameEditor->addListener(this);

    userMin_ = 0;
    userStep_ = 0.000001;
    userMax_ = 1;
    userEnabled_ = false;

    minEditor->setSelectAllWhenFocused(true);
    minEditor->setInputRestrictions(8, "0123456789.-\n");
    minEditor->addListener(this);
    stepEditor->setSelectAllWhenFocused(true);
    stepEditor->setInputRestrictions(8, "0123456789.\n");
    stepEditor->addListener(this);
    maxEditor->setSelectAllWhenFocused(true);
    maxEditor->setInputRestrictions(8, "0123456789.-\n");
    maxEditor->addListener(this);

    //[/Constructor]
}

WidgetDialogComponent::~WidgetDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    nameEditor->removeListener(this);
    //[/Destructor_pre]

    nameEditor = nullptr;
    typeCombo = nullptr;
    leftToggleButton = nullptr;
    rightToggleButton = nullptr;
    bottomToggleButton = nullptr;
    topToggleButton = nullptr;
    typeLabel = nullptr;
    nameLabel = nullptr;
    positionLabel = nullptr;
    enableToggleButton = nullptr;
    enabledLabel = nullptr;
    minLabel = nullptr;
    minEditor = nullptr;
    maxLabel = nullptr;
    maxEditor = nullptr;
    stepLabel = nullptr;
    stepEditor = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void WidgetDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void WidgetDialogComponent::resized()
{
    nameEditor->setBounds (8, 40, 272, 24);
    typeCombo->setBounds (56, 167, 176, 24);
    leftToggleButton->setBounds (112, 104, 56, 24);
    rightToggleButton->setBounds (160, 104, 56, 24);
    bottomToggleButton->setBounds (136, 128, 72, 24);
    topToggleButton->setBounds (136, 80, 56, 24);
    typeLabel->setBounds (8, 167, 40, 24);
    nameLabel->setBounds (8, 8, 150, 24);
    positionLabel->setBounds (8, 72, 104, 24);
    enableToggleButton->setBounds (256, 280, 24, 24);
    enabledLabel->setBounds (192, 280, 64, 24);
    minLabel->setBounds (8, 208, 72, 24);
    minEditor->setBounds (8, 240, 80, 24);
    maxLabel->setBounds (189, 208, 75, 24);
    maxEditor->setBounds (189, 240, 80, 24);
    stepLabel->setBounds (99, 208, 56, 24);
    stepEditor->setBounds (99, 240, 80, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void WidgetDialogComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == typeCombo)
    {
        //[UserComboBoxCode_typeCombo] -- add your combo box handling code here..
        //[/UserComboBoxCode_typeCombo]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void WidgetDialogComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == leftToggleButton)
    {
        //[UserButtonCode_leftToggleButton] -- add your button handler code here..
        textPosition_ = "left";
        //[/UserButtonCode_leftToggleButton]
    }
    else if (buttonThatWasClicked == rightToggleButton)
    {
        //[UserButtonCode_rightToggleButton] -- add your button handler code here..
        textPosition_ = "right";
        //[/UserButtonCode_rightToggleButton]
    }
    else if (buttonThatWasClicked == bottomToggleButton)
    {
        //[UserButtonCode_bottomToggleButton] -- add your button handler code here..
        textPosition_ = "bottom";
        //[/UserButtonCode_bottomToggleButton]
    }
    else if (buttonThatWasClicked == topToggleButton)
    {
        //[UserButtonCode_topToggleButton] -- add your button handler code here..
        textPosition_ = "top";
        //[/UserButtonCode_topToggleButton]
    }
    else if (buttonThatWasClicked == enableToggleButton)
    {
        //[UserButtonCode_enableToggleButton] -- add your button handler code here..
        //[/UserButtonCode_enableToggleButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void WidgetDialogComponent::initialize(DialogFrameworkComponent* framework, WidgetComponent* widget)
{
    framework_ = framework;
    widget_ = widget;

    String name = widget->getWidgetName();
    String textPosition = widget->getTextPosition();
    String widgetType = widget->getWidgetType();
    String domainType = widget->getDomainType();

    bool connected = widget->getConnected();
    bool userEnabled_ = widget->getUserEnabled();
    enableToggleButton->setEnabled(connected);
    enabledLabel->setEnabled(connected);

    if(connected)
        // connected in EigenD so can change enable state
        enableToggleButton->setToggleState(userEnabled_, dontSendNotification);
    else
        // not connected in EigenD, so disabled and cannot be enabled
        enableToggleButton->setToggleState(false, dontSendNotification);


    nameEditor->setText(name, false);
    comboData_.clear();

    if (domainType=="bint" || domainType=="bfloat" || domainType=="bintn" || domainType=="bfloatn")
    {
        comboData_.add("horizontalSlider");
        typeCombo->addItem ("Horizontal Slider", comboData_.size());
        comboData_.add("verticalSlider");
        typeCombo->addItem ("Vertical Slider", comboData_.size());
        comboData_.add("rotaryKnob");
        typeCombo->addItem ("Rotary Knob", comboData_.size());
        typeCombo->setSelectedId(comboData_.size(), dontSendNotification);
        comboData_.add("incDecButtons");
        typeCombo->addItem ("-/+ Buttons", comboData_.size());

        setDoubleEditorValue(minEditor, widget->getUserMin());
        setDoubleEditorValue(stepEditor, widget->getUserStep());
        setDoubleEditorValue(maxEditor, widget->getUserMax());
    }
    else if(domainType == "bool" || domainType=="trigger")
    {
        comboData_.add("toggleButton");
        typeCombo->addItem ("Toggle Button", comboData_.size());
        typeCombo->setSelectedId(comboData_.size(), dontSendNotification);
        comboData_.add("triggerButton");
        typeCombo->addItem ("Trigger Button", comboData_.size());
        typeCombo->setEnabled(false);

        minEditor->setEnabled(false);
        minEditor->setVisible(false);
        minLabel->setVisible(false);
        maxEditor->setEnabled(false);
        maxEditor->setVisible(false);
        maxLabel->setVisible(false);
        stepEditor->setEnabled(false);
        stepEditor->setVisible(false);
        stepLabel->setVisible(false);
    }
    else
    {
        minEditor->setEnabled(false);
        minEditor->setVisible(false);
        minLabel->setVisible(false);
        maxEditor->setEnabled(false);
        maxEditor->setVisible(false);
        maxLabel->setVisible(false);
        stepEditor->setEnabled(false);
        stepEditor->setVisible(false);
        stepLabel->setVisible(false);
        typeCombo->setVisible(false);
        typeCombo->setEnabled(false);
        typeLabel->setVisible(false);
    }

    if (textPosition=="top")
        topToggleButton->setToggleState(true, dontSendNotification);
    if (textPosition=="bottom")
        bottomToggleButton->setToggleState(true, dontSendNotification);
    if (textPosition=="left")
        leftToggleButton->setToggleState(true, dontSendNotification);
    if (textPosition=="right")
        rightToggleButton->setToggleState(true, dontSendNotification);

    textPosition_ = textPosition;

    if(comboData_.size()>0)
    {
        for(int i=0;i<comboData_.size();i++)
        {
            if(comboData_[i] == widgetType)
            {
                typeCombo->setSelectedId(i+1);
            }
        }

        typeCombo->setEnabled(true);
    }
}

void WidgetDialogComponent::setFocusOrder(DialogFrameworkComponent* framework)
{
    // only OK button can be triggered with return
    nameEditor->setWantsKeyboardFocus(true);
    nameEditor->setExplicitFocusOrder(1);
    framework->getOKButton()->setWantsKeyboardFocus(false);
    framework->getCancelButton()->setWantsKeyboardFocus(false);
    leftToggleButton->setWantsKeyboardFocus(false);
    rightToggleButton->setWantsKeyboardFocus(false);
    topToggleButton->setWantsKeyboardFocus(false);
    bottomToggleButton->setWantsKeyboardFocus(false);
    enableToggleButton->setWantsKeyboardFocus(false);
    typeCombo->setWantsKeyboardFocus(true);
    typeCombo->setExplicitFocusOrder(2);
    minEditor->setExplicitFocusOrder(3);
    minEditor->setWantsKeyboardFocus(true);
    stepEditor->setExplicitFocusOrder(4);
    stepEditor->setWantsKeyboardFocus(true);
    maxEditor->setExplicitFocusOrder(5);
    maxEditor->setWantsKeyboardFocus(true);
}




String WidgetDialogComponent::getName()
{
    return nameEditor->getText();
}

String WidgetDialogComponent::getWidgetType()
{
    if(comboData_.size()>0)
    {
        return comboData_[typeCombo->getSelectedId()-1];
    }

    return widget_->getWidgetType();
}

String WidgetDialogComponent::getTextPosition()
{
    return textPosition_;
}

bool WidgetDialogComponent::getUserEnabled()
{
    if(enableToggleButton->isEnabled())
        return enableToggleButton->getToggleState();
    else
        return userEnabled_;
}

void WidgetDialogComponent::setDoubleEditorValue(TextEditor* editor, double value)
{
    // number of decimal places is 3
    String valueStr = String(value, 8);
    valueStr = valueStr.trimCharactersAtEnd("0");
    valueStr = valueStr.trimCharactersAtEnd(".");
    editor->setText(valueStr, true);
}

double WidgetDialogComponent::getUserMin()
{
    double userMin = minEditor->getText().getDoubleValue();
    userMin_ = userMin;
    return userMin_;
}

double WidgetDialogComponent::getUserStep()
{
    double userStep = stepEditor->getText().getDoubleValue();
    if (userStep==0)
        userStep = 0.000001;
    userStep_ = userStep;
    return userStep_;
}

double WidgetDialogComponent::getUserMax()
{
    double userMax = maxEditor->getText().getDoubleValue();
    userMax_ = userMax;
    return userMax_;
}

void WidgetDialogComponent::textEditorReturnKeyPressed(TextEditor& editor)
{
    if(framework_)
        framework_->exitModalState(1);
}

void WidgetDialogComponent::textEditorEscapeKeyPressed(TextEditor& editor)
{
    if(framework_)
        framework_->exitModalState(0);
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="WidgetDialogComponent" componentName="WidgetDialog"
                 parentClasses="public Component, public TextEditorListener" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="288" initialHeight="315">
  <BACKGROUND backgroundColour="0"/>
  <TEXTEDITOR name="name editor" id="9798f5ebc3105c74" memberName="nameEditor"
              virtualName="" explicitFocusOrder="0" pos="8 40 272 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <COMBOBOX name="type combo" id="a3c92041cfd8137c" memberName="typeCombo"
            virtualName="" explicitFocusOrder="0" pos="56 167 176 24" editable="0"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <TOGGLEBUTTON name="left toggle button" id="d11f6fd623df4358" memberName="leftToggleButton"
                virtualName="" explicitFocusOrder="0" pos="112 104 56 24" txtcol="ff999999"
                buttonText="left" connectedEdges="0" needsCallback="1" radioGroupId="1"
                state="0"/>
  <TOGGLEBUTTON name="right toggle button" id="f580c304418eb47c" memberName="rightToggleButton"
                virtualName="" explicitFocusOrder="0" pos="160 104 56 24" txtcol="ff999999"
                buttonText="right" connectedEdges="0" needsCallback="1" radioGroupId="1"
                state="0"/>
  <TOGGLEBUTTON name="bottom toggle button" id="dd4476449fc42440" memberName="bottomToggleButton"
                virtualName="" explicitFocusOrder="0" pos="136 128 72 24" txtcol="ff999999"
                buttonText="bottom" connectedEdges="0" needsCallback="1" radioGroupId="1"
                state="0"/>
  <TOGGLEBUTTON name="top toggle button" id="93d3343a472ed049" memberName="topToggleButton"
                virtualName="" explicitFocusOrder="0" pos="136 80 56 24" txtcol="ff999999"
                buttonText="top" connectedEdges="0" needsCallback="1" radioGroupId="1"
                state="1"/>
  <LABEL name="type label" id="8f708b6f5fd1337c" memberName="typeLabel"
         virtualName="" explicitFocusOrder="0" pos="8 167 40 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Type" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="name label" id="d5cd34efd972170" memberName="nameLabel"
         virtualName="" explicitFocusOrder="0" pos="8 8 150 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Name" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="position label" id="229c5c4387284fc5" memberName="positionLabel"
         virtualName="" explicitFocusOrder="0" pos="8 72 104 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Name position" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="enable toggle button" id="3e75316060b7e84a" memberName="enableToggleButton"
                virtualName="" explicitFocusOrder="0" pos="256 280 24 24" txtcol="ff999999"
                buttonText="" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <LABEL name="enabled label" id="27d226905df98f5" memberName="enabledLabel"
         virtualName="" explicitFocusOrder="0" pos="192 280 64 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Enabled" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="min label" id="3567fdccdc5daa92" memberName="minLabel"
         virtualName="" explicitFocusOrder="0" pos="8 208 72 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Minimum" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="min editor" id="f473f2fc073119b9" memberName="minEditor"
              virtualName="" explicitFocusOrder="0" pos="8 240 80 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="0"
              caret="1" popupmenu="1"/>
  <LABEL name="max label" id="351383bb716d4589" memberName="maxLabel"
         virtualName="" explicitFocusOrder="0" pos="189 208 75 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Maximum" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="max editor" id="a3eb8502c9660134" memberName="maxEditor"
              virtualName="" explicitFocusOrder="0" pos="189 240 80 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="0"
              caret="1" popupmenu="1"/>
  <LABEL name="step label" id="e19da5ddac3e9232" memberName="stepLabel"
         virtualName="" explicitFocusOrder="0" pos="99 208 56 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Step" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="step editor" id="eb32a0337e1022f" memberName="stepEditor"
              virtualName="" explicitFocusOrder="0" pos="99 240 80 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="0"
              caret="1" popupmenu="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
