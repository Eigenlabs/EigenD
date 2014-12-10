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

#ifndef __JUCE_HEADER_90065FA8057BEA42__
#define __JUCE_HEADER_90065FA8057BEA42__

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

#include "EigenDialogWindow.h"
//[/Headers]

#include "DialogButtonComponent.h"


//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class DialogFrameworkComponent  : public Component,
                                  public ComponentBoundsConstrainer,
                                  public Button::Listener
{
public:
    //==============================================================================
    DialogFrameworkComponent (EigenDialogWindow* window);
    ~DialogFrameworkComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void setTitleText(const String text);
    void setHideTitle(const bool hide);
    void setContentComponent(Component* content);
    void buttonClicked (Button* buttonThatWasClicked);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    //bool keyPressed(const KeyPress& key);
    DialogButtonComponent* getOKButton() { return OKButton; }
    DialogButtonComponent* getCancelButton() { return cancelButton; }
    void showResizer() { if (resizer_) resizer_->setVisible(true); }
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    Component* content_;
    ResizableCornerComponent* resizer_;
    bool dragging_;
    int startX_;
    int startY_;
    EigenDialogWindow* window_;
	bool hideTitle_;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> titleText;
    ScopedPointer<DialogButtonComponent> OKButton;
    ScopedPointer<DialogButtonComponent> cancelButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DialogFrameworkComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_90065FA8057BEA42__
