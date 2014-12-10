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

#include "juce.h"

#ifndef __PROGRESS_BAR_SYNC__
#define __PROGRESS_BAR_SYNC__



//==============================================================================
/**
 A progress bar component.
 
  Derived from the Juce ProgressBar component, this one can be redrawn synchronously.
  This allows the task and the progress bar to both be run in the main message thread.
 */
class ProgressBarSync : public Component, public SettableTooltipClient
{
public:
    //==============================================================================
    /** Creates a ProgressBar.
     
     @param progress     pass in a reference to a double that you're going to
     update with your task's progress.
     */
    ProgressBarSync (double& progress);
    
    /** Destructor. */
    ~ProgressBarSync();
    
    //==============================================================================
    /** Turns the percentage display on or off.
     
     By default this is on, and the progress bar will display a text string showing
     its current percentage.
     */
    void setPercentageDisplay (const bool shouldDisplayPercentage);
    
    /** Gives the progress bar a string to display inside it.
     
     If you call this, it will turn off the percentage display.
     @see setPercentageDisplay
     */
    void setTextToDisplay (const String& text);
    
    // draw progress bar in a way derived from the look and feel class
    void drawProgressBar (Graphics& g, int width, int height, double progress, const String& textToShow);

    // update an redraw
    void updateProgress();
    
    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the bar.
     
     These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
     methods.
     
     @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
     */
    enum ColourIds
    {
        backgroundColourId              = 0x1001900,    /**< The background colour, behind the bar. */
        foregroundColourId              = 0x1001a00,    /**< The colour to use to draw the bar itself. LookAndFeel
                                                         classes will probably use variations on this colour. */
    };
    
protected:
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void lookAndFeelChanged();
    /** @internal */
    void colourChanged();
    
private:
    double& progress;
    double currentValue;
    bool displayPercentage;
    String displayedMessage, currentMessage;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgressBarSync);
};


#endif   // __PROGRESS_BAR_SYNC__
