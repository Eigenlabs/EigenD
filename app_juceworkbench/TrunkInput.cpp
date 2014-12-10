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

#include "TrunkInput.h"

TrunkInput::TrunkInput (Trunk* t,String id,int apertureIndex,float zoomFactor,bool input)
{

    t_=t;
    id_=id;
    zoomFactor_=zoomFactor;
    trueX_=t_->getApertureX(apertureIndex);
    trueY_=t_->getApertureY(apertureIndex);

    index_=apertureIndex;
    input_=input;
    indexChanged_=false;

    setBounds(trueX_*zoomFactor,trueY_*zoomFactor,8.0f*zoomFactor, 8.0f*zoomFactor);
    dragging_=false;
    removeOnDragEnd_=false;
    move_init_=false;
}

TrunkInput::~TrunkInput()
{
    pic::logmsg()<<"TrunkInput destructor"<<std::string(id_.toUTF8())<<" input="<<input_;
}

void TrunkInput::setApertureIndex(int apertureIndex)
{
    int oldIndex=index_;
    index_=apertureIndex;
    if (!indexChanged_)
    {
        if (oldIndex!=index_)
        {
            pic::logmsg()<< "TrunkInput: index changed from "<<oldIndex<<" to "<<index_;
            indexChanged_=true;
        }
    }
}

bool TrunkInput::isDragging()
{
    return dragging_;
}

void TrunkInput::removeOnDragEnd()
{
    pic::logmsg()<<"TrunkInput : set removeOnDragEnd_ to true";
    id_="toBeDeleted";
    removeOnDragEnd_=true;
}

Trunk* TrunkInput::getTrunk()
{
    return t_;
}

int TrunkInput:: getApertureIndex()
{
    return index_;
}

bool TrunkInput::isInput()
{
    return input_;
}

bool TrunkInput:: isOutput()
{
    return !input_;
}

void TrunkInput::paint (Graphics& g)
{
    if(!removeOnDragEnd_)
    {
        Colour col=t_->getWireColour(id_);
        g.setColour (col);
        g.fillEllipse (0.0f, 0.0f, getWidth(), getHeight());
    }
}

String TrunkInput::getID()
{
    return id_;
}
void TrunkInput::resized()
{
}

void TrunkInput::moved()
{
}

void TrunkInput::mouseEnter (const MouseEvent& e)
{
    setMouseCursor(t_->getMouseCursor());
    if(t_->getTool()==ToolManager::MOVETOOL)
    {
        t_->setSelected(true,id_);
        t_->getMainPanel()->trunkInputMouseEnter(e);
    }
    else if(t_->getTool()==ToolManager::DELETETOOL)
    {
        t_->setSelected(true,id_);
    }

}

void TrunkInput::mouseExit (const MouseEvent& e)
{
    if(t_->getTool()==ToolManager::MOVETOOL)
    {
        t_->setSelected(false,id_);
        t_->getMainPanel()->trunkInputMouseExit(e);
    }
}

void TrunkInput::mouseDown (const MouseEvent& e)
{ 	
    if(t_->getTool()==ToolManager::MOVETOOL)
    {
        setMouseCursor(t_->getMainPanel()->getCursor(ToolManager::GRAB));
        t_->getMainPanel()->trunkInputMouseDown(e);
        mouseDownX=getX();
        mouseDownY=getY();
        t_->setSelected(true,id_);
        Component* c=getParentComponent();
        c->repaint();
    }
}

void TrunkInput::mouseDrag (const MouseEvent& e)
{   
    if(t_->getTool()==ToolManager::MOVETOOL)
    {
        dragging_=true;
        if(!move_init_)
        {
            t_->getMainPanel()->initTrunkInputMove(getID());
            move_init_=true;
        }
        t_->getMainPanel()->trunkInputMouseDrag(e);

    }
}

void TrunkInput::mouseUp (const MouseEvent& e)
{

    move_init_=false;
    if(t_->getTool()==ToolManager::MOVETOOL)
    {
        //setMouseCursor(MouseCursor::DraggingHandCursor);
        if(e.mouseWasClicked())
        {
           toBack(); 
        }
        else
        {
            t_->getMainPanel()->trunkInputMouseUp(e);
            dragging_=false;
            if (removeOnDragEnd_)
            {
                t_->doRemoveInput(this);
            }
        } 
    }
    else if(t_->getTool()==ToolManager::DELETETOOL)
    {
        if (e.mouseWasClicked())
        {
            t_->setSelected(false,id_);
            //t_->getMainPanel()->removeRouteFromRoutingElement(id_, t_);
            t_->getMainPanel()->removeRouteFromTrunkAssembly(id_, t_->getAssemblyId());
        }
    }
    else if(t_->getTool()==ToolManager::WIRINGTOOL||t_->getTool()==ToolManager::WIRING_LEFT)
    {
        // XXX is this needed anymore?
        if(e.mouseWasClicked())
        {
            t_->toggleSelected(id_);
            Component* c=getParentComponent();
            c->repaint();
        }
    }
}

void TrunkInput::onDrag(int x, int y)
{

    if (x<0)
    {
        x=0;
    }
    else if (x>t_->getXInputRange())
    {
          x=t_->getXInputRange();
    }

    if (y<0)
    {
        y=0;
    }
    else if(y>(t_->getYInputRange()))
    {
        y=t_->getYInputRange();
    }

//    pic::logmsg()<<"y="<<y<<" trunkheight="<<t_->getHeight()<<" zoomFactor_="<<zoomFactor_;
    t_->setInputTopLeftPosition(this,x,y);

    int rx=t_->getX()+x;
    int ry=t_->getY()+y;

    if ((!t_->getMainPanel()->inCentralViewPort(rx,ry)))
    {
        t_->getMainPanel()->autoComponentDrag(rx,ry,this);
    }
    else
    {
        t_->getMainPanel()->stopAutoDrag();
    }
}

int TrunkInput::getConnectionX()
{
    return getX()+4.0f*zoomFactor_;
}

int TrunkInput::getConnectionY()
{
    return getY()+4.0f*zoomFactor_;
}

int TrunkInput::getTrueConnectionX()
{
    return trueX_+4;
}

int TrunkInput::getTrueConnectionY()
{
    return trueY_+4;
}

void TrunkInput::doAutoDrag(int dx,int dy)
{
    int x=getX()+dx;
    int y=getY()+dy;

    if (x<0)
    {
        x=0;
        t_->getMainPanel()->stopAutoDrag();
    }
    else if (x>t_->getXInputRange())
    {
          x=t_->getXInputRange();
          t_->getMainPanel()->stopAutoDrag();
    }

    if (y<0)
    {
        y=0;
        t_->getMainPanel()->stopAutoDrag();
    }
    else if(y>(t_->getYInputRange()))
    {
        y=t_->getYInputRange();
        t_->getMainPanel()->stopAutoDrag();
    }

    t_->setInputTopLeftPosition(this,x,y);

    t_->getMainPanel()->thingChanged();
}

void TrunkInput::onMoveEndTest()
{
    pic::logmsg()<<"TrunkInput::onMoveEndTest";
    if (!checkRemove(t_->getApertureIndex(getX(),getY(), true),getX(),getY()))
    {
        // for all trunk inputs(with same input/outputness as this) with same hash
        // set pos does it for all
        pic::logmsg()<<"TrunkInput setPos";
        setPos(getX(),getY(),true);
        t_->setSelected(false,id_);
        t_->getMainPanel()->thingMoved();
    }
}

void TrunkInput::onMoveEnd()
{
    pic::logmsg()<<"TrunkInput::onMoveEnd";
    if (removeOnDragEnd_)
    {
        t_->doRemoveInput(this);
    }

    else
    {
        if (!checkRemove(t_->getApertureIndex(getX(),getY(), true),getX(),getY()))
        {
            pic::logmsg()<<"TrunkInput setPos";
            setPos(getX(),getY(),true);
            t_->setSelected(false,id_);
            t_->getMainPanel()->thingMoved();
        }
    }
}

void TrunkInput::setPos(int x, int y, bool endsAllowed)
{
    t_->setInputIndex(this,t_->getApertureIndex(x,y, endsAllowed)); 

    trueX_=t_->getApertureX(index_);
    trueY_=t_->getApertureY(index_);

//    int ax=(float)t_->getApertureX(index_)*zoomFactor_;
//    int ay=(float)t_->getApertureY(index_)*zoomFactor_;

    t_->setInputTopLeftPosition(this,trueX_*zoomFactor_,trueY_*zoomFactor_);
    t_->saveInputProps(this);

    Component* c=getParentComponent();
    c->getParentComponent()->repaint();
}

void TrunkInput::setPos(int x, int y)
{
    trueX_=(float)x/zoomFactor_;
    trueY_=(float)y/zoomFactor_;
    setTopLeftPosition(x,y);
}

bool TrunkInput::checkRemove2(int index1, int x, int y)
{
    if(indexChanged_)
    {
        return removeIfEqual(index1,t_->getApertureIndex(x,y,true));
    }

    return false;
}

bool TrunkInput::checkRemove(int index1, int x, int y)
{
    pic::logmsg() <<"checkRemove: index1="<<index1<<" x="<<x<<" y="<<y;
    int x2;
    int y2;

    if (isInput())
    {
          x2= t_->getXoutInternal(id_);
          y2= t_->getYoutInternal(id_);
    }
    else
    {
           x2= t_->getXinInternal(id_);
           y2= t_->getYinInternal(id_);
    }

    int index2 =t_->getApertureIndex((x2-t_->getX()),(y2-t_->getY()),true); 

    return removeIfEqual(index1,index2);
}

bool TrunkInput::removeIfEqual(int index1, int index2)
{
    if (index1==index2)
    {
        t_->setSelected(false,id_);
        t_-> getMainPanel()->removeRouteFromRoutingElement(id_,t_);
        return true;
    }
    else
    {
        return false;
    }
}

void TrunkInput::reposition(bool save)
{
    int maxIndex=t_->getNumSections();
    if (abs(index_)>maxIndex && index_!=1000)
    {
        index_=maxIndex;
        if(save)
        {
            save_props();
        }
    }

    trueX_=t_->getApertureX(index_);
    trueY_=t_->getApertureY(index_);

    setBounds(trueX_*zoomFactor_,trueY_*zoomFactor_,8.0f*zoomFactor_, 8.0f*zoomFactor_);
}

void TrunkInput::doZoom(float zoomFactor)
{
    zoomFactor_=zoomFactor;
    setBounds(trueX_*zoomFactor_,trueY_*zoomFactor_,8.0f*zoomFactor_, 8.0f*zoomFactor_);
}

void TrunkInput::save_props()
{
    int i;
    if(isInput())
    {
        i=1;
    }
    else
    {
        i=2;
    }

    PropertyStore* p =t_->getProps();

    String listname="input"+String(id_.hashCode());
    p->set_list(listname,i,String(index_));
    t_->save_props();
}
