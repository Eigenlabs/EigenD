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

#include "DummyStageTab.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
DummyStageTab::DummyStageTab (Conductor* conductor)
{
    addAndMakeVisible (clipManagerWidget = new ClipManagerWidget (conductor->getClipManagerBackend()));
    clipManagerWidget->setName ("clipManagerWidget");

    addAndMakeVisible (clipPoolWidget = new ClipPoolWidget (conductor->getClipPoolBackend()));
    clipPoolWidget->setName ("clipPoolWidget");

    addAndMakeVisible (component = new SceneViewWidget (conductor->getSceneBackend(),0));
    component->setName ("new component");

    addAndMakeVisible (component2 = new ArrangementViewWidget (conductor->getArrangementBackend(), 0));
    component2->setName ("new component");

    addAndMakeVisible (setViewWidget = new SetViewWidget (conductor->getSetBackend()));
    setViewWidget->setName ("setViewWidget");


    //[UserPreSize]
    //[/UserPreSize]

    setSize (1000, 900);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

DummyStageTab::~DummyStageTab()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    clipManagerWidget = nullptr;
    clipPoolWidget = nullptr;
    component = nullptr;
    component2 = nullptr;
    setViewWidget = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void DummyStageTab::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void DummyStageTab::resized()
{
    clipManagerWidget->setBounds (0, 0, proportionOfWidth (0.4893f), proportionOfHeight (0.3009f));
    clipPoolWidget->setBounds (proportionOfWidth (0.4893f), 0, proportionOfWidth (0.4893f), proportionOfHeight (0.3009f));
    component->setBounds (0, proportionOfHeight (0.3009f), proportionOfWidth (0.4893f), proportionOfHeight (0.3009f));
    component2->setBounds (proportionOfWidth (0.4893f), proportionOfHeight (0.3009f), proportionOfWidth (0.4893f), proportionOfHeight (0.3009f));
    setViewWidget->setBounds (0, proportionOfHeight (0.5996f), proportionOfWidth (0.4893f), proportionOfHeight (0.3009f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="DummyStageTab" componentName=""
                 parentClasses="public Component, public DragAndDropContainer"
                 constructorParams="Conductor* conductor" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="1000" initialHeight="900">
  <BACKGROUND backgroundColour="ffffffff"/>
  <GENERICCOMPONENT name="clipManagerWidget" id="9930dc0c1bcfe0fa" memberName="clipManagerWidget"
                    virtualName="" explicitFocusOrder="0" pos="0 0 48.881% 30.046%"
                    class="ClipManagerWidget" params="conductor-&gt;getClipManagerBackend()"/>
  <GENERICCOMPONENT name="clipPoolWidget" id="86caa151a6f5dc4e" memberName="clipPoolWidget"
                    virtualName="" explicitFocusOrder="0" pos="48.881% 0 48.881% 30.046%"
                    class="ClipPoolWidget" params="conductor-&gt;getClipPoolBackend()"/>
  <GENERICCOMPONENT name="new component" id="32de6347681d8375" memberName="component"
                    virtualName="" explicitFocusOrder="0" pos="0 30.046% 48.881% 30.046%"
                    class="SceneViewWidget" params="conductor-&gt;getSceneBackend(),0"/>
  <GENERICCOMPONENT name="new component" id="61634f269ee80889" memberName="component2"
                    virtualName="" explicitFocusOrder="0" pos="48.881% 30.046% 48.881% 30.046%"
                    class="ArrangementViewWidget" params="conductor-&gt;getArrangementBackend(), 0"/>
  <GENERICCOMPONENT name="setViewWidget" id="3b3d75b77ff41c8d" memberName="setViewWidget"
                    virtualName="" explicitFocusOrder="0" pos="0 59.977% 48.881% 30.046%"
                    class="SetViewWidget" params="conductor-&gt;getSetBackend()"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
