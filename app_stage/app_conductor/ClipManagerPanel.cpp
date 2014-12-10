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

#include "ClipManagerPanel.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ClipManagerPanel::ClipManagerPanel (ClipManagerBackend* backend)
{
    addAndMakeVisible (fileBrowser = new FileBrowserComponent (13,File::nonexistent,0,0));
    fileBrowser->setName ("fileBrowser");

    addAndMakeVisible (clipSelectorPanel = new ClipLibraryTable (backend));
    clipSelectorPanel->setName ("clipSelectorPanel");

    addAndMakeVisible (poolButton = new TextButton ("poolbutton"));
    poolButton->setButtonText (TRANS("Add to clip pool"));
    poolButton->addListener (this);
    poolButton->setColour (TextButton::buttonColourId, Colour (0xffababab));

    addAndMakeVisible (selectionTypeButton = new TextButton ("selectionTypeButton"));
    selectionTypeButton->setButtonText (TRANS("Tags / File system"));
    selectionTypeButton->addListener (this);
    selectionTypeButton->setColour (TextButton::buttonColourId, Colour (0xffababab));

    addAndMakeVisible (clipManagerList = new ClipManagerList (backend));
    clipManagerList->setName ("clipManagerList");


    //[UserPreSize]
    poolButton->setEnabled(false);
    selectByTag_=true;
    fileBrowser->setVisible(false);
    selectionTypeButton->setVisible(false);
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ClipManagerPanel::~ClipManagerPanel()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    fileBrowser = nullptr;
    clipSelectorPanel = nullptr;
    poolButton = nullptr;
    selectionTypeButton = nullptr;
    clipManagerList = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ClipManagerPanel::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.setColour (Colour (0xffababab));
    g.drawRoundedRectangle (4.0f, 4.0f, static_cast<float> (getWidth() - 8), static_cast<float> (getHeight() - 8), 10.000f, 1.000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ClipManagerPanel::resized()
{
    fileBrowser->setBounds (16, 12, proportionOfWidth (0.5823f), getHeight() - 34);
    clipSelectorPanel->setBounds (16, 12, proportionOfWidth (0.5823f), getHeight() - 33);
    poolButton->setBounds (getWidth() - 9 - 116, getHeight() - 16 - 24, 116, 24);
    selectionTypeButton->setBounds (76, 12, 136, 24);
    clipManagerList->setBounds (proportionOfWidth (0.6672f), 12, proportionOfWidth (0.3102f), getHeight() - 59);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ClipManagerPanel::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == poolButton)
    {
        //[UserButtonCode_poolButton] -- add your button handler code here..
        clipManagerList->addToClipPool();
        //[/UserButtonCode_poolButton]
    }
    else if (buttonThatWasClicked == selectionTypeButton)
    {
        //[UserButtonCode_selectionTypeButton] -- add your button handler code here..
        if(selectByTag_)
        {
            selectByTag_=false;
            clipSelectorPanel->setVisible(false);
            fileBrowser->setVisible(true);

        }
        else
        {
            selectByTag_=true;
            clipSelectorPanel->setVisible(true);
            fileBrowser->setVisible(false);
        }
        //[/UserButtonCode_selectionTypeButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ClipManagerPanel::clipSelected(Clip* clip)
{
    if(clip==0)
    {
        poolButton->setEnabled(false);
    }
    else
    {
        poolButton->setEnabled(true);
    }
}

void ClipManagerPanel::tagSelectionChanged(XmlElement* e)
{
    std::cout<<"ClipManagerPanel::TagSelectionChanged"<<std::endl;
    // XXX send the new tags to the clipList
    clipManagerList->updateClips(e);
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ClipManagerPanel" componentName=""
                 parentClasses="public Component, public ClipSelector" constructorParams="ClipManagerBackend* backend"
                 variableInitialisers="" snapPixels="4" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffff">
    <ROUNDRECT pos="4 4 8M 8M" cornerSize="10" fill="solid: ffffff" hasStroke="1"
               stroke="1, mitered, butt" strokeColour="solid: ffababab"/>
  </BACKGROUND>
  <GENERICCOMPONENT name="fileBrowser" id="219610a7ba3acf00" memberName="fileBrowser"
                    virtualName="" explicitFocusOrder="0" pos="16 12 58.186% 34M"
                    class="FileBrowserComponent" params="13,File::nonexistent,0,0"/>
  <GENERICCOMPONENT name="clipSelectorPanel" id="f131eda074349971" memberName="clipSelectorPanel"
                    virtualName="ClipLibraryTable" explicitFocusOrder="0" pos="16 12 58.186% 33M"
                    class="Component" params="backend"/>
  <TEXTBUTTON name="poolbutton" id="8425568e92534038" memberName="poolButton"
              virtualName="" explicitFocusOrder="0" pos="9Rr 16Rr 116 24" bgColOff="ffababab"
              buttonText="Add to clip pool" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
  <TEXTBUTTON name="selectionTypeButton" id="af47dba87f52c6b7" memberName="selectionTypeButton"
              virtualName="" explicitFocusOrder="0" pos="76 12 136 24" bgColOff="ffababab"
              buttonText="Tags / File system" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
  <GENERICCOMPONENT name="clipManagerList" id="99bd514f935244b0" memberName="clipManagerList"
                    virtualName="" explicitFocusOrder="0" pos="66.667% 12 30.978% 59M"
                    class="ClipManagerList" params="backend"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
