/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  23 Nov 2010 3:30:31pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "JucerPopupDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
PopupDialogComponent::PopupDialogComponent ()
    : Component (T("PopupDialog"))
{

    //[UserPreSize]
    content_ = 0;
    x_offset_ = 0;
    y_offset_ = 0;
    //[/UserPreSize]

    setSize (240, 176);

    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

PopupDialogComponent::~PopupDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void PopupDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    g.saveState();

    g.setColour (Colour (0xcf000000));
    g.fillRoundedRectangle (3.0f, 3.0f, (float) (getWidth() - 6), (float) (getHeight() - 6), 5.0000f);

    g.setColour (Colour (0xffa3a3a3));
    g.drawRoundedRectangle (3.0f, 3.0f, (float) (getWidth() - 6), (float) (getHeight() - 6), 5.0000f, 2.7000f);

    g.restoreState();
    //[/UserPaint]
}

void PopupDialogComponent::resized()
{
    //[UserResized] Add your own custom resize handling here..
    if(content_)
        content_->setBounds (16 + x_offset_, 16 + y_offset_, getWidth() - 32 - x_offset_, getHeight() - 32 - y_offset_);
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void PopupDialogComponent::setContentComponent(Component* content)
{
    setContentComponent(content, 0, 0);
}

void PopupDialogComponent::setContentComponent(Component* content, int x_offset, int y_offset)
{
    if (content_) {
        removeChildComponent(content_);
    }
    content_ = content;
    x_offset_ = x_offset;
    y_offset_ = y_offset;
    addAndMakeVisible(content_);

    resized();
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="PopupDialogComponent" componentName="PopupDialog"
                 parentClasses="public Component"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330000013" fixedSize="1" initialWidth="320"
                 initialHeight="176">
  <BACKGROUND backgroundColour="0">
    <ROUNDRECT pos="3 3 6M 6M" cornerSize="5" fill="solid: cf000000" hasStroke="1"
               stroke="2.70000005, mitered, butt" strokeColour="solid: ffa3a3a3"/>
  </BACKGROUND>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
