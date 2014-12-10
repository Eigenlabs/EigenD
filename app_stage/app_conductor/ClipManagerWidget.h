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

#ifndef __JUCE_HEADER_B20B08716215BECA__
#define __JUCE_HEADER_B20B08716215BECA__

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
#include "ClipPanelSwitcher.h"
#include "ClipManagerPanel.h"
#include "ClipEditorPanel.h"
#include "Conductor.h"
#include "Clip.h"

class ClipManagerPanel;
class ClipEditorPanel;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ClipManagerWidget  : public Component,
                           public ClipPanelSwitcher
{
public:
    //==============================================================================
    ClipManagerWidget (ClipManagerBackend* backend);
    ~ClipManagerWidget();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void showEditorPanel(Clip*);
    void showSelectorPanel();
    virtual void switchPanel(String, Clip*);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    ClipManagerBackend* backend_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<ClipEditorPanel> clipEditorPanel;
    ScopedPointer<ClipManagerPanel> clipManagerPanel;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipManagerWidget)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_B20B08716215BECA__
