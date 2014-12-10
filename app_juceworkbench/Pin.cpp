/*
 Copyright 2012-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include "Pin.h"

Pin::Pin(String id,int x, int y,float zoomFactor=1.0f,bool loose=false)
{
    trueWidth_=PINWIDTH;
    trueHeight_=PINHEIGHT;
    trueX_=(float)x/zoomFactor;
    trueY_=(float)y/zoomFactor;
    zoomFactor_=zoomFactor;
    setBounds(x,y,zoomFactor*trueWidth_, zoomFactor*trueHeight_);
    id_=id;
    mouseDownX_=0;
    mouseDownY_=0;
    loose_=loose;
    highlight_=false;
    selected_=false;
    mouseMode_=0;
    currentIndex_=0;
    move_init_=false;
    needsRefresh_=false;
}

Pin::Pin(PropertyStore* p,float zoomFactor)
{
    props_=p;
    trueWidth_=PINWIDTH;
    trueHeight_=PINHEIGHT;
    zoomFactor_=zoomFactor;

    trueX_=props_->get_number("x");
    trueY_=props_->get_number("y");;
    int x=trueX_*zoomFactor_;
    int y=trueY_*zoomFactor_;

    setBounds(x,y,zoomFactor*trueWidth_, zoomFactor*trueHeight_);
    id_=props_->get_string("id");
    mouseDownX_=0;
    mouseDownY_=0;
    loose_=true;
    highlight_=false;
    selected_=false;
    mouseMode_=0;
    currentIndex_=0;
    move_init_=false;
    needsRefresh_=false;
}

void Pin::setDragging(bool dragging)
{
    dragging_=dragging;
//    pic::logmsg()<<"***** Pin set dragging"<<id_<< " "<<this<< " dragging="<<dragging_;
}

bool Pin::wiringAllowed()
{
    Box* b=findParentComponentOfClass<Box>();
    if(b!=0)
    {
        return b->wiringAllowed();
    }

    return true; 
}

int Pin::getNumWires()
{
    return wires_.size();
}

void Pin::onDragEnd()
{
//    pic::logmsg()<<"Pin::on drag end: needsRefresh="<<needsRefresh_<<"  "<<this;
//    std::cout<<"Pin::on drag end: needsRefresh="<<needsRefresh_<<"  "<<this<<std::endl;
    setDragging(false);
    if(needsRefresh_)
    {
        refreshBox();
    }
}

void Pin::changed(PropertyStore* props)
{
    if (props!=0)
    {   
        getMainPanel()->clearWireMoveList();
        getMainPanel()->initRoutingElementMove(this);
        props_=props;
        trueX_=props_->get_number("x");
        trueY_=props_->get_number("y");

        int x=trueX_*zoomFactor_;
        int y=trueY_*zoomFactor_;
        setBounds(x,y,zoomFactor_*trueWidth_, zoomFactor_*trueHeight_);
        id_=props_->get_string("id");
        getMainPanel()->thingMoved();
        getMainPanel()->endThingMoved();
    }
}

PropertyStore* Pin::getProps()
{
    return props_;
}

int Pin::getInputAt(int x, int y)
{
    return 1;
}

int Pin::getOutputAt(int x, int y)
{
    return 1;
}

bool Pin::isConnecting()
{
    return mouseMode_==CONNECTING;
}

void Pin::highlight(bool hl)
{
    highlight_=hl;
}

void Pin::setSelected(bool s, String id)
{
    if (isLoosePin())
    {
        selected_=s;
        if(selected_)
        {
            toBehind(getMainPanel()->getDisplayLayer());
            repaint();
        }
        else
        {
            toBack();
        }
    }
    else
    {
        Box* b=findParentComponentOfClass<Box>();
        if(b!=0)
        {
            b->highlight(s);
        }
    }
}

void Pin::setForegrounded(bool foregrounded, String id)
{
    if(isLoosePin())
    {
        if(foregrounded)
        {
            setAlwaysOnTop(true);
            getMainPanel()->getDisplayLayer()->toBack();
        }
        else
        {
            setAlwaysOnTop(false);
            toBehind(getMainPanel()->getDisplayLayer());
        }
    }
}


void Pin::doZoom(float zoomFactor)
{
    zoomFactor_=zoomFactor;
    float x=trueX_*zoomFactor;
    float y=trueY_*zoomFactor;
    setBounds(x,y,trueWidth_*zoomFactor,trueHeight_*zoomFactor);
    repaint();
}

String Pin::getId()
{
    return id_;
}

String Pin::get_id()
{
    return id_;
}

String Pin::get_hash(String wireId)
{
    return get_id();
}

void Pin::addWire(Wire* w)
{
    wires_.insert(std::pair<String, Wire*>(w->getId(),w));

    if(!w->isForcedChange() && !isLoosePin())
    {
        //std::cout<<"pin::addWire"<<std::endl;
        refreshBox();
    }
}

void Pin::refreshBox()
{
    //std::cout<<"Pin::RefreshBox pin="<<this<<std::endl;
    Box* b =findParentComponentOfClass<Box>();
    if(b!=0)
    {
        b->refresh();
    }
    needsRefresh_=false;
}

void Pin::removeWire(Wire* w)
{
    doRemoveWire(w);
    //std::cout<<"pin removeWire id="<<id_<<" isdragging="<< isDragging()<< " "<<this<<" isForcedChange"<<w->isForcedChange()<<std::endl;

    //if(!w->isForcedChange() && !isLoosePin())
    if(!isLoosePin())
    {
        if(!isDragging())
        {
            //std::cout<<"    pin::call refreshBox"<<std::endl;
            refreshBox();
        }
        else
        {
            //pic::logmsg()<<"   Pin is being dragged cant delete it yet!"<<this;
            //std::cout<<"   Pin is being dragged cant delete it yet!"<<this<<std::endl;
            needsRefresh_=true;
        }
    }
}

void Pin::doRemoveWire(Wire* w)
{
    int n=wires_.erase(w->getId());
    if(n==0)
    {
        pic::logmsg() << "Pin: did not remove wire from wires_ "<<std::string(get_id().toUTF8()); 
    }
}

Wire* Pin::getWire()
{
    std::map<int, Wire*> routes;
    for(std::map<String, Wire*>::iterator i=wires_.begin();i!=wires_.end();i++)
    {

        int h=(i->second)->getRouteHash();
        std::map<int,Wire*>::iterator routeIter=routes.find(h);
        if (routeIter==routes.end())
        {
            routes.insert(std::pair<int,Wire*>(h,i->second));
        }
    }

    if(currentIndex_>=routes.size())
    {
        currentIndex_=0;
    }
    unsigned count=0;

    for(std::map<int, Wire*>::iterator i=routes.begin();i!=routes.end();i++)
    {
        if(count==currentIndex_)
        {
            return i->second;
        }
        count++;
    }

    return 0;

}

void Pin::highlightParentBox(bool val)
{
    Box* b=findParentComponentOfClass<Box>();
    if(b!=0)
    {
        b->highlight(val);
        b->repaint();
    }
}

bool Pin::isLoosePin()
{
    return loose_;
}

Path Pin::getPathFrom(RoutingElement* other, String id)
{
    return getMainPanel()->getWirePath(other->getXout(id),other->getYout(id),getXin(id),getYin(id),1.1f);
}

void Pin::selectWire()
{
    if(!wires_.empty())
    {
        Wire* w=getWire();
        if(w!=0)
        {

            getMainPanel()->repaintWire(getMainPanel()->getSelectedWire());
            getMainPanel()->selectWire(w);
            getMainPanel()->repaintWire(w);
        }
    }
}

void Pin::mouseExit (const MouseEvent& e)
{

    if (!isLoosePin())
    {
        int tool=getMainPanel()->getTool();
        if(tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
        {
            highlightParentBox(false);
        }
    }
}

void Pin::setMouseMode(const MouseEvent& e)
{
    if(e.mods.isCommandDown())
    {
        mouseMode_=DISCONNECTING;
    }
    else
    {
        mouseMode_=CONNECTING;
    }
    setDragging(true);
}

void Pin::doSetPositionProps()
{
    if (props_!=0)
    {
        int x=getX();
        props_->set_number("x",(float)x/zoomFactor_);
        int y =getY();
        props_->set_number("y",(float)y/zoomFactor_);
        getMainPanel()->save_props(props_->get_string("id"));
    }
}

void Pin::doWiringToolMouseClick()
{
    // cycle through wires on clicks
    // cycle through wires on clicks and highlight them 
    // the highlighted wire will be used for subsequent disconnect
    // increment currentIndex_
    currentIndex_++;
    pic::logmsg()<<"Pin doWiringToolMouseClick: currentIndex="<<currentIndex_<<"wires_.size()="<<wires_.size();
    Wire* w =getWire();
    if(w!=0)
    {
        getMainPanel()->repaintWire(getMainPanel()->getSelectedWire());
        getMainPanel()->selectWire(w);
        getMainPanel()->repaintWire(w);
    }
}

void Pin::doAutoDrag(int dx,int dy)
{
    mouseDownX_=mouseDownX_+dx;
    mouseDownY_=mouseDownY_+dy;
    int x=getX()+dx;
    int y=getY()+dy;
    x=getMainPanel()->conditionX(x,getWidth());
    y=getMainPanel()->conditionY(y,getHeight());
    setTopLeftPosition(x,y);
    trueX_=(float)(x)/zoomFactor_;
    trueY_=(float)(y)/zoomFactor_;
    getMainPanel()->thingMoved();
}

MainComponent* Pin::getMainPanel() const throw()
{
    return findParentComponentOfClass<MainComponent>();
}



DestinationPin::DestinationPin(String id,int x, int y,float zoomFactor=1.0f,bool loose=false):Pin(id,x,y,zoomFactor,loose)
{
    dragging_=false;
}

DestinationPin::DestinationPin(PropertyStore* p,float zoomFactor):Pin(p,zoomFactor)
{
    dragging_=false;
}

bool DestinationPin::isDragging()
{
    return dragging_;
}

void DestinationPin::paint (Graphics& g)
{
    int w=getWidth();
    int h=getHeight();
    float el_x;
    float rect_x;
    float rect_w;
    Path p;

    if (isLoosePin())
    {
        el_x=0;
        rect_x=0.25;
        rect_w=0.75;

        if (selected_)
        {
            g.setColour(Colour(selectedWireColour));
        }
        else if (highlight_)
        {
            g.setColour(Colour(wireColour));
        }
        else
        {
            g.setColour (Colour(looseWireColour));
        }

        p.addEllipse (el_x*w, 0.36*h, 0.5*w, 0.29*h);
        g.fillPath (p);

    }
    else
    {
        if(getMainPanel()->showOutlines())
        {
            g.setColour (Colours::black);
            g.drawRect(0,0,getWidth(),getHeight(),1);
        }

        el_x=0;
        rect_x=0.25;
        rect_w=0.75;
        g.setColour (Colours::black);
        p.addRectangle (0.5*w,  0.393*h , 0.688*w, 0.214*h);
        p.addEllipse (0.0, 0.32*h, 0.357*h, 0.375*h);

        g.fillPath (p);

        Path p2;
        g.setColour (Colour(1,209,20));
        p2.addRectangle (rect_x*w,  0.43*h , rect_w*w, 0.14*h);
        p2.addEllipse (0.0625*w, 0.357*h, 0.5*w, 0.5*w);
        g.fillPath (p2);
    }
}

void DestinationPin::doLoosePinDrag(int x, int y)
{
    getMainPanel()->routeThroughPeg(this,x,y);
    trueX_=(float)(x)/zoomFactor_;
    trueY_=(float)(y)/zoomFactor_;
    setTopLeftPosition(x, y);

    if (!getMainPanel()->inCentralViewPort(x,y))
    {
//        pic::logmsg()<<"doLoosePinDrag ("<<x<<","<<y<<") not in central viewport";
        getMainPanel()->autoComponentDrag(x,y,this);
    }
    else
    {
        getMainPanel()->stopAutoDrag();
    }

}


bool DestinationPin::hitTest(int x, int y)
{
    if(isLoosePin())
    {
        return true;
    }
    else
    {
        if(isShowing())
        {
            int h=getHeight();
            int w=getWidth();
            int miny=0.357f*(float)h;
            int maxy=miny+0.5f*(float)w;
            return (y>miny)&&(y<maxy);
        }
        else
        { 
           return false;
        }
    }
}

int DestinationPin::getXin(String id)
{
    if (isLoosePin())
    {
        return trueX_ + 0.25*trueWidth_; 
    }
    else
    {
       Box* b=findParentComponentOfClass<Box>();
       if(b==0)
       {
           //std::cout<<" ***** DestinationPin: getXin b=0"<<std::endl;
           return 0;
       }
       return b->getDstX()+0.25*trueWidth_;
    }
}

int DestinationPin::getYin(String id)
{
    if (isLoosePin())
    {
        return trueY_+0.5*trueHeight_;
    }
    else
    {

        Box* b=findParentComponentOfClass<Box>();
        if(b==0)
        {
            //std::cout<<" ***** DestinationPin: getYin b=0"<<std::endl;
            return 0;
        }
        return b->getDstY()+0.5*trueHeight_;
    }
}

int DestinationPin::getXout(String id)
{
    return getXin(id);
}

int DestinationPin::getYout(String id)
{
    return getYin(id);
}

void DestinationPin::mouseEnter (const MouseEvent& e)
{
    if(!isLoosePin())
    {
        int tool=getMainPanel()->getTool();
        if(tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
        {
            setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRING_LEFT));
            getMainPanel()->setTool(ToolManager::WIRING_LEFT);
            highlightParentBox(true);
            selectWire();
        } 
    } 

    setMouseCursor(getMainPanel()->getCursor());
}

void DestinationPin::mouseDown(const MouseEvent& e)
{
    pic::logmsg()<<"DestinationPin mouseDown"<<std::string(getId().toUTF8())<<"  "<<this;

    int tool=getMainPanel()->getTool();
    if (isLoosePin())
    {
        if(tool==ToolManager::MOVETOOL)
        {
            pic::logmsg()<<"DstLoosePin mouseDown"<<std::string(getId().toUTF8());
            //setMouseCursor(MouseCursor::DraggingHandCursor);
            mouseDownX_=getX();
            mouseDownY_=getY();
        }
    }

    else
    {
        if (tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
        {
            setDragging(true);
            getMainPanel()->dstPinMouseDown(e);
        }
    }

    if (tool==ToolManager::ZOOMTOOL)
    {
        getMainPanel()->mouseDown(e.getEventRelativeTo(getMainPanel()));
    }

}   

	
void DestinationPin::mouseUp(const MouseEvent& e)
{
    pic::logmsg()<<"DestinationPin mouseUp"<<std::string(getId().toUTF8())<<"  "<<this;
    move_init_=false;
    int tool=getMainPanel()->getTool();
    if (isLoosePin())
    {
        if(tool==ToolManager::MOVETOOL)
        {
            doSetPositionProps();
            setMouseCursor(getMainPanel()->getCursor());
            getMainPanel()->stopAutoDrag();
            getMainPanel()->endThingMoved();
            getMainPanel()->dropLoosePin(e);
        }

        else if(tool==ToolManager::DELETETOOL)
        {
           if(e.mouseWasClicked())
           {
                if(getNumWires()==0)
                {
                    getMainPanel()->removeLoosePin(this);
                }
           }
        }

    }

    else
    {
        if(tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
        {
            setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRING_LEFT));
            if(e.mouseWasClicked())
            {
                doWiringToolMouseClick();
            }
            else if(mouseMode_==CONNECTING)
            {
                getMainPanel()->dstPinMouseUp(e);
                mouseMode_=0;
            }
            else if(mouseMode_==DISCONNECTING)
            {
                getMainPanel()->dstPinMouseUp(e);
                mouseMode_=0;
            }
            setDragging(false);
            if(needsRefresh_)
            {
                //std::cout<<"DestinationPin::mouseUp"<<std::endl;
                refreshBox();
            }
        }
        else if(tool==ToolManager::POINTERTOOL)
        {
            pic::logmsg()<<"DestinationPin: size of wires_= "<<wires_.size();
        }
    }

    if(tool==ToolManager::ZOOMTOOL)
    {
        if(e.mouseWasClicked())
        {
            getMainPanel()->doMouseClick(e.getEventRelativeTo(getMainPanel()));
        }
        else
        {
            getMainPanel()->mouseUp(e.getEventRelativeTo(getMainPanel()));
        }
    }

}

void DestinationPin::mouseDrag (const MouseEvent& e)
{
    int tool=getMainPanel()->getTool();
    if (isLoosePin())
    {
        if(tool==ToolManager::MOVETOOL)
        {
//            pic::logmsg()<<"DestinationPin:: mouseDrag";
            if (!move_init_)
            {
                move_init_=true;
                getMainPanel()->initDestinationPinMove(getId());
            }
            int x=mouseDownX_+e.getDistanceFromDragStartX();
            int y=mouseDownY_+e.getDistanceFromDragStartY();

            x=getMainPanel()->conditionX(x,getWidth());
            y=getMainPanel()->conditionY(y,getHeight());
            doLoosePinDrag(x,y);

            bool hl=(getMainPanel()->doHighLights(e));
            getMainPanel()->highlightPin(this,hl);
            getMainPanel()->thingMoved();
        }
    }
    else
    {
        if (tool== ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
        {
            if(mouseMode_==CONNECTING)
            {
                getMainPanel()->dstPinMouseDrag(e);
            }
            else if(mouseMode_==DISCONNECTING)
            {
                pic::logmsg()<<" DestinationPin: disconnecting drag "<<std::string(getId().toUTF8());
                getMainPanel()->dstPinMouseDisconnectDrag(e);
            }
        }
    }

    if(tool==ToolManager::ZOOMTOOL)
    {
       getMainPanel()->mouseDrag(e.getEventRelativeTo(getMainPanel()));
    }

}

bool DestinationPin::isRevConnect()
{
    return false;
}

SourcePin::SourcePin(String id,int x, int y,float zoomFactor=1.0f,bool loose=false):Pin(id,x,y,zoomFactor,loose)
{
    dragging_=false;
}

SourcePin::SourcePin(PropertyStore* p,float zoomFactor):Pin(p,zoomFactor)
{
    dragging_=false;
}

bool SourcePin::isDragging()
{
    return dragging_;
}


void SourcePin::paint (Graphics& g)
{
    int w=getWidth();
    int h=getHeight();
    Path p;

    if (isLoosePin())
    {
        float rect_w;
        float el_x;

        rect_w=0.75;
        el_x=rect_w-0.25;
        if (selected_)
        {
            g.setColour(Colour(selectedWireColour));
        }
        else if (highlight_)
        {
            g.setColour(Colour(wireColour));
        }
        else
        {
            g.setColour (Colour(looseWireColour));
        }

        p.addEllipse (el_x*w, 0.36*h, 0.5*w, 0.29*h);
        g.fillPath (p);
    }
    else
    {
        if(getMainPanel()->showOutlines())
        {
            g.setColour (Colours::black);
            g.drawRect(0,0,getWidth(),getHeight(),1);
        }

        g.setColour (Colours::black);
        int w=getWidth();
        int h=getHeight();
        p.addRectangle (0,  0.393*h , 0.688*w, 0.214*h);
        p.addEllipse (0.375*w, 0.32*h, 0.357*h, 0.357*h);
        g.fillPath (p);

        Path p2;
        g.setColour (Colour(181,0,1));
        p2.addRectangle (0,  0.43*h , 0.688*w, 0.14*h);
        p2.addEllipse (0.4375*w, 0.357*h, 0.5*w, 0.5*w);
        g.fillPath (p2);
    }
}

void SourcePin::doLoosePinDrag(int x, int y)
{
    getMainPanel()->routeThroughPeg(this,x,y);
    trueX_=(float)(x)/zoomFactor_;
    trueY_=(float)(y)/zoomFactor_;
    setTopLeftPosition(x, y);

    if (!getMainPanel()->inCentralViewPort(x,y))
    {
        getMainPanel()->autoComponentDrag(x,y,this);
    }
    else
    {
        getMainPanel()->stopAutoDrag();
    }

}

bool SourcePin::hitTest(int x, int y)
{
    if(isLoosePin())
    {
        return true;
    }
    else
    {
        if(isShowing())
        {
            int h=getHeight();
            int w=getWidth();
            int miny=0.357f*(float)h;
            int maxy=miny+0.5f*(float)w;
            return (y>miny)&&(y<maxy);
        }
        else
        {
            return false;
        }
    }

}

int SourcePin::getXin(String id)
{
    if (isLoosePin())
    {
        return trueX_+0.75*trueWidth_;
    }
    else
    {
        Box* b=findParentComponentOfClass<Box>();
        return b->getSrcX()+0.75*trueWidth_;
    }
}

int SourcePin::getYin(String id)
{
    if (isLoosePin())
    {
        return trueY_+0.5*trueHeight_;
    }
    else
    {
        Box* b=findParentComponentOfClass<Box>();
        return b->getSrcY()+0.5*trueHeight_;
    }

}

int SourcePin::getXout(String id)
{
    return getXin(id);
}

int SourcePin::getYout(String id)
{
    return getYin(id);
}


void SourcePin::mouseEnter (const MouseEvent& e)
{
    if(!isLoosePin())
    {
        int tool=getMainPanel()->getTool();
        if(tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
        {
            setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRINGTOOL));
            getMainPanel()->setTool(ToolManager::WIRINGTOOL);
            pic::logmsg()<<"sourcePin mouseEnter: tool=wiring";
            highlightParentBox(true);
            selectWire();
        } 
    }

    setMouseCursor(getMainPanel()->getCursor());
}


void SourcePin::mouseDown(const MouseEvent& e)
{
    int tool=getMainPanel()->getTool();
    //pic::logmsg()<<"SourcePin: mouseDown "<<mouseMode_;
    pic::logmsg()<<"SrcPin mouseDown"<<std::string(getId().toUTF8())<< "  "<<this;
    if (isLoosePin())
    {
        if(tool==ToolManager::MOVETOOL)
        {
            //setMouseCursor(MouseCursor::DraggingHandCursor);
            mouseDownX_=getX();
            mouseDownY_=getY();
        }
    }
    else
    {
        if (tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
        {
            setDragging(true);
            getMainPanel()->srcPinMouseDown(e);
        }
    }

    if (tool==ToolManager::ZOOMTOOL)
    {
        getMainPanel()->mouseDown(e.getEventRelativeTo(getMainPanel()));
    }

}


void SourcePin::mouseUp(const MouseEvent& e)
{
    move_init_=false;
    int tool=getMainPanel()->getTool();
    pic::logmsg()<<"SourcePin mouseUp"<<std::string(getId().toUTF8())<<"  "<<this;
    if (isLoosePin())
    {
        if(tool==ToolManager::MOVETOOL)
        {
            doSetPositionProps();
            setMouseCursor(getMainPanel()->getCursor());
            getMainPanel()->endThingMoved();
            getMainPanel()->stopAutoDrag();
            pic::logmsg()<<"srcLoosePin mouseUp"<<std::string(getId().toUTF8());
            getMainPanel()->dropLoosePin(e);
        }
        else if(tool==ToolManager::DELETETOOL)
        {
           if(e.mouseWasClicked())
           {
                if(getNumWires()==0)
                {
                    getMainPanel()->removeLoosePin(this);
                }
           }
        }
    }
    else
    {
        if (tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
        {
            setMouseCursor(getMainPanel()->getCursor());
            if(e.mouseWasClicked())
            {
                doWiringToolMouseClick();
            }
            else if (mouseMode_==CONNECTING)
            {
                setMouseCursor(getMainPanel()->getCursor());
                getMainPanel()->srcPinMouseUp(e);

                mouseMode_=0;
            }
            else if (mouseMode_==DISCONNECTING)
            {
                getMainPanel()->srcPinMouseUp(e);
                mouseMode_=0;
            }
            setDragging(false);
            if(needsRefresh_)
            {
                std::cout<<"sourcePin::mouseUp"<<std::endl;
                refreshBox();
            }
        }
        else if(tool==ToolManager::POINTERTOOL)
        {
            pic::logmsg()<<"SourcePin: size of wires_= "<<wires_.size();
        }

    }

    if(tool==ToolManager::ZOOMTOOL)
    {
        if(e.mouseWasClicked())
        {
            getMainPanel()->doMouseClick(e.getEventRelativeTo(getMainPanel()));
        }
        else
        {
            getMainPanel()->mouseUp(e.getEventRelativeTo(getMainPanel()));
        }
    }

}

void SourcePin::mouseDrag (const MouseEvent& e)
{
    int tool=getMainPanel()->getTool();
    if (isLoosePin())
    {
        if(tool==ToolManager::MOVETOOL)
        {
            if (!move_init_)
            {
                move_init_=true;
                getMainPanel()->initSourcePinMove(getId());
            }

            int x=mouseDownX_+e.getDistanceFromDragStartX();
            int y=mouseDownY_+e.getDistanceFromDragStartY();

            x=getMainPanel()->conditionX(x,getWidth());
            y=getMainPanel()->conditionY(y,getHeight());
            doLoosePinDrag(x,y);

            bool hl=(getMainPanel()->doHighLights(e));
            getMainPanel()->highlightPin(this,hl);
            getMainPanel()->thingMoved();
        }
    }

    else
    {
        if(tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
        {
            if(mouseMode_==CONNECTING)
            {
                getMainPanel()->srcPinMouseDrag(e);
            }
            else if(mouseMode_==DISCONNECTING)
            {
                getMainPanel()->srcPinMouseDisconnectDrag(e);
            }
        }
//        else if(tool==ToolManager::MOVETOOL)
//        {
//            if (mouseMode_==DISCONNECTING)
//            {
//                getMainPanel()->srcPinMouseDisconnectDrag(e);
//            }
//        }
    }

    if(tool==ToolManager::ZOOMTOOL)
    {
       getMainPanel()->mouseDrag(e.getEventRelativeTo(getMainPanel()));
    }

}

bool SourcePin::isRevConnect()
{
    return false;
}

ReverseDestinationPin::ReverseDestinationPin(String id,int x, int y,float zoomFactor=1.0f,bool loose=false):DestinationPin(id,x,y,zoomFactor,loose)
{
}

ReverseDestinationPin::ReverseDestinationPin(PropertyStore* p,float zoomFactor):DestinationPin(p,zoomFactor)
{
}

void ReverseDestinationPin::paint (Graphics& g)
{
    int w=getWidth();
    int h=getHeight();
    Path p;

    if (isLoosePin())
    {
        float rect_w;
        float el_x;

        rect_w=0.75;
        el_x=rect_w-0.25;
        if (selected_)
        {
            g.setColour(Colour(selectedWireColour));
        }
        else if (highlight_)
        {
            g.setColour(Colour(wireColour));
        }
        else
        {
            g.setColour (Colour(looseWireColour));
        }

        p.addEllipse (el_x*w, 0.36*h, 0.5*w, 0.29*h);
        g.fillPath (p);
    }
    else
    {
        if(getMainPanel()->showOutlines())
        {
            g.setColour (Colours::black);
            g.drawRect(0,0,getWidth(),getHeight(),1);
        }

        g.setColour (Colours::black);
        int w=getWidth();
        int h=getHeight();
        p.addRectangle (0,  0.393*h , 0.688*w, 0.214*h);
        p.addEllipse (0.375*w, 0.32*h, 0.357*h, 0.357*h);
        g.fillPath (p);

        Path p2;
        g.setColour (Colour(1,209,20));
        p2.addRectangle (0,  0.43*h , 0.688*w, 0.14*h);
        p2.addEllipse (0.4375*w, 0.357*h, 0.5*w, 0.5*w);
        g.fillPath (p2);
    }
}

bool ReverseDestinationPin::hitTest(int x, int y)
{
    if(isLoosePin())
    {
        return true;
    }
    else
    {
        if(isShowing())
        {
            int h=getHeight();
            int w=getWidth();
            int miny=0.357f*(float)h;
            int maxy=miny+0.5f*(float)w;
            return (y>miny)&&(y<maxy);
        }
        else
        {
            return false;
        }
    }

}

int ReverseDestinationPin::getXin(String id)
{
    if (isLoosePin())
    {
        return trueX_+0.75*trueWidth_;
    }
    else
    {
        Box* b=findParentComponentOfClass<Box>();
        return b->getSrcX()+0.75*trueWidth_;
    }
}

int ReverseDestinationPin::getYin(String id)
{
    if (isLoosePin())
    {
        return trueY_+0.5*trueHeight_;
    }
    else
    {
        Box* b=findParentComponentOfClass<Box>();
        return b->getSrcY()+0.5*trueHeight_;
    }

}

int ReverseDestinationPin::getXout(String id)
{
    return getXin(id);
}

int ReverseDestinationPin::getYout(String id)
{
    return getYin(id);
}

bool ReverseDestinationPin::isRevConnect()
{
    return true;
}

ReverseSourcePin::ReverseSourcePin(String id,int x, int y,float zoomFactor=1.0f,bool loose=false):SourcePin(id,x,y,zoomFactor,loose)
{
}

ReverseSourcePin::ReverseSourcePin(PropertyStore* p,float zoomFactor):SourcePin(p,zoomFactor)
{
}

void ReverseSourcePin::paint (Graphics& g)
{
    int w=getWidth();
    int h=getHeight();
    float el_x;
    float rect_x;
    float rect_w;
    Path p;

    if (isLoosePin())
    {
        el_x=0;
        rect_x=0.25;
        rect_w=0.75;

        if (selected_)
        {
            g.setColour(Colour(selectedWireColour));
        }
        else if (highlight_)
        {
            g.setColour(Colour(wireColour));
        }
        else
        {
            g.setColour (Colour(looseWireColour));
        }

        p.addEllipse (el_x*w, 0.36*h, 0.5*w, 0.29*h);
        g.fillPath (p);

    }
    else
    {
        if(getMainPanel()->showOutlines())
        {
            g.setColour (Colours::black);
            g.drawRect(0,0,getWidth(),getHeight(),1);
        }

        el_x=0;
        rect_x=0.25;
        rect_w=0.75;
        g.setColour (Colours::black);
        p.addRectangle (0.5*w,  0.393*h , 0.688*w, 0.214*h);
        p.addEllipse (0.0, 0.32*h, 0.357*h, 0.375*h);

        g.fillPath (p);

        Path p2;
        g.setColour (Colour(181,0,1));
        p2.addRectangle (rect_x*w,  0.43*h , rect_w*w, 0.14*h);
        p2.addEllipse (0.0625*w, 0.357*h, 0.5*w, 0.5*w);
        g.fillPath (p2);
    }
}

bool ReverseSourcePin::hitTest(int x, int y)
{
    if(isLoosePin())
    {
        return true;
    }
    else
    {
        if(isShowing())
        {
            int h=getHeight();
            int w=getWidth();
            int miny=0.357f*(float)h;
            int maxy=miny+0.5f*(float)w;
            return (y>miny)&&(y<maxy);
        }
        else
        { 
           return false;
        }
    }
}

int ReverseSourcePin::getXin(String id)
{
    if (isLoosePin())
    {
        return trueX_ + 0.25*trueWidth_; 
    }
    else
    {
       Box* b=findParentComponentOfClass<Box>();
       return b->getDstX()+0.25*trueWidth_;
    }
}

int ReverseSourcePin::getYin(String id)
{
    if (isLoosePin())
    {
        return trueY_+0.5*trueHeight_;
    }
    else
    {

        Box* b=findParentComponentOfClass<Box>();
        return b->getDstY()+0.5*trueHeight_;
    }
}

int ReverseSourcePin::getXout(String id)
{
    return getXin(id);
}

int ReverseSourcePin::getYout(String id)
{
    return getYin(id);
}

bool ReverseSourcePin::isRevConnect()
{
    return true;
}


