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

#include "BackgroundComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
BackgroundComponent::BackgroundComponent ()
{
    cachedImage_dots_png = ImageCache::getFromMemory (dots_png, dots_pngSize);

    //[UserPreSize]
    //[/UserPreSize]

    setSize (96, 64);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

BackgroundComponent::~BackgroundComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void BackgroundComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::black);

    g.setColour (Colours::black);
    g.drawImage (cachedImage_dots_png,
                 85, 22, 11, 31,
                 0, 0, cachedImage_dots_png.getWidth(), cachedImage_dots_png.getHeight());

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void BackgroundComponent::resized()
{
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

<JUCER_COMPONENT documentType="Component" className="BackgroundComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="96" initialHeight="64">
  <BACKGROUND backgroundColour="ff000000">
    <IMAGE pos="85 22 11 31" resource="dots_png" opacity="1" mode="0"/>
  </BACKGROUND>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: dots_png, 225, "graphics/dots.png"
static const unsigned char resource_BackgroundComponent_dots_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,10,0,0,0,30,8,2,0,0,0,154,23,24,103,0,0,0,1,115,82,71,66,0,174,206,28,233,0,0,
0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,7,116,73,77,69,7,218,6,23,14,11,0,212,149,251,174,0,0,0,115,73,68,65,84,40,207,157,82,201,13,192,48,8,131,238,47,101,1,70,96,7,118,96,135,108,
144,127,250,104,85,69,17,225,168,95,17,216,14,32,3,132,64,196,100,113,103,92,43,43,80,204,57,235,115,120,158,107,111,140,1,73,188,178,212,126,207,163,247,14,53,100,207,130,136,170,234,181,11,99,26,16,
145,64,23,251,31,63,99,102,248,147,27,143,247,85,137,168,190,110,22,173,53,195,41,149,181,141,116,140,172,125,93,51,23,55,224,66,19,41,75,205,105,175,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* BackgroundComponent::dots_png = (const char*) resource_BackgroundComponent_dots_png;
const int BackgroundComponent::dots_pngSize = 225;


//[EndFile] You can add extra defines here...
//[/EndFile]
