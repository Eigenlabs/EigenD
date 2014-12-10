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

#include "TabDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
TabDialogComponent::TabDialogComponent ()
{
    setName ("TabDialog");
    addAndMakeVisible (nameEditor = new TextEditor ("name editor"));
    nameEditor->setMultiLine (false);
    nameEditor->setReturnKeyStartsNewLine (false);
    nameEditor->setReadOnly (false);
    nameEditor->setScrollbarsShown (true);
    nameEditor->setCaretVisible (true);
    nameEditor->setPopupMenuEnabled (true);
    nameEditor->setText (String::empty);

    addAndMakeVisible (aspectCombo = new ComboBox ("aspect combo"));
    aspectCombo->setEditableText (false);
    aspectCombo->setJustificationType (Justification::centredLeft);
    aspectCombo->setTextWhenNothingSelected (String::empty);
    aspectCombo->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    aspectCombo->addListener (this);

    addAndMakeVisible (canvasAspectLabel = new Label ("canvas aspect label",
                                                      TRANS("Canvas Aspect Ratio")));
    canvasAspectLabel->setFont (Font (15.00f, Font::plain));
    canvasAspectLabel->setJustificationType (Justification::centredLeft);
    canvasAspectLabel->setEditable (false, false, false);
    canvasAspectLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    canvasAspectLabel->setColour (TextEditor::textColourId, Colours::black);
    canvasAspectLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (nameLabel = new Label ("name label",
                                              TRANS("Name")));
    nameLabel->setFont (Font (15.00f, Font::plain));
    nameLabel->setJustificationType (Justification::centredLeft);
    nameLabel->setEditable (false, false, false);
    nameLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    nameLabel->setColour (TextEditor::textColourId, Colours::black);
    nameLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (customButton = new ToggleButton ("custom button"));
    customButton->setButtonText (TRANS("Use Custom Aspect Ratio"));
    customButton->addListener (this);
    customButton->setColour (ToggleButton::textColourId, Colour (0xffeeeeee));

    addAndMakeVisible (widthLabel = new Label ("width label",
                                               TRANS("width")));
    widthLabel->setFont (Font (15.00f, Font::plain));
    widthLabel->setJustificationType (Justification::centred);
    widthLabel->setEditable (false, false, false);
    widthLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    widthLabel->setColour (TextEditor::textColourId, Colours::black);
    widthLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (heightLabel = new Label ("height label",
                                                TRANS("height")));
    heightLabel->setFont (Font (15.00f, Font::plain));
    heightLabel->setJustificationType (Justification::centred);
    heightLabel->setEditable (false, false, false);
    heightLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    heightLabel->setColour (TextEditor::textColourId, Colours::black);
    heightLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (widthEditor = new TextEditor ("width editor"));
    widthEditor->setMultiLine (false);
    widthEditor->setReturnKeyStartsNewLine (false);
    widthEditor->setReadOnly (false);
    widthEditor->setScrollbarsShown (true);
    widthEditor->setCaretVisible (true);
    widthEditor->setPopupMenuEnabled (true);
    widthEditor->setText (String::empty);

    addAndMakeVisible (heightEditor = new TextEditor ("height editor"));
    heightEditor->setMultiLine (false);
    heightEditor->setReturnKeyStartsNewLine (false);
    heightEditor->setReadOnly (false);
    heightEditor->setScrollbarsShown (true);
    heightEditor->setCaretVisible (true);
    heightEditor->setPopupMenuEnabled (true);
    heightEditor->setText (String::empty);

    addAndMakeVisible (ratioLabel = new Label ("ratio label",
                                               TRANS(":")));
    ratioLabel->setFont (Font (15.00f, Font::plain));
    ratioLabel->setJustificationType (Justification::centred);
    ratioLabel->setEditable (false, false, false);
    ratioLabel->setColour (Label::textColourId, Colour (0xffeeeeee));
    ratioLabel->setColour (TextEditor::textColourId, Colours::black);
    ratioLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (288, 224);


    //[Constructor] You can add your own custom stuff here..

    aspectCombo->addItem("Monitor", 1);
    aspectCombo->addItem("Half Monitor Horizontal", 2);
    aspectCombo->addItem("Half Monitor Vertical", 3);
    aspectCombo->addSeparator();
    aspectCombo->addItem("Widescreen Monitor", 4);
    aspectCombo->addItem("Half Widescreen Monitor Horizontal", 5);
    aspectCombo->addItem("Half Widescreen Monitor Vertical", 6);
    aspectCombo->addSeparator();
    aspectCombo->addItem("iPhone/iPod 3.5\" Horizontal", 7);
    aspectCombo->addItem("iPhone/iPod 3.5\" Vertical", 8);
    aspectCombo->addItem("iPhone/iPod 4\", 4.7\", 5.5\" Horizontal", 9);
    aspectCombo->addItem("iPhone/iPod 4\", 4.7\", 5.5\" Vertical", 10);
    aspectCombo->addSeparator();
    aspectCombo->addItem("iPad Horizontal", 11);
    aspectCombo->addItem("iPad Vertical", 12);

    widthEditor->setInputRestrictions(5, "0123456789.\n");
    heightEditor->setInputRestrictions(5, "0123456789.\n");


    widthEditor->setSelectAllWhenFocused(true);
    heightEditor->setSelectAllWhenFocused(true);

    nameEditor->addListener(this);
    widthEditor->addListener(this);
    heightEditor->addListener(this);
    //[/Constructor]
}

TabDialogComponent::~TabDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    nameEditor->removeListener(this);
    widthEditor->removeListener(this);
    heightEditor->removeListener(this);
    //[/Destructor_pre]

    nameEditor = nullptr;
    aspectCombo = nullptr;
    canvasAspectLabel = nullptr;
    nameLabel = nullptr;
    customButton = nullptr;
    widthLabel = nullptr;
    heightLabel = nullptr;
    widthEditor = nullptr;
    heightEditor = nullptr;
    ratioLabel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void TabDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void TabDialogComponent::resized()
{
    nameEditor->setBounds (8, 40, 272, 24);
    aspectCombo->setBounds (8, 112, 272, 24);
    canvasAspectLabel->setBounds (8, 80, 136, 24);
    nameLabel->setBounds (8, 8, 150, 24);
    customButton->setBounds (8, 144, 184, 24);
    widthLabel->setBounds (64, 168, 64, 24);
    heightLabel->setBounds (160, 168, 64, 24);
    widthEditor->setBounds (64, 192, 64, 24);
    heightEditor->setBounds (160, 192, 64, 24);
    ratioLabel->setBounds (120, 192, 48, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void TabDialogComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == aspectCombo)
    {
        //[UserComboBoxCode_aspectCombo] -- add your combo box handling code here..
        aspectMode_ = aspectCombo->getSelectedId();

        switch (aspectMode_)
        {
            case 1:
                setWidthAspect(4);
                setHeightAspect(3);
                break;
            case 2:
                setWidthAspect(4);
                setHeightAspect(1.5);
                break;
            case 3:
                setWidthAspect(2);
                setHeightAspect(3);
                break;
            case 4:
                setWidthAspect(16);
                setHeightAspect(9);
                break;
            case 5:
                setWidthAspect(16);
                setHeightAspect(4.5);
                break;
            case 6:
                setWidthAspect(8);
                setHeightAspect(9);
                break;
            case 7:
                setWidthAspect(12);
                setHeightAspect(7);
                break;
            case 8:
                setWidthAspect(5);
                setHeightAspect(7);
                break;
            case 9:
                setWidthAspect(2);
                setHeightAspect(1);
                break;
            case 10:
                setWidthAspect(3);
                setHeightAspect(5);
                break;
            case 11:
                setWidthAspect(7);
                setHeightAspect(5);
                break;
            case 12:
                setWidthAspect(11);
                setHeightAspect(14);
                break;
        }

        //[/UserComboBoxCode_aspectCombo]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void TabDialogComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == customButton)
    {
        //[UserButtonCode_customButton] -- add your button handler code here..
        if (customButton->getToggleState())
        {
            aspectCombo->setEnabled(false);
            widthEditor->setEnabled(true);
            heightEditor->setEnabled(true);
            aspectCombo->setColour(ComboBox::backgroundColourId, Colour(0xffffffff).withMultipliedAlpha(0.75));
            widthEditor->setColour(TextEditor::backgroundColourId, Colour(0xffffffff));
            heightEditor->setColour(TextEditor::backgroundColourId, Colour(0xffffffff));
        }
        else
        {
            if (aspectMode_==11)
                aspectMode_=1;
            aspectCombo->setSelectedId(aspectMode_, dontSendNotification);
            aspectCombo->setEnabled(true);
            widthEditor->setEnabled(false);
            heightEditor->setEnabled(false);
            aspectMode_ = aspectCombo->getSelectedId();
            aspectCombo->setColour(ComboBox::backgroundColourId, Colour(0xffffffff));
            widthEditor->setColour(TextEditor::backgroundColourId, Colour(0xffffffff).withMultipliedAlpha(0.75));
            heightEditor->setColour(TextEditor::backgroundColourId, Colour(0xffffffff).withMultipliedAlpha(0.75));
        }

        //[/UserButtonCode_customButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void TabDialogComponent::initialize(DialogFrameworkComponent* framework, const String& name,
                                    float widthAspect, float heightAspect, int aspectMode)
{
    framework_ = framework;
    widthAspect_ = widthAspect;
    heightAspect_ = heightAspect;
    aspectMode_ = aspectMode;

    nameEditor->setText(name, false);

    if(aspectMode==13)
    {
        customButton->setToggleState(true, dontSendNotification);
        aspectCombo->setSelectedId(0, dontSendNotification);
        aspectCombo->setEnabled(false);
        widthEditor->setEnabled(true);
        heightEditor->setEnabled(true);
        aspectCombo->setColour(ComboBox::backgroundColourId, Colour(0xffffffff).withMultipliedAlpha(0.75));
        widthEditor->setColour(TextEditor::backgroundColourId, Colour(0xffffffff));
        heightEditor->setColour(TextEditor::backgroundColourId, Colour(0xffffffff));
    }
    else
    {
        customButton->setToggleState(false, dontSendNotification);
        aspectCombo->setSelectedId(aspectMode, dontSendNotification);
        aspectCombo->setEnabled(true);
        widthEditor->setEnabled(false);
        heightEditor->setEnabled(false);
        aspectCombo->setColour(ComboBox::backgroundColourId, Colour(0xffffffff));
        widthEditor->setColour(TextEditor::backgroundColourId, Colour(0xffffffff).withMultipliedAlpha(0.75));
        heightEditor->setColour(TextEditor::backgroundColourId, Colour(0xffffffff).withMultipliedAlpha(0.75));
    }

    // update the text editors
    setWidthAspect(widthAspect);
    setHeightAspect(heightAspect);
}

String TabDialogComponent::getName()
{
    return nameEditor->getText();
}

void TabDialogComponent::setWidthAspect(float widthAspect)
{
    String widthAspectStr = String(widthAspect, 3);
    widthAspectStr = widthAspectStr.trimCharactersAtEnd("0");
    widthAspectStr = widthAspectStr.trimCharactersAtEnd(".");
    widthEditor->setText(widthAspectStr, true);
}

void TabDialogComponent::setHeightAspect(float heightAspect)
{
    String heightAspectStr = String(heightAspect, 3);
    heightAspectStr = heightAspectStr.trimCharactersAtEnd("0");
    heightAspectStr = heightAspectStr.trimCharactersAtEnd(".");
    heightEditor->setText(heightAspectStr, true);
}

float TabDialogComponent::getWidthAspect()
{
    float widthAspect = widthEditor->getText().getFloatValue();
    if (widthAspect!=0)
        widthAspect_ = widthAspect;

    return widthAspect_;
}

float TabDialogComponent::getHeightAspect()
{
    float heightAspect = heightEditor->getText().getFloatValue();
    if (heightAspect!=0)
        heightAspect_ = heightAspect;

    return heightAspect_;
}

int TabDialogComponent::getAspectMode()
{
    if (customButton->getToggleState())
        return 11;
    else
        return aspectCombo->getSelectedId();

}

void TabDialogComponent::setFocusOrder(DialogFrameworkComponent* framework)
{
    // only OK button can be triggered with return
    nameEditor->setSelectAllWhenFocused(true);
    nameEditor->setWantsKeyboardFocus(true);
    nameEditor->setExplicitFocusOrder(1);
    aspectCombo->setWantsKeyboardFocus(true);
    aspectCombo->setExplicitFocusOrder(2);
    widthEditor->setWantsKeyboardFocus(true);
    widthEditor->setExplicitFocusOrder(3);
    heightEditor->setWantsKeyboardFocus(true);
    heightEditor->setExplicitFocusOrder(4);

    customButton->setWantsKeyboardFocus(false);
    framework->getOKButton()->setWantsKeyboardFocus(false);
    framework->getCancelButton()->setWantsKeyboardFocus(false);

}

void TabDialogComponent::textEditorReturnKeyPressed(TextEditor& editor)
{
    if(framework_)
        framework_->exitModalState(1);
}

void TabDialogComponent::textEditorEscapeKeyPressed(TextEditor& editor)
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

<JUCER_COMPONENT documentType="Component" className="TabDialogComponent" componentName="TabDialog"
                 parentClasses="public Component, public TextEditorListener" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="288" initialHeight="224">
  <BACKGROUND backgroundColour="0"/>
  <TEXTEDITOR name="name editor" id="9798f5ebc3105c74" memberName="nameEditor"
              virtualName="" explicitFocusOrder="0" pos="8 40 272 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <COMBOBOX name="aspect combo" id="a3c92041cfd8137c" memberName="aspectCombo"
            virtualName="" explicitFocusOrder="0" pos="8 112 272 24" editable="0"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <LABEL name="canvas aspect label" id="8f708b6f5fd1337c" memberName="canvasAspectLabel"
         virtualName="" explicitFocusOrder="0" pos="8 80 136 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Canvas Aspect Ratio"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="name label" id="d5cd34efd972170" memberName="nameLabel"
         virtualName="" explicitFocusOrder="0" pos="8 8 150 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="Name" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="custom button" id="3a7e3e4338909adb" memberName="customButton"
                virtualName="" explicitFocusOrder="0" pos="8 144 184 24" txtcol="ffeeeeee"
                buttonText="Use Custom Aspect Ratio" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
  <LABEL name="width label" id="7dbc2613987bcf07" memberName="widthLabel"
         virtualName="" explicitFocusOrder="0" pos="64 168 64 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="width" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
  <LABEL name="height label" id="33ab9ac01fa7df02" memberName="heightLabel"
         virtualName="" explicitFocusOrder="0" pos="160 168 64 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText="height" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
  <TEXTEDITOR name="width editor" id="510d70e08df79ce1" memberName="widthEditor"
              virtualName="" explicitFocusOrder="0" pos="64 192 64 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <TEXTEDITOR name="height editor" id="eef64dfb47d89752" memberName="heightEditor"
              virtualName="" explicitFocusOrder="0" pos="160 192 64 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <LABEL name="ratio label" id="40ba5f264987f385" memberName="ratioLabel"
         virtualName="" explicitFocusOrder="0" pos="120 192 48 24" textCol="ffeeeeee"
         edTextCol="ff000000" edBkgCol="0" labelText=":" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
