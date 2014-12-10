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

#include "ClipManagerWidget.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ClipManagerWidget::ClipManagerWidget (ClipManagerBackend* backend)
    : backend_(backend)
{
    addAndMakeVisible (clipEditorPanel = new ClipEditorPanel (backend));
    clipEditorPanel->setExplicitFocusOrder (1);
    clipEditorPanel->setName ("clipEditorPanel");

    addAndMakeVisible (clipManagerPanel = new ClipManagerPanel (backend));
    clipManagerPanel->setName ("clipManagerPanel");


    //[UserPreSize]
    clipEditorPanel->setVisible(false);
    //[/UserPreSize]

    setSize (700, 300);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ClipManagerWidget::~ClipManagerWidget()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    clipEditorPanel = nullptr;
    clipManagerPanel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ClipManagerWidget::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0x02ffffff));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ClipManagerWidget::resized()
{
    clipEditorPanel->setBounds (0, 0, proportionOfWidth (1.0000f), proportionOfHeight (1.0000f));
    clipManagerPanel->setBounds (0, 0, proportionOfWidth (1.0000f), proportionOfHeight (1.0000f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ClipManagerWidget::switchPanel(String panel,Clip* clip)
{
    if(panel.equalsIgnoreCase("clipEditor"))
    {
        showEditorPanel(clip);
    }
    else if(panel.equalsIgnoreCase("clipSelector"))
    {
        showSelectorPanel();
    }
}

void ClipManagerWidget::showEditorPanel(Clip* clip)
{

    clipManagerPanel->setVisible(false);
    clipEditorPanel->setClip(clip);
    clipEditorPanel->setVisible(true);
}

void ClipManagerWidget::showSelectorPanel()
{
    clipEditorPanel->setVisible(false);
    clipManagerPanel->setVisible(true);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ClipManagerWidget" componentName=""
                 parentClasses="public Component, public ClipPanelSwitcher" constructorParams="ClipManagerBackend* backend"
                 variableInitialisers="backend_(backend)" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="0" initialWidth="700"
                 initialHeight="300">
  <BACKGROUND backgroundColour="2ffffff"/>
  <GENERICCOMPONENT name="clipEditorPanel" id="4374e1a47923b5cc" memberName="clipEditorPanel"
                    virtualName="ClipEditorPanel" explicitFocusOrder="1" pos="0 0 100% 100%"
                    class="Component" params="backend"/>
  <GENERICCOMPONENT name="clipManagerPanel" id="920904055182120f" memberName="clipManagerPanel"
                    virtualName="ClipManagerPanel" explicitFocusOrder="0" pos="0 0 100% 100%"
                    class="Component" params="backend"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
