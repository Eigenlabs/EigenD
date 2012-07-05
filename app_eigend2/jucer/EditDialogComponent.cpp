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

// JUCER_RESOURCE: eigenD_png, 2157, "eigenD.png"
static const unsigned char resource_EditDialogComponent_eigenD_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,146,0,0,0,60,8,6,0,0,0,141,109,227,16,0,0,0,4,115,66,73,84,8,8,8,8,124,8,100,
136,0,0,0,9,112,72,89,115,0,0,8,179,0,0,8,179,1,132,47,149,219,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,101,0,119,119,119,46,105,110,107,115,99,97,112,101,46,111,114,103,155,238,60,26,0,0,7,234,
73,68,65,84,120,156,237,156,123,140,85,197,29,199,63,187,92,132,214,133,173,44,212,168,224,11,88,132,224,3,181,42,138,154,216,136,241,17,109,131,36,106,53,77,109,107,19,219,24,181,177,73,99,26,95,255,
152,104,108,66,163,241,29,173,239,23,86,141,72,208,4,20,136,34,169,15,52,154,178,80,86,170,180,10,168,187,172,20,5,247,231,31,191,57,185,227,112,102,206,89,246,158,187,247,238,153,79,114,114,38,51,191,
51,51,119,207,247,252,230,55,115,230,108,139,136,16,25,246,140,1,250,128,194,110,118,107,81,21,71,26,138,31,161,98,42,140,40,164,114,208,14,140,45,178,129,40,164,230,103,116,14,155,177,168,152,10,35,10,
169,249,153,156,195,166,157,40,164,72,6,147,201,190,143,209,35,69,50,153,128,6,211,33,162,71,138,100,210,1,140,203,176,25,75,12,182,35,25,116,152,35,68,225,30,169,82,100,229,145,186,48,142,124,30,169,
165,200,78,68,33,53,62,35,128,111,3,229,121,61,82,161,163,79,28,218,26,159,163,8,175,21,197,24,41,146,139,233,192,148,64,121,67,196,72,81,72,141,79,167,57,124,228,245,72,49,216,46,57,157,192,246,64,249,
56,242,121,164,17,53,235,81,10,81,72,141,207,52,224,255,158,178,54,96,47,242,121,164,40,164,146,51,21,191,71,234,112,206,62,162,71,42,57,7,0,123,227,143,145,18,1,133,60,210,72,116,214,87,168,144,98,176,
221,216,36,2,26,15,236,147,82,158,199,35,37,65,246,72,224,7,53,234,215,110,68,33,53,54,211,60,233,132,68,64,161,24,200,94,63,42,108,230,22,133,212,216,116,122,210,9,201,144,214,130,127,120,179,197,83,
216,162,228,158,10,233,10,224,31,192,25,53,236,75,61,24,3,156,0,156,2,28,129,14,25,141,76,150,144,236,33,205,39,164,186,120,164,61,13,182,143,6,206,3,22,215,176,47,245,96,22,240,170,147,247,13,240,95,
224,117,224,206,148,242,161,100,131,39,157,208,225,73,219,180,123,210,53,101,79,133,180,0,245,72,239,212,176,47,245,32,249,28,103,57,250,27,198,161,79,250,169,192,5,230,248,0,184,17,120,98,40,58,232,112,
19,250,194,118,11,240,72,74,121,211,123,164,183,204,209,108,244,155,243,70,224,105,167,236,72,224,183,192,165,192,99,232,98,223,125,245,235,90,42,159,1,87,6,202,243,8,169,46,49,146,43,164,179,129,147,
208,69,176,117,192,18,96,105,202,117,71,162,195,196,50,160,59,80,255,24,83,223,4,244,135,182,3,255,6,30,78,177,29,9,204,3,142,3,38,162,158,193,231,245,126,134,110,47,253,59,42,142,153,192,89,230,188,17,
184,7,248,40,229,186,126,231,108,243,46,240,7,224,89,224,5,224,110,96,167,105,163,81,201,51,180,213,197,35,33,34,136,72,135,136,60,39,202,87,34,178,90,170,220,105,108,236,227,47,166,108,94,74,25,34,50,
75,68,94,16,145,29,178,59,139,82,236,59,173,54,183,136,200,123,38,189,83,68,174,73,177,127,223,148,207,49,237,184,108,19,145,153,41,215,157,96,202,31,240,244,59,57,230,154,190,239,18,145,9,25,182,67,121,
124,104,253,230,235,60,54,55,231,176,25,244,145,204,218,30,6,206,1,126,15,236,7,252,4,93,183,120,14,248,29,112,161,163,191,228,137,222,153,162,205,11,128,21,192,108,224,86,224,68,96,95,224,16,207,53,163,
129,151,128,67,129,249,192,254,192,225,168,103,122,11,184,25,245,126,54,73,29,203,129,81,192,249,168,215,155,14,220,143,14,75,55,248,30,158,28,44,1,22,161,107,51,115,6,81,79,209,44,246,164,109,234,230,
145,230,25,181,62,148,162,180,67,68,228,91,17,89,231,228,255,217,92,115,166,147,127,153,201,255,64,68,14,117,202,246,53,101,207,56,249,137,119,251,83,74,251,115,77,217,243,78,254,155,38,127,182,167,207,
34,34,221,41,101,121,61,18,34,114,149,177,189,173,168,167,184,6,71,69,68,126,46,58,2,248,108,30,145,42,247,89,249,199,138,200,37,34,242,195,90,244,165,2,92,108,52,181,16,221,141,231,178,26,56,30,141,119,
182,37,250,51,231,93,142,237,101,38,239,84,96,179,83,246,141,231,154,139,128,175,80,47,230,182,191,25,248,20,141,201,108,250,77,61,175,167,244,119,3,58,157,31,236,183,238,171,204,121,246,32,235,41,146,
93,104,76,23,162,205,74,239,109,206,199,162,247,21,52,182,116,71,156,1,83,1,102,152,244,194,12,219,41,192,219,78,158,189,151,120,18,112,12,42,8,87,68,80,21,159,253,31,49,246,50,245,86,128,149,129,182,
251,209,96,220,30,22,67,251,152,123,201,222,90,145,197,84,115,246,109,225,104,22,94,6,206,53,233,87,204,249,116,171,252,204,90,52,82,65,99,162,173,168,74,67,252,47,37,207,158,253,36,113,140,43,182,16,
227,77,31,86,161,177,85,136,144,112,6,106,159,231,139,138,100,213,126,221,0,219,109,52,238,0,54,161,35,194,139,38,175,207,42,239,219,237,138,42,251,160,49,226,203,192,142,80,35,21,116,218,59,7,253,227,
166,173,158,134,176,95,20,38,2,58,113,0,215,111,66,23,219,102,160,211,246,180,105,249,158,146,54,17,72,200,18,210,24,170,79,237,131,181,233,206,144,209,207,238,163,77,175,149,238,241,92,215,6,172,71,197,
180,146,140,73,71,43,240,134,73,255,98,0,157,75,110,132,45,164,255,0,31,163,175,79,210,254,177,129,239,230,189,129,222,184,179,7,208,126,158,119,132,33,81,134,174,31,143,174,157,141,7,158,34,60,228,54,
43,61,158,180,205,105,84,183,174,156,68,198,98,102,43,112,27,58,180,221,72,53,240,206,34,89,200,116,111,200,13,168,96,86,160,75,8,54,163,60,117,93,143,122,143,199,77,135,243,208,74,120,163,214,8,194,94,
199,87,54,19,93,82,56,6,120,136,26,4,161,13,74,30,143,180,205,74,11,58,33,242,210,138,206,112,126,5,124,13,60,0,60,10,204,69,189,74,27,170,68,119,67,148,239,137,190,23,248,35,186,110,180,28,117,169,191,
70,213,125,184,177,113,5,240,79,224,26,116,61,105,9,112,59,112,50,112,16,58,203,104,71,131,114,183,253,208,235,157,10,26,156,187,180,90,231,10,234,117,142,3,46,71,227,128,53,192,97,192,223,128,95,50,240,
184,172,89,200,227,145,182,90,233,94,178,254,22,214,90,192,193,34,242,148,164,115,137,179,110,112,189,201,63,199,179,174,48,85,68,254,42,34,95,164,212,229,174,35,37,199,44,17,121,205,211,254,209,142,237,
26,147,63,210,83,87,183,136,244,164,228,159,226,169,95,140,253,2,17,153,238,169,115,56,29,83,172,223,125,151,199,230,0,203,102,189,199,102,190,136,220,34,34,157,246,83,221,141,174,44,255,24,125,42,167,
1,7,26,37,174,112,244,183,204,156,187,60,250,236,2,174,2,174,54,245,29,132,238,1,186,5,248,151,231,154,183,141,205,68,171,253,253,81,79,233,94,115,23,186,146,237,27,162,22,144,190,173,244,99,116,40,31,
129,78,235,55,1,159,152,227,125,50,220,247,48,34,207,208,102,123,164,207,83,202,143,2,158,52,233,147,235,249,20,92,109,212,125,110,3,60,145,101,63,70,73,149,107,3,118,125,198,102,113,74,217,124,171,142,
207,235,181,213,118,34,186,61,67,168,206,18,35,67,199,215,84,223,52,248,60,18,84,189,82,154,71,90,107,167,139,22,82,27,186,172,240,46,250,66,245,55,232,30,155,200,208,211,227,156,211,8,9,169,139,234,91,
138,181,181,254,174,173,5,93,123,105,71,99,152,153,104,60,178,25,125,167,214,8,187,14,35,74,47,122,143,190,12,216,108,117,206,54,219,209,152,115,18,5,8,73,208,197,197,126,212,11,45,66,167,244,203,25,190,
83,233,102,101,176,30,9,116,120,155,4,116,21,241,165,109,179,125,89,82,86,6,34,164,52,143,4,58,155,254,41,117,136,145,34,141,75,178,4,48,88,143,4,208,21,133,84,94,106,229,145,54,1,125,81,72,229,37,17,
80,111,192,38,143,71,234,130,248,201,118,153,233,65,247,34,133,38,65,89,66,234,6,222,131,40,164,50,211,67,120,88,3,21,146,0,95,120,202,251,209,153,121,20,82,137,201,35,164,45,232,58,83,104,111,215,82,
136,66,42,51,121,61,146,47,208,78,216,1,81,72,101,38,143,144,122,201,249,74,43,10,169,188,228,17,146,224,223,42,244,61,162,144,202,75,30,33,65,20,82,36,131,30,194,47,108,19,214,102,155,68,33,149,153,47,
137,30,41,82,3,182,147,61,35,131,40,164,72,14,54,230,176,201,181,143,61,10,169,220,228,17,82,46,90,68,36,219,42,50,92,177,255,195,204,160,248,14,56,24,144,199,239,47,222,31,0,0,0,0,73,69,78,68,174,66,
96,130,0,0};

const char* EditDialogComponent::eigenD_png = (const char*) resource_EditDialogComponent_eigenD_png;
const int EditDialogComponent::eigenD_pngSize = 2157;
