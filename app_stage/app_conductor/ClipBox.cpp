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

#include "ClipBox.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

//[/MiscUserDefs]

//==============================================================================
ClipBox::ClipBox (Clip* clip)
    : clip_(clip)
{
    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("Clip 1")));
    label->setFont (Font (12.00f, Font::bold));
    label->setJustificationType (Justification::centred);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colour (0xff484848));
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    cachedImage_audio_clip_png = ImageCache::getFromMemory (audio_clip_png, audio_clip_pngSize);

    //[UserPreSize]
    if(clip_->getCategory().equalsIgnoreCase("instrument"))
    {
        cachedImage_audio_clip_png = ImageCache::getFromMemory (instrument_clip_png, instrument_clip_pngSize);
    }
    else if(clip_->getCategory().equalsIgnoreCase("talker"))
    {

        cachedImage_audio_clip_png = ImageCache::getFromMemory (talker_clip_png, talker_clip_pngSize);
    }
    else if(clip_->getCategory().equalsIgnoreCase("scene"))
    {

        cachedImage_audio_clip_png = ImageCache::getFromMemory (scene_png, scene_pngSize);
    }

    else if(clip_->getCategory().equalsIgnoreCase("arrangement"))
    {

        cachedImage_audio_clip_png = ImageCache::getFromMemory (arrangement_png, arrangement_pngSize);
    }


    label->setInterceptsMouseClicks(false,false);
    label->setText(clip_->getName(),dontSendNotification);
    selected_=false;
    //[/UserPreSize]

    setSize (120, 20);


    //[Constructor] You can add your own custom stuff here..

    //[/Constructor]
}

ClipBox::~ClipBox()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ClipBox::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    if(!selected_)
    {
    //[/UserPrePaint]

    g.setColour (Colour (0xffe4e4e4));
    g.fillRoundedRectangle (1.0f, 1.0f, 118.0f, 18.0f, 10.000f);

    g.setColour (Colour (0xff979797));
    g.drawRoundedRectangle (1.0f, 1.0f, 118.0f, 18.0f, 10.000f, 1.000f);

    g.setColour (Colours::black);
    g.drawImageWithin (cachedImage_audio_clip_png,
                       6, 0, 18, 20,
                       RectanglePlacement::centred,
                       false);

    //[UserPaint] Add your own custom painting code here..
    // XXX

    }
    else
    {
//    g.fillAll (Colours::white);

    g.setColour (Colours::lightgreen);
    g.fillRoundedRectangle (1.0f, 1.0f, 118.0f, 18.0f, 10.0000f);

    g.setColour (Colour (0xff979797));
    g.drawRoundedRectangle (1.0f, 1.0f, 118.0f, 18.0f, 10.0000f, 1.0000f);
    g.setColour (Colours::black);
    g.drawImageWithin (cachedImage_audio_clip_png,
                       6, 0, 18, 20,
                       RectanglePlacement::centred,
                       false);


    }

    //[/UserPaint]
}

void ClipBox::resized()
{
    label->setBounds (24, 4, 84, 12);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ClipBox::mouseEnter (const MouseEvent& e)
{
    //[UserCode_mouseEnter] -- Add your code here...
    //[/UserCode_mouseEnter]
}

void ClipBox::mouseExit (const MouseEvent& e)
{
    //[UserCode_mouseExit] -- Add your code here...
    //[/UserCode_mouseExit]
}

void ClipBox::mouseDown (const MouseEvent& e)
{
    //[UserCode_mouseDown] -- Add your code here...
    //[/UserCode_mouseDown]
}

void ClipBox::mouseDrag (const MouseEvent& e)
{
    //[UserCode_mouseDrag] -- Add your code here...
    //[/UserCode_mouseDrag]
}

void ClipBox::mouseUp (const MouseEvent& e)
{
    //[UserCode_mouseUp] -- Add your code here...
    if(e.mouseWasClicked())
    {
        selected_=!selected_;
        repaint();
    }
//    dragging_=false;
    //[/UserCode_mouseUp]
}

void ClipBox::mouseDoubleClick (const MouseEvent& e)
{
    //[UserCode_mouseDoubleClick] -- Add your code here...
    //[/UserCode_mouseDoubleClick]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
bool ClipBox::isSelected()
{
    return selected_;
}

void ClipBox::toggleSelected()
{
    selected_=!selected_;
    repaint();
}

void ClipBox::setSelected(bool shouldBeSelected)
{
    selected_=shouldBeSelected;
    repaint();
}

Clip* ClipBox::getClip()
{
    return clip_;
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ClipBox" componentName=""
                 parentClasses="public Component" constructorParams="Clip* clip"
                 variableInitialisers="clip_(clip)" snapPixels="2" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="120"
                 initialHeight="20">
  <METHODS>
    <METHOD name="mouseUp (const MouseEvent&amp; e)"/>
    <METHOD name="mouseEnter (const MouseEvent&amp; e)"/>
    <METHOD name="mouseExit (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDown (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDrag (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDoubleClick (const MouseEvent&amp; e)"/>
  </METHODS>
  <BACKGROUND backgroundColour="fcfcfc">
    <ROUNDRECT pos="1 1 118 18" cornerSize="10" fill="solid: ffe4e4e4" hasStroke="1"
               stroke="1, mitered, butt" strokeColour="solid: ff979797"/>
    <IMAGE pos="6 0 18 20" resource="audio_clip_png" opacity="1" mode="1"/>
  </BACKGROUND>
  <LABEL name="new label" id="cff45a608be57b60" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="24 4 84 12" textCol="ff484848" edTextCol="ff000000"
         edBkgCol="0" labelText="Clip 1" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="12"
         bold="1" italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: audio_clip_png, 445, "../../../Desktop/audio_clip.png"
static const unsigned char resource_ClipBox_audio_clip_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,18,0,0,0,14,8,6,0,0,0,34,218,76,183,0,0,0,1,115,82,71,66,0,174,206,28,233,0,0,0,6,98,
75,71,68,0,255,0,255,0,255,160,189,167,147,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,7,116,73,77,69,7,220,10,25,13,49,14,231,180,207,166,0,0,1,61,73,68,65,84,40,207,141,211,191,106,
212,65,20,197,241,207,217,44,33,107,4,17,69,162,137,8,70,81,16,3,86,33,193,222,151,16,4,133,212,90,137,133,8,62,129,165,111,96,99,99,99,97,35,8,162,133,177,176,20,34,134,64,202,64,212,136,73,132,73,115,
17,18,246,183,238,192,101,152,153,115,191,115,238,252,73,107,205,255,90,146,21,108,181,214,94,118,105,122,99,64,22,241,4,179,53,158,74,50,151,36,99,131,146,156,198,35,204,96,163,166,7,184,141,115,99,129,
146,204,227,25,110,224,19,126,38,153,196,121,108,226,86,146,222,80,80,146,126,146,123,73,94,224,13,174,225,46,62,226,20,110,226,126,229,45,23,116,168,163,30,46,96,15,91,120,128,247,216,199,73,220,193,
137,2,236,226,242,168,210,126,227,3,222,98,13,19,21,223,112,12,211,165,219,169,243,234,116,212,199,143,74,78,69,31,83,117,224,3,252,41,135,187,93,160,96,30,215,113,22,147,165,153,174,254,57,86,177,142,
75,229,120,40,104,31,95,112,17,75,120,140,43,229,110,187,181,246,21,79,113,6,175,241,253,95,102,107,237,80,148,171,9,92,197,171,218,245,51,150,107,189,87,143,115,112,40,239,40,232,8,244,56,30,226,29,22,
70,105,51,230,95,155,173,210,126,117,106,234,54,70,181,134,191,85,110,231,79,56,0,119,137,128,63,117,17,176,92,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* ClipBox::audio_clip_png = (const char*) resource_ClipBox_audio_clip_png;
const int ClipBox::audio_clip_pngSize = 445;

// JUCER_RESOURCE: instrument_clip_png, 489, "../../../../conductor_images/png/instrument_clip.png"
static const unsigned char resource_ClipBox_instrument_clip_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,17,0,0,0,15,8,6,0,0,0,2,177,36,17,0,0,0,1,115,82,71,66,0,174,206,28,233,0,0,0,
6,98,75,71,68,0,255,0,255,0,255,160,189,167,147,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,7,116,73,77,69,7,220,11,14,10,53,54,94,59,226,47,0,0,1,105,73,68,65,84,56,203,141,211,177,
75,213,97,20,198,241,207,73,187,36,149,14,77,73,97,184,8,110,45,10,226,16,181,212,63,224,16,13,13,65,99,100,174,130,91,83,16,66,180,137,75,136,56,136,91,163,152,88,67,96,91,147,210,16,137,17,81,40,116,
21,229,184,188,23,126,253,238,213,58,112,224,229,229,251,30,158,247,156,231,200,76,245,196,115,124,194,44,238,160,175,19,215,202,40,143,254,138,136,152,197,59,220,192,56,118,177,136,213,204,60,110,123,
112,138,146,121,92,47,231,43,184,139,37,172,224,102,157,63,167,115,236,99,44,34,230,113,15,239,241,24,31,241,36,34,26,85,248,180,34,77,12,98,7,19,120,141,97,124,198,126,102,30,86,225,238,51,148,28,224,
101,102,238,70,196,56,30,160,23,95,235,112,119,165,153,93,101,18,151,209,131,63,136,210,183,245,136,184,132,103,248,210,177,72,68,92,196,20,6,176,129,171,69,73,35,34,250,112,190,168,203,194,71,86,198,
218,82,50,128,91,120,132,109,44,96,18,183,203,253,30,126,149,94,53,208,133,163,122,145,239,5,186,150,153,91,229,107,163,24,193,139,204,92,45,10,238,99,36,51,143,218,166,147,153,63,48,131,167,17,177,140,
183,216,196,207,86,129,202,212,14,218,204,89,117,108,68,12,99,12,223,240,1,211,248,141,57,92,192,67,108,102,230,210,63,29,91,113,110,63,222,20,219,239,224,21,122,254,107,119,106,123,212,91,154,219,196,
90,102,54,219,24,12,57,59,142,113,88,216,70,39,151,159,0,162,211,213,36,227,170,6,205,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* ClipBox::instrument_clip_png = (const char*) resource_ClipBox_instrument_clip_png;
const int ClipBox::instrument_clip_pngSize = 489;

// JUCER_RESOURCE: scene_png, 437, "../../../../conductor_images/png/scene.png"
static const unsigned char resource_ClipBox_scene_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,18,0,0,0,15,8,6,0,0,0,233,134,159,18,0,0,0,1,115,82,71,66,0,174,206,28,233,0,0,0,6,98,75,
71,68,0,255,0,255,0,255,160,189,167,147,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,7,116,73,77,69,7,220,11,14,11,29,28,217,31,239,100,0,0,1,53,73,68,65,84,56,203,181,211,61,78,219,
65,16,5,240,223,6,99,62,34,130,1,81,144,38,77,146,158,3,36,5,18,18,101,110,193,33,56,10,226,12,20,112,0,36,90,154,72,41,34,226,144,38,5,178,73,20,25,140,28,28,24,10,6,244,15,198,78,97,229,73,171,29,237,
236,188,125,243,177,5,203,238,48,135,21,124,195,77,158,61,71,241,55,38,42,254,7,95,137,136,59,163,148,119,216,193,47,124,193,121,222,121,129,70,6,159,225,43,246,240,41,34,254,60,69,180,128,53,204,96,22,
211,143,214,253,235,215,104,97,63,34,78,6,136,198,69,237,129,177,148,130,55,216,192,42,230,43,10,218,248,142,38,142,241,25,191,163,162,162,154,218,107,108,38,217,82,18,92,161,143,46,2,117,76,225,16,219,
17,113,246,255,82,171,164,216,192,219,236,84,45,139,92,82,85,23,63,35,162,57,16,87,85,84,74,89,199,22,94,101,42,173,108,121,31,207,48,153,251,54,118,35,226,102,24,209,108,14,232,12,58,232,101,173,34,7,
177,150,68,221,136,184,24,170,232,41,148,82,234,232,199,63,46,62,86,212,192,7,188,199,98,126,145,233,84,118,154,173,255,136,131,136,232,140,42,118,15,71,248,129,151,88,72,187,141,11,92,102,221,122,35,
21,141,131,146,67,54,54,110,1,23,229,122,167,188,239,114,238,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* ClipBox::scene_png = (const char*) resource_ClipBox_scene_png;
const int ClipBox::scene_pngSize = 437;

// JUCER_RESOURCE: talker_clip_png, 465, "../../../../conductor_images/png/talker_clip.png"
static const unsigned char resource_ClipBox_talker_clip_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,18,0,0,0,13,8,6,0,0,0,164,78,62,25,0,0,0,1,115,82,71,66,0,174,206,28,233,0,0,0,6,98,
75,71,68,0,255,0,255,0,255,160,189,167,147,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,7,116,73,77,69,7,220,11,14,11,28,39,113,15,55,1,0,0,1,81,73,68,65,84,40,207,141,211,59,107,149,
65,16,198,241,223,115,8,136,18,177,48,32,98,154,16,27,19,27,65,172,44,180,176,81,76,147,54,159,64,196,70,91,27,253,8,42,98,23,75,177,179,10,228,3,168,149,151,66,176,18,44,20,2,9,26,188,43,103,108,54,230,
61,199,215,131,3,203,222,102,254,187,207,236,108,176,199,174,13,91,131,116,250,129,126,11,166,32,85,53,186,147,28,106,129,95,240,173,170,190,251,15,251,3,74,178,136,203,248,137,247,216,135,35,152,193,
173,170,90,159,4,154,234,140,207,99,27,247,241,185,129,14,96,14,231,146,60,195,102,141,75,232,1,29,196,122,85,189,234,200,28,52,216,41,220,193,199,36,175,241,8,111,170,234,87,31,104,136,31,99,7,45,225,
12,174,224,19,246,227,40,174,227,46,30,247,129,222,226,98,146,175,120,142,121,156,192,106,85,189,236,248,189,72,242,1,11,73,158,86,213,208,216,179,62,196,70,59,253,18,238,225,44,182,122,82,242,174,197,
238,148,200,46,168,170,54,241,160,77,231,112,13,107,184,144,100,54,201,222,36,211,73,102,112,178,73,29,254,37,45,73,112,172,229,225,106,85,109,36,217,194,205,118,195,85,204,182,2,158,199,141,145,23,172,
42,109,30,28,199,226,206,90,103,239,54,158,96,185,37,251,48,6,35,62,227,65,61,144,133,38,121,101,146,95,254,81,95,221,47,115,26,211,85,181,54,201,239,55,81,220,171,43,76,57,213,99,0,0,0,0,73,69,78,68,
174,66,96,130,0,0};

const char* ClipBox::talker_clip_png = (const char*) resource_ClipBox_talker_clip_png;
const int ClipBox::talker_clip_pngSize = 465;

// JUCER_RESOURCE: arrangement_png, 683, "../../../../conductor_images/png/arrangement.png"
static const unsigned char resource_ClipBox_arrangement_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,6,0,0,0,59,48,174,162,0,0,0,1,115,82,71,66,0,174,206,28,233,0,0,0,6,
98,75,71,68,0,255,0,255,0,255,160,189,167,147,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,7,116,73,77,69,7,220,11,28,15,17,42,71,9,2,57,0,0,2,43,73,68,65,84,72,199,237,151,205,78,84,
65,16,133,191,51,227,12,204,15,56,66,64,50,128,138,36,40,198,196,196,189,47,224,198,149,113,163,59,227,218,165,15,225,131,248,30,174,77,92,176,112,173,134,132,81,64,17,101,126,24,218,133,53,147,155,178,
155,185,215,4,217,80,73,47,230,76,85,159,58,93,213,213,185,10,33,112,30,86,226,156,236,130,248,47,147,84,145,52,117,102,196,146,218,146,234,17,159,171,192,203,68,82,155,146,30,59,172,36,233,149,164,213,
188,138,215,129,167,146,22,156,79,21,120,150,136,191,11,188,144,212,200,96,139,192,115,224,161,164,106,30,226,111,192,19,160,29,241,91,145,180,20,193,23,128,85,224,122,6,91,6,106,192,3,160,158,135,248,
208,54,184,45,41,139,11,40,3,183,18,241,51,70,54,178,99,160,15,108,0,83,121,136,247,128,46,112,223,50,206,226,2,214,34,241,39,64,211,148,143,236,151,17,223,176,50,77,36,238,217,218,4,26,238,36,2,176,146,
216,163,12,84,50,191,59,38,160,121,218,173,41,69,20,92,115,71,20,128,29,167,106,100,67,171,227,92,6,59,0,142,128,105,224,82,17,226,182,59,162,96,42,150,35,241,29,43,195,88,113,248,51,252,119,13,215,68,
226,16,66,23,248,105,217,251,76,119,172,137,188,165,94,152,195,76,99,230,86,92,142,40,254,10,204,71,226,123,70,226,9,182,255,117,100,198,54,154,141,248,29,91,35,157,201,35,17,128,47,64,171,192,190,149,
162,196,169,154,236,219,28,86,162,179,7,14,91,42,74,220,75,248,117,172,246,245,72,124,207,134,76,158,166,75,18,239,38,148,15,12,171,69,30,144,217,200,62,205,162,196,125,107,152,19,135,79,155,10,143,55,
236,234,249,6,107,77,82,238,137,247,172,145,250,14,191,108,138,27,14,191,98,179,249,192,225,71,69,21,111,3,91,145,90,207,153,239,124,228,58,13,128,31,14,255,104,216,48,47,241,39,224,125,36,227,97,226,
232,246,141,192,31,245,59,243,45,167,136,253,104,252,0,124,183,209,57,158,189,146,182,172,206,190,233,62,3,111,51,35,114,100,111,128,71,167,41,38,132,48,94,214,165,53,160,228,240,25,224,53,112,207,225,
77,224,14,176,152,197,237,191,155,64,213,227,163,165,188,95,18,146,90,64,215,30,147,44,46,75,116,88,100,100,234,226,19,230,127,217,111,206,157,185,244,65,0,244,238,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* ClipBox::arrangement_png = (const char*) resource_ClipBox_arrangement_png;
const int ClipBox::arrangement_pngSize = 683;


//[EndFile] You can add extra defines here...
//[/EndFile]
