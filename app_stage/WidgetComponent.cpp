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

#include "WidgetComponent.h"


WidgetComponent :: WidgetComponent(XmlElement* sourceXML, MainComponent* mainComponent, const XmlElement *oldXml)
    : CanvasComponent(), Timer(),
      graphicsX(0), graphicsY(0), graphicsWidth(0), graphicsHeight(0), graphicsScale(0),
      widget(0), label(0), valueLabel(0),
      helpFound_(false),
      widgetIndex_(-1), latentValue_(0), resendValue_(false), resendBlink_(true),
      enabled_(true), userEnabled_(true), connected_(true)
{
    OSCManager_ = mainComponent->getOSCManager();
    XMLRPCManager_ = mainComponent->getXMLRPCManager();
#if STAGE_BUILD==DESKTOP
    agentView_ = mainComponent->getAgentViewComponent();
#endif // STAGE_BUILD==DESKTOP
    
    textHeightPixels_ = 32;
    
    setName("widget");

    //std::cout << "source xml: " << sourceXML->createDocument("") << std::endl;

    // source XML can either be an agent atom or from a previously created widget

    // create xml for the new widget
    xml_ = new XmlElement("widget");
    
    // get widget metadata from agent xml
    xml_->setAttribute("name", sourceXML->getStringAttribute("name"));
    xml_->setAttribute("path", sourceXML->getStringAttribute("path"));
    xml_->setAttribute("address", sourceXML->getStringAttribute("address"));
    xml_->setAttribute("enabled", sourceXML->getStringAttribute("enabled"));
    xml_->setAttribute("widget", sourceXML->getStringAttribute("widget"));

    userEnabled_ = sourceXML->getBoolAttribute("userEnabled", true);
    xml_->setAttribute("userEnabled", userEnabled_);
    setEnabled(userEnabled_);

    String widgetType = xml_->getStringAttribute("widget");
                       
    //std::cout << sourceXML->getStringAttribute("type", "rotaryKnob");
    
    xmlBounds_ = new XmlElement("bounds");
    xml_->addChildElement(xmlBounds_);
    xmlBounds_->setAttribute("x", 0);
    xmlBounds_->setAttribute("y", 0);
    xmlBounds_->setAttribute("width", 0);
    xmlBounds_->setAttribute("height", 0);
    xmlBounds_->setAttribute("textHeight", 0);
    xmlBounds_->setAttribute("textPosition", "top");

    textPosition_ = "top";
    
    xmlDomain_ = new XmlElement("domain");
    xml_->addChildElement(xmlDomain_);
    XmlElement* sourceXMLDomain = sourceXML->getChildByName("domain");

    domainType_ = sourceXMLDomain->getStringAttribute("type");
    xmlDomain_->setAttribute("type", domainType_);
    xmlDomain_->setAttribute("full", sourceXMLDomain->getStringAttribute("full"));

    if(!widgetType.startsWith("custom-"))
    {
        if (domainType_=="bint" || domainType_=="bfloat" || domainType_=="bintn" || domainType_=="bfloatn")
        {
            xml_->setAttribute("type", sourceXML->getStringAttribute("type", "rotaryKnob"));

            xmlDomain_->setAttribute("min", sourceXMLDomain->getDoubleAttribute("min"));
            xmlDomain_->setAttribute("max", sourceXMLDomain->getDoubleAttribute("max"));

            double min = 0, max = 1;

            min = xmlDomain_->getDoubleAttribute("min");
            max = xmlDomain_->getDoubleAttribute("max");

            xmlDomain_->setAttribute("userMin", sourceXMLDomain->getDoubleAttribute("userMin", min));
            xmlDomain_->setAttribute("userStep", sourceXMLDomain->getDoubleAttribute("userStep", (max-min)/100.0));
            xmlDomain_->setAttribute("userMax", sourceXMLDomain->getDoubleAttribute("userMax", max));
            xmlDomain_->setAttribute("distribution", sourceXMLDomain->getStringAttribute("distribution","linear"));

            userMin_ = xmlDomain_->getDoubleAttribute("userMin");
            userStep_ = xmlDomain_->getDoubleAttribute("userStep");
            userMax_ = xmlDomain_->getDoubleAttribute("userMax");

            if(oldXml)
            {
                xml_->setAttribute("type", oldXml->getStringAttribute("type", "rotaryKnob"));

                XmlElement *oldXmlDomain = oldXml->getChildByName("domain");

                if(oldXmlDomain && oldXmlDomain->getStringAttribute("type")==domainType_)
                {
                    userMin_ = xmlDomain_->getDoubleAttribute("userMin");
                    userStep_ = xmlDomain_->getDoubleAttribute("userStep");
                    userMax_ = xmlDomain_->getDoubleAttribute("userMax");

                }
            }
        }
        else if (domainType_=="bool")
        {
            xml_->setAttribute("type", sourceXML->getStringAttribute("type", "toggleButton"));
        }
        else if (domainType_=="trigger")
        {
            xml_->setAttribute("type", sourceXML->getStringAttribute("type", "triggerButton"));
        }
    }
    else
    {
        xml_->setAttribute("type", sourceXML->getStringAttribute("type", widgetType));
    }

    // set up child components
    String labelText = xml_->getStringAttribute("name");
    label = new ResizableLabel("label_"+labelText, labelText);
    label->setEditable(false, false, false);
    label->setColour(ResizableLabel::textColourId, Colour(0xff5d5d5d));
    addAndMakeVisible(label);

    label->setJustificationType(Justification::centred);
    //label->setVisible(false);

    label->setWidget(this);

    
    
    
    valueLabel = new ResizableLabel("value_"+labelText, "0.0");
    addAndMakeVisible(valueLabel);
#if STAGE_BUILD==DESKTOP
    valueLabel->setFont(Font(1, Font::bold));
#elif STAGE_BUILD==IOS
    valueLabel->setFont(Font().withTypefaceStyle("Condensed Bold"));
#endif // STAGE_BUILD
    valueLabel->setJustificationType(Justification::centred);
    valueLabel->setEditable(false, false, false);
    //valueLabel->setVisible(false);
    
    setRepaintsOnMouseActivity(true);

    // add itself to the OSC manager
    OSCPath_ = xml_->getStringAttribute("path");
    OSCPath_ = OSCPath_.replaceCharacter(' ', '_');
    OSCManager_->addWidgetToPath(OSCPath_, this);

    // setting widget type happens last
    setWidgetType(xml_->getStringAttribute("type"));
    
    
}

WidgetComponent :: ~WidgetComponent()
{
    OSCManager_->removeWidgetFromPath(OSCPath_, this);

    // TODO: handle removing xml elements
    
    
    deleteAllChildren();

}


void WidgetComponent :: setBoundsCanvasUnits(float x, float y, float width, float height)
{
    CanvasComponent::setBoundsCanvasUnits(x,y,width,height);
    
    xmlBounds_->setAttribute("x", x_);
    xmlBounds_->setAttribute("y", y_);
    xmlBounds_->setAttribute("width", width_);
    xmlBounds_->setAttribute("height", height_);

    // send changes to eigenD
    setWidgetInEigenD();
    
}


void WidgetComponent :: setPositionCanvasUnits(float x, float y)
{
    CanvasComponent::setPositionCanvasUnits(x,y);
    
    xmlBounds_->setAttribute("x", x_);
    xmlBounds_->setAttribute("y", y_);

    // send changes to eigenD
    setWidgetInEigenD();
    
}


void WidgetComponent :: setTextHeightCanvasUnits(float textHeight)
{
    CanvasComponent::setTextHeightCanvasUnits(textHeight);
    
    xmlBounds_->setAttribute("textHeight", textHeight_);

    // send changes to eigenD
    setWidgetInEigenD();

}

void WidgetComponent :: setTextPosition(const String& textPosition)
{
    CanvasComponent::setTextPosition(textPosition);
    
    xmlBounds_->setAttribute("textPosition", textPosition_);
    
    repaint();
    resized();
    
    // send changes to eigenD
    setWidgetInEigenD();
}



void WidgetComponent :: setWidgetName(String name)
{
    if(label)
        label->setText(name, dontSendNotification);
    
    xml_->setAttribute("name", name);

    // send changes to eigenD
    setWidgetInEigenD();

}


String WidgetComponent :: getWidgetName()
{
    return xml_->getStringAttribute("name");
}

void WidgetComponent::setWidgetType(const String &widgetType)
{
    float widgetValue = 0;
    //std::cout << "Set ComponentType " << widgetType << std::endl; std::cout.flush();
    
    // remove existing component
    if (widget) 
    {
        if(widgetType_=="horizontalSlider"|| widgetType_=="verticalSlider"||
           widgetType_=="rotaryKnob" || widgetType_=="incDecButtons")
            widgetValue = (float)(((EigenSlider*)widget)->getValue());
        
        removeChildComponent(widget);
        delete widget;
    }

    widgetType_ = widgetType;

    
    EigenSlider* sliderWidget;
    EigenButton* buttonWidget;
    EigenTrigger* triggerWidget;
    //EigenLookAndFeel* lookAndFeel;

    // get meta data from xml

    double min = xmlDomain_->getDoubleAttribute("userMin");
    double max = xmlDomain_->getDoubleAttribute("userMax");
    double step = xmlDomain_->getDoubleAttribute("userStep");
    int steps = (int)((max-min)/step);
    String labelText = xml_->getStringAttribute("name");
    
    if(widgetType=="horizontalSlider"|| widgetType=="verticalSlider"|| widgetType=="rotaryKnob" || widgetType=="incDecButtons")
    {
        widget = new EigenSlider("widget_"+labelText);
        addChildComponent(widget);
        sliderWidget = dynamic_cast<EigenSlider*>(widget);

        if(widgetType=="horizontalSlider") sliderWidget->setSliderStyle(Slider::LinearHorizontal);
        if(widgetType=="verticalSlider") sliderWidget->setSliderStyle(Slider::LinearVertical);
#if STAGE_BUILD==DESKTOP
        if(widgetType=="rotaryKnob") sliderWidget->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
#else
        if(widgetType=="rotaryKnob") sliderWidget->setSliderStyle(Slider::Rotary);
#endif
        if(widgetType=="incDecButtons") sliderWidget->setSliderStyle(Slider::IncDecButtons);

        sliderWidget->setRange(min,max,step);
        sliderWidget->setMouseDragSensitivity(jmax(250,steps));

        if(0 == xmlDomain_->getStringAttribute("distribution","linear").compare("tapered"))
        {
            //std::cout << "distribution is tapered" << std::endl;
            sliderWidget->setSkewFactor(3);
        }

        // no text box, draw it manually
        sliderWidget->setTextBoxStyle(EigenSlider::NoTextBox, true, 72, 16);
        // set up rotary start and end positions to exactly match widget design
        sliderWidget->setRotaryParameters(3.141f+0.595f, 2.0f*3.141f+3.141f-0.595f, false);
        sliderWidget->setVisible(true);
        sliderWidget->toBack();
        sliderWidget->addListener(this);
        setValue(widgetValue);
        setLatentValue(latentValue_);
        // setting textbox left helps keep the inc dec button together as it
        // adjusts slightly the image size of the slider buttons
        sliderWidget->setTextBoxStyle(Slider::TextBoxLeft, false, 0, 0);
        sliderWidget->setWidget(this);
        valueLabel->setVisible(true);
    }
    else if(widgetType=="toggleButton")
    {
        widget = new EigenButton("widget_"+labelText, DrawableButton::ImageFitted);
        addChildComponent(widget);
        //lookAndFeel = dynamic_cast<EigenLookAndFeel*> (&LookAndFeel::getDefaultLookAndFeel());
        buttonWidget = dynamic_cast<EigenButton*>(widget);
        buttonWidget->updateImages();
        buttonWidget->setClickingTogglesState(true);
        buttonWidget->setColour(DrawableButton::backgroundColourId, Colour(0));
        buttonWidget->setColour(DrawableButton::backgroundOnColourId, Colour(0));
        buttonWidget->setVisible(true);
        buttonWidget->toBack();
        buttonWidget->addListener(this);
        buttonWidget->setWidget(this);
        valueLabel->setVisible(false);
    }
    else if(widgetType=="triggerButton")
    {
        widget = new EigenTrigger("widget_"+labelText, DrawableButton::ImageFitted);
        addChildComponent(widget);
        //lookAndFeel = dynamic_cast<EigenLookAndFeel*> (&LookAndFeel::getDefaultLookAndFeel());
        triggerWidget = dynamic_cast<EigenTrigger*>(widget);
        triggerWidget->updateImages();
        triggerWidget->setClickingTogglesState(true);
        triggerWidget->setColour(DrawableButton::backgroundColourId, Colour(0));
        triggerWidget->setColour(DrawableButton::backgroundOnColourId, Colour(0));
        triggerWidget->setVisible(true);
        triggerWidget->toBack();
        triggerWidget->addListener(this);
        triggerWidget->setWidget(this);
        valueLabel->setVisible(false);
    }
#if STAGE_BUILD==DESKTOP
    else
    {
        //std::cout << "creating custom " << widgetType << std::endl; std::cout.flush();
        widget = EigenCustomWidget::widgetFactory(widgetType,this);
        addChildComponent(widget);
        //std::cout << "created custom" << std::endl; std::cout.flush();
        widget->setVisible(true);
        valueLabel->setVisible(false);
    }
#endif

    resized();
    repaint();

    // update model
    
    xml_->setAttribute("type", getWidgetType());

    // send changes to eigenD
    setWidgetInEigenD();
    
    OSCManager_->requestWidgetValue(OSCPath_);

}


//==============================================================================


void WidgetComponent::setUserRange(double userMin, double userStep, double userMax)
{
    userMin_ = userMin;
    userStep_ = userStep;
    userMax_ = userMax;

    double min = xmlDomain_->getDoubleAttribute("min", 0);
    double max = xmlDomain_->getDoubleAttribute("max", 1);
    
    if(userMin_ < min)
        userMin_ = min;
    
    if(userMax_ > max)
        userMax_ = max;

    if(userMax_ < userMin_)
        userMax_ = userMin_;
    
    if(userStep_ < 0.000001)
        userStep_ = 0.000001;
    
    if(userStep_ > (userMax_ - userMin_))
        userStep_ = userMax_ - userMin_;
    
    
    if(widgetType_=="rotaryKnob" || widgetType_=="horizontalSlider" || widgetType_=="verticalSlider" || widgetType_=="incDecButtons")
    {
        EigenSlider* slider = dynamic_cast<EigenSlider*>(widget);
        slider->setRange(userMin_, userMax_, userStep_);

        // make sure there are enough pixels for all the steps for the slider - at least 250
        int steps = (int)((userMax_-userMin_)/userStep_);
        slider->setMouseDragSensitivity(jmax(250,steps));
        
        // TODO: make label update also
        if(slider->getValue()<userMin_)
            slider->setValue(userMin_, sendNotificationSync);
        if(slider->getValue()>userMax_)
            slider->setValue(userMax_, sendNotificationSync);
    }
    
    // update model
    xmlDomain_->setAttribute("userMin", userMin_);
    xmlDomain_->setAttribute("userStep", userStep_);
    xmlDomain_->setAttribute("userMax", userMax_);
    
    // send changes to eigenD
    setWidgetInEigenD();
    
}

void WidgetComponent::setUserStep(double userStep)
{
    userStep_ = userStep;
 
    if(userStep_ < 0.000001)
        userStep_ = 0.000001;
    
    if(userStep_ > (userMax_ - userMin_))
        userStep_ = userMax_ - userMin_;
    
    if(widgetType_=="rotaryKnob" || widgetType_=="horizontalSlider" || widgetType_=="verticalSlider" || widgetType_=="incDecButtons")
    {
        EigenSlider* slider = dynamic_cast<EigenSlider*>(widget);
        slider->setRange(userMin_, userMax_, userStep_);

        // make sure there are enough pixels for all the steps for the slider - at least 250
        int steps = (int)((userMax_-userMin_)/userStep_);
        slider->setMouseDragSensitivity(jmax(250,steps));

        //slider->setValue(slider->getValue(), true, true);
        double value = slider->getValue();
        slider->setValue(userMin_, dontSendNotification);
        slider->setValue(value, sendNotificationSync);
        setValue(value);
    }
    
    // update model
    xmlDomain_->setAttribute("userStep", userStep_);
    
    // send changes to eigenD
    setWidgetInEigenD();
    
}


double WidgetComponent::getUserMin()
{
    return userMin_;
}

double WidgetComponent::getUserStep()
{
    return userStep_;
}

double WidgetComponent::getUserMax()
{
    return userMax_;
}


//==============================================================================


#if STAGE_BUILD==DESKTOP
void WidgetComponent::findHelp()
{
    // use the path to determine the tip and help strings from the agent tree
    XmlElement* agentElement = agentView_->getElementByPath(xml_->getStringAttribute("path"));

    
    if(agentElement)
    {
        helpFound_ = true;

        XmlElement* helpElement = agentElement->getChildByName("help");
        XmlElement* tipElement = agentElement->getChildByName("tip");

        // set help
        if(helpElement)
            helpText_ = helpElement->getAllSubText();
        else
            helpText_ = "No description available.";

        // set tool tip
        tipText_ = xml_->getStringAttribute("path");
        tipText_ = tipText_.replace("_", " ");
        tipText_ += "\n\n";
        
        String tip;
        if(tipElement)
            tip = tipElement->getAllSubText();
        else
            if(helpElement)
                tip = helpText_;
            else
                tip = "No description available.";
        
        // tool tip is combination of tool tip from database and the full name
        tipText_ += tip;

        //std::cout << "help found\n";

        if(dynamic_cast<SettableTooltipClient*>(widget))
            dynamic_cast<SettableTooltipClient*>(widget)->setTooltip(tipText_);
        if(label)
            label->setTooltip(tipText_);
        
    }
    
}

const String WidgetComponent::getHelpDescription()
{
    if(!helpFound_)
    {
        findHelp();
        if (helpFound_) 
        {
            return helpText_;
        }
        else
        {
            return "Help is currently loading. Help will be ready when the setup completes loading.";
        }
    }
    else 
    {
        return helpText_;
    }
}



String WidgetComponent::getTooltip()
{
    
    if(!helpFound_)
    {
        findHelp();
        if (helpFound_) 
        {
            return tipText_;
        }
        else
        {
            return "";
        }
    }
    else 
    {
        if(dynamic_cast<SettableTooltipClient*>(widget))
            dynamic_cast<SettableTooltipClient*>(widget)->setTooltip(tipText_);
        if(label)
            label->setTooltip(tipText_);
        
        return tipText_;
    }

}

#endif // STAGE_BUILD==DESKTOP


void WidgetComponent::paint (Graphics& g)
{
    
    //g.setColour(Colour(0xffff0000));
    //g.drawRect((int)graphicsX, (int)graphicsY, (int)(graphicsScale*graphicsWidth), (int)(graphicsScale*graphicsHeight), 1);

    // paint background of inc dec buttons because look and feel doesn't allow for buttons plus
    // a background images below the buttons
    if(widgetType_=="incDecButtons")
    {
        EigenLookAndFeel* lookAndFeel;
        lookAndFeel = dynamic_cast<EigenLookAndFeel*> (&LookAndFeel::getDefaultLookAndFeel());
        
        Drawable* plusMinusBack = lookAndFeel->getImage(EigenLookAndFeel::plusMinusBack);

        // scale and locate background, making small adjustment because the buttons as separated images
        // are slightly large overall than original combined image
        plusMinusBack->draw(g, 1, AffineTransform::translation(2,0).scaled(graphicsScale, graphicsScale).translated(graphicsX, graphicsY));
    }

}

void WidgetComponent::resized()
{
    // bounds for value label
    float vx = 0, vy = 0, vw = 0, vh = 0;

    // set dimensions of widgets
    if (widgetType_=="rotaryKnob")
    {
        graphicsWidth = 137.98f;
        graphicsHeight = 154.48f;
        vw = 90.f;
        vh = 24.f;
        vx = graphicsWidth/2.0f-vw/2.0f;
        vy = graphicsHeight-vh-5.5f;
        resizeSubComponents(textHeightPixels_, vx, vy, vw, vh);
    }
    else if (widgetType_== "horizontalSlider")
    {
        graphicsWidth = 237.0f;
        graphicsHeight = 67.1f+8.0f;
        vw = 90.f;
        vh = 24.f;
        vx = graphicsWidth/2.0f-vw/2.0f;
        vy = graphicsHeight-vh-5.5f;
        resizeSubComponents(textHeightPixels_, vx, vy, vw, vh);
    }
    else if (widgetType_=="verticalSlider")
    {
        graphicsWidth = 68.0f;
        graphicsHeight = 257.0f;
        vw = 90.f*(205.0f/257.0f);
        vh = 20.5f*(205.0f/257.0f);
        vx = graphicsWidth/2.0f-vw/2.0f;
        vy = graphicsHeight-vh-7;

        // fit widget to available area maintaining aspect ratio
        resizeLabel(textHeightPixels_, 0);
        
        // this fit leaves a gap for the Juce slider thumb track at the top and bottom
        // want to remove the top gap, so adjust the height now by a fraction of the thumb radius
        int thumbRadius = 0.67*(graphicsHeight*graphicsScale)/7.2;

        // fit again, adjusting for thumb radius at top of slider
        resizeLabel(textHeightPixels_, thumbRadius);

        resizeWidget(vx, vy, vw, vh);

    }
    else if (widgetType_=="incDecButtons")
    {
        graphicsWidth = 67.870f*2.0f;
        graphicsHeight = 69.021f;
        vw = 90.f;
        vh = 20.5f;
        vx = graphicsWidth/2.0f-vw/2.0f;
        vy = graphicsHeight-vh-6.0f;
        
        // draw elements
        resizeLabel(textHeightPixels_, 0);
        
        // draw +- widget smaller than full height so that buttons do not fill entire area
        // (to keep clickable area to the area of the button graphics)
        graphicsHeight = 40.05f;
        resizeWidget(vx, vy, vw, vh);
        
    }
    else if (widgetType_=="toggleButton")
    {
        graphicsWidth = 130.0f;
        graphicsHeight = 38.0f;
        resizeSubComponents(textHeightPixels_, vx, vy, vw, vh);
    }
    else if (widgetType_=="triggerButton")
    {
        graphicsWidth = 130.0f;
        graphicsHeight = 38.0f;
        resizeSubComponents(textHeightPixels_, vx, vy, vw, vh);
    }
#if STAGE_BUILD==DESKTOP
    else
    {
        EigenCustomWidget *cwidget = dynamic_cast<EigenCustomWidget *>(widget);

        if(cwidget)
        {
            graphicsWidth = cwidget->getGraphicsWidth();
            graphicsHeight = cwidget->getGraphicsHeight();
        }
        else
        {
            graphicsWidth = 100.0f;
            graphicsHeight = 100.0f;
        }

        resizeSubComponents(textHeightPixels_, vx, vy, vw, vh);
    }
#endif // STAGE_BUILD
    
}

bool WidgetComponent::hasPreferredSize(int *width, int *height)
{
    EigenCustomWidget *cwidget = dynamic_cast<EigenCustomWidget *>(widget);

    if(cwidget)
    {
        if(width) *width = cwidget->getGraphicsWidth();
        if(height) *height = cwidget->getGraphicsHeight();
        return true;
    }

    return false;
}

void WidgetComponent::resizeSubComponents(int textHeightPixels, float vx, float vy, float vw, float vh)
{
    resizeLabel(textHeightPixels, 0);
    resizeWidget(vx,vy,vw,vh);
}

void WidgetComponent::resizeLabel(int textHeightPixels, int yOff)
{
    // fit widget to available area according to label maintaining aspect ratio and position label
    if(textPosition_=="top")
    {
        fitToRectangle(0, textHeightPixels-yOff, getWidth(), getHeight()-(textHeightPixels-yOff), graphicsWidth, graphicsHeight, graphicsX, graphicsY, graphicsScale);
        label->setBounds(0,0,getWidth(),textHeightPixels);
        label->setVertical(false, false);
    }
    if(textPosition_=="bottom")
    {
        fitToRectangle(0, 0-yOff, getWidth(), getHeight()-(textHeightPixels-yOff), graphicsWidth, graphicsHeight, graphicsX, graphicsY, graphicsScale);
        label->setBounds(0,getHeight()-textHeightPixels,getWidth(),textHeightPixels);
        label->setVertical(false, false);
    }
    if(textPosition_=="left")
    {
        fitToRectangle(textHeightPixels, 0-yOff, getWidth()-textHeightPixels, getHeight()+yOff, graphicsWidth, graphicsHeight, graphicsX, graphicsY, graphicsScale);
        label->setBounds(0,0,textHeightPixels,getHeight());
        label->setVertical(true, true);
    }
    if(textPosition_=="right")
    {
        fitToRectangle(0, 0-yOff, getWidth()-textHeightPixels, getHeight()+yOff, graphicsWidth, graphicsHeight, graphicsX, graphicsY, graphicsScale);
        label->setBounds(getWidth()-textHeightPixels,0,textHeightPixels,getHeight());
        label->setVertical(true, false);
    }
    
}

void WidgetComponent::resizeWidget(float vx, float vy, float vw, float vh)
{
    
    // draw widget
    widget->setBounds((int)graphicsX,(int)graphicsY,(int)(graphicsWidth*graphicsScale),(int)(graphicsHeight*graphicsScale));
    
    if(widgetType_=="rotaryKnob" || widgetType_=="horizontalSlider" || widgetType_=="verticalSlider" || widgetType_=="incDecButtons")
    {
        // apply transforms to value bounds
        vx = (vx * graphicsScale)+graphicsX;
        vy = (vy * graphicsScale)+graphicsY;
        vw = (vw * graphicsScale);
        vh = (vh * graphicsScale);
        std::cout << "ww=" << widget->getWidth() << " wh=" << widget->getHeight() << " vx=" << (int)vx << " vy=" << (int)vy << " vw=" << (int)vw << " vh=" << (int)vh << std::endl;
        valueLabel->setBounds((int)vx,(int)vy,(int)vw,(int)vh);
    }
    
}

void WidgetComponent::fitToRectangle(int x, int y, int width, int height, float graphicsWidth, float graphicsHeight, 
                                     float& graphicsX, float& graphicsY, float& graphicsScale)
{
    // calculate the offset and scaling (graphicsX,graphicsY,graphicsScale) to transform an area of a certain width and height into another area whilst maintaining aspect ratio
    
    float graphicsRatio = graphicsWidth/graphicsHeight;
    float viewRatio = (float)width/(float)height;
    
    if(graphicsRatio>=viewRatio)
    {
        graphicsScale = (float)width/graphicsWidth;
        graphicsX = (float)x;
        graphicsY = (float)y+((float)height-(graphicsHeight*graphicsScale))/2;
    }
    else 
    {
        graphicsScale = height/graphicsHeight;
        graphicsX = (float)x+((float)width-(graphicsScale*graphicsWidth))/2;
        graphicsY = (float)y;
    }
    
}


//==============================================================================
//
// OSC management functions
//
//==============================================================================

void WidgetComponent::sliderValueChanged(Slider *slider)
{
    double value = slider->getValue();
    
    // slider listener callback to update the value label
    //valueLabel->setText(slider->getTextFromValue(value), sendNotification);

    resendBlink_ = true;
    
    // wait for latent reply before indicating that value late and retrying
    startTimer(50);
    

    resendValue_ = false;
    
    // send osc value
    OSCManager_->sendFloat(OSCPath_, (float)value);

    // no need to keep resending as the state is same as desired state
    if (value==dynamic_cast<EigenSlider*> (slider)->getLatentValue())
    {
        stopTimer();
        valueLabel->setColour(Label::textColourId, Colour(0xff000000));
    }
        
}

void WidgetComponent::sendValue(const WidgetData &value)
{
    OSCManager_->sendValue(OSCPath_, value);
}

void WidgetComponent::buttonClicked(Button *button)
{
	buttonStateChanged(button);

}

void WidgetComponent::buttonStateChanged(Button *button)
{
	if (widgetType_=="toggleButton")
	{
        EigenButton* eigenButton = dynamic_cast<EigenButton*>(button);

        bool state = eigenButton->getToggleState();
        bool lastState = eigenButton->getLastToggleState();
        bool latentState = eigenButton->getLatentState();

        resendBlink_ = true;

        // send osc value if it's changed
        if(state!=lastState)
        {
            double value = 1.0?state:0.0;
            OSCManager_->sendFloat(OSCPath_, (float)value);
            
            eigenButton->updateImages();
            
            // wait for latent reply before retrying
            startTimer(50);
            
        }
        
        if (state==latentState) 
        {
            // no need to keep resending as the state is same as desired state
            stopTimer();
        }
        
        eigenButton->setLastToggleState(state);
	}
	else if (widgetType_=="triggerButton")
	{
        EigenTrigger* eigenTrigger = dynamic_cast<EigenTrigger*>(button);
        bool state = eigenTrigger->getToggleState();
        bool lastState = eigenTrigger->getLastToggleState();
        if (state != lastState && button->getToggleState())
        {
            OSCManager_->sendFloat(OSCPath_, (float)1.0);
            eigenTrigger->updateImages();
        }
        
        eigenTrigger->setLastToggleState(state);
	}
}


void WidgetComponent::timerCallback()
{
    // set label red to indicate latent value
    if (resendBlink_)
        valueLabel->setColour(Label::textColourId, Colour(0xffbb4d3f));
    else
        valueLabel->setColour(Label::textColourId, Colour(0xffffffff));

    resendBlink_ = !resendBlink_;
    
    if (resendValue_) 
    {
        double value = 0.0;
        
        // resend value
        if(widgetType_=="rotaryKnob" || widgetType_=="horizontalSlider" || widgetType_=="verticalSlider" || widgetType_=="incDecButtons")
        {
            value = dynamic_cast<EigenSlider*>(widget)->getValue();
            OSCManager_->sendFloat(OSCPath_, (float)value);
        }
        else if(widgetType_=="toggleButton")
        {
            value = 1.0?(dynamic_cast<EigenButton*>(widget)->getToggleState()):0.0;
            OSCManager_->sendFloat(OSCPath_, (float)value);
        }

        //std::cout << "resending " << OSCPath_ << " " << value << "\n";
    }
    
    stopTimer();

    // retry sending
    startTimer(500);
    resendValue_ = true;
}

void WidgetComponent::handleMessage(const Message& message)
{
    EigenCustomWidget *cw = dynamic_cast<EigenCustomWidget *>(widget);

    // handle OSC message
    const WidgetValueMessage *vm = dynamic_cast<const WidgetValueMessage *>(&message);

    if(vm)
    {
        if(cw)
        {
            std::cout << "received message\n";
            cw->handleValueMessage(vm);
            return;
        }

        if(vm->data.type_=='f')
        {
            //std::cout << "received float " << vm->float_ << std::endl;
            if(!(widget->isMouseButtonDown()))
            {
                setValue(vm->data.float_);
            }
            
            setLatentValue(vm->data.float_);
        }

        return;
    }

    const WidgetConnectedMessage *cm = dynamic_cast<const WidgetConnectedMessage *>(&message);

    if(cm)
    {
        setConnected(cm->isConnected_);
        return;
    }
}

void WidgetComponent::setConnected(bool connected)
{
    EigenCustomWidget *cw = dynamic_cast<EigenCustomWidget *>(widget);

    connected_ = connected;
    enabled_ = userEnabled_ && connected_;

    if(cw)
    {
        cw->enableChanged();
    }
    
    setEnabled(enabled_);
    repaint();
}

void WidgetComponent::setUserEnabled(bool userEnabled)
{
    EigenCustomWidget *cw = dynamic_cast<EigenCustomWidget *>(widget);

    userEnabled_ = userEnabled;
    enabled_ = userEnabled_ && connected_;

    if(cw)
    {
        cw->enableChanged();
    }
    
    setEnabled(enabled_);
    repaint();

    xml_->setAttribute("userEnabled", userEnabled_);

    setWidgetInEigenD();
}

bool WidgetComponent::getEnabled()
{
    return enabled_;
}

bool WidgetComponent::getUserEnabled()
{
    return userEnabled_;
}

bool WidgetComponent::getConnected()
{
    return connected_;
}

void WidgetComponent::setValue(double value)
{
    if(widgetType_=="rotaryKnob" || widgetType_=="horizontalSlider" || widgetType_=="verticalSlider" || widgetType_=="incDecButtons")
    {
        dynamic_cast<EigenSlider*>(widget)->setValue(value, dontSendNotification);
        // value label is set from set value
        //valueLabel->setText(dynamic_cast<EigenSlider*>(widget)->getTextFromValue(value), sendNotification);
        // note OSC value not sent here
        if(value!=dynamic_cast<EigenSlider*>(widget)->getLatentValue())
            valueLabel->setColour(Label::textColourId, Colour(0xffbb4d3f));
        else
            valueLabel->setColour(Label::textColourId, Colour(0xff000000));

        valueLabel->repaint();
    }
    else if(widgetType_=="toggleButton")
    {
        EigenButton* buttonWidget = dynamic_cast<EigenButton*>(widget);
        buttonWidget->setToggleState(value==1.0,dontSendNotification);
        buttonWidget->updateImages();
    }
    else if(widgetType_=="triggerButton")
    {
        EigenTrigger* triggerWidget = dynamic_cast<EigenTrigger*>(widget);
        triggerWidget->setToggleState(false,dontSendNotification);
        triggerWidget->setStatus(value);
        triggerWidget->updateImages();
    }
}

void WidgetComponent::setLatentValue(double value)
{
    double roundedSetValue = 0, roundedLatentValue = 0;

    EigenSlider* slider; 
    double interval;

    latentValue_ = value;

    if(widgetType_=="rotaryKnob" || widgetType_=="horizontalSlider" || widgetType_=="verticalSlider")
    {
        dynamic_cast<EigenSlider*>(widget)->setLatentValue((float)(value));
        // value label is set from latent value 
        valueLabel->setText(dynamic_cast<EigenSlider*>(widget)->getTextFromValue(value), sendNotification);

        slider = dynamic_cast<EigenSlider*>(widget);
        interval = slider->getInterval();

        roundedSetValue = (double)roundDoubleToInt(dynamic_cast<EigenSlider*>(widget)->getValue()/interval)*interval;
        roundedLatentValue = (double)roundDoubleToInt(value/interval)*interval;
        
        stopTimer();
        if(roundedSetValue!=roundedLatentValue)
            valueLabel->setColour(Label::textColourId, Colour(0xffbb4d3f));
        else
            valueLabel->setColour(Label::textColourId, Colour(0xff000000));
        widget->repaint();
    }
    else if(widgetType_=="incDecButtons")
    {
        // value label is set from latent value 
        valueLabel->setText(dynamic_cast<EigenSlider*>(widget)->getTextFromValue(value), sendNotification);

        slider = dynamic_cast<EigenSlider*>(widget);
        interval = slider->getInterval();

        roundedSetValue = (double)roundDoubleToInt(dynamic_cast<EigenSlider*>(widget)->getValue()/interval)*interval;
        roundedLatentValue = (double)roundDoubleToInt(value/interval)*interval;
        
        stopTimer();

        if(roundedSetValue!=roundedLatentValue)
            valueLabel->setColour(Label::textColourId, Colour(0xffbb4d3f));
        else
            valueLabel->setColour(Label::textColourId, Colour(0xff000000));

    }
    else if(widgetType_=="toggleButton")
    {
        EigenButton* buttonWidget = dynamic_cast<EigenButton*>(widget);

        buttonWidget->setLatentState(value==1.0);
        buttonWidget->updateImages();
        
        stopTimer();
    }
}



//==============================================================================
//
// XMLRPC management functions
//
//==============================================================================

void WidgetComponent::setWidgetInEigenD()
{
    if(widgetIndex_ < 0)
    {
        return;
    }

    // sends this widget to eigenD
    XmlRpcValue params, result;

    WidgetViewComponent* parent = (findParentComponentOfClass<WidgetViewComponent>());
    
    if(parent)
    {   
        parent->incSessionChanges();
        
        params.clear();
        params[0] = parent->getTabIndex();
        params[1] = widgetIndex_;
        params[2] = XMLRPCManager::juceToStdString(xml_->createDocument("", false, false));
        try {
            XMLRPCManager_->execute("setWidget", params, result,true);
        }
        catch (XMLRPCAbortException e) {
            // app is shutting down after an xmlrpc has blocked so bail
            return;
        }
    }
    
}

bool WidgetComponent::doServerRpc(const char *method, const String &args, String &result)
{
    XmlRpcValue rparams, rresult;

    WidgetViewComponent* parent = (findParentComponentOfClass<WidgetViewComponent>());
    
    if(parent)
    {   
        rparams.clear();
        rparams[0] = parent->getTabIndex();
        rparams[1] = widgetIndex_;
        rparams[2] = method;
        rparams[3] = XMLRPCManager::juceToStdString(args);

        try {
            std::cout << "Calling rpc " << method << " " << args;

            if(!XMLRPCManager_->execute("widgetRpc", rparams, rresult,false))
            {
                std::cout << "Rpc " << method << " failed" << std::endl;
                requestCurrentValue();
                return false;
            }

            result = XMLRPCManager::stdToJuceString(rresult);
            std::cout << "Rpc " << method << " returns " << result << std::endl;
            return true;
        }
        catch (XMLRPCAbortException e) {
            // app is shutting down after an xmlrpc has blocked so bail
            throw;
        }
    }

    return false;
}

void WidgetComponent::requestCurrentValue()
{
    OSCManager_->requestWidgetValue(OSCPath_);
}







