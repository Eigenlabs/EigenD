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

#include "ClipEditorPanel.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ClipEditorPanel::ClipEditorPanel (ClipManagerBackend* backend)
{
    addAndMakeVisible (backButton = new TextButton ("backbutton"));
    backButton->setButtonText (TRANS("Back"));
    backButton->addListener (this);
    backButton->setColour (TextButton::buttonColourId, Colour (0xffc1c1c1));

    addAndMakeVisible (nameLabel = new Label ("nameLabel",
                                              TRANS("Blues cello loop")));
    nameLabel->setFont (Font (15.00f, Font::plain));
    nameLabel->setJustificationType (Justification::centredLeft);
    nameLabel->setEditable (false, false, false);
    nameLabel->setColour (Label::outlineColourId, Colour (0xffbababa));
    nameLabel->setColour (TextEditor::textColourId, Colours::black);
    nameLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (tagTable = new ClipEditTable (backend));
    tagTable->setName ("tagTable");


    //[UserPreSize]

    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ClipEditorPanel::~ClipEditorPanel()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    backButton = nullptr;
    nameLabel = nullptr;
    tagTable = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ClipEditorPanel::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.setColour (Colour (0xffababab));
    g.drawRoundedRectangle (4.0f, 4.0f, static_cast<float> (getWidth() - 8), static_cast<float> (getHeight() - 8), 10.000f, 1.000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ClipEditorPanel::resized()
{
    backButton->setBounds (20, getHeight() - 36, 116, 24);
    nameLabel->setBounds (proportionOfWidth (0.0349f), 16, proportionOfWidth (0.9285f), 24);
    tagTable->setBounds (proportionOfWidth (0.0416f), 52, proportionOfWidth (0.9301f), getHeight() - 102);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ClipEditorPanel::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == backButton)
    {
        //[UserButtonCode_backButton] -- add your button handler code here..
        ClipManagerWidget* p=findParentComponentOfClass<ClipManagerWidget>();
        if(p!=0)
        {
            p->showSelectorPanel();
        }

        //[/UserButtonCode_backButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ClipEditorPanel::setClip(Clip* clip)
{
    nameLabel->setText(clip->getName(),dontSendNotification);
    tagTable->clearSelection();
    std::cout<<"ClipEditorPanel:setClip"<<clip->toXml()->createDocument("")<<std::endl;

    //labelsLabel->setText(clip->getLabelText(),false);

    // XXX other properties?
    tagTable->setClip(clip);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ClipEditorPanel" componentName=""
                 parentClasses="public Component, public ClipSelector" constructorParams="ClipManagerBackend* backend"
                 variableInitialisers="" snapPixels="4" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffff">
    <ROUNDRECT pos="4 4 8M 8M" cornerSize="10" fill="solid: ffffff" hasStroke="1"
               stroke="1, mitered, butt" strokeColour="solid: ffababab"/>
  </BACKGROUND>
  <TEXTBUTTON name="backbutton" id="57499e50c57afb32" memberName="backButton"
              virtualName="" explicitFocusOrder="0" pos="20 36R 116 24" bgColOff="ffc1c1c1"
              buttonText="Back" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="nameLabel" id="37aa76fd270869f4" memberName="nameLabel"
         virtualName="" explicitFocusOrder="0" pos="3.534% 16 92.815% 24"
         outlineCol="ffbababa" edTextCol="ff000000" edBkgCol="0" labelText="Blues cello loop"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="tagTable" id="ce96b9cdd4cf2705" memberName="tagTable" virtualName="ClipEditTable"
                    explicitFocusOrder="0" pos="4.122% 52 93.051% 102M" class="Component"
                    params="backend"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
