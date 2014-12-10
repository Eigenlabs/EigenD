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

#ifndef __CANVAS_COMPONENT__
#define __CANVAS_COMPONENT__

#include "juce.h"

// TODO: 

//==============================================================================

class CanvasComponent : public Component
{
public:
    CanvasComponent() : x_(0), y_(0), width_(0), height_(0), textHeight_(0), textHeightPixels_(0) {}
    virtual ~CanvasComponent() {}
    
    virtual void setBoundsCanvasUnits(float x, float y, float width, float height)
    {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
    }

    virtual juce::Rectangle<float> getBoundsCanvasUnits() const
    {
        return juce::Rectangle<float>(x_, y_, width_, height_);
    }
    
    virtual void setPositionCanvasUnits(float x, float y)
    {
        x_ = x;
        y_ = y;
    }

    virtual void setTextHeightCanvasUnits(float textHeight)
    {
        textHeight_ = textHeight;
    }
    
    virtual float getTextHeightCanvasUnits()
    {
        return textHeight_;
    }

    virtual void setTextPosition(const String& textPosition)
    {
        textPosition_ = textPosition;
    }
    
    virtual String getTextPosition()
    {
        return textPosition_;
    }
    
    void updateBounds(float canvasWidth, float canvasHeight, 
                      float canvasWidthAspect, float canvasHeightAspect)
    {
        // TODO: scale with width also when on sides
        
        textHeightPixels_ = (int)(canvasHeight*textHeight_/canvasHeightAspect);
        
        setBounds((int)(canvasWidth*x_/canvasWidthAspect), (int)(canvasHeight*y_/canvasHeightAspect), 
                  (int)(canvasWidth*width_/canvasWidthAspect), (int)(canvasHeight*height_/canvasHeightAspect));
        
    }
    
protected:
    
    // canvas unit bounds
    float x_;
    float y_;
    float width_;
    float height_;
    
    // height of a text label of the component
    float textHeight_;
    int textHeightPixels_;
    
    // position of text label
    String textPosition_;
};

#endif // __CANVAS_COMPONENT__

