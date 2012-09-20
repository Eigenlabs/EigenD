/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  28 Jan 2011 3:50:54pm

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

#include "JucerScanCompleteComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
JucerScanCompleteComponent::JucerScanCompleteComponent ()
    : label (0),
      textButton (0),
      label2 (0),
      label3 (0),
      failed_list (0),
      good_list (0),
      cachedImage_backgroundBoxInner_png (0),
      cachedImage_backgroundBoxT_png (0),
      cachedImage_backgroundBoxL_png (0),
      cachedImage_backgroundBoxR_png (0),
      cachedImage_backgroundBoxTl_png (0),
      cachedImage_backgroundBoxTr_png (0),
      cachedImage_backgroundBoxB_png (0),
      cachedImage_backgroundBoxBr_png (0),
      cachedImage_backgroundBoxBl_png (0),
      cachedImage_eigenD_png (0),
      cachedImage_innerBoxBr_png (0),
      cachedImage_innerBoxInner_png (0),
      cachedImage_innerBoxTl_png (0),
      cachedImage_innerBoxTr_png (0),
      cachedImage_innerBoxBl_png (0),
      cachedImage_innerBoxT_png (0),
      cachedImage_innerBoxB_png (0),
      cachedImage_innerBoxL_png (0),
      cachedImage_innerBoxR_png (0)
{
    addAndMakeVisible (label = new Label ("new label",
                                          "Scan complete."));
    label->setFont (Font (15.0000f, Font::bold));
    label->setJustificationType (Justification::centred);
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (textButton = new TextButton ("new button"));
    textButton->setButtonText ("OK");
    textButton->addListener (this);
    textButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (label2 = new Label ("new label",
                                           "Failed Plugins:"));
    label2->setFont (Font (15.0000f, Font::bold));
    label2->setJustificationType (Justification::centredLeft);
    label2->setEditable (false, false, false);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label3 = new Label ("new label",
                                           "Good Plugins:"));
    label3->setFont (Font (15.0000f, Font::bold));
    label3->setJustificationType (Justification::centredLeft);
    label3->setEditable (false, false, false);
    label3->setColour (TextEditor::textColourId, Colours::black);
    label3->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (failed_list = new ListBox());
    failed_list->setName ("new component");

    addAndMakeVisible (good_list = new ListBox());
    good_list->setName ("new component");

    cachedImage_backgroundBoxInner_png = ImageCache::getFromMemory (backgroundBoxInner_png, backgroundBoxInner_pngSize);
    cachedImage_backgroundBoxT_png = ImageCache::getFromMemory (backgroundBoxT_png, backgroundBoxT_pngSize);
    cachedImage_backgroundBoxL_png = ImageCache::getFromMemory (backgroundBoxL_png, backgroundBoxL_pngSize);
    cachedImage_backgroundBoxR_png = ImageCache::getFromMemory (backgroundBoxR_png, backgroundBoxR_pngSize);
    cachedImage_backgroundBoxTl_png = ImageCache::getFromMemory (backgroundBoxTl_png, backgroundBoxTl_pngSize);
    cachedImage_backgroundBoxTr_png = ImageCache::getFromMemory (backgroundBoxTr_png, backgroundBoxTr_pngSize);
    cachedImage_backgroundBoxB_png = ImageCache::getFromMemory (backgroundBoxB_png, backgroundBoxB_pngSize);
    cachedImage_backgroundBoxBr_png = ImageCache::getFromMemory (backgroundBoxBr_png, backgroundBoxBr_pngSize);
    cachedImage_backgroundBoxBl_png = ImageCache::getFromMemory (backgroundBoxBl_png, backgroundBoxBl_pngSize);
    cachedImage_eigenD_png = ImageCache::getFromMemory (eigenD_png, eigenD_pngSize);
    cachedImage_innerBoxBr_png = ImageCache::getFromMemory (innerBoxBr_png, innerBoxBr_pngSize);
    cachedImage_innerBoxInner_png = ImageCache::getFromMemory (innerBoxInner_png, innerBoxInner_pngSize);
    cachedImage_innerBoxTl_png = ImageCache::getFromMemory (innerBoxTl_png, innerBoxTl_pngSize);
    cachedImage_innerBoxTr_png = ImageCache::getFromMemory (innerBoxTr_png, innerBoxTr_pngSize);
    cachedImage_innerBoxBl_png = ImageCache::getFromMemory (innerBoxBl_png, innerBoxBl_pngSize);
    cachedImage_innerBoxT_png = ImageCache::getFromMemory (innerBoxT_png, innerBoxT_pngSize);
    cachedImage_innerBoxB_png = ImageCache::getFromMemory (innerBoxB_png, innerBoxB_pngSize);
    cachedImage_innerBoxL_png = ImageCache::getFromMemory (innerBoxL_png, innerBoxL_pngSize);
    cachedImage_innerBoxR_png = ImageCache::getFromMemory (innerBoxR_png, innerBoxR_pngSize);

    //[UserPreSize]
    //[/UserPreSize]

    setSize (500, 600);

    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

JucerScanCompleteComponent::~JucerScanCompleteComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (label);
    deleteAndZero (textButton);
    deleteAndZero (label2);
    deleteAndZero (label3);
    deleteAndZero (failed_list);
    deleteAndZero (good_list);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void JucerScanCompleteComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::black);

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxInner_png,
                 25, 100, getWidth() - 53, getHeight() - 125,
                 0, 0, cachedImage_backgroundBoxInner_png.getWidth(), cachedImage_backgroundBoxInner_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxT_png,
                 23, 85, getWidth() - 45, 15,
                 0, 0, cachedImage_backgroundBoxT_png.getWidth(), cachedImage_backgroundBoxT_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxL_png,
                 11, 99, 15, getHeight() - 123,
                 0, 0, cachedImage_backgroundBoxL_png.getWidth(), cachedImage_backgroundBoxL_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxR_png,
                 getWidth() - 29, 99, 15, getHeight() - 123,
                 0, 0, cachedImage_backgroundBoxR_png.getWidth(), cachedImage_backgroundBoxR_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxTl_png,
                 11, 85, 30, 15,
                 0, 0, cachedImage_backgroundBoxTl_png.getWidth(), cachedImage_backgroundBoxTl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxTr_png,
                 getWidth() - 44, 85, 30, 15,
                 0, 0, cachedImage_backgroundBoxTr_png.getWidth(), cachedImage_backgroundBoxTr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxB_png,
                 35, getHeight() - 30, getWidth() - 75, 15,
                 0, 0, cachedImage_backgroundBoxB_png.getWidth(), cachedImage_backgroundBoxB_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxBr_png,
                 getWidth() - 44, getHeight() - 30, 30, 15,
                 0, 0, cachedImage_backgroundBoxBr_png.getWidth(), cachedImage_backgroundBoxBr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_backgroundBoxBl_png,
                 11, getHeight() - 30, 30, 15,
                 0, 0, cachedImage_backgroundBoxBl_png.getWidth(), cachedImage_backgroundBoxBl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_eigenD_png,
                 getWidth() - 178, 13, 146, 60,
                 0, 0, cachedImage_eigenD_png.getWidth(), cachedImage_eigenD_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxBr_png,
                 getWidth() - 84, getHeight() - 295, 18, 12,
                 0, 0, cachedImage_innerBoxBr_png.getWidth(), cachedImage_innerBoxBr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxInner_png,
                 419, 180, getWidth() - 487, getHeight() - 474,
                 0, 0, cachedImage_innerBoxInner_png.getWidth(), cachedImage_innerBoxInner_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxTl_png,
                 52, 171, 19, 10,
                 0, 0, cachedImage_innerBoxTl_png.getWidth(), cachedImage_innerBoxTl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxTr_png,
                 getWidth() - 85, 171, 19, 10,
                 0, 0, cachedImage_innerBoxTr_png.getWidth(), cachedImage_innerBoxTr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxBl_png,
                 53, getHeight() - 294, 19, 11,
                 0, 0, cachedImage_innerBoxBl_png.getWidth(), cachedImage_innerBoxBl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxT_png,
                 70, 171, getWidth() - 155, 9,
                 0, 0, cachedImage_innerBoxT_png.getWidth(), cachedImage_innerBoxT_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxB_png,
                 69, getHeight() - 295, getWidth() - 152, 12,
                 0, 0, cachedImage_innerBoxB_png.getWidth(), cachedImage_innerBoxB_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxL_png,
                 52, 180, 13, getHeight() - 471,
                 0, 0, cachedImage_innerBoxL_png.getWidth(), cachedImage_innerBoxL_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxR_png,
                 getWidth() - 435, 180, 355, getHeight() - 475,
                 0, 0, cachedImage_innerBoxR_png.getWidth(), cachedImage_innerBoxR_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxBr_png,
                 getWidth() - 87, getHeight() - 88, 18, 12,
                 0, 0, cachedImage_innerBoxBr_png.getWidth(), cachedImage_innerBoxBr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxInner_png,
                 416, 382, getWidth() - 487, getHeight() - 469,
                 0, 0, cachedImage_innerBoxInner_png.getWidth(), cachedImage_innerBoxInner_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxTl_png,
                 53, 373, 19, 10,
                 0, 0, cachedImage_innerBoxTl_png.getWidth(), cachedImage_innerBoxTl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxTr_png,
                 getWidth() - 88, 373, 19, 10,
                 0, 0, cachedImage_innerBoxTr_png.getWidth(), cachedImage_innerBoxTr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxBl_png,
                 54, getHeight() - 87, 19, 11,
                 0, 0, cachedImage_innerBoxBl_png.getWidth(), cachedImage_innerBoxBl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxT_png,
                 71, 373, getWidth() - 159, 9,
                 0, 0, cachedImage_innerBoxT_png.getWidth(), cachedImage_innerBoxT_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxB_png,
                 70, getHeight() - 88, getWidth() - 156, 12,
                 0, 0, cachedImage_innerBoxB_png.getWidth(), cachedImage_innerBoxB_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxL_png,
                 53, 382, 13, getHeight() - 466,
                 0, 0, cachedImage_innerBoxL_png.getWidth(), cachedImage_innerBoxL_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_innerBoxR_png,
                 getWidth() - 434, 382, 351, getHeight() - 470,
                 0, 0, cachedImage_innerBoxR_png.getWidth(), cachedImage_innerBoxR_png.getHeight());

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void JucerScanCompleteComponent::resized()
{
    label->setBounds ((getWidth() / 2) - ((280) / 2), 96, 280, 24);
    textButton->setBounds ((getWidth() / 2) - ((80) / 2), 545, 80, 24);
    label2->setBounds ((getWidth() / 2) + -62 - ((280) / 2), 138, 280, 24);
    label3->setBounds ((getWidth() / 2) + -62 - ((280) / 2), 342, 280, 24);
    failed_list->setBounds (72, 186, 345, 117);
    good_list->setBounds (70, 393, 345, 117);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void JucerScanCompleteComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == textButton)
    {
        //[UserButtonCode_textButton] -- add your button handler code here..
        //[/UserButtonCode_textButton]
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

<JUCER_COMPONENT documentType="Component" className="JucerScanCompleteComponent"
                 componentName="" parentClasses="public Component" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="0" snapShown="1"
                 overlayOpacity="0.330000013" fixedSize="1" initialWidth="500"
                 initialHeight="600">
  <BACKGROUND backgroundColour="ff000000">
    <IMAGE pos="25 100 53M 125M" resource="backgroundBoxInner_png" opacity="1"
           mode="0"/>
    <IMAGE pos="23 85 45M 15" resource="backgroundBoxT_png" opacity="1"
           mode="0"/>
    <IMAGE pos="11 99 15 123M" resource="backgroundBoxL_png" opacity="1"
           mode="0"/>
    <IMAGE pos="29R 99 15 123M" resource="backgroundBoxR_png" opacity="1"
           mode="0"/>
    <IMAGE pos="11 85 30 15" resource="backgroundBoxTl_png" opacity="1"
           mode="0"/>
    <IMAGE pos="44R 85 30 15" resource="backgroundBoxTr_png" opacity="1"
           mode="0"/>
    <IMAGE pos="35 30R 75M 15" resource="backgroundBoxB_png" opacity="1"
           mode="0"/>
    <IMAGE pos="44R 30R 30 15" resource="backgroundBoxBr_png" opacity="1"
           mode="0"/>
    <IMAGE pos="11 30R 30 15" resource="backgroundBoxBl_png" opacity="1"
           mode="0"/>
    <IMAGE pos="178R 13 146 60" resource="eigenD_png" opacity="1" mode="0"/>
    <GROUP>
      <IMAGE pos="84R 295R 18 12" resource="innerBoxBr_png" opacity="1" mode="0"/>
      <IMAGE pos="419 180 487M 474M" resource="innerBoxInner_png" opacity="1"
             mode="0"/>
      <IMAGE pos="52 171 19 10" resource="innerBoxTl_png" opacity="1" mode="0"/>
      <IMAGE pos="85R 171 19 10" resource="innerBoxTr_png" opacity="1" mode="0"/>
      <IMAGE pos="53 294R 19 11" resource="innerBoxBl_png" opacity="1" mode="0"/>
      <IMAGE pos="70 171 155M 9" resource="innerBoxT_png" opacity="1" mode="0"/>
      <IMAGE pos="69 295R 152M 12" resource="innerBoxB_png" opacity="1" mode="0"/>
      <IMAGE pos="52 180 13 471M" resource="innerBoxL_png" opacity="1" mode="0"/>
      <IMAGE pos="435R 180 355 475M" resource="innerBoxR_png" opacity="1"
             mode="0"/>
    </GROUP>
    <GROUP>
      <IMAGE pos="87R 88R 18 12" resource="innerBoxBr_png" opacity="1" mode="0"/>
      <IMAGE pos="416 382 487M 469M" resource="innerBoxInner_png" opacity="1"
             mode="0"/>
      <IMAGE pos="53 373 19 10" resource="innerBoxTl_png" opacity="1" mode="0"/>
      <IMAGE pos="88R 373 19 10" resource="innerBoxTr_png" opacity="1" mode="0"/>
      <IMAGE pos="54 87R 19 11" resource="innerBoxBl_png" opacity="1" mode="0"/>
      <IMAGE pos="71 373 159M 9" resource="innerBoxT_png" opacity="1" mode="0"/>
      <IMAGE pos="70 88R 156M 12" resource="innerBoxB_png" opacity="1" mode="0"/>
      <IMAGE pos="53 382 13 466M" resource="innerBoxL_png" opacity="1" mode="0"/>
      <IMAGE pos="434R 382 351 470M" resource="innerBoxR_png" opacity="1"
             mode="0"/>
    </GROUP>
  </BACKGROUND>
  <LABEL name="new label" id="799f376af6e14dac" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0Cc 96 280 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Scan complete." editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="1" italic="0" justification="36"/>
  <TEXTBUTTON name="new button" id="b46826541bb0d6b1" memberName="textButton"
              virtualName="" explicitFocusOrder="0" pos="0Cc 545 80 24" bgColOff="ffaeaeae"
              buttonText="OK" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="new label" id="b4977d7be0d87e9" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="-62Cc 138 280 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Failed Plugins:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="1" italic="0" justification="33"/>
  <LABEL name="new label" id="c33444c41a86dfe7" memberName="label3" virtualName=""
         explicitFocusOrder="0" pos="-62Cc 342 280 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Good Plugins:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="1" italic="0" justification="33"/>
  <GENERICCOMPONENT name="new component" id="35f780889a178815" memberName="failed_list"
                    virtualName="" explicitFocusOrder="0" pos="72 186 345 117" class="ListBox"
                    params=""/>
  <GENERICCOMPONENT name="new component" id="701455ad497cebb4" memberName="good_list"
                    virtualName="" explicitFocusOrder="0" pos="70 393 345 117" class="ListBox"
                    params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: backgroundBoxB_png, 316, "../../app_eigend2/jucer/BackgroundBoxB.png"
static const unsigned char resource_JucerScanCompleteComponent_backgroundBoxB_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,
0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,
46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,
48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,75,73,68,65,84,120,156,236,203,177,
13,0,16,0,4,192,223,155,154,25,132,41,152,66,12,33,6,16,17,3,188,29,190,147,40,174,60,156,189,169,194,207,207,228,53,39,85,24,189,83,133,86,43,85,40,57,83,133,20,35,85,240,206,81,5,107,76,80,93,0,0,0,
255,255,3,0,110,207,10,168,241,237,162,147,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::backgroundBoxB_png = (const char*) resource_JucerScanCompleteComponent_backgroundBoxB_png;
const int JucerScanCompleteComponent::backgroundBoxB_pngSize = 316;

// JUCER_RESOURCE: backgroundBoxBl_png, 696, "../../app_eigend2/jucer/BackgroundBoxBL.png"
static const unsigned char resource_JucerScanCompleteComponent_backgroundBoxBl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,15,8,6,0,0,0,243,186,127,156,0,0,0,9,112,72,89,115,
0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,
46,46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,
51,48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,199,73,68,65,84,120,156,188,212,
223,47,66,97,28,199,241,110,253,31,254,140,108,44,178,210,230,199,24,99,93,101,174,200,148,77,68,145,112,37,195,108,78,233,151,173,152,101,200,102,205,214,112,17,253,226,194,48,75,105,205,12,37,21,155,
205,213,199,57,207,202,142,184,81,234,226,125,253,250,62,231,251,60,135,83,197,229,42,232,144,171,173,181,21,171,86,43,110,195,97,188,101,50,37,139,243,27,108,181,88,202,15,183,52,55,195,108,50,33,114,
115,83,94,88,36,18,65,79,81,8,93,95,227,53,157,46,31,92,203,227,97,97,126,30,23,231,231,72,191,188,148,20,150,176,97,38,237,228,36,252,94,47,18,79,79,37,59,53,3,215,229,195,253,82,41,92,123,123,136,69,
163,72,38,18,37,193,25,184,130,238,157,13,55,53,54,146,61,159,6,2,184,139,197,240,28,143,35,147,74,253,235,0,28,128,224,182,252,83,203,6,6,224,216,216,64,208,239,39,55,252,225,254,30,241,199,71,242,5,
82,201,100,209,229,224,74,186,15,54,92,207,231,99,92,173,198,154,205,134,3,183,27,103,193,32,174,46,47,17,14,133,16,141,68,200,26,138,137,192,89,92,145,127,106,230,103,162,26,27,131,65,175,199,166,195,
129,125,151,11,71,135,135,56,246,120,224,59,57,33,23,144,41,224,243,253,185,47,56,139,155,243,241,174,206,78,140,12,15,99,78,167,131,201,104,196,186,221,78,134,216,222,218,130,115,103,7,187,78,103,65,
125,131,179,184,138,253,217,107,170,171,33,22,139,33,151,201,160,213,104,160,155,157,197,210,226,34,168,229,101,114,1,87,12,134,130,250,1,179,118,110,203,221,118,161,80,72,240,190,222,94,40,134,134,200,
238,153,33,166,180,90,204,76,79,23,212,175,48,107,0,230,169,53,8,4,130,158,142,246,246,137,110,137,132,162,223,56,53,40,151,83,244,0,20,189,2,106,84,169,44,168,79,0,0,0,255,255,3,0,185,224,111,32,20,80,
120,234,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::backgroundBoxBl_png = (const char*) resource_JucerScanCompleteComponent_backgroundBoxBl_png;
const int JucerScanCompleteComponent::backgroundBoxBl_pngSize = 696;

// JUCER_RESOURCE: backgroundBoxBr_png, 646, "../../app_eigend2/jucer/BackgroundBoxBR.png"
static const unsigned char resource_JucerScanCompleteComponent_backgroundBoxBr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,15,8,6,0,0,0,243,186,127,156,0,0,0,9,112,72,89,115,
0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,
46,46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,
51,48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,149,73,68,65,84,120,156,228,210,
205,74,2,97,20,198,241,23,186,134,46,160,85,16,116,85,173,186,6,205,48,240,35,204,176,149,141,102,6,217,66,90,105,154,180,144,48,204,80,147,48,177,4,9,27,45,36,241,123,212,81,17,158,206,76,14,218,215,
102,194,90,180,248,51,203,223,57,103,94,38,180,90,152,101,157,118,27,189,78,7,253,94,15,131,126,31,195,193,0,195,225,16,236,79,96,234,119,224,110,247,63,193,130,240,21,60,154,57,220,253,0,143,241,242,
76,97,233,204,223,192,241,217,195,244,162,197,49,220,23,69,5,215,176,70,173,134,159,214,172,215,209,106,52,208,110,54,223,161,210,255,85,96,81,129,69,113,68,240,34,227,11,5,168,169,248,248,40,87,226,121,
60,21,139,120,46,149,240,82,46,163,94,173,126,130,123,10,252,134,187,41,198,146,241,56,212,118,157,72,32,149,76,226,38,149,66,38,157,70,238,254,94,30,74,198,37,88,249,199,147,173,121,106,158,6,96,236,
196,239,135,218,130,129,0,78,131,65,156,133,66,56,15,135,113,21,139,201,3,72,87,152,126,92,227,173,107,132,47,19,204,164,216,158,211,9,181,237,187,92,56,112,187,113,228,241,224,216,235,149,7,185,136,68,
144,205,100,80,173,84,38,231,22,132,91,218,122,137,112,166,196,76,70,35,212,100,54,153,176,105,54,99,203,98,193,142,205,134,93,187,29,158,195,67,248,125,62,92,70,163,120,200,231,165,173,115,132,175,16,
60,71,49,218,156,209,0,242,151,105,53,26,78,77,107,90,45,183,174,211,113,27,122,61,103,52,24,184,109,171,149,115,58,28,28,109,190,74,103,95,185,203,102,23,232,197,51,130,229,100,120,170,87,0,0,0,255,255,
3,0,97,155,52,159,169,173,107,216,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::backgroundBoxBr_png = (const char*) resource_JucerScanCompleteComponent_backgroundBoxBr_png;
const int JucerScanCompleteComponent::backgroundBoxBr_pngSize = 646;

// JUCER_RESOURCE: backgroundBoxInner_png, 503, "../../app_eigend2/jucer/BackgroundBoxInner.png"
static const unsigned char resource_JucerScanCompleteComponent_backgroundBoxInner_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,100,0,0,0,100,8,6,0,0,0,112,226,149,84,0,0,0,9,112,72,89,
115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,
32,46,46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,
56,51,48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,6,73,68,65,84,120,156,236,209,
49,13,0,48,12,192,176,242,103,91,13,192,6,99,57,124,248,143,148,57,187,151,142,249,29,128,33,105,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,
99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,
196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,
33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,99,72,140,33,49,134,196,24,18,
99,72,140,33,49,134,196,60,0,0,0,255,255,3,0,183,209,145,252,233,144,222,223,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::backgroundBoxInner_png = (const char*) resource_JucerScanCompleteComponent_backgroundBoxInner_png;
const int JucerScanCompleteComponent::backgroundBoxInner_pngSize = 503;

// JUCER_RESOURCE: backgroundBoxL_png, 292, "../../app_eigend2/jucer/BackgroundBoxL.png"
static const unsigned char resource_JucerScanCompleteComponent_backgroundBoxL_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,
0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,
46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,
48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,51,73,68,65,84,120,156,98,176,181,
177,169,0,226,255,48,28,22,26,250,127,209,194,133,255,239,221,185,243,255,243,199,143,120,49,195,168,230,81,205,163,154,169,172,25,0,0,0,255,255,3,0,173,65,168,191,39,5,63,71,0,0,0,0,73,69,78,68,174,66,
96,130,0,0};

const char* JucerScanCompleteComponent::backgroundBoxL_png = (const char*) resource_JucerScanCompleteComponent_backgroundBoxL_png;
const int JucerScanCompleteComponent::backgroundBoxL_pngSize = 292;

// JUCER_RESOURCE: backgroundBoxR_png, 296, "../../app_eigend2/jucer/BackgroundBoxR.png"
static const unsigned char resource_JucerScanCompleteComponent_backgroundBoxR_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,
0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,
46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,
48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,55,73,68,65,84,120,156,236,203,161,
17,0,48,8,4,65,250,239,22,194,48,240,230,82,66,76,36,98,229,218,113,231,37,35,168,76,186,138,233,70,51,72,194,54,111,222,252,57,95,0,0,0,255,255,3,0,22,184,99,19,252,156,58,252,0,0,0,0,73,69,78,68,174,
66,96,130,0,0};

const char* JucerScanCompleteComponent::backgroundBoxR_png = (const char*) resource_JucerScanCompleteComponent_backgroundBoxR_png;
const int JucerScanCompleteComponent::backgroundBoxR_pngSize = 296;

// JUCER_RESOURCE: backgroundBoxT_png, 279, "../../app_eigend2/jucer/BackgroundBoxT.png"
static const unsigned char resource_JucerScanCompleteComponent_backgroundBoxT_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,
0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,
46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,
48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,38,73,68,65,84,120,156,98,248,242,
233,211,127,114,49,195,72,212,252,249,227,199,255,228,226,81,205,163,154,9,98,0,0,0,0,255,255,3,0,54,84,94,99,248,244,114,238,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::backgroundBoxT_png = (const char*) resource_JucerScanCompleteComponent_backgroundBoxT_png;
const int JucerScanCompleteComponent::backgroundBoxT_pngSize = 279;

// JUCER_RESOURCE: backgroundBoxTl_png, 612, "../../app_eigend2/jucer/BackgroundBoxTL.png"
static const unsigned char resource_JucerScanCompleteComponent_backgroundBoxTl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,15,8,6,0,0,0,243,186,127,156,0,0,0,9,112,72,89,115,
0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,
46,46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,
51,48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,115,73,68,65,84,120,156,188,212,
63,75,66,81,24,199,113,167,222,75,239,193,2,115,115,49,111,218,96,163,13,246,6,12,3,91,108,176,193,6,205,193,12,211,48,105,205,150,180,180,192,64,107,243,31,114,77,18,73,196,52,255,92,43,90,170,95,231,
220,80,142,232,230,209,225,59,127,56,207,195,115,20,0,20,147,58,112,58,213,103,161,80,226,54,30,151,10,249,252,79,163,94,199,187,36,253,215,239,227,99,202,198,64,65,167,91,220,52,153,196,93,155,13,199,
62,31,174,163,81,16,24,175,141,198,16,158,22,29,131,151,148,74,129,244,189,110,48,96,219,98,145,225,187,68,2,79,162,136,110,187,205,13,29,129,9,184,76,250,37,97,85,171,197,142,213,10,50,106,60,164,82,
120,169,86,185,190,118,8,19,108,129,244,73,81,26,25,55,246,236,118,92,70,34,40,228,114,35,99,230,186,99,130,5,6,40,109,195,104,132,199,237,198,125,50,137,231,114,25,111,205,230,40,204,161,1,252,197,194,
91,102,51,206,195,97,228,50,25,212,107,53,180,91,45,244,58,29,244,123,61,57,46,48,129,212,44,74,163,251,189,137,197,80,173,84,100,84,234,118,185,129,44,236,97,209,21,149,10,251,14,7,30,211,105,52,201,
110,41,202,123,204,3,248,130,133,53,26,13,14,93,46,228,179,89,249,132,120,191,148,133,175,88,152,158,210,145,215,11,177,88,156,25,58,17,94,19,4,156,248,253,40,151,74,51,67,39,194,6,189,30,193,64,64,62,
163,185,195,167,193,224,252,97,250,79,207,3,254,3,0,0,255,255,3,0,111,74,2,27,78,221,106,98,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::backgroundBoxTl_png = (const char*) resource_JucerScanCompleteComponent_backgroundBoxTl_png;
const int JucerScanCompleteComponent::backgroundBoxTl_pngSize = 612;

// JUCER_RESOURCE: backgroundBoxTr_png, 456, "../../app_eigend2/jucer/BackgroundBoxTR.png"
static const unsigned char resource_JucerScanCompleteComponent_backgroundBoxTr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,30,0,0,0,15,8,6,0,0,0,243,186,127,156,0,0,0,9,112,72,89,115,
0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,
46,46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,
51,48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,215,73,68,65,84,120,156,228,211,
93,10,194,48,12,192,241,128,224,85,60,143,79,130,199,240,24,222,99,135,17,188,129,224,29,182,38,253,136,66,76,102,59,177,236,177,5,193,135,255,211,96,191,165,233,0,167,73,154,229,156,144,115,79,66,188,
122,162,33,16,237,131,247,27,13,162,22,114,250,12,160,37,76,111,88,60,162,232,203,69,17,81,240,18,67,216,105,80,50,188,41,140,107,112,8,150,79,49,158,52,176,12,239,2,83,5,39,43,70,235,92,240,182,112,53,
181,238,216,142,122,129,57,37,235,160,1,184,113,148,86,213,251,158,241,50,245,7,190,105,219,166,112,253,1,51,110,83,127,195,194,204,199,46,240,92,198,203,212,21,60,244,131,243,228,54,245,114,220,25,126,
48,223,187,194,214,12,87,199,173,112,234,14,227,218,158,153,165,63,108,123,46,191,213,255,192,122,179,127,6,126,1,0,0,255,255,3,0,44,184,173,26,174,253,155,180,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::backgroundBoxTr_png = (const char*) resource_JucerScanCompleteComponent_backgroundBoxTr_png;
const int JucerScanCompleteComponent::backgroundBoxTr_pngSize = 456;

// JUCER_RESOURCE: innerBoxB_png, 292, "../../app_eigend2/jucer/InnerBoxB.png"
static const unsigned char resource_JucerScanCompleteComponent_innerBoxB_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,51,73,68,65,84,120,156,98,248,254,253,
251,127,114,49,195,168,230,81,205,131,88,243,151,47,95,254,147,139,25,222,191,127,255,159,92,204,240,252,249,243,255,228,98,0,0,0,0,255,255,3,0,150,147,102,211,150,251,224,206,0,0,0,0,73,69,78,68,174,
66,96,130,0,0};

const char* JucerScanCompleteComponent::innerBoxB_png = (const char*) resource_JucerScanCompleteComponent_innerBoxB_png;
const int JucerScanCompleteComponent::innerBoxB_pngSize = 292;

// JUCER_RESOURCE: innerBoxBl_png, 548, "../../app_eigend2/jucer/InnerBoxBL.png"
static const unsigned char resource_JucerScanCompleteComponent_innerBoxBl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,51,73,68,65,84,120,156,212,146,91,106,
131,80,24,132,133,64,183,210,13,21,10,221,74,215,208,101,20,186,10,9,81,9,222,18,147,96,20,77,32,94,80,68,77,188,37,161,79,211,243,31,208,151,180,152,60,246,97,30,191,57,51,243,31,65,81,20,44,151,75,236,
118,59,164,105,138,211,233,132,174,235,112,189,94,71,37,252,83,88,150,101,44,22,139,1,62,30,143,15,193,21,193,190,239,35,73,146,199,96,73,146,92,211,52,225,121,30,135,203,178,68,219,182,184,92,46,227,
240,108,54,251,212,117,29,174,235,34,142,99,20,69,129,166,105,238,131,167,211,233,155,170,170,176,109,27,65,16,32,203,50,212,117,141,243,249,60,14,139,162,248,52,159,207,253,213,106,133,253,126,63,140,
118,79,116,1,224,189,95,13,195,128,227,56,8,195,144,191,94,85,21,31,142,12,254,50,225,48,137,189,254,97,89,22,95,157,186,231,121,206,13,40,1,85,248,205,100,128,217,112,19,214,253,125,179,217,124,211,205,
201,128,18,80,5,218,128,70,36,35,74,211,107,128,123,105,154,246,188,94,175,85,58,29,13,72,231,35,19,186,2,157,145,204,72,244,19,111,96,214,95,96,9,38,172,194,203,118,187,253,98,41,140,195,225,128,40,138,
184,17,13,218,235,7,0,0,255,255,3,0,117,78,55,159,207,157,119,196,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::innerBoxBl_png = (const char*) resource_JucerScanCompleteComponent_innerBoxBl_png;
const int JucerScanCompleteComponent::innerBoxBl_pngSize = 548;

// JUCER_RESOURCE: innerBoxBr_png, 448, "../../app_eigend2/jucer/InnerBoxBR.png"
static const unsigned char resource_JucerScanCompleteComponent_innerBoxBr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,207,73,68,65,84,120,156,228,210,59,10,
194,64,16,6,224,129,128,135,241,60,130,224,85,60,131,199,176,242,24,169,22,44,82,133,20,65,2,9,89,242,126,108,146,197,106,220,95,72,42,35,174,150,22,127,249,237,60,118,72,107,205,182,25,134,129,155,166,
97,250,51,92,215,245,79,88,89,227,105,154,88,41,197,85,85,5,95,225,190,239,185,44,203,179,53,30,199,145,187,174,227,60,207,15,86,120,110,217,204,123,147,82,110,62,194,64,8,22,213,182,45,103,89,182,103,
102,122,139,103,132,86,81,17,176,40,138,19,224,19,227,181,181,0,96,57,152,209,180,122,55,21,143,73,146,56,11,198,165,188,10,142,192,124,7,182,138,54,69,154,166,219,25,45,216,12,206,43,185,26,112,137,227,
120,23,69,145,19,134,33,249,190,79,158,231,145,16,130,92,215,165,7,0,0,0,255,255,3,0,114,215,77,107,154,3,251,42,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::innerBoxBr_png = (const char*) resource_JucerScanCompleteComponent_innerBoxBr_png;
const int JucerScanCompleteComponent::innerBoxBr_pngSize = 448;

// JUCER_RESOURCE: innerBoxInner_png, 272, "../../app_eigend2/jucer/InnerBoxInner.png"
static const unsigned char resource_JucerScanCompleteComponent_innerBoxInner_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,
11,19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,
46,122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,
48,48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,31,73,68,65,84,120,156,98,248,254,
253,251,127,114,49,195,168,230,81,205,163,154,169,172,25,0,0,0,255,255,3,0,191,225,107,146,12,78,209,219,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::innerBoxInner_png = (const char*) resource_JucerScanCompleteComponent_innerBoxInner_png;
const int JucerScanCompleteComponent::innerBoxInner_pngSize = 272;

// JUCER_RESOURCE: innerBoxL_png, 298, "../../app_eigend2/jucer/InnerBoxL.png"
static const unsigned char resource_JucerScanCompleteComponent_innerBoxL_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,57,73,68,65,84,120,156,98,56,122,244,
232,255,243,231,207,255,191,115,231,206,255,231,207,159,255,127,255,254,253,255,47,95,190,252,255,254,253,59,65,204,48,170,121,84,243,168,102,42,107,6,0,0,0,255,255,3,0,52,36,82,66,236,158,243,63,0,0,
0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::innerBoxL_png = (const char*) resource_JucerScanCompleteComponent_innerBoxL_png;
const int JucerScanCompleteComponent::innerBoxL_pngSize = 298;

// JUCER_RESOURCE: innerBoxR_png, 281, "../../app_eigend2/jucer/InnerBoxR.png"
static const unsigned char resource_JucerScanCompleteComponent_innerBoxR_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,40,73,68,65,84,120,156,98,248,254,253,
251,127,82,241,151,47,95,254,191,127,255,254,63,195,168,230,81,205,163,154,169,172,25,0,0,0,255,255,3,0,126,38,105,163,103,155,98,81,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::innerBoxR_png = (const char*) resource_JucerScanCompleteComponent_innerBoxR_png;
const int JucerScanCompleteComponent::innerBoxR_pngSize = 281;

// JUCER_RESOURCE: innerBoxT_png, 301, "../../app_eigend2/jucer/InnerBoxT.png"
static const unsigned char resource_JucerScanCompleteComponent_innerBoxT_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,60,73,68,65,84,120,156,236,203,161,13,
0,64,8,3,192,238,191,28,11,224,145,53,36,149,253,17,62,169,70,156,60,84,149,83,232,110,167,48,51,78,129,164,83,216,93,167,32,201,169,203,151,191,30,0,0,0,255,255,3,0,152,47,91,12,243,218,47,22,0,0,0,0,
73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::innerBoxT_png = (const char*) resource_JucerScanCompleteComponent_innerBoxT_png;
const int JucerScanCompleteComponent::innerBoxT_pngSize = 301;

// JUCER_RESOURCE: innerBoxTl_png, 607, "../../app_eigend2/jucer/InnerBoxTL.png"
static const unsigned char resource_JucerScanCompleteComponent_innerBoxTl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,110,73,68,65,84,120,156,188,146,73,110,
194,80,16,68,145,34,229,42,57,79,86,145,114,140,28,35,247,224,16,22,51,6,44,131,1,3,50,88,24,36,6,27,196,108,38,179,137,84,113,117,132,149,144,1,86,89,212,238,191,95,221,85,29,3,16,187,84,34,145,184,203,
100,50,143,249,124,62,94,42,149,12,93,215,223,42,149,10,170,213,42,106,181,154,168,94,175,127,7,83,169,212,67,46,151,211,53,77,3,1,211,52,97,89,22,108,219,70,183,219,133,227,56,232,245,122,162,75,240,
69,85,213,32,116,18,168,221,110,203,163,193,96,128,241,120,12,207,243,48,153,76,48,157,78,69,159,193,215,16,68,185,92,70,179,217,20,151,225,112,40,143,103,179,25,150,203,37,86,171,21,214,235,181,104,179,
217,124,128,201,100,242,41,220,15,116,108,181,90,226,230,186,174,64,124,232,251,62,118,187,29,246,251,61,14,135,67,164,152,162,40,247,97,56,46,119,228,168,220,137,224,98,177,16,136,192,241,120,68,16,4,
162,211,233,20,137,174,207,28,215,48,12,116,58,29,140,70,35,113,36,200,223,127,130,34,56,157,78,199,233,218,104,52,208,239,247,37,8,142,74,199,223,160,8,206,102,179,54,119,101,29,12,104,62,159,99,187,
221,202,168,127,129,2,135,65,249,236,147,61,114,87,166,202,112,174,185,10,204,125,121,57,172,134,181,176,142,91,70,22,184,80,40,200,185,49,101,194,220,151,65,93,3,191,192,236,246,28,214,205,112,177,88,
148,35,63,195,188,156,127,129,223,1,0,0,255,255,3,0,2,100,43,234,136,147,167,24,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::innerBoxTl_png = (const char*) resource_JucerScanCompleteComponent_innerBoxTl_png;
const int JucerScanCompleteComponent::innerBoxTl_pngSize = 607;

// JUCER_RESOURCE: innerBoxTr_png, 490, "../../app_eigend2/jucer/InnerBoxTR.png"
static const unsigned char resource_JucerScanCompleteComponent_innerBoxTr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,15,0,0,0,15,8,6,0,0,0,59,214,149,74,0,0,0,9,112,72,89,115,0,0,11,
19,0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,
122,37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,
48,48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,249,73,68,65,84,120,156,196,146,73,106,
132,80,20,69,133,64,109,37,235,201,40,80,203,200,50,178,143,90,140,24,155,137,34,42,133,189,40,246,45,129,192,45,239,31,132,196,84,133,88,147,12,206,240,220,127,223,123,95,210,117,29,91,52,77,251,88,121,
83,85,245,164,40,202,147,44,203,15,0,164,45,146,231,121,184,134,235,186,112,28,7,182,109,195,52,77,217,48,140,199,31,114,150,101,184,70,154,166,72,146,4,81,20,193,247,125,6,206,150,101,189,124,147,155,
166,193,150,186,174,5,85,85,161,44,75,228,121,46,194,130,32,96,155,215,79,121,28,71,220,98,24,6,244,125,143,182,109,69,24,67,226,56,102,139,103,33,47,203,130,91,204,243,44,152,166,73,4,49,164,40,10,6,
172,43,113,15,191,202,219,16,182,97,0,27,132,97,120,252,147,252,53,132,13,56,194,186,212,211,46,153,112,132,174,235,88,255,188,91,230,235,92,226,122,133,247,187,100,86,231,25,119,203,132,139,227,220,119,
203,252,76,255,39,95,0,0,0,255,255,3,0,114,14,69,91,18,254,252,79,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* JucerScanCompleteComponent::innerBoxTr_png = (const char*) resource_JucerScanCompleteComponent_innerBoxTr_png;
const int JucerScanCompleteComponent::innerBoxTr_pngSize = 490;

// JUCER_RESOURCE: eigenD_png, 3575, "eigenD@2x.png"
static const unsigned char resource_JucerScanCompleteComponent_eigenD_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,1,36,0,0,0,120,8,2,0,0,0,212,160,24,131,0,0,0,25,116,69,88,116,83,111,102,
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

const char* JucerScanCompleteComponent::eigenD_png = (const char*) resource_JucerScanCompleteComponent_eigenD_png;
const int JucerScanCompleteComponent::eigenD_pngSize = 3575;
