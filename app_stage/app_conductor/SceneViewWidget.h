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

#ifndef __JUCE_HEADER_FF0B6B5A1F788E__
#define __JUCE_HEADER_FF0B6B5A1F788E__

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
#include "Conductor.h"
#include "SceneList.h"
#include "SceneToolbarItemFactory.h"
#include "ClipPanel.h"
#include "ClipSelector.h"

class SceneList;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class SceneViewWidget  : public Component,
                         public ClipSelector,
                         public ButtonListener
{
public:
    //==============================================================================
    SceneViewWidget (SceneBackend* backend, int mode);
    ~SceneViewWidget();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    static const int PLAYING=0;
    static const int LAST=1;
    void setSceneName(String name);
    void clipSelected(Clip*);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    SceneBackend* backend_;
    Toolbar* toolbar_;
    SceneToolbarItemFactory* tm_;
    int mode_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> sceneLabel;
    ScopedPointer<TextButton> duplicateButton;
    ScopedPointer<TextButton> removeButton;
    ScopedPointer<SceneList> sceneList;
    ScopedPointer<ClipPanel> clipPanel;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneViewWidget)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_FF0B6B5A1F788E__
