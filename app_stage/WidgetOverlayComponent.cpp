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

#include "WidgetOverlayComponent.h"
#include "ToolManager.h"


WidgetOverlayComponent :: WidgetOverlayComponent(bool editMode, CommandID toolMode, WidgetComponent *target, WidgetViewComponent *widgetView)
    : borderThickness(4), target_(target), widgetView_(widgetView), selected(true), dragging(false), 
    relDragStartX_(0), relDragStartY_(0), somethingIsBeingDraggedOver_(false)
{
    setName("widget overlay");

    // setup bounds
    bounds_ = new ComponentBoundsConstrainer();
    
    // set min on screen to large numbers to stop widgets being moved or resized
    // off the canvas
    bounds_->setMinimumOnscreenAmounts(0xffffff,0xffffff,0xffffff,0xfffffff);

    textHeightPixels_ = 32;
    
    // prevent making the widget so small it disappears!
    bounds_->setMinimumWidth(borderThickness*2);
    bounds_->setMinimumHeight(textHeightPixels_+borderThickness*2);
    
    // create resizeable border
    addChildComponent (border = new ModalResizableBorderComponent (this, bounds_));
    border->setBorderThickness(BorderSize<int>(borderThickness));

    addAndMakeVisible(border);


    addChildComponent (textBorder = new Component ());
    addAndMakeVisible(textBorder);
    textBorder->setMouseCursor(MouseCursor::UpDownResizeCursor);
    
    float cx=0, cy=0, cwidth=0, cheight=0;
    widgetView_->getCanvasBounds(cx, cy, cwidth, cheight);

    setRepaintsOnMouseActivity(true);
    border->setRepaintsOnMouseActivity(true);

    // listen to mouse events to catch mouse up to know when to update canvas size
    border->addMouseListener(this, true);
    textBorder->addMouseListener(this, true);

    // set edit mode action, can edit the widget but not its subcomponents
    setEditMode(editMode);
    
    setToolMode(toolMode);

}

WidgetOverlayComponent :: ~WidgetOverlayComponent()
{

    delete bounds_;
    
    deleteAllChildren();

}


void WidgetOverlayComponent::setEditMode(bool editMode)
{
    
    editMode_ = editMode;
    setInterceptsMouseClicks(editMode_, editMode_);
    border->setVisible(editMode_);
    repaint();
}

void WidgetOverlayComponent::setToolMode(CommandID toolMode)
{
    toolMode_ = toolMode;

    // set the current tool mouse pointer over this widget
    setMouseCursor(ToolManager::getMouseCursor(toolMode_));
    // disable sizing (unless in size mode below)
    border->setModeEnabled(false);

    switch (toolMode_) 
    {
        case MainComponent::toolPerform:
            target_->setInterceptsMouseClicks(true, true);
            break;
        case MainComponent::toolCreate:
            target_->setInterceptsMouseClicks(false, false);
            setInterceptsMouseClicks(false, false);
            break;
        case MainComponent::toolEdit:
            textBorder->setVisible(false);
            break;
        case MainComponent::toolMove:
            setInterceptsMouseClicks(true, false);
            textBorder->setVisible(false);
            border->setVisible(false);
            break;
        case MainComponent::toolSize:
            border->setVisible(true);
            border->setModeEnabled(true);
            textBorder->setVisible(true);
            setInterceptsMouseClicks(true, false);
            break;
        case MainComponent::toolDelete:
            textBorder->setVisible(false);
            break;
        case MainComponent::toolMulti:
            setMouseCursor(MouseCursor::DraggingHandCursor);
            border->setVisible(true);
            border->setModeEnabled(true);
            textBorder->setVisible(true);
            setInterceptsMouseClicks(true, true);
            break;
        case MainComponent::toolHelp:
            textBorder->setVisible(false);
            break;
        default:
            break;
    }
    
    

    
    
}


void WidgetOverlayComponent::paint (Graphics& g)
{
    if (selected && editMode_)
    {
        BorderSize<int> borderSize (border->getBorderThickness());
        
        drawResizableBorder (g, getWidth(), getHeight(), borderSize, (isMouseOverOrDragging() || border->isMouseOverOrDragging()));

        String textPosition = target_->getTextPosition();
        
        if (textPosition=="top")
            drawTextHeight (g, 0, textHeightPixels_, getWidth(), borderThickness, (isMouseOverOrDragging() || textBorder->isMouseOverOrDragging()));
        if (textPosition=="bottom")
            drawTextHeight (g, 0, getHeight()-textHeightPixels_, getWidth(), borderThickness, (isMouseOverOrDragging() || textBorder->isMouseOverOrDragging()));        
        if (textPosition=="left")
            drawTextHeight (g, textHeightPixels_, 0, borderThickness, getHeight(), (isMouseOverOrDragging() || textBorder->isMouseOverOrDragging()));
        if (textPosition=="right")
            drawTextHeight (g, getWidth()-textHeightPixels_, 0, borderThickness, getHeight(), (isMouseOverOrDragging() || textBorder->isMouseOverOrDragging()));
    
    }
    else if (isMouseOverOrDragging())
    {
        drawMouseOverCorners (g, getWidth(), getHeight());
    }

    
    if (!target_->isEnabled()) 
    {
        g.setColour(Colour(0x50ffffff));
        g.fillRect(0, 0, getWidth(), getHeight());
    }
    
}

void WidgetOverlayComponent::resized()
{
    
    // resize the associated widget
    target_->setBounds(getX(), getY(), getWidth(), getHeight());
    
    // set the border to the component bounds
    border->setBounds(0, 0, getWidth(), getHeight());
    
    textBorder->setBounds(0, textHeightPixels_, getWidth(), borderThickness);

    
    String textPosition = target_->getTextPosition();
    
    if (textPosition=="top")
    {
        textBorder->setBounds(0, textHeightPixels_, getWidth(), borderThickness);
        bounds_->setMinimumHeight(textHeightPixels_+borderThickness*2);
        bounds_->setMinimumWidth(borderThickness*2);
    }
    if (textPosition=="bottom")
    {
        textBorder->setBounds(0, getHeight()-textHeightPixels_, getWidth(), borderThickness);
        bounds_->setMinimumHeight(textHeightPixels_+borderThickness);
        bounds_->setMinimumWidth(borderThickness*2);
    }
    if (textPosition=="left")
    {
        textBorder->setBounds(textHeightPixels_, 0, borderThickness, getHeight());
        bounds_->setMinimumWidth(textHeightPixels_+borderThickness*2);
        bounds_->setMinimumHeight(borderThickness*2);
    }
    if (textPosition=="right")
    {
        textBorder->setBounds(getWidth()-textHeightPixels_, 0, borderThickness, getHeight());
        bounds_->setMinimumWidth(textHeightPixels_+borderThickness);
        bounds_->setMinimumHeight(borderThickness*2);
    }
    

    
}

void WidgetOverlayComponent::moved()
{

    // move the associated widget
    target_->setBounds(getX(),getY(),getWidth(),getHeight());

  
}

#if STAGE_BUILD==DESKTOP
String WidgetOverlayComponent::getTooltip()
{
    return target_->getTooltip();
}
#endif // STAGE_BUILD==DESKTOP

void WidgetOverlayComponent::updateToTarget()
{
    String textPosition = target_->getTextPosition();

    if (textPosition=="top" || textPosition=="bottom")
        textBorder->setMouseCursor(MouseCursor::UpDownResizeCursor);
    else
        textBorder->setMouseCursor(MouseCursor::LeftRightResizeCursor);
    
    // reposition text border if it has changed
    resized();
}


//------------------------------------------------------------------------------
// Mouse functions
//------------------------------------------------------------------------------

void WidgetOverlayComponent::mouseDown (const MouseEvent& e)
{
    dragging = false;

    if(e.originalComponent==this)
    {
    
        if (e.mods.isLeftButtonDown()) 
        {
            // toggle selected state
            //selected = !selected;
            // TODO: handle selection, for now always selected
            selected = true;
            border->setVisible(selected);
        }
    }
    
    if(e.originalComponent==textBorder)
    {
        relDragStartX_ = e.getMouseDownX();
        relDragStartY_ = e.getMouseDownY();
    }
    
}

void WidgetOverlayComponent::mouseDrag (const MouseEvent& e)
{
    int dx=0, dy=0;
    
    if(e.originalComponent==this)
    {
        if (e.mods.isLeftButtonDown()) 
        {

            //if (!dragging && (toolMode_==MainComponent::toolMove || toolMode_==MainComponent::toolMulti))
            if (!dragging && (toolMode_==MainComponent::toolSize || toolMode_==MainComponent::toolMulti))
            {
                dragging = ! e.mouseWasClicked();

                // start dragging
                if (dragging)
                {
                    startX = getX();
                    startY = getY();

                    if (!selected) 
                    {
                        selected = true;
                        border->setVisible(selected);
                    }
                }

            }

            // continue dragging
            if (dragging)
            {
                dx = e.getDistanceFromDragStartX();
                dy = e.getDistanceFromDragStartY();
                
                juce::Rectangle<int> newBounds((startX+dx),(startY+dy), getWidth(), getHeight());
				bounds_->setBoundsForComponent(this, newBounds, false, false, false, false);

            }

        }
    }

    // TODO: remove this
    if(e.originalComponent==border)
    {
        dx = e.getDistanceFromDragStartX();
    }

    if(e.originalComponent==textBorder)
    {
        positionTextBorder();
        
        resized();
        repaint();
    }
    
}

void WidgetOverlayComponent::mouseUp (const MouseEvent& e)
{
    if(e.originalComponent==this)
    {
        if (dragging)
        {
            // dropped after dragging so update canvas position
            bounds_->checkComponentBounds(this);
            
            Point<float> position = widgetView_->pixelsToCanvasUnits((float)getX(),(float)getY());
            setPositionCanvasUnits(position.getX(), position.getY());
            target_->setPositionCanvasUnits(position.getX(), position.getY());
        }
        border->setVisible(selected);
        repaint();
    }
    if(e.originalComponent==border)
    {
        // corner resizing dropped so update canvas position and size
        Point<float> position = widgetView_->pixelsToCanvasUnits((float)getX(),(float)getY());
        Point<float> size = widgetView_->pixelsToCanvasUnits((float)getWidth(),(float)getHeight());
        setBoundsCanvasUnits(position.getX(), position.getY(), size.getX(), size.getY());
        target_->setBoundsCanvasUnits(position.getX(), position.getY(), size.getX(), size.getY());
    }
    if(e.originalComponent==textBorder)
    {
        positionTextBorder();
        
        float size = widgetView_->pixelsToCanvasUnits((float)textHeightPixels_);
        setTextHeightCanvasUnits(size);
        target_->setTextHeightCanvasUnits(size);
    }
    
}

void WidgetOverlayComponent::positionTextBorder()
{
    String textPosition = target_->getTextPosition();

    int position = 0;
    
    if (textPosition=="top" || textPosition=="bottom")
    {
        position = getMouseXYRelative().getY()-borderThickness+relDragStartY_;
        if(position<borderThickness)
            position = borderThickness;
        if(position>getHeight()-(borderThickness*2))
            position = getHeight()-borderThickness*2;
    }

    if (textPosition=="left" || textPosition=="right")
    {
        position = getMouseXYRelative().getX()-borderThickness+relDragStartX_;
        if(position<borderThickness)
            position = borderThickness;
        if(position>getWidth()-(borderThickness*2))
            position = getWidth()-borderThickness*2;
    }

    if (textPosition=="top")
    {
        textHeightPixels_ = position;
        bounds_->setMinimumHeight(position+borderThickness*2);
        bounds_->setMinimumWidth(borderThickness*2);
    }
    if (textPosition=="bottom")
    {   
        textHeightPixels_ = getHeight()-position;
        bounds_->setMinimumHeight(position+borderThickness);
        bounds_->setMinimumWidth(borderThickness*2);
    }
    if (textPosition=="left")
    {
        textHeightPixels_ = position;
        bounds_->setMinimumWidth(position+borderThickness*2);
        bounds_->setMinimumHeight(borderThickness*2);
    }
    if (textPosition=="right")
    {
        textHeightPixels_ = getWidth()-position;
        bounds_->setMinimumWidth(position+borderThickness);
        bounds_->setMinimumHeight(borderThickness*2);
    }
    
    target_->setTextHeightPixels(textHeightPixels_);
    target_->resized();
}



//------------------------------------------------------------------------------
// Draw resizeable borders
//------------------------------------------------------------------------------

void WidgetOverlayComponent::drawResizableBorder (Graphics& g,
                          int w, int h,
                          const BorderSize<int> borderSize,
                          const bool isMouseOver)
{
    if(!somethingIsBeingDraggedOver_)
    {
        if (target_->getConnected())
            g.setColour (Colours::orange.withAlpha (isMouseOver ? 0.4f : 0.3f));
        else
            g.setColour (Colours::red.withAlpha (isMouseOver ? 0.7f : 0.5f));

        g.fillRect (0, 0, w, borderSize.getTop());
        g.fillRect (0, 0, borderSize.getLeft(), h);
        g.fillRect (0, h - borderSize.getBottom(), w, borderSize.getBottom());
        g.fillRect (w - borderSize.getRight(), 0, borderSize.getRight(), h);
        
        g.drawRect (borderSize.getLeft() - 1, borderSize.getTop() - 1,
                    w - borderSize.getRight() - borderSize.getLeft() + 2,
                    h - borderSize.getTop() - borderSize.getBottom() + 2);
    }
    else 
    {
        g.setColour(Colours::red);
        g.drawRect(0, 0, getWidth(), getHeight(), 2);
    }
}

void WidgetOverlayComponent::drawTextHeight (Graphics& g,
                                                  int x, int y, int w, int h,
                                                  const bool isMouseOver)
{
    if(!somethingIsBeingDraggedOver_)
    {
        if (target_->getConnected())
            g.setColour (Colours::orange.withAlpha (isMouseOver ? 0.4f : 0.3f));
        else
            g.setColour (Colours::red.withAlpha (isMouseOver ? 0.7f : 0.5f));
        g.fillRect (x, y, w, h);
    }
    
}


void WidgetOverlayComponent::drawMouseOverCorners (Graphics& g, int w, int h)
{
    juce::RectangleList<int> r (juce::Rectangle<int> (0, 0, w, h));
    r.subtract (juce::Rectangle<int> (1, 1, w - 2, h - 2));
    
    const int size = jmin (w / 3, h / 3, 12);
    r.subtract (juce::Rectangle<int> (size, 0, w - size - size, h));
    r.subtract (juce::Rectangle<int> (0, size, w, h - size - size));
    
    g.setColour (Colours::darkgrey);
    
    for (int i = r.getNumRectangles(); --i >= 0;)
        g.fillRect (r.getRectangle (i));
}

