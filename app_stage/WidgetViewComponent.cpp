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

#include "WidgetViewComponent.h"
#include "WidgetOverlayComponent.h"
#include "WidgetComponent.h"
#include "DialogFrameworkComponent.h"
#include "WidgetDialogComponent.h"



//------------------------------------------------------------------------------

WidgetViewComponent :: WidgetViewComponent(const String& tabName, const int tabIndex, float canvasWidthAspect, float canvasHeightAspect, int canvasAspectMode,
                                           bool editMode, CommandID toolMode, bool greyOutsideCanvas, const MouseCursor& editMouseCursor, 
                                           MainComponent* mainComponent, WidgetTabbedComponent* tabbedComponent)
    : editMode_(editMode), toolMode_(toolMode), editMouseCursor_(editMouseCursor), mainComponent_(mainComponent),
      canvasAspectMode_(canvasAspectMode), canvasWidthAspect_(canvasWidthAspect), canvasHeightAspect_(canvasHeightAspect),
      greyOutsideCanvas_(greyOutsideCanvas),
      xml_(0), tabName_(tabName), tabIndex_(tabIndex), sessionChanges_(1)
{
    
    canvasArea_ = new CanvasAreaComponent(this, mainComponent);
    addAndMakeVisible(canvasArea_);
    canvasArea_->addMouseListener(this, true);
    canvasArea_->addMouseListener(tabbedComponent, false);
    canvasArea_->setName("canvas area");
    canvasArea_->toFront(true);
    
    
    frame_ = new TabsFrameBox();
    addAndMakeVisible(frame_);
    frame_->toBack();
    //frame_->setVisible(false);
    
    setInterceptsMouseClicks(false, true);

    XMLRPCManager_ = mainComponent->getXMLRPCManager();

    // initialise with 1 sessionChange to match eigenD since setting the xml
    // in eigenD constitutes 1 sessionChange
    
    // set up default xml
    xml_ = new XmlElement("tab");
    xml_->setAttribute("name", tabName_);
    xml_->setAttribute("aspectMode", canvasAspectMode_);
    xml_->setAttribute("widthAspect", canvasWidthAspect_);
    xml_->setAttribute("heightAspect", canvasHeightAspect_);
   
}

WidgetViewComponent :: ~WidgetViewComponent()
{
    delete xml_;
    
    canvasArea_->deleteAllChildren();
    
    deleteAllChildren();

}

void WidgetViewComponent :: clearHelp()
{
    // clear help from a change in agents to ensure widgets pull new help changes from agent tree
    WidgetOverlayComponent *overlay;
    int numWidgets = overlayComponents_.size();
    for (int i=0; i<numWidgets; i++)
    {
        overlay = overlayComponents_[i];
#if STAGE_BUILD==DESKTOP
        WidgetComponent *widget = overlay->getTarget();
        widget->clearHelp();
#endif // STAGE_BUILD==DESKTOP
    }
}

void WidgetViewComponent :: paint (Graphics& g)
{
    if(greyOutsideCanvas_)
    {
        g.setColour(Colour((uint8)0xe7, (uint8)0xe7, (uint8)0xe7, (uint8)0xff));
        g.fillRoundedRectangle(0, 0, (float)getWidth(), (float)getHeight(), 6);
        
        g.setColour(Colour((uint8)0xff, (uint8)0xff, (uint8)0xff, (uint8)0xff));
        g.fillRoundedRectangle(canvasX_, canvasY_, canvasWidth_, canvasHeight_, 6);
    }
    else
    {
        g.setColour(Colour((uint8)0xff, (uint8)0xff, (uint8)0xff, (uint8)0xff));
        g.fillRoundedRectangle(0, 0, (float)getWidth(), (float)getHeight(), 6);
    }
    
}

//------------------------------------------------------------------------------
//
// Canvas Area
//
// for holding widgets and handling drag and drop
//
//------------------------------------------------------------------------------

void CanvasAreaComponent :: paint (Graphics& g)
{
    // draw a red line around the comp if the user's currently dragging something over it..
    if (somethingIsBeingDraggedOver_)
    {
        g.setColour (Colours::red.withAlpha(0.7f));
        g.drawRect(0, 0, getWidth(), getHeight(), 3);
    }
}

//------------------------------------------------------------------------------
// Widget drag and drop
//------------------------------------------------------------------------------

bool CanvasAreaComponent :: isInterestedInDragSource (const SourceDetails& source)
{
    XmlElement* check=XmlDocument::parse(source.description);
    if(check!=0)
    {
        delete check;
        XmlElement* droppedXmlElem = XmlDocument(source.description).getDocumentElement(false);

        // not a node with children, and an enabled leaf node
        return (!(droppedXmlElem->getFirstChildElement()->hasTagName("atom") ||
                  droppedXmlElem->getFirstChildElement()->hasTagName("agent")) &&
                    droppedXmlElem->getStringAttribute("enabled")=="true" );

    }
    return false;
}

void CanvasAreaComponent :: itemDragEnter (const SourceDetails& source)
{
    somethingIsBeingDraggedOver_ = true;
   
    // highlight a widget for drop if it is not enabled
    WidgetOverlayComponent *overlay;
    for (int i=0; i<widgetView_->getOverlayComponents()->size(); i++)
    {
        overlay = (*widgetView_->getOverlayComponents())[i];

        if(juce::Rectangle<int>(overlay->getX(), overlay->getY(), overlay->getWidth(), overlay->getHeight()).contains(source.localPosition.x,source.localPosition.y))
        {
            if (!overlay->getTarget()->isEnabled())
            {
                overlay->setSomethingIsBeingDraggedOver(true);
                somethingIsBeingDraggedOver_ = false;
            }
        }
        else
        {
            overlay->setSomethingIsBeingDraggedOver(false);
        }
    }
    
    repaint();
}

void CanvasAreaComponent :: itemDragMove (const SourceDetails& source)
{
    somethingIsBeingDraggedOver_ = true;

    // highlight a widget for drop if it is not enabled
    WidgetOverlayComponent *overlay;
    for (int i=0; i<widgetView_->getOverlayComponents()->size(); i++)
    {
        overlay = (*widgetView_->getOverlayComponents())[i];
        if(juce::Rectangle<int>(overlay->getX(), overlay->getY(), overlay->getWidth(), overlay->getHeight()).contains(source.localPosition.x,source.localPosition.y))
        {
            if (!overlay->getTarget()->isEnabled())
            {   
                overlay->setSomethingIsBeingDraggedOver(true);
                somethingIsBeingDraggedOver_ = false;
            }
        }
        else
        {
            overlay->setSomethingIsBeingDraggedOver(false);
        }
    }
    
    repaint();
}

void CanvasAreaComponent :: itemDragExit (const SourceDetails& source)
{
    somethingIsBeingDraggedOver_ = false;
    for (int i=0; i<widgetView_->getOverlayComponents()->size(); i++)
        (*widgetView_->getOverlayComponents())[i]->setSomethingIsBeingDraggedOver(false);
        
    repaint();
}

void CanvasAreaComponent :: itemDropped (const SourceDetails& source)
{
    bool newWidget = true;
    
    // dropped over a widget, if it is not enabled change the OSC path of the widget
    WidgetOverlayComponent *overlay;
    for (int i=0; i<widgetView_->getOverlayComponents()->size(); i++)
    {
        overlay = (*widgetView_->getOverlayComponents())[i];
        if(juce::Rectangle<int>(overlay->getX(), overlay->getY(), overlay->getWidth(), overlay->getHeight()).contains(source.localPosition.x,source.localPosition.y))
        {
            if (!overlay->getTarget()->isEnabled())
            {   
                widgetView_->changeWidget(source.description, overlay);
                newWidget = false;
                break;
            }
        }
    }

    // dropped over the canvas, create a new widget
    if(newWidget)
        widgetView_->createWidget(source.description, source.localPosition.x, source.localPosition.y);

    
    for (int i=0; i<widgetView_->getOverlayComponents()->size(); i++)
        (*widgetView_->getOverlayComponents())[i]->setSomethingIsBeingDraggedOver(false);
    somethingIsBeingDraggedOver_ = false;
    repaint();

    
    // clear selected items after creating a widget
#if STAGE_BUILD==DESKTOP
    mainComponent_->getAgentViewComponent()->getTreeView()->clearSelectedItems();
#endif // STAGE_BUILD==DESKTOP
    
}


//------------------------------------------------------------------------------
// Widget modes
//------------------------------------------------------------------------------

void WidgetViewComponent :: mouseDown(const MouseEvent &e)
{
#if STAGE_BUILD==DESKTOP
    
    if(editMode_)
    {
        
        // TODO: need to check the component is a widget correctly
        if((e.mods.isPopupMenu() && e.eventComponent!=this && e.eventComponent!=canvasArea_ && mainComponent_->getToolMode()==MainComponent::toolMulti) ||
           (e.mods.isLeftButtonDown() && e.eventComponent!=canvasArea_ && mainComponent_->getToolMode()==MainComponent::toolEdit))
        {
            // widget menu from right click on widget overlay
            
            // target widget
            WidgetComponent* widget = ((WidgetOverlayComponent*)e.eventComponent)->getTarget();

            // open the widget properties editing dialogue
            
            EigenDialogWindow window("Widget Dialog Window");
            DialogFrameworkComponent framework(&window);
            WidgetDialogComponent content;

            content.initialize(&framework, widget);
            window.setContentComponent(&framework, false, true);
            window.setSize(320, 419);
            window.centreAroundComponent(mainComponent_, framework.getWidth(), framework.getHeight());
            window.setVisible(true);
            framework.setTitleText("Widget Properties");
            framework.setContentComponent(&content);
            content.setFocusOrder(&framework);

            int result = framework.runModalLoop();
            
            if(result!=0)
            {
                widget->setWidgetType(content.getWidgetType());
                widget->setWidgetName(content.getName());
                widget->setTextPosition(content.getTextPosition());
                widget->setUserEnabled(content.getUserEnabled());
                widget->setUserRange(content.getUserMin(), content.getUserStep(), content.getUserMax());
                ((WidgetOverlayComponent*)e.eventComponent)->updateToTarget();

                repaint();
            }
        }
    }
    
    if(!mainComponent_->getLockMode() && mainComponent_->getToolMode()!=MainComponent::toolMulti)
    {
        
        if(e.mods.isPopupMenu())
        {
            // tool menu from right click on canvas
            
            PopupMenu m;
            Image img;

            CommandID toolMode = mainComponent_->getToolMode();

            if(toolMode==MainComponent::toolPerform)
                img=ImageCache::getFromMemory(ToolbarIcons::usewidget_on_png,ToolbarIcons::usewidget_on_pngSize);
            else
                img=ImageCache::getFromMemory(ToolbarIcons::usewidget_off_png,ToolbarIcons::usewidget_off_pngSize);
            m.addItem(MainComponent::toolPerform, "Perform", true, toolMode==MainComponent::toolPerform, img);
            
            if(toolMode==MainComponent::toolCreate)
                img=ImageCache::getFromMemory(ToolbarIcons::create_on_png,ToolbarIcons::create_on_pngSize);
            else
                img=ImageCache::getFromMemory(ToolbarIcons::create_off_png,ToolbarIcons::create_off_pngSize);
            m.addItem(MainComponent::toolCreate, "Create", true, toolMode==MainComponent::toolCreate, img);
                
            if(toolMode==MainComponent::toolEdit)
                img=ImageCache::getFromMemory(ToolbarIcons::edit_on_png,ToolbarIcons::edit_on_pngSize);
            else
                img=ImageCache::getFromMemory(ToolbarIcons::edit_off_png,ToolbarIcons::edit_off_pngSize);
            m.addItem(MainComponent::toolEdit, "Edit", true, toolMode==MainComponent::toolEdit, img);

//            if(toolMode==MainComponent::toolMove)
//                img=ImageCache::getFromMemory(ToolbarIcons::move_on_png,ToolbarIcons::move_on_pngSize);
//            else
//                img=ImageCache::getFromMemory(ToolbarIcons::move_off_png,ToolbarIcons::move_off_pngSize);
//            m.addItem(MainComponent::toolMove, "Move", true, toolMode==MainComponent::toolMove, img);
            
            if(toolMode==MainComponent::toolSize)
                img=ImageCache::getFromMemory(ToolbarIcons::resize_on_png,ToolbarIcons::resize_on_pngSize);
            else
                img=ImageCache::getFromMemory(ToolbarIcons::resize_off_png,ToolbarIcons::resize_off_pngSize);
            m.addItem(MainComponent::toolSize, "Size", true, toolMode==MainComponent::toolSize, img);
            
            if(toolMode==MainComponent::toolDelete)
                img=ImageCache::getFromMemory(ToolbarIcons::delete_on_png,ToolbarIcons::delete_on_pngSize);
            else
                img=ImageCache::getFromMemory(ToolbarIcons::delete_off_png,ToolbarIcons::delete_off_pngSize);
            m.addItem(MainComponent::toolDelete, "Delete", true, toolMode==MainComponent::toolDelete, img);

            if(toolMode==MainComponent::toolHelp)
                img=ImageCache::getFromMemory(ToolbarIcons::help_on_png,ToolbarIcons::help_on_pngSize);
            else
                img=ImageCache::getFromMemory(ToolbarIcons::help_off_png,ToolbarIcons::help_off_pngSize);
            m.addItem(MainComponent::toolHelp, "Help", true, toolMode==MainComponent::toolHelp, img);
            

            
            MouseCursor c = canvasArea_->getMouseCursor();
            canvasArea_->setMouseCursor(MouseCursor::NormalCursor);
            
            int result = m.showAt(juce::Rectangle<int>(e.getScreenX(), e.getScreenY(),1,96));

            canvasArea_->setMouseCursor(c);

            if(result!=0)
                mainComponent_->invokeDirectly(result, false);
        }
    
    }

    // left button actions
    if(e.mods.isLeftButtonDown())
    {       
        // create clicked on canvas area
        if(e.eventComponent==canvasArea_ && toolMode_==MainComponent::toolCreate)
        {
            const String& sourceDescription = mainComponent_->getAgentViewComponent()->getSelectedTreeItem();
            if(sourceDescription!="")
            {
                bool newWidget = true;
                
                int x = e.getMouseDownX();
                int y = e.getMouseDownY();
                
                // clicked over a widget, if it is not enabled change the OSC path of the widget
                WidgetOverlayComponent *overlay;
                for (int i=0; i<overlayComponents_.size(); i++)
                {
                    overlay = overlayComponents_[i];
                    if(juce::Rectangle<int>(overlay->getX(), overlay->getY(), overlay->getWidth(), overlay->getHeight()).contains(x,y))
                        if (!overlay->getTarget()->isEnabled())
                        {   
                            changeWidget(sourceDescription, overlay);
                            newWidget = false;
                            break;
                        }
                }
                
                if (newWidget)
                    createWidget(sourceDescription, e.getMouseDownX(), e.getMouseDownY());
                
            }

            // clear selected items after creating a widget
            mainComponent_->getAgentViewComponent()->getTreeView()->clearSelectedItems();
            
        }

        // delete clicked on widget
        if(e.eventComponent!=canvasArea_ && toolMode_==MainComponent::toolDelete)
        {
            deleteWidget((WidgetOverlayComponent*)e.eventComponent);
        }

        // help clicked on widget
        if(e.eventComponent!=canvasArea_ && toolMode_==MainComponent::toolHelp)
        {
            WidgetOverlayComponent* widgetOverlay = dynamic_cast<WidgetOverlayComponent*>(e.eventComponent);
            if(widgetOverlay)
            {
                String text = widgetOverlay->getTarget()->getHelpDescription();
                mainComponent_->getHelpViewComponent()->setHelpDescription(text);
            }
        }
        
    }
    
#endif // STAGE_BUILD==DESKTOP

    
}

void WidgetViewComponent :: createWidget(const String& sourceDescription, int x, int y)
{

    // parse the agent xml from the dropped component to get the widget meta data
    XmlElement* agentXml = XmlDocument(sourceDescription).getDocumentElement(false);

    // make sure a widget can be created from this
    if(agentXml->getFirstChildElement()->hasTagName("atom") ||
       agentXml->getFirstChildElement()->hasTagName("agent") || 
       agentXml->getStringAttribute("enabled")=="false" )
    {
        delete agentXml;
        return;
    }

    //--------------------------------------------------------------------------
    // Create a new widget on the canvas
    //--------------------------------------------------------------------------
    
    // calculate position and size
    
    float defaultWidth = 0;
    float defaultHeight = 0;
    float defaultTextHeight = 0;
    if(canvasHeightAspect_<canvasWidthAspect_)
    {
        defaultWidth = 0.25f*canvasHeightAspect_;
        defaultHeight = 0.25f*canvasHeightAspect_;
    }
    else 
    {
        defaultWidth = 0.25f*canvasWidthAspect_;
        defaultHeight = 0.25f*canvasWidthAspect_;
    }
    defaultTextHeight = defaultHeight/8;
    String defaultTextPosition = "top";
    
    // reposition on canvas if dropped partially off the edge
    Point<float> size = canvasUnitsToPixels(defaultWidth, defaultHeight);
    if ((x+size.getX())>canvasWidth_)
        x = (int)(canvasWidth_-size.getX() + 1.0f);
    
    if ((y+size.getY())>canvasHeight_)
        y = (int)(canvasHeight_-size.getY() + 1.0f);
    
    
    // convert position created on canvas to canvas units
    Point<float> position = pixelsToCanvasUnits((float)x,(float)y);

    //--------------------------------------------------------------------------
    // update GUI
    //--------------------------------------------------------------------------
    

    
    // create new widget component
    int widgetIndex;
    WidgetOverlayComponent* newWidgetOverlay = addWidgetToCanvas(agentXml, position.getX(), position.getY(), defaultWidth, defaultHeight, defaultTextHeight, defaultTextPosition,0,&widgetIndex);
    WidgetComponent* newWidget = newWidgetOverlay->getTarget();

    delete agentXml;

    int preferredWidth, preferredHeight;

    if(newWidget->hasPreferredSize(&preferredWidth,&preferredHeight))
    {
        std::cout << "resizing to " << preferredWidth << ":" << preferredHeight << std::endl;

        if(x+preferredWidth > canvasWidth_) 
        {
            preferredWidth = canvasWidth_-x;
            if(preferredWidth <= 0)
            {
                preferredWidth = canvasWidth_;
                x = 0;
            }
        }

        if(y+preferredHeight > canvasHeight_)
        {
            preferredHeight = canvasHeight_-y;
            if(preferredHeight <= 0)
            {
                preferredHeight = canvasHeight_;
                y = 0;
            }
        }

        Point<float> new_pos = pixelsToCanvasUnits((float)x,(float)y);
        Point<float> new_size = pixelsToCanvasUnits((float)preferredWidth,(float)preferredHeight);

        newWidgetOverlay->setBoundsCanvasUnits(new_pos.getX(),new_pos.getY(),new_size.getX(),new_size.getY());
        newWidget->setBoundsCanvasUnits(new_pos.getX(),new_pos.getY(),new_size.getX(),new_size.getY());
        newWidgetOverlay->updateBounds(canvasWidth_, canvasHeight_, canvasWidthAspect_, canvasHeightAspect_);
        newWidget->updateBounds(canvasWidth_, canvasHeight_, canvasWidthAspect_, canvasHeightAspect_);
    }
    
    //--------------------------------------------------------------------------
    // update model
    //--------------------------------------------------------------------------

    incSessionChanges();
    
    XmlElement* widgetXml = newWidget->getXml();
    
    // send new widget xml to eigenD
    String widgetXmlDoc = widgetXml->createDocument("", false, false);
    std::cout << "adding widget to eigend" << widgetXmlDoc;

    XmlRpcValue params, result;
    params.clear();
    params[0] = tabIndex_;
    params[1] = XMLRPCManager::juceToStdString(widgetXmlDoc);

    try {
        XMLRPCManager_->execute("addWidget", params, result, true);
    }
    catch (XMLRPCAbortException e) {
        // app is shutting down after an xmlrpc has blocked so bail
        return;
    }

    newWidget->setIndex(widgetIndex);
    std::cout << "add widget done\n";
    
}

WidgetOverlayComponent* WidgetViewComponent :: addWidgetToCanvas(XmlElement* xml, float x, float y, float width, float height, float textHeight, String& textPosition, const XmlElement *oldXml, int *newIndex)
{
    // TODO: can overlay components be used?
    int widgetIndex = overlayComponents_.size();
    
    // create the actual widget component, it will add itself to the osc manager
    WidgetComponent* widget = new WidgetComponent(xml, mainComponent_, oldXml);
    widget->setBoundsCanvasUnits(x, y, width, height);
    widget->setTextHeightCanvasUnits(textHeight);
    widget->setTextPosition(textPosition);
    // update the bounds in pixels from the stored sizes
    widget->updateBounds(canvasWidth_, canvasHeight_, canvasWidthAspect_, canvasHeightAspect_);
    
    canvasArea_->addAndMakeVisible(widget);

    // create overlay for controlling position and size
    WidgetOverlayComponent *widgetOverlay = new WidgetOverlayComponent(editMode_, toolMode_, widget, this);
    widgetOverlay->setBoundsCanvasUnits(x, y, width, height);
    widgetOverlay->setTextHeightCanvasUnits(textHeight);
    widgetOverlay->setTextPosition(textPosition);
    widgetOverlay->updateBounds(canvasWidth_, canvasHeight_, canvasWidthAspect_, canvasHeightAspect_);
    widgetOverlay->updateToTarget();
    
    canvasArea_->addAndMakeVisible(widgetOverlay);
    widgetOverlay->toFront(true);
    
    overlayComponents_.add(widgetOverlay);
    
    // listen to mouse clicks on the widget
    widgetOverlay->addMouseListener(canvasArea_, false);

    resized();
    if(newIndex) *newIndex = widgetIndex;

    return widgetOverlay;
    
}

void WidgetViewComponent :: changeWidget(const String& sourceDescription, WidgetOverlayComponent* overlay)
{
    // parse the agent xml from the dropped component to get the widget meta data
    XmlElement* agentXml = XmlDocument(sourceDescription).getDocumentElement(false);
    
    // make sure a widget can be changed to this
    if(agentXml->getFirstChildElement()->hasTagName("atom") ||
       agentXml->getFirstChildElement()->hasTagName("agent") || 
       agentXml->getStringAttribute("enabled")=="false" )
    {
        delete agentXml;
        return;
    }

    WidgetComponent *widget = overlay->getTarget();
    XmlElement oldWidgetXml = XmlElement(*widget->getXml());

    juce::Rectangle<float> r = widget->getBoundsCanvasUnits();
    float textHeight = widget->getTextHeightCanvasUnits();
    String textPosition = widget->getTextPosition();

    deleteWidget(overlay);

    int widgetIndex;
    overlay = addWidgetToCanvas(agentXml,r.getX(),r.getY(),r.getWidth(),r.getHeight(),textHeight,textPosition,&oldWidgetXml,&widgetIndex);
    widget = overlay->getTarget();
    widget->setUserEnabled(true);

    XmlElement *widgetXml = widget->getXml();
    String widgetXmlDoc = widgetXml->createDocument("", false, false);
    //std::cout << widgetXmlDoc << std::endl;
    
    try 
    {
        incSessionChanges();
        XmlRpcValue params, result;
        params.clear();
        params[0] = tabIndex_;
        params[1] = XMLRPCManager::juceToStdString(widgetXmlDoc);
        XMLRPCManager_->execute("addWidget", params, result, true);
    }
    catch (XMLRPCAbortException e) {
        // app is shutting down after an xmlrpc has blocked so bail
        delete agentXml;
        return;
    }

    widget->setIndex(widgetIndex);
    delete agentXml;
}



void WidgetViewComponent :: deleteWidget(WidgetOverlayComponent* widgetOverlay)
{
    WidgetComponent* widget = widgetOverlay->getTarget();
    int deleteWidgetIndex = widget->getIndex();
    
    overlayComponents_.removeAllInstancesOf(widgetOverlay);

    // set the widget indices to consequetive numbers when a hole left from removing a widget
    for(int i=0; i<overlayComponents_.size(); i++)
        overlayComponents_[i]->getTarget()->setIndex(i);

    incSessionChanges();
    
    // remove widget in eigenD
    XmlRpcValue params, result;
    params.clear();
    params[0] = tabIndex_;
    params[1] = deleteWidgetIndex;
    try {
        XMLRPCManager_->execute("removeWidget", params, result, true);
    }
    catch (XMLRPCAbortException e) {
        // app is shutting down after an xmlrpc has blocked so bail
        // tidy up anyway
        delete widget;
        delete widgetOverlay;
        return;
    }
    
    // widget removes itself from OSC manager on destruction
    delete widget;
    delete widgetOverlay;
}

void WidgetViewComponent :: setEditMode(bool editMode)
{
    editMode_ = editMode;
    
    // set edit mode for all subcomponent widgets
    for (int i=0; i<overlayComponents_.size(); i++)
        overlayComponents_[i]->setEditMode(editMode);

    // set the mouse pointer
    if (editMode_)
        canvasArea_->setMouseCursor(editMouseCursor_);
    else
        canvasArea_->setMouseCursor(MouseCursor::NormalCursor);

}

void WidgetViewComponent :: setToolMode(CommandID toolMode, const MouseCursor& mouseCursor)
{
    toolMode_ = toolMode;
    editMouseCursor_ = mouseCursor;
    setEditMode(editMode_);

    // set tool mode for all subcomponent widgets
    for (int i=0; i<overlayComponents_.size(); i++)
    {
        overlayComponents_[i]->setToolMode(toolMode);
    }
    
}


//------------------------------------------------------------------------------
// Aspect ratio
//------------------------------------------------------------------------------

void WidgetViewComponent :: setAspect(int canvasAspectMode, float canvasWidthAspect, float canvasHeightAspect)
{

    WidgetComponent* widgetComponent;
    
    // renormalize all widgets to new aspect
    for (int i=0; i<overlayComponents_.size(); i++)
    {
        widgetComponent = overlayComponents_[i]->getTarget();

        juce::Rectangle<float> r = widgetComponent->getBoundsCanvasUnits();
        float textHeight = widgetComponent->getTextHeightCanvasUnits();
        String textPosition = widgetComponent->getTextPosition();
        
        // renormalize bounds
        float nX = (r.getX()/canvasWidthAspect_)*canvasWidthAspect;
        float nY = (r.getY()/canvasHeightAspect_)*canvasHeightAspect;
        float nWidth = (r.getWidth()/canvasWidthAspect_)*canvasWidthAspect;
        float nHeight = (r.getHeight()/canvasHeightAspect_)*canvasHeightAspect;

        // renormalize text
        float newTextHeight = 0;
        if(textPosition=="top" || textPosition=="bottom")
            newTextHeight = (textHeight/canvasWidthAspect_)*canvasWidthAspect;
        else
            newTextHeight = (textHeight/canvasHeightAspect_)*canvasHeightAspect;
            
        overlayComponents_[i]->setBoundsCanvasUnits(nX, nY, nWidth, nHeight);
        widgetComponent->setBoundsCanvasUnits(nX, nY, nWidth, nHeight);
        
        overlayComponents_[i]->setTextHeightCanvasUnits(newTextHeight);
        widgetComponent->setTextHeightCanvasUnits(newTextHeight);
        
    }
    
    canvasAspectMode_ = canvasAspectMode;
    
    canvasWidthAspect_ = canvasWidthAspect;
    canvasHeightAspect_ = canvasHeightAspect;
    
    setAspectXml();
    
    resized();
    repaint();
}

void WidgetViewComponent :: resized()
{
    canvasRatio_ = canvasWidthAspect_/canvasHeightAspect_;
    float viewRatio = (float)getWidth()/(float)getHeight();
    
    // calc canvas size in pixels
    if(canvasRatio_>=viewRatio)
    {
        canvasWidth_ = (float)getWidth();
        canvasHeight_ = canvasWidth_ / canvasRatio_;
    }
    else 
    {
        canvasHeight_ = (float)getHeight();
        canvasWidth_ = canvasHeight_ * canvasRatio_;
    }

    // calc canvas position in pixels
    canvasX_ = ((float)(getWidth()-(int)canvasWidth_))/2.0f;
    canvasY_ = ((float)(getHeight()-(int)canvasHeight_))/2.0f;
    
    canvasArea_->setBounds((int)canvasX_, (int)canvasY_, (int)canvasWidth_, (int)canvasHeight_);
    
    // resize and reposition the widgets
    CanvasComponent *component;
    for(int i=0; i<canvasArea_->getNumChildComponents(); i++)
    {
        component = (CanvasComponent *)canvasArea_->getChildComponent(i);
        component->updateBounds(canvasWidth_, canvasHeight_, 
                                canvasWidthAspect_, canvasHeightAspect_);
    }

    frame_->setBounds(0,0,getWidth(),getHeight());
    
}


const Point<float> WidgetViewComponent :: pixelsToCanvasUnits(float x, float y)
{
    // convert size in view component in pixels to canvas units
    return Point<float>(canvasWidthAspect_*x/canvasWidth_, canvasHeightAspect_*y/canvasHeight_);
}

float WidgetViewComponent :: pixelsToCanvasUnits(float x)
{
    return canvasWidthAspect_*x/canvasWidth_;
}

const Point<float> WidgetViewComponent :: canvasUnitsToPixels(float x, float y)
{
    // convert size in view component in canvas units to pixels
    return Point<float>(canvasWidth_*x/canvasWidthAspect_, canvasHeight_*y/canvasHeightAspect_); 
}



//------------------------------------------------------------------------------
// XML Management
//------------------------------------------------------------------------------

void WidgetViewComponent::setXml(XmlElement* xml)
{
    // set the tab xml, stores tab meta data only - name, aspect etc.
    // not the widget data
    
    // the element will be created from XmlDocument this has responsibility for
    // deleting an old element
    if(xml_)
        delete xml_;

    // deep copy the xml element, since this class is where the xml for a tab
    // is actually stored!
    xml_ = new XmlElement(*xml);

}

XmlElement* WidgetViewComponent::getXml()
{
    return xml_;
}

void WidgetViewComponent::getCanvasPropertiesFromXml()
{
    //--------------------------------------------------------------------------
    // set tab meta data from xml
    
    // set canvas name
    tabName_ = xml_->getStringAttribute("name", "untitled");
    
    // set canvas aspect
    canvasAspectMode_ = xml_->getIntAttribute("aspectMode", 1);
    canvasWidthAspect_ = (float)xml_->getDoubleAttribute("widthAspect", 4);
    canvasHeightAspect_ = (float)xml_->getDoubleAttribute("heightAspect", 3);
    
}

void WidgetViewComponent::getCanvasFromXml(XmlElement *tabXml)
{
    // clear entire canvas ready for changes
    clearCanvas();

    // get canvas properties from stored tab xml
    getCanvasPropertiesFromXml();

    //--------------------------------------------------------------------------
    // create widgets from tab xml element

    // now add widgets and their xml
    
    XmlElement* widget = tabXml->getChildByName("widget");
    while (widget) 
    {
        addWidgetFromXml(widget, false);
        widget = widget->getNextElementWithTagName("widget");
    }


}


// TODO: refactor this, rename to updateXml()?
void WidgetViewComponent::getXmlFromCanvas()
{
    // updates xml from widgets
    
    WidgetComponent* widgetComponent;
    
    // add all widgets (ovelays)
    for (int i=0; i<overlayComponents_.size(); i++)
    {
        widgetComponent = overlayComponents_[i]->getTarget();
        widgetComponent->getXml();
    }
    
}

void WidgetViewComponent::clearCanvas()
{
    // clear the entire canvas
    
    // clear the array of widget overlay pointers
    overlayComponents_.clear();
    
    // delete all children will make sure actual overlays are deleted
    canvasArea_->deleteAllChildren();
    
}



void WidgetViewComponent::setTabNameXml(const String& tabName)
{
    
    // TODO: don't need to store tabName with this
    tabName_ = tabName;
    
    xml_->setAttribute("name", tabName_);

    setTabInEigenD();
}

void WidgetViewComponent::setAspectXml()
{
    xml_->setAttribute("aspectMode", canvasAspectMode_);
    xml_->setAttribute("widthAspect", canvasWidthAspect_);
    xml_->setAttribute("heightAspect", canvasHeightAspect_);

    setTabInEigenD();
}

int WidgetViewComponent::getNumWidgets()
{
    return overlayComponents_.size();
}

XmlElement* WidgetViewComponent::getWidgetXml(int i)
{
    // get the xml element representing a widget on the canvas
    return overlayComponents_[i]->getTarget()->getXml();
}

void WidgetViewComponent::addWidgetFromXml(XmlElement* widgetXml, bool setInEigenD)
{
    // add widget to canvas from an xml element representing the widget
    XmlElement* bounds = widgetXml->getChildByName("bounds");
    float x = (float)bounds->getDoubleAttribute("x");
    float y = (float)bounds->getDoubleAttribute("y");
    float width = (float)bounds->getDoubleAttribute("width");
    float height = (float)bounds->getDoubleAttribute("height");
    float textHeight = (float)bounds->getDoubleAttribute("textHeight", height/8);
    String textPosition = bounds->getStringAttribute("textPosition", "top");
    String path = widgetXml->getStringAttribute("path");
    XmlElement* domain = widgetXml->getChildByName("domain");

    if (domain)
    {
        juce::String type = domain->getStringAttribute("type");
        if (juce::String("bool") == type && path.contains(juce::String("talker")))
        {
            domain->setAttribute("type", "trigger");
            widgetXml->setAttribute("type", "triggerButton");
        }
    }
    
    int widgetIndex;
    WidgetOverlayComponent *widgetOverlay = addWidgetToCanvas(widgetXml, x, y, width, height, textHeight, textPosition,0,&widgetIndex);
    WidgetComponent *widget = widgetOverlay->getTarget();
 
    
    //--------------------------------------------------------------------------
    // update model
    //--------------------------------------------------------------------------
    
    if(setInEigenD)
    {
        incSessionChanges();
        
        // send new widget xml to eigenD
        String widgetXmlDoc = widgetXml->createDocument("", false, false);
        //std::cout << widgetXmlDoc;
        
        XmlRpcValue params, result;
        params.clear();
        params[0] = tabIndex_;
        params[1] = XMLRPCManager::juceToStdString(widgetXmlDoc);
        
        try {
            XMLRPCManager_->execute("addWidget", params, result, true);
        }
        catch (XMLRPCAbortException e) {
            // app is shutting down after an xmlrpc has blocked so bail
            return;
        }

        widget->setIndex(widgetIndex);
        widget->setWidgetInEigenD();
    }
    else
    {
        widget->setIndex(widgetIndex);
    }
}


//------------------------------------------------------------------------------
// XMLRPC Management
//------------------------------------------------------------------------------

void WidgetViewComponent::setTabInEigenD()
{
    // sends this tab to eigenD
    XmlRpcValue params, result;
    
    params.clear();
    params[0] = tabIndex_;
    params[1] = XMLRPCManager::juceToStdString(xml_->createDocument("", false, false));

    try {
        XMLRPCManager_->execute("setTab", params, result, true);
    }
    catch (XMLRPCAbortException e) {
        // app is shutting down after an xmlrpc has blocked so bail
        return;
    }

    incSessionChanges();

}
