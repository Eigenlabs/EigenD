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

/* HelpViewComponent: Component to hold the tree view of the agents within the
                       setup running in EigenD

*/


//[/Headers]
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

#include "HelpViewComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
HelpViewComponent::HelpViewComponent ()
{
    setName ("HelpView");
    addAndMakeVisible (titleLabel = new Label ("title label",
                                               TRANS("Help")));
    titleLabel->setFont (Font (17.20f, Font::bold));
    titleLabel->setJustificationType (Justification::centredLeft);
    titleLabel->setEditable (false, false, false);
    titleLabel->setColour (Label::textColourId, Colour (0xffc8c8c8));
    titleLabel->setColour (TextEditor::textColourId, Colours::black);
    titleLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (descriptionLabel = new Label ("description label",
                                                     String::empty));
    descriptionLabel->setFont (Font (15.00f, Font::plain));
    descriptionLabel->setJustificationType (Justification::topLeft);
    descriptionLabel->setEditable (false, false, false);
    descriptionLabel->setColour (TextEditor::textColourId, Colours::black);
    descriptionLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    resizer_ = new ResizableCornerComponent(this, this);
    addAndMakeVisible(resizer_);
    //[/UserPreSize]

    setSize (304, 744);


    //[Constructor] You can add your own custom stuff here..

    titleLabel->addMouseListener(this, true);

    setMinimumWidth(256);
    setMinimumHeight(128);

    descriptionLabel->setMinimumHorizontalScale(1);

    //[/Constructor]
}

HelpViewComponent::~HelpViewComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    titleLabel = nullptr;
    descriptionLabel = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    deleteAndZero(resizer_);
    //[/Destructor]
}

//==============================================================================
void HelpViewComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.setColour (Colour (0xcf000000));
    g.fillRoundedRectangle (3.0f, 3.0f, static_cast<float> (getWidth() - 6), static_cast<float> (getHeight() - 6), 5.000f);

    g.setColour (Colour (0xffa3a3a3));
    g.drawRoundedRectangle (3.0f, 3.0f, static_cast<float> (getWidth() - 6), static_cast<float> (getHeight() - 6), 5.000f, 2.700f);

    g.setColour (Colour (0xffe9e9e9));
    g.fillRoundedRectangle (26.0f, 63.0f, static_cast<float> (getWidth() - 50), static_cast<float> (getHeight() - 87), 3.000f);

    g.setColour (Colour (0xffa3a3a3));
    g.drawRoundedRectangle (26.0f, 63.0f, static_cast<float> (getWidth() - 50), static_cast<float> (getHeight() - 87), 3.000f, 2.700f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void HelpViewComponent::resized()
{
    titleLabel->setBounds (12, 12, 276, 24);
    descriptionLabel->setBounds (32, 72, getWidth() - 64, getHeight() - 104);
    //[UserResized] Add your own custom resize handling here..
    resizer_->setBounds(getWidth()-20,getHeight()-20,16,16);
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void HelpViewComponent::mouseDrag (const MouseEvent& e)
{
    int dx=0, dy=0;

    if (e.mods.isLeftButtonDown())
    {

        if (!dragging_)
        {
            dragging_ = ! e.mouseWasClicked();

            // start dragging_
            if (dragging_)
            {
                startX_ = getX();
                startY_ = getY();

            }

        }

        // continue dragging_
        if (dragging_)
        {
            dx = e.getDistanceFromDragStartX();
            dy = e.getDistanceFromDragStartY();

            setTopLeftPosition(startX_ + dx,
                               startY_ + dy);
        }

    }

}


void HelpViewComponent::mouseUp (const MouseEvent& e)
{
    dragging_ = false;

}


bool HelpViewComponent::keyPressed(const KeyPress& key)
{
	return false;
}

void HelpViewComponent::broughtToFront()
{
}


void HelpViewComponent::setHelpDescription(const String& description)
{
    descriptionLabel->setText(description, dontSendNotification);
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="HelpViewComponent" componentName="HelpView"
                 parentClasses="public Component, public ComponentBoundsConstrainer"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="304"
                 initialHeight="744">
  <BACKGROUND backgroundColour="0">
    <ROUNDRECT pos="3 3 6M 6M" cornerSize="5" fill="solid: cf000000" hasStroke="1"
               stroke="2.7, mitered, butt" strokeColour="solid: ffa3a3a3"/>
    <ROUNDRECT pos="26 63 50M 87M" cornerSize="3" fill="solid: ffe9e9e9" hasStroke="1"
               stroke="2.7, mitered, butt" strokeColour="solid: ffa3a3a3"/>
  </BACKGROUND>
  <LABEL name="title label" id="d02617478241afef" memberName="titleLabel"
         virtualName="" explicitFocusOrder="0" pos="12 12 276 24" textCol="ffc8c8c8"
         edTextCol="ff000000" edBkgCol="0" labelText="Help" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="17.199999999999999289" bold="1" italic="0" justification="33"/>
  <LABEL name="description label" id="2eb3f309a31a42f2" memberName="descriptionLabel"
         virtualName="" explicitFocusOrder="0" pos="32 72 64M 104M" edTextCol="ff000000"
         edBkgCol="0" labelText="" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="9"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
