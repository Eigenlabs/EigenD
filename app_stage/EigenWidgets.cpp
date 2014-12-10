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

#include "EigenWidgets.h"
#include <algorithm>

//------------------------------------------------------------------------------
// 
// Classes inherited from the Juce widgets to provide latent values
// 
//------------------------------------------------------------------------------

bool EigenCustomWidget::isEnabled()
{
    return widget_->getEnabled();
}

void EigenRPCWidget::timerCallback()
{
    value_age_++;

    if(value_age_ > 10)
    {
        getWidget()->requestCurrentValue();
    }
}

String EigenCustomWidget::getTooltip() 
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

EigenOSCWidget::EigenOSCWidget(WidgetComponent *widget): EigenCustomWidget(widget)
{
    timeout_ = 0;
}

EigenOSCWidget::~EigenOSCWidget()
{
    stopTimer();
}

void EigenOSCWidget::timerCallback()
{
    stopTimer();

    if(timeout_ > 1)
    {
        sendValue(latent_value_);
        timeout_--;
        startTimer(50);
        return;
    }

    if(timeout_ == 1)
    {
        latent_value_ = current_value_;
        resetValue(false,current_value_,current_value_);
        return;
    }
}

void EigenOSCWidget::handleValueMessage(const WidgetValueMessage *vm)
{
    current_value_ = vm->data;

    if(timeout_>0)
    {
        if(compareValues(current_value_,latent_value_))
        {
            resetValue(false,current_value_,current_value_);
            stopTimer();
            timeout_ = 0;
        }
        else
        {
            resetValue(true,current_value_,latent_value_);
        }

        return;
    }

    latent_value_ = current_value_;
    resetValue(false,current_value_,current_value_);
}

void EigenOSCWidget::sendValue(const WidgetData &data)
{
    stopTimer();

    latent_value_ = data;
    getWidget()->sendValue(data);

    if(compareValues(current_value_,latent_value_))
    {
        timeout_ = 0;
        resetValue(false,current_value_,current_value_);
    }
    else
    {
        startTimer(50);
        timeout_ = 100;
        resetValue(true,current_value_,latent_value_);
    }
}


EigenSlider::EigenSlider(const String &componentName) : 
    Slider(componentName), lastX(0), lastY(0), lastWidth(0), lastHeight(0),
    latentValue_(0.5), widget_(0)
{
}


EigenSlider::~EigenSlider()
{
    
}

double EigenSlider::prop2val(double v)
{
    if(0 == v) return 0;
    return exp(log(v)/getSkewFactor());
}

double EigenSlider::val2prop(double v)
{
    return pow(v,getSkewFactor());
}

double EigenSlider::proportionOfLengthToValue(double proportion)
{
    const double max = getMaximum();
    const double min = getMinimum();

    if(getSkewFactor() != 1.0)
    {
        if(min < 0.f && max > 0.f)
        {
            double range = std::max<double>(std::abs(min),max);
            double minpart = 1-val2prop((range+min)/range);
            double maxpart = 1-val2prop((range-max)/range);
            double stretch = minpart+maxpart;

            double p = proportion*stretch;
            double value;
            if(p<=minpart)
            {
                value = prop2val(1-minpart+p)*range-range;
            }
            else
            {
                value = range-prop2val(minpart+1-p)*range;
            }
            return value;
        }
        else if(proportion > 0.0)
        {
            return min + (max - min) * prop2val(proportion);
        }
    }

    return min + (max - min) * proportion;
}

double EigenSlider::valueToProportionOfLength(double value)
{
    const double max = getMaximum();
    const double min = getMinimum();

    if(getSkewFactor() != 1.0)
    {
        if(min < 0.f && max > 0.f)
        {
            // This approach effectively builds two identical sliders at full
            // to ensure that the taper behaves exactly the same upwards and
            // downwards.
            // Initially it calculates how long the negative and positive sliders
            // have to be to reach the minimum and maximum values, this is then
            // used to scale and offset those two sliders so that the values
            // are windowed correctly for the single bipolar slider that's available.
            // The rest of the calculations ensure that the calculation puts the
            // taper at the extremes and not in the center, since it's when
            // the values approach zero that the slope is the steepest, meaning
            // that it has to be inverted and offset.
            double range = std::max<double>(std::abs(min),max);
            double minpart = 1-val2prop((range+min)/range);
            double maxpart = 1-val2prop((range-max)/range);
            double stretch = minpart+maxpart;
            double proportion;
            if(value <= 0.0)
            {
                proportion = minpart-1+val2prop((range+value)/range);
            }
            else
            {
                proportion = minpart+1-val2prop((range-value)/range);
            }
            proportion = proportion/stretch;
            return proportion;
        }
        else
        {
            return val2prop((value - min) / (max - min));
        }
    }

    return (value - min) / (max - min);
}

void EigenSlider::mouseDown(const MouseEvent& e)
{
    // filter out right mouse button, can only click with left
    if(!e.mods.isRightButtonDown())
        Slider::mouseDown(e);
}

void EigenSlider::mouseDrag(const MouseEvent& e)
{
    // filter out right mouse button, can only drag with left
    if(!e.mods.isRightButtonDown())
        Slider::mouseDrag(e);
}

void EigenSlider::setWidget(WidgetComponent* widget) 
{
    widget_ = widget;
}

String EigenSlider::getTooltip() 
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


//------------------------------------------------------------------------------

EigenButton::EigenButton(const String &buttonName, const ButtonStyle buttonStyle) : 
    DrawableButton(buttonName, buttonStyle), lastState_(false), latentState_(false),
    widget_(0) 
{
}

void EigenButton::updateImages()
{
    EigenLookAndFeel* lookAndFeel = dynamic_cast<EigenLookAndFeel*> (&LookAndFeel::getDefaultLookAndFeel());

    if(getToggleState() == latentState_) 
        setImages(lookAndFeel->getImage(EigenLookAndFeel::buttonUpOff), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonDownOff), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonUpGreen), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonDownOff));
    else if(getToggleState())
        setImages(lookAndFeel->getImage(EigenLookAndFeel::buttonUpOff), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonDownOff), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonLatent), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonDownOff));
    else 
        setImages(lookAndFeel->getImage(EigenLookAndFeel::buttonLatent), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonDownOff), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonUpGreen), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonDownOff));
    
}

void EigenButton::mouseDown(const MouseEvent& e)
{
    // filter out right mouse button, can only click with left
    if(!e.mods.isRightButtonDown())
        DrawableButton::mouseDown(e);
}

void EigenButton::mouseDrag(const MouseEvent& e)
{
    // filter out right mouse button, can only drag with left
    if(!e.mods.isRightButtonDown())
        DrawableButton::mouseDrag(e);
}

void EigenButton::setWidget(WidgetComponent* widget) 
{
    widget_ = widget;
}

String EigenButton::getTooltip() 
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


//------------------------------------------------------------------------------

EigenTrigger::EigenTrigger(const String &buttonName, const ButtonStyle buttonStyle) : 
    DrawableButton(buttonName, buttonStyle), status_(0),
    widget_(0) 
{
}

void EigenTrigger::updateImages()
{
    EigenLookAndFeel* lookAndFeel = dynamic_cast<EigenLookAndFeel*> (&LookAndFeel::getDefaultLookAndFeel());

    if(getToggleState())
    {
        setImages(lookAndFeel->getImage(EigenLookAndFeel::buttonUpOff), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonDownOff), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonLatent), 0,
                  lookAndFeel->getImage(EigenLookAndFeel::buttonDownOff));
    }
    else 
    {
        EigenLookAndFeel::ImageTag status_image = EigenLookAndFeel::buttonUpOff;
        switch(status_)
        {
            case 1:
                status_image = EigenLookAndFeel::buttonUpGreen;
                break;
            case 2:
                status_image = EigenLookAndFeel::buttonUpRed;
                break;
            case 3:
                status_image = EigenLookAndFeel::buttonUpOrange;
                break;
        }
        setImages(lookAndFeel->getImage(status_image), 0,
                lookAndFeel->getImage(EigenLookAndFeel::buttonDownOff), 0,
                lookAndFeel->getImage(status_image), 0,
                lookAndFeel->getImage(EigenLookAndFeel::buttonDownOff));
    }
}

void EigenTrigger::mouseDown(const MouseEvent& e)
{
    // filter out right mouse button, can only click with left
    if(!e.mods.isRightButtonDown())
        DrawableButton::mouseDown(e);
}

void EigenTrigger::mouseDrag(const MouseEvent& e)
{
    // filter out right mouse button, can only drag with left
    if(!e.mods.isRightButtonDown())
        DrawableButton::mouseDrag(e);
}

void EigenTrigger::setWidget(WidgetComponent* widget) 
{
    widget_ = widget;
}

String EigenTrigger::getTooltip() 
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
