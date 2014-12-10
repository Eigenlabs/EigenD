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

#ifndef __WIDGET_COMPONENT_H__
#define __WIDGET_COMPONENT_H__

#include "Network.h"
#include "juce.h"
#include "ResizableLabel.h"
#include "EigenWidgets.h"
#include "EigenLookAndFeel.h"
#include "CanvasComponent.h"
#include "MainComponent.h"

class MainComponent;
class OSCManager;
class XMLRPCManager;
#if STAGE_BUILD==DESKTOP
class AgentViewComponent;
#endif // STAGE_BUILD==DESKTOP
class ResizableLabel;

//==============================================================================

class WidgetComponent : public CanvasComponent, public SliderListener, public Button::Listener, public Timer, public MessageListener
#if STAGE_BUILD==DESKTOP
                        , public TooltipClient
#endif // STAGE_BUILD==DESKTOP
{
public:
    //==============================================================================

    WidgetComponent(XmlElement* sourceXML, MainComponent* mainComponent, const XmlElement *oldXml);
    ~WidgetComponent();

    //==============================================================================

    XmlElement* getXml() { return xml_; }

    void setWidgetName(String labelString);
    String getWidgetName();

    String getWidgetType() { return widgetType_; }
    String getDomainType() { return domainType_; }
    
    void setOSCPath(const String& path) { OSCPath_ = path; }
    String getOSCPath() { return OSCPath_; }

    void setUserRange(double userMin, double userStep, double userMax);
    void setUserStep(double userStep);
    double getUserMin();
    double getUserStep();
    double getUserMax();
    
#if STAGE_BUILD==DESKTOP
    void findHelp();
    void clearHelp() { helpFound_ = false; }
    const String getHelpDescription();
    String getTooltip();
#endif // STAGE_BUILD==DESKTOP

    //==============================================================================
    // enable state management
    void setConnected(bool connected);
    void setUserEnabled(bool userEnabled);
    bool getEnabled();
    bool getUserEnabled();
    bool getConnected();
    
    
    //==============================================================================

    void paint(Graphics& g);
    void resized();
    void buttonClicked(Button* button);
    void buttonStateChanged(Button *button);
    void sliderValueChanged(Slider *slider);
    
    void handleMessage(const Message& message);

    //==============================================================================
    // canvas component functions
    
    void setBoundsCanvasUnits(float x, float y, float width, float height);
    void setPositionCanvasUnits(float x, float y);
    void setTextHeightCanvasUnits(float textHeight);
    void setTextHeightPixels(int textHeightPixels) { textHeightPixels_ = textHeightPixels; }
    void setTextPosition(const String& textPosition);
    
    //==============================================================================

    void fitToRectangle(int x, int y, int width, int height, float graphicsWidth, float graphicsHeight, 
                        float& graphicsX, float& graphicsY, float& graphicsScale);

    void resizeLabel(int textHeightPixels, int yOff);
    void resizeWidget(float vx, float vy, float vw, float vh);
    void resizeSubComponents(int textHeightPixels, float vx, float vy, float vw, float vh);
    
    // TODO: remove
    void componentMovedOrResized(Component& component,
                                 bool wasMoved,
                                 bool wasResized);

    void setWidgetType(const String &type);

    bool hasPreferredSize(int *width, int *height);

   
    //==============================================================================
    
    void setValue(double value);
    void setLatentValue(double latentValue);

    void timerCallback();
    
    //==============================================================================    

    void setWidgetInEigenD();
    void setIndex(int widgetIndex) { widgetIndex_ = widgetIndex; }
    int getIndex() { return widgetIndex_; }

    bool doServerRpc(const char *method, const String &args, String &result);

    void requestCurrentValue();
    void sendValue(const WidgetData &value);
    
    //==============================================================================    

    friend class OSCManager;
    
private:

    //==============================================================================

    float graphicsX;
    float graphicsY;
    float graphicsWidth;
    float graphicsHeight;
    float graphicsScale;

    Component* widget;
    ResizableLabel* label;
    ResizableLabel* valueLabel;
    
    String widgetType_;
    String domainType_;
    
    OSCManager* OSCManager_;
    XMLRPCManager* XMLRPCManager_;
#if STAGE_BUILD==DESKTOP
    AgentViewComponent* agentView_;
#endif // STAGE_BUILD==DESKTOP
    String OSCPath_;
    
    XmlElement* xml_;
    XmlElement* xmlBounds_;
    XmlElement* xmlDomain_;
    XmlElement* xmlTip_;

    bool helpFound_;
    String tipText_;
    String helpText_;
    
    int widgetIndex_;

    double latentValue_;
    bool resendValue_;
    bool resendBlink_;

    // enabled = user enabled and connected
    bool enabled_;
    // user enable setting
    bool userEnabled_;
    // connected in EigenD
    bool connected_;

    double userMin_;
    double userStep_;
    double userMax_;
    
};


#endif // __WIDGET_COMPONENT_H__
