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

#include "Peg.h"

Peg::Peg(PropertyStore* props, float zoomFactor)
{
    props_=props;
    zoomFactor_=zoomFactor;
    trueWidth_=40;
    trueHeight_=40;

    trueX_=props_->get_number("x");
    trueY_=props_->get_number("y");

    String listname;
    for (int i=0;i<props_->list_count();i++)
    {
        listname=props_->get_listkey(i);

        if (listname.startsWith("input"))
        {
            String wire_id=props_->get_list(listname,0);
            String input=props_->get_list(listname,1);
            if(input.contains("999"))
            {
                directionMap_.insert(std::pair<String, int> (wire_id,1));
            }
            wireIds_.insert(wire_id);
        }
    }


    int x=ceil(((float)trueX_*zoomFactor_)-0.5);
    int y=ceil(((float)trueY_*zoomFactor_)-0.5);
    setBounds(x,y,ceil(((float)zoomFactor*trueWidth_)-0.5), ceil(((float)zoomFactor*trueHeight_)-0.5));
    testId_=props_->get_string("id");
    move_init_=false;
    mouseOver_=false;
    lassoed_=false;
    foregrounded_=false;
    selected_=false;
    sticky_=props_->get_number("sticky");
}

Peg::~Peg()
{
}

bool Peg::isSticky()
{
    return sticky_;
}

void Peg::setSticky(bool shouldBeSticky)
{
    sticky_=shouldBeSticky;
    if(props_!=0)
    {
        if(sticky_)
        {
            props_->set_number("sticky",1);
        }
        else
        {
            props_->set_number("sticky",0);
        }
        getMainPanel()->save_props(props_->get_string("id"));
    }
}

void Peg::addId(String id, int input, int output)
{
    // XXX use input and output to determine direction wire should go over peg
    if((input==-999) || (input==999))
    {
        directionMap_.insert(std::pair<String, int> (id,1));
    }
        //std::cout<<"***** inserting "<<id.toUTF8()<<" into direction map"<<std::endl;
        String listname="input"+String(id.hashCode());
        if(!props_->has_list(listname))
        {
            pic::logmsg() <<"Peg addId "<<std::string(id.toUTF8());

            props_->set_list(listname,0,id);
            props_->set_list(listname,1,String(input));
            props_->set_list(listname,2,String(output));

            getMainPanel()->save_props(props_->get_string("id"));
        }
//    }

//    std::set<String>::iterator wireIter;  
//    wireIter=wireIds_.find(id);
//    if(wireIter==wireIds_.end())
//    {
        wireIds_.insert(id);
//    }
}
void Peg::doRemoveId(String id)
{
    int n=wireIds_.erase(id);
    pic::logmsg()<<"removed "<<n<<" ids from wireIds_";
    bool fg=false;
    for (std::set<String>::iterator i =wireIds_.begin();i!=wireIds_.end();i++)
    {
        Wire* w=getMainPanel()->getWireById((*i));
        if(w!=0)
        {
            if(w->isForegrounded())
            {
                fg=true;
                break;
            }
        }
    }
    if(fg==false)
    {
        setForegrounded(false,id);
    }

    directionMap_.erase(id);

}

void Peg::removeId(String id)
{
    doRemoveId(id);
    String listname="input"+String(id.hashCode());
    if (props_->has_list(listname))
    {
        pic::logmsg()<< "remove_list input"<<std::string(id.toUTF8());
        props_->remove_list(listname);
        getMainPanel()->save_props(props_->get_string("id"));
    }
}

void Peg::lasso(bool lassoed)
{
    lassoed_=lassoed;
    repaint();
}

void Peg::doSelectedGroupMove(int dx, int dy)
{
    toFront(false);
    int x=getX()+dx;
    int y=getY()+dy;
    trueX_=floor(((float)x/zoomFactor_)+0.5);
    trueY_=floor(((float)y/zoomFactor_)+0.5);
    setTopLeftPosition(x, y);
}

void Peg::doSetPositionProps()
{
    if (props_!=0)
    {
        int x=getX();
        props_->set_number("x",floor(((float)x/zoomFactor_)+0.5));
        int y =getY();
        props_->set_number("y",floor(((float)y/zoomFactor_)+0.5));
        getMainPanel()->save_props(props_->get_string("id"));
    }
}

void Peg::setSelected(bool selected, String id)
{
    selected_=selected;
    if(selected)
    {
        toFront(true);
    }
    else
    {
        if(foregrounded_)
        {
            setForegrounded(foregrounded_,id);
        }
        else
        {
            toBack();
        }
    }
    repaint();
}

void Peg::setForegrounded(bool foregrounded, String id)
{
    if(foregrounded)
    {
//        pic::logmsg()<<"peg foregrounded";
        foregrounded_=true;
    // XXX temporary just bring to front - long term need to only foreground the relevant wires
 
        setAlwaysOnTop(true);
        getMainPanel()->getDisplayLayer()->toBack();
    }
    else
    {
        foregrounded_=false;
        setAlwaysOnTop(false);
        toBehind(getMainPanel()->getDisplayLayer());
    }
}

void Peg::bringToFront()
{
   if(foregrounded_)
   {
        toFront(false);
   }
   else
   {
       toBehind(getMainPanel()->getDisplayLayer());
   }
}

void Peg::initMove()
{
    getMainPanel()->initRoutingElementMove(this);
}

bool Peg::shouldBeHidden()
{
    bool hidden;
    if(wireIds_.size()==0)
    {
        hidden=false;
    }
    else
    {
        hidden=true;
        for (std::set<String>::iterator i =wireIds_.begin();i!=wireIds_.end();i++)
        {
            Wire* w=getMainPanel()->getWireById((*i));
            if(w!=0)
            {
//                std::cout<<"Peg:paint wire="<<w->getId().toUTF8()<<" hidden="<<w->isHidden()<<std::endl;
                if(!(w->isHidden())) 
                {
                    hidden=false;
                    break;
                }
            }
            else
            {
                hidden=false;
            }
        }
    }
    return hidden;
}

void Peg::paint (Graphics& g)
{
    if(!shouldBeHidden())
    {
        Path p;
        p.addPieSegment(0.0f,0.0f,getWidth(),getHeight(),1.5f*3.14f,2.5f*3.14f,0.0);
        if(mouseOver_||lassoed_)
        {
            g.setColour(Colour (0xbba7a7a7));
            g.fillPath(p);

            g.setGradientFill (ColourGradient (Colour (0xffacd0ed),
                                               16.0f, 16.0f,
                                               Colour (0xff53a2e6),
                                               24.0f, 24.0f,
                                               false));
            if(isSticky())
            {
                g.fillRect ((float) (proportionOfWidth (0.2500f)), (float) (proportionOfHeight (0.2500f)), (float) (proportionOfWidth (0.5000f)), (float) (proportionOfHeight (0.5000f)));
            }
            else
            {
                g.fillEllipse ((float) (proportionOfWidth (0.2500f)), (float) (proportionOfHeight (0.2500f)), (float) (proportionOfWidth (0.5000f)), (float) (proportionOfHeight (0.5000f)));
            }

            g.setColour (Colour (0xff272727));
            if(isSticky())
            {
                g.drawRect ((float) (proportionOfWidth (0.2500f)), (float) (proportionOfHeight (0.2500f)), (float) (proportionOfWidth (0.5000f)), (float) (proportionOfHeight (0.5000f)), 2.0000f);
            }
            else
            {
                g.drawEllipse ((float) (proportionOfWidth (0.2500f)), (float) (proportionOfHeight (0.2500f)), (float) (proportionOfWidth (0.5000f)), (float) (proportionOfHeight (0.5000f)), 2.0000f);
            }
        }

        else
        {
            g.setColour(Colour (0xbba7a7a7));
            g.fillPath(p);
            g.setGradientFill (ColourGradient (Colour (0xffe0dede),
                                               16.0f, 16.0f,
                                               Colour (0xffa8a7a7),
                                               24.0f, 24.0f,
                                               false));
            if(isSticky())
            {
                g.fillRect ((float) (proportionOfWidth (0.2500f)), (float) (proportionOfHeight (0.2500f)), (float) (proportionOfWidth (0.5000f)), (float) (proportionOfHeight (0.5000f)));
            }
            else
            {
                g.fillEllipse ((float) (proportionOfWidth (0.2500f)), (float) (proportionOfHeight (0.2500f)), (float) (proportionOfWidth (0.5000f)), (float) (proportionOfHeight (0.5000f)));
            }

            g.setColour (Colour (0xff272727));
     
        }

    }
}

void Peg::resized()
{
}

void Peg::mouseMove (const MouseEvent& e)
{
}

void Peg::mouseEnter (const MouseEvent& e)
{
    setMouseCursor(getMainPanel()->getCursor());
    int tool =getMainPanel()->getTool();
    if(tool==ToolManager::MOVETOOL||tool==ToolManager::DELETETOOL)
    {
        setMouseOver(true);
    }

}

void Peg::mouseExit (const MouseEvent& e)
{
    int tool =getMainPanel()->getTool();
    if(tool==ToolManager::MOVETOOL||tool==ToolManager::DELETETOOL)
    {
        setMouseOver(false);
    }
}

void Peg::mouseDown (const MouseEvent& e)
{
    if(getMainPanel()->getTool()==ToolManager::MOVETOOL)
    {
        mouseDownX=getX();
        mouseDownY=getY();
        if(lassoed_)
        {
            getMainPanel()->initSelectedItemMove();
            bringToFront();
            // XXX bring all selected routing elements to front
            // also required in trunk
        }
        else
        {
            getMainPanel()->addToSelectionBasedOnModifiers(this, e.mods);
            bringToFront();
            if(!move_init_) 
            {
                getMainPanel()->clearWireMoveList();
                getMainPanel()->initRoutingElementMove(this);
                move_init_=true;
            }
        }
    }
}

void Peg::mouseDrag (const MouseEvent& e)
{
    if(getMainPanel()->getTool()==ToolManager::MOVETOOL)
    {
        if(lassoed_)
        {
            doMove(e);
        }
        else
        {
            int x=mouseDownX+e.getDistanceFromDragStartX();
            int y=mouseDownY+e.getDistanceFromDragStartY();
            x=getMainPanel()->conditionX(x,getWidth());
            y=getMainPanel()->conditionY(y,getHeight());

            trueX_=floor(((float)x/zoomFactor_)+0.5);
            trueY_=floor(((float)y/zoomFactor_)+0.5);

            setTopLeftPosition(x, y);
            if ((!getMainPanel()->offCanvas(x,y,getWidth(),getHeight())) &&(!getMainPanel()->inCentralViewPort(x,y)))
            {
                getMainPanel()->autoComponentDrag(x,y,this);
            }
            else
            {
                getMainPanel()->stopAutoDrag();
            }

            getMainPanel()->thingMoved();
        }
    }
}

void Peg::doMove(const MouseEvent& e)
{
    int x=mouseDownX+e.getDistanceFromDragStartX();
    int y=mouseDownY+e.getDistanceFromDragStartY();
    x=getMainPanel()->conditionX(x,getWidth());
    y=getMainPanel()->conditionY(y,getHeight());
    int dx=x-getX();
    int dy=y-getY();
    getMainPanel()->moveSelectedGroup(dx,dy);
    autoDragIfRequired(getX(),getY(), e.x+getX(), e.y+getY());
}

void Peg::autoDragIfRequired(int x, int y, int mx, int my)
{
    if ((!getMainPanel()->inCentralViewPort(mx,my)))
    {
        getMainPanel()->autoComponentDrag(x,y,this);
    }
    else
    {
        getMainPanel()->stopAutoDrag();
    }
}

void Peg::mouseUp (const MouseEvent& e)
{
    move_init_=false;
 	ModifierKeys m=e.mods;
	if (m.isPopupMenu())
	{
        getMainPanel()->showTools(e);
	}
    else if (e.mouseWasClicked())
    {
        doMouseClick(e);
    }
    else
    {
        if(lassoed_)
        {
            getMainPanel()->endThingMoved();
            getMainPanel()->stopAutoDrag();
            getMainPanel()->setSelectedGroupProps();
        }
        else
        {
            getMainPanel()->stopAutoDrag();
            getMainPanel()->endThingMoved();
            pic::logmsg()<<"Peg mouseup "<<props_;

            doSetPositionProps();
//            if (props_!=0)
//            {
//                int x=getX();
//                props_->set_number("x",(float)x/zoomFactor_);
//                int y =getY();
//                props_->set_number("y",(float)y/zoomFactor_);
//                getMainPanel()->save_props(props_->get_string("id"));
//            }
        }

        getMainPanel()->clearSelectedItemsIfOne();
        getMainPanel()->doToolChangeIfRequired();
        setSelected(selected_,String::empty);
    }
}

void Peg::doMouseClick(const MouseEvent& e)
{
    if(getMainPanel()->getTool()==ToolManager::ZOOMTOOL)
    {
        getMainPanel()->doMouseClick(e.getEventRelativeTo(getMainPanel()));
    }

    else if(getMainPanel()->getTool()==ToolManager::DELETETOOL)
    {
        deleteToolMouseClick(e);
    }

    else if(getMainPanel()->getTool()==ToolManager::SELECTTOOL)
    {
        selectToolMouseClick(e);
    }

    else if(getMainPanel()->getTool()==ToolManager::POINTERTOOL)
    {
        pointerToolMouseClick(e);
    }
    else if(getMainPanel()->getTool()==ToolManager::HOOKTOOL)
    {
        hookToolMouseClick(e);
    }
}

void Peg::hookToolMouseClick(const MouseEvent& e)
{
    setSticky(!isSticky());
    repaint();
    if(!move_init_) 
    {
        getMainPanel()->clearWireMoveList();
        getMainPanel()->initRoutingElementMove(this);
        move_init_=true;
    }
    move_init_=false;
    getMainPanel()->thingMoved();
    getMainPanel()->endThingMoved();
}

void Peg::deleteToolMouseClick(const MouseEvent& e)
{
    bool doDelete=true;
    if(getMainPanel()->requiresDeleteConfirmation("Peg"))
    {
        DeletePegConfirmation* da=new DeletePegConfirmation();
        DialogWindow::showModalDialog("Delete hook",da,this,Colour(0xffababab),true);
        if(da->dontShowAgain_&& da->okPressed_)
        {
           getMainPanel()->setDeleteConfirmationRequired("Peg"); 
        }

        doDelete=da->okPressed_;
        delete da;
    }

    if(doDelete)
    {
        getMainPanel()->deleteRoutingElement(this);
    }
}

void Peg::selectToolMouseClick(const MouseEvent& e)
{
    ModifierKeys m=e.mods;
    if(lassoed_ && !(m.isCommandDown()||m.isShiftDown()))
    {
        getMainPanel()->deselect(this);
    }
    else
    {
          getMainPanel()->addToSelectionBasedOnModifiers(this,m);
    }
}

void Peg::pointerToolMouseClick(const MouseEvent& e)
{
    selectToolMouseClick(e);
}

PropertyStore* Peg::getProps()
{
    return props_;
}

void Peg::setMouseOver(bool over)
{
    mouseOver_=over;
    repaint();
}

void Peg::changed(PropertyStore* props)
{
    if (props!=0)
    {   
        getMainPanel()->clearWireMoveList();
        getMainPanel()->initRoutingElementMove(this);
        props_=props;
        int s=props_->get_number("sticky");
        if(s==0)
        {
            sticky_=false;
        }
        else
        {
            sticky_=true;
        }

        String listname;
        for (int i=0;i<props_->list_count();i++)
        {
            listname=props_->get_listkey(i);

            if (listname.startsWith("input"))
            {
                String wire_id=props_->get_list(listname,0);
                String input=props_->get_list(listname,1);
                if(input.contains("999"))
                {
                    directionMap_.insert(std::pair<String, int> (wire_id,1));
                }
                wireIds_.insert(wire_id);
            }
        }

        std::vector<String> v;

        for (std::set<String>::iterator i =wireIds_.begin();i!=wireIds_.end();i++)
        {
            String listname="input"+String((*i).hashCode());
            if(!props_->has_list(listname))
            {
                v.push_back(*i); 
            }
        }

        for(std::vector<String>::iterator i=v.begin();i!=v.end();i++)
        {
            doRemoveId(*i);
        }

        trueX_=props_->get_number("x");
        trueY_=props_->get_number("y");
        int x=ceil(((float)trueX_*zoomFactor_)-0.5);
        int y=ceil(((float)trueY_*zoomFactor_)-0.5);

        setBounds(x,y,ceil((zoomFactor_*(float)trueWidth_)-0.5), ceil((zoomFactor_*(float)trueHeight_)-0.5));
        testId_=props_->get_string("id");
        getMainPanel()->thingMoved();
        getMainPanel()->endThingMoved();
    }
}

String Peg::get_id()
{
    return testId_;
}

String Peg::get_hash(String wireId)
{
    return get_id();
}

int Peg:: getXin(String id)
{
    Wire* w=getMainPanel()->getWireById(id);
    if(w!=0 && w->isRevConnect())
    {
        return getXin(id,true);
    }
    else
    {
        return getXin(id,false);
    }
}

int Peg::getXin(String id, bool revconnect)
{
    std::map<String,int>::iterator iter=directionMap_.find(id);
    if(iter!=directionMap_.end())
    {
        if(revconnect)
        {
            return getLeft();
        }
        return getRight();
    }
    else
    {
        if(revconnect)
        {
            return getRight();
        }
        return getLeft();
    }

}

int Peg::getXout(String id, bool revconnect)
{
    std::map<String,int>::iterator iter=directionMap_.find(id);
    if(iter!=directionMap_.end())
    {
        if(revconnect)
        {
            return getRight();
        }
        return getLeft();
    }
    else
    {
        if(revconnect)
        {
            return getLeft();
        }
        return getRight();
    }
}

int Peg:: getYin(String id)
{
    return trueY_+0.5f*trueHeight_;
}

int Peg::getLeft()
{
    return trueX_+0.13f*trueWidth_;
}

int Peg::getRight()
{
    return trueX_+0.87f*trueWidth_;
}

int Peg:: getXout(String id)
{
    Wire* w=getMainPanel()->getWireById(id);
    if(w!=0 && w->isRevConnect())
    {
        return getXout(id,true);
    }
    else
    {
        return getXout(id,false);
    }
}

int Peg::getYout(String id)
{
    return getYin(id);
}

Path Peg::getPathFrom(RoutingElement* other, String id)
{
    if(isSticky() && other->isSticky())
    {
        return getMainPanel()-> getLinearWirePath(other->getXout(id),other->getYout(id),getXin(id),getYin(id));
    }
    else
    {
        return getMainPanel()-> getWirePath(other->getXout(id),other->getYout(id),getXin(id),getYin(id),1.1f);
    }
}

void Peg::doAutoDrag(int dx,int dy)
{
    mouseDownX=mouseDownX+dx;
    mouseDownY=mouseDownY+dy;
    int x=getX()+dx;
    int y=getY()+dy;
    x=getMainPanel()->conditionX(x,getWidth());
    y=getMainPanel()->conditionY(y,getHeight());

    if(getMainPanel()->offCanvas(x,y,getWidth(),getHeight()))
    {
        getMainPanel()->stopAutoDrag();
    }
    else
    {
        setTopLeftPosition(x,y);
        trueX_=floor(((float)x/zoomFactor_)+0.5);
        trueY_=floor(((float)y/zoomFactor_)+0.5);
        getMainPanel()->thingMoved();
    }
}


void Peg::doZoom(float zoomFactor)
{
    zoomFactor_=zoomFactor;
    int x=ceil(((float)trueX_*zoomFactor)-0.5);
    int y=ceil(((float)trueY_*zoomFactor)-0.5);
    setBounds(x,y,ceil(((float)zoomFactor*trueWidth_)-0.5), ceil(((float)zoomFactor*trueHeight_)-0.5));
    repaint();
}

int Peg::getInputAt(int x, int y)
{
    // XXX 
    //std::cout<<"Peg: getinputAt x="<<x<<" width="<<getWidth()<<std::endl;
    if(x>(0.5*(float)getWidth()))
    {
        return 999;
    }
    //
    return -999;
}

int Peg::getOutputAt(int x, int y)
{
    return 1;

}
