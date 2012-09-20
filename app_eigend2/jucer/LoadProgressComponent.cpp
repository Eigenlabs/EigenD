/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  28 Jan 2011 3:52:00pm

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

#include "LoadProgressComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
LoadProgressComponent::LoadProgressComponent ()
    : progress_slider (0),
      message_label (0),
      cachedImage_textBoxTl_png (0),
      cachedImage_textBoxTr_png (0),
      cachedImage_textBoxBr_png (0),
      cachedImage_textBoxBl_png (0),
      cachedImage_textBoxL_png (0),
      cachedImage_textBoxR_png (0),
      cachedImage_textBoxB_png (0),
      cachedImage_textBoxT_png (0),
      cachedImage_textBoxInner_png (0)
{
    addAndMakeVisible (progress_slider = new Slider ("new slider"));
    progress_slider->setRange (0, 100, 1);
    progress_slider->setSliderStyle (Slider::LinearBar);
    progress_slider->setTextBoxStyle (Slider::TextBoxLeft, true, 80, 12);
    progress_slider->setColour (Slider::thumbColourId, Colour (0xff8a8a8a));
    progress_slider->addListener (this);

    addAndMakeVisible (message_label = new Label ("message_label",
                                                  "label text"));
    message_label->setFont (Font (15.0000f, Font::plain));
    message_label->setJustificationType (Justification::centredBottom);
    message_label->setEditable (false, false, false);
    message_label->setColour (TextEditor::textColourId, Colours::black);
    message_label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    cachedImage_textBoxTl_png = ImageCache::getFromMemory (textBoxTl_png, textBoxTl_pngSize);
    cachedImage_textBoxTr_png = ImageCache::getFromMemory (textBoxTr_png, textBoxTr_pngSize);
    cachedImage_textBoxBr_png = ImageCache::getFromMemory (textBoxBr_png, textBoxBr_pngSize);
    cachedImage_textBoxBl_png = ImageCache::getFromMemory (textBoxBl_png, textBoxBl_pngSize);
    cachedImage_textBoxL_png = ImageCache::getFromMemory (textBoxL_png, textBoxL_pngSize);
    cachedImage_textBoxR_png = ImageCache::getFromMemory (textBoxR_png, textBoxR_pngSize);
    cachedImage_textBoxB_png = ImageCache::getFromMemory (textBoxB_png, textBoxB_pngSize);
    cachedImage_textBoxT_png = ImageCache::getFromMemory (textBoxT_png, textBoxT_pngSize);
    cachedImage_textBoxInner_png = ImageCache::getFromMemory (textBoxInner_png, textBoxInner_pngSize);

    //[UserPreSize]
    //[/UserPreSize]

    setSize (400, 60);

    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

LoadProgressComponent::~LoadProgressComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (progress_slider);
    deleteAndZero (message_label);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void LoadProgressComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colours::black,
                                       (float) ((getWidth() / 2)), 0.0f,
                                       Colour (0xff6c6c6c),
                                       (float) ((getWidth() / 2)), (float) (getHeight()),
                                       false));
    g.fillRect (-3, -2, 403, 106);

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxTl_png,
                 4, 4, 12, 12,
                 0, 0, cachedImage_textBoxTl_png.getWidth(), cachedImage_textBoxTl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxTr_png,
                 getWidth() - 16, 4, 12, 12,
                 0, 0, cachedImage_textBoxTr_png.getWidth(), cachedImage_textBoxTr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxBr_png,
                 getWidth() - 16, getHeight() - 16, 12, 12,
                 0, 0, cachedImage_textBoxBr_png.getWidth(), cachedImage_textBoxBr_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxBl_png,
                 4, getHeight() - 16, 12, 12,
                 0, 0, cachedImage_textBoxBl_png.getWidth(), cachedImage_textBoxBl_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxL_png,
                 4, 16, getWidth() - 388, 28,
                 0, 0, cachedImage_textBoxL_png.getWidth(), cachedImage_textBoxL_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxR_png,
                 getWidth() - 16, 16, 12, getHeight() - 32,
                 0, 0, cachedImage_textBoxR_png.getWidth(), cachedImage_textBoxR_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxB_png,
                 16, getHeight() - 16, getWidth() - 32, 12,
                 0, 0, cachedImage_textBoxB_png.getWidth(), cachedImage_textBoxB_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxT_png,
                 16, 4, getWidth() - 32, 12,
                 0, 0, cachedImage_textBoxT_png.getWidth(), cachedImage_textBoxT_png.getHeight());

    g.setColour (Colours::black);
    g.drawImage (cachedImage_textBoxInner_png,
                 16, 16, 368, 28,
                 0, 0, cachedImage_textBoxInner_png.getWidth(), cachedImage_textBoxInner_png.getHeight());

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void LoadProgressComponent::resized()
{
    progress_slider->setBounds (32, 28, 336, 19);
    message_label->setBounds (32, 9, 336, 15);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void LoadProgressComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == progress_slider)
    {
        //[UserSliderCode_progress_slider] -- add your slider handling code here..
        //[/UserSliderCode_progress_slider]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="LoadProgressComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="3" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="400" initialHeight="60">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="-3 -2 403 106" fill="linear: 0C 0, 0C 0R, 0=ff000000, 1=ff6c6c6c"
          hasStroke="0"/>
    <IMAGE pos="4 4 12 12" resource="textBoxTl_png" opacity="1" mode="0"/>
    <IMAGE pos="16R 4 12 12" resource="textBoxTr_png" opacity="1" mode="0"/>
    <IMAGE pos="16R 16R 12 12" resource="textBoxBr_png" opacity="1" mode="0"/>
    <IMAGE pos="4 16R 12 12" resource="textBoxBl_png" opacity="1" mode="0"/>
    <IMAGE pos="4 16 388M 28" resource="textBoxL_png" opacity="1" mode="0"/>
    <IMAGE pos="16R 16 12 32M" resource="textBoxR_png" opacity="1" mode="0"/>
    <IMAGE pos="16 16R 32M 12" resource="textBoxB_png" opacity="1" mode="0"/>
    <IMAGE pos="16 4 32M 12" resource="textBoxT_png" opacity="1" mode="0"/>
    <IMAGE pos="16 16 368 28" resource="textBoxInner_png" opacity="1" mode="0"/>
  </BACKGROUND>
  <SLIDER name="new slider" id="9b89265549376ef5" memberName="progress_slider"
          virtualName="" explicitFocusOrder="0" pos="32 28 336 19" thumbcol="ff8a8a8a"
          min="0" max="100" int="1" style="LinearBar" textBoxPos="TextBoxLeft"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="12" skewFactor="1"/>
  <LABEL name="message_label" id="2c350388f0cc1ad6" memberName="message_label"
         virtualName="" explicitFocusOrder="0" pos="32 9 336 15" edTextCol="ff000000"
         edBkgCol="0" labelText="label text" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="20"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: textBoxTl_png, 517, "TextBoxTL.png"
static const unsigned char resource_LoadProgressComponent_textBoxTl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,
0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,20,73,68,65,84,120,156,140,145,63,107,131,80,
20,197,133,66,191,74,63,79,21,44,116,112,83,247,6,132,64,4,51,5,250,17,28,116,49,163,8,46,14,226,224,88,232,34,56,212,193,77,76,54,171,66,193,63,241,52,247,65,2,77,45,246,192,157,222,249,93,238,59,135,
3,192,241,60,127,59,119,231,121,20,4,97,175,40,202,251,106,181,26,215,235,53,116,93,7,55,3,60,72,146,244,182,221,110,97,154,38,92,215,69,16,4,8,195,16,81,20,253,2,94,52,77,251,178,109,155,61,38,73,130,
60,207,81,20,5,14,135,3,142,199,227,15,224,213,48,12,120,158,199,140,101,89,162,105,26,116,93,135,113,28,49,77,19,72,23,224,105,179,217,192,247,125,100,89,134,170,170,152,241,116,58,225,86,100,190,151,
101,185,112,28,7,105,154,162,174,107,12,195,112,221,56,7,60,239,118,59,196,113,204,110,236,251,126,214,120,5,68,81,220,91,150,197,182,183,109,251,231,230,43,160,170,234,7,69,71,105,208,221,75,226,206,
165,124,82,206,20,29,165,177,8,80,131,84,10,229,188,116,14,3,40,78,42,137,62,252,31,125,3,0,0,255,255,3,0,206,234,209,136,115,191,70,54,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* LoadProgressComponent::textBoxTl_png = (const char*) resource_LoadProgressComponent_textBoxTl_png;
const int LoadProgressComponent::textBoxTl_pngSize = 517;

// JUCER_RESOURCE: textBoxTr_png, 534, "TextBoxTR.png"
static const unsigned char resource_LoadProgressComponent_textBoxTr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,
0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,37,73,68,65,84,120,156,132,210,49,107,131,64,
24,6,96,161,208,191,210,223,83,3,22,58,56,137,144,181,130,16,131,16,167,64,127,130,67,92,146,81,4,151,12,81,193,177,208,69,112,9,69,39,147,232,36,81,40,104,212,183,119,130,208,180,33,126,240,221,246,220,
123,119,223,49,170,170,98,54,155,65,146,164,70,20,197,207,201,100,178,97,89,246,153,244,3,0,230,111,51,174,235,98,183,219,97,187,221,194,52,77,232,186,14,77,211,192,243,252,7,65,79,255,64,154,166,56,157,
78,72,146,4,81,20,33,8,2,208,77,12,195,128,44,203,223,4,189,93,1,178,160,235,58,52,77,131,170,170,80,20,5,142,199,99,15,45,203,194,98,177,0,65,239,87,224,119,181,109,219,195,60,207,177,223,239,97,219,
54,230,243,57,69,47,55,193,144,120,185,92,112,62,159,17,134,33,214,235,53,4,65,248,34,232,241,38,24,170,174,107,208,59,250,190,143,229,114,73,83,94,239,2,154,84,150,101,159,178,90,173,192,113,220,230,
46,24,82,226,56,238,159,124,58,157,198,163,128,190,222,225,112,232,231,68,134,91,141,2,122,44,122,15,199,113,160,40,10,70,1,173,44,203,224,121,30,232,55,250,1,0,0,255,255,3,0,175,133,229,239,22,229,209,
25,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* LoadProgressComponent::textBoxTr_png = (const char*) resource_LoadProgressComponent_textBoxTr_png;
const int LoadProgressComponent::textBoxTr_pngSize = 534;

// JUCER_RESOURCE: textBoxBr_png, 526, "TextBoxBR.png"
static const unsigned char resource_LoadProgressComponent_textBoxBr_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,
0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,29,73,68,65,84,120,156,140,145,177,106,131,
80,20,134,133,66,31,166,143,19,75,10,93,92,4,31,192,65,146,234,144,197,173,111,32,226,96,167,34,136,139,67,80,200,88,232,36,6,138,96,39,69,55,81,39,53,250,215,115,33,33,208,18,123,225,78,247,251,126,206,
253,15,135,127,156,178,44,17,4,1,52,77,3,183,4,79,211,132,162,40,176,223,239,177,217,108,150,133,211,233,132,44,203,224,251,62,100,89,110,23,133,174,235,144,166,41,28,199,129,36,73,95,55,5,26,167,109,
91,196,113,12,211,52,177,94,175,223,110,10,125,223,179,15,31,14,7,232,186,142,213,106,245,252,167,64,201,195,48,160,174,107,28,143,71,216,182,13,81,20,191,103,225,254,151,48,142,35,155,187,170,42,36,73,
2,207,243,160,170,42,165,63,205,207,28,119,78,164,54,8,108,154,134,213,24,69,17,92,215,197,110,183,35,248,149,96,38,208,140,4,228,121,206,218,32,48,12,67,88,150,5,69,81,186,25,126,153,239,221,69,160,71,
90,10,245,76,213,25,134,193,82,5,65,248,152,193,135,51,120,17,104,221,219,237,150,150,66,61,127,242,60,255,62,131,143,215,169,215,247,7,0,0,255,255,3,0,135,93,228,21,19,153,235,168,0,0,0,0,73,69,78,68,
174,66,96,130,0,0};

const char* LoadProgressComponent::textBoxBr_png = (const char*) resource_LoadProgressComponent_textBoxBr_png;
const int LoadProgressComponent::textBoxBr_pngSize = 526;

// JUCER_RESOURCE: textBoxBl_png, 512, "TextBoxBL.png"
static const unsigned char resource_LoadProgressComponent_textBoxBl_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,
0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,1,15,73,68,65,84,120,156,140,145,177,106,131,
112,24,196,133,66,95,165,239,163,149,66,7,39,209,7,112,16,173,14,89,220,250,4,33,224,96,161,139,16,92,28,68,193,177,208,73,29,42,197,77,52,91,170,78,154,232,37,159,52,144,161,193,254,225,198,223,119,255,
187,99,12,195,64,24,134,216,237,118,248,207,99,52,77,67,16,4,168,235,26,211,52,45,3,138,162,252,248,190,143,178,44,113,60,30,151,1,89,150,191,92,215,69,81,20,232,251,126,25,224,121,254,109,179,217,32,
77,83,116,93,183,248,45,134,101,217,103,203,178,16,199,241,28,124,24,134,69,224,94,20,197,111,199,113,144,101,25,154,166,193,225,112,184,233,196,252,66,79,84,175,231,121,200,243,28,251,253,126,206,51,
142,227,77,128,244,186,90,173,176,221,110,145,36,9,170,170,66,219,182,51,72,237,93,28,175,129,187,179,94,84,85,237,109,219,70,20,69,51,72,237,81,229,180,19,101,188,6,46,122,16,4,225,131,220,214,235,53,
168,114,218,137,198,165,35,127,1,23,183,71,142,227,222,37,73,250,60,143,11,93,215,97,154,38,78,0,0,0,255,255,3,0,71,160,209,169,46,96,253,11,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* LoadProgressComponent::textBoxBl_png = (const char*) resource_LoadProgressComponent_textBoxBl_png;
const int LoadProgressComponent::textBoxBl_pngSize = 512;

// JUCER_RESOURCE: textBoxL_png, 278, "TextBoxL.png"
static const unsigned char resource_LoadProgressComponent_textBoxL_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,
11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,37,73,68,65,84,120,156,98,168,173,173,253,191,
127,255,254,255,47,95,190,252,79,12,96,24,213,48,170,1,59,0,0,0,0,255,255,3,0,192,63,31,127,25,29,95,216,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* LoadProgressComponent::textBoxL_png = (const char*) resource_LoadProgressComponent_textBoxL_png;
const int LoadProgressComponent::textBoxL_pngSize = 278;

// JUCER_RESOURCE: textBoxR_png, 280, "TextBoxR.png"
static const unsigned char resource_LoadProgressComponent_textBoxR_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,
11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,39,73,68,65,84,120,156,98,248,79,4,120,249,
242,229,255,253,251,247,255,175,173,173,253,207,48,170,97,84,3,118,13,0,0,0,0,255,255,3,0,127,138,31,127,172,190,106,191,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* LoadProgressComponent::textBoxR_png = (const char*) resource_LoadProgressComponent_textBoxR_png;
const int LoadProgressComponent::textBoxR_pngSize = 280;

// JUCER_RESOURCE: textBoxB_png, 286, "TextBoxB.png"
static const unsigned char resource_LoadProgressComponent_textBoxB_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,
11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,45,73,68,65,84,120,156,98,248,79,34,96,24,213,
48,56,52,188,124,249,242,63,41,152,97,255,254,253,255,73,193,12,181,181,181,255,73,193,0,0,0,0,255,255,3,0,51,127,31,127,148,136,189,231,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* LoadProgressComponent::textBoxB_png = (const char*) resource_LoadProgressComponent_textBoxB_png;
const int LoadProgressComponent::textBoxB_pngSize = 286;

// JUCER_RESOURCE: textBoxT_png, 286, "TextBoxT.png"
static const unsigned char resource_LoadProgressComponent_textBoxT_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,12,0,0,0,12,8,6,0,0,0,86,117,92,231,0,0,0,9,112,72,89,115,0,0,11,19,0,0,
11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,37,
46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,48,
48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,45,73,68,65,84,120,156,98,168,173,173,253,79,
10,102,216,191,127,255,127,82,48,195,203,151,47,255,147,130,25,254,147,8,70,53,12,14,13,0,0,0,0,255,255,3,0,12,89,31,127,241,76,165,114,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* LoadProgressComponent::textBoxT_png = (const char*) resource_LoadProgressComponent_textBoxT_png;
const int LoadProgressComponent::textBoxT_pngSize = 286;

// JUCER_RESOURCE: textBoxInner_png, 265, "TextBoxInner.png"
static const unsigned char resource_LoadProgressComponent_textBoxInner_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,10,0,0,0,10,8,6,0,0,0,141,50,207,189,0,0,0,9,112,72,89,115,0,0,11,19,
0,0,11,19,1,0,154,156,24,0,0,0,151,116,69,88,116,67,111,109,109,101,110,116,0,99,72,82,77,32,99,104,117,110,107,108,101,110,32,51,50,32,105,103,110,111,114,101,100,58,13,65,83,67,73,73,58,32,46,46,122,
37,46,46,195,132,195,137,46,46,203,152,203,135,46,46,195,132,195,136,46,46,117,48,46,46,195,141,96,46,46,58,195,178,46,46,46,111,13,72,69,88,58,32,48,48,48,48,55,65,50,53,48,48,48,48,56,48,56,51,48,48,
48,48,70,57,70,70,48,48,48,48,56,48,69,57,48,48,48,48,55,53,51,48,48,48,48,48,69,65,54,48,48,48,48,48,51,65,57,56,48,48,48,48,49,55,54,70,163,246,141,123,0,0,0,24,73,68,65,84,120,156,98,248,79,36,96,24,
85,72,95,133,0,0,0,0,255,255,3,0,35,251,142,128,175,59,63,124,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* LoadProgressComponent::textBoxInner_png = (const char*) resource_LoadProgressComponent_textBoxInner_png;
const int LoadProgressComponent::textBoxInner_pngSize = 265;
