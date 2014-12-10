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

#ifndef __WIDGET_VIEW_COMPONENT_H__
#define __WIDGET_VIEW_COMPONENT_H__

#include "Network.h"
#include "juce.h"
#include "WidgetOverlayComponent.h"
#include "CustomCursor.h"
#include "MainComponent.h"
#include "TabsFrameBox.h"
#include "ToolbarIcons.h"


class MainComponent;
class WidgetComponent;
class WidgetTabbedComponent;
class WidgetOverlayComponent;
class WidgetViewComponent;
class XMLRPCManager;

class CanvasAreaComponent : public Component, public DragAndDropTarget
{
public:
    CanvasAreaComponent(WidgetViewComponent* widgetView, MainComponent* mainComponent)
        : widgetView_(widgetView), mainComponent_(mainComponent), somethingIsBeingDraggedOver_(false) {}
    ~CanvasAreaComponent() {}

    //==============================================================================
    // overridden component functions

    void paint (Graphics& g);

    //==============================================================================
    // drag and drop target target functions
    
    bool isInterestedInDragSource (const SourceDetails& source);
    void itemDragEnter (const SourceDetails& source);
    void itemDragMove (const SourceDetails& source);
    void itemDragExit (const SourceDetails& source);
    void itemDropped (const SourceDetails& source);

private:
    WidgetViewComponent* widgetView_;
    MainComponent* mainComponent_;
    bool somethingIsBeingDraggedOver_;


};


class WidgetViewComponent : public Component
{
public:
    //==============================================================================
    WidgetViewComponent (const String& tabName, const int tabIndex, float canvasWidthAspect, float canvasHeightAspect, int canvasAspectMode,
                         bool editMode, CommandID toolMode, bool greyOutsideCanvas, const MouseCursor& mouseCursor, 
                         MainComponent* mainComponent, WidgetTabbedComponent* tabbedComponent);
    ~WidgetViewComponent();
/*
    WidgetViewComponent (const WidgetViewComponent& other)
    {
        
        editMode_ = other.editMode_;
        toolMode_ = other.toolMode_;
        editMouseCursor_ = other.editMouseCursor_;
        mainComponent_ = other.mainComponent_;
        canvasAspectMode_ = other.canvasAspectMode_;
        
        canvasWidthAspect_ = other.canvasWidthAspect_;
        canvasHeightAspect_ = other.canvasHeightAspect_;
        canvasRatio_ = other.canvasRatio_;
        
        canvasX_ = other.canvasX_;
        canvasY_ = other.canvasY_;
        canvasHeight_ = other.canvasHeight_;
        canvasWidth_ = other.canvasWidth_;
        
        greyOutsideCanvas_ = other.greyOutsideCanvas_;
        
        canvasArea_ = new CanvasAreaComponent(*other.canvasArea_);
        
        overlayComponents_ = other.overlayComponents_;
        
        frame_ = new TabsFrameBox(*other.frame_);
        
        xml_ = new XmlElement(*other.xml_);
        
        tabName_ = other.tabName_;
        tabIndex_ = other.tabIndex_;
        
        sessionChanges_ = other.sessionChanges_;
        
        XMLRPCManager_ = other.XMLRPCManager_;
        
        
    }
*/
    
    Component* getCanvasComponent() { return canvasArea_; }
    Array<WidgetOverlayComponent *>* getOverlayComponents() { return &overlayComponents_; }

    void clearHelp();
    
    //==============================================================================
    // mode functions
    
    void setEditMode(bool editMode);
    void setToolMode(CommandID toolMode, const MouseCursor& mouseCursor);

    //==============================================================================
    // widget functions

    void createWidget(const String& sourceDescription, int x, int y);
    WidgetOverlayComponent* addWidgetToCanvas(XmlElement* xml, float x, float y, float width, float height, float textHeight, String& textPosition, const XmlElement *oldxml, int *newIndex);
    void changeWidget(const String& sourceDescription, WidgetOverlayComponent* widget);
    void deleteWidget(WidgetOverlayComponent* widgetOverlay);

    
    //==============================================================================
    // overridden component functions
    
    void paint (Graphics& g);
    void resized();
    
    //==============================================================================
    // mouse listener functions

    void mouseDown(const MouseEvent &e);

    //==============================================================================
    // aspect ratio functions

    void setAspect(int canvasAspectMode, float canvasWidthAspect, float canvasHeightAspect);
    void getAspect(int& canvasAspectMode, float& canvasWidthAspect, float& canvasHeightAspect)
    {
        canvasAspectMode = canvasAspectMode_;
        canvasWidthAspect = canvasWidthAspect_;
        canvasHeightAspect = canvasHeightAspect_;
    }

    void getCanvasBounds(float& canvasX, float& canvasY, float& canvasWidth, float& canvasHeight)
    {
        canvasX = canvasX_;
        canvasY = canvasY_;
        canvasWidth = canvasWidth_;
        canvasHeight = canvasHeight_;
    }

    const Point<float> pixelsToCanvasUnits(float x, float y);
    const Point<float> canvasUnitsToPixels(float x, float y);
    float pixelsToCanvasUnits(float x);
    
    //==============================================================================
    // xml management functions

    void setTabIndex(int tabIndex) { tabIndex_ = tabIndex; }
    int getTabIndex() { return tabIndex_; }
    void setXml(XmlElement* xml);
    XmlElement* getXml();
    void getCanvasPropertiesFromXml();
    void getCanvasFromXml(XmlElement *tabXml);
    void getXmlFromCanvas();
    void clearCanvas();
    void setTabNameXml(const String& tabName);
    String getTabName() { return tabName_; }
    void setAspectXml();

    int getNumWidgets();
    XmlElement* getWidgetXml(int i);
    void addWidgetFromXml(XmlElement* widgetXml, bool setInEigenD = true);
    
    //==============================================================================
    // xmlrpc management functions

    void setTabInEigenD();

    void setSessionChanges(int sessionChanges)
    {
        sessionChanges_ = sessionChanges;
    }
    int getSessionChanges()
    {
        return sessionChanges_;
    }
    void incSessionChanges()
    {
        sessionChanges_++;
    }
    
    //==============================================================================
    // appearance functions
    
    void setGreyOutsideCanvas(bool greyOutsideCanvas)
    {
        greyOutsideCanvas_ = greyOutsideCanvas;
        repaint();
    }
    
    
private:

    bool editMode_;
    CommandID toolMode_;
    MouseCursor editMouseCursor_;

    MainComponent* mainComponent_;

    // canvas aspect
    int canvasAspectMode_;
    
    float canvasWidthAspect_;
    float canvasHeightAspect_;
    float canvasRatio_;

    // canvas position and size in pixels
    float canvasX_;
    float canvasY_;
    float canvasHeight_;
    float canvasWidth_;

    bool greyOutsideCanvas_;

    // canvas area fitted to window
    CanvasAreaComponent* canvasArea_;
    
    // the widget overlays
    Array<WidgetOverlayComponent *> overlayComponents_;

    // component that draws rounded edge
    TabsFrameBox* frame_;

    
    // xml data representing the widgets for this view
    XmlElement* xml_;

    String tabName_;
    int tabIndex_;
    
    // number of changes to this view since the tab created this session
    int sessionChanges_;
    
    // XMLRPC manager
    XMLRPCManager* XMLRPCManager_;
    
};

#endif // __WIDGET_VIEW_COMPONENT_H__


