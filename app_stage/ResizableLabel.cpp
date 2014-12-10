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

#include "ResizableLabel.h"


ResizableLabel :: ResizableLabel(const String &componentName, const String &labelText) : 
    Label(componentName, labelText), isVertical_(false), isUp_(false), widget_(0)
{
    setMinimumHorizontalScale(1);
    
    //setColour(Label::backgroundColourId, Colour(0xffdddddd));

    setEditable(true);
}


void ResizableLabel :: resized()
{
    int textHeight = 0;
    
    if(isVertical_)
        textHeight = getWidth() - 2 * getBorderSize().getLeftAndRight();
    else
        textHeight = getHeight() - 2 * getBorderSize().getTopAndBottom();

    Font font = getFont();
                        
    font.setHeight((float)textHeight);
    setFont(font);

    repaint();
}


void ResizableLabel :: paint(Graphics& g)
{
	g.fillAll (findColour (Label::backgroundColourId));

	if (! isBeingEdited())
	{
		const float alpha = isEnabled() ? 1.0f : 0.5f;
        
		g.setColour (findColour (Label::textColourId).withMultipliedAlpha (alpha));
		g.setFont (getFont());

        int textHeight=0;
        if(isVertical_)
            textHeight=getWidth();
        else 
            textHeight=getHeight();

		drawFittedText (g, getText(),
						  getBorderSize().getLeftAndRight(),
                          getBorderSize().getTopAndBottom(), 
                          getWidth() - 2 * getBorderSize().getLeftAndRight(),
                          getHeight() - 2 * getBorderSize().getTopAndBottom(),
						  getJustificationType(),
						  jmax (1, (int) (textHeight / getFont().getHeight())),
                          getMinimumHorizontalScale());
        
		g.setColour (findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
		g.drawRect (0, 0, getWidth(), getHeight());
	}
	else if (isEnabled())
	{
		g.setColour (findColour (Label::outlineColourId));
		g.drawRect (0, 0, getWidth(), getHeight());
	}
    
}


void ResizableLabel::drawFittedText (const Graphics& g, const String& text,
                                       const int x, const int y, const int width, const int height,
                                       const Justification& justification,
                                       const int maximumNumberOfLines,
                                       const float minimumHorizontalScale) const
{
	LowLevelGraphicsContext& context = g.getInternalContext();
    
    if (text.isNotEmpty()
        && width > 0 && height > 0
        && context.clipRegionIntersects (juce::Rectangle<int> (x, y, width, height)))
	{
		GlyphArrangement arr;

        if(isVertical_)
            arr.addFittedText (context.getFont(), text,
                               (float) y, (float) x, (float) height, (float) width,
                               justification,
                               maximumNumberOfLines,
                               minimumHorizontalScale);
        else
            arr.addFittedText (context.getFont(), text,
                               (float) x, (float) y, (float) width, (float) height,
                               justification,
                               maximumNumberOfLines,
                               minimumHorizontalScale);
        
        if (isVertical_)
            if (isUp_)
                arr.draw (g, AffineTransform::rotation(-3.141f/2.0f, x, y).translated((float)(-x),(float)(height)) );
            else
                arr.draw (g, AffineTransform::rotation(3.141f/2.0f, x, y).translated((float)(width+x),0.0f) );
        else
            arr.draw (g);
	}
}


void ResizableLabel::setWidget(WidgetComponent* widget) 
{
    widget_ = widget;
}

String ResizableLabel::getTooltip()
{
#if STAGE_BUILD==DESKTOP
    if (widget_)
        return widget_->getTooltip();
    else
        return "";
#else
    return "";
#endif // STAGE_BUILD==DESKTOP
}


