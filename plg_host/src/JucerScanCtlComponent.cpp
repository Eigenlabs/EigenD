/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  24 Sep 2012 8:22:30pm

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

#include "JucerScanCtlComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
JucerScanCtlComponent::JucerScanCtlComponent ()
    : component (0),
      label (0),
      textButton (0),
      textButton2 (0),
      toggleButton (0),
      label2 (0),
      label3 (0),
      add_text_button (0),
      del_text_button (0),
      path_box (0),
      cachedImage_backgroundBoxInner_png (0),
      cachedImage_backgroundBoxT_png (0),
      cachedImage_backgroundBoxL_png (0),
      cachedImage_backgroundBoxR_png (0),
      cachedImage_backgroundBoxB_png (0),
      cachedImage_backgroundBoxBr_png (0),
      cachedImage_backgroundBoxBl_png (0),
      cachedImage_eigenD_png (0),
      cachedImage_backgroundBoxTl_png (0),
      cachedImage_backgroundBoxTr_png (0),
      cachedImage_innerBoxBr_png (0),
      cachedImage_innerBoxL_png (0),
      cachedImage_innerBoxR_png (0),
      cachedImage_innerBoxInner_png (0),
      cachedImage_innerBoxTl_png (0),
      cachedImage_innerBoxT_png (0),
      cachedImage_innerBoxTr_png (0),
      cachedImage_innerBoxBl_png (0),
      cachedImage_innerBoxB_png (0)
{
    addAndMakeVisible (component = new TableListBox ("table",0));
    component->setName ("new component");

    addAndMakeVisible (label = new Label ("new label",
                                          "Scan your computer for new or updated plugins.  This may take a long time."));
    label->setFont (Font (15.0000f, Font::plain));
    label->setJustificationType (Justification::topLeft);
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (textButton = new TextButton ("new button"));
    textButton->setButtonText ("Cancel");
    textButton->addListener (this);
    textButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (textButton2 = new TextButton ("new button"));
    textButton2->setButtonText ("Scan");
    textButton2->addListener (this);
    textButton2->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (toggleButton = new ToggleButton ("new toggle button"));
    toggleButton->setButtonText ("Perform Full Scan");
    toggleButton->addListener (this);

    addAndMakeVisible (label2 = new Label ("new label",
                                           "These plugins failed last time.  Select to rescan."));
    label2->setFont (Font (15.0000f, Font::plain));
    label2->setJustificationType (Justification::centredLeft);
    label2->setEditable (false, false, false);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label3 = new Label ("new label",
                                           "VST Search Path:"));
    label3->setFont (Font (15.0000f, Font::plain));
    label3->setJustificationType (Justification::centredLeft);
    label3->setEditable (false, false, false);
    label3->setColour (TextEditor::textColourId, Colours::black);
    label3->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (add_text_button = new TextButton ("new button"));
    add_text_button->setButtonText ("Add");
    add_text_button->addListener (this);
    add_text_button->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (del_text_button = new TextButton ("new button"));
    del_text_button->setButtonText ("Remove");
    del_text_button->addListener (this);
    del_text_button->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (path_box = new ListBox());
    path_box->setName ("new component");

    cachedImage_backgroundBoxInner_png = ImageCache::getFromMemory (backgroundBoxInner_png, backgroundBoxInner_pngSize);
    cachedImage_backgroundBoxT_png = ImageCache::getFromMemory (backgroundBoxT_png, backgroundBoxT_pngSize);
    cachedImage_backgroundBoxL_png = ImageCache::getFromMemory (backgroundBoxL_png, backgroundBoxL_pngSize);
    cachedImage_backgroundBoxR_png = ImageCache::getFromMemory (backgroundBoxR_png, backgroundBoxR_pngSize);
    cachedImage_backgroundBoxB_png = ImageCache::getFromMemory (backgroundBoxB_png, backgroundBoxB_pngSize);
    cachedImage_backgroundBoxBr_png = ImageCache::getFromMemory (backgroundBoxBr_png, backgroundBoxBr_pngSize);
    cachedImage_backgroundBoxBl_png = ImageCache::getFromMemory (backgroundBoxBl_png, backgroundBoxBl_pngSize);
    cachedImage_eigenD_png = ImageCache::getFromMemory (eigenD_png, eigenD_pngSize);
    cachedImage_backgroundBoxTl_png = ImageCache::getFromMemory (backgroundBoxTl_png, backgroundBoxTl_pngSize);
    cachedImage_backgroundBoxTr_png = ImageCache::getFromMemory (backgroundBoxTr_png, backgroundBoxTr_pngSize);
    cachedImage_innerBoxBr_png = ImageCache::getFromMemory (innerBoxBr_png, innerBoxBr_pngSize);
    cachedImage_innerBoxL_png = ImageCache::getFromMemory (innerBoxL_png, innerBoxL_pngSize);
    cachedImage_innerBoxR_png = ImageCache::getFromMemory (innerBoxR_png, innerBoxR_pngSize);
    cachedImage_innerBoxInner_png = ImageCache::getFromMemory (innerBoxInner_png, innerBoxInner_pngSize);
    cachedImage_innerBoxTl_png = ImageCache::getFromMemory (innerBoxTl_png, innerBoxTl_pngSize);
    cachedImage_innerBoxT_png = ImageCache::getFromMemory (innerBoxT_png, innerBoxT_pngSize);
    cachedImage_innerBoxTr_png = ImageCache::getFromMemory (innerBoxTr_png, innerBoxTr_pngSize);
    cachedImage_innerBoxBl_png = ImageCache::getFromMemory (innerBoxBl_png, innerBoxBl_pngSize);
    cachedImage_innerBoxB_png = ImageCache::getFromMemory (innerBoxB_png, innerBoxB_pngSize);

    //[UserPreSize]
    //[/UserPreSize]

    setSize (640, 700);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

JucerScanCtlComponent::~JucerScanCtlComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (component);
    deleteAndZero (label);
    deleteAndZero (textButton);
    deleteAndZero (textButton2);
    deleteAndZero (toggleButton);
    deleteAndZero (label2);
    deleteAndZero (label3);
    deleteAndZero (add_text_button);
    deleteAndZero (del_text_button);
    deleteAndZero (path_box);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void JucerScanCtlComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::black);

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxInner_png,
                 26, 97, getWidth() - 49, getHeight() - 128,
                 0, 0, cachedImage_backgroundBoxInner_png.getWidth(), cachedImage_backgroundBoxInner_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxT_png,
                 42, 84, getWidth() - 85, 15,
                 0, 0, cachedImage_backgroundBoxT_png.getWidth(), cachedImage_backgroundBoxT_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxL_png,
                 12, 97, 15, getHeight() - 128,
                 0, 0, cachedImage_backgroundBoxL_png.getWidth(), cachedImage_backgroundBoxL_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxR_png,
                 getWidth() - 28, 97, 15, getHeight() - 128,
                 0, 0, cachedImage_backgroundBoxR_png.getWidth(), cachedImage_backgroundBoxR_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxB_png,
                 42, getHeight() - 31, getWidth() - 85, 15,
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
    g.drawImage (cachedImage_eigenD_png,
                 getWidth() - 177, 12, 146, 60,
                 0, 0, cachedImage_eigenD_png.getWidth(), cachedImage_eigenD_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxTl_png,
                 12, 84, 30, 15,
                 0, 0, cachedImage_backgroundBoxTl_png.getWidth(), cachedImage_backgroundBoxTl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxTr_png,
                 getWidth() - 43, 84, 30, 15,
                 0, 0, cachedImage_backgroundBoxTr_png.getWidth(), cachedImage_backgroundBoxTr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxBr_png,
                 getWidth() - 175, 265, 15, 15,
                 0, 0, cachedImage_innerBoxBr_png.getWidth(), cachedImage_innerBoxBr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxL_png,
                 56, 183, 15, 82,
                 0, 0, cachedImage_innerBoxL_png.getWidth(), cachedImage_innerBoxL_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxR_png,
                 getWidth() - 175, 183, 15, 82,
                 0, 0, cachedImage_innerBoxR_png.getWidth(), cachedImage_innerBoxR_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxInner_png,
                 71, 183, getWidth() - 246, 82,
                 0, 0, cachedImage_innerBoxInner_png.getWidth(), cachedImage_innerBoxInner_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxTl_png,
                 56, 168, 15, 15,
                 0, 0, cachedImage_innerBoxTl_png.getWidth(), cachedImage_innerBoxTl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxT_png,
                 71, 168, getWidth() - 246, 15,
                 0, 0, cachedImage_innerBoxT_png.getWidth(), cachedImage_innerBoxT_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxTr_png,
                 getWidth() - 175, 168, 15, 15,
                 0, 0, cachedImage_innerBoxTr_png.getWidth(), cachedImage_innerBoxTr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxBl_png,
                 56, 265, 15, 15,
                 0, 0, cachedImage_innerBoxBl_png.getWidth(), cachedImage_innerBoxBl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxB_png,
                 71, 265, getWidth() - 246, 15,
                 0, 0, cachedImage_innerBoxB_png.getWidth(), cachedImage_innerBoxB_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxInner_png,
                 71, 367, getWidth() - 134, getHeight() - 455,
                 0, 0, cachedImage_innerBoxInner_png.getWidth(), cachedImage_innerBoxInner_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxTr_png,
                 getWidth() - 63, 352, 15, 15,
                 0, 0, cachedImage_innerBoxTr_png.getWidth(), cachedImage_innerBoxTr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxR_png,
                 getWidth() - 63, 367, 15, getHeight() - 455,
                 0, 0, cachedImage_innerBoxR_png.getWidth(), cachedImage_innerBoxR_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxBr_png,
                 getWidth() - 63, getHeight() - 88, 15, 15,
                 0, 0, cachedImage_innerBoxBr_png.getWidth(), cachedImage_innerBoxBr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxB_png,
                 71, getHeight() - 88, getWidth() - 134, 15,
                 0, 0, cachedImage_innerBoxB_png.getWidth(), cachedImage_innerBoxB_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxBl_png,
                 56, getHeight() - 88, 15, 15,
                 0, 0, cachedImage_innerBoxBl_png.getWidth(), cachedImage_innerBoxBl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxL_png,
                 56, 367, 15, getHeight() - 455,
                 0, 0, cachedImage_innerBoxL_png.getWidth(), cachedImage_innerBoxL_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxTl_png,
                 56, 352, 15, 15,
                 0, 0, cachedImage_innerBoxTl_png.getWidth(), cachedImage_innerBoxTl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxT_png,
                 71, 352, getWidth() - 134, 15,
                 0, 0, cachedImage_innerBoxT_png.getWidth(), cachedImage_innerBoxT_png.getHeight());

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void JucerScanCtlComponent::resized()
{
    component->setBounds (70, 366, getWidth() - 130, getHeight() - 454);
    label->setBounds (40, 96, 536, 24);
    textButton->setBounds (getWidth() - 232, getHeight() - 60, 80, 24);
    textButton2->setBounds (getWidth() - 136, getHeight() - 60, 80, 24);
    toggleButton->setBounds (56, getHeight() - 60, 136, 24);
    label2->setBounds (48, 315, 376, 24);
    label3->setBounds (48, 135, 376, 24);
    add_text_button->setBounds (getWidth() - 128, 168, 71, 24);
    del_text_button->setBounds (getWidth() - 128, 208, 71, 24);
    path_box->setBounds (70, 182, getWidth() - 242, 84);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void JucerScanCtlComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == textButton)
    {
        //[UserButtonCode_textButton] -- add your button handler code here..
        //[/UserButtonCode_textButton]
    }
    else if (buttonThatWasClicked == textButton2)
    {
        //[UserButtonCode_textButton2] -- add your button handler code here..
        //[/UserButtonCode_textButton2]
    }
    else if (buttonThatWasClicked == toggleButton)
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == add_text_button)
    {
        //[UserButtonCode_add_text_button] -- add your button handler code here..
        //[/UserButtonCode_add_text_button]
    }
    else if (buttonThatWasClicked == del_text_button)
    {
        //[UserButtonCode_del_text_button] -- add your button handler code here..
        //[/UserButtonCode_del_text_button]
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

<JUCER_COMPONENT documentType="Component" className="JucerScanCtlComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="0" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="640" initialHeight="700">
  <BACKGROUND backgroundColour="ff000000">
    <IMAGE pos="26 97 49M 128M" resource="backgroundBoxInner_png" opacity="1"
           mode="0"/>
    <IMAGE pos="42 84 85M 15" resource="backgroundBoxT_png" opacity="1"
           mode="0"/>
    <IMAGE pos="12 97 15 128M" resource="backgroundBoxL_png" opacity="1"
           mode="0"/>
    <IMAGE pos="28R 97 15 128M" resource="backgroundBoxR_png" opacity="1"
           mode="0"/>
    <IMAGE pos="42 31R 85M 15" resource="backgroundBoxB_png" opacity="1"
           mode="0"/>
    <IMAGE pos="43R 31R 30 15" resource="backgroundBoxBr_png" opacity="1"
           mode="0"/>
    <IMAGE pos="12 31R 30 15" resource="backgroundBoxBl_png" opacity="1"
           mode="0"/>
    <IMAGE pos="177R 12 146 60" resource="eigenD_png" opacity="1" mode="0"/>
    <IMAGE pos="12 84 30 15" resource="backgroundBoxTl_png" opacity="1"
           mode="0"/>
    <IMAGE pos="43R 84 30 15" resource="backgroundBoxTr_png" opacity="1"
           mode="0"/>
    <IMAGE pos="175R 265 15 15" resource="innerBoxBr_png" opacity="1" mode="0"/>
    <IMAGE pos="56 183 15 82" resource="innerBoxL_png" opacity="1" mode="0"/>
    <IMAGE pos="175R 183 15 82" resource="innerBoxR_png" opacity="1" mode="0"/>
    <IMAGE pos="71 183 246M 82" resource="innerBoxInner_png" opacity="1"
           mode="0"/>
    <IMAGE pos="56 168 15 15" resource="innerBoxTl_png" opacity="1" mode="0"/>
    <IMAGE pos="71 168 246M 15" resource="innerBoxT_png" opacity="1" mode="0"/>
    <IMAGE pos="175R 168 15 15" resource="innerBoxTr_png" opacity="1" mode="0"/>
    <IMAGE pos="56 265 15 15" resource="innerBoxBl_png" opacity="1" mode="0"/>
    <IMAGE pos="71 265 246M 15" resource="innerBoxB_png" opacity="1" mode="0"/>
    <IMAGE pos="71 367 134M 455M" resource="innerBoxInner_png" opacity="1"
           mode="0"/>
    <IMAGE pos="63R 352 15 15" resource="innerBoxTr_png" opacity="1" mode="0"/>
    <IMAGE pos="63R 367 15 455M" resource="innerBoxR_png" opacity="1" mode="0"/>
    <IMAGE pos="63R 88R 15 15" resource="innerBoxBr_png" opacity="1" mode="0"/>
    <IMAGE pos="71 88R 134M 15" resource="innerBoxB_png" opacity="1" mode="0"/>
    <IMAGE pos="56 88R 15 15" resource="innerBoxBl_png" opacity="1" mode="0"/>
    <IMAGE pos="56 367 15 455M" resource="innerBoxL_png" opacity="1" mode="0"/>
    <IMAGE pos="56 352 15 15" resource="innerBoxTl_png" opacity="1" mode="0"/>
    <IMAGE pos="71 352 134M 15" resource="innerBoxT_png" opacity="1" mode="0"/>
  </BACKGROUND>
  <GENERICCOMPONENT name="new component" id="1a73d1f864256d21" memberName="component"
                    virtualName="" explicitFocusOrder="0" pos="70 366 130M 454M"
                    class="TableListBox" params="&quot;table&quot;,0"/>
  <LABEL name="new label" id="92ab364da97e6c42" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="40 96 536 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Scan your computer for new or updated plugins.  This may take a long time."
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="9"/>
  <TEXTBUTTON name="new button" id="73831e733dd22993" memberName="textButton"
              virtualName="" explicitFocusOrder="0" pos="232R 60R 80 24" bgColOff="ffaeaeae"
              buttonText="Cancel" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="new button" id="24c0060f8840b8e3" memberName="textButton2"
              virtualName="" explicitFocusOrder="0" pos="136R 60R 80 24" bgColOff="ffaeaeae"
              buttonText="Scan" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TOGGLEBUTTON name="new toggle button" id="db8d82320283c8d3" memberName="toggleButton"
                virtualName="" explicitFocusOrder="0" pos="56 60R 136 24" buttonText="Perform Full Scan"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <LABEL name="new label" id="95b39ebb511eab10" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="48 315 376 24" edTextCol="ff000000"
         edBkgCol="0" labelText="These plugins failed last time.  Select to rescan."
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="44ab2a1698ce94d7" memberName="label3" virtualName=""
         explicitFocusOrder="0" pos="48 135 376 24" edTextCol="ff000000"
         edBkgCol="0" labelText="VST Search Path:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="new button" id="6da2bc573994fec2" memberName="add_text_button"
              virtualName="" explicitFocusOrder="0" pos="128R 168 71 24" bgColOff="ffaeaeae"
              buttonText="Add" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="new button" id="f6862fed6446f891" memberName="del_text_button"
              virtualName="" explicitFocusOrder="0" pos="128R 208 71 24" bgColOff="ffaeaeae"
              buttonText="Remove" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <GENERICCOMPONENT name="new component" id="e1c06dd9bd6005e1" memberName="path_box"
                    virtualName="" explicitFocusOrder="0" pos="70 182 242M 84" class="ListBox"
                    params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: backgroundBoxB_png, 985, "../../app_eigend2/jucer/BackgroundBoxB.png"
static const unsigned char resource_JucerScanCtlComponent_backgroundBoxB_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,2,0,0,0,180,82,57,245,0,0,0,25,116,69,88,116,83,111,
102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,66,70,69,55,70,67,65,49,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,66,70,69,55,70,67,65,50,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,66,70,69,55,70,67,57,70,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,66,70,69,55,70,67,65,48,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,28,29,173,229,0,0,0,77,73,68,65,84,120,218,236,210,193,9,192,48,12,67,209,164,116,255,93,67,33,150,13,134,76,241,15,161,210,0,15,33,52,191,181,6,147,103,96,49,109,218,180,105,
211,191,167,223,238,166,232,170,162,232,204,164,104,73,20,29,17,55,210,224,32,123,95,216,90,220,214,194,126,125,4,24,0,221,15,35,42,179,100,196,19,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::backgroundBoxB_png = (const char*) resource_JucerScanCtlComponent_backgroundBoxB_png;
const int JucerScanCtlComponent::backgroundBoxB_pngSize = 985;

// JUCER_RESOURCE: backgroundBoxBl_png, 1610, "../../app_eigend2/jucer/BackgroundBoxBL.png"
static const unsigned char resource_JucerScanCtlComponent_backgroundBoxBl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,60,0,0,0,30,8,6,0,0,0,112,152,125,79,0,0,0,25,116,69,88,116,83,111,
102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,66,70,69,55,70,67,65,53,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,66,70,69,55,70,67,65,54,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,66,70,69,55,70,67,65,51,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,66,70,69,55,70,67,65,52,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,93,87,221,30,0,0,2,190,73,68,65,84,120,218,236,152,187,142,218,64,20,134,143,141,47,92,22,4,24,20,131,33,34,37,213,110,66,1,69,168,54,229,22,233,242,26,41,34,37,111,144,38,239,
146,38,207,17,101,123,164,40,15,128,188,226,110,108,32,231,76,102,172,193,64,148,114,236,48,210,209,12,182,41,62,255,231,255,103,192,120,115,127,127,128,196,120,230,186,208,110,181,192,243,60,232,116,
58,208,165,234,118,161,92,46,67,218,135,14,255,217,184,2,95,129,175,192,25,5,62,28,14,87,133,179,11,156,80,151,212,206,138,226,250,223,218,153,205,18,104,22,160,47,2,239,247,251,88,217,67,134,60,125,22,
88,192,210,44,42,243,192,151,42,237,224,103,129,163,40,130,221,110,199,42,226,51,21,1,167,221,203,231,129,9,20,161,163,48,140,231,144,175,9,60,115,10,111,183,91,8,130,0,182,8,73,179,12,155,246,214,214,
103,243,249,83,242,226,14,225,8,122,179,217,252,153,17,90,172,35,14,29,39,120,202,192,245,96,179,249,121,238,198,106,181,130,245,122,13,203,229,146,173,151,88,4,29,18,180,164,118,218,60,157,171,213,106,
189,74,165,242,250,68,101,132,177,45,11,242,249,60,171,98,161,0,22,126,214,117,157,21,104,26,104,252,89,77,211,226,239,201,107,21,135,70,234,140,199,227,48,111,219,70,242,38,190,12,104,183,219,172,158,
119,187,224,186,46,84,171,85,40,22,139,236,37,152,166,9,134,97,196,47,65,192,170,12,205,32,125,223,255,214,114,221,183,201,155,212,198,179,217,12,74,165,18,248,55,55,96,217,54,131,17,219,149,133,192,164,
122,46,151,59,134,150,212,87,18,24,189,249,30,131,233,33,169,242,22,195,138,60,60,159,207,193,127,122,2,27,85,37,40,242,46,249,184,128,109,30,226,108,32,176,80,154,128,69,41,11,60,153,76,126,245,251,253,
47,29,207,251,148,124,96,141,65,181,88,44,24,52,1,146,154,226,52,70,62,183,48,196,72,101,2,214,132,202,188,173,85,132,214,228,132,189,187,189,253,222,108,54,95,30,197,56,2,212,235,117,192,235,224,56,14,
91,147,143,233,47,91,10,50,155,123,217,68,224,147,214,86,85,97,49,126,60,62,190,26,14,135,126,165,92,174,202,231,234,128,183,182,141,30,102,65,133,96,178,151,41,205,67,188,46,128,115,186,174,108,120,157,
36,243,116,58,189,35,118,25,154,124,26,96,235,210,190,44,160,9,134,29,60,184,159,197,139,16,10,139,173,75,121,96,242,51,237,72,114,123,147,146,33,63,110,18,52,193,137,182,21,1,102,114,31,199,109,157,134,
150,78,182,55,6,217,231,70,163,241,1,183,35,131,212,36,176,144,159,175,69,42,139,223,205,212,218,194,199,154,148,214,74,135,214,165,49,24,12,190,122,158,247,224,212,235,6,29,58,168,104,111,22,167,48,242,
176,193,15,33,71,219,147,170,39,173,127,29,163,209,232,93,195,113,62,98,82,191,232,245,122,85,2,181,184,167,77,1,43,249,88,197,241,91,128,1,0,218,47,166,181,189,145,54,198,0,0,0,0,73,69,78,68,174,66,96,
130,0,0};

const char* JucerScanCtlComponent::backgroundBoxBl_png = (const char*) resource_JucerScanCtlComponent_backgroundBoxBl_png;
const int JucerScanCtlComponent::backgroundBoxBl_pngSize = 1610;

// JUCER_RESOURCE: backgroundBoxBr_png, 1460, "../../app_eigend2/jucer/BackgroundBoxBR.png"
static const unsigned char resource_JucerScanCtlComponent_backgroundBoxBr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,60,0,0,0,30,8,6,0,0,0,112,152,125,79,0,0,0,25,116,69,88,116,83,111,
102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,66,70,69,55,70,67,57,68,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,66,70,69,55,70,67,57,69,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,54,70,55,66,50,51,49,65,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,66,70,69,55,70,67,57,67,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,215,148,131,187,0,0,2,40,73,68,65,84,120,218,236,153,63,79,2,49,24,198,175,71,143,132,111,34,14,130,124,3,195,34,163,171,9,155,236,4,195,192,42,145,196,132,47,226,78,4,25,152,
48,142,168,163,12,196,4,240,112,48,160,9,28,4,174,246,45,237,165,156,10,76,166,135,109,242,114,215,246,254,228,119,207,243,182,119,5,125,142,70,196,8,120,33,132,24,196,117,13,151,6,219,247,133,65,136,
119,156,105,252,179,162,129,53,176,6,214,192,26,88,3,107,224,191,43,104,165,130,180,194,193,125,189,212,150,214,192,44,159,119,3,152,108,48,52,66,187,163,48,251,4,220,6,122,39,128,183,132,68,92,229,199,
167,167,97,160,129,201,134,58,146,236,12,191,182,109,119,204,32,91,217,83,89,216,122,67,14,219,131,193,45,26,190,191,43,185,196,131,164,129,6,246,229,186,12,252,211,114,14,145,31,2,223,246,251,253,249,
193,225,161,133,103,179,153,186,211,8,7,21,97,154,166,17,10,133,214,190,66,18,89,85,0,135,115,233,246,238,254,190,66,129,13,60,157,78,149,135,5,80,177,5,245,48,198,12,144,72,176,196,119,46,28,135,56,116,
207,182,231,221,110,55,11,125,216,113,156,64,0,131,178,16,194,202,0,13,48,200,167,42,226,54,23,208,208,86,175,215,203,151,165,210,11,59,111,50,153,40,153,187,30,48,133,101,192,52,0,18,150,98,161,120,214,
150,115,157,231,171,172,112,181,90,109,157,231,243,5,113,140,114,192,235,212,93,240,117,103,104,135,122,56,28,246,20,103,202,250,20,110,61,60,12,79,211,233,132,124,109,60,81,212,210,114,222,10,117,173,
197,194,83,216,228,109,226,193,16,159,194,0,219,104,52,226,71,201,164,177,10,60,30,43,171,176,80,215,164,129,41,236,130,134,172,48,68,36,18,89,182,113,133,161,84,111,110,90,199,169,84,34,30,139,125,187,
182,82,10,203,115,173,80,24,160,48,216,25,242,151,91,152,61,12,209,7,202,91,214,114,174,237,245,230,181,90,173,156,205,229,10,191,221,3,59,10,229,176,127,192,98,208,52,0,8,115,59,179,255,135,248,64,6,
176,16,131,183,183,121,179,217,172,156,101,50,39,123,251,251,107,239,129,29,197,230,97,255,128,5,225,130,157,65,69,49,64,209,190,231,118,123,248,49,26,117,94,109,251,234,162,88,188,222,139,70,183,186,
254,151,0,3,0,191,226,30,124,137,123,72,168,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::backgroundBoxBr_png = (const char*) resource_JucerScanCtlComponent_backgroundBoxBr_png;
const int JucerScanCtlComponent::backgroundBoxBr_pngSize = 1460;

// JUCER_RESOURCE: backgroundBoxInner_png, 503, "../../app_eigend2/jucer/BackgroundBoxInner.png"
static const unsigned char resource_JucerScanCtlComponent_backgroundBoxInner_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,100,0,0,0,100,8,6,0,0,0,112,226,149,84,0,0,0,9,112,72,89,115,
0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,
46,46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,
51,48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,6,73,68,65,84,120,156,236,209,49,
13,0,48,12,192,176,242,103,91,13,192,6,99,57,124,248,143,148,57,187,151,142,249,29,128,33,105,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,
72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,
24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,
134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,
140,33,49,134,196,60,0,0,0,255,255,3,0,183,209,145,252,233,144,222,223,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::backgroundBoxInner_png = (const char*) resource_JucerScanCtlComponent_backgroundBoxInner_png;
const int JucerScanCtlComponent::backgroundBoxInner_pngSize = 503;

// JUCER_RESOURCE: backgroundBoxL_png, 973, "../../app_eigend2/jucer/BackgroundBoxL.png"
static const unsigned char resource_JucerScanCtlComponent_backgroundBoxL_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,2,0,0,0,180,82,57,245,0,0,0,25,116,69,88,116,83,111,
102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,48,51,54,48,55,57,56,56,70,69,56,56,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,48,51,54,48,55,57,56,57,70,69,56,56,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,48,51,54,48,55,57,56,54,70,69,56,56,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,48,51,54,48,55,57,56,55,70,69,56,56,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,83,97,14,124,0,0,0,65,73,68,65,84,120,218,236,204,171,13,0,32,12,0,209,130,175,230,211,118,26,216,127,38,144,104,18,12,185,203,233,151,230,24,114,84,106,237,173,153,153,187,
199,62,66,85,229,170,44,207,130,134,134,134,134,134,134,134,134,254,159,94,2,12,0,73,251,3,17,230,31,202,104,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::backgroundBoxL_png = (const char*) resource_JucerScanCtlComponent_backgroundBoxL_png;
const int JucerScanCtlComponent::backgroundBoxL_pngSize = 973;

// JUCER_RESOURCE: backgroundBoxR_png, 964, "../../app_eigend2/jucer/BackgroundBoxR.png"
static const unsigned char resource_JucerScanCtlComponent_backgroundBoxR_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,2,0,0,0,180,82,57,245,0,0,0,25,116,69,88,116,83,111,
102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,54,70,55,66,50,51,49,56,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,54,70,55,66,50,51,49,57,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,54,70,55,66,50,51,49,54,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,54,70,55,66,50,51,49,55,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,58,74,80,144,0,0,0,56,73,68,65,84,120,218,98,252,252,241,35,3,89,224,63,16,252,3,129,255,168,0,40,1,145,101,98,160,25,24,53,122,212,232,81,163,71,141,30,53,122,212,232,81,163,
71,141,30,53,122,248,27,13,16,96,0,54,167,31,239,91,116,214,142,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::backgroundBoxR_png = (const char*) resource_JucerScanCtlComponent_backgroundBoxR_png;
const int JucerScanCtlComponent::backgroundBoxR_pngSize = 964;

// JUCER_RESOURCE: backgroundBoxT_png, 973, "../../app_eigend2/jucer/BackgroundBoxT.png"
static const unsigned char resource_JucerScanCtlComponent_backgroundBoxT_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,2,0,0,0,180,82,57,245,0,0,0,25,116,69,88,116,83,111,
102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,54,70,55,66,50,51,49,48,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,54,70,55,66,50,51,49,49,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,49,69,69,50,49,65,54,66,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,49,69,69,50,49,65,54,67,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,3,230,191,158,0,0,0,65,73,68,65,84,120,218,236,209,209,13,0,48,8,66,65,41,238,191,165,233,24,182,75,240,65,194,27,224,98,16,119,166,52,53,73,21,253,51,164,1,136,232,83,178,186,
118,253,174,150,14,226,248,198,208,161,67,135,14,29,218,146,126,2,12,0,213,117,5,29,179,101,98,203,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::backgroundBoxT_png = (const char*) resource_JucerScanCtlComponent_backgroundBoxT_png;
const int JucerScanCtlComponent::backgroundBoxT_pngSize = 973;

// JUCER_RESOURCE: backgroundBoxTl_png, 1501, "../../app_eigend2/jucer/BackgroundBoxTL.png"
static const unsigned char resource_JucerScanCtlComponent_backgroundBoxTl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,60,0,0,0,30,8,6,0,0,0,112,152,125,79,0,0,0,25,116,69,88,116,83,111,
102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,49,69,69,50,49,65,54,57,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,49,69,69,50,49,65,54,65,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,49,69,69,50,49,65,54,55,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,49,69,69,50,49,65,54,56,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,77,247,6,118,0,0,2,81,73,68,65,84,120,218,236,151,73,106,227,64,20,134,75,82,105,178,59,141,12,182,193,168,137,9,120,209,114,172,12,167,176,27,178,239,141,114,139,64,142,144,
109,223,164,111,18,114,31,15,234,250,171,223,51,229,41,113,18,8,85,70,5,143,74,20,109,190,124,111,146,87,215,181,56,246,220,87,213,239,225,249,249,227,32,207,47,174,202,50,75,147,68,196,113,44,194,40,
18,50,8,132,148,82,4,234,246,125,95,7,142,231,121,194,166,227,29,3,92,85,213,223,201,229,229,93,158,231,178,221,110,139,111,42,210,86,75,180,210,84,36,10,58,12,67,17,1,218,0,246,16,194,190,35,95,251,227,
108,58,125,26,143,199,15,229,100,34,1,185,90,173,214,214,0,166,67,65,2,88,210,205,118,241,142,109,118,95,5,254,53,155,61,255,44,138,219,84,89,92,46,151,162,38,80,192,0,14,41,172,65,233,70,240,63,129,223,
115,2,184,44,203,225,143,60,127,25,12,6,217,98,62,215,176,218,42,94,102,155,148,194,219,169,204,183,141,160,124,252,237,7,157,78,231,69,213,105,54,95,44,254,155,165,26,215,176,42,34,64,26,192,108,214,
5,216,29,195,55,215,215,207,253,126,63,155,43,179,49,155,165,122,133,85,116,227,72,117,101,221,153,25,150,172,239,133,125,199,4,248,114,195,69,81,60,245,122,189,91,24,93,40,88,110,80,168,69,0,1,50,161,
0,52,27,150,70,221,218,14,187,6,30,141,70,195,110,183,251,192,15,77,88,164,42,224,0,138,6,6,240,136,236,154,29,217,149,163,83,90,205,210,63,10,104,35,189,57,149,185,94,99,53,111,49,115,113,227,119,238,
200,188,96,236,78,120,207,94,195,170,81,221,109,195,250,212,136,80,183,0,197,146,161,13,83,42,155,219,84,109,105,250,238,5,70,237,238,179,203,35,104,93,187,128,101,187,212,145,55,75,214,13,104,169,140,
77,183,31,2,38,164,218,133,81,192,98,111,78,168,59,75,74,101,19,218,149,58,150,202,218,197,206,67,78,103,30,69,84,199,188,100,152,176,46,53,44,157,210,223,207,206,178,125,134,245,250,72,208,122,4,25,169,
108,235,151,208,135,54,45,30,71,129,97,57,160,221,89,210,188,181,245,195,224,83,192,28,1,223,198,119,174,171,176,7,129,217,32,195,173,191,113,29,6,125,19,216,180,108,134,235,208,7,129,55,108,59,218,160,
142,6,230,181,80,67,154,63,159,44,240,129,154,62,217,26,62,37,163,31,50,220,0,55,192,13,112,3,220,0,55,192,95,115,254,9,48,0,68,95,86,182,0,111,217,153,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::backgroundBoxTl_png = (const char*) resource_JucerScanCtlComponent_backgroundBoxTl_png;
const int JucerScanCtlComponent::backgroundBoxTl_pngSize = 1501;

// JUCER_RESOURCE: backgroundBoxTr_png, 1411, "../../app_eigend2/jucer/BackgroundBoxTR.png"
static const unsigned char resource_JucerScanCtlComponent_backgroundBoxTr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,60,0,0,0,30,8,6,0,0,0,112,152,125,79,0,0,0,25,116,69,88,116,83,111,
102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,54,70,55,66,50,51,49,52,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,54,70,55,66,50,51,49,53,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,54,70,55,66,50,51,49,50,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,54,70,55,66,50,51,49,51,70,69,56,55,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,117,38,168,210,0,0,1,247,73,68,65,84,120,218,236,151,191,79,194,64,20,199,239,74,255,0,212,0,197,144,24,255,1,64,71,102,98,29,92,76,136,43,113,197,213,1,221,13,138,14,254,11,
16,23,119,19,145,129,137,25,225,15,16,99,34,133,24,98,138,174,134,243,90,122,205,235,217,86,140,131,87,236,75,94,238,103,47,124,238,125,251,94,193,195,193,128,32,193,12,99,108,123,36,18,65,178,44,35,73,
146,16,33,214,79,165,109,167,219,213,135,154,246,248,208,239,159,29,148,74,215,198,156,189,110,110,153,245,151,99,49,231,217,47,163,145,144,192,6,160,225,6,176,49,54,129,32,12,27,211,246,89,211,62,218,
237,246,205,94,161,176,11,97,93,129,95,199,99,97,35,108,0,155,176,16,2,128,18,174,63,160,224,119,205,230,197,126,177,120,196,246,173,196,227,206,179,223,116,93,72,96,104,110,82,181,163,204,65,27,118,219,
104,220,111,171,234,38,114,3,126,159,76,132,3,230,104,17,225,225,249,40,243,151,64,251,221,94,79,111,181,90,217,147,74,229,9,30,39,7,9,22,17,226,179,213,130,182,198,217,116,58,74,39,187,180,187,4,247,
73,200,186,21,209,156,240,176,78,205,207,117,81,153,76,38,122,85,171,117,156,192,34,6,246,23,160,142,231,233,88,85,213,141,243,106,181,34,52,48,158,99,13,179,11,240,186,4,32,255,173,124,254,240,184,92,
94,155,1,179,135,4,115,179,52,185,193,91,243,12,20,131,50,230,170,20,10,190,154,76,202,169,84,234,82,216,8,59,36,12,190,186,120,104,204,71,216,71,242,185,92,110,39,24,101,201,39,35,179,228,68,166,83,52,
165,78,96,153,2,165,139,69,187,86,175,159,74,40,128,102,71,27,180,80,5,216,35,9,42,137,132,26,72,96,8,141,249,119,154,191,12,144,196,20,69,89,15,164,164,121,121,155,238,37,107,248,21,38,124,210,250,97,
9,195,126,117,218,90,11,60,48,123,127,191,72,218,35,115,7,31,216,35,234,94,177,94,72,96,228,149,184,22,22,216,199,164,69,146,241,191,2,254,238,159,21,14,37,29,2,135,192,33,112,8,28,2,135,192,127,110,159,
2,12,0,236,127,82,60,3,212,32,63,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::backgroundBoxTr_png = (const char*) resource_JucerScanCtlComponent_backgroundBoxTr_png;
const int JucerScanCtlComponent::backgroundBoxTr_pngSize = 1411;

// JUCER_RESOURCE: innerBoxB_png, 1031, "../../app_eigend2/jucer/InnerBoxB.png"
static const unsigned char resource_JucerScanCtlComponent_innerBoxB_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,2,0,0,0,180,82,57,245,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,102,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,
49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,
115,111,117,114,99,101,82,101,102,35,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,32,120,109,112,77,
77,58,79,114,105,103,105,110,97,108,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,48,50,56,48,49,49,55,52,48,55,50,48,54,56,49,49,56,55,49,70,68,57,53,48,49,57,50,49,55,55,51,
55,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,68,53,48,52,67,48,66,56,70,69,56,67,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,
32,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,68,53,48,52,67,48,66,55,70,69,56,67,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,
109,112,58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,62,32,60,120,109,112,77,77,58,68,101,
114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,48,50,56,48,49,49,55,52,48,55,50,48,54,56,49,49,56,55,49,70,68,57,53,
48,49,57,50,49,55,55,51,55,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,48,50,56,48,49,49,55,52,48,55,50,48,54,56,49,49,56,55,49,70,68,57,53,48,
49,57,50,49,55,55,51,55,34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,
97,99,107,101,116,32,101,110,100,61,34,114,34,63,62,82,30,129,139,0,0,0,55,73,68,65,84,120,218,236,208,49,17,0,48,12,195,192,180,87,254,124,189,216,153,11,32,153,36,0,63,232,72,170,153,110,141,5,13,13,
13,13,13,13,253,247,146,64,47,209,182,25,178,67,183,0,3,0,138,14,27,28,26,167,249,78,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::innerBoxB_png = (const char*) resource_JucerScanCtlComponent_innerBoxB_png;
const int JucerScanCtlComponent::innerBoxB_pngSize = 1031;

// JUCER_RESOURCE: innerBoxBl_png, 1322, "../../app_eigend2/jucer/InnerBoxBL.png"
static const unsigned char resource_JucerScanCtlComponent_innerBoxBl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,6,0,0,0,59,48,174,162,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,
99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,52,54,50,69,68,65,55,56,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,52,54,50,69,68,65,55,57,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,68,53,48,52,67,48,66,57,70,69,56,67,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,68,53,48,52,67,48,66,65,70,69,56,67,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,181,222,114,193,0,0,1,158,73,68,65,84,120,218,236,150,223,109,131,48,16,198,177,249,243,204,8,44,128,196,6,205,8,217,160,237,8,29,165,19,116,132,118,131,168,3,32,229,9,33,33,
68,16,226,129,37,160,190,40,23,93,174,7,165,128,213,135,198,210,197,78,2,254,221,247,157,109,89,165,105,58,104,173,29,215,117,29,223,247,207,17,4,129,227,121,222,245,59,252,7,161,148,186,198,218,166,157,
63,106,119,240,29,252,15,193,195,48,220,244,86,192,182,32,34,152,159,66,18,212,70,34,122,202,98,155,202,61,233,220,69,40,133,243,68,214,158,215,160,248,68,39,225,42,109,41,135,26,31,249,143,125,223,127,
91,108,155,175,106,3,254,68,197,146,181,144,4,183,126,139,68,0,252,49,86,95,9,186,213,106,215,113,28,67,141,143,188,206,8,181,166,248,252,161,245,43,183,155,43,199,160,192,53,9,40,124,49,203,178,202,116,
17,94,131,224,234,3,129,99,188,254,64,192,51,16,152,236,146,235,144,38,251,242,133,170,166,10,169,98,73,253,18,251,21,125,56,207,243,119,3,223,67,2,84,161,20,168,152,43,159,123,192,120,55,242,181,126,
6,187,205,75,137,84,103,126,203,4,40,253,157,66,127,82,175,248,3,69,81,36,230,197,131,25,134,88,111,172,233,88,157,177,151,148,207,6,67,43,203,50,49,221,193,76,16,82,168,52,230,224,185,112,53,102,73,85,
85,0,127,3,219,57,132,130,57,84,90,225,82,18,106,170,22,117,93,135,23,248,158,130,199,198,124,107,77,169,86,115,182,64,211,52,59,51,9,36,16,81,107,231,88,61,6,87,191,217,123,109,219,62,25,200,163,153,
108,135,16,201,234,81,123,233,202,95,114,228,117,93,23,93,246,251,3,172,1,116,98,174,90,104,95,2,12,0,21,249,65,31,156,76,137,157,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::innerBoxBl_png = (const char*) resource_JucerScanCtlComponent_innerBoxBl_png;
const int JucerScanCtlComponent::innerBoxBl_pngSize = 1322;

// JUCER_RESOURCE: innerBoxBr_png, 1208, "../../app_eigend2/jucer/InnerBoxBR.png"
static const unsigned char resource_JucerScanCtlComponent_innerBoxBr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,6,0,0,0,59,48,174,162,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,
99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,52,54,50,69,68,65,55,67,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,52,54,50,69,68,65,55,68,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,52,54,50,69,68,65,55,65,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,52,54,50,69,68,65,55,66,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,188,71,37,152,0,0,1,44,73,68,65,84,120,218,236,86,203,13,131,48,12,109,34,22,97,20,70,97,132,142,210,13,218,13,24,1,54,96,132,140,192,149,11,212,65,78,101,140,147,38,45,31,33,
97,41,138,157,24,158,127,216,168,190,239,199,219,10,164,148,10,238,156,244,237,32,186,128,47,224,11,248,28,192,190,230,113,126,143,185,103,33,79,55,241,56,6,208,81,182,71,126,41,63,142,211,76,50,217,222,
97,198,251,86,111,225,85,132,33,141,250,117,30,75,160,210,12,246,0,231,122,235,34,18,168,133,231,141,94,43,167,62,99,176,152,40,61,38,253,148,80,75,47,143,253,213,193,59,3,43,255,251,59,150,64,4,15,233,
221,253,136,150,89,105,173,171,15,176,181,48,100,101,130,55,51,158,233,182,176,202,89,231,114,10,84,49,148,75,171,39,241,1,185,179,160,224,109,39,2,167,228,213,61,195,121,1,188,3,185,0,185,93,12,9,23,
22,186,134,97,152,22,63,163,187,20,94,198,91,176,2,247,229,144,176,47,242,133,152,123,195,239,120,135,34,103,21,240,37,134,89,158,78,62,171,125,134,208,239,85,48,204,96,17,213,223,138,50,42,199,14,144,
130,49,192,26,206,95,32,63,121,36,162,60,78,40,34,131,185,107,108,88,81,78,234,227,111,1,6,0,34,24,205,23,156,48,191,33,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::innerBoxBr_png = (const char*) resource_JucerScanCtlComponent_innerBoxBr_png;
const int JucerScanCtlComponent::innerBoxBr_pngSize = 1208;

// JUCER_RESOURCE: innerBoxInner_png, 951, "../../app_eigend2/jucer/InnerBoxInner.png"
static const unsigned char resource_JucerScanCtlComponent_innerBoxInner_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,2,0,0,0,180,82,57,245,0,0,0,25,116,69,88,116,83,111,
102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,
97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,52,54,50,69,68,65,56,48,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,52,54,50,69,68,65,56,49,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,52,54,50,69,68,65,55,69,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,52,54,50,69,68,65,55,70,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,12,71,194,222,0,0,0,43,73,68,65,84,120,218,236,204,49,17,0,0,8,4,32,181,127,225,223,76,225,226,65,0,58,73,221,152,58,163,86,171,213,106,181,90,173,254,95,175,0,3,0,100,174,3,
39,171,113,64,65,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::innerBoxInner_png = (const char*) resource_JucerScanCtlComponent_innerBoxInner_png;
const int JucerScanCtlComponent::innerBoxInner_pngSize = 951;

// JUCER_RESOURCE: innerBoxL_png, 970, "../../app_eigend2/jucer/InnerBoxL.png"
static const unsigned char resource_JucerScanCtlComponent_innerBoxL_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,2,0,0,0,180,82,57,245,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,
99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,57,69,50,70,54,70,67,55,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,57,69,50,70,54,70,67,56,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,52,54,50,69,68,65,56,50,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,57,69,50,70,54,70,67,54,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,82,172,199,164,0,0,0,62,73,68,65,84,120,218,236,204,177,13,0,32,8,5,81,249,32,251,47,198,78,38,24,7,176,176,49,119,213,85,207,170,74,146,187,207,46,51,35,98,191,119,118,26,151,
105,60,11,26,26,26,26,26,26,26,26,250,127,122,9,48,0,91,93,3,39,221,167,221,199,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::innerBoxL_png = (const char*) resource_JucerScanCtlComponent_innerBoxL_png;
const int JucerScanCtlComponent::innerBoxL_pngSize = 970;

// JUCER_RESOURCE: innerBoxR_png, 958, "../../app_eigend2/jucer/InnerBoxR.png"
static const unsigned char resource_JucerScanCtlComponent_innerBoxR_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,2,0,0,0,180,82,57,245,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,
99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,57,69,50,70,54,70,67,66,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,57,69,50,70,54,70,67,67,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,57,69,50,70,54,70,67,57,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,57,69,50,70,54,70,67,65,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,233,69,86,204,0,0,0,50,73,68,65,84,120,218,236,204,161,17,0,0,8,195,64,202,254,11,215,97,17,24,4,134,75,68,228,203,118,236,147,52,190,151,113,22,52,52,52,52,52,52,52,52,244,
127,186,4,24,0,225,154,3,51,104,217,15,61,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::innerBoxR_png = (const char*) resource_JucerScanCtlComponent_innerBoxR_png;
const int JucerScanCtlComponent::innerBoxR_pngSize = 958;

// JUCER_RESOURCE: innerBoxT_png, 972, "../../app_eigend2/jucer/InnerBoxT.png"
static const unsigned char resource_JucerScanCtlComponent_innerBoxT_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,2,0,0,0,180,82,57,245,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,
99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,57,69,50,70,54,70,67,70,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,57,69,50,70,54,70,68,48,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,57,69,50,70,54,70,67,68,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,57,69,50,70,54,70,67,69,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,90,114,10,222,0,0,0,64,73,68,65,84,120,218,236,211,49,13,0,48,16,66,209,114,212,191,66,196,244,198,26,96,32,225,11,120,3,9,144,116,60,93,0,46,154,100,233,159,222,2,233,204,173,
103,38,240,50,145,180,107,232,210,165,75,151,46,29,67,63,1,6,0,122,2,3,63,215,133,214,189,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::innerBoxT_png = (const char*) resource_JucerScanCtlComponent_innerBoxT_png;
const int JucerScanCtlComponent::innerBoxT_pngSize = 972;

// JUCER_RESOURCE: innerBoxTl_png, 1308, "../../app_eigend2/jucer/InnerBoxTL.png"
static const unsigned char resource_JucerScanCtlComponent_innerBoxTl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,6,0,0,0,59,48,174,162,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,
99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,68,48,68,68,67,51,55,48,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,68,48,68,68,67,51,55,49,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,68,48,68,68,67,51,54,69,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,68,48,68,68,67,51,54,70,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,29,136,18,156,0,0,1,144,73,68,65,84,120,218,236,150,221,109,132,48,12,199,33,124,60,51,2,35,176,65,111,4,54,104,55,232,40,29,225,70,184,17,110,0,16,32,132,120,237,38,80,187,
58,75,206,39,73,185,220,83,45,229,156,32,200,239,254,182,113,72,247,125,79,66,109,93,215,58,77,211,22,166,111,224,27,24,184,78,112,160,145,87,231,220,210,16,240,178,44,31,224,222,97,179,11,129,112,8,33,
18,19,216,6,245,6,207,243,124,1,119,229,202,8,134,158,131,125,225,78,240,52,77,213,3,216,114,160,105,112,240,41,197,227,56,54,15,104,163,2,179,44,147,230,60,2,106,110,131,20,15,195,128,208,59,60,84,209,
166,8,32,32,205,9,234,202,179,13,174,129,17,10,215,36,104,158,231,18,140,255,9,87,168,37,144,114,77,2,3,180,130,245,8,211,223,34,34,0,129,209,227,80,195,29,170,22,45,231,139,109,219,174,8,85,243,201,161,
38,197,174,162,58,204,113,223,247,216,16,110,60,167,4,43,138,66,82,125,86,173,170,248,139,110,182,41,229,138,109,106,143,128,146,98,80,139,29,9,195,172,41,37,181,106,168,109,80,95,48,41,254,196,31,83,
110,93,213,108,123,87,189,20,119,93,87,131,255,54,169,45,203,82,203,49,87,235,219,151,109,138,91,122,144,55,3,222,44,92,133,20,10,36,19,120,180,241,77,76,144,103,67,9,220,240,106,54,29,4,106,87,58,3,228,
224,218,118,182,158,125,101,142,192,90,248,66,142,183,63,131,77,213,137,42,185,178,88,161,214,32,42,48,134,9,19,36,54,84,83,108,237,50,49,114,124,244,169,18,171,192,132,207,215,66,212,28,191,218,196,171,
149,6,21,215,63,248,25,246,35,192,0,66,67,85,146,57,122,208,132,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::innerBoxTl_png = (const char*) resource_JucerScanCtlComponent_innerBoxTl_png;
const int JucerScanCtlComponent::innerBoxTl_pngSize = 1308;

// JUCER_RESOURCE: innerBoxTr_png, 1249, "../../app_eigend2/jucer/InnerBoxTR.png"
static const unsigned char resource_JucerScanCtlComponent_innerBoxTr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,30,8,6,0,0,0,59,48,174,162,0,0,0,25,116,69,88,116,83,111,102,
116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,
99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,
54,48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,
34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,
112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,
116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,
58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,
99,101,73,68,61,34,120,109,112,46,105,105,100,58,68,48,68,68,67,51,55,52,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,
73,68,61,34,120,109,112,46,100,105,100,58,68,48,68,68,67,51,55,53,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,
114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,68,48,68,68,67,51,55,50,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,
70,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,68,48,68,68,67,51,55,51,70,69,56,68,49,49,69,49,66,70,53,51,70,49,48,52,51,57,51,67,68,65,57,70,
34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,
110,100,61,34,114,34,63,62,184,16,215,105,0,0,1,85,73,68,65,84,120,218,236,148,219,109,131,48,20,134,141,241,3,151,151,142,224,110,192,6,77,54,32,27,164,19,116,148,142,144,21,216,32,234,6,25,0,169,140,
144,23,64,130,7,242,27,145,200,165,199,24,165,224,72,85,142,244,235,248,38,127,231,28,95,188,60,207,59,102,48,207,243,126,249,65,5,116,130,190,160,140,115,94,64,253,156,242,148,162,40,250,177,55,159,90,
172,129,198,125,9,165,208,39,246,248,238,186,238,8,237,33,102,210,216,184,239,251,70,176,154,163,230,175,1,104,85,216,160,121,80,65,64,155,57,96,161,54,182,149,154,26,31,151,127,104,75,184,35,148,65,239,
208,249,46,48,5,215,251,227,182,22,64,10,39,7,248,137,218,87,192,152,205,76,153,91,2,73,134,236,183,20,124,22,120,110,16,196,186,23,120,18,110,45,181,13,76,157,31,1,63,148,101,185,141,227,248,124,55,216,
102,84,32,170,236,10,142,230,238,54,86,85,85,183,54,88,203,126,23,134,97,214,247,235,186,94,20,108,10,98,0,23,65,16,188,222,126,174,191,202,244,235,17,159,142,108,154,102,191,24,120,234,147,33,214,126,
244,151,107,169,178,206,125,102,176,164,109,91,41,214,58,95,61,16,226,194,165,98,109,168,161,26,111,156,61,198,18,177,102,182,19,231,46,31,149,49,227,46,46,151,83,176,205,132,11,8,245,180,156,103,124,
13,130,187,204,86,31,251,127,183,250,9,126,130,47,2,12,0,239,146,113,98,115,191,7,186,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::innerBoxTr_png = (const char*) resource_JucerScanCtlComponent_innerBoxTr_png;
const int JucerScanCtlComponent::innerBoxTr_pngSize = 1249;

// JUCER_RESOURCE: eigenD_png, 3575, "eigenD@2x.png"
static const unsigned char resource_JucerScanCtlComponent_eigenD_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,1,36,0,0,0,120,8,2,0,0,0,212,160,24,131,0,0,0,25,116,69,88,116,83,111,102,116,
119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,99,
107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,58,120,109,112,109,101,116,
97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,48,45,99,48,54,
48,32,54,49,46,49,51,52,55,55,55,44,32,50,48,49,48,47,48,50,47,49,50,45,49,55,58,51,50,58,48,48,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,34,
104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,112,
116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,
34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,116,82,
101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,58,67,
114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,83,53,32,77,97,99,105,110,116,111,115,104,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,
101,73,68,61,34,120,109,112,46,105,105,100,58,66,67,69,50,54,65,54,53,70,57,68,57,49,49,69,49,66,67,69,50,70,54,67,67,50,52,66,53,56,70,57,53,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,
68,61,34,120,109,112,46,100,105,100,58,66,67,69,50,54,65,54,54,70,57,68,57,49,49,69,49,66,67,69,50,70,54,67,67,50,52,66,53,56,70,57,53,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,
111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,66,67,69,50,54,65,54,51,70,57,68,57,49,49,69,49,66,67,69,50,70,54,67,67,50,52,66,53,56,70,57,53,
34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,66,67,69,50,54,65,54,52,70,57,68,57,49,49,69,49,66,67,69,50,70,54,67,67,50,52,66,53,56,70,57,53,34,
47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,110,
100,61,34,114,34,63,62,220,169,174,226,0,0,10,107,73,68,65,84,120,218,236,157,187,114,211,74,28,198,61,158,244,36,188,0,144,60,0,97,72,79,152,137,107,66,65,90,160,57,41,153,84,184,35,116,184,50,109,104,
2,45,41,18,106,123,198,161,79,38,225,1,8,225,5,18,120,130,156,239,104,15,123,246,172,100,105,47,210,106,37,125,191,194,163,56,90,105,45,237,167,255,101,47,234,245,8,105,35,171,171,171,177,85,169,207,187,
66,90,201,221,187,119,41,54,66,104,217,8,105,23,235,235,235,20,27,33,93,132,98,35,237,228,214,173,91,20,27,33,118,44,46,46,50,102,35,36,234,208,43,54,189,81,108,36,118,156,53,227,102,18,41,54,210,93,30,
61,122,212,142,31,66,177,17,58,159,132,144,132,155,155,27,183,82,135,135,135,180,108,132,132,8,219,24,179,17,210,120,217,80,108,132,54,205,171,20,197,70,104,211,172,135,240,187,149,162,216,8,113,148,13,
197,70,72,131,5,67,177,145,150,139,205,118,84,49,99,54,66,28,177,21,143,204,94,70,149,198,164,216,72,212,120,206,148,137,202,196,81,108,36,52,47,94,188,104,186,67,72,177,145,198,132,97,182,3,23,109,247,
191,115,231,14,197,70,72,239,254,253,251,85,135,82,113,230,48,41,54,82,131,101,51,119,14,61,221,200,168,198,254,83,108,36,52,208,15,140,155,225,206,210,6,182,32,120,163,216,72,104,165,185,185,121,86,158,
39,221,72,66,254,149,129,161,153,114,214,12,197,70,200,127,50,51,209,155,170,25,55,253,68,181,164,2,197,70,130,34,163,53,219,132,100,11,6,73,82,108,36,40,82,99,38,121,66,183,30,130,104,103,154,82,108,
36,40,82,99,38,253,206,170,171,105,62,110,43,218,188,37,197,70,194,225,19,131,181,96,202,54,197,70,234,17,91,152,238,102,142,250,39,29,69,179,51,133,74,48,239,251,110,4,20,27,9,135,22,167,21,250,120,170,
26,205,45,97,180,203,179,82,108,164,54,203,22,38,160,138,167,207,128,98,35,245,196,108,38,110,100,41,58,161,216,8,197,86,60,188,67,219,191,233,253,218,20,27,169,199,135,236,85,54,136,36,218,180,10,197,
70,2,145,150,86,69,49,155,118,162,120,178,255,20,27,9,68,102,146,48,199,88,165,247,111,250,251,126,23,74,63,226,195,135,15,151,150,150,196,246,245,245,245,233,233,41,219,25,201,241,12,47,47,47,205,101,
115,116,116,212,220,31,91,190,216,222,189,123,183,177,177,33,182,167,211,233,96,48,96,147,2,184,38,120,12,165,191,191,184,184,192,35,233,34,161,221,87,32,216,108,151,104,243,40,11,148,65,24,160,52,60,
134,242,247,129,23,128,199,147,248,132,2,91,118,5,204,45,152,167,102,162,21,27,99,182,184,4,249,250,245,235,207,159,63,95,93,93,237,237,237,73,7,161,29,124,250,244,41,45,191,227,227,99,115,205,184,217,
198,120,230,143,150,111,217,212,32,141,1,219,188,75,36,13,215,60,69,253,149,0,19,55,26,141,240,217,130,95,13,93,189,124,249,114,60,30,139,60,7,148,246,244,233,83,54,6,82,62,48,89,55,127,208,4,182,180,
180,132,111,164,77,187,73,1,255,83,230,156,90,192,250,250,186,73,134,112,119,119,87,187,14,179,217,172,176,20,148,236,80,138,116,69,108,26,207,158,61,155,76,38,90,139,57,57,57,105,147,222,76,128,72,180,
139,240,227,199,15,19,37,71,43,54,198,108,209,113,112,112,48,72,80,157,112,132,115,80,96,215,244,86,74,230,163,49,253,108,178,211,172,137,61,102,21,85,30,199,20,73,252,74,243,245,136,211,214,214,214,224,
64,194,36,170,122,131,8,219,151,168,172,148,104,151,36,249,207,147,65,252,144,14,30,240,37,162,246,252,178,216,97,242,135,204,158,165,124,121,160,109,237,237,237,137,226,233,10,152,36,232,124,42,223,75,
250,9,101,253,181,186,225,95,223,191,127,87,143,137,63,165,24,202,114,35,211,215,83,139,223,58,34,146,179,179,179,244,77,116,112,35,77,74,213,102,13,50,91,185,74,190,63,227,208,170,112,52,148,210,218,
113,38,249,7,244,175,60,80,143,32,190,193,73,243,15,107,226,224,57,139,77,43,11,150,151,151,187,32,182,204,75,93,232,19,166,211,42,145,138,13,141,85,203,134,33,46,207,52,50,57,241,186,109,171,194,254,
153,41,56,91,177,149,82,121,77,108,133,50,51,55,56,62,98,211,106,5,203,223,5,177,193,91,78,95,231,194,176,45,83,108,209,121,146,106,99,197,6,26,135,214,34,225,158,169,198,7,94,153,103,171,194,241,209,
238,213,139,130,243,10,103,15,5,229,217,77,14,88,86,229,181,102,173,153,47,84,76,90,149,180,14,243,13,142,167,216,80,68,61,87,23,50,37,251,251,251,14,217,200,76,177,197,181,80,2,110,158,108,139,104,172,
243,98,45,77,30,153,141,198,176,85,161,105,170,173,31,219,104,202,153,109,168,240,128,37,86,62,83,108,176,36,243,132,164,238,156,31,188,121,138,13,168,151,11,207,142,46,36,54,160,46,249,147,97,232,220,
122,231,226,17,91,95,70,225,178,61,109,109,109,205,203,221,225,7,227,191,106,3,114,214,54,108,139,60,227,135,15,31,214,214,214,240,233,150,103,171,174,242,7,7,7,43,43,43,219,219,219,243,178,142,216,65,
181,174,149,222,42,245,92,93,8,219,126,253,250,245,224,193,131,157,157,157,183,111,223,226,243,222,189,123,231,231,231,133,165,76,214,126,133,47,186,155,96,245,194,225,210,204,154,244,193,76,226,1,236,
147,227,59,153,60,194,17,225,152,71,32,249,7,44,183,242,153,9,146,28,112,4,213,207,172,212,178,169,105,201,28,55,184,227,164,187,194,53,203,6,129,169,255,58,59,59,11,22,209,245,69,60,35,253,183,209,104,
100,245,136,117,104,55,104,160,210,170,192,98,12,135,67,159,31,16,184,242,26,33,231,197,168,231,234,120,239,182,45,50,173,2,93,141,199,99,245,95,112,77,131,217,183,5,213,255,193,237,92,78,176,178,138,
14,242,144,219,206,174,99,218,121,11,83,121,210,104,32,173,180,29,123,242,228,201,251,247,239,67,139,13,45,53,223,23,202,105,235,86,137,53,85,108,158,63,32,112,229,73,228,124,249,242,37,157,14,201,153,
200,19,58,65,226,211,230,28,140,131,20,155,152,164,92,150,216,194,84,190,70,212,95,202,185,75,243,248,248,241,163,150,71,217,217,217,177,157,183,90,161,101,147,76,19,172,202,251,168,165,220,128,39,112,
229,235,21,27,135,71,206,67,228,48,17,134,137,56,237,56,161,148,35,195,255,196,97,241,137,83,64,210,248,244,21,155,73,142,33,78,26,93,121,19,35,172,198,186,237,152,78,90,169,125,203,252,62,179,243,224,
235,215,175,38,193,222,108,54,147,241,222,171,87,175,30,63,126,108,107,48,251,170,133,9,19,195,200,167,114,41,157,69,129,43,95,23,234,16,234,211,4,42,202,205,238,185,21,148,19,204,5,48,155,90,86,211,84,
108,242,206,133,89,244,66,158,206,54,121,152,127,180,150,173,216,161,57,144,234,216,75,255,172,18,177,37,157,116,217,220,220,244,18,27,124,21,147,73,40,254,254,158,220,246,31,118,20,184,242,181,56,144,
106,23,54,174,30,197,214,80,250,189,255,119,118,57,143,192,50,71,237,86,198,233,60,221,191,192,149,15,111,211,38,147,137,180,255,248,165,219,219,219,108,181,229,122,146,110,185,74,7,143,180,47,110,161,
76,45,224,190,86,61,131,3,81,150,60,29,30,219,56,157,79,10,62,112,229,67,34,22,35,81,31,70,57,163,52,137,33,233,28,137,155,216,76,6,106,102,136,77,216,7,121,23,225,140,85,221,100,33,15,53,177,113,114,
114,226,19,113,5,174,124,0,196,252,29,120,143,234,99,8,74,83,157,2,18,146,82,250,15,250,210,62,108,109,109,73,127,12,77,214,83,0,133,230,72,61,157,24,252,1,220,66,184,192,149,175,212,105,132,39,44,230,
188,170,245,199,163,68,204,138,96,163,111,52,11,106,166,97,48,24,200,25,254,34,90,192,109,198,211,84,172,134,173,166,155,197,162,55,62,189,61,226,116,176,66,210,77,218,72,16,39,18,103,20,246,202,36,99,
25,184,242,254,162,210,182,197,155,0,50,221,233,81,2,123,177,99,195,196,249,20,67,49,225,112,102,7,120,194,200,152,44,4,112,117,117,149,121,130,186,150,69,40,165,242,61,203,41,54,61,101,169,12,243,41,
54,134,228,76,90,37,206,164,103,151,154,148,58,60,60,212,74,225,56,249,50,83,23,44,18,19,11,250,233,236,133,88,180,176,240,193,95,86,191,42,30,219,43,43,43,195,225,208,255,128,225,43,95,5,168,27,174,198,
237,219,183,153,14,137,135,111,223,190,89,237,191,191,191,175,206,43,199,159,155,155,155,217,235,70,138,113,134,120,172,10,247,6,27,154,147,131,70,80,162,27,38,50,138,34,169,40,95,173,36,141,152,109,223,
119,224,202,251,63,32,196,128,108,104,76,84,140,30,99,211,89,77,208,190,124,254,252,121,3,170,174,58,96,156,20,67,106,113,35,211,165,114,214,53,201,92,187,114,54,155,53,96,249,113,78,45,33,213,37,54,12,
187,203,172,122,213,50,211,33,248,178,1,98,147,254,36,149,70,74,23,155,225,64,16,171,241,34,153,202,68,212,23,187,216,212,21,32,217,165,75,226,33,223,214,101,14,10,139,90,108,162,147,87,108,95,95,95,83,
108,36,18,123,88,104,235,50,7,133,197,43,54,241,130,14,117,229,44,230,193,137,63,154,72,12,253,67,219,241,147,233,253,29,198,82,86,142,152,41,163,189,100,163,35,171,219,147,48,152,247,77,207,43,85,216,
61,147,153,243,92,168,81,84,34,205,136,13,209,141,38,251,196,180,61,135,195,97,139,215,59,32,237,11,216,122,201,192,229,55,111,222,168,127,214,41,54,147,149,231,166,211,105,41,35,75,8,241,7,130,49,127,
103,64,166,179,90,155,216,114,36,36,6,34,171,19,103,8,105,180,233,19,163,189,22,106,172,144,28,51,37,223,151,43,150,178,225,120,37,18,204,70,185,77,84,51,89,144,11,122,147,131,182,132,246,234,20,219,96,
48,224,141,39,13,50,86,86,175,158,82,61,73,145,156,236,243,34,18,98,194,239,223,191,157,173,159,176,108,20,27,33,214,152,116,187,201,125,228,6,197,70,186,232,16,102,110,219,186,133,20,27,33,22,14,161,
249,8,99,219,33,32,50,245,34,253,73,138,141,16,107,172,6,121,209,178,17,82,142,35,74,177,17,98,17,77,89,185,133,230,8,7,82,22,164,216,8,197,86,161,183,169,158,130,98,35,196,14,115,19,7,111,147,98,35,164,
194,80,77,221,95,45,66,177,145,206,33,115,137,182,111,162,113,216,95,93,112,146,98,35,221,53,80,182,75,175,58,68,122,234,11,135,41,54,66,236,100,246,243,231,79,183,226,20,27,33,166,56,203,140,98,35,196,
43,228,163,216,8,41,70,164,239,221,102,142,58,175,147,69,177,17,98,39,81,186,145,132,196,14,197,70,72,80,251,70,72,183,24,143,199,134,47,139,82,89,92,92,116,40,69,203,70,58,141,237,130,34,2,231,60,36,
197,70,136,53,62,75,246,83,108,132,80,108,132,84,198,229,229,165,155,108,124,6,145,80,108,164,163,98,115,11,192,124,230,155,82,108,132,88,112,116,116,68,177,17,18,2,159,132,36,197,70,58,170,153,170,23,
32,161,216,8,249,135,243,243,115,207,249,50,20,27,33,241,66,177,17,66,177,17,82,37,225,99,54,66,72,32,254,22,96,0,67,188,50,239,194,216,58,162,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCtlComponent::eigenD_png = (const char*) resource_JucerScanCtlComponent_eigenD_png;
const int JucerScanCtlComponent::eigenD_pngSize = 3575;
