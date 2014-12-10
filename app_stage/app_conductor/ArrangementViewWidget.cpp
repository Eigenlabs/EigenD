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

#include "ArrangementViewWidget.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ArrangementViewWidget::ArrangementViewWidget (ArrangementBackend* backend, int mode)
    : backend_(backend), mode_(mode)
{
    addAndMakeVisible (duplicateButton = new TextButton ("duplicateButton"));
    duplicateButton->setButtonText (TRANS("Duplicate"));
    duplicateButton->addListener (this);
    duplicateButton->setColour (TextButton::buttonColourId, Colour (0xffbababa));

    addAndMakeVisible (removeButton = new TextButton ("removeButton"));
    removeButton->setButtonText (TRANS("Remove from arangement"));
    removeButton->addListener (this);
    removeButton->setColour (TextButton::buttonColourId, Colour (0xffababab));

    addAndMakeVisible (arrangementLabel = new Label ("arrangementLabel",
                                                     String::empty));
    arrangementLabel->setFont (Font (15.00f, Font::plain));
    arrangementLabel->setJustificationType (Justification::centredLeft);
    arrangementLabel->setEditable (false, false, false);
    arrangementLabel->setColour (TextEditor::textColourId, Colours::black);
    arrangementLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (arrangementList = new ArrangementList (backend_,mode_));
    arrangementList->setName ("arrangementList");

    addAndMakeVisible (scenePanel = new ScenePanel());
    scenePanel->setName ("scenePanel");


    //[UserPreSize]
    removeButton->setEnabled(false);
    duplicateButton->setEnabled(false);

    addAndMakeVisible(toolbar_=new Toolbar());
    toolbar_->setVertical(false);
    toolbar_->setColour(Toolbar::backgroundColourId, Colour(0xffababab));
    toolbar_->setColour(Toolbar::buttonMouseOverBackgroundColourId, Colour(0xffababab));
    toolbar_->setColour(Toolbar::buttonMouseDownBackgroundColourId, Colour(0xffababab));
    toolbar_->setStyle(Toolbar::textOnly);
    tm_=new SceneToolbarItemFactory(this);
    toolbar_->addDefaultItems(*tm_);
    setArrangementName(arrangementList->getArrangementName());


    //[/UserPreSize]

    setSize (700, 300);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ArrangementViewWidget::~ArrangementViewWidget()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    duplicateButton = nullptr;
    removeButton = nullptr;
    arrangementLabel = nullptr;
    arrangementList = nullptr;
    scenePanel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ArrangementViewWidget::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setColour (Colour (0xffababab));
    g.drawRoundedRectangle (4.0f, 4.0f, static_cast<float> (getWidth() - 8), static_cast<float> (getHeight() - 8), 10.000f, 1.000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ArrangementViewWidget::resized()
{
    duplicateButton->setBounds (16, getHeight() - 16 - 24, 122, 24);
    removeButton->setBounds (getWidth() - 16 - 170, getHeight() - 16 - 24, 170, 24);
    arrangementLabel->setBounds (68, 12, 112, 16);
    arrangementList->setBounds (16, 36, proportionOfWidth (0.4012f), getHeight() - 89);
    scenePanel->setBounds ((getWidth() / 2) + -7, 48, proportionOfWidth (0.4826f), getHeight() - 102);
    //[UserResized] Add your own custom resize handling here..

    toolbar_->setBounds(getWidth()-88,8,80,40);
    //[/UserResized]
}

void ArrangementViewWidget::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == duplicateButton)
    {
        //[UserButtonCode_duplicateButton] -- add your button handler code here..
        arrangementList->duplicate();
        //[/UserButtonCode_duplicateButton]
    }
    else if (buttonThatWasClicked == removeButton)
    {
        //[UserButtonCode_removeButton] -- add your button handler code here..
            arrangementList->removeFromArrangement();


        //[/UserButtonCode_removeButton]
    }

    //[UserbuttonClicked_Post]
    else
    {
        ToolbarItemComponent * tbi=dynamic_cast<ToolbarItemComponent*>(buttonThatWasClicked);
        if(tbi!=0)
        {
            if(tbi->getItemId()==1)
            {
                mode_=PLAYING;
            }
            else
            {
                mode_=LAST;
            }
            arrangementList->setMode(mode_);
            arrangementList->arrangementChanged(mode_);

            std::cout<<"Toolbar button clicked "<<tbi->getItemId()<<std::endl;
        }
    }


    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ArrangementViewWidget::setArrangementName(String name)
{
    arrangementLabel->setText(name,dontSendNotification);
}

void ArrangementViewWidget::clipSelected(Clip* clip)
{
    if(clip==0)
    {
        scenePanel->setClip(0);
        removeButton->setEnabled(false);
        duplicateButton->setEnabled(false);
    }
    else
    {
        scenePanel->setClip(clip);
        removeButton->setEnabled(true);
        duplicateButton->setEnabled(true);
    }
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ArrangementViewWidget" componentName=""
                 parentClasses="public Component, public ClipSelector" constructorParams="ArrangementBackend* backend, int mode"
                 variableInitialisers="backend_(backend), mode_(mode)" snapPixels="4"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="0"
                 initialWidth="700" initialHeight="300">
  <BACKGROUND backgroundColour="ffffffff">
    <ROUNDRECT pos="4 4 8M 8M" cornerSize="10" fill="solid: ffffff" hasStroke="1"
               stroke="1, mitered, butt" strokeColour="solid: ffababab"/>
  </BACKGROUND>
  <TEXTBUTTON name="duplicateButton" id="cd407349dd11df4d" memberName="duplicateButton"
              virtualName="" explicitFocusOrder="0" pos="16 16Rr 122 24" bgColOff="ffbababa"
              buttonText="Duplicate" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="removeButton" id="d16be70d6ab8561a" memberName="removeButton"
              virtualName="" explicitFocusOrder="0" pos="16Rr 16Rr 170 24"
              bgColOff="ffababab" buttonText="Remove from arangement" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <LABEL name="arrangementLabel" id="d9d07e59188b630b" memberName="arrangementLabel"
         virtualName="" explicitFocusOrder="0" pos="68 12 112 16" edTextCol="ff000000"
         edBkgCol="0" labelText="" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="arrangementList" id="927ea7fc96c07705" memberName="arrangementList"
                    virtualName="" explicitFocusOrder="0" pos="16 36 40.165% 89M"
                    class="ArrangementList" params="backend_,mode_"/>
  <GENERICCOMPONENT name="scenePanel" id="9db4da930daa2f74" memberName="scenePanel"
                    virtualName="" explicitFocusOrder="0" pos="-7C 48 48.292% 102M"
                    class="ScenePanel" params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
