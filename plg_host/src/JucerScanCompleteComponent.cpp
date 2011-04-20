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
    addAndMakeVisible (label = new Label (T("new label"),
                                          T("Scan complete.")));
    label->setFont (Font (15.0000f, Font::bold));
    label->setJustificationType (Justification::centred);
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (textButton = new TextButton (T("new button")));
    textButton->setButtonText (T("OK"));
    textButton->addListener (this);
    textButton->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    addAndMakeVisible (label2 = new Label (T("new label"),
                                           T("Failed Plugins:")));
    label2->setFont (Font (15.0000f, Font::bold));
    label2->setJustificationType (Justification::centredLeft);
    label2->setEditable (false, false, false);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label3 = new Label (T("new label"),
                                           T("Good Plugins:")));
    label3->setFont (Font (15.0000f, Font::bold));
    label3->setJustificationType (Justification::centredLeft);
    label3->setEditable (false, false, false);
    label3->setColour (TextEditor::textColourId, Colours::black);
    label3->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (failed_list = new ListBox());
    failed_list->setName (T("new component"));

    addAndMakeVisible (good_list = new ListBox());
    good_list->setName (T("new component"));

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

// JUCER_RESOURCE: eigenD_png, 2157, "../../app_eigend2/jucer/eigenD.png"
static const unsigned char resource_JucerScanCompleteComponent_eigenD_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,146,0,0,0,60,8,6,0,0,0,141,109,227,16,0,0,0,4,115,66,73,84,8,8,8,8,124,
8,100,136,0,0,0,9,112,72,89,115,0,0,8,179,0,0,8,179,1,132,47,149,219,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,101,0,119,119,119,46,105,110,107,115,99,97,112,101,46,111,114,103,155,238,60,26,0,
0,7,234,73,68,65,84,120,156,237,156,123,140,85,197,29,199,63,187,92,132,214,133,173,44,212,168,224,11,88,132,224,3,181,42,138,154,216,136,241,17,109,131,36,106,53,77,109,107,19,219,24,181,177,73,99,26,
95,255,152,104,108,66,163,241,29,173,239,23,86,141,72,208,4,20,136,34,169,15,52,154,178,80,86,170,180,10,168,187,172,20,5,247,231,31,191,57,185,227,112,102,206,89,246,158,187,247,238,153,79,114,114,38,
51,191,51,51,119,207,247,252,230,55,115,230,108,139,136,16,25,246,140,1,250,128,194,110,118,107,81,21,71,26,138,31,161,98,42,140,40,164,114,208,14,140,45,178,129,40,164,230,103,116,14,155,177,168,152,
10,35,10,169,249,153,156,195,166,157,40,164,72,6,147,201,190,143,209,35,69,50,153,128,6,211,33,162,71,138,100,210,1,140,203,176,25,75,12,182,35,25,116,152,35,68,225,30,169,82,100,229,145,186,48,142,124,
30,169,165,200,78,68,33,53,62,35,128,111,3,229,121,61,82,161,163,79,28,218,26,159,163,8,175,21,197,24,41,146,139,233,192,148,64,121,67,196,72,81,72,141,79,167,57,124,228,245,72,49,216,46,57,157,192,246,
64,249,56,242,121,164,17,53,235,81,10,81,72,141,207,52,224,255,158,178,54,96,47,242,121,164,40,164,146,51,21,191,71,234,112,206,62,162,71,42,57,7,0,123,227,143,145,18,1,133,60,210,72,116,214,87,168,144,
98,176,221,216,36,2,26,15,236,147,82,158,199,35,37,65,246,72,224,7,53,234,215,110,68,33,53,54,211,60,233,132,68,64,161,24,200,94,63,42,108,230,22,133,212,216,116,122,210,9,201,144,214,130,127,120,179,
197,83,216,162,228,158,10,233,10,224,31,192,25,53,236,75,61,24,3,156,0,156,2,28,129,14,25,141,76,150,144,236,33,205,39,164,186,120,164,61,13,182,143,6,206,3,22,215,176,47,245,96,22,240,170,147,247,13,
240,95,224,117,224,206,148,242,161,100,131,39,157,208,225,73,219,180,123,210,53,101,79,133,180,0,245,72,239,212,176,47,245,32,249,28,103,57,250,27,198,161,79,250,169,192,5,230,248,0,184,17,120,98,40,58,
232,112,19,250,194,118,11,240,72,74,121,211,123,164,183,204,209,108,244,155,243,70,224,105,167,236,72,224,183,192,165,192,99,232,98,223,125,245,235,90,42,159,1,87,6,202,243,8,169,46,49,146,43,164,179,
129,147,208,69,176,117,192,18,96,105,202,117,71,162,195,196,50,160,59,80,255,24,83,223,4,244,135,182,3,255,6,30,78,177,29,9,204,3,142,3,38,162,158,193,231,245,126,134,110,47,253,59,42,142,153,192,89,230,
188,17,184,7,248,40,229,186,126,231,108,243,46,240,7,224,89,224,5,224,110,96,167,105,163,81,201,51,180,213,197,35,33,34,136,72,135,136,60,39,202,87,34,178,90,170,220,105,108,236,227,47,166,108,94,74,25,
34,50,75,68,94,16,145,29,178,59,139,82,236,59,173,54,183,136,200,123,38,189,83,68,174,73,177,127,223,148,207,49,237,184,108,19,145,153,41,215,157,96,202,31,240,244,59,57,230,154,190,239,18,145,9,25,182,
67,121,124,104,253,230,235,60,54,55,231,176,25,244,145,204,218,30,6,206,1,126,15,236,7,252,4,93,183,120,14,248,29,112,161,163,191,228,137,222,153,162,205,11,128,21,192,108,224,86,224,68,96,95,224,16,207,
53,163,129,151,128,67,129,249,192,254,192,225,168,103,122,11,184,25,245,126,54,73,29,203,129,81,192,249,168,215,155,14,220,143,14,75,55,248,30,158,28,44,1,22,161,107,51,115,6,81,79,209,44,246,164,109,
234,230,145,230,25,181,62,148,162,180,67,68,228,91,17,89,231,228,255,217,92,115,166,147,127,153,201,255,64,68,14,117,202,246,53,101,207,56,249,137,119,251,83,74,251,115,77,217,243,78,254,155,38,127,182,
167,207,34,34,221,41,101,121,61,18,34,114,149,177,189,173,168,167,184,6,71,69,68,126,46,58,2,248,108,30,145,42,247,89,249,199,138,200,37,34,242,195,90,244,165,2,92,108,52,181,16,221,141,231,178,26,56,
30,141,119,182,37,250,51,231,93,142,237,101,38,239,84,96,179,83,246,141,231,154,139,128,175,80,47,230,182,191,25,248,20,141,201,108,250,77,61,175,167,244,119,3,58,157,31,236,183,238,171,204,121,246,32,
235,41,146,93,104,76,23,162,205,74,239,109,206,199,162,247,21,52,182,116,71,156,1,83,1,102,152,244,194,12,219,41,192,219,78,158,189,151,120,18,112,12,42,8,87,68,80,21,159,253,31,49,246,50,245,86,128,149,
129,182,251,209,96,220,30,22,67,251,152,123,201,222,90,145,197,84,115,246,109,225,104,22,94,6,206,53,233,87,204,249,116,171,252,204,90,52,82,65,99,162,173,168,74,67,252,47,37,207,158,253,36,113,140,43,
182,16,227,77,31,86,161,177,85,136,144,112,6,106,159,231,139,138,100,213,126,221,0,219,109,52,238,0,54,161,35,194,139,38,175,207,42,239,219,237,138,42,251,160,49,226,203,192,142,80,35,21,116,218,59,7,
253,227,166,173,158,134,176,95,20,38,2,58,113,0,215,111,66,23,219,102,160,211,246,180,105,249,158,146,54,17,72,200,18,210,24,170,79,237,131,181,233,206,144,209,207,238,163,77,175,149,238,241,92,215,6,
172,71,197,180,146,140,73,71,43,240,134,73,255,98,0,157,75,110,132,45,164,255,0,31,163,175,79,210,254,177,129,239,230,189,129,222,184,179,7,208,126,158,119,132,33,81,134,174,31,143,174,157,141,7,158,34,
60,228,54,43,61,158,180,205,105,84,183,174,156,68,198,98,102,43,112,27,58,180,221,72,53,240,206,34,89,200,116,111,200,13,168,96,86,160,75,8,54,163,60,117,93,143,122,143,199,77,135,243,208,74,120,163,214,
8,194,94,199,87,54,19,93,82,56,6,120,136,26,4,161,13,74,30,143,180,205,74,11,58,33,242,210,138,206,112,126,5,124,13,60,0,60,10,204,69,189,74,27,170,68,119,67,148,239,137,190,23,248,35,186,110,180,28,117,
169,191,70,213,125,184,177,113,5,240,79,224,26,116,61,105,9,112,59,112,50,112,16,58,203,104,71,131,114,183,253,208,235,157,10,26,156,187,180,90,231,10,234,117,142,3,46,71,227,128,53,192,97,192,223,128,
95,50,240,184,172,89,200,227,145,182,90,233,94,178,254,22,214,90,192,193,34,242,148,164,115,137,179,110,112,189,201,63,199,179,174,48,85,68,254,42,34,95,164,212,229,174,35,37,199,44,17,121,205,211,254,
209,142,237,26,147,63,210,83,87,183,136,244,164,228,159,226,169,95,140,253,2,17,153,238,169,115,56,29,83,172,223,125,151,199,230,0,203,102,189,199,102,190,136,220,34,34,157,246,83,221,141,174,44,255,24,
125,42,167,1,7,26,37,174,112,244,183,204,156,187,60,250,236,2,174,2,174,54,245,29,132,238,1,186,5,248,151,231,154,183,141,205,68,171,253,253,81,79,233,94,115,23,186,146,237,27,162,22,144,190,173,244,99,
116,40,31,129,78,235,55,1,159,152,227,125,50,220,247,48,34,207,208,102,123,164,207,83,202,143,2,158,52,233,147,235,249,20,92,109,212,125,110,3,60,145,101,63,70,73,149,107,3,118,125,198,102,113,74,217,
124,171,142,207,235,181,213,118,34,186,61,67,168,206,18,35,67,199,215,84,223,52,248,60,18,84,189,82,154,71,90,107,167,139,22,82,27,186,172,240,46,250,66,245,55,232,30,155,200,208,211,227,156,211,8,9,169,
139,234,91,138,181,181,254,174,173,5,93,123,105,71,99,152,153,104,60,178,25,125,167,214,8,187,14,35,74,47,122,143,190,12,216,108,117,206,54,219,209,152,115,18,5,8,73,208,197,197,126,212,11,45,66,167,244,
203,25,190,83,233,102,101,176,30,9,116,120,155,4,116,21,241,165,109,179,125,89,82,86,6,34,164,52,143,4,58,155,254,41,117,136,145,34,141,75,178,4,48,88,143,4,208,21,133,84,94,106,229,145,54,1,125,81,72,
229,37,17,80,111,192,38,143,71,234,130,248,201,118,153,233,65,247,34,133,38,65,89,66,234,6,222,131,40,164,50,211,67,120,88,3,21,146,0,95,120,202,251,209,153,121,20,82,137,201,35,164,45,232,58,83,104,111,
215,82,136,66,42,51,121,61,146,47,208,78,216,1,81,72,101,38,143,144,122,201,249,74,43,10,169,188,228,17,146,224,223,42,244,61,162,144,202,75,30,33,65,20,82,36,131,30,194,47,108,19,214,102,155,68,33,149,
153,47,137,30,41,82,3,182,147,61,35,131,40,164,72,14,54,230,176,201,181,143,61,10,169,220,228,17,82,46,90,68,36,219,42,50,92,177,255,195,204,160,248,14,56,24,144,199,239,47,222,31,0,0,0,0,73,69,78,68,
174,66,96,130,0,0};

const char* JucerScanCompleteComponent::eigenD_png = (const char*) resource_JucerScanCompleteComponent_eigenD_png;
const int JucerScanCompleteComponent::eigenD_pngSize = 2157;
