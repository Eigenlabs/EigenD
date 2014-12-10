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

#ifndef __EIGEN_WIDGETS__
#define __EIGEN_WIDGETS__

#include "WidgetComponent.h"
#include "EigenLookAndFeel.h"
#include "Network.h"

class WidgetComponent;
class WidgetConnectedMessage;
class WidgetValueMessage;

// Base class for new style custom widgets

class EigenCustomWidget : public Component
{
public:
    EigenCustomWidget(WidgetComponent *widget): widget_(widget) {}
    virtual ~EigenCustomWidget() {}
    
    static EigenCustomWidget *widgetFactory(const String &widgetType, WidgetComponent *widget);

    WidgetComponent *getWidget() { return widget_; }
    virtual String getTooltip();
    virtual float getGraphicsWidth() { return 100.0; }
    virtual float getGraphicsHeight() { return 100.0; }

    virtual void handleValueMessage(const WidgetValueMessage *vm) {}

    virtual void enableChanged() {}
    bool isEnabled();
    
private:
    WidgetComponent* widget_;
};

class EigenRPCWidget: public EigenCustomWidget, public Timer
{
public:
    EigenRPCWidget(WidgetComponent *widget): EigenCustomWidget(widget), value_age_(10) { startTimer(100); }
    ~EigenRPCWidget() { stopTimer(); }
    virtual void valueChanged(const WidgetValueMessage *vm) {}
    virtual void timerCallback();
    void handleValueMessage(const WidgetValueMessage *vm) { value_age_ = 0; valueChanged(vm); startTimer(10000); }
private:
    int value_age_;
};

// Base class for single value OSC based custom widgets

class EigenOSCWidget : public EigenCustomWidget, public Timer
{
public:
    EigenOSCWidget(WidgetComponent *widget);
    ~EigenOSCWidget();
    
    void timerCallback();
    void handleValueMessage(const WidgetValueMessage *vm);
    void sendValue(const WidgetData &data);

    virtual void resetValue(bool latent, const WidgetData &current_value, const WidgetData &latent_value) {}
    virtual bool compareValues(const WidgetData &c, const WidgetData &l) { return c==l; }


private:
    WidgetData current_value_;
    WidgetData latent_value_;
    unsigned timeout_;

};

// Slider class with a latent value for indicating network latency

class EigenSlider : public Slider 
{
public:
    EigenSlider(const String &componentName);
    ~EigenSlider();

    int getLastX() { return lastX; }
    int getLastY() { return lastY; }
    int getLastWidth() { return lastWidth; }
    int getLastHeight() { return lastHeight; }

    void setLastBounds() 
    {
        lastX = getX();
        lastY = getY();
        lastWidth = getWidth();
        lastHeight = getHeight();
    }

    float getLatentValue() { return latentValue_; }
    void setLatentValue(float latentValue) { latentValue_ = latentValue; }
    virtual double proportionOfLengthToValue(double proportion);
    virtual double valueToProportionOfLength(double value);
    
    void mouseDown(const MouseEvent& e);
    void mouseDrag(const MouseEvent& e);

    void setWidget(WidgetComponent* widget);
    String getTooltip();
    
private: 
    double prop2val(double proportion);
    double val2prop(double value);

    int lastX;
    int lastY;
    int lastWidth;
    int lastHeight;

    float latentValue_;
    
    WidgetComponent* widget_;
};

// Button class with a latent value for indicating network latency

class EigenButton : public DrawableButton 
{
public:
    EigenButton(const String &buttonName, const ButtonStyle buttonStyle);
    
    void updateImages();
    
    bool getLastToggleState() { return lastState_; }
    void setLastToggleState(bool lastState) { lastState_ = lastState; }
    
    bool getLatentState() { return latentState_; }
    void setLatentState(bool latentState) { latentState_ = latentState; }

    void mouseDown(const MouseEvent& e);
    void mouseDrag(const MouseEvent& e);

    void setWidget(WidgetComponent* widget);
    
    String getTooltip();
    
private:
    bool lastState_;
    bool latentState_;

    WidgetComponent* widget_;
};

// Button class that triggers an action

class EigenTrigger : public DrawableButton 
{
public:
    EigenTrigger(const String &buttonName, const ButtonStyle buttonStyle);
    
    void updateImages();

    bool getLastToggleState() { return lastState_; }
    void setLastToggleState(bool lastState) { lastState_ = lastState; }
    
    unsigned getStatus() { return status_; }
    void setStatus(unsigned status) { status_ = status; }

    void mouseDown(const MouseEvent& e);
    void mouseDrag(const MouseEvent& e);

    void setWidget(WidgetComponent* widget);
    
    String getTooltip();
    
private:
    unsigned status_;
    bool lastState_;

    WidgetComponent* widget_;
};




#endif // __EIGEN_WIDGETS__
