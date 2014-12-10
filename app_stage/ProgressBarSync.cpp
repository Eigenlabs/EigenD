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

#include "ProgressBarSync.h"

//==============================================================================
ProgressBarSync::ProgressBarSync (double& progress_)
    : progress (progress_), displayPercentage (true)
{
    currentValue = jlimit (0.0, 1.0, progress);
}

ProgressBarSync::~ProgressBarSync()
{
}

//==============================================================================
void ProgressBarSync::setPercentageDisplay (const bool shouldDisplayPercentage)
{
    displayPercentage = shouldDisplayPercentage;
    repaint();
}

void ProgressBarSync::setTextToDisplay (const String& text)
{
    displayPercentage = false;
    displayedMessage = text;
}

void ProgressBarSync::lookAndFeelChanged()
{
    setOpaque (findColour (backgroundColourId).isOpaque());
}

void ProgressBarSync::colourChanged()
{
    lookAndFeelChanged();
}

void ProgressBarSync::paint (Graphics& g)
{
    String text;
    
    if (displayPercentage)
    {
        if (currentValue >= 0 && currentValue <= 1.0)
            text << roundToInt (currentValue * 100.0) << "%";
    }
    else
    {
        text = displayedMessage;
    }
    
    drawProgressBar (g, getWidth(), getHeight(), currentValue, text);
}


void ProgressBarSync::drawProgressBar (Graphics& g, int width, int height,
                                       double progress, const String& textToShow)
{
	const Colour background (findColour (ProgressBarSync::backgroundColourId));
	const Colour foreground (findColour (ProgressBarSync::foregroundColourId));
    
	g.fillAll (background);
    
	if (progress >= 0.0f && progress < 1.0f)
	{
		LookAndFeel_V2::drawGlassLozenge (g, 1.0f, 1.0f,
                                              (float) jlimit (0.0, width - 2.0, progress * (width - 2.0)),
                                              (float) (height - 2),
                                              foreground,
                                              0.5f, 0.0f,
                                              true, true, true, true);
	}
	else
	{
		// spinning bar..
		g.setColour (foreground);
        
		const int stripeWidth = height * 2;
		const int position = (Time::getMillisecondCounter() / 15) % stripeWidth;
        
		Path p;
        
		for (float x = (float) (- position); x < width + stripeWidth; x += stripeWidth)
			p.addQuadrilateral (x, 0.0f,
								x + stripeWidth * 0.5f, 0.0f,
								x, (float) height,
								x - stripeWidth * 0.5f, (float) height);
        
		Image im (Image::ARGB, width, height, true);
        
		{
			Graphics g2 (im);
			LookAndFeel_V2::drawGlassLozenge (g2, 1.0f, 1.0f,
                                              (float) (width - 2),
                                              (float) (height - 2),
                                              foreground,
                                              0.5f, 0.0f,
                                              true, true, true, true);
		}
        
		g.setTiledImageFill (im, 0, 0, 0.85f);
		g.fillPath (p);
	}
    
	if (textToShow.isNotEmpty())
	{
		g.setColour (Colour::contrasting (background, foreground));
		g.setFont (height * 0.6f);
        
		g.drawText (textToShow, 0, 0, width, height, Justification::centred, false);
	}
}



void ProgressBarSync::updateProgress()
{
    double newProgress = progress;
    
    if (currentValue != newProgress
        || newProgress < 0 || newProgress >= 1.0
        || currentMessage != displayedMessage)
    {
        currentValue = newProgress;
        currentMessage = displayedMessage;
        repaint();
    }
    ComponentPeer* peer = getPeer();
    if(peer)
        peer->performAnyPendingRepaintsNow();
}


