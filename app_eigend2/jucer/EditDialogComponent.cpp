/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  5 Jul 2012 5:18:59pm

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

#include "EditDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
EditDialogComponent::EditDialogComponent ()
    : description (0),
      save_button (0),
      summary (0),
      label (0),
      label2 (0),
      help_button (0),
      slot (0),
      cachedImage_backgroundBoxInner_png (0),
      cachedImage_backgroundBoxT_png (0),
      cachedImage_backgroundBoxL_png (0),
      cachedImage_backgroundBoxR_png (0),
      cachedImage_backgroundBoxTl_png (0),
      cachedImage_backgroundBoxTr_png (0),
      cachedImage_backgroundBoxB_png (0),
      cachedImage_backgroundBoxBr_png (0),
      cachedImage_backgroundBoxBl_png (0),
      cachedImage_textBoxTl_png (0),
      cachedImage_textBoxTr_png (0),
      cachedImage_textBoxBr_png (0),
      cachedImage_textBoxBl_png (0),
      cachedImage_textBoxL_png (0),
      cachedImage_textBoxT_png (0),
      cachedImage_textBoxR_png (0),
      cachedImage_textBoxB_png (0),
      cachedImage_textBoxInner_png (0),
      cachedImage_eigenD_png (0)
{
    addAndMakeVisible (description = new TextEditor (L"new text editor"));
    description->setMultiLine (true);
    description->setReturnKeyStartsNewLine (true);
    description->setReadOnly (false);
    description->setScrollbarsShown (true);
    description->setCaretVisible (true);
    description->setPopupMenuEnabled (false);
    description->setColour (TextEditor::textColourId, Colours::black);
    description->setColour (TextEditor::backgroundColourId, Colour (0x363636));
    description->setColour (TextEditor::highlightColourId, Colour (0x8c8b8b8b));
    description->setColour (TextEditor::shadowColourId, Colour (0x0));
    description->setText (String::empty);

    addAndMakeVisible (save_button = new TextButton (L"Save"));
    save_button->addListener (this);
    save_button->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (summary = new TextEditor (L"new text editor"));
    summary->setMultiLine (false);
    summary->setReturnKeyStartsNewLine (false);
    summary->setReadOnly (false);
    summary->setScrollbarsShown (true);
    summary->setCaretVisible (true);
    summary->setPopupMenuEnabled (true);
    summary->setColour (TextEditor::highlightColourId, Colour (0x8c8a8a8a));
    summary->setColour (TextEditor::shadowColourId, Colour (0x0));
    summary->setText (String::empty);

    addAndMakeVisible (label = new Label (L"new label",
                                          L"Short Tag:"));
    label->setFont (Font (15.0000f, Font::plain));
    label->setJustificationType (Justification::centredRight);
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label2 = new Label (L"new label",
                                           L"Long Description:"));
    label2->setFont (Font (15.0000f, Font::plain));
    label2->setJustificationType (Justification::centredRight);
    label2->setEditable (false, false, false);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (help_button = new TextButton (L"new button"));
    help_button->setButtonText (L"About Setups");
    help_button->addListener (this);
    help_button->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (slot = new Label (L"slot",
                                         String::empty));
    slot->setFont (Font (19.6000f, Font::plain));
    slot->setJustificationType (Justification::centredLeft);
    slot->setEditable (false, false, false);
    slot->setColour (TextEditor::textColourId, Colours::black);
    slot->setColour (TextEditor::backgroundColourId, Colour (0x0));

    cachedImage_backgroundBoxInner_png = ImageCache::getFromMemory (backgroundBoxInner_png, backgroundBoxInner_pngSize);
    cachedImage_backgroundBoxT_png = ImageCache::getFromMemory (backgroundBoxT_png, backgroundBoxT_pngSize);
    cachedImage_backgroundBoxL_png = ImageCache::getFromMemory (backgroundBoxL_png, backgroundBoxL_pngSize);
    cachedImage_backgroundBoxR_png = ImageCache::getFromMemory (backgroundBoxR_png, backgroundBoxR_pngSize);
    cachedImage_backgroundBoxTl_png = ImageCache::getFromMemory (backgroundBoxTl_png, backgroundBoxTl_pngSize);
    cachedImage_backgroundBoxTr_png = ImageCache::getFromMemory (backgroundBoxTr_png, backgroundBoxTr_pngSize);
    cachedImage_backgroundBoxB_png = ImageCache::getFromMemory (backgroundBoxB_png, backgroundBoxB_pngSize);
    cachedImage_backgroundBoxBr_png = ImageCache::getFromMemory (backgroundBoxBr_png, backgroundBoxBr_pngSize);
    cachedImage_backgroundBoxBl_png = ImageCache::getFromMemory (backgroundBoxBl_png, backgroundBoxBl_pngSize);
    cachedImage_textBoxTl_png = ImageCache::getFromMemory (textBoxTl_png, textBoxTl_pngSize);
    cachedImage_textBoxTr_png = ImageCache::getFromMemory (textBoxTr_png, textBoxTr_pngSize);
    cachedImage_textBoxBr_png = ImageCache::getFromMemory (textBoxBr_png, textBoxBr_pngSize);
    cachedImage_textBoxBl_png = ImageCache::getFromMemory (textBoxBl_png, textBoxBl_pngSize);
    cachedImage_textBoxL_png = ImageCache::getFromMemory (textBoxL_png, textBoxL_pngSize);
    cachedImage_textBoxT_png = ImageCache::getFromMemory (textBoxT_png, textBoxT_pngSize);
    cachedImage_textBoxR_png = ImageCache::getFromMemory (textBoxR_png, textBoxR_pngSize);
    cachedImage_textBoxB_png = ImageCache::getFromMemory (textBoxB_png, textBoxB_pngSize);
    cachedImage_textBoxInner_png = ImageCache::getFromMemory (textBoxInner_png, textBoxInner_pngSize);
    cachedImage_eigenD_png = ImageCache::getFromMemory (eigenD_png, eigenD_pngSize);

    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

EditDialogComponent::~EditDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (description);
    deleteAndZero (save_button);
    deleteAndZero (summary);
    deleteAndZero (label);
    deleteAndZero (label2);
    deleteAndZero (help_button);
    deleteAndZero (slot);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void EditDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::black);

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxInner_png,
                 26, 98, getWidth() - 49, getHeight() - 125,
                 0, 0, cachedImage_backgroundBoxInner_png.getWidth(), cachedImage_backgroundBoxInner_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxT_png,
                 24, 84, getWidth() - 45, 15,
                 0, 0, cachedImage_backgroundBoxT_png.getWidth(), cachedImage_backgroundBoxT_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxL_png,
                 12, 98, 15, getHeight() - 123,
                 0, 0, cachedImage_backgroundBoxL_png.getWidth(), cachedImage_backgroundBoxL_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxR_png,
                 getWidth() - 28, 98, 15, getHeight() - 123,
                 0, 0, cachedImage_backgroundBoxR_png.getWidth(), cachedImage_backgroundBoxR_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxTl_png,
                 12, 84, 30, 15,
                 0, 0, cachedImage_backgroundBoxTl_png.getWidth(), cachedImage_backgroundBoxTl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxTr_png,
                 getWidth() - 43, 84, 30, 15,
                 0, 0, cachedImage_backgroundBoxTr_png.getWidth(), cachedImage_backgroundBoxTr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxB_png,
                 36, getHeight() - 31, getWidth() - 75, 15,
                 0, 0, cachedImage_backgroundBoxB_png.getWidth(), cachedImage_backgroundBoxB_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxBr_png,
                 getWidth() - 43, getHeight() - 31, 30, 15,
                 0, 0, cachedImage_backgroundBoxBr_png.getWidth(), cachedImage_backgroundBoxBr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxBl_png,
                 12, getHeight() - 31, 30, 15,
                 0, 0, cachedImage_backgroundBoxBl_png.getWidth(), cachedImage_backgroundBoxBl_png.getHeight());

    g.setColour (Colours::black);
    g.setFont (Font (19.6000f, Font::plain));
    g.drawText (L"Edit Setup",
                106 - ((140) / 2), 123 - ((30) / 2), 140, 30,
                Justification::centred, true);

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxTl_png,
                 196, 162, 12, 12,
                 0, 0, cachedImage_textBoxTl_png.getWidth(), cachedImage_textBoxTl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxTr_png,
                 getWidth() - 43, 162, 12, 12,
                 0, 0, cachedImage_textBoxTr_png.getWidth(), cachedImage_textBoxTr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxBr_png,
                 getWidth() - 43, 186, 12, 12,
                 0, 0, cachedImage_textBoxBr_png.getWidth(), cachedImage_textBoxBr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxBl_png,
                 196, 186, 12, 12,
                 0, 0, cachedImage_textBoxBl_png.getWidth(), cachedImage_textBoxBl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxL_png,
                 196, 174, 12, 12,
                 0, 0, cachedImage_textBoxL_png.getWidth(), cachedImage_textBoxL_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxT_png,
                 208, 162, getWidth() - 251, 12,
                 0, 0, cachedImage_textBoxT_png.getWidth(), cachedImage_textBoxT_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxR_png,
                 getWidth() - 43, 174, 12, 12,
                 0, 0, cachedImage_textBoxR_png.getWidth(), cachedImage_textBoxR_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxB_png,
                 208, 186, getWidth() - 251, 12,
                 0, 0, cachedImage_textBoxB_png.getWidth(), cachedImage_textBoxB_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxInner_png,
                 204, 170, getWidth() - 243, 20,
                 0, 0, cachedImage_textBoxInner_png.getWidth(), cachedImage_textBoxInner_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxTl_png,
                 196, 214, 12, 12,
                 0, 0, cachedImage_textBoxTl_png.getWidth(), cachedImage_textBoxTl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxTr_png,
                 getWidth() - 43, 214, 12, 12,
                 0, 0, cachedImage_textBoxTr_png.getWidth(), cachedImage_textBoxTr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxBr_png,
                 getWidth() - 43, getHeight() - 100, 12, 12,
                 0, 0, cachedImage_textBoxBr_png.getWidth(), cachedImage_textBoxBr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxBl_png,
                 196, getHeight() - 100, 12, 12,
                 0, 0, cachedImage_textBoxBl_png.getWidth(), cachedImage_textBoxBl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxL_png,
                 196, 226, 12, getHeight() - 326,
                 0, 0, cachedImage_textBoxL_png.getWidth(), cachedImage_textBoxL_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxT_png,
                 208, 214, getWidth() - 251, 12,
                 0, 0, cachedImage_textBoxT_png.getWidth(), cachedImage_textBoxT_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxR_png,
                 getWidth() - 43, 226, 12, getHeight() - 326,
                 0, 0, cachedImage_textBoxR_png.getWidth(), cachedImage_textBoxR_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxB_png,
                 208, getHeight() - 100, getWidth() - 251, 12,
                 0, 0, cachedImage_textBoxB_png.getWidth(), cachedImage_textBoxB_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxInner_png,
                 204, 223, getWidth() - 243, getHeight() - 320,
                 0, 0, cachedImage_textBoxInner_png.getWidth(), cachedImage_textBoxInner_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_eigenD_png,
                 getWidth() - 177, 12, 146, 60,
                 0, 0, cachedImage_eigenD_png.getWidth(), cachedImage_eigenD_png.getHeight());

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void EditDialogComponent::resized()
{
    description->setBounds (208, 224, getWidth() - 250, getHeight() - 325);
    save_button->setBounds ((getWidth() / 2) - ((50) / 2), getHeight() - 67, 50, 24);
    summary->setBounds (208, 168, getWidth() - 250, 24);
    label->setBounds (182 - 150, 168, 150, 24);
    label2->setBounds (182 - 150, 216, 150, 24);
    help_button->setBounds (getWidth() - 127, 124 - ((24) / 2), 96, 24);
    slot->setBounds (200, 112, 150, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void EditDialogComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == save_button)
    {
        //[UserButtonCode_save_button] -- add your button handler code here..
        //[/UserButtonCode_save_button]
    }
    else if (buttonThatWasClicked == help_button)
    {
        //[UserButtonCode_help_button] -- add your button handler code here..
        //[/UserButtonCode_help_button]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="EditDialogComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ff000000">
    <IMAGE pos="26 98 49M 125M" resource="backgroundBoxInner_png" opacity="1"
           mode="0"/>
    <IMAGE pos="24 84 45M 15" resource="backgroundBoxT_png" opacity="1"
           mode="0"/>
    <IMAGE pos="12 98 15 123M" resource="backgroundBoxL_png" opacity="1"
           mode="0"/>
    <IMAGE pos="28R 98 15 123M" resource="backgroundBoxR_png" opacity="1"
           mode="0"/>
    <IMAGE pos="12 84 30 15" resource="backgroundBoxTl_png" opacity="1"
           mode="0"/>
    <IMAGE pos="43R 84 30 15" resource="backgroundBoxTr_png" opacity="1"
           mode="0"/>
    <IMAGE pos="36 31R 75M 15" resource="backgroundBoxB_png" opacity="1"
           mode="0"/>
    <IMAGE pos="43R 31R 30 15" resource="backgroundBoxBr_png" opacity="1"
           mode="0"/>
    <IMAGE pos="12 31R 30 15" resource="backgroundBoxBl_png" opacity="1"
           mode="0"/>
    <TEXT pos="106c 123c 140 30" fill="solid: ff000000" hasStroke="0" text="Edit Setup"
          fontname="Default font" fontsize="19.6" bold="0" italic="0" justification="36"/>
    <IMAGE pos="196 162 12 12" resource="textBoxTl_png" opacity="1" mode="0"/>
    <IMAGE pos="43R 162 12 12" resource="textBoxTr_png" opacity="1" mode="0"/>
    <IMAGE pos="43R 186 12 12" resource="textBoxBr_png" opacity="1" mode="0"/>
    <IMAGE pos="196 186 12 12" resource="textBoxBl_png" opacity="1" mode="0"/>
    <IMAGE pos="196 174 12 12" resource="textBoxL_png" opacity="1" mode="0"/>
    <IMAGE pos="208 162 251M 12" resource="textBoxT_png" opacity="1" mode="0"/>
    <IMAGE pos="43R 174 12 12" resource="textBoxR_png" opacity="1" mode="0"/>
    <IMAGE pos="208 186 251M 12" resource="textBoxB_png" opacity="1" mode="0"/>
    <IMAGE pos="204 170 243M 20" resource="textBoxInner_png" opacity="1"
           mode="0"/>
    <IMAGE pos="196 214 12 12" resource="textBoxTl_png" opacity="1" mode="0"/>
    <IMAGE pos="43R 214 12 12" resource="textBoxTr_png" opacity="1" mode="0"/>
    <IMAGE pos="43R 100R 12 12" resource="textBoxBr_png" opacity="1" mode="0"/>
    <IMAGE pos="196 100R 12 12" resource="textBoxBl_png" opacity="1" mode="0"/>
    <IMAGE pos="196 226 12 326M" resource="textBoxL_png" opacity="1" mode="0"/>
    <IMAGE pos="208 214 251M 12" resource="textBoxT_png" opacity="1" mode="0"/>
    <IMAGE pos="43R 226 12 326M" resource="textBoxR_png" opacity="1" mode="0"/>
    <IMAGE pos="208 100R 251M 12" resource="textBoxB_png" opacity="1" mode="0"/>
    <IMAGE pos="204 223 243M 320M" resource="textBoxInner_png" opacity="1"
           mode="0"/>
    <IMAGE pos="177R 12 146 60" resource="eigenD_png" opacity="1" mode="0"/>
  </BACKGROUND>
  <TEXTEDITOR name="new text editor" id="b5761399f2e9dcd7" memberName="description"
              virtualName="" explicitFocusOrder="0" pos="208 224 250M 325M"
              textcol="ff000000" bkgcol="363636" hilitecol="8c8b8b8b" shadowcol="0"
              initialText="" multiline="1" retKeyStartsLine="1" readonly="0"
              scrollbars="1" caret="1" popupmenu="0"/>
  <TEXTBUTTON name="Save" id="f494606ca745ecc4" memberName="save_button" virtualName=""
              explicitFocusOrder="0" pos="0Cc 67R 50 24" bgColOff="ffaeaeae"
              buttonText="Save" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="new text editor" id="fb9c84709fca8fae" memberName="summary"
              virtualName="" explicitFocusOrder="0" pos="208 168 250M 24" hilitecol="8c8a8a8a"
              shadowcol="0" initialText="" multiline="0" retKeyStartsLine="0"
              readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="new label" id="5677b737325eee92" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="182r 168 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Short Tag:" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="dad4f1e9843aba4d" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="182r 216 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Long Description:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <TEXTBUTTON name="new button" id="3ae8cdb38f941043" memberName="help_button"
              virtualName="" explicitFocusOrder="0" pos="127R 124c 96 24" bgColOff="ffaeaeae"
              buttonText="About Setups" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
  <LABEL name="slot" id="6699eaed9863b0b7" memberName="slot" virtualName=""
         explicitFocusOrder="0" pos="200 112 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="19.6"
         bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: backgroundBoxT_png, 279, "BackgroundBoxT.png"
static const unsigned char resource_EditDialogComponent_backgroundBoxT_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,19,
0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,
37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,
48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,38,73,68,65,84,120,156,98,248,242,233,211,
127,114,49,195,72,212,252,249,227,199,255,228,226,81,205,163,154,9,98,0,0,0,0,255,255,3,0,54,84,94,99,248,244,114,238,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::backgroundBoxT_png = (const char*) resource_EditDialogComponent_backgroundBoxT_png;
const int EditDialogComponent::backgroundBoxT_pngSize = 279;

// JUCER_RESOURCE: backgroundBoxL_png, 292, "BackgroundBoxL.png"
static const unsigned char resource_EditDialogComponent_backgroundBoxL_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,19,
0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,
37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,
48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,51,73,68,65,84,120,156,98,176,181,177,169,
0,226,255,48,28,22,26,250,127,209,194,133,255,239,221,185,243,255,243,199,143,120,49,195,168,230,81,205,163,154,169,172,25,0,0,0,255,255,3,0,173,65,168,191,39,5,63,71,0,0,0,0,73,69,78,68,174,66,96,130,
0,0};

const char* EditDialogComponent::backgroundBoxL_png = (const char*) resource_EditDialogComponent_backgroundBoxL_png;
const int EditDialogComponent::backgroundBoxL_pngSize = 292;

// JUCER_RESOURCE: backgroundBoxR_png, 296, "BackgroundBoxR.png"
static const unsigned char resource_EditDialogComponent_backgroundBoxR_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,19,
0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,
37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,
48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,55,73,68,65,84,120,156,236,203,161,17,0,
48,8,4,65,250,239,22,194,48,240,230,82,66,76,36,98,229,218,113,231,37,35,168,76,186,138,233,70,51,72,194,54,111,222,252,57,95,0,0,0,255,255,3,0,22,184,99,19,252,156,58,252,0,0,0,0,73,69,78,68,174,66,96,
130,0,0};

const char* EditDialogComponent::backgroundBoxR_png = (const char*) resource_EditDialogComponent_backgroundBoxR_png;
const int EditDialogComponent::backgroundBoxR_pngSize = 296;

// JUCER_RESOURCE: backgroundBoxB_png, 316, "BackgroundBoxB.png"
static const unsigned char resource_EditDialogComponent_backgroundBoxB_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,19,
0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,
37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,
48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,75,73,68,65,84,120,156,236,203,177,13,0,
16,0,4,192,223,155,154,25,132,41,152,66,12,33,6,16,17,3,188,29,190,147,40,174,60,156,189,169,194,207,207,228,53,39,85,24,189,83,133,86,43,85,40,57,83,133,20,35,85,240,206,81,5,107,76,80,93,0,0,0,255,255,
3,0,110,207,10,168,241,237,162,147,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::backgroundBoxB_png = (const char*) resource_EditDialogComponent_backgroundBoxB_png;
const int EditDialogComponent::backgroundBoxB_pngSize = 316;

// JUCER_RESOURCE: backgroundBoxTl_png, 612, "BackgroundBoxTL.png"
static const unsigned char resource_EditDialogComponent_backgroundBoxTl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,15,8,6,0,0,0,243,186,127,156,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,115,73,68,65,84,120,156,188,212,63,75,
66,81,24,199,113,167,222,75,239,193,2,115,115,49,111,218,96,163,13,246,6,12,3,91,108,176,193,6,205,193,12,211,48,105,205,150,180,180,192,64,107,243,31,114,77,18,73,196,52,255,92,43,90,170,95,231,220,80,
142,232,230,209,225,59,127,56,207,195,115,20,0,20,147,58,112,58,213,103,161,80,226,54,30,151,10,249,252,79,163,94,199,187,36,253,215,239,227,99,202,198,64,65,167,91,220,52,153,196,93,155,13,199,62,31,
174,163,81,16,24,175,141,198,16,158,22,29,131,151,148,74,129,244,189,110,48,96,219,98,145,225,187,68,2,79,162,136,110,187,205,13,29,129,9,184,76,250,37,97,85,171,197,142,213,10,50,106,60,164,82,120,169,
86,185,190,118,8,19,108,129,244,73,81,26,25,55,246,236,118,92,70,34,40,228,114,35,99,230,186,99,130,5,6,40,109,195,104,132,199,237,198,125,50,137,231,114,25,111,205,230,40,204,161,1,252,197,194,91,102,
51,206,195,97,228,50,25,212,107,53,180,91,45,244,58,29,244,123,61,57,46,48,129,212,44,74,163,251,189,137,197,80,173,84,100,84,234,118,185,129,44,236,97,209,21,149,10,251,14,7,30,211,105,52,201,110,41,
202,123,204,3,248,130,133,53,26,13,14,93,46,228,179,89,249,132,120,191,148,133,175,88,152,158,210,145,215,11,177,88,156,25,58,17,94,19,4,156,248,253,40,151,74,51,67,39,194,6,189,30,193,64,64,62,163,185,
195,167,193,224,252,97,250,79,207,3,254,3,0,0,255,255,3,0,111,74,2,27,78,221,106,98,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::backgroundBoxTl_png = (const char*) resource_EditDialogComponent_backgroundBoxTl_png;
const int EditDialogComponent::backgroundBoxTl_pngSize = 612;

// JUCER_RESOURCE: backgroundBoxTr_png, 456, "BackgroundBoxTR.png"
static const unsigned char resource_EditDialogComponent_backgroundBoxTr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,15,8,6,0,0,0,243,186,127,156,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,215,73,68,65,84,120,156,228,211,93,10,
194,48,12,192,241,128,224,85,60,143,79,130,199,240,24,222,99,135,17,188,129,224,29,182,38,253,136,66,76,102,59,177,236,177,5,193,135,255,211,96,191,165,233,0,167,73,154,229,156,144,115,79,66,188,122,162,
33,16,237,131,247,27,13,162,22,114,250,12,160,37,76,111,88,60,162,232,203,69,17,81,240,18,67,216,105,80,50,188,41,140,107,112,8,150,79,49,158,52,176,12,239,2,83,5,39,43,70,235,92,240,182,112,53,181,238,
216,142,122,129,57,37,235,160,1,184,113,148,86,213,251,158,241,50,245,7,190,105,219,166,112,253,1,51,110,83,127,195,194,204,199,46,240,92,198,203,212,21,60,244,131,243,228,54,245,114,220,25,126,48,223,
187,194,214,12,87,199,173,112,234,14,227,218,158,153,165,63,108,123,46,191,213,255,192,122,179,127,6,126,1,0,0,255,255,3,0,44,184,173,26,174,253,155,180,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::backgroundBoxTr_png = (const char*) resource_EditDialogComponent_backgroundBoxTr_png;
const int EditDialogComponent::backgroundBoxTr_pngSize = 456;

// JUCER_RESOURCE: backgroundBoxBl_png, 696, "BackgroundBoxBL.png"
static const unsigned char resource_EditDialogComponent_backgroundBoxBl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,15,8,6,0,0,0,243,186,127,156,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,199,73,68,65,84,120,156,188,212,223,47,
66,97,28,199,241,110,253,31,254,140,108,44,178,210,230,199,24,99,93,101,174,200,148,77,68,145,112,37,195,108,78,233,151,173,152,101,200,102,205,214,112,17,253,226,194,48,75,105,205,12,37,21,155,205,213,
199,57,207,202,142,184,81,234,226,125,253,250,62,231,251,60,135,83,197,229,42,232,144,171,173,181,21,171,86,43,110,195,97,188,101,50,37,139,243,27,108,181,88,202,15,183,52,55,195,108,50,33,114,115,83,
94,88,36,18,65,79,81,8,93,95,227,53,157,46,31,92,203,227,97,97,126,30,23,231,231,72,191,188,148,20,150,176,97,38,237,228,36,252,94,47,18,79,79,37,59,53,3,215,229,195,253,82,41,92,123,123,136,69,163,72,
38,18,37,193,25,184,130,238,157,13,55,53,54,146,61,159,6,2,184,139,197,240,28,143,35,147,74,253,235,0,28,128,224,182,252,83,203,6,6,224,216,216,64,208,239,39,55,252,225,254,30,241,199,71,242,5,82,201,
100,209,229,224,74,186,15,54,92,207,231,99,92,173,198,154,205,134,3,183,27,103,193,32,174,46,47,17,14,133,16,141,68,200,26,138,137,192,89,92,145,127,106,230,103,162,26,27,131,65,175,199,166,195,129,125,
151,11,71,135,135,56,246,120,224,59,57,33,23,144,41,224,243,253,185,47,56,139,155,243,241,174,206,78,140,12,15,99,78,167,131,201,104,196,186,221,78,134,216,222,218,130,115,103,7,187,78,103,65,125,131,
179,184,138,253,217,107,170,171,33,22,139,33,151,201,160,213,104,160,155,157,197,210,226,34,168,229,101,114,1,87,12,134,130,250,1,179,118,110,203,221,118,161,80,72,240,190,222,94,40,134,134,200,238,153,
33,166,180,90,204,76,79,23,212,175,48,107,0,230,169,53,8,4,130,158,142,246,246,137,110,137,132,162,223,56,53,40,151,83,244,0,20,189,2,106,84,169,44,168,79,0,0,0,255,255,3,0,185,224,111,32,20,80,120,234,
0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::backgroundBoxBl_png = (const char*) resource_EditDialogComponent_backgroundBoxBl_png;
const int EditDialogComponent::backgroundBoxBl_pngSize = 696;

// JUCER_RESOURCE: backgroundBoxBr_png, 646, "BackgroundBoxBR.png"
static const unsigned char resource_EditDialogComponent_backgroundBoxBr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,15,8,6,0,0,0,243,186,127,156,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,149,73,68,65,84,120,156,228,210,205,74,
2,97,20,198,241,23,186,134,46,160,85,16,116,85,173,186,6,205,48,240,35,204,176,149,141,102,6,217,66,90,105,154,180,144,48,204,80,147,48,177,4,9,27,45,36,241,123,212,81,17,158,206,76,14,218,215,102,194,
90,180,248,51,203,223,57,103,94,38,180,90,152,101,157,118,27,189,78,7,253,94,15,131,126,31,195,193,0,195,225,16,236,79,96,234,119,224,110,247,63,193,130,240,21,60,154,57,220,253,0,143,241,242,76,97,233,
204,223,192,241,217,195,244,162,197,49,220,23,69,5,215,176,70,173,134,159,214,172,215,209,106,52,208,110,54,223,161,210,255,85,96,81,129,69,113,68,240,34,227,11,5,168,169,248,248,40,87,226,121,60,21,139,
120,46,149,240,82,46,163,94,173,126,130,123,10,252,134,187,41,198,146,241,56,212,118,157,72,32,149,76,226,38,149,66,38,157,70,238,254,94,30,74,198,37,88,249,199,147,173,121,106,158,6,96,236,196,239,135,
218,130,129,0,78,131,65,156,133,66,56,15,135,113,21,139,201,3,72,87,152,126,92,227,173,107,132,47,19,204,164,216,158,211,9,181,237,187,92,56,112,187,113,228,241,224,216,235,149,7,185,136,68,144,205,100,
80,173,84,38,231,22,132,91,218,122,137,112,166,196,76,70,35,212,100,54,153,176,105,54,99,203,98,193,142,205,134,93,187,29,158,195,67,248,125,62,92,70,163,120,200,231,165,173,115,132,175,16,60,71,49,218,
156,209,0,242,151,105,53,26,78,77,107,90,45,183,174,211,113,27,122,61,103,52,24,184,109,171,149,115,58,28,28,109,190,74,103,95,185,203,102,23,232,197,51,130,229,100,120,170,87,0,0,0,255,255,3,0,97,155,
52,159,169,173,107,216,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::backgroundBoxBr_png = (const char*) resource_EditDialogComponent_backgroundBoxBr_png;
const int EditDialogComponent::backgroundBoxBr_pngSize = 646;

// JUCER_RESOURCE: backgroundBoxInner_png, 503, "BackgroundBoxInner.png"
static const unsigned char resource_EditDialogComponent_backgroundBoxInner_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,100,0,0,0,100,8,6,0,0,0,112,226,149,84,0,0,0,9,112,72,89,115,0,
0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,
46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,
48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,6,73,68,65,84,120,156,236,209,49,13,
0,48,12,192,176,242,103,91,13,192,6,99,57,124,248,143,148,57,187,151,142,249,29,128,33,105,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,
140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,
18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,
196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,
33,49,134,196,60,0,0,0,255,255,3,0,183,209,145,252,233,144,222,223,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::backgroundBoxInner_png = (const char*) resource_EditDialogComponent_backgroundBoxInner_png;
const int EditDialogComponent::backgroundBoxInner_pngSize = 503;

// JUCER_RESOURCE: logoText_png, 2177, "LogoText.png"
static const unsigned char resource_EditDialogComponent_logoText_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,166,0,0,0,68,8,0,0,0,0,46,111,17,34,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,
19,1,0,154,156,24,0,0,3,24,105,67,67,80,80,104,111,116,111,115,104,111,112,32,73,67,67,32,112,114,111,102,105,108,101,0,0,120,218,99,96,96,158,224,232,226,228,202,36,192,192,80,80,84,82,228,30,228,24,
25,17,25,165,192,126,158,129,141,129,153,129,129,129,129,129,33,49,185,184,192,49,32,192,135,129,129,129,33,47,63,47,149,1,21,48,50,48,124,187,198,192,200,192,192,192,112,89,215,209,197,201,149,129,52,
192,154,92,80,84,194,192,192,112,128,129,129,193,40,37,181,56,153,129,129,225,11,3,3,67,122,121,73,65,9,3,3,99,12,3,3,131,72,82,118,65,9,3,3,99,1,3,3,131,72,118,72,144,51,3,3,99,11,3,3,19,79,73,106,69,
9,3,3,3,131,115,126,65,101,81,102,122,70,137,130,161,165,165,165,130,99,74,126,82,170,66,112,101,113,73,106,110,177,130,103,94,114,126,81,65,126,81,98,73,106,10,3,3,3,212,14,6,6,6,6,94,151,252,18,5,247,
196,204,60,5,35,3,85,6,42,131,136,200,40,5,8,11,17,62,8,49,4,72,46,45,42,131,7,37,3,131,0,131,2,131,1,131,3,67,0,67,34,67,61,195,2,134,163,12,111,24,197,25,93,24,75,25,87,48,222,99,18,99,10,98,154,192,
116,129,89,152,57,146,121,33,243,27,22,75,150,14,150,91,172,122,172,173,172,247,216,44,217,166,177,125,99,15,103,223,205,161,196,209,197,241,133,51,145,243,2,151,35,215,22,110,77,238,5,60,82,60,83,121,
133,120,39,241,9,243,77,227,151,225,95,44,160,35,176,67,208,85,240,138,80,170,208,15,225,94,17,21,145,189,162,225,162,95,196,38,137,27,137,95,145,168,144,148,147,60,38,149,47,45,45,125,66,166,76,86,93,
246,150,92,159,188,139,252,31,133,173,138,133,74,122,74,111,149,215,170,20,168,154,168,254,84,59,168,222,165,17,170,169,164,249,65,235,128,246,36,157,84,93,43,61,65,189,87,250,71,12,22,24,214,26,197,24,
219,154,200,155,50,155,190,52,187,96,190,211,98,137,229,4,171,58,235,92,155,56,219,64,59,87,123,107,7,99,71,29,39,53,103,37,23,5,87,121,55,5,119,101,15,117,79,93,47,19,111,27,31,119,223,96,191,4,255,252,
128,250,192,137,65,75,131,119,133,92,12,125,25,206,20,33,23,105,21,21,17,93,17,51,51,118,79,220,131,4,182,68,221,164,176,228,134,148,53,169,55,211,57,50,44,50,51,179,230,102,95,204,101,207,179,207,175,
40,216,84,248,174,88,187,36,171,116,85,217,155,10,253,202,146,170,93,53,140,181,94,117,83,235,31,54,234,53,213,52,159,109,149,107,43,108,63,218,41,221,85,212,125,186,87,181,175,177,255,238,68,155,73,179,
39,255,157,26,63,237,240,12,141,153,253,179,190,207,73,152,123,122,190,249,130,165,139,68,22,183,46,249,182,44,115,249,189,149,33,171,78,175,113,89,187,111,189,229,134,109,155,76,54,111,217,106,178,109,
251,14,171,157,251,119,187,238,57,187,47,108,255,131,131,57,135,126,30,105,63,38,126,124,197,73,235,83,231,206,36,159,253,117,126,210,69,237,75,71,175,36,94,253,119,125,206,77,155,91,119,239,212,223,83,
190,127,226,97,222,99,177,39,251,159,101,190,16,121,121,240,117,254,91,249,119,23,62,52,125,50,253,252,234,235,130,239,225,63,5,126,157,250,211,250,207,241,255,127,0,13,0,15,52,250,150,241,93,0,0,0,32,
99,72,82,77,0,0,122,37,0,0,128,131,0,0,249,255,0,0,128,233,0,0,117,48,0,0,234,96,0,0,58,152,0,0,23,111,146,95,197,70,0,0,4,227,73,68,65,84,120,218,236,154,91,76,92,69,24,199,255,115,128,189,192,178,176,
11,148,182,164,218,2,109,66,162,54,22,154,24,47,209,7,74,188,196,62,152,80,210,164,246,173,128,111,250,82,208,196,168,137,201,178,125,244,73,240,217,164,151,71,227,131,128,81,31,140,49,176,36,141,77,108,
19,88,170,182,5,182,156,165,236,178,23,182,187,243,249,48,51,167,236,141,93,16,215,33,113,94,206,204,153,217,61,191,252,191,111,254,243,157,205,50,134,74,53,250,7,159,53,176,47,154,22,152,172,102,95,96,
26,173,251,2,179,166,73,103,76,107,247,58,180,86,179,69,113,214,180,233,140,249,172,83,65,28,211,25,243,132,67,118,170,181,198,60,168,212,116,121,116,198,108,85,106,122,15,48,157,49,237,178,211,232,169,
214,24,179,89,5,189,214,169,33,166,97,147,27,167,197,45,239,212,185,108,250,97,54,246,201,140,108,85,106,214,217,236,250,97,54,188,42,113,27,93,106,167,219,107,245,195,108,121,70,194,161,89,169,105,56,
244,195,60,122,92,212,109,245,120,90,222,113,194,169,31,230,201,35,205,18,83,85,28,245,168,211,15,243,136,173,11,0,224,70,189,50,36,120,245,195,108,195,81,177,149,224,182,212,108,211,14,211,118,16,79,
73,76,165,166,11,30,237,48,219,14,91,152,46,113,248,48,15,244,51,164,206,70,25,116,47,156,98,131,187,90,44,93,53,82,19,233,19,30,0,56,4,135,176,75,187,115,119,106,118,243,153,221,33,116,115,206,185,233,
27,217,110,205,33,100,26,188,0,112,24,181,66,77,103,181,181,153,42,163,230,26,0,120,70,124,230,96,241,53,119,96,95,92,6,96,115,146,67,208,57,109,37,213,4,219,203,230,165,49,198,122,198,102,137,198,243,
39,213,19,223,120,191,29,0,220,139,105,122,29,0,240,92,156,190,175,60,38,99,108,40,76,163,69,49,101,59,176,148,162,139,0,128,23,210,244,75,153,65,31,153,225,124,178,95,14,60,252,218,147,116,243,153,156,
115,159,26,246,79,114,62,111,229,158,57,51,232,227,156,231,229,226,196,0,46,149,210,199,89,203,229,6,119,87,145,163,186,44,53,199,137,136,136,206,40,81,172,152,201,137,49,57,28,18,67,37,85,152,178,199,
94,107,225,117,245,93,69,213,236,218,76,208,71,0,128,119,41,125,219,81,142,154,253,131,129,211,134,225,135,47,107,47,0,248,114,48,232,111,50,78,227,145,148,217,183,54,108,24,195,107,151,173,83,227,198,
176,97,248,113,42,239,123,231,208,93,66,159,191,230,29,153,95,229,222,167,90,91,57,106,78,133,189,66,2,111,182,40,61,52,43,46,82,174,81,58,183,229,194,88,120,150,49,198,58,232,122,158,154,163,249,201,
153,251,228,174,207,222,20,157,79,105,51,212,134,134,75,111,21,167,20,57,209,238,49,197,176,61,144,237,131,248,42,107,124,12,215,68,214,182,231,217,80,78,59,133,64,169,228,252,253,19,217,113,131,92,13,
247,191,184,136,243,87,183,199,196,116,95,153,198,216,84,222,186,222,254,224,116,217,110,251,39,236,161,21,219,43,153,170,151,174,110,159,155,211,189,189,5,103,3,57,27,118,206,51,82,214,131,7,39,113,165,
252,67,97,252,243,111,223,51,51,81,134,13,169,238,139,174,194,185,217,17,166,241,158,124,251,99,108,138,22,198,188,79,114,147,205,210,212,208,150,116,19,185,233,181,114,179,195,178,247,49,86,202,55,115,
219,207,68,31,139,234,243,39,250,161,190,176,33,157,203,50,150,45,134,52,149,109,72,29,11,68,68,22,86,46,102,143,52,168,133,33,182,99,204,239,136,62,0,0,156,165,36,189,86,216,222,111,52,249,131,0,130,
234,182,101,56,125,3,254,173,203,131,157,31,78,111,93,87,48,127,253,195,157,19,59,175,7,98,64,12,0,144,6,33,179,155,195,242,76,129,163,111,231,173,4,230,215,68,23,0,0,47,243,100,242,249,221,84,72,151,
11,122,206,30,183,13,32,10,0,8,39,140,84,28,0,106,186,143,239,0,211,55,217,27,152,248,247,49,227,64,2,0,16,90,175,74,197,0,224,202,236,205,183,179,125,179,104,157,59,3,0,129,129,10,84,245,81,100,226,34,
73,99,70,42,6,192,113,150,156,239,124,83,22,230,26,0,255,98,5,180,4,54,144,17,190,153,138,194,140,137,171,149,108,37,48,131,149,123,87,138,35,45,130,158,137,97,37,5,128,47,159,196,61,13,126,134,205,13,
122,228,145,210,85,136,248,0,152,215,14,51,134,104,196,2,150,197,96,228,150,118,152,97,36,54,69,47,130,117,0,192,109,172,173,106,135,25,194,6,23,189,117,169,230,3,90,141,104,135,25,121,156,132,194,20,
106,62,140,46,65,59,204,228,102,66,189,8,74,76,211,188,163,31,230,166,133,121,87,6,157,207,221,210,15,243,177,56,200,1,60,148,152,248,237,158,142,106,110,40,163,79,202,55,179,153,251,40,239,20,170,96,
203,36,85,208,19,171,43,162,243,35,215,15,19,49,75,77,51,102,21,77,218,5,29,241,152,10,255,106,234,191,255,25,182,180,154,233,229,180,198,152,102,72,97,134,116,86,51,162,14,240,212,18,105,140,249,135,
10,122,102,1,58,99,202,2,9,116,147,105,140,185,172,124,19,139,58,7,221,140,170,30,215,57,232,235,241,226,115,85,76,27,204,76,146,23,157,99,255,255,27,118,15,219,223,3,0,247,239,201,228,119,3,254,21,0,
0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::logoText_png = (const char*) resource_EditDialogComponent_logoText_png;
const int EditDialogComponent::logoText_pngSize = 2177;

// JUCER_RESOURCE: textBoxTl_png, 517, "TextBoxTL.png"
static const unsigned char resource_EditDialogComponent_textBoxTl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,
11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,20,73,68,65,84,120,156,140,145,63,107,131,80,
20,197,133,66,191,74,63,79,21,44,116,112,83,247,6,132,64,4,51,5,250,17,28,116,49,163,8,46,14,226,224,88,232,34,56,212,193,77,76,54,171,66,193,63,241,52,247,65,2,77,45,246,192,157,222,249,93,238,59,135,
3,192,241,60,127,59,119,231,121,20,4,97,175,40,202,251,106,181,26,215,235,53,116,93,7,55,3,60,72,146,244,182,221,110,97,154,38,92,215,69,16,4,8,195,16,81,20,253,2,94,52,77,251,178,109,155,61,38,73,130,
60,207,81,20,5,14,135,3,142,199,227,15,224,213,48,12,120,158,199,140,101,89,162,105,26,116,93,135,113,28,49,77,19,72,23,224,105,179,217,192,247,125,100,89,134,170,170,152,241,116,58,225,86,100,190,151,
101,185,112,28,7,105,154,162,174,107,12,195,112,221,56,7,60,239,118,59,196,113,204,110,236,251,126,214,120,5,68,81,220,91,150,197,182,183,109,251,231,230,43,160,170,234,7,69,71,105,208,221,75,226,206,
165,124,82,206,20,29,165,177,8,80,131,84,10,229,188,116,14,3,40,78,42,137,62,252,31,125,3,0,0,255,255,3,0,206,234,209,136,115,191,70,54,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::textBoxTl_png = (const char*) resource_EditDialogComponent_textBoxTl_png;
const int EditDialogComponent::textBoxTl_pngSize = 517;

// JUCER_RESOURCE: textBoxTr_png, 534, "TextBoxTR.png"
static const unsigned char resource_EditDialogComponent_textBoxTr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,
11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,37,73,68,65,84,120,156,132,210,49,107,131,64,
24,6,96,161,208,191,210,223,83,3,22,58,56,137,144,181,130,16,131,16,167,64,127,130,67,92,146,81,4,151,12,81,193,177,208,69,112,9,69,39,147,232,36,81,40,104,212,183,119,130,208,180,33,126,240,221,246,220,
123,119,223,49,170,170,98,54,155,65,146,164,70,20,197,207,201,100,178,97,89,246,153,244,3,0,230,111,51,174,235,98,183,219,97,187,221,194,52,77,232,186,14,77,211,192,243,252,7,65,79,255,64,154,166,56,157,
78,72,146,4,81,20,33,8,2,208,77,12,195,128,44,203,223,4,189,93,1,178,160,235,58,52,77,131,170,170,80,20,5,142,199,99,15,45,203,194,98,177,0,65,239,87,224,119,181,109,219,195,60,207,177,223,239,97,219,
54,230,243,57,69,47,55,193,144,120,185,92,112,62,159,17,134,33,214,235,53,4,65,248,34,232,241,38,24,170,174,107,208,59,250,190,143,229,114,73,83,94,239,2,154,84,150,101,159,178,90,173,192,113,220,230,
46,24,82,226,56,238,159,124,58,157,198,163,128,190,222,225,112,232,231,68,134,91,141,2,122,44,122,15,199,113,160,40,10,70,1,173,44,203,224,121,30,232,55,250,1,0,0,255,255,3,0,175,133,229,239,22,229,209,
25,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::textBoxTr_png = (const char*) resource_EditDialogComponent_textBoxTr_png;
const int EditDialogComponent::textBoxTr_pngSize = 534;

// JUCER_RESOURCE: textBoxBr_png, 526, "TextBoxBR.png"
static const unsigned char resource_EditDialogComponent_textBoxBr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,
11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,29,73,68,65,84,120,156,140,145,177,106,131,
80,20,134,133,66,31,166,143,19,75,10,93,92,4,31,192,65,146,234,144,197,173,111,32,226,96,167,34,136,139,67,80,200,88,232,36,6,138,96,39,69,55,81,39,53,250,215,115,33,33,208,18,123,225,78,247,251,126,206,
253,15,135,127,156,178,44,17,4,1,52,77,3,183,4,79,211,132,162,40,176,223,239,177,217,108,150,133,211,233,132,44,203,224,251,62,100,89,110,23,133,174,235,144,166,41,28,199,129,36,73,95,55,5,26,167,109,
91,196,113,12,211,52,177,94,175,223,110,10,125,223,179,15,31,14,7,232,186,142,213,106,245,252,167,64,201,195,48,160,174,107,28,143,71,216,182,13,81,20,191,103,225,254,151,48,142,35,155,187,170,42,36,73,
2,207,243,160,170,42,165,63,205,207,28,119,78,164,54,8,108,154,134,213,24,69,17,92,215,197,110,183,35,248,149,96,38,208,140,4,228,121,206,218,32,48,12,67,88,150,5,69,81,186,25,126,153,239,221,69,160,71,
90,10,245,76,213,25,134,193,82,5,65,248,152,193,135,51,120,17,104,221,219,237,150,150,66,61,127,242,60,255,62,131,143,215,169,215,247,7,0,0,255,255,3,0,135,93,228,21,19,153,235,168,0,0,0,0,73,69,78,68,
174,66,96,130,0,0};

const char* EditDialogComponent::textBoxBr_png = (const char*) resource_EditDialogComponent_textBoxBr_png;
const int EditDialogComponent::textBoxBr_pngSize = 526;

// JUCER_RESOURCE: textBoxBl_png, 512, "TextBoxBL.png"
static const unsigned char resource_EditDialogComponent_textBoxBl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,
11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,15,73,68,65,84,120,156,140,145,177,106,131,
112,24,196,133,66,95,165,239,163,149,66,7,39,209,7,112,16,173,14,89,220,250,4,33,224,96,161,139,16,92,28,68,193,177,208,73,29,42,197,77,52,91,170,78,154,232,37,159,52,144,161,193,254,225,198,223,119,255,
187,99,12,195,64,24,134,216,237,118,248,207,99,52,77,67,16,4,168,235,26,211,52,45,3,138,162,252,248,190,143,178,44,113,60,30,151,1,89,150,191,92,215,69,81,20,232,251,126,25,224,121,254,109,179,217,32,
77,83,116,93,183,248,45,134,101,217,103,203,178,16,199,241,28,124,24,134,69,224,94,20,197,111,199,113,144,101,25,154,166,193,225,112,184,233,196,252,66,79,84,175,231,121,200,243,28,251,253,126,206,51,
142,227,77,128,244,186,90,173,176,221,110,145,36,9,170,170,66,219,182,51,72,237,93,28,175,129,187,179,94,84,85,237,109,219,70,20,69,51,72,237,81,229,180,19,101,188,6,46,122,16,4,225,131,220,214,235,53,
168,114,218,137,198,165,35,127,1,23,183,71,142,227,222,37,73,250,60,143,11,93,215,97,154,38,78,0,0,0,255,255,3,0,71,160,209,169,46,96,253,11,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::textBoxBl_png = (const char*) resource_EditDialogComponent_textBoxBl_png;
const int EditDialogComponent::textBoxBl_pngSize = 512;

// JUCER_RESOURCE: textBoxL_png, 278, "TextBoxL.png"
static const unsigned char resource_EditDialogComponent_textBoxL_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,
19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,46,
46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,48,
70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,37,73,68,65,84,120,156,98,168,173,173,253,191,
127,255,254,255,47,95,190,252,79,12,96,24,213,48,170,1,59,0,0,0,0,255,255,3,0,192,63,31,127,25,29,95,216,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::textBoxL_png = (const char*) resource_EditDialogComponent_textBoxL_png;
const int EditDialogComponent::textBoxL_pngSize = 278;

// JUCER_RESOURCE: textBoxT_png, 286, "TextBoxT.png"
static const unsigned char resource_EditDialogComponent_textBoxT_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,
19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,46,
46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,48,
70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,45,73,68,65,84,120,156,98,168,173,173,253,79,10,
102,216,191,127,255,127,82,48,195,203,151,47,255,147,130,25,254,147,8,70,53,12,14,13,0,0,0,0,255,255,3,0,12,89,31,127,241,76,165,114,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::textBoxT_png = (const char*) resource_EditDialogComponent_textBoxT_png;
const int EditDialogComponent::textBoxT_pngSize = 286;

// JUCER_RESOURCE: textBoxR_png, 280, "TextBoxR.png"
static const unsigned char resource_EditDialogComponent_textBoxR_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,
19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,46,
46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,48,
70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,39,73,68,65,84,120,156,98,248,79,4,120,249,242,
229,255,253,251,247,255,175,173,173,253,207,48,170,97,84,3,118,13,0,0,0,0,255,255,3,0,127,138,31,127,172,190,106,191,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::textBoxR_png = (const char*) resource_EditDialogComponent_textBoxR_png;
const int EditDialogComponent::textBoxR_pngSize = 280;

// JUCER_RESOURCE: textBoxB_png, 286, "TextBoxB.png"
static const unsigned char resource_EditDialogComponent_textBoxB_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,
19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,46,
46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,48,
70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,45,73,68,65,84,120,156,98,248,79,34,96,24,213,
48,56,52,188,124,249,242,63,41,152,97,255,254,253,255,73,193,12,181,181,181,255,73,193,0,0,0,0,255,255,3,0,51,127,31,127,148,136,189,231,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::textBoxB_png = (const char*) resource_EditDialogComponent_textBoxB_png;
const int EditDialogComponent::textBoxB_pngSize = 286;

// JUCER_RESOURCE: textBoxInner_png, 265, "TextBoxInner.png"
static const unsigned char resource_EditDialogComponent_textBoxInner_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,10,0,0,0,10,8,6,0,0,0,141,50,207,189,0,0,0,9,112,72,89,115,0,0,11,19,
0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,
37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,
48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,24,73,68,65,84,120,156,98,248,79,36,96,24,
85,72,95,133,0,0,0,0,255,255,3,0,35,251,142,128,175,59,63,124,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::textBoxInner_png = (const char*) resource_EditDialogComponent_textBoxInner_png;
const int EditDialogComponent::textBoxInner_pngSize = 265;

// JUCER_RESOURCE: eigenD_png, 3575, "eigenD@2x.png"
static const unsigned char resource_EditDialogComponent_eigenD_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,1,36,0,0,0,120,8,2,0,0,0,212,160,24,131,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,
99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,66,67,69,50,54,65,54,53,70,57,68,57,49,49,69,49,66,67,69,50,70,54,67,67,50,52,66,53,56,70,57,53,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,66,67,69,50,54,65,54,54,70,57,68,57,49,49,69,49,66,67,69,50,70,54,67,67,50,52,66,53,56,70,57,53,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,66,67,69,50,54,65,54,51,70,57,68,57,49,49,69,49,66,67,69,50,70,54,67,67,50,52,66,53,56,70,57,
53,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,66,67,69,50,54,65,54,52,70,57,68,57,49,49,69,49,66,67,69,50,70,54,67,67,50,52,66,53,56,70,57,53,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,220,169,174,226,0,0,10,107,73,68,65,84,120,218,236,157,187,114,211,74,28,198,61,158,244,36,188,0,144,60,0,97,72,79,152,137,107,66,65,90,160,57,41,153,84,184,35,116,184,50,109,
104,2,45,41,18,106,123,198,161,79,38,225,1,8,225,5,18,120,130,156,239,104,15,123,246,172,100,105,47,210,106,37,125,191,194,163,56,90,105,45,237,167,255,101,47,234,245,8,105,35,171,171,171,177,85,169,207,
187,66,90,201,221,187,119,41,54,66,104,217,8,105,23,235,235,235,20,27,33,93,132,98,35,237,228,214,173,91,20,27,33,118,44,46,46,50,102,35,36,234,208,43,54,189,81,108,36,118,156,53,227,102,18,41,54,210,
93,30,61,122,212,142,31,66,177,17,58,159,132,144,132,155,155,27,183,82,135,135,135,180,108,132,132,8,219,24,179,17,210,120,217,80,108,132,54,205,171,20,197,70,104,211,172,135,240,187,149,162,216,8,113,
148,13,197,70,72,131,5,67,177,145,150,139,205,118,84,49,99,54,66,28,177,21,143,204,94,70,149,198,164,216,72,212,120,206,148,137,202,196,81,108,36,52,47,94,188,104,186,67,72,177,145,198,132,97,182,3,23,
109,247,191,115,231,14,197,70,72,239,254,253,251,85,135,82,113,230,48,41,54,82,131,101,51,119,14,61,221,200,168,198,254,83,108,36,52,208,15,140,155,225,206,210,6,182,32,120,163,216,72,104,165,185,185,
121,86,158,39,221,72,66,254,149,129,161,153,114,214,12,197,70,200,127,50,51,209,155,170,25,55,253,68,181,164,2,197,70,130,34,163,53,219,132,100,11,6,73,82,108,36,40,82,99,38,121,66,183,30,130,104,103,
154,82,108,36,40,82,99,38,253,206,170,171,105,62,110,43,218,188,37,197,70,194,225,19,131,181,96,202,54,197,70,234,17,91,152,238,102,142,250,39,29,69,179,51,133,74,48,239,251,110,4,20,27,9,135,22,167,21,
250,120,170,26,205,45,97,180,203,179,82,108,164,54,203,22,38,160,138,167,207,128,98,35,245,196,108,38,110,100,41,58,161,216,8,197,86,60,188,67,219,191,233,253,218,20,27,169,199,135,236,85,54,136,36,218,
180,10,197,70,2,145,150,86,69,49,155,118,162,120,178,255,20,27,9,68,102,146,48,199,88,165,247,111,250,251,126,23,74,63,226,195,135,15,151,150,150,196,246,245,245,245,233,233,41,219,25,201,241,12,47,47,
47,205,101,115,116,116,212,220,31,91,190,216,222,189,123,183,177,177,33,182,167,211,233,96,48,96,147,2,184,38,120,12,165,191,191,184,184,192,35,233,34,161,221,87,32,216,108,151,104,243,40,11,148,65,24,
160,52,60,134,242,247,129,23,128,199,147,248,132,2,91,118,5,204,45,152,167,102,162,21,27,99,182,184,4,249,250,245,235,207,159,63,95,93,93,237,237,237,73,7,161,29,124,250,244,41,45,191,227,227,99,115,205,
184,217,198,120,230,143,150,111,217,212,32,141,1,219,188,75,36,13,215,60,69,253,149,0,19,55,26,141,240,217,130,95,13,93,189,124,249,114,60,30,139,60,7,148,246,244,233,83,54,6,82,62,48,89,55,127,208,4,
182,180,180,132,111,164,77,187,73,1,255,83,230,156,90,192,250,250,186,73,134,112,119,119,87,187,14,179,217,172,176,20,148,236,80,138,116,69,108,26,207,158,61,155,76,38,90,139,57,57,57,105,147,222,76,128,
72,180,139,240,227,199,15,19,37,71,43,54,198,108,209,113,112,112,48,72,80,157,112,132,115,80,96,215,244,86,74,230,163,49,253,108,178,211,172,137,61,102,21,85,30,199,20,73,252,74,243,245,136,211,214,214,
214,224,64,194,36,170,122,131,8,219,151,168,172,148,104,151,36,249,207,147,65,252,144,14,30,240,37,162,246,252,178,216,97,242,135,204,158,165,124,121,160,109,237,237,237,137,226,233,10,152,36,232,124,
42,223,75,250,9,101,253,181,186,225,95,223,191,127,87,143,137,63,165,24,202,114,35,211,215,83,139,223,58,34,146,179,179,179,244,77,116,112,35,77,74,213,102,13,50,91,185,74,190,63,227,208,170,112,52,148,
210,218,113,38,249,7,244,175,60,80,143,32,190,193,73,243,15,107,226,224,57,139,77,43,11,150,151,151,187,32,182,204,75,93,232,19,166,211,42,145,138,13,141,85,203,134,33,46,207,52,50,57,241,186,109,171,
194,254,153,41,56,91,177,149,82,121,77,108,133,50,51,55,56,62,98,211,106,5,203,223,5,177,193,91,78,95,231,194,176,45,83,108,209,121,146,106,99,197,6,26,135,214,34,225,158,169,198,7,94,153,103,171,194,
241,209,238,213,139,130,243,10,103,15,5,229,217,77,14,88,86,229,181,102,173,153,47,84,76,90,149,180,14,243,13,142,167,216,80,68,61,87,23,50,37,251,251,251,14,217,200,76,177,197,181,80,2,110,158,108,139,
104,172,243,98,45,77,30,153,141,198,176,85,161,105,170,173,31,219,104,202,153,109,168,240,128,37,86,62,83,108,176,36,243,132,164,238,156,31,188,121,138,13,168,151,11,207,142,46,36,54,160,46,249,147,97,
232,220,122,231,226,17,91,95,70,225,178,61,109,109,109,205,203,221,225,7,227,191,106,3,114,214,54,108,139,60,227,135,15,31,214,214,214,240,233,150,103,171,174,242,7,7,7,43,43,43,219,219,219,243,178,142,
216,65,181,174,149,222,42,245,92,93,8,219,126,253,250,245,224,193,131,157,157,157,183,111,223,226,243,222,189,123,231,231,231,133,165,76,214,126,133,47,186,155,96,245,194,225,210,204,154,244,193,76,226,
1,236,147,227,59,153,60,194,17,225,152,71,32,249,7,44,183,242,153,9,146,28,112,4,213,207,172,212,178,169,105,201,28,55,184,227,164,187,194,53,203,6,129,169,255,58,59,59,11,22,209,245,69,60,35,253,183,
209,104,100,245,136,117,104,55,104,160,210,170,192,98,12,135,67,159,31,16,184,242,26,33,231,197,168,231,234,120,239,182,45,50,173,2,93,141,199,99,245,95,112,77,131,217,183,5,213,255,193,237,92,78,176,
178,138,14,242,144,219,206,174,99,218,121,11,83,121,210,104,32,173,180,29,123,242,228,201,251,247,239,67,139,13,45,53,223,23,202,105,235,86,137,53,85,108,158,63,32,112,229,73,228,124,249,242,37,157,14,
201,153,200,19,58,65,226,211,230,28,140,131,20,155,152,164,92,150,216,194,84,190,70,212,95,202,185,75,243,248,248,241,163,150,71,217,217,217,177,157,183,90,161,101,147,76,19,172,202,251,168,165,220,128,
39,112,229,235,21,27,135,71,206,67,228,48,17,134,137,56,237,56,161,148,35,195,255,196,97,241,137,83,64,210,248,244,21,155,73,142,33,78,26,93,121,19,35,172,198,186,237,152,78,90,169,125,203,252,62,179,
243,224,235,215,175,38,193,222,108,54,147,241,222,171,87,175,30,63,126,108,107,48,251,170,133,9,19,195,200,167,114,41,157,69,129,43,95,23,234,16,234,211,4,42,202,205,238,185,21,148,19,204,5,48,155,90,
86,211,84,108,242,206,133,89,244,66,158,206,54,121,152,127,180,150,173,216,161,57,144,234,216,75,255,172,18,177,37,157,116,217,220,220,244,18,27,124,21,147,73,40,254,254,158,220,246,31,118,20,184,242,
181,56,144,106,23,54,174,30,197,214,80,250,189,255,119,118,57,143,192,50,71,237,86,198,233,60,221,191,192,149,15,111,211,38,147,137,180,255,248,165,219,219,219,108,181,229,122,146,110,185,74,7,143,180,
47,110,161,76,45,224,190,86,61,131,3,81,150,60,29,30,219,56,157,79,10,62,112,229,67,34,22,35,81,31,70,57,163,52,137,33,233,28,137,155,216,76,6,106,102,136,77,216,7,121,23,225,140,85,221,100,33,15,53,177,
113,114,114,226,19,113,5,174,124,0,196,252,29,120,143,234,99,8,74,83,157,2,18,146,82,250,15,250,210,62,108,109,109,73,127,12,77,214,83,0,133,230,72,61,157,24,252,1,220,66,184,192,149,175,212,105,132,39,
44,230,188,170,245,199,163,68,204,138,96,163,111,52,11,106,166,97,48,24,200,25,254,34,90,192,109,198,211,84,172,134,173,166,155,197,162,55,62,189,61,226,116,176,66,210,77,218,72,16,39,18,103,20,246,202,
36,99,25,184,242,254,162,210,182,197,155,0,50,221,233,81,2,123,177,99,195,196,249,20,67,49,225,112,102,7,120,194,200,152,44,4,112,117,117,149,121,130,186,150,69,40,165,242,61,203,41,54,61,101,169,12,243,
41,54,134,228,76,90,37,206,164,103,151,154,148,58,60,60,212,74,225,56,249,50,83,23,44,18,19,11,250,233,236,133,88,180,176,240,193,95,86,191,42,30,219,43,43,43,195,225,208,255,128,225,43,95,5,168,27,174,
198,237,219,183,153,14,137,135,111,223,190,89,237,191,191,191,175,206,43,199,159,155,155,155,217,235,70,138,113,134,120,172,10,247,6,27,154,147,131,70,80,162,27,38,50,138,34,169,40,95,173,36,141,152,109,
223,119,224,202,251,63,32,196,128,108,104,76,84,140,30,99,211,89,77,208,190,124,254,252,121,3,170,174,58,96,156,20,67,106,113,35,211,165,114,214,53,201,92,187,114,54,155,53,96,249,113,78,45,33,213,37,
54,12,187,203,172,122,213,50,211,33,248,178,1,98,147,254,36,149,70,74,23,155,225,64,16,171,241,34,153,202,68,212,23,187,216,212,21,32,217,165,75,226,33,223,214,101,14,10,139,90,108,162,147,87,108,95,95,
95,83,108,36,18,123,88,104,235,50,7,133,197,43,54,241,130,14,117,229,44,230,193,137,63,154,72,12,253,67,219,241,147,233,253,29,198,82,86,142,152,41,163,189,100,163,35,171,219,147,48,152,247,77,207,43,
85,216,61,147,153,243,92,168,81,84,34,205,136,13,209,141,38,251,196,180,61,135,195,97,139,215,59,32,237,11,216,122,201,192,229,55,111,222,168,127,214,41,54,147,149,231,166,211,105,41,35,75,8,241,7,130,
49,127,103,64,166,179,90,155,216,114,36,36,6,34,171,19,103,8,105,180,233,19,163,189,22,106,172,144,28,51,37,223,151,43,150,178,225,120,37,18,204,70,185,77,84,51,89,144,11,122,147,131,182,132,246,234,20,
219,96,48,224,141,39,13,50,86,86,175,158,82,61,73,145,156,236,243,34,18,98,194,239,223,191,157,173,159,176,108,20,27,33,214,152,116,187,201,125,228,6,197,70,186,232,16,102,110,219,186,133,20,27,33,22,
14,161,249,8,99,219,33,32,50,245,34,253,73,138,141,16,107,172,6,121,209,178,17,82,142,35,74,177,17,98,17,77,89,185,133,230,8,7,82,22,164,216,8,197,86,161,183,169,158,130,98,35,196,14,115,19,7,111,147,
98,35,164,194,80,77,221,95,45,66,177,145,206,33,115,137,182,111,162,113,216,95,93,112,146,98,35,221,53,80,182,75,175,58,68,122,234,11,135,41,54,66,236,100,246,243,231,79,183,226,20,27,33,166,56,203,140,
98,35,196,43,228,163,216,8,41,70,164,239,221,102,142,58,175,147,69,177,17,98,39,81,186,145,132,196,14,197,70,72,80,251,70,72,183,24,143,199,134,47,139,82,89,92,92,116,40,69,203,70,58,141,237,130,34,2,
231,60,36,197,70,136,53,62,75,246,83,108,132,80,108,132,84,198,229,229,165,155,108,124,6,145,80,108,164,163,98,115,11,192,124,230,155,82,108,132,88,112,116,116,68,177,17,18,2,159,132,36,197,70,58,170,
153,170,23,32,161,216,8,249,135,243,243,115,207,249,50,20,27,33,241,66,177,17,66,177,17,82,37,225,99,54,66,72,32,254,22,96,0,67,188,50,239,194,216,58,162,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* EditDialogComponent::eigenD_png = (const char*) resource_EditDialogComponent_eigenD_png;
const int EditDialogComponent::eigenD_pngSize = 3575;
