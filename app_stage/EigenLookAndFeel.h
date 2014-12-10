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

#ifndef __EIGEN_LOOK_AND_FEEL__
#define __EIGEN_LOOK_AND_FEEL__

#include "EigenWidgets.h"
#include "WidgetImages.h" 
using namespace WidgetImages;

class EigenLookAndFeel : public LookAndFeel_V2
{
public:
    EigenLookAndFeel();
    ~EigenLookAndFeel();
    
    void setHasRedHatching(bool hasRedHatching);
    
    virtual void drawRotarySlider(Graphics &g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider &slider);
    virtual void drawHorizontalSliderBar (Graphics &g, AffineTransform& barTransform, float graphicsScale, int height, float sx, float swidth, float lx, float lwidth);
    virtual void drawVerticalSliderBar (Graphics &g, AffineTransform& barTransform, float graphicsScale, int width, float sy, float sh, float ly, float lh);
    virtual void drawLinearSliderBackground(Graphics &g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &slider);
    virtual void drawLinearSliderThumb(Graphics &g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &slider);
    virtual int  getSliderThumbRadius(Slider &slider);
    virtual Button* createSliderButton(Slider &slider, const bool isIncrement);
    virtual void drawToggleButton (Graphics& g, ToggleButton& button, bool isMouseOverButton, bool isButtonDown);
    virtual void drawTickBox (Graphics& g, Component& component, float x, float y, float w, float h, const bool ticked, const bool isEnabled, const bool isMouseOverButton, const bool isButtonDown);
    
    bool hasBoundsChanged(Slider* slider);

    enum ImageTag
    {
        buttonUpOff = 0,
        buttonDownOff,
        buttonLatent,
        buttonUpGreen,
        buttonUpOrange,
        buttonUpRed,
        plusMinusBack,
        minusUp,
        minusDown,
        minusLatent,
        plusUp,
        plusDown,
        plusLatent
    };
    
    Drawable* getImage(ImageTag imageTag);
    
private:
    Drawable* knobBack_;
    Drawable* knobBackRed_;
    Drawable* knobDial_;
    Drawable* knobPointer_;
    Drawable* horiSliderBack_;
    Drawable* horiSliderBar_;
    Drawable* horiSliderBarRed_;
    Drawable* horiSliderButton_;
    Drawable* vertSliderBack_;
    Drawable* vertSliderBar_;
    Drawable* vertSliderBarRed_;
    Drawable* vertSliderButton_;
    Drawable* buttonUpOff_;
    Drawable* buttonDownOff_;
    Drawable* buttonLatent_;
    Drawable* buttonUpGreen_;
    Drawable* buttonUpOrange_;
    Drawable* buttonUpRed_;
    Drawable* plusMinusBack_;
    Drawable* minusUp_;
    Drawable* minusDown_;
    Drawable* minusLatent_;
    Drawable* plusUp_;
    Drawable* plusDown_;
    Drawable* plusLatent_;

    bool hasRedHatching_;
    
};








#endif // __EIGEN_LOOK_AND_FEEL__
