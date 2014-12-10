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
//[/Headers]

#include "ClipPoolWidget.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ClipPoolWidget::ClipPoolWidget (ClipPoolBackend * backend)
    : backend_(backend)
{
    addAndMakeVisible (addToArrangementButton = new TextButton ("addToArrangementButton"));
    addToArrangementButton->setButtonText (TRANS("Add to arrangement"));
    addToArrangementButton->addListener (this);
    addToArrangementButton->setColour (TextButton::buttonColourId, Colour (0xffbababa));

    addAndMakeVisible (addToSceneButton = new TextButton ("addToSceneButton"));
    addToSceneButton->setButtonText (TRANS("Add to scene"));
    addToSceneButton->addListener (this);
    addToSceneButton->setColour (TextButton::buttonColourId, Colour (0xffbababa));

    addAndMakeVisible (removeButton = new TextButton ("removebutton"));
    removeButton->setButtonText (TRANS("Remove"));
    removeButton->addListener (this);
    removeButton->setColour (TextButton::buttonColourId, Colour (0xffababab));

    addAndMakeVisible (clipPoolLabel = new Label ("clipPoolLabel",
                                                  TRANS("Clip Pool 1")));
    clipPoolLabel->setFont (Font (15.00f, Font::plain));
    clipPoolLabel->setJustificationType (Justification::centredLeft);
    clipPoolLabel->setEditable (false, false, false);
    clipPoolLabel->setColour (TextEditor::textColourId, Colours::black);
    clipPoolLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (clipPoolList = new ClipPoolList (backend_));
    clipPoolList->setName ("clipPoolList");


    //[UserPreSize]
    removeButton->setEnabled(false);
    addToSceneButton->setEnabled(false);
    addToArrangementButton->setEnabled(false);

    addAndMakeVisible(toolbar_=new Toolbar());
    toolbar_->setVertical(true);
    toolbar_->setColour(Toolbar::backgroundColourId, Colour(0xffababab));
    toolbar_->setColour(Toolbar::buttonMouseOverBackgroundColourId, Colour(0xffababab));
    toolbar_->setColour(Toolbar::buttonMouseDownBackgroundColourId, Colour(0xffababab));
    toolbar_->setStyle(Toolbar::iconsOnly);
    tm_=new ClipPoolFactory(this);
    toolbar_->addDefaultItems(*tm_);

    addToArrangementButton->setVisible(false);

    //[/UserPreSize]

    setSize (700, 300);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ClipPoolWidget::~ClipPoolWidget()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    addToArrangementButton = nullptr;
    addToSceneButton = nullptr;
    removeButton = nullptr;
    clipPoolLabel = nullptr;
    clipPoolList = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ClipPoolWidget::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setColour (Colour (0xffababab));
    g.drawRoundedRectangle (4.0f, 4.0f, static_cast<float> (getWidth() - 8), static_cast<float> (getHeight() - 8), 10.000f, 1.000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ClipPoolWidget::resized()
{
    addToArrangementButton->setBounds (getWidth() - 16 - 136, getHeight() - 16 - 24, 136, 24);
    addToSceneButton->setBounds (getWidth() - 16 - 104, getHeight() - 16 - 24, 104, 24);
    removeButton->setBounds (16, getHeight() - 16 - 24, 112, 24);
    clipPoolLabel->setBounds (16, 16, 128, 16);
    clipPoolList->setBounds (16, 44, proportionOfWidth (0.5000f), getHeight() - 98);
    //[UserResized] Add your own custom resize handling here..
    toolbar_->setBounds(getWidth()-40,12,32,128);
    //[/UserResized]
}

void ClipPoolWidget::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == addToArrangementButton)
    {
        //[UserButtonCode_addToArrangementButton] -- add your button handler code here..
        clipPoolList->addToArrangement();
        //[/UserButtonCode_addToArrangementButton]
    }
    else if (buttonThatWasClicked == addToSceneButton)
    {
        //[UserButtonCode_addToSceneButton] -- add your button handler code here..
        clipPoolList->addToScene();

        //[/UserButtonCode_addToSceneButton]
    }
    else if (buttonThatWasClicked == removeButton)
    {
        //[UserButtonCode_removeButton] -- add your button handler code here..
        clipPoolList->removeFromClipPool();
        //[/UserButtonCode_removeButton]
    }

    //[UserbuttonClicked_Post]
    ToolbarItemComponent * tbi=dynamic_cast<ToolbarItemComponent*>(buttonThatWasClicked);
    if(tbi!=0)
    {
//      std::cout<<"Toolbar button clicked "<<tbi->getItemId()<<std::endl;
        int clipFilter=tbi->getItemId();
        if(clipFilter==4)
        {
            addToSceneButton->setVisible(false);
            addToArrangementButton->setVisible(true);
        }
        else
        {
            addToSceneButton->setVisible(true);
            addToArrangementButton->setVisible(false);
        }
        clipPoolList->filterByType(clipFilter);
    }
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ClipPoolWidget::clipSelected(Clip* clip)
{
    if(clip==0)
    {
        removeButton->setEnabled(false);
        addToSceneButton->setEnabled(false);
        addToArrangementButton->setEnabled(false);
    }
    else
    {
        removeButton->setEnabled(true);
        addToSceneButton->setEnabled(true);
        addToArrangementButton->setEnabled(true);
    }
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ClipPoolWidget" componentName=""
                 parentClasses="public Component, public ClipSelector" constructorParams="ClipPoolBackend * backend"
                 variableInitialisers="backend_(backend)" snapPixels="4" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="0" initialWidth="700"
                 initialHeight="300">
  <BACKGROUND backgroundColour="ffffffff">
    <ROUNDRECT pos="4 4 8M 8M" cornerSize="10" fill="solid: ffffff" hasStroke="1"
               stroke="1, mitered, butt" strokeColour="solid: ffababab"/>
  </BACKGROUND>
  <TEXTBUTTON name="addToArrangementButton" id="7294b3fcec49c8cd" memberName="addToArrangementButton"
              virtualName="" explicitFocusOrder="0" pos="16Rr 16Rr 136 24"
              bgColOff="ffbababa" buttonText="Add to arrangement" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="addToSceneButton" id="d39ef258df213911" memberName="addToSceneButton"
              virtualName="" explicitFocusOrder="0" pos="16Rr 16Rr 104 24"
              bgColOff="ffbababa" buttonText="Add to scene" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="removebutton" id="bb69f6ae4a6d5919" memberName="removeButton"
              virtualName="" explicitFocusOrder="0" pos="16 16Rr 112 24" bgColOff="ffababab"
              buttonText="Remove" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="clipPoolLabel" id="12874305da829096" memberName="clipPoolLabel"
         virtualName="" explicitFocusOrder="0" pos="16 16 128 16" edTextCol="ff000000"
         edBkgCol="0" labelText="Clip Pool 1" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="clipPoolList" id="35654b8c9f430388" memberName="clipPoolList"
                    virtualName="" explicitFocusOrder="0" pos="16 44 49.941% 98M"
                    class="ClipPoolList" params="backend_"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
