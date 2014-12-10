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

#ifndef __WIDGET_OVERLAY_COMPONENT_H__
#define __WIDGET_OVERLAY_COMPONENT_H__

#include "WidgetComponent.h"
#include "WidgetViewComponent.h"
#include "CanvasComponent.h"

#include "juce.h"

class WidgetViewComponent;
class WidgetComponent;

class ModalResizableBorderComponent : public ResizableBorderComponent
{
public:
    ModalResizableBorderComponent  (Component  *const componentToResize, ComponentBoundsConstrainer  *const constrainer)
        : ResizableBorderComponent(componentToResize, constrainer), modeEnabled_(false), somethingIsBeingDraggedOver_(false) {}

  
    void setModeEnabled(bool modeEnabled)
    {
        modeEnabled_ = modeEnabled;
    }

private:
    bool modeEnabled_;

    bool somethingIsBeingDraggedOver_;
    
protected:
    virtual bool hitTest(int x, int y)
    {
        if (modeEnabled_)
            return ResizableBorderComponent :: hitTest(x, y);
        else
            return false;

    }

    void paint(Graphics& g)
    {
    }

};

//==============================================================================

class WidgetOverlayComponent : public CanvasComponent
#if STAGE_BUILD==DESKTOP
                                , public TooltipClient
#endif // STAGE_BUILD==DESKTOP
{
public:
    //==============================================================================
    WidgetOverlayComponent(bool editMode, CommandID toolMode, WidgetComponent* target, WidgetViewComponent* widgetView);
    ~WidgetOverlayComponent();

    //==============================================================================

    void setEditMode(bool editMode);
    void setToolMode(CommandID toolMode);
    WidgetComponent* getTarget() { return target_; }
    void setTarget(WidgetComponent *widget) { target_ = widget; }
    
    //==============================================================================

    void paint (Graphics& g);
    void resized();
    void moved();
#if STAGE_BUILD==DESKTOP
    String getTooltip();
#endif // STAGE_BUILD==DESKTOP

    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    
    void setSomethingIsBeingDraggedOver(bool somethingIsBeingDraggedOver)
    {
        somethingIsBeingDraggedOver_ = somethingIsBeingDraggedOver;
        repaint();
    }
    
    //==============================================================================
    
    void updateToTarget();
    
    void positionTextBorder();
    
    void drawResizableBorder (Graphics& g,
                              int w, int h,
                              const BorderSize<int> borderSize,
                              const bool isMouseOver);
    
    void drawTextHeight (Graphics& g,
                         int x, int y, int w, int h,
                         const bool isMouseOver);
    
    
    void drawMouseOverCorners (Graphics& g, int w, int h);

    //==============================================================================
    
    const int borderThickness;
    
private:
    ModalResizableBorderComponent* border;
    Component* textBorder;
    WidgetComponent* target_;
    WidgetViewComponent* widgetView_;
    
    bool selected, dragging;
    bool editMode_;
    CommandID toolMode_;
    
    int startX, startY;
    
    ComponentBoundsConstrainer* bounds_;

//    int textHeightPixels_;
    int relDragStartX_;
    int relDragStartY_;
    
    bool somethingIsBeingDraggedOver_;

};


#endif // __WIDGET_OVERLAY_COMPONENT_H__
