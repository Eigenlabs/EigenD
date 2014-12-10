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

#ifndef __JUCE_HEADER_AF3BB62498F3E58E__
#define __JUCE_HEADER_AF3BB62498F3E58E__

//[Headers]     -- You can add your own extra header files here --
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

#include "juce.h"
//#include "ClipPanelSwitcher.h"
#include "Clip.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ClipBox  : public Component
{
public:
    //==============================================================================
    ClipBox (Clip* clip);
    ~ClipBox();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    bool isSelected();
    Clip* getClip();
    void toggleSelected();
    void setSelected(bool);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void mouseEnter (const MouseEvent& e);
    void mouseExit (const MouseEvent& e);
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    void mouseDoubleClick (const MouseEvent& e);

    // Binary resources:
    static const char* audio_clip_png;
    static const int audio_clip_pngSize;
    static const char* instrument_clip_png;
    static const int instrument_clip_pngSize;
    static const char* scene_png;
    static const int scene_pngSize;
    static const char* talker_clip_png;
    static const int talker_clip_pngSize;
    static const char* arrangement_png;
    static const int arrangement_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    bool selected_;
    Clip* clip_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> label;
    Image cachedImage_audio_clip_png;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipBox)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_AF3BB62498F3E58E__
