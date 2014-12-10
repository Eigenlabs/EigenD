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

#include "SceneViewWidget.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
SceneViewWidget::SceneViewWidget (SceneBackend* backend, int mode)
    : backend_(backend), mode_(mode)
{
    addAndMakeVisible (sceneLabel = new Label ("sceneLabel",
                                               String::empty));
    sceneLabel->setFont (Font (15.00f, Font::plain));
    sceneLabel->setJustificationType (Justification::centred);
    sceneLabel->setEditable (false, false, false);
    sceneLabel->setColour (TextEditor::textColourId, Colours::black);
    sceneLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (duplicateButton = new TextButton ("duplicateButton"));
    duplicateButton->setButtonText (TRANS("Duplicate"));
    duplicateButton->addListener (this);
    duplicateButton->setColour (TextButton::buttonColourId, Colour (0xffababab));

    addAndMakeVisible (removeButton = new TextButton ("removebutton"));
    removeButton->setButtonText (TRANS("Remove from scene"));
    removeButton->addListener (this);
    removeButton->setColour (TextButton::buttonColourId, Colour (0xffbababa));

    addAndMakeVisible (sceneList = new SceneList (backend_, mode_));
    sceneList->setName ("sceneList");

    addAndMakeVisible (clipPanel = new ClipPanel());
    clipPanel->setName ("clipPanel");


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


    //[/UserPreSize]

    setSize (700, 300);


    //[Constructor] You can add your own custom stuff here..
    setSceneName(sceneList->getSceneName());
    //[/Constructor]
}

SceneViewWidget::~SceneViewWidget()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    sceneLabel = nullptr;
    duplicateButton = nullptr;
    removeButton = nullptr;
    sceneList = nullptr;
    clipPanel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SceneViewWidget::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setColour (Colour (0xffababab));
    g.drawRoundedRectangle (4.0f, 4.0f, static_cast<float> (getWidth() - 8), static_cast<float> (getHeight() - 8), 10.000f, 1.000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SceneViewWidget::resized()
{
    sceneLabel->setBounds (16, 12, proportionOfWidth (0.4012f), 20);
    duplicateButton->setBounds (16, getHeight() - 16 - 24, 110, 24);
    removeButton->setBounds (getWidth() - 16 - 126, getHeight() - 16 - 24, 126, 24);
    sceneList->setBounds (16, 40, proportionOfWidth (0.4012f), getHeight() - 100);
    clipPanel->setBounds ((getWidth() / 2) + -7, 48, proportionOfWidth (0.4680f), getHeight() - 108);
    //[UserResized] Add your own custom resize handling here..
    toolbar_->setBounds(getWidth()-88,8,80,40);
    //[/UserResized]
}

void SceneViewWidget::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == duplicateButton)
    {
        //[UserButtonCode_duplicateButton] -- add your button handler code here..
        sceneList->duplicate();
        //[/UserButtonCode_duplicateButton]
    }
    else if (buttonThatWasClicked == removeButton)
    {
        //[UserButtonCode_removeButton] -- add your button handler code here..
        // XXX
        //ScenePanel* sp =dynamic_cast<ScenePanel*>(viewport->getViewedComponent());
        //if (sp!=0)
       // {
            sceneList->removeFromScene();
       // }


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
            sceneList->setMode(mode_);
            sceneList->sceneChanged(mode_);

            std::cout<<"Toolbar button clicked "<<tbi->getItemId()<<std::endl;
        }
    }

    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void SceneViewWidget::setSceneName(String name)
{
    sceneLabel->setText(name,dontSendNotification);
}

void SceneViewWidget::clipSelected(Clip* clip)
{
    if(clip==0)
    {
        clipPanel->setClip(0);
        removeButton->setEnabled(false);
        duplicateButton->setEnabled(false);
    }
    else
    {
        clipPanel->setClip(clip);
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

<JUCER_COMPONENT documentType="Component" className="SceneViewWidget" componentName=""
                 parentClasses="public Component, public ClipSelector" constructorParams="SceneBackend* backend, int mode"
                 variableInitialisers="backend_(backend), mode_(mode)" snapPixels="4"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="0"
                 initialWidth="700" initialHeight="300">
  <BACKGROUND backgroundColour="ffffffff">
    <ROUNDRECT pos="4 4 8M 8M" cornerSize="10" fill="solid: ffffff" hasStroke="1"
               stroke="1, mitered, butt" strokeColour="solid: ffababab"/>
  </BACKGROUND>
  <LABEL name="sceneLabel" id="d1bc1f0bfe07980" memberName="sceneLabel"
         virtualName="" explicitFocusOrder="0" pos="16 12 40.165% 20"
         edTextCol="ff000000" edBkgCol="0" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
  <TEXTBUTTON name="duplicateButton" id="a7e34b647e065b91" memberName="duplicateButton"
              virtualName="" explicitFocusOrder="0" pos="16 16Rr 110 24" bgColOff="ffababab"
              buttonText="Duplicate" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="removebutton" id="28fffdd000a299a4" memberName="removeButton"
              virtualName="" explicitFocusOrder="0" pos="16Rr 16Rr 126 24"
              bgColOff="ffbababa" buttonText="Remove from scene" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <GENERICCOMPONENT name="sceneList" id="2be42be6444446d0" memberName="sceneList"
                    virtualName="" explicitFocusOrder="0" pos="16 40 40.165% 100M"
                    class="SceneList" params="backend_, mode_"/>
  <GENERICCOMPONENT name="clipPanel" id="ce382b956e9f4cd" memberName="clipPanel"
                    virtualName="" explicitFocusOrder="0" pos="-7C 48 46.761% 108M"
                    class="ClipPanel" params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
