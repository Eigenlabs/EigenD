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

#include "DialogButtonComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
DialogButtonComponent::DialogButtonComponent ()
    : Button("DialogButton")
{
    setName ("DialogButton");
    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("label text")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centred);
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    label->setInterceptsMouseClicks (false, false);
    //[/UserPreSize]

    setSize (128, 32);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

DialogButtonComponent::~DialogButtonComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void DialogButtonComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    Button::paint(g);
    return;
    //[/UserPrePaint]

    g.setGradientFill (ColourGradient (Colour (0xfffdfdfd),
                                       static_cast<float> ((getWidth() / 2)), 0.0f,
                                       Colour (0xffcecece),
                                       static_cast<float> ((getWidth() / 2)), static_cast<float> (getHeight()),
                                       false));
    g.fillRoundedRectangle (1.0f, 1.0f, static_cast<float> (getWidth() - 2), static_cast<float> (getHeight() - 2), 16.000f);

    g.setColour (Colour (0xff999999));
    g.drawRoundedRectangle (1.0f, 1.0f, static_cast<float> (getWidth() - 2), static_cast<float> (getHeight() - 2), 16.000f, 2.000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void DialogButtonComponent::resized()
{
    label->setBounds (0, 0, getWidth() - 0, getHeight() - 0);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void DialogButtonComponent::setText(const String& text)
{
    label->setText(text, dontSendNotification);
}

void DialogButtonComponent::paintButton (Graphics &g, bool isMouseOverButton, bool isButtonDown)
{
    if(isButtonDown)
        g.setGradientFill (ColourGradient (Colour (0xff717171),
                                           (float) ((getWidth() / 2)), 0.0f,
                                           Colour (0xfff3f3f3),
                                           (float) ((getWidth() / 2)), (float) (getHeight()),
                                           false));
    else
        g.setGradientFill (ColourGradient (Colour (0xfffdfdfd),
                                           (float) ((getWidth() / 2)), 0.0f,
                                           Colour (0xffcecece),
                                           (float) ((getWidth() / 2)), (float) (getHeight()),
                                           false));


    g.fillRoundedRectangle (1.0f, 1.0f, (float) (getWidth() - 2), (float) (getHeight() - 2), (getHeight()/2));

    g.setColour (Colour (0xff999999));
    g.drawRoundedRectangle (1.0f, 1.0f, (float) (getWidth() - 2), (float) (getHeight() - 2), (getHeight()/2), 2.0000f);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="DialogButtonComponent" componentName="DialogButton"
                 parentClasses="public Button" constructorParams="" variableInitialisers="Button(&quot;DialogButton&quot;)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="128" initialHeight="32">
  <BACKGROUND backgroundColour="ffffff">
    <ROUNDRECT pos="1 1 2M 2M" cornerSize="16" fill="linear: 0C 0, 0C 0R, 0=fffdfdfd, 1=ffcecece"
               hasStroke="1" stroke="2, mitered, butt" strokeColour="solid: ff999999"/>
  </BACKGROUND>
  <LABEL name="new label" id="5efac89d904a1de3" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0 0 0M 0M" edTextCol="ff000000" edBkgCol="0"
         labelText="label text" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
