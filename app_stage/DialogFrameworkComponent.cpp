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

#include "DialogFrameworkComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
DialogFrameworkComponent::DialogFrameworkComponent (EigenDialogWindow* window)
    : window_(window)
{
    setName ("DialogFramework");
    addAndMakeVisible (titleText = new Label ("title text label",
                                              TRANS("Title text")));
    titleText->setFont (Font (17.20f, Font::bold));
    titleText->setJustificationType (Justification::centredLeft);
    titleText->setEditable (false, false, false);
    titleText->setColour (Label::textColourId, Colour (0xffc8c8c8));
    titleText->setColour (TextEditor::textColourId, Colours::black);
    titleText->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (OKButton = new DialogButtonComponent());
    addAndMakeVisible (cancelButton = new DialogButtonComponent());

    //[UserPreSize]
#if STAGE_BUILD==DESKTOP
    addChildComponent(resizer_ = new ResizableCornerComponent(this, this));
    content_ = 0;
    dragging_ = false;
#endif
	hideTitle_ = false;
    //[/UserPreSize]

    setSize (320, 176);


    //[Constructor] You can add your own custom stuff here..
    OKButton->setText ("OK");
    OKButton->addListener (this);

    cancelButton->setText ("Cancel");
    cancelButton->addListener (this);

    // keyboard shortcuts for buttons
#if STAGE_BUILD==DESKTOP
    const KeyPress returnKey(KeyPress::returnKey, 0, 0);
    const KeyPress escapeKey(KeyPress::escapeKey, 0, 0);
    OKButton->addShortcut(returnKey);
    cancelButton->addShortcut(escapeKey);

    titleText->addMouseListener(this, false);
#endif
    //[/Constructor]
}

DialogFrameworkComponent::~DialogFrameworkComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    titleText = nullptr;
    OKButton = nullptr;
    cancelButton = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
#if STAGE_BUILD==DESKTOP
    deleteAndZero(resizer_);
#endif
    //[/Destructor]
}

//==============================================================================
void DialogFrameworkComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.setColour (Colour (0xcf000000));
    g.fillRoundedRectangle (3.0f, 3.0f, static_cast<float> (getWidth() - 6), static_cast<float> (getHeight() - 6), 5.000f);

    g.setColour (Colour (0xffa3a3a3));
    g.drawRoundedRectangle (3.0f, 3.0f, static_cast<float> (getWidth() - 6), static_cast<float> (getHeight() - 6), 5.000f, 2.700f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void DialogFrameworkComponent::resized()
{
    titleText->setBounds (12, 12, getWidth() - 20, 24);
    OKButton->setBounds (getWidth() - 112, getHeight() - 40, 98, 24);
    cancelButton->setBounds (getWidth() - 216, getHeight() - 40, 98, 24);
    //[UserResized] Add your own custom resize handling here..
	if (hideTitle_)
	{
		titleText->setVisible(false);
		if (content_) content_->setBounds (16, 12, getWidth() - 32, getHeight() - 60);
	}
	else
	{
		if (content_) content_->setBounds (16, 48, getWidth() - 32, getHeight() - 96);
	}
#if STAGE_BUILD==DESKTOP
    resizer_->setBounds(getWidth()-20,getHeight()-20,16,16);
#endif
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void DialogFrameworkComponent::setTitleText(const String text)
{
    titleText->setText(text, dontSendNotification);
}

void DialogFrameworkComponent::setHideTitle(const bool hide)
{
	hideTitle_ = hide;
	resized();
}

void DialogFrameworkComponent::setContentComponent(Component* content)
{
    if (content_) {
        removeChildComponent(content_);
    }
    content_ = content;
    addAndMakeVisible(content_);

    resized();

#if STAGE_BUILD==DESKTOP
    content_->addMouseListener(this, false);
#endif
}

void DialogFrameworkComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == OKButton)
    {
        exitModalState(1);
    }
    else if (buttonThatWasClicked == cancelButton)
    {
        exitModalState(0);
    }

}

/*
bool DialogFrameworkComponent::keyPressed(const KeyPress& key)
{
    // handle esc out of full screen mode
    if(key.getKeyCode()==KeyPress::escapeKey)
    {
        exitModalState(0);
    }

    return false;
}
*/

void DialogFrameworkComponent::mouseDrag (const MouseEvent& e)
{
#if STAGE_BUILD==DESKTOP
    int dx=0, dy=0;
//    if(e.originalComponent==this || e.originalComponent==titleText || e.originalComponent==content_)
    if(e.originalComponent==titleText)
    {
        if (e.mods.isLeftButtonDown())
        {
            if (!dragging_)
            {
                dragging_ = ! e.mouseWasClicked();
                // start dragging_
                if (dragging_)
                {
                    startX_ = getScreenX();
                    startY_ = getScreenY();
                }
            }
            // continue dragging_
            if (dragging_)
            {
                dx = e.getDistanceFromDragStartX();
                dy = e.getDistanceFromDragStartY();
                window_->setTopLeftPosition(startX_ + dx,
                                   startY_ + dy);
            }
        }
    }
#endif
}
void DialogFrameworkComponent::mouseUp (const MouseEvent& e)
{
    dragging_ = false;
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="DialogFrameworkComponent"
                 componentName="DialogFramework" parentClasses="public Component, public ComponentBoundsConstrainer, public Button::Listener"
                 constructorParams="EigenDialogWindow* window" variableInitialisers="window_(window)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="320" initialHeight="176">
  <BACKGROUND backgroundColour="0">
    <ROUNDRECT pos="3 3 6M 6M" cornerSize="5" fill="solid: cf000000" hasStroke="1"
               stroke="2.7, mitered, butt" strokeColour="solid: ffa3a3a3"/>
  </BACKGROUND>
  <LABEL name="title text label" id="d02617478241afef" memberName="titleText"
         virtualName="" explicitFocusOrder="0" pos="12 12 20M 24" textCol="ffc8c8c8"
         edTextCol="ff000000" edBkgCol="0" labelText="Title text" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="17.199999999999999289" bold="1" italic="0" justification="33"/>
  <JUCERCOMP name="OK Button" id="7e276cf364a05bda" memberName="OKButton"
             virtualName="" explicitFocusOrder="0" pos="112R 40R 98 24" sourceFile="DialogButtonComponent.cpp"
             constructorParams=""/>
  <JUCERCOMP name="cancel button" id="e46b373db2b4fbbf" memberName="cancelButton"
             virtualName="" explicitFocusOrder="0" pos="216R 40R 98 24" sourceFile="DialogButtonComponent.cpp"
             constructorParams=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
