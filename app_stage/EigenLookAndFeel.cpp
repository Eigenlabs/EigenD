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

#include "EigenLookAndFeel.h"

//==============================================================================

EigenLookAndFeel::EigenLookAndFeel() : LookAndFeel_V2(), hasRedHatching_(false)
{
    knobBack_ = Drawable::createFromImageData(big_knob_1_svg, big_knob_1_svgSize);
    if(hasRedHatching_)
        knobBackRed_ = Drawable::createFromImageData(big_knob_2b_svg, big_knob_2b_svgSize);
    else
        knobBackRed_ = 0;
    knobDial_ = Drawable::createFromImageData(big_knob_3_svg, big_knob_3_svgSize);
    knobPointer_ = Drawable::createFromImageData(big_knob_4_svg, big_knob_4_svgSize);

    horiSliderBack_ = Drawable::createFromImageData(fader_horizontal_1_svg, fader_horizontal_1_svgSize);
    horiSliderBar_ = Drawable::createFromImageData(fader_horizontal_2a_svg, fader_horizontal_2a_svgSize);
    if(hasRedHatching_)
        horiSliderBarRed_ = Drawable::createFromImageData(fader_horizontal_2b_svg, fader_horizontal_2b_svgSize);
    else
        horiSliderBarRed_ = 0;
    horiSliderButton_ = Drawable::createFromImageData(fader_horizontal_3_svg, fader_horizontal_3_svgSize);
    
    vertSliderBack_ = Drawable::createFromImageData(fader_vertical_1_svg, fader_vertical_1_svgSize);
    vertSliderBar_ = Drawable::createFromImageData(fader_vertical_2a_svg, fader_vertical_2a_svgSize);
    if(hasRedHatching_)
        vertSliderBarRed_ = Drawable::createFromImageData(fader_vertical_2b_svg, fader_vertical_2b_svgSize);
    else
        vertSliderBarRed_ = 0;
    vertSliderButton_ = Drawable::createFromImageData(fader_vertical_3_svg, fader_vertical_3_svgSize);

    buttonUpOff_ = Drawable::createFromImageData(on_off_btn_1_svg, on_off_btn_1_svgSize);
    buttonDownOff_ = Drawable::createFromImageData(on_off_btn_2_svg, on_off_btn_2_svgSize);
    buttonLatent_ = Drawable::createFromImageData(on_off_btn_3_svg, on_off_btn_3_svgSize);
    buttonUpGreen_ = Drawable::createFromImageData(on_off_btn_4_svg, on_off_btn_4_svgSize);
    buttonUpOrange_ = Drawable::createFromImageData(on_off_btn_5_svg, on_off_btn_5_svgSize);
    buttonUpRed_ = Drawable::createFromImageData(on_off_btn_6_svg, on_off_btn_6_svgSize);
    
    plusMinusBack_ = Drawable::createFromImageData(plus_minus_back_svg, plus_minus_back_svgSize);
    minusUp_ = Drawable::createFromImageData(minus_1_svg, minus_1_svgSize);
    minusDown_ = Drawable::createFromImageData(minus_2_svg, minus_2_svgSize);
    //minusLatent_ = Drawable::createFromImageData(minus_3_svg, minus_3_svgSize);
    minusLatent_ = 0;
    plusUp_ = Drawable::createFromImageData(plus_1_svg, plus_1_svgSize);
    plusDown_ = Drawable::createFromImageData(plus_2_svg, plus_2_svgSize);
    //plusLatent_ = Drawable::createFromImageData(plus_3_svg, plus_3_svgSize);
    plusLatent_ = 0;


}

EigenLookAndFeel::~EigenLookAndFeel()
{
    deleteAndZero(knobBack_);
    if(knobBackRed_)
        deleteAndZero(knobBackRed_);
    deleteAndZero(knobDial_);
    deleteAndZero(knobPointer_);

    deleteAndZero(horiSliderBack_);
    deleteAndZero(horiSliderBar_);
    if(horiSliderBarRed_)
        deleteAndZero(horiSliderBarRed_);
    deleteAndZero(horiSliderButton_);

    deleteAndZero(vertSliderBack_);
    deleteAndZero(vertSliderBar_);
    if(vertSliderBarRed_)
        deleteAndZero(vertSliderBarRed_);
    deleteAndZero(vertSliderButton_);

    deleteAndZero(buttonUpOff_);
    deleteAndZero(buttonDownOff_);
    if(buttonLatent_)
        deleteAndZero(buttonLatent_);
    deleteAndZero(buttonUpGreen_);
    deleteAndZero(buttonUpOrange_);
    deleteAndZero(buttonUpRed_);
    
    deleteAndZero(plusMinusBack_);
    deleteAndZero(minusUp_);
    deleteAndZero(minusDown_);
    if(minusLatent_)
        deleteAndZero(minusLatent_);
    deleteAndZero(plusUp_);
    deleteAndZero(plusDown_);
    if(plusLatent_)
        deleteAndZero(plusLatent_);
}

void EigenLookAndFeel::setHasRedHatching(bool hasRedHatching)
{
    hasRedHatching_ = hasRedHatching;
    
    if(hasRedHatching)
    {
        if(knobBackRed_ == 0)
            knobBackRed_ = Drawable::createFromImageData(big_knob_2b_svg, big_knob_2b_svgSize);
        if (horiSliderBarRed_ == 0)
            horiSliderBarRed_ = Drawable::createFromImageData(fader_horizontal_2b_svg, fader_horizontal_2b_svgSize);
        if (vertSliderBarRed_ == 0)
            vertSliderBarRed_ = Drawable::createFromImageData(fader_vertical_2b_svg, fader_vertical_2b_svgSize);
    }


}

//==============================================================================

Drawable* EigenLookAndFeel::getImage(ImageTag imageTag)
{
    switch (imageTag) {
        case buttonUpOff:
            return buttonUpOff_;
        case buttonDownOff:
            return buttonDownOff_;
        case buttonLatent:
            return buttonLatent_;
        case buttonUpGreen:
            return buttonUpGreen_;
        case buttonUpOrange:
            return buttonUpOrange_;
        case buttonUpRed:
            return buttonUpRed_;
        case plusMinusBack:
            return plusMinusBack_;
        case minusUp:
            return minusUp_;
        case minusDown:
            return minusDown_;
        case minusLatent:
            return minusLatent_;
        case plusUp:
            return plusUp_;
        case plusDown:
            return plusDown_;
        case plusLatent:
            return plusLatent_;
        default:
            return 0;
    }
}

//==============================================================================

bool EigenLookAndFeel::hasBoundsChanged(Slider* slider)
{
    // check to see if bounds have changed to try and determine if the background of a slider needs drawing
    // rather than just the pointer, but Juce painting must draw all the component every time so need to
    // rethink this...
    EigenSlider* eigenSlider = dynamic_cast<EigenSlider*> (slider);
    
    bool resized = (eigenSlider->getLastX()!=eigenSlider->getX() ||
                    eigenSlider->getLastY()!=eigenSlider->getY() ||
                    eigenSlider->getLastWidth()!=eigenSlider->getWidth() ||
                    eigenSlider->getLastHeight()!=eigenSlider->getHeight());

    eigenSlider->setLastBounds();
    
    return resized;

}


//==============================================================================
// Draw rotary slider
//
//==============================================================================

void EigenLookAndFeel::drawRotarySlider(Graphics  &g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider &slider)
{
    //g.setColour(Colour(0x7fff0000));
    //g.drawRect(x, y, width, height, 1);
   
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    //const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

    EigenSlider* eigenSlider = dynamic_cast<EigenSlider*> (&slider);
    const float latentSliderPos = (float)eigenSlider->valueToProportionOfLength((double)eigenSlider->getLatentValue());
    const float latentAngle = rotaryStartAngle + latentSliderPos * (rotaryEndAngle - rotaryStartAngle);

    // angle to start drawing the arc from
    float minimum = (float)eigenSlider->getMinimum();
    float maximum = (float)eigenSlider->getMaximum();
    float trackStartPos = 0.0f, startAngle = 0.0f;
    if(minimum < 0 && maximum > 0)
    {
        // slider where 0 somewhere in the middle of the slider
        trackStartPos = (float)eigenSlider->valueToProportionOfLength(0.0);
        startAngle = rotaryStartAngle + trackStartPos * (rotaryEndAngle - rotaryStartAngle);
    }
    else
        if (minimum >= 0 && maximum > 0) 
        {
            // start from left
            trackStartPos = 0.0f;
            startAngle = rotaryStartAngle - 0.01f;
        }
        else 
        {
            // start from right
            trackStartPos = 1.0f;
            startAngle = rotaryEndAngle + 0.02f;
        }
        
    float halfPointerWidth = 13.06f/2.0f;
    float dialWidth = 105.27f;
    float backWidth = 137.98f;
    //float backHeight = 154.48f;
    //float graphicsHeight = backHeight;
    float graphicsWidth = backWidth+4;
    float graphicsCentre = backWidth/2.0f;
    float graphicsScale = width/graphicsWidth;

    AffineTransform backTransform = AffineTransform::translation(2, 2).scaled(graphicsScale, graphicsScale);
    AffineTransform redBackTransform = AffineTransform::translation(2.75, 2.75).scaled(width/(graphicsWidth+1.5f), width/(graphicsWidth+1.5f));
    
    // draw background
    knobBack_->draw(g, 1, backTransform);

    float arcOff = 2*graphicsScale;
    
    Path filledArc, latentFilledArc, backFilledArc1, backFilledArc2;
    const float thickness = 0.7f;
    
    if(latentSliderPos<sliderPos)
    {
        if(hasRedHatching_)
            //knobBackRed_->draw(g, 1, AffineTransform::translation(2.75f, 2.75f).scaled(height/(graphicsHeight+1.5f), height/(graphicsHeight+1.5f)));
            knobBackRed_->draw(g, 1, redBackTransform);
        
        // draw current value arc
        g.setColour(Colour(0xff000000));

        if(latentSliderPos<trackStartPos)
            filledArc.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                     startAngle, angle, thickness);
        else
            filledArc.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                     startAngle, latentAngle, thickness);
        g.fillPath (filledArc);

        if (hasRedHatching_) 
        {
            // draw grey beyond latent value arc
            g.setColour(Colour(0xffe6e6e6));

            if(latentSliderPos<trackStartPos)
                backFilledArc1.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                              rotaryStartAngle-0.01f, latentAngle, thickness);
            else
                backFilledArc1.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                              rotaryStartAngle-0.01f, startAngle, thickness);
            if(sliderPos>trackStartPos)
                backFilledArc2.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                              angle, rotaryEndAngle+0.02f, thickness);
            else
                backFilledArc2.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                              startAngle, rotaryEndAngle+0.02f, thickness);

            g.fillPath (backFilledArc1);
            g.fillPath (backFilledArc2);
        }
        else 
        {
            // draw red latent arc
            g.setColour(Colour(0xffbb4d3f));
            if(latentSliderPos<trackStartPos && sliderPos>trackStartPos)
                latentFilledArc.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                               latentAngle, startAngle, thickness);
            else
                latentFilledArc.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                           latentAngle, angle, thickness);
            g.fillPath (latentFilledArc);
        }

    }
    else if(latentSliderPos>sliderPos)
    {
        if(hasRedHatching_)
            //knobBackRed_->draw(g, 1, AffineTransform::translation(2.75f, 2.75f).scaled(height/(graphicsHeight+1.5f), height/(graphicsHeight+1.5f)));
            knobBackRed_->draw(g, 1, redBackTransform);

        // draw current value arc
        g.setColour(Colour(0xff000000));

        if(latentSliderPos>trackStartPos)
            filledArc.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                     startAngle, angle, thickness);
        else
            filledArc.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                     latentAngle, startAngle, thickness);
        
        g.fillPath (filledArc);

        if(hasRedHatching_)
        {
            // draw grey beyond latent value arc
            g.setColour(Colour(0xffe6e6e6));
            
            if(sliderPos<trackStartPos)
                backFilledArc1.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                              rotaryStartAngle-0.01f, angle, thickness);
            else
                backFilledArc1.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                              rotaryStartAngle-0.01f, startAngle, thickness);
            if(latentSliderPos>trackStartPos)
                backFilledArc2.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                              latentAngle, rotaryEndAngle+0.02f, thickness);
            else
                backFilledArc2.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                              startAngle, rotaryEndAngle+0.02f, thickness);
            
            g.fillPath (backFilledArc1);
            g.fillPath (backFilledArc2);

        }
        else 
        {
            // draw red latent arc
            g.setColour(Colour(0xffbb4d3f));
            if(latentSliderPos>trackStartPos && sliderPos<trackStartPos)
                latentFilledArc.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                               startAngle, latentAngle, thickness);
            else
                latentFilledArc.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                               latentAngle, angle, thickness);
            g.fillPath (latentFilledArc);
        }

    }
    else 
    {
        g.setColour(Colour(0xff000000));
        filledArc.addPieSegment (arcOff, arcOff, (backWidth)*graphicsScale, (backWidth)*graphicsScale,
                                 startAngle, angle, thickness);
        g.fillPath (filledArc);

    }


    AffineTransform dialTransform = AffineTransform::translation(graphicsCentre-dialWidth/2.0f, graphicsCentre-dialWidth/2.0f).followedBy(backTransform);
    knobDial_->draw(g, 1, dialTransform);
    
    
    
    
    // draw pointer
    AffineTransform pointerTransform = AffineTransform::rotation(angle-3.141f, halfPointerWidth, halfPointerWidth).
                                        translated(graphicsCentre-halfPointerWidth, graphicsCentre-halfPointerWidth).
                                        followedBy(backTransform);

    knobPointer_->draw(g, 1, pointerTransform);

}


//==============================================================================
// Draw linear slider
// 
//==============================================================================

void EigenLookAndFeel::drawLinearSliderBackground (Graphics &g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &slider)
{
    //g.setColour(Colour(0xffeeeeee));
    //g.fillRect(x, y, width, height);

    float graphicsScale = 0, graphicsWidth = 0, graphicsHeight = 0;
    AffineTransform backTransform;
    
    if(slider.isHorizontal())
    {
        graphicsWidth = 237.0f;
        // scale up by 1/(1-(2*1/15.8)) = 1.145 to account for thumb radius as a proportion of length (distance from edge of image thumb starts)
        graphicsScale = 1.145f*width/graphicsWidth;
        backTransform = AffineTransform::scale(graphicsScale, graphicsScale).translated((float)x,(float)y);
        // unscaled, thumb starts 15 pixels in, so offset slider to left (-15)
        // offset down (8) to account for thumb height
        horiSliderBack_->draw(g, 1, AffineTransform::translation(-15.0f, 8.0f).followedBy(backTransform));
    }
    else 
    {
        graphicsWidth = 68.0f;
        graphicsHeight = 257.0f;
        graphicsScale = (257.0f/205.0f)*height/graphicsHeight;
        backTransform = AffineTransform::scale(graphicsScale, graphicsScale).translated((float)x,(float)y);
        vertSliderBack_->draw(g, 1, AffineTransform::translation(3.5f,-15.0f).followedBy(backTransform));
    }

    
}

void EigenLookAndFeel::drawHorizontalSliderBar (Graphics &g, AffineTransform& barTransform, float graphicsScale, int height, float sx, float sw, float lx, float lw)
{
    // draw the set value indicator
    g.saveState();
    g.reduceClipRegion((int)floorf(sx),0,(int)floorf(sw),height);
    horiSliderBar_->draw(g, 1, barTransform);
    g.restoreState();
    
    // draw latent value indicator
    g.saveState();
    g.reduceClipRegion((int)lx,0,(int)lw,height);
    if(hasRedHatching_)
        horiSliderBarRed_->draw(g, 1, barTransform);
    else
    {
        Path latentBar;
        g.setColour(Colour(0xffbb4d3f));
        latentBar.addRoundedRectangle(6.35f*graphicsScale, (8.0f+6.0f)*graphicsScale, 221.973*graphicsScale, 17.998*graphicsScale, (17.998/2.0)*graphicsScale);
        g.fillPath (latentBar);
    }
    g.restoreState();
}

void EigenLookAndFeel::drawVerticalSliderBar (Graphics &g, AffineTransform& barTransform, float graphicsScale, int width, float sy, float sh, float ly, float lh)
{
    // draw the set value indicator
    g.saveState();
    g.reduceClipRegion(0,(int)sy,width,(int)sh);
    vertSliderBar_->draw(g, 1, barTransform);
    g.restoreState();
    
    // draw latent value indicator
    g.saveState();
    g.reduceClipRegion(0,(int)ly,width,(int)ceilf(lh));
    if(hasRedHatching_)
        vertSliderBarRed_->draw(g, 1, barTransform);
    else
    {
        Path latentBar;
        g.setColour(Colour(0xffbb4d3f));
        latentBar.addRoundedRectangle((24.0f+3.5f)*graphicsScale, (6.0f+22.0f)*graphicsScale, 17.998*graphicsScale, 221.973*graphicsScale, (17.998/2.0)*graphicsScale);
        g.fillPath (latentBar);
    }
    g.restoreState();
}

void EigenLookAndFeel::drawLinearSliderThumb (Graphics &g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &slider)
{
    //g.setColour(Colour(0xffff0000));
    //g.drawRect(x, y, width, height, 1);

    float graphicsWidth = 0, graphicsHeight = 0;
    float graphicsScale = 0;
    float edgeOffset = 0;
    
    AffineTransform barTransform;
    AffineTransform sliderTransform;

    EigenSlider* eigenSlider = dynamic_cast<EigenSlider*> (&slider);

    float latentSliderPos = eigenSlider->getPositionOfValue(eigenSlider->getLatentValue());

    if(slider.isHorizontal())
    {
        graphicsWidth = 237.0f;
        graphicsHeight = 67.1f+8.0f;
        graphicsScale = 1.145f*width/graphicsWidth;
        // additional offset to make bar go to curved edge of slider
        edgeOffset = 10.0f*graphicsScale;
    }
    else 
    {
        graphicsWidth = 68.0;
        graphicsHeight = 257.0;
        // (257.0/205.0) scaling of total size over track size
        graphicsScale = (257.0f/205.0f)*height/graphicsHeight;
        // additional offset to make bar go to curved edge of slider
        edgeOffset = -10.0f*graphicsScale;
    }

    float minimum = (float)eigenSlider->getMinimum();
    float maximum = (float)eigenSlider->getMaximum();
    float trackStartPos = 0.0f;
    if(minimum < 0 && maximum > 0)
    {
        // slider where 0 somewhere in the middle of the slider
        trackStartPos = (float)eigenSlider->getPositionOfValue(0.0);
    }
    else
        if (minimum >= 0 && maximum > 0) 
        {
            // start from left
            trackStartPos = (float)eigenSlider->getPositionOfValue(minimum) - edgeOffset;
        }
        else 
        {
            // start from right
            trackStartPos = (float)eigenSlider->getPositionOfValue(maximum) + edgeOffset;
        }
    
    if(slider.isHorizontal())
    {
        // locate the thumb relative to the slider background
        barTransform = AffineTransform::translation(6.35f, 8.0f+6.0f).scaled(graphicsScale, graphicsScale);
        sliderTransform = AffineTransform::translation(-5.5f,1.0f).scaled(graphicsScale, graphicsScale).translated(sliderPos, 0);
        
        if (latentSliderPos<sliderPos) 
            if(latentSliderPos<trackStartPos && sliderPos<trackStartPos)
                drawHorizontalSliderBar(g, barTransform, graphicsScale, height, sliderPos, (trackStartPos-sliderPos), latentSliderPos, (sliderPos-latentSliderPos));
            else if(latentSliderPos<trackStartPos && sliderPos>trackStartPos)
                drawHorizontalSliderBar(g, barTransform, graphicsScale, height, trackStartPos, (sliderPos-trackStartPos), latentSliderPos, (trackStartPos-latentSliderPos));
            else 
                drawHorizontalSliderBar(g, barTransform, graphicsScale, height, trackStartPos, (latentSliderPos-trackStartPos), latentSliderPos, (sliderPos-latentSliderPos));
        else if (latentSliderPos>sliderPos)
            if(latentSliderPos<trackStartPos && sliderPos<trackStartPos)
                drawHorizontalSliderBar(g, barTransform, graphicsScale, height, latentSliderPos, (trackStartPos-latentSliderPos), sliderPos, (latentSliderPos-sliderPos));
            else if(latentSliderPos>trackStartPos && sliderPos<trackStartPos)
                drawHorizontalSliderBar(g, barTransform, graphicsScale, height, sliderPos, (trackStartPos-sliderPos), trackStartPos, (latentSliderPos-trackStartPos));
            else 
                drawHorizontalSliderBar(g, barTransform, graphicsScale, height, trackStartPos, (sliderPos-trackStartPos), sliderPos, (latentSliderPos-sliderPos));
        else 
        {
            g.saveState();
            if (trackStartPos<sliderPos)
                g.reduceClipRegion((int)trackStartPos,0,(int)(sliderPos-trackStartPos),height);
            else
                g.reduceClipRegion((int)sliderPos,0,(int)(trackStartPos-sliderPos),height);
            horiSliderBar_->draw(g, 1, barTransform);
            g.restoreState();
        }

        horiSliderButton_->draw(g, 1, sliderTransform);
        
    }
    else 
    {
        barTransform = AffineTransform::translation(24.0f+3.5f, 6.0f+22.0f).scaled(graphicsScale, graphicsScale);
        sliderTransform = AffineTransform::translation(14.5f,-6.5f).scaled(graphicsScale, graphicsScale).translated(0, sliderPos);

        if (latentSliderPos<sliderPos) 
            if(latentSliderPos<trackStartPos && sliderPos<trackStartPos)
                drawVerticalSliderBar(g, barTransform, graphicsScale, width, sliderPos, (trackStartPos-sliderPos), latentSliderPos, (sliderPos-latentSliderPos));
            else if(latentSliderPos<trackStartPos && sliderPos>trackStartPos)
                drawVerticalSliderBar(g, barTransform, graphicsScale, width, trackStartPos, (sliderPos-trackStartPos), latentSliderPos, (trackStartPos-latentSliderPos));
            else 
                drawVerticalSliderBar(g, barTransform, graphicsScale, width, trackStartPos, (latentSliderPos-trackStartPos), latentSliderPos, (sliderPos-latentSliderPos));
        else if (latentSliderPos>sliderPos)
            if(latentSliderPos<trackStartPos && sliderPos<trackStartPos)
                drawVerticalSliderBar(g, barTransform, graphicsScale, width, latentSliderPos, (trackStartPos-latentSliderPos), sliderPos, (latentSliderPos-sliderPos));
            else if(latentSliderPos>trackStartPos && sliderPos<trackStartPos)
                drawVerticalSliderBar(g, barTransform, graphicsScale, width, sliderPos, (trackStartPos-sliderPos), trackStartPos, (latentSliderPos-trackStartPos));
            else 
                drawVerticalSliderBar(g, barTransform, graphicsScale, width, trackStartPos, (sliderPos-trackStartPos), sliderPos, (latentSliderPos-sliderPos));
        else 
        {
            g.saveState();
            if (trackStartPos<sliderPos)
                g.reduceClipRegion(0,(int)trackStartPos,width,(int)(sliderPos-trackStartPos));
            else
                g.reduceClipRegion(0,(int)sliderPos,width,(int)(trackStartPos-sliderPos));
            vertSliderBar_->draw(g, 1, barTransform);
            g.restoreState();
        }
        
        vertSliderButton_->draw(g, 1, sliderTransform);
    }

}

int	EigenLookAndFeel::getSliderThumbRadius (Slider &slider)
{
    if(slider.isHorizontal())
    {
        // slider thumb starts at 1/15.8 along the slider width
        return (int)(((float)slider.getWidth())/15.8);
    }
    else 
    {
        // slider thumb starts at 1/7.2 along the slider height taking into account slider value label
        return (int)(((float)slider.getHeight())/7.2);
    }

}

//==============================================================================
// Draw +- slider
// 
//==============================================================================

Button* EigenLookAndFeel::createSliderButton(Slider &slider, const bool isIncrement)
{
    DrawableButton* button;
    
    if(isIncrement)
    {
        button = new DrawableButton("plus_button",DrawableButton::ImageFitted);
        button->setImages(plusUp_, 0, plusDown_, 0, 0, 0, 0, 0);

        
        // remove the edge indent to ensure images draw to the right size
        button->setEdgeIndent(0);
    }
    else 
    {
        button = new DrawableButton("minus_button",DrawableButton::ImageFitted);
        button->setImages(minusUp_, 0, minusDown_, 0, 0, 0, 0, 0);
        // remove the edge indent to ensure images draw to the right size
        button->setEdgeIndent(0);
    }
    return button;

}

//==============================================================================
// Draw toggle button
// 
//==============================================================================

void EigenLookAndFeel::drawToggleButton (Graphics& g,
                                            ToggleButton& button,
                                            bool isMouseOverButton,
                                            bool isButtonDown)
{
	if (button.hasKeyboardFocus (true))
	{
		g.setColour (button.findColour (TextEditor::focusedOutlineColourId));
		g.drawRect (0, 0, button.getWidth(), button.getHeight());
	}
    
	float fontSize = jmin (15.0f, button.getHeight() * 0.75f);
	const float tickWidth = fontSize * 1.1f;

    if (! button.isEnabled())
		g.setOpacity (0.5f);

	drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
				 tickWidth, tickWidth,
				 button.getToggleState(),
				 button.isEnabled(),
				 isMouseOverButton,
				 isButtonDown);
    
	g.setColour (button.findColour (ToggleButton::textColourId));
	g.setFont (fontSize);
    
    
	const int textX = (int) tickWidth + 5;
    
	g.drawFittedText (button.getButtonText(),
					  textX, 0,
					  button.getWidth() - textX - 2, button.getHeight(),
					  Justification::centredLeft, 10);
}

void EigenLookAndFeel::drawTickBox (Graphics& g,
                                       Component& component,
                                       float x, float y, float w, float h,
                                       const bool ticked,
                                       const bool isEnabled,
                                       const bool isMouseOverButton,
                                       const bool isButtonDown)
{
	const float boxSize = w * 0.7f;

    g.setColour(Colour(0xfffdfdfd));
    if (! component.isEnabled())
		g.setOpacity (0.5f);

    g.fillEllipse(x, y + (h - boxSize) * 0.5f, boxSize, boxSize);

    g.setColour(Colour(0xff999999));
    if (! component.isEnabled())
		g.setOpacity (0.5f);
    g.drawEllipse(x, y + (h - boxSize) * 0.5f, boxSize, boxSize, 1.5);
    

	if (ticked)
	{
		Path tick;
		tick.startNewSubPath (1.5f, 3.0f);
		tick.lineTo (3.0f, 6.0f);
		tick.lineTo (6.0f, 0.0f);
        
		g.setColour (isEnabled ? Colours::black : Colours::grey);
        
		const AffineTransform trans (AffineTransform::scale (w / 9.0f, h / 9.0f)
                                     .translated (x, y));
        
		g.strokePath (tick, PathStrokeType (2.5f), trans);
	}
}