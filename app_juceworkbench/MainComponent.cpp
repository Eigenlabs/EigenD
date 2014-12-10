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

#include "MainComponent.h"

void SelectedItems::itemSelected(Selectable* item)
{
    item->lasso(true);
}

void SelectedItems::itemDeselected(Selectable* item)
{
    item->lasso(false);
}

DisplayLayer::DisplayLayer(MainComponent* mc)
{
    mc_=mc;
}

void DisplayLayer::paint(Graphics& g)
{
    if(mc_->hasForegroundedItem())
    {
        //g.fillAll(Colour(0xa0ffffff));
        g.fillAll(Colour(0xc9ffffff));
    }

    if(mc_->hasForegroundWires())
    {
        mc_->drawForegroundWires(g);
    }
}

ChangeWireProperties::ChangeWireProperties(String id, String sid, String did, String u, String f, String c)
{
    id_=id;
    sid_=sid;
    did_=did;
    u_=u;
    f_=f;
    c_=c;
}

ChangeWireProperties::~ChangeWireProperties()
{
}

String ChangeWireProperties::getId()
{
    return id_;
}

String ChangeWireProperties::getSrcId()
{
    return sid_;
}

String ChangeWireProperties::getDstId()
{
    return did_;
}

String ChangeWireProperties::getUsing()
{
    return u_;
}

String ChangeWireProperties::getFilter()
{
    return f_;
}

String ChangeWireProperties::getControl()
{
    return c_;
}

StoredDropPin::StoredDropPin(Wire* w, Pin* p)
{
    p_=p;
    w_=w;
}

Wire* StoredDropPin::getWire()
{
    return w_;
}
Pin* StoredDropPin::getPin()
{
    return p_;
}

StoredRouteElement::StoredRouteElement(String id, int input, int output,bool revconnect)
{
    id_=id;
    input_=input;
    output_=output;
    revconnect_=revconnect;
    createTime_=Time::currentTimeMillis();
}

StoredRouteElement::~StoredRouteElement()
{

}

void StoredRouteElement::tick()
{
    createTime_=Time::currentTimeMillis();
}

bool StoredRouteElement::expired(int lifetime)
{
    return (Time::currentTimeMillis()-createTime_)>lifetime;
}

String StoredRouteElement::getId()
{
    return id_;
}

int StoredRouteElement::getInput()
{
    return input_;
}

int StoredRouteElement::getOutput()
{
    return output_;
}

bool StoredRouteElement::isRevConnect()
{
    return revconnect_;
}

Anchor::Anchor(int x, int y)
{
   setBounds(x,y,20, 20); 
}

DummyBox:: DummyBox(int x, int y, float zoomFactor)
{
   setBounds(x,y,BOXWIDTH*zoomFactor, 28*zoomFactor); 
   zoomFactor_=zoomFactor;
   startTimer(1000);
}

void DummyBox::paint(Graphics& g)
{
    g.setColour (Colours::black);
    g.drawRoundedRectangle(16.0f*zoomFactor_, 1, getWidth()-(32.0f*zoomFactor_), getHeight(), 14.0f*zoomFactor_,1.0f*zoomFactor_);
}

void DummyBox::timerCallback()
{
    stopTimer();
    getParentComponent()->removeChildComponent(this);
    delete this;
}

DummyInput:: DummyInput(int x, int y, float zoomFactor)
{
   setBounds(x,y,8*zoomFactor, 8*zoomFactor); 
}

void DummyInput::paint(Graphics& g)
{
    g.setColour(Colour(selectedWireColour));
    g.fillEllipse (0.0f, 0.0f, getWidth(), getHeight());
}

WBScrollListener::WBScrollListener(Viewport* comp)
{
    comp_=comp;
}

void WBScrollListener::scrollBarMoved(ScrollBar* scrollBarThatHasMoved, double newRangeStart)
{
    if(!isTimerRunning())
    {
        startTimer(100);
    }
}

void WBScrollListener::timerCallback()
{
    if(!Component::isMouseButtonDownAnywhere())
    {
        stopTimer();
        Component* c=comp_->getViewedComponent();
        if(c!=0)
        {
            c->postCommandMessage(1099);
        }
    }
}

WBViewport::WBViewport(const String &name):Viewport(name)
{
    hbarListener_=new WBScrollListener(this);
    vbarListener_=new WBScrollListener(this);
    ScrollBar* hbar=getHorizontalScrollBar();
    hbar->addListener(hbarListener_);
    getHorizontalScrollBar()->addListener(hbarListener_);
    getVerticalScrollBar()->addListener(vbarListener_);
}
void WBViewport::parentSizeChanged()
{
    setBounds(0,0,getParentWidth(),getParentHeight());
}

MainComponent::MainComponent(ToolManager* tm, Workspace* model, MenuManager* mm, HelpViewComponent* hv, WorkspaceFactory* wsf)
        
{
    selectedWire_=0;
    dragPinReady_=false;
    dragSrcPinReady_=false;
    wireDragStarted_=false;
    dragPin_=0;
    dragSrcPin_=0;
    eventOriginPin_=0;
    zoomFactor_=1.0f;
    zoomTot_=0.0f;
    trueWidth_=12000;
    trueHeight_=8000;
    nx_=100;
    ny_=50;
    setName("MainComponent");
    setSize (trueWidth_*zoomFactor_, trueHeight_*zoomFactor_);

    // XXX
    //showOutlines_=true;
    showOutlines_=false;
    mouseMode_=0;
    autoDragMode_=false;
    autoDragComp_=0;
    vp_=0;
    tm_=tm;
    highlight_=0;
    loosePinCount_=0;
    model_=model;
    mm_=mm;
    hv_=hv;
    wsf_=wsf;
    routeThroughBack_=true;
    routeThroughFront_=true;
    currentRoutingElement_=0;
    unhookPeg_=0;
    dummyTrunk_=0;
    dragInput_=0;
    currentTrunk_=0;
    currentInput_=0;
    tooltip_=String();
    dx_=0;
    dy_=0;
    createDialogX_=0;
    createDialogY_=0;
    currentCreateAgent_=String();
    showMetronomeOutputs(getProperty("showMetronomeOutputs",true),false);
    showControllerOutputs(getProperty("showControllerOutputs",true),false);

    paintMode_=FULL;
    paintOverMode_=FULL;
    paintForegroundMode_=FULL;
    overElement_=0;
    instanceParent_=0;
    instanceName_=String::empty;
    dc_=0;
    animated_=false;
    wireIndex_=0;
    //lassoComponent_=new LassoComponent<Selectable*> (2);
    lassoComponent_=new LassoComponent<Selectable*> ();
    addAndMakeVisible(lassoComponent_);

    displayLayer_=new DisplayLayer(this);
    displayLayer_->setBounds(0,0,getWidth(),getHeight());
    displayLayer_->setAlwaysOnTop(true);
    displayLayer_->setInterceptsMouseClicks(false,false);
    spaceKeyDown_=false;
    storedTool_=0;

    candidateWiringBox_=0;
    candidateMovable_=0;
    props_=0;
    toolChangeRequired_=false;
    moveToolChangeRequired_=false;
    mouseWheeling_=false;
    overrideDraft_=false;
    foregroundWireOverrideDraft_=false;
    pathQuality_=1;
    addAndMakeVisible(displayLayer_);
}


MainComponent::~MainComponent()
{

}

Workspace* MainComponent::getWorkspace()
{
    return model_;
}

void MainComponent::stopMonitor(Atom* atom)
{
    //std::cout<<"stopMonitor "<<atom<<std::endl;
    //std::cout<<"   model_="<<model_<<std::endl;
    //std::cout<<"   valueMonitor="<<model_->getValueMonitor()<<std::endl;
    model_->getValueMonitor()->printInfo();
    model_->getValueMonitor()->remove(atom);
}

void MainComponent::setViewport(WBViewport* vp)
{
    vp_=vp;
}

bool MainComponent::requiresDeleteConfirmation(String t)
{
    return !tm_->getPropertyValue(t+String("Delete"),false);
}

void MainComponent::setDeleteConfirmationRequired(String t)
{
    tm_->setPropertyValue(t+String("Delete"),true);
}

bool MainComponent::getProperty(String propertyName,bool defaultSetting)
{
    return tm_->getPropertyValue(propertyName,defaultSetting);
}

void MainComponent::setProperty(String propertyName,bool setting)
{
    return tm_->setPropertyValue(propertyName,setting);
}

bool MainComponent::showOutlines()
{
    return showOutlines_;
}
void MainComponent::showOutlines(bool val)
{
    showOutlines_=val;
    repaint();
}

void MainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
    pic::logmsg()<<"changeListenerCallback";
    animated_=!animated_;
    if(!animated_)
    {
        endThingMoved();
    }
}

void MainComponent::moveObscuredBoxes(Box* b)
{
    pic::logmsg()<<"move boxes obscured by "<<std::string(b->getId().toUTF8());
    // XXX
    juce::Rectangle <int> r=b->getBoundsInParent();

    for(int i =selectedItems_.getNumSelected();--i>=0;)
    {
        Box* b2=dynamic_cast<Box*>(selectedItems_.getSelectedItem(i));
        //if(b2 !=0 && b2!=b && b2->isExpanded())
        if(b2 !=0 && b2!=b)
        {
            if(r.intersects(b2->getBoundsInParent()))    
            {
                pic::logmsg()<<"Box "<<std::string(b2->getId().toUTF8())<<" obscured by box "<< std::string(b->getId().toUTF8());
                // record id and current position
                juce::Rectangle <int> r2=b2->getBoundsInParent();
                b->addObscuredBox(b2->getId());
                b2->cachePosition(b->getId());
                b2->initBoxMove();
//                ComponentAnimator& ca=Desktop::getInstance().getAnimator();
//                ca.addChangeListener(this);
//                //ca.animateComponent(b2,r2.translated(b2->getWidth(),0),1.0f,500,false,1.0,1.0);
//                ca.animateComponent(b2,r2.translated(b2->getWidth(),0),1.0f,500,false,0.0,0.0);
                b2->setAutoPositioned(true);
                //b2->setBounds(r2.translated(1.2*b2->getWidth(),0));

                //b2->setBounds(r.translated(1.2*b->getWidth(),0));
                b2->setTopLeftPosition(b->getX()+(1.2*b->getWidth()),b2->getY());
                b2->setTrueBounds();
                endThingMoved();
            }
        }
    }
}

void MainComponent::returnObscuredBoxes(Box* b)
{
    // XXX
    pic::logmsg()<<"return boxes no longer obscured by "<<std::string(b->getId().toUTF8());
    std::set<String> ids=b->getObscuredBoxIds();
    for(std::set<String>::iterator i=ids.begin();i!=ids.end();i++)
    {
        String id=*i;
        if(id.isNotEmpty())
        {
            Box* b1 =getBoxById(id);
            if(b1!=0)
            {
                if(b1->isAutoPositioned())
                {
                    b1->returnToManualPosition(b->getId());
    //                int originalX=b1->getObscuredBoxTrueX();
    //                int originalY=b1->getObscuredBoxTrueY();
    //    //            juce::Rectangle <int> originalPos=b->getObscuredBoxPos();
    //                // if no longer obscured move them back to saved position
    //        //        juce::Rectangle <int> r2=b2->getBoundsInParent();
    //                // what if has been zoomed in the meantime
    //                // moved?
    //                //deleted?
    //                b1->initBoxMove();
    //    //            ComponentAnimator& ca=Desktop::getInstance().getAnimator();
    //    //            //ca.animateComponent(b1,originalPos,1.0f,500,false,1.0,1.0);
    //    //            ca.animateComponent(b1,originalPos,1.0f,500,false,0.0,0.0);
    //    //            b1->setBounds(originalPos);
    //                b1->setTopLeftPosition(originalX*zoomFactor_,originalY*zoomFactor_);
    //                b1->setTrueBounds();
    //                endThingMoved();
    //                // need to call thingMoved() while moving
                }
            }
        }
    }
    b->clearObscuredBoxIds();
}

bool MainComponent::hasForegroundedItem()
{
    for (std::map<String,Box*>::iterator iter=boxes_.begin();iter!=boxes_.end();iter++)
    {
        if((iter->second)->isForegrounded())
        {
            return true;
        }
    }
    return false;
}

int MainComponent::getGridNumber(int x, int y)
{
    int gx=ceil(nx_*((float)x/(float)trueWidth_));
    int gy=ceil(ny_*((float)y/(float)trueHeight_));
    return (gy-1)*nx_+gx;
}

void MainComponent::clearGridMap(Wire* w)
{
    for (std::map<int,std::list<Wire*> >::iterator iter=gridSquareMap_.begin();iter!=gridSquareMap_.end();iter++)
    {
        (iter->second).remove(w);
    }
}

void MainComponent::getTemporaryPosition(int& x, int & y)
{
    x=0;
    y=0;
    bool overlapping=true;
    while(overlapping)
    {
        y=y+42*zoomFactor_;
        if(y>getHeight())
        {
            y=42*zoomFactor_;
            x=x+(1.25*BOXWIDTH*zoomFactor_);
        }
//        std::cout<<"getTemporaryPosition: x="<<x<<" y="<<y<<" w="<<BOXWIDTH*zoomFactor_<<" h="<<28*zoomFactor_<<std::endl;

        juce::Rectangle<int> r=juce::Rectangle<int>(x,y,BOXWIDTH*zoomFactor_,28*zoomFactor_);
        bool intersects=false;
        for (int i =getNumChildComponents();--i>=0;)
        {
            Box* const bb =dynamic_cast <Box *>(getChildComponent (i));
            if (bb!=0)
            {
                juce::Rectangle<int> br=bb->getBounds();
//                std::cout<<"Box rect x="<<br.getX()<<" y="<<br.getY()<<" w="<<br.getWidth()<<" h="<<br.getHeight()<<std::endl;
                if(br.intersects(r))
                {
                   intersects=true; 
                   break;
                }
            }
        }

        if (!intersects)
        {
            overlapping=false;
        }
    }

    x=(float)x/zoomFactor_;
    y=(float)y/zoomFactor_;

}

void MainComponent::updateGridMap(int gs,Wire* w)
{
    std::map<int,std::list<Wire*> >::iterator i=gridSquareMap_.find(gs);
    if(i!=gridSquareMap_.end())
    {
        (i->second).push_back(w);
    }
    else
    {
        std::list<Wire*> wires;
        wires.push_back(w);
        gridSquareMap_.insert(std::pair<int,std::list<Wire*> >(gs,wires)); 
    }
}

Component* MainComponent::getDisplayLayer()
{
    return displayLayer_;
}

SelectedItemSet<Selectable*>& MainComponent:: getLassoSelection()
{
    return selectedItems_;
}

void MainComponent::initSelectedItemMove()
{
    clearWireMoveList();
    for(int i =selectedItems_.getNumSelected();--i>=0;)
    {
        selectedItems_.getSelectedItem(i)->initMove();
    }
}

void MainComponent::moveSelectedGroup(int dx, int dy)
{
    for(int i =selectedItems_.getNumSelected();--i>=0;)
    {
        selectedItems_.getSelectedItem(i)->doSelectedGroupMove(dx,dy);
    }
    thingMoved();
}

void MainComponent::setSelectedGroupProps()
{
    pic::logmsg()<<"setSelectedGroupProps";
    for(int i =selectedItems_.getNumSelected();--i>=0;)
    {
        selectedItems_.getSelectedItem(i)->doSetPositionProps();
    }
}

void MainComponent::clearSelectedItems()
{
    selectedItems_.deselectAll();
}

void MainComponent::clearSelectedItemsIfOne()
{
    if(selectedItems_.getNumSelected()==1)
    {
        selectedItems_.deselectAll();
    }
}

void MainComponent::selectOnly(Selectable* b)
{
    selectedItems_.selectOnly(b);
}

void MainComponent::addToSelection(Selectable* b)
{
    selectedItems_.addToSelection(b);
}

bool MainComponent::isSelected(Selectable* b)
{
    return selectedItems_.isSelected(b);
}

void MainComponent::addToSelectionBasedOnModifiers(Selectable* b,const ModifierKeys& mods)
{
    selectedItems_.addToSelectionBasedOnModifiers(b,mods);
}

void MainComponent::clearCandidateWiringBox(Box* b)
{
    if(candidateWiringBox_!=0)
    {
        if(b==candidateWiringBox_)
        {
            candidateWiringBox_=0;
        }
    }
}


void MainComponent::findLassoItemsInArea(Array<Selectable*>&itemsFound,const juce::Rectangle<int>& area)
{
    for (std::map<String,Box*>::iterator iter=boxes_.begin();iter!=boxes_.end();iter++)
    {
        if((iter->second)->isTopLevel())
        {
            if(area.intersects((iter->second)->getBounds())) 
            {
                itemsFound.addIfNotAlreadyThere(iter->second);
                selectedItems_.addToSelection(iter->second);
            }
            else
            {
                selectedItems_.deselect(iter->second);
            }
        }
    }

    for (std::map<String,Peg*>::iterator iter=pegs_.begin();iter!=pegs_.end();iter++)
    {
        Peg* p=iter->second;
        if (p!=0)
        {
            if(area.intersects((iter->second)->getBounds())) 
            {
                itemsFound.addIfNotAlreadyThere(iter->second);
                selectedItems_.addToSelection(iter->second);
            }
            else
            {
                selectedItems_.deselect(iter->second);
            }
        }
    }

    bool assemblyAlreadyThere;
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        assemblyAlreadyThere=false;
        Trunk* p=iter->second;
        if (p!=0)
        {
            String assemblyId=p->getAssemblyId();
            for(int i =selectedItems_.getNumSelected();--i>=0;)
            {
                Trunk* t=dynamic_cast<Trunk*>(selectedItems_.getSelectedItem(i));
                if(t!=0)
                {
                    if((t!=p)&&(t->getAssemblyId()==assemblyId)) 
                    {
                        pic::logmsg()<<"assemblyId already in the selectedItems";
                        assemblyAlreadyThere=true;
                        break;
                    }
                }
            }

            if(area.intersects((iter->second)->getBounds())) 
            {
                if(!assemblyAlreadyThere)
                {
                    itemsFound.addIfNotAlreadyThere(iter->second);
                    selectedItems_.addToSelection(iter->second);
                }
            }
            else
            {
                selectedItems_.deselect(iter->second);
            }
        }
    }
}


void MainComponent::handleCommandMessage(int commandId)
{
    pic::logmsg()<<"handleCommandMessage "<<commandId;
    if (commandId==1003)
    {
        showMetronomeOutputs(false,true);
        repaint();
    }
    else if (commandId==1004)
    {
        showMetronomeOutputs(true,true);
        repaint();
    }
    else if (commandId==1005)
    {
        showControllerOutputs(false,true);
        repaint();
    }
    else if (commandId==1006)
    {
        showControllerOutputs(true,true);
        repaint();
    }

    else if (commandId==1011)
    {
        showFindDialog();
    }
    else if(commandId==1012)
    {
        wsf_->removeCurrentTab();
    }
    else if (commandId==1021)
    {
        showPreferencesDialog();
    }
    else if (commandId==1099)
    {
        setWorkspaceProps();
    }
    else if(commandId>2000)
    {
        float zf=(float)(commandId-2100)/100.0f;
        int x=vp_->getViewPositionX()+0.5*vp_->getMaximumVisibleWidth();
        int y=vp_->getViewPositionY()+0.5*vp_->getMaximumVisibleHeight();
        doZoom(zf,x,y);
        setWorkspaceProps();
    }
}

bool MainComponent::hideControllerOutputs()
{
    return hideControllerOutputs_;
}

bool MainComponent::hideMetronomeOutputs()
{
    for(int i =selectedItems_.getNumSelected();--i>=0;)
    {
        Box* b =dynamic_cast<Box*>(selectedItems_.getSelectedItem(i));
        if(b!=0)
        {
            Atom* a =b->getAtom();
            if(a!=0)
            {
                if(a->has_protocol("metronome"))
                {

                return false;
                }
            }
        }

    }
    return hideMetronomeOutputs_;
}

void MainComponent::showHelpText(String help)
{
    hv_->setHelpDescription(help);
}

String MainComponent::getTooltip()
{
    return tooltip_; 
}

void MainComponent::addForegroundWire(Wire* w)
{
    foregroundWires_.insert(std::pair<String,Wire*>(w->getId(),w));
}

bool MainComponent::hasForegroundWires()
{
    return foregroundWires_.size()>0;
}

void MainComponent::removeForegroundWire(Wire* w)
{
    foregroundWires_.erase(w->getId());
}

void MainComponent::removeLooseWire(Wire* w)
{
    if (w->isLoose())
    {
        int n =looseWires_.erase(w->getId());
        pic::logmsg() <<"erased "<<n<<" wires from looseWires";
    }
}

void MainComponent::removePendingWire(Wire* w)
{
    int n =pendingWires_.erase(w->getId());
    if(n==1)
    {
        w->setPending(false);
    }
    pic::logmsg() <<"erased "<<n<<" wires from pendingWires_";
}

void MainComponent::initSourcePinMove(String id)
{
    wireMoveList_.clear();
    for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
    {
        Wire* w=iter->second;

        if ((w->getId().startsWith(id+":"))||(w->getId().endsWith(":"+id)))
        {
            addToWireMoveList(w);
        }
    }
}

void MainComponent::initDestinationPinMove(String id)
{
    wireMoveList_.clear();
    for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
    {
        Wire* w=iter->second;

        if ((w->getId().startsWith(id+":"))||(w->getId().endsWith(":"+id)))
        {
            addToWireMoveList(w);
        }
    }
}

void MainComponent::removeRouteFromTrunkAssembly(String routeId, String assemblyId)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* tt=iter->second;

        if (tt!=0)
        {
            if(tt->getProps()->get_string("assemblyId")==assemblyId)
            {
                removeRouteFromRoutingElement(routeId,tt);
            }
        }
    }


}


void MainComponent:: initAssemblyMove(String assemblyId)
{
    std::vector<RoutingElement*> routingElements;

    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* tt=iter->second;

        if (tt!=0)
        {
            if(tt->getProps()->get_string("assemblyId")==assemblyId)
            {
                routingElements.push_back(tt);
            }
        }
    }

    wireMoveList_.clear();
    pic::logmsg()<<"Wires affected by move:";
    for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
    {
        Wire* w=iter->second;
        for(std::vector<RoutingElement*>::iterator i=routingElements.begin();i!=routingElements.end();i++)
        {
            if (w->routedVia(*i))
            {
                pic::logmsg()<<"    "<<std::string(w->getId().toUTF8());
                addToWireMoveList(w);
            }
        }
    }

}


void MainComponent::initRoutingElementMove(RoutingElement* r)
{
    std::vector<RoutingElement*> routingElements;
    Trunk* t=dynamic_cast<Trunk*>(r);
    if(t!=0)
    {
        String assemblyId=t->getProps()->get_string("assemblyId");
        routingElements.push_back(t);

        for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
        {
            Trunk* tt=iter->second;

            if (tt!=0)
            {
                if(tt->getProps()->get_string("assemblyId")==assemblyId)
                {
                    routingElements.push_back(tt);
                }
            }
        }
    }
    else
    {
        routingElements.push_back(r);
    }

//    wireMoveList_.clear();
    pic::logmsg()<<"Wires affected by move:";
    for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
    {
        Wire* w=iter->second;
        for(std::vector<RoutingElement*>::iterator i=routingElements.begin();i!=routingElements.end();i++)
        {
            if (w->routedVia(*i))
            {
                pic::logmsg()<<"    "<<std::string(w->getId().toUTF8());
                addToWireMoveList(w);
            }
        }
    }
}

void MainComponent::initTrunkInputMove(String id)
{
    pic::logmsg()<<"initTrunkInputMove: wires affected= "<<std::string(id.toUTF8());
    wireMoveList_.clear();
    Wire* w=getWireById(id);
    if(w!=0)
    {
        int routehash=w->getRouteHash();
        int count=1;
        for(std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
        {
            if((iter->second)->getRouteHash()==routehash)
            {
                addToWireMoveList(iter->second);
                pic::logmsg()<<"    "<<count<<std::string((iter->second)->getId().toUTF8());
                count++;
            }
        }
    }
}

void MainComponent::clearWireMoveList()
{
    wireMoveList_.clear();
}

void MainComponent::addToWireMoveList(Wire* w)
{
    wireMoveList_.insert(w);
    //pic::logmsg()<<"addToWireMoveList "<<std::string(w->getId().toUTF8());
}

void MainComponent::setDraftMode(bool shouldBeDraft)
{
    if(shouldBeDraft)
    {
        pathQuality_=2;
    }
    else
    {
        pathQuality_=1;
    }
}

void MainComponent::thingChanged()
{
    stopTimer(5);
    if(!wireMoveList_.empty())
    {
        paintOverMode_=QUICK;
    }

    juce::Rectangle <float> u;
    juce::Rectangle <float> overall=juce::Rectangle<float>();

    for(std::set<Wire*>::iterator i=wireMoveList_.begin();i!=wireMoveList_.end();i++)
    {
        Wire* wire=*i;
        wire->routeChanged(false,false);
        wire->preDraw();
        wire->getPathChangeBounds(u);
        if(overall.isEmpty())
        {
            overall=u;
        }
        else
        {
            overall=overall.getUnion(u);
        }
    }

    if(!wireMoveList_.empty())
    {
        repaint((overall.getSmallestIntegerContainer()).expanded(50,50));
        startTimer(5,100);
    }
}

void MainComponent::thingChangedFullDraw()
{
    if(!wireMoveList_.empty())
    {
        paintMode_=FULL;
        paintOverMode_=FULL;
    }

    juce::Rectangle <float> u;
    juce::Rectangle <float> overall=juce::Rectangle<float>();

    for(std::set<Wire*>::iterator i=wireMoveList_.begin();i!=wireMoveList_.end();i++)
    {
        Wire* wire=*i;
        wire->routeChanged(false,false);
        wire->preDraw();
        wire->getPathChangeBounds(u);
        if(overall.isEmpty())
        {
            overall=u;
        }
        else
        {
            overall=overall.getUnion(u);
        }
    }

    if(!wireMoveList_.empty())
    {
        repaint((overall.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
    }

}

void MainComponent::endThingChanged()
{
    if(!wireMoveList_.empty())
    {
        paintMode_=FULL;
        paintOverMode_=FULL;
        paintForegroundMode_=FULL;
    }

    for(std::set<Wire*>::iterator i=wireMoveList_.begin();i!=wireMoveList_.end();i++)
    {
        Wire* wire=*i;
        wire->routeChanged(false,false);
        wire->preDraw();
    }
    
    if(!wireMoveList_.empty())
    {
        wireMoveList_.clear();
        repaint();
    }
}


void MainComponent::thingMoved()
{
    stopTimer(3);
    if(!wireMoveList_.empty())
    {
        paintOverMode_=QUICK;
    }

    juce::Rectangle <float> u;
    juce::Rectangle <float> overall=juce::Rectangle<float>();

    for(std::set<Wire*>::iterator i=wireMoveList_.begin();i!=wireMoveList_.end();i++)
    {
        Wire* wire=*i;
        wire->pathChanged();
        wire->preDraw();
        wire->getPathChangeBounds(u);
        if(overall.isEmpty())
        {
            overall=u;
        }
        else
        {
            overall=overall.getUnion(u);
        }
    }

    if(!wireMoveList_.empty())
    {
        repaint((overall.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
        startTimer(3,100);
    }
}

void MainComponent::thingMovedFullDraw()
{
    if(!wireMoveList_.empty())
    {
        paintMode_=FULL;
        paintOverMode_=FULL;
    }

    juce::Rectangle <float> u;
    juce::Rectangle <float> overall=juce::Rectangle<float>();

    for(std::set<Wire*>::iterator i=wireMoveList_.begin();i!=wireMoveList_.end();i++)
    {
        Wire* wire=*i;
        wire->pathChanged();
        wire->preDraw();
        wire->getPathChangeBounds(u);
        if(overall.isEmpty())
        {
            overall=u;
        }
        else
        {
            overall=overall.getUnion(u);
        }
    }

    if(!wireMoveList_.empty())
    {
        repaint((overall.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
    }
}

void MainComponent::endThingMoved()
{
    if(!wireMoveList_.empty())
    {
        paintMode_=FULL;
        paintOverMode_=FULL;
        paintForegroundMode_=FULL;
    }

    for(std::set<Wire*>::iterator i=wireMoveList_.begin();i!=wireMoveList_.end();i++)
    {
        Wire* wire=*i;
        wire->pathChanged();
        // XXX ensure hook is set to zero
        // temporary fix for bug 807
        wire->removeHook();

        wire->preDraw();
    }
    
    if(!wireMoveList_.empty())
    {
        wireMoveList_.clear();
        repaint();
    }
}

bool MainComponent::keyStateChanged(bool isKeyDown)
{
    KeyPress spaceKey=KeyPress(KeyPress::spaceKey);
    if(!isKeyDown)
    {
        if(spaceKeyDown_&& !spaceKey.isCurrentlyDown())
        {
            pic::logmsg()<<"space released";
            if(storedTool_!=0)
            {
                if(tm_->getTool()==ToolManager::MOVETOOL)
                {
                    if(isMouseButtonDownAnywhere())
                    {
                        pic::logmsg()<<"MoveTool and mouse button is down";
                        toolChangeRequired_=true;
                    }
                    else
                    {
                        tm_->setTool(storedTool_, toolChangeComponent());
                        storedTool_=0;
                    }
                }
            }
            spaceKeyDown_=false;
        }
    }
    return false;
}


bool MainComponent::keyPressed(const KeyPress &key)
{
//    pic::logmsg()<<"MainComponent keyPressed "<<key.getKeyCode()<<" "<<this;
    if(key.getKeyCode()==KeyPress::spaceKey)
    {
        if(!spaceKeyDown_)
        {
            if(isMouseButtonDownAnywhere())
            {
                moveToolChangeRequired_=true;
            }
            else
            {
                storedTool_=tm_->getTool();
                tm_->setTool(ToolManager::MOVETOOL,toolChangeComponent());
            }
            spaceKeyDown_=true;
        }
        return true;
    }

    if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==45||key.getKeyCode()==KeyPress::numberPadSubtract))
    {
        pic::logmsg()<<"Zoom out";
        int x = vp_->getViewPositionX()+ 0.5*vp_->getViewWidth();
        int y = vp_->getViewPositionY()+0.5* vp_->getViewHeight();
        doZoomInc(-0.1f,x,y,true);
        return true;
    }
    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==43||key.getKeyCode()==KeyPress::numberPadAdd))
    {
        pic::logmsg()<<"zoom in";
        int x = vp_->getViewPositionX()+ 0.5*vp_->getViewWidth();
        int y = vp_->getViewPositionY()+0.5* vp_->getViewHeight();
        doZoomInc(0.1f,x,y,true);
        return true;
    }

    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==70||key.getKeyCode()==102))
    {
        pic::logmsg()<<"Find";
        showFindDialog();
        return true;
    }
    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==69||key.getKeyCode()==101))
    {
        //cmd-e
        if(isMouseButtonDownAnywhere())
        {
            storedTool_=ToolManager::EDITTOOL;
            toolChangeRequired_=true;
        }
        else
        {
            tm_->setTool(ToolManager::EDITTOOL, toolChangeComponent());
        }
        return true;
    }
    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==87||key.getKeyCode()==119))
    {
        //cmd-w
        if(isMouseButtonDownAnywhere())
        {
            storedTool_=ToolManager::WIRINGTOOL;
            toolChangeRequired_=true;
        }
        else
        {
            storedTool_=0;
            tm_->setTool(ToolManager::WIRINGTOOL, toolChangeComponent());
        }
        return true;
    }
    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==68||key.getKeyCode()==100))
    {
        //cmd-d
        if(isMouseButtonDownAnywhere())
        {
            storedTool_=ToolManager::DELETETOOL;
            toolChangeRequired_=true;
        }
        else
        {
            storedTool_=0;
            tm_->setTool(ToolManager::DELETETOOL, toolChangeComponent());
        }
        return true;
    }
    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==65||key.getKeyCode()==97))
    {
        //cmd-a
        if(isMouseButtonDownAnywhere())
        {
            storedTool_=ToolManager::CREATETOOL;
            toolChangeRequired_=true;
        }
        else
        {
            storedTool_=0;
            tm_->setTool(ToolManager::CREATETOOL, toolChangeComponent());
        }
        return true;
    }
    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==82||key.getKeyCode()==114))
    {
        //cmd-r
        if(isMouseButtonDownAnywhere())
        {
            storedTool_=ToolManager::POINTERTOOL;
            toolChangeRequired_=true;
        }
        else
        {
            storedTool_=0;
            tm_->setTool(ToolManager::POINTERTOOL, toolChangeComponent());
        }

        return true;
    }

    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==76||key.getKeyCode()==108))
    {
        //cmd-l
        doLeftAlign();
        return true;
    }

    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==84||key.getKeyCode()==116))
    {
        //cmd-t
        doTopAlign();
        return true;
    }

    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==62))
    {
        //cmd->
        findUpstream();
        return true;
    }

    else if(key.getModifiers().isCommandDown()&&(key.getKeyCode()==60))
    {
        //cmd-<
        findDownstream();
        return true;
    }
   
    return false;
}

void MainComponent::doTopAlign()
{
    if(selectedItems_.getNumSelected()>0)
    {
        int minX =100000;
        int newY =-1;
        for(int i =selectedItems_.getNumSelected();--i>=0;)
        {
            Box* b=dynamic_cast<Box*>(selectedItems_.getSelectedItem(i));
            if(b!=0)
            {
                if(b->getX()<minX)
                {
                    minX=b->getX();
                    newY=b->getY();
                }
            }
        }

        if(newY!=-1)
        {
            for(int i =selectedItems_.getNumSelected();--i>=0;)
            {
                Box* b=dynamic_cast<Box*>(selectedItems_.getSelectedItem(i));
                if(b!=0)
                {
                    b->initBoxMove();
                    b->setScreenPos(b->getX(), newY);
                }
            }

            endThingMoved();
            repaint();
        }
    }
}


void MainComponent::doLeftAlign()
{
    if(selectedItems_.getNumSelected()>0)
    {
        int minY =100000;
        int newX =-1;
        for(int i =selectedItems_.getNumSelected();--i>=0;)
        {
            Box* b=dynamic_cast<Box*>(selectedItems_.getSelectedItem(i));
            if(b!=0)
            {
                if(b->getY()<minY)
                {
                    minY=b->getY();
                    newX=b->getX();
                }
            }
        }

        if(newX!=-1)
        {
            for(int i =selectedItems_.getNumSelected();--i>=0;)
            {
                Box* b=dynamic_cast<Box*>(selectedItems_.getSelectedItem(i));
                if(b!=0)
                {
                    b->initBoxMove();
                    b->setScreenPos(newX, b->getY());
                }
            }
            endThingMoved();
        }
    }
}

int MainComponent::conditionX(int x, int w)
{
    if(x<0)
    {
        x=0;
        if(vp_->getViewPositionX()==0)
        {
        pic::logmsg()<<"conditionX calling stopAutoDrag  viewpositionx="<<vp_->getViewPositionX();
            stopAutoDrag();
        }
    }
    else if(x>(getWidth()-w))
    {
        x=getWidth()-w;    
        if(vp_->getViewPositionX()>=getWidth()-vp_->getMaximumVisibleWidth())
        {
            pic::logmsg()<<"conditionX calling stopAutoDrag vpositionx>=getWidth-maxVisibleWidth";
            stopAutoDrag();
        }
    }
    return x;  
}

int MainComponent::conditionY(int y,int h)
{
    if(y<0)
    {
        y=0;
        if(vp_->getViewPositionY()==0)
        {
            pic::logmsg()<<"conditionY calling stopAutoDrag viewpositiony="<<vp_->getViewPositionY();
            stopAutoDrag();
        }
    }
    else if(y>(getHeight()-h))
    {
        y=getHeight()-h;    
        if(vp_->getViewPositionY()>=getHeight()-vp_->getMaximumVisibleHeight())
        {
            pic::logmsg()<<"conditionY calling stopAutoDrag vpositiony>=getHeight-maxVisibleHeight";
            stopAutoDrag();
        }
    }
    return y;  
}

std::vector<String> MainComponent::getTopLevelBoxNames()
{
    std::vector<String> boxNames;
    for (std::map<String,Box*>::iterator iter=boxes_.begin();iter!=boxes_.end();iter++)
    {
        if((iter->second)->isTopLevel())
        {
            boxNames.push_back((iter->second)->getName());
        }
    }
    std::sort(boxNames.begin(),boxNames.end(),sortNames);
    return boxNames; 
}

Path MainComponent::getWirePath(int x1,int y1,int x2, int y2, float lengthFactor)
{
    return getCatenaryPath(x1,y1,x2,y2,lengthFactor,getHeight(),2.0f,pathQuality_);
}

Path MainComponent::getLinearWirePath(int x1,int y1,int x2, int y2)
{
    return getLinearPath(x1,y1,x2,y2,getHeight(),2.0f,pathQuality_);
}

void MainComponent::checkPendingWires()
{
    checkPendingWires(String::empty);
}


void MainComponent::checkPendingWires(String aid)
{
    String checkId=String::empty;
    if(aid.isNotEmpty())
    {
        checkId=aid.upToFirstOccurrenceOf("#",false,true);
    }

    juce::Rectangle <float> u;
    juce::Rectangle <float> overall=juce::Rectangle<float>();
    for(std::map<String,Wire*>::iterator i=pendingWires_.begin();i!=pendingWires_.end();)
    {
        if((i->first).contains(checkId))
        {
        //    std::cout<<"pendingwire id="<<(i->first).toUTF8()<<"  checkId="<<checkId.toUTF8()<<std::endl;
            Wire* w=i->second;
            w->preAdd();
            int h=w->getRouteHash();
            if(h!=0)
            {
                //pic::logmsg()<<"checkPendingWires: hash="<<h;
                if(w->shouldNotBeLoaded())
                {
                    //pic::logmsg()<<"    hidden wire - dont draw it - remove from wires_ "<<w->getId().toUTF8();
                    //int n =wires_.erase(w->getId());
                    wires_.erase(w->getId());
                    //pic::logmsg()<<n<<" wires erased from wires_ ";
                    pendingWires_.erase(i++);
                    w->setPending(false);
                    removeCachedWire(w);
                    w->removeFromPins();
                    delete w;
                }
                else
                {
                    if(w->isLoose())
                    {
                        looseWires_.insert(std::pair<String,Wire*>(w->getId(),w));
                        cacheWire(w);
                        foregroundWire(w);
                    }   
                    else
                    {
                        cacheWire(w);
                        foregroundWire(w);
                        std::map<int,int>::iterator iter=routes_.find(h);
                        if (iter!=routes_.end())
                        {
                            int c=iter->second;
                            routes_.erase(h);
                            routes_.insert(std::pair<int,int>(h,c+1));
                            //pic::logmsg()<<"  duplicate route:"<<w->getId().toUTF8()<<"  c="<<c+1;
                        }
                        else
                        {
                           routes_.insert(std::pair<int,int>(h,1)); 
                           normalWires_.insert(std::pair<String,Wire*>(w->getId(),w));
                           //pic::logmsg()<<"    unique route "<<w->getId().toUTF8();
                        }
                    }

                    pendingWires_.erase(i++);
                    w->setPending(false);
                    w->getPathBounds(u);
                    if(overall.isEmpty())
                    {
                        overall=u;
                    }
                    else
                    {
                        overall=overall.getUnion(u);
                    }
                }
            }
            else
            {
                if (w->getUsingDP()!=0 || w->getUsingSP()!=0)
                {
                    cacheWire(w);     
                }

                ++i;
            }
        }
        else
        {
            ++i;
        }
    }
    if(!(overall.isEmpty()))
    {
        repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
    }

    pic::logmsg()<<"Number of pending wires "<<pendingWires_.size();
}


void MainComponent::paint(Graphics& g)
{
    // Keep this as fast as possible
//    int t1=Time::currentTimeMillis();
    bool draft=false;
    if((paintMode_==QUICK) || (((mouseMode_==DRAGZOOM)||mouseWheeling_) && (!overrideDraft_)))
    {
        draft=true;
    }
    // display hooked area path for testing
    //g.fillPath(testPath_);
    g.fillAll(Colour(0xffffffff));

    juce::Rectangle <float> u;
    std::map<int,Segment*> segmentsToDraw;

    for(std::map<String,Wire*>::iterator iter=looseWires_.begin();iter!=looseWires_.end();iter++)
    {
        Wire* w =iter->second;
        if(!w->isHidden())
        {
            w->getPathBounds(u);
            if (g.clipRegionIntersects(u.getSmallestIntegerContainer()))
            {
                if((!(w->isSelected())) && (wireMoveList_.count(w)==0))
                {
                    std::vector<Segment*> wireSegments;
                    w->getSegments(draft,wireSegments);
                    for (std::vector<Segment*>::iterator i=wireSegments.begin();i!=wireSegments.end();i++)
                    {
                        segmentsToDraw.insert(std::pair<int,Segment*>((*i)->getHash(),(*i)));
                    }
                }
            }
        }
    }

    for (std::map<String,Wire*>::iterator iter=normalWires_.begin();iter!=normalWires_.end();iter++)
    {
        if(!(iter->second)->isSelected())
        {
            if(!(iter->second)->isHidden())
            {
                (iter->second)->getPathBounds(u);
                if (g.clipRegionIntersects(u.getSmallestIntegerContainer()))
                {
                    if(wireMoveList_.count(iter->second)==0)
                    {
                        std::vector<Segment*> wireSegments;
                        (iter->second)->getSegments(draft,wireSegments);
                        for (std::vector<Segment*>::iterator i=wireSegments.begin();i!=wireSegments.end();i++)
                        {
                            segmentsToDraw.insert(std::pair<int,Segment*>((*i)->getHash(),(*i)));
                        }
                    }
                }
            }
        }
    }
    
    for (std::map<int,Segment*>::iterator j=segmentsToDraw.begin();j!=segmentsToDraw.end();j++)
    {
        (j->second)->draw(g,zoomFactor_);
    }

    if(dummyTrunk_!=0)
    {
        dummyTrunk_->draw(g);
    }

    paintMode_=FULL; 
    overrideDraft_=false;

//    int t2=Time::currentTimeMillis();
//    pic::logmsg()<<"MainComponent paint: time="<<t2-t1;
//    pic::logmsg()<<"MainComponent paint"; 
}

void MainComponent::paintOverChildren(Graphics& g)
{
//  Draws selected wire (red) and wireChangeList wires on top of everything

    bool draft=false;

    if((paintOverMode_==QUICK)||(((mouseMode_==DRAGZOOM)||mouseWheeling_)))
    {
        draft=true;
    }

    std::map<int,Segment*> segmentsToDraw;

    if (selectedWire_!=0 )
    {
        if(!selectedWire_->isHidden())
        {
            std::vector<Segment*> wireSegments;
            selectedWire_->getSegments(draft,wireSegments);
            for (std::vector<Segment*>::iterator i=wireSegments.begin();i!=wireSegments.end();i++)
            {
                segmentsToDraw.insert(std::pair<int,Segment*>((*i)->getHash(),(*i)));
            }
        }
    }

    std::map<String, Wire*>::iterator iter;
    g.setColour(Colour(wireColour));

    juce::Rectangle <float> u;
    for(std::set<Wire*>::iterator i=wireMoveList_.begin();i!=wireMoveList_.end();i++)
    {
        Wire* w=*i;
        if(!w->isHidden())
        {
            w->getPathChangeBounds(u);
            if (g.clipRegionIntersects(u.getSmallestIntegerContainer()))
            {
                std::vector<Segment*> wireSegments;
                w->getSegments(draft,wireSegments);
                for (std::vector<Segment*>::iterator i=wireSegments.begin();i!=wireSegments.end();i++)
                {
                    segmentsToDraw.insert(std::pair<int,Segment*>((*i)->getHash(),(*i)));
                }
            }
        }
    }

    for (std::map<int,Segment*>::iterator j=segmentsToDraw.begin();j!=segmentsToDraw.end();j++)
    {
        (j->second)->draw(g,zoomFactor_);
    }

    paintOverMode_=FULL; 
}

void MainComponent::drawForegroundWires(Graphics& g)
{
    // Draws foregrounded wires on the displayLayer

    bool draft=false;
    if(((paintForegroundMode_==QUICK||mouseMode_==DRAGZOOM)||mouseWheeling_) && (!foregroundWireOverrideDraft_))
    {
        draft=true;
    }

    juce::Rectangle <float> u;
    std::map<int,Segment*> foregroundSegmentsToDraw;
    for (std::map<String,Wire*>::iterator iter=foregroundWires_.begin();iter!=foregroundWires_.end();iter++)
    {
        if(!(iter->second)->isHidden())
        {
            if( (!((iter->second)->isSelected())) && (wireMoveList_.count(iter->second)==0))
            {
                (iter->second)->getPathChangeBounds(u);
                if (g.clipRegionIntersects(u.getSmallestIntegerContainer()))
                {
                    std::vector<Segment*> wireSegments;
                    (iter->second)->getSegments(draft,wireSegments);
                    for (std::vector<Segment*>::iterator i=wireSegments.begin();i!=wireSegments.end();i++)
                    {
                        foregroundSegmentsToDraw.insert(std::pair<int,Segment*>((*i)->getHash(),(*i)));
                    }
                }
            }
        }
    }

    for (std::map<int,Segment*>::iterator j=foregroundSegmentsToDraw.begin();j!=foregroundSegmentsToDraw.end();j++)
    {
        (j->second)->draw(g,zoomFactor_);
    }

    foregroundWireOverrideDraft_=false;
}

Box* MainComponent::getTopLevelBox(DestinationPin* dp)
{
    if(dp!=0)
    {
        Box* b=dp->findParentComponentOfClass<Box>();
        if(b!=0)
        {
            return b->getTopLevelBox();
        }
    }
    return 0;
}

Box* MainComponent::getTopLevelSrcBox(Wire* w)
{
    String wid=w->getId();
    Box* b =getBoxById(w->getTopLevelSrcId());
    if(b!=0)
    {
        return b;
    }
    else
    {
        pic::logmsg()<<"top level src box for "<<std::string(wid.toUTF8())<<" not found";
        return 0;
    }
}

Box* MainComponent::getTopLevelDstBox(Wire* w)
{
    String wid=w->getId();
    Box* b =getBoxById(w->getTopLevelDstId());
    if(b!=0)
    {
        return b;
    }
    else
    {
        pic::logmsg()<<"top level dst box for "<<std::string(wid.toUTF8())<<" not found";
        return 0;
    }
}

Box* MainComponent::getTopLevelBox(SourcePin* sp)
{
    if(sp!=0)
    {
        Box* b=sp->findParentComponentOfClass<Box>();
        if(b!=0)
        {
            return b->getTopLevelBox();
        }
    }
    return 0;
}

void MainComponent::foregroundBoxes(Wire* w)
{
    Box* srcBox=getTopLevelBox(w->getUsingSP());
    Box* dstBox=getTopLevelBox(w->getUsingDP());

    if(srcBox!=0 && selectedItems_.isSelected(srcBox) && srcBox->isForegrounded() && !dstBox->isForegrounded() )
    {
        dstBox->setForeground(true,false);        
    }
    else if (dstBox!=0 && selectedItems_.isSelected(dstBox) && dstBox->isForegrounded() && !srcBox->isForegrounded()) 
    {
        srcBox->setForeground(true,false);        
    }
}

void MainComponent::unForegroundBoxes(Wire* w)
{
    //pic::logmsg()<<"unForegroundBoxes";
    Box* srcBox=getTopLevelSrcBox(w);
    Box* dstBox=getTopLevelDstBox(w);

    if(srcBox!=0 && dstBox!=0)
    {
        //pic::logmsg()<<"srcBox="<<srcBox->getId().toUTF8();
        //pic::logmsg()<<"dstBox="<<dstBox->getId().toUTF8();

        Atom* src=srcBox->getAtom();
        Atom* dst=dstBox->getAtom();
        if(!(src->is_slave_of(dst)||dst->is_slave_of(src))) 
        {

            //pic::logmsg()<<"not connected";
            if(srcBox!=0 && selectedItems_.isSelected(srcBox) && srcBox->isForegrounded() && dstBox->isForegrounded() && !selectedItems_.isSelected(dstBox))
            {
                //pic::logmsg()<<"dstBox->setForeground(false,false)";
                dstBox->setForeground(false,false);        
            }
            else if (dstBox!=0 && selectedItems_.isSelected(dstBox) && dstBox->isForegrounded() && srcBox->isForegrounded() && !selectedItems_.isSelected(srcBox)) 
            {
                //pic::logmsg()<<"dstBox->setForeground(false,false)";
                srcBox->setForeground(false,false);        
            }
        }
        else
        {
            //pic::logmsg()<<"still connected - do nothing";
        }
    }
    else if (srcBox==0)
    {
        //pic::logmsg()<<"srcBox=0";    
        
    }
    else if (dstBox==0)
    {
        //pic::logmsg()<<"dstBox=0";
    }
}

void MainComponent:: foregroundWire(Wire* w)
{
    bool fg=false;
    Box* srcBox=getTopLevelBox(w->getUsingSP());
    Box* dstBox=getTopLevelBox(w->getUsingDP());
    if(srcBox!=0 && srcBox->isForegrounded())
    {
        fg=true;
    }
    if(dstBox!=0 && dstBox->isForegrounded())
    {
        fg=true;
    } 

    if(fg)
    {
        w->setForegrounded(true);
    }
}

void MainComponent:: cacheWire(Wire* w)
{
    Box* srcBox=getTopLevelBox(w->getUsingSP());
    Box* dstBox=getTopLevelBox(w->getUsingDP());
    if(srcBox!=0)
    {
        srcBox->addSrcWire(w); 
    }
    if(dstBox!=0)
    {
        dstBox->addDstWire(w); 
    } 
}

void MainComponent::removeCachedWire(Wire* w)
{
    Box* srcBox=getTopLevelSrcBox(w);
    Box* dstBox=getTopLevelDstBox(w);

    if(srcBox!=0)
    {
        srcBox->removeSrcWire(w); 
    }
    else
    {
        pic::logmsg()<<" **** removeCachedWire: cant find toplevel box for sp: wire="<<std::string(w->getId().toUTF8());
    }

    if(dstBox!=0)
    {
        dstBox->removeDstWire(w); 
    }
    else
    {
        pic::logmsg()<<" **** removeCachedWire: cant find toplevel box for dp: wire="<<std::string(w->getId().toUTF8());
    }
}

void MainComponent::addDrawingWire(Wire* w)
{
    //pic::logmsg()<<"addDrawingWire "<<std::string(w->getId().toUTF8());
    w->preAdd();
    int h=w->getRouteHash();
    //pic::logmsg() <<"     hash="<<h;
    if(h!=0)
    {
        //find toplevel src and dst boxes and insert wire in their caches
        cacheWire(w);
        foregroundBoxes(w);
        foregroundWire(w);
        std::map<int,int>::iterator i=routes_.find(h);
        if (i!=routes_.end())
        {
            int c=i->second;
            routes_.erase(h);
            routes_.insert(std::pair<int,int>(h,c+1));
            //pic::logmsg()<<"     duplicate route: routes_incremented c="<<c+1;
        }
        else
        {
           routes_.insert(std::pair<int,int>(h,1)); 
           normalWires_.insert(std::pair<String,Wire*>(w->getId(),w));
           //pic::logmsg()<<"      new route: added to normalWires_";
        }
    }
    else
    {
        pendingWires_.insert(std::pair<String,Wire*>(w->getId(),w));
        w->setPending(true);
        if (w->getUsingDP()!=0 || w->getUsingSP()!=0)
        {
            cacheWire(w);     
        }
        pic::logmsg()<<"       added to pendingWires_";
    }

//    pic::logmsg()<<"maincomponent after addDrawingWire:  mormalwires contains ";
//    for (std::map<String,Wire*>::iterator iter=normalWires_.begin();iter!=normalWires_.end();iter++)
//    {
//        pic::logmsg()<<"     "<<std::string((iter->second)->getId().toUTF8());
//    }

}

Wire* MainComponent::getWireWithSameRoute(Wire* w)
{
    String id=w->getId();
    int hash=w->getRouteHash();
    for (std::map<String,Wire*>::iterator i=wires_.begin();i!=wires_.end();i++)
    {
        String tid=i->first;
        if(tid!=id)
        {
             if((i->second)->getRouteHash()==hash)
             {
                return i->second;
             }
        }
    }

    return 0;
}

void MainComponent::decrementRoutes(int h, Wire* w, int c)
{
    routes_.insert(std::pair<int,int>(h,c-1));
//    pic::logmsg()<<"     duplicate route: routes_decremented: routehash="<<h<<" c="<<(c-1);
//    std::cout<<"     duplicate route: routes_decremented: routehash="<<h<<" c="<<(c-1)<<std::endl;

    // is this wire (w) the wire representing this route in normalWires_?
    std::map<String,Wire*>::iterator j=normalWires_.find(w->getId());
    // if it is we need to replace it with another one as w is going to be deleted.
    if(j!=normalWires_.end())
    {
//       std::cout<<"this is the wire representing this route in normal wires"<<std::endl;
//       int n=normalWires_.erase(w->getId()); 
       normalWires_.erase(w->getId()); 
//       std::cout<<" removed "<<n<<" wires from normalWires"<<std::endl;
       Wire* rw=getWireWithSameRoute(w);
       // insert a different one to represent this route
       if(rw!=0)
       {
//            std::cout<<"replacing it with "<<rw<<" "<<rw->getId()<<std::endl;
            normalWires_.insert(std::pair<String,Wire*>(rw->getId(),rw));
       }
       else
       {
            pic::logmsg()<<"removeDrawingWire: route with duplicate removed but could not find replacement";
//            std::cout<<"removeDrawingWire: route with duplicate removed but could not find replacement"<<std::endl;
       }
    }
    else
    {
//        std::cout<<"this is not the wire representing this route in normal wires"<<std::endl;
    }
}

void MainComponent::removeUniqueRoute(int h, Wire* w)
{
//    pic::logmsg()<<"only 1 wire has this route";
//    std::cout<<"only 1 wire has this route"<<std::endl;
//    //test
//    std::map<String,Wire*>::iterator j=normalWires_.find(w->getId());
//    if(j!=normalWires_.end())
//    {
//
//    }
//    else
//    {
//        //std::cout<<"but cant find it in normalwires_"<<std::endl;
//    }
//
    for (std::map<String,Wire*>::iterator iter=normalWires_.begin();iter!=normalWires_.end();iter++)
    {
        Wire* w=iter->second;
        String id=iter->first;
        if(id!=w->getId())
        {
            pic::logmsg()<<"  ********* normalWires_ invalid! ******* "; 
//            std::cout<<"  ********* normalWires_ invalid! ******* "<<std::endl; 
            pic::logmsg()<<std::string(iter->first.toUTF8())<<"  "<<w->getId();
//            std::cout<<iter->first.toUTF8()<<"  "<<w->getId()<<std::endl;
        }

        if (w->getRouteHash()==h)
        {
            pic::logmsg()<<std::string(iter->first.toUTF8())<<"  "<<w->getId();

            //int c=normalWires_.erase(w->getId());
            normalWires_.erase(w->getId());
//            pic::logmsg()<<"      removed "<<c<<" wires from normalWires_";
//            std::cout<<"      removed "<<c<<" wires from normalWires_"<<std::endl;
            break;
        }
    }
}

void MainComponent::removeDrawingWire(Wire* w)
{
//    std::cout<<"removeDrawingWire "<<w<<std::endl;
    int h=w->getRouteHash();
    if (h!=0)
    {
//        pic::logmsg()<<"removeDrawingWire "<<w->getId().toUTF8();
//        std::cout<<"removeDrawingWire "<<w->getId().toUTF8()<<std::endl;
//        pic::logmsg()<<"     hash="<<h;
//        std::cout<<"     hash="<<h<<std::endl;;
        std::map<int,int>::iterator i=routes_.find(h);

        if (i!=routes_.end())
        {
            int c=i->second;
//            pic::logmsg()<<"hash "<<h<<" in routes: c= "<<c;
//            std::cout<<"hash "<<h<<" in routes: c= "<<c<<std::endl;
            routes_.erase(h);

            if(c>1)
            {
                decrementRoutes(h,w,c);
            }
            else
            {
                removeUniqueRoute(h,w);
            }
        }
        else
        {
           pic::logmsg()<<"Route hash "<<h<<" not found in routes_";
//           std::cout<<"Route hash "<<h<<" not found in routes_"<<std::endl;
           pic::logmsg()<<"this should not occur! Wire="<< std::string(w->getId().toUTF8());
//           std::cout<<"this should not occur! Wire="<< w->getId().toUTF8()<<std::endl;
        }
    }
    else
    {
//        std::cout<<"removeDrawingWire called but h=0"<<std::endl;
        //int n=normalWires_.erase(w->getId()); 
        normalWires_.erase(w->getId()); 
//        std::cout<<"      removed "<<n<<" wires from normalWires_"<<std::endl;
    }
}

void MainComponent::addRoute(int h, Wire* w)
{
//    pic::logmsg()<<"addRoute";
    std::map<int,int>::iterator iter=routes_.find(h);
    if (iter!=routes_.end())
    {
        int c=iter->second;
        routes_.erase(h);
        routes_.insert(std::pair<int,int>(h,c+1));
//        pic::logmsg()<<"     duplicate route: routes_ incremented c="<<c+1;
        // wire with this route must already be in normalWires
        // if this wire in normalWires (because its old route was different) remove it
        normalWires_.erase(w->getId());
    }
    else
    {
       routes_.insert(std::pair<int,int>(h,1));
//       pic::logmsg()<<"       unique route: added to routes_";
       std::pair<std::map<String,Wire*>::iterator,bool> p=normalWires_.insert(std::pair<String, Wire*>(w->getId(),w));
       if(p.second)
       {
//           pic::logmsg()<<"       unique route: added to normalWires_";
       } 
    }
}

void MainComponent::removeOldRoute(int oldh, Wire* w)
{
    std::map<int,int>::iterator i=routes_.find(oldh);
    if (i!=routes_.end())
    {
        int c=i->second;
//        int n=routes_.erase(oldh);
        routes_.erase(oldh);
//        pic::logmsg()<<"erased "<<n<<" routes from routes_ : occurred "<<c<<" times";
        if(c>1)
        {
            routes_.insert(std::pair<int,int>(oldh,c-1));
//            pic::logmsg()<<"     remove old duplicate route and decrement routes";
        }
        else
        {
//            for (std::map<String,Wire*>::iterator iter=normalWires_.begin();iter!=normalWires_.end();iter++)
//            {
//                Wire* w=iter->second;
//                if (w->getRouteHash()==oldh)
//                {
             normalWires_.erase(w->getId());
//                    pic::logmsg()<<"    remove only wire using old route from normalWires_ ";
//                    break;
//                }
//            }
        }
    }
    else
    {
//        pic::logmsg()<<"removeOldRoute: old hash "<<oldh<<" not found in routes_";
    }

}

void MainComponent::removeOldRoute1(int oldh)
{
    std::map<int,int>::iterator i=routes_.find(oldh);
    if (i!=routes_.end())
    {
        int c=i->second;
        //int n=routes_.erase(oldh);
        routes_.erase(oldh);
        //pic::logmsg()<<"erased "<<n<<" routes from routes_ : occurred "<<c<<" times";
        if(c>1)
        {
            routes_.insert(std::pair<int,int>(oldh,c-1));
            //pic::logmsg()<<"     remove old duplicate route and decrement routes";
        }
    }

    else
    {
        //pic::logmsg()<<"removeOldRoute1: old hash "<<oldh<<" not found in routes_";
    }

}


void MainComponent::insertLooseWire(Wire* w)
{
    if(w->isLoose())
    {
        looseWires_.insert(std::pair<String,Wire*>(w->getId(),w));
    }
    else
    {
        pic::logmsg()<<"insertLooseWire called but wire is not loose "<<w->getId();
    }
}

void MainComponent::changeDrawingWire(Wire* w)
{
    int h=w->getRouteHash();
    int oldh=w->getOldRouteHash();
    //pic::logmsg()<<"     hash="<<h;
    if(h!=0)
    {
        addRoute(h,w);
        //pic::logmsg()<<" old hash="<<oldh;
        if(h!=oldh)     // silly check needed because of forced change
        {
            removeOldRoute1(oldh);
        }
    }
    else
    {
//        std::cout<<"changedrawing wire called - but hash=0"<<std::endl;
        if(oldh!=0)     
        {
            removeOldRoute(oldh,w);
        }
    }

//    pic::logmsg()<<"maincomponent after changeDrawingWire:  mormalwires contains ";
//    for (std::map<String,Wire*>::iterator iter=normalWires_.begin();iter!=normalWires_.end();iter++)
//    {
//        pic::logmsg()<<"     "<<std::string((iter->second)->getId().toUTF8());
//    }
}

void MainComponent::resized()
{
}

void MainComponent::createSourcePin(int x, int y)
{
    int trueX=(float) x/zoomFactor_;
    int trueY=(float) y/zoomFactor_;
    model_->createSourcePin(trueX, trueY);
}

void MainComponent::createDestinationPin(int x, int y)
{
    int trueX=(float) x/zoomFactor_;
    int trueY=(float) y/zoomFactor_;
    model_->createDestinationPin(trueX, trueY);
}

void MainComponent::createHook(int x, int y)
{
    int trueX=(float) x/zoomFactor_;
    int trueY=(float) y/zoomFactor_;
    model_->createHook(trueX, trueY);
}

void MainComponent:: hookAdded(String id)
{
    //pic::logmsg()<< "MainComponent:hookAdded "<<std::string(id.toUTF8());
    Peg* p =new Peg(model_->get_store(id),zoomFactor_);
    pegs_.insert(std::pair<String,Peg*>(id,p));
    addAndMakeVisible (p);
}

void MainComponent::horizontalTrunkAdded(String id)
{
    PropertyStore* props=model_->get_store(id);
    Trunk* t=new Trunk (props,zoomFactor_,Trunk::HORIZONTAL);
    trunks_.insert(std::pair<String,Trunk*>(id,t));
    addAndMakeVisible (t);
}

void MainComponent::verticalTrunkAdded(String id)
{
    Trunk* t=new Trunk (model_->get_store(id),zoomFactor_,Trunk::VERTICAL);
    trunks_.insert(std::pair<String,Trunk*>(id,t));
    addAndMakeVisible (t);
}

void MainComponent::trunkCornerAdded(String id)
{
    TrunkCorner* c=new TrunkCorner(model_->get_store(id), zoomFactor_);
    corners_.insert(std::pair<String,TrunkCorner*>(id,c));
    addAndMakeVisible(c);
}

void MainComponent::routingAdded(String id)
{
    Wire* w=getWireById(id);
    if (w!=0)
    {
        w->addRouting();
    }
}

void MainComponent::sourcePinAdded(String id)
{
    SourcePin* sp = new SourcePin(model_->get_store(id),zoomFactor_);
    loose_sps_.insert(std::pair<String,SourcePin*>(sp->getId(),sp));
    addAndMakeVisible(sp);
    //pic::logmsg()<<"sourcepin created "<<std::string(sp->getId().toUTF8());
}

void MainComponent::destinationPinAdded(String id)
{
    DestinationPin* dp = new DestinationPin(model_->get_store(id),zoomFactor_);
    loose_dps_.insert(std::pair<String,DestinationPin*>(dp->getId(),dp));
    addAndMakeVisible(dp);
    //pic::logmsg()<<"destinationpin created "<<std::string(dp->getId().toUTF8());
}

void MainComponent::agentPropertyChanged(String id)
{
    //pic::logmsg()<<"MainComponent:agentPropertyChanged"<< std::string(id.toUTF8());
    //std::cout<<"MainComponent:agentPropertyChanged"<< std::endl;
    Box* b=getBoxById(id);
    if (b!=0)
    {
        b->changed(model_->get_store(id));
    }
}

void MainComponent:: hookChanged(String id)
{
    //pic::logmsg()<< "MainComponent:hookChanged "<<std::string(id.toUTF8());
    //std::cout<< "MainComponent:hookChanged "<<id.toUTF8()<<std::endl;
    RoutingElement* p =getRoutingElement(id);
    if(p!=0)
    {
        p->changed(model_->get_store(id));
    }
}

void MainComponent::horizontalTrunkChanged(String id)
{
    //pic::logmsg()<< "MainComponent:horizontalTrunkChanged "<<std::string(id.toUTF8());
    //std::cout<< "MainComponent:horizontalTrunkChanged "<<id.toUTF8()<<std::endl;
    RoutingElement* p =getRoutingElement(id);
    if(p!=0)
    {
        p->changed(model_->get_store(id));
    }
}

void MainComponent::verticalTrunkChanged(String id)
{
    //pic::logmsg()<< "MainComponent:verticalTrunkChanged "<<std::string(id.toUTF8());
    //std::cout<< "MainComponent:verticalTrunkChanged "<<id.toUTF8()<<std::endl;
    RoutingElement* p =getRoutingElement(id);
    if(p!=0)
    {
        p->changed(model_->get_store(id));
    }
}

void MainComponent::trunkCornerChanged(String id)
{
    //pic::logmsg()<< "MainComponent:trunkCornerChanged "<<std::string(id.toUTF8());
    //std::cout<< "MainComponent:trunkCornerChanged "<<id.toUTF8()<<std::endl;
    std::map<String,TrunkCorner*>::iterator pos=corners_.find(id);
    if(pos!=corners_.end())
    {
        TrunkCorner* c=pos->second;
        c->changed(model_->get_store(id));
    }

}

void MainComponent::sourcePinChanged(String id)
{
    //pic::logmsg()<< "MainComponent:sourcePinChanged "<<std::string(id.toUTF8());
    //std::cout<< "MainComponent:sourcePinChanged "<<id.toUTF8()<<std::endl;
    std::map<String,SourcePin*>::iterator pos=loose_sps_.find(id);
    if(pos!=loose_sps_.end())
    {
        SourcePin* c=pos->second;
        c->changed(model_->get_store(id));
    }
}

void MainComponent::destinationPinChanged(String id)
{
    //pic::logmsg()<< "MainComponent:destinationPinChanged "<<std::string(id.toUTF8());
    //std::cout<< "MainComponent:destinationPinChanged "<<id.toUTF8()<<std::endl;
    std::map<String,DestinationPin*>::iterator pos=loose_dps_.find(id);
    if(pos!=loose_dps_.end())
    {
        DestinationPin* c=pos->second;
        c->changed(model_->get_store(id));
    }

}

void MainComponent::routingChanged(String id)
{
    //std::cout<< "MainComponent:routingChanged "<<id.toUTF8()<<std::endl;
    Wire* w=getWireById(id);
    if (w!=0)
    {
        w->changeRouting();
    }
}

void MainComponent::propertiesRemoved(String id)
{
    //pic::logmsg()<<"MainComponent: propertiesRemoved";
    props_=0;
}

void MainComponent::propertiesAdded(String id)
{
    props_=model_->get_store(id);
    //pic::logmsg()<<"MainComponent: propertiesAdded zoomfactor="<<props_->get_double("zoomfactor");
    doZoom(props_->get_double("zoomfactor")); 
    setViewPosition(props_->get_number("viewoffsetx"),props_->get_number("viewoffsety"),false);
}

void MainComponent::propertiesChanged(String id)
{
    props_=model_->get_store(id);
    //pic::logmsg()<<"MainComponent: propertiesChanged zoomfactor="<<props_->get_double("zoomfactor");
    doZoom(props_->get_double("zoomfactor")); 
    setViewPosition(props_->get_number("viewoffsetx"),props_->get_number("viewoffsety"),false);
}


void MainComponent:: hookRemoved(String id)
{
    //pic::logmsg()<< "MainComponent:hookRemoved "<<std::string(id.toUTF8());
    RoutingElement* p =getRoutingElement(id);
    if(p!=0)
    {
        deleteRoutingElement(p);
    }
}

void MainComponent::horizontalTrunkRemoved(String id)
{
    //pic::logmsg()<< "MainComponent:horizontalTrunkRemoved "<<std::string(id.toUTF8());
    RoutingElement* p =getRoutingElement(id);
    if(p!=0)
    {
        doDeleteRoutingElement(p);
    }   
}

void MainComponent::verticalTrunkRemoved(String id)
{
    //pic::logmsg()<< "MainComponent:verticalTrunkRemoved "<<std::string(id.toUTF8());
    RoutingElement* p =getRoutingElement(id);
    if(p!=0)
    {
        doDeleteRoutingElement(p);
    } 
}

void MainComponent::trunkCornerRemoved(String id)
{
    //pic::logmsg()<< "MainComponent:trunkCornerRemoved "<<std::string(id.toUTF8());
    std::map<String,TrunkCorner*>::iterator pos=corners_.find(id);
    if(pos!=corners_.end())
    {
        TrunkCorner* c=pos->second;
        PropertyStore* props=c->getProps();
        corners_.erase(id);
        model_->remove_store(props);
        removeChildComponent(c);
        delete c;
    }
   
}

void MainComponent::routingRemoved(String id)
{
    Wire* w=getWireById(id);
    if (w!=0)
    {
        w->removeRouting();
    }
}

void MainComponent::createAgentBox(int x,int y, String  agentType, int ordinal)
{
    DummyBox* db=new DummyBox(x,y,zoomFactor_);
    addAndMakeVisible(db);

    int trueX=(float) x/zoomFactor_;
    int trueY=(float) y/zoomFactor_;
    model_->createAgent(trueX,trueY,agentType, ordinal);
}

int MainComponent::getDefaultX()
{
    return vp_->getViewPositionX()+20;
}

int MainComponent::getDefaultY()
{
    return vp_->getViewPositionY()+20;
}

void MainComponent::addBox(String id,Box* box)
{
    boxes_.insert(std::pair<String,Box*>(id,box));
}

void MainComponent::testRemoveBox(String id)
{
    int n=boxes_.erase(id);
    if (n==0)
    {
        pic::logmsg()<<"testRemoveBox: no boxes erased "<<std::string(id.toUTF8());
    }
}

void MainComponent::removeBox(String id)
{
    Box* b =getBoxById(id);
    if (b!=0)
    {
        for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
        {
            Wire* w=iter->second;
            if(w->getUsingDP()==b->getDstPin()||w->getUsingSP()==b->getSrcPin())
            {
                w->routeChanged(true,true);
            }
        }
    }
    else
    {
        pic::logmsg()<<"MainComponent:removeBox - box not found for "<<std::string(id.toUTF8());
    }

    int n=boxes_.erase(id);
    pic::logmsg()<<"erased "<<n<<" boxes";
}

void MainComponent::agentAdded(String id)
{
    pic::logmsg()<<"MainComponent::agentAdded:"<<std::string(id.toUTF8());

    if (boxes_.count(id)==0)
    {
        Agent* agent=model_->getAgent(id);
//        pic::logmsg()<<"is rig="<<agent->is_workspace();
        Box* box = new Box (agent,id,Box::TOPLEVEL,zoomFactor_);

        addBox(id,box);
        addAndMakeVisible (box);
//        pic::logmsg()<<"box position "<<box->getX()<<","<<box->getY();
        
        if(box->get_props()->get_number("expanded")==1)
        {
            box->doExpand(false,false);
        }
    }

//    else if(instanceParent_!=0)
//    {
//        Box* b = getBoxById(instanceParent_->get_id());        
//        if(b!=0)
//        {
//            b->refreshExpanded();
//            b->refreshButton();
//        }
//    }
//    
    else
    {
        Box* b=getBoxById(id);
        //pic::logmsg()<<"need to refresh box "<<b->getId();
        b->refreshPinVisibility();
        b->refreshExpanded();
        b->refreshButton();
        refreshProperties(b);
    }

    get_connections(id);
}

void MainComponent::get_connections(String id)
{
    model_->get_connections(id);
}

void MainComponent::deleteAgent(Box* b)
{
    if(b!=0)
    {
        String id=b->getId();
        if (id.isNotEmpty())
        {
            model_->deleteAgent(id);
        }
    }
}

void MainComponent::agentRemoved(String id)
{
    Box* b =getBoxById(id);
    if (b!=0)
    {
        deleteTopLevelBox(b);
    }
    pic::logmsg()<<"MainComponent:agentRemoved - absID="<< model_->get_absoluteID(id);
    String absId=model_->get_absoluteID(id);
    if(absId.equalsIgnoreCase("<main:eigend1>"))
    {
        JUCEApplication::quit();
    }
    else
    {
        wsf_->remove(absId);
    }
}

void MainComponent::instanceRemoved(String id)
{
    //pic::logmsg()<<"MainComponent:: instanceRemoved "<<std::string(id.toUTF8());
    Box* b = getBoxById(id);        
    if(b!=0)
    {
        b->refreshExpanded();
        b->refreshButton();
    }
    refreshProperties(id);
}

void MainComponent::portRemoved(String id)
{
    //pic::logmsg()<<"MainComponent: portRemoved "<< std::string(id.toUTF8());
    Box* b = getBoxById(id);        
    if(b!=0)
    {
        b->refreshExpanded();
        b->refreshButton();
    }
}

void MainComponent::nameChanged(String id)
{
    wsf_->nameChanged(model_->getFullName(id),model_->get_absoluteID(id));

    Box* b =getBoxById(id);
    if (b!=0)
    {
        b->nameChanged();

    }
    else
    {
        pic::logmsg()<<"MainComponent::NameChanged:box not found"; 
    }

    refreshProperties(id);
}

void MainComponent::instanceName(String id, String name)
{
    Box* b =getBoxById(id);
    if (b!=0)
    {
        b->setCreateInstanceName(name);
    }
}

void MainComponent::checkConnections(String id, std::set<String> conset )
{
    std::vector<String> v;
    for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
    {
        Wire* w=iter->second;
        String cid=w->getId();         
        if ((cid.fromFirstOccurrenceOf(":",false,true)).contains(id))
        {
            if((conset.count(cid)==0) && (!cid.contains("Pin")))
            {
                v.push_back(cid);
            }
        }
    }

//    if(v.size()>0)
//    {
//        pic::logmsg()<<"checkConnections:  Connection to be removed ";
//        //std::cout<<"checkConnections:  Connection to be removed "<<std::endl;
//        for(std::vector<String>::iterator i=v.begin();i!=v.end();i++)
//        {
//            pic::logmsg()<<"      "<<std::string((*i).toUTF8());
//            //std::cout<<"      "<<(*i).toUTF8()<<std::endl;;
//        }
//    }
//
   
    for(std::vector<String>::iterator i=v.begin();i!=v.end();i++)
    {
        connectionRemoved(*i);
    }

    checkPendingWires(id);
}

void MainComponent::deleteAssemblyElement(RoutingElement* p)
{
//    pic::logmsg()<<"deleteAssemblyElement";
    PropertyStore* props=p->getProps();
    if (props->has_key("assemblyId"))
    {
        String assemblyId=props->get_string("assemblyId");
        int delIndex=props->get_number("assemblyIndex");
        int size=props->get_number("assemblySize");

        for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
        {
            Trunk* t=iter->second;
            if (t!=0)
            {
                if(t->getProps()->get_string("assemblyId")==assemblyId)
                {
                    t->getProps()->set_number("assemblySize",size-1);
                    int elementIndex=t->getProps()->get_number("assemblyIndex");
                    if(elementIndex>delIndex)
                    {
                        t->getProps()->set_number("assemblyIndex",elementIndex-1);
                    }
                    t->save_props();
                }
            }
        }

        for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();)
        {
            TrunkCorner* tc=iter->second;
            if (tc->getAssemblyId()==assemblyId)
            {
               PropertyStore* props=tc->getProps();
               if((props->get_number("assemblyIndex")==delIndex)||(props->get_number("assemblyIndex")==delIndex-1))
               {
                   model_->remove_store(props);
                   corners_.erase(iter++);
                   removeChildComponent(tc);
                   delete tc;
               }
               else
               {
                    int cIndex=props->get_number("assemblyIndex");
                    if (cIndex>delIndex) 
                    {
                        props->set_number("assemblyIndex",cIndex-1);
                    }
                    ++iter;
               }
            }
            else
            {
                ++iter;
            }
        }

        doDeleteRoutingElement(p);
        repaint();
    }
}

void MainComponent::deleteAssembly(String assemblyId)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();)
    {
        Trunk* t=iter->second;
        PropertyStore* props=t->getProps();
        if(props->get_string("assemblyId")==assemblyId)
        {

        //  remove the trunk from any wires route that contains it

            for (std::map<String,Wire*>::iterator i=wires_.begin();i!=wires_.end();i++)
            {
                Wire* w=i->second;
                w->removePeg(t);
            }

            model_->remove_store(props);
            trunks_.erase(iter++);
            removeChildComponent(dynamic_cast<Component*>(t));
            delete(t);
        }
        else
        {
           ++iter;
        }
    }

    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();)
    {
        TrunkCorner* tc=iter->second;
        if (tc->getAssemblyId()==assemblyId)
        {
           PropertyStore* props=tc->getProps();
           model_->remove_store(props);
           corners_.erase(iter++);
           removeChildComponent(tc);
           delete tc;
        }
        else
        {
            ++iter;
        }
    }
    repaint();
}

void MainComponent::deleteRoutingElement(RoutingElement* p)
{
    //pic::logmsg()<<"deleteRoutingElement";
    PropertyStore* props=p->getProps();
    if (props->has_key("assemblyId"))
    {
        String assemblyId=props->get_string("assemblyId");
        deleteAssembly(assemblyId);
    }

    else
    {
        doDeleteRoutingElement(p);
    }        
    repaint();
}

void MainComponent::doDeleteRoutingElement(RoutingElement* p)
{
    String id =p->get_id();

    for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
    {
        Wire* w=iter->second;
        w->removePeg(p);
    }

    Selectable* s=dynamic_cast<Selectable*>(p);
    if(s!=0)
    {
        if(selectedItems_.isSelected(s))
        {
            selectedItems_.deselect(s);
        }
    }
    Movable* m=dynamic_cast<Movable*>(p);
    if(m!=0)
    {
        if(candidateMovable_==m)
        {
            candidateMovable_=0;
        }
    }

    PropertyStore* props=p->getProps();
    model_->remove_store(props);

    pegs_.erase(id);
    trunks_.erase(id);

    removeChildComponent(dynamic_cast<Component*>(p));
    delete(p);
}

void MainComponent::deselect(Selectable* b)
{
    if (selectedItems_.isSelected(b))
    {
        selectedItems_.deselect(b);
    }
}

void MainComponent::contractBox(Box* b, bool clearExpandedList)
{
//    pic::logmsg()<<"MainComponent contractBox id="<<b->getId().toUTF8();
    b->onContract(clearExpandedList);
    deselect(b);
	removeChildComponent(b);
    testRemoveBox(b->getId());
    clearCandidateWiringBox(b);
	delete(b);
    repaint();
}

void MainComponent::deleteBox(Box* b)
{
    //pic::logmsg()<<"MainComponent deleteBox id="<<std::string(b->getId().toUTF8());
    b->onDelete();
    deselect(b);
    removeChildComponent(b);
    removeBox(b->getId());
    clearCandidateWiringBox(b);
    delete(b);
    repaint();
}

void MainComponent::deleteWiresForBox(Box* b)
{
    String id=b->getId();
//    pic::logmsg()<<"DeleteWires for Box "<<std::string(b->getId().toUTF8());
    std::map<String,Wire*>::iterator iter=wires_.begin();
    while (iter!=wires_.end())
    {
        String k=iter->first;
        if(k.contains(id))
        {
            Wire* w=iter->second;
            if(selectedWire_==w)
            {
                selectedWire_->setSelected(false);
                selectedWire_=0;
            }
            removeForegroundWire(w);
            removeLoosePins(w);
            removeLooseWire(w);

            removeCachedWire(w);
            removePendingWire(w);
            wires_.erase(iter++);
            removeDrawingWire(w);
            
            w->removeFromPins();
            //pic::logmsg()<<"deleteWiresForBox 1: erasing wire "<<std::string(k.toUTF8())<<" wires remaining="<<wires_.size();
            delete w;
        }
        else
        {
            ++iter;
        }
    }
    repaint();
}

void MainComponent::deleteTopLevelBox(Box* b)
{

    if(b->isTopLevel())
    {
        if(selectedItems_.isSelected(b))
        {
            selectedItems_.deselect(b);
        }

        deleteWiresForBox(b);
        b->onDelete();

        pic::logmsg()<<"MainComponent deleteTopLevelBox id="<<std::string(b->getId().toUTF8());

        int n=boxes_.erase(b->getId());
        pic::logmsg() <<"erased "<<n<<" boxes";
        
        PropertyStore* p=b->get_props();
        if(p!=0)
        {
            model_->remove_store(p);
        }
        removeChildComponent(b);
        clearCandidateWiringBox(b);
        if(candidateMovable_==b)
        {
            candidateMovable_=0;
        }
        delete b;
        repaint();
    }
}

String MainComponent::getParentId(String id)
{
    if(id.contains("["))
    {
        return id.upToFirstOccurrenceOf("[",false,true);
    }
    else
    {
        return model_->getParentId(id);
    }
}

DestinationPin*  MainComponent::getDstPinById(String id, Wire* w)
{
    bool revconnect;
    //pic::logmsg()<<"getDstPinById "<<id.toUTF8()<<" revconnect="<<revconnect;
    String pid=id;
    while(pid.isNotEmpty())
    {
        Box* b =getBoxById(pid);
        if (b!=0)
        {
            revconnect=model_->has_protocol(id,"revconnect");
            if (b->hasInputBoxes())
            {
                Box* inputBox=0;
                inputBox=b->getFreeInputBox();
                if (inputBox!=0)
                {
                    String u=w->get_dstUsing();
                    pic::logmsg()<<std::string(u.toUTF8())<<" using";
                    if(u.isNotEmpty())
                    {
                        inputBox->changeName("Channel "+u);
                    }
                    else
                    {
                        inputBox->changeName("No channel set");
                    }
                    pic::logmsg()<<"returning pin on single input box";
                    if(revconnect)
                    {
                        return inputBox->getRevDstPin();
                    }
                    else
                    {
                        return inputBox->getDstPin();
                    }
                }
                if(inputBox==0)
                {
                    pic::logmsg()<<" ******  inputBox==0 "<<std::string(b->getId().toUTF8());
                    // fallback shouldnt occur
                    if(revconnect)
                    {
                        return b->getRevDstPin();
                    }
                    else
                    {
                        return b->getDstPin(); 
                    }
                }
            }
            else
            {
//                pic::logmsg()<<"returning pin on box";
                String u=w->get_dstUsing();
//                pic::logmsg()<<u.toUTF8()<<" using";
                if(u.isNotEmpty())
                {
                    b->refreshButton();
                }

                if(revconnect)
                {
                    return b->getRevDstPin();
                }
                else
                {
                    return b->getDstPin();
                }
            }
        }
        pid=getParentId(pid);
    }

    // try looking in the DestinationPins which are children of this   (LoosePins)
    DestinationPin* p=getLooseDestinationPinById(id);
    if (p!=0)
    {
        return p;
    }

    return 0;
}

SourcePin*  MainComponent::getSrcPinById(String id)
{
    bool revconnect;
    String pid=id;
    while(pid.isNotEmpty())
    {
        Box* b =getBoxById(pid);
        if (b!=0)
        {
            revconnect=model_->has_protocol(id,"revconnect");
            if(revconnect)
            {
                return b->getRevSrcPin();
            }
            else
            {
                return b->getSrcPin();
            }
        }
        pid=getParentId(pid);
    }

        // try looking in the SourcePins which are children of this   (LoosePins)
        SourcePin* p=getLooseSourcePinById(id);
        if (p!=0)
        {
            return p;
        }

    return 0;
}


void MainComponent::removeWireFromRoutingElement(String id, RoutingElement * r)
{
    Wire* w =getWireById(id);

    if (w!=0)
    {
        w->removePeg(r);
        r->removeId(id);
        repaint();
    }
}

void MainComponent::removeRouteFromRoutingElement(String id, RoutingElement* r)
{
    Wire* w=getWireById(id);
    if(w!=0)
    {
        int hash=w->getRouteHash();

        for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
        {
            w=iter->second;    
            if (w->getRouteHash()==hash)
            {
                w->removePeg(r);
                r->removeId(id);
            }
        }

        repaint();
    }
}


RoutingElement* MainComponent::getRoutingElement(String id)
{

    std::map<String, Peg*>::iterator pos=pegs_.find(id);
    if (pos!=pegs_.end())
    {
        return pos->second;
    }
    else
    {
        std::map<String, Trunk*>::iterator pos=trunks_.find(id);
        if (pos!=trunks_.end())
        {
            return pos->second;
        }
    }
    return 0;

}

SourcePin* MainComponent::getLooseSourcePinById(String id)
{
    std::map<String, SourcePin*>::iterator pos=loose_sps_.find(id);
    if (pos!=loose_sps_.end())
    {
        return pos->second;
    }
    else
    {
        return 0;
    }
}

DestinationPin* MainComponent::getLooseDestinationPinById(String id)
{
    std::map<String, DestinationPin*>::iterator pos=loose_dps_.find(id);
    if (pos!=loose_dps_.end())
    {
        return pos->second;
    }
    else
    {
        return 0;
    }
}

Box* MainComponent::getBoxByName(String name)
{

    for (std::map<String,Box*>::iterator iter=boxes_.begin();iter!=boxes_.end();iter++)
    {
        if((iter->second)->getName()==name)
        {
            return iter->second;
        }
    }

    return 0;
}

Box*  MainComponent::getBoxById(String id)
{
    std::map<String, Box*>::iterator pos=boxes_.find(id);
    if (pos!=boxes_.end())
    {
        return pos->second;
    }
    else
    {
        return 0;
    }
}

void MainComponent::dropLoosePin(const MouseEvent& e)
{
    DestinationPin* const p =dynamic_cast <DestinationPin *>(e.originalComponent);
    if(p!=0)
    {
        MouseEvent ev=e.getEventRelativeTo(this);
        int x=ev.x;
        int y=ev.y;
        pic::logmsg()<<"dropLoosePin- original component was dstPin "<<ev.x<<" "<<ev.y;
        dropLoosePin(p,x,y);
        clearHighlight();
    }
    else
    {
        SourcePin* const sp =dynamic_cast <SourcePin *>(e.originalComponent);
        if (sp!=0)
        {
            MouseEvent ev=e.getEventRelativeTo(this);
            int x=ev.x;
            int y=ev.y;
            pic::logmsg()<<"dropLoosePin- original component was srcPin "<<ev.x<<" "<<ev.y;
            dropLoosePin(sp,x,y);
            clearHighlight();
        }
    }
}

Wire* MainComponent::getWireById(String wid)
{
    std::map<String, Wire*>::iterator iter=wires_.find(wid);
    if (iter!=wires_.end())
    {
        return iter->second;
    }
    return 0;
}

Wire* MainComponent::getWireForDstPin(DestinationPin * dp)
{
    return dp->getWire();
}

Wire* MainComponent::getWireForSrcPin(SourcePin * sp)
{
    return sp->getWire();
}

int MainComponent::getInput(RoutingElement* r, Wire* w)
{
    PropertyStore* props=r->getProps();
    String listname="input"+String(w->getId().hashCode());
    
    if(props->has_list(listname))
    {
        return (props->get_list(listname,1)).getIntValue();
    }

    return 1;
}

int MainComponent::getOutput(RoutingElement* r, Wire* w)
{
    PropertyStore* props=r->getProps();
    String listname="input"+String(w->getId().hashCode());
    if(props->has_list(listname))
    {
        return (props->get_list(listname,2)).getIntValue();
    }

    return 1;
}

void MainComponent::dropLoosePin(SourcePin* p, int x, int y)
{
    Wire* w =getWireForSrcPin(p);
    if (w!=0)
    {
        Box* const b2=getBoxAt(x,y);
        if (b2!=0)
        {
            pic::logmsg()<<"ended on "<<std::string(b2->getName().toUTF8());
            String did=w->getDstId();
            if(!(did.startsWith(b2->getTopLevelBox()->getId())))
            {
                dropPinMap_.insert(std::pair<String,StoredDropPin*> ((b2->getId()+":"+did),new StoredDropPin(w,p)));
                model_->check_connection(b2->getId(),did);
            }
        }
    }
}

void MainComponent::dropLoosePin(DestinationPin* p, int x, int y)
{
    Wire* w =getWireForDstPin(p);

    if (w!=0)
    {
        Box* const b2=getBoxAt(x,y);
        if (b2!=0)
        {
            pic::logmsg()<<"ended on "<<std::string(b2->getName().toUTF8());
            String sid=w->getSrcId();

            if(!(sid.startsWith(b2->getTopLevelBox()->getId())))
            {
                // if its a single input box use the parent
                String boxId=b2->getId();
                String name=b2->getName();

                if(b2->isSingleInput())
                {
                    pic::logmsg()<<"ended on singleInput box";
                    Box* pb =dynamic_cast <Box *>(b2->getParentComponent());
                    boxId=pb->getId();
                    name=pb->getName();
                }

                dropPinMap_.insert(std::pair<String,StoredDropPin*> ((sid+":"+boxId),new StoredDropPin(w,p)));
                model_->check_connection(sid,boxId);
            }
        }
        else
        {
            pic::logmsg()<<"not dropped on valid box";
        }
    }
}

void MainComponent::connectionPossible(String sid,String did, bool possible)
{
    String id=sid+":"+did;
    if(possible)
    {
        pic::logmsg()<<"MainComponent: connectionPosible sid="<<std::string(sid.toUTF8())<<" did="<<std::string(did.toUTF8());
        std::map<String,StoredDropPin*>::iterator i=dropPinMap_.find(id);
        if(i!=dropPinMap_.end())
        {
            Pin* p=(i->second)->getPin();
            DestinationPin* dp=dynamic_cast<DestinationPin*>(p);
            if(dp!=0)
            {
                doDropPin(sid, did,(i->second)->getWire(),dp);
            }
            else
            {
                doDropPin(sid,did,(i->second)->getWire(),dynamic_cast<SourcePin*>(p));
            }
            dropPinMap_.erase(i);
        }
    }
    else
    {
        pic::logmsg()<<"MainComponent: connection not posible sid="<<std::string(sid.toUTF8())<<" did="<<std::string(did.toUTF8());
        std::map<String,StoredDropPin*>::iterator i=dropPinMap_.find(id);
        if(i!=dropPinMap_.end())
        {
            Pin* p=(i->second)->getPin();
            Wire* w=(i->second)->getWire();

            DestinationPin* dp=dynamic_cast<DestinationPin*>(p);
            if (dp!=0)
            {

                Box* b=getBoxById(did);
                if(b!=0)
                {
                    Box* tlb= b->getTopLevelBox();
                    dp->doLoosePinDrag(tlb->getX()-(16*zoomFactor_),tlb->getY());
                    w->pathChanged();
                    w->preDraw(); 
                    thingMoved();
                }
            }
            else
            {
                SourcePin* sp=dynamic_cast<SourcePin*>(p);
                if(sp!=0)
                {
                    Box* b=getBoxById(sid);
                    if(b!=0)
                    {
                        Box* tlb= b->getTopLevelBox();
                        sp->doLoosePinDrag(tlb->getX()+((BOXWIDTH+16)*zoomFactor_),tlb->getY());
                        w->pathChanged();
                        w->preDraw(); 
                        thingMoved();
                    }
                }
            }
// get the box to which connection was attempted
// move the looose pin to the left of the box

            p->highlight(false);
            juce::Rectangle <float> u;
            w->getPathBounds(u);
            repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
            dropPinMap_.erase(i);
        }

        ErrorReportComponent* e=new ErrorReportComponent("No matching inputs and outputs were found", "");
        DialogWindow::showModalDialog(String("Connection not made"),e,getTopLevelComponent(),Colour (0xffababab),true);
        delete e; 
    }
}


void MainComponent::doDropPin(String sid, String did,Wire* w, SourcePin* p)
{
    juce::Rectangle <float> u;
    w->getPathBounds(u);

    std::list<StoredRouteElement> route;
    storeRoute(w,route);
    saveRoute(sid+":"+did,route);

    model_->create_connection(sid,did);

    if(selectedWire_==w)
    {
        selectedWire_=0;
    }

    removeCachedWire(w);
    removeLoosePin(p);
    repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));

}

void MainComponent::doDropPin(String sid, String did,Wire* w, DestinationPin* p)
{
    juce::Rectangle <float> u;
    w->getPathBounds(u);

    std::list<StoredRouteElement> route;
    storeRoute(w,route);
    saveRoute(sid+":"+did,route);
    model_->create_connection(sid,did);
    if(selectedWire_==w)
    {
        selectedWire_=0;
    }
    removeCachedWire(w);
    removeLoosePin(p);

    repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
}

void MainComponent::removeLoosePin(DestinationPin* d)
{       

    if(d->isLoosePin())
    {
        String t=":"+d->getId();
        std::map<String,Wire*>::iterator iter=wires_.begin();
        while (iter!=wires_.end())
        {
            String k=iter->first;
            if(k.endsWith(t))
            {
                Wire* w=iter->second;
                removeForegroundWire(w);
                removeLooseWire(w);
                if(selectedWire_==w)
                {
                    w->setSelected(false);
                    selectedWire_=0;
                }
                wires_.erase(iter++);
                w->removeFromPins();
                delete w;
            }
            else
            {
                ++iter;
            }
        }

        int n=loose_dps_.erase(d->getId());
        pic::logmsg()<<"erased "<<n<<" loose destination pins";
        PropertyStore* props=d->getProps();
        model_->remove_store(props);
        AutoDragable* ad=dynamic_cast<AutoDragable*>(d);
        if(ad!=0)
        {
            if(ad==autoDragComp_)
            {
                autoDragComp_=0;
                stopAutoDrag();
//                std::cout<<" ******* autoDragComp stll points to this pin !"<<std::endl;
            }
        }
        removeChildComponent(d);
        delete d;
    }
}

void MainComponent::removeLoosePin(SourcePin* d)
{       
    if(d->isLoosePin())
    {
        String t=d->getId()+":";
        std::map<String,Wire*>::iterator iter=wires_.begin();
        while (iter!=wires_.end())
        {
            String k=iter->first;
            if(k.startsWith(t))
            {
                Wire* w=iter->second;
                removeForegroundWire(w);
                removeLooseWire(w);
                if (selectedWire_==w)
                {
                    w->setSelected(false);
                    selectedWire_=0;
                }
                wires_.erase(iter++);
                w->removeFromPins();
                delete w;
            }
            else
            {
                ++iter;
            }
        }

        int n=loose_sps_.erase(d->getId());
        pic::logmsg()<<"erased "<<n<<" loose source pins";
        PropertyStore* props=d->getProps();
        model_->remove_store(props);
        AutoDragable* ad=dynamic_cast<AutoDragable*>(d);
        if(ad!=0)
        {
            if(ad==autoDragComp_)
            {
                autoDragComp_=0;
                stopAutoDrag();
//                std::cout<<" ***** autoDragComp stll points to this pin !"<<std::endl;
            }
        }

        removeChildComponent(d);
        delete d;
    }
}

String MainComponent::get_fulldesc(String id)
{
    return model_->getFullName(id);
}

String MainComponent::getLooseWireDesc(Wire* w)
{
    String u=String();
    String wid=w->getUsingId();
    String id=w->getId();

    String src;
    String dst;
    String fullSrc;
    String fullDst;
    getWireDescription(w,src,fullSrc,dst,fullDst,u);

    return fullSrc + " to " + fullDst;
}

void MainComponent::getWireDescription(Wire* w, String& src, String& fullSrc, String& dst, String& fullDst, String& u )
{
        String wid=w->getUsingId();
        String id=w->getId();

        if(w->isLoose())
        {
            if(wid.upToFirstOccurrenceOf(":",false,true).contains("SourcePin"))
            {
                src="unconnected";
                fullSrc="unconnected";
            }
            else
            {
                src=model_->getFullName(wid.upToFirstOccurrenceOf(":",false,true));
                fullSrc=model_->getFullName(id.upToFirstOccurrenceOf(":",false,true));
            }
            if(wid.fromFirstOccurrenceOf(":",false,true).contains("DestinationPin"))
            {
                dst="unconnected";
                fullDst="unconnected";
            }
            else
            {
                dst=model_->getFullName(wid.fromFirstOccurrenceOf(":",false,true));
                fullDst=model_->getFullName(id.fromFirstOccurrenceOf(":",false,true));
            }
        }
        else
        {
            u=w->get_dstUsing();
            src=model_->getFullName(wid.upToFirstOccurrenceOf(":",false,true));
            dst=model_->getFullName(wid.fromFirstOccurrenceOf(":",false,true));
            fullSrc=model_->getFullName(id.upToFirstOccurrenceOf(":",false,true));
            fullDst=model_->getFullName(id.fromFirstOccurrenceOf(":",false,true));
        }
}

void MainComponent::createTooltip( Wire* w,int segmentHash)
{
    if(w!=0)
    {
//        std::cout<<"createTooltip w="<<w<<std::endl;
        String u=String();
        String wid=w->getUsingId();
        String id=w->getId();

        String src;
        String dst;
        String fullSrc;
        String fullDst;
        getWireDescription(w,src,fullSrc,dst,fullDst,u);

        int count=0;
        for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
        {
            Wire* w1=iter->second;
            if(w->hasSameRoute(w1))
            {
                count++;
            }
        }

        if (count==1)
        {
            dst=fullDst;
            src=fullSrc;
            if(u.isNotEmpty())
            {
                dst=dst+" ("+ u+ ")";
            }
        }

        if(src.isNotEmpty() && dst.isNotEmpty())
        {
            tooltip_=src + " to " + dst;
        }
        else
        {
            pic::logmsg()<<"createtooltip: description not found";
            tooltip_=String();
        }
        if (count>1)
        {
            tooltip_=tooltip_+" ("+String(count) +" connections)"  ;
        }

        int shareCount=0;
        for(std::vector<Wire*>::iterator iter=wiresAt_.begin();iter!=wiresAt_.end();iter++)
        {
            if((*iter)->getRouteHash()!=w->getRouteHash())
            {
               if((*iter)->hasSegment(segmentHash)) 
               {
                    shareCount++;
               }
            }
        }

        if(shareCount>=1)
        {
            tooltip_=tooltip_+"\n(Other connections also use this wire section - shift-click to cycle through them)";
        }

    }
    else
    {
        tooltip_=String();
    }
}

void MainComponent::repaintWire(Wire* w)
{
    if(w!=0)
    {
        juce::Rectangle <float> u;
        w->getPathBounds(u);
        repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
    }
}
void MainComponent::repaintReRoutedWire(Wire* w)
{
    if(w!=0)
    {
        juce::Rectangle <float> u;
        w->getPathChangeBounds(u);
        repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
    }
}

void MainComponent::repaintSelectedWire()
{
    if(selectedWire_!=0)
    {
        repaintWire(selectedWire_);
    }
}

Wire* MainComponent::getSelectedWire()
{
    return selectedWire_;
}

void MainComponent::mouseMove (const MouseEvent& e)
{
//    if(this==e.originalComponent)
//    {
        listWiresAt(e.x,e.y);
        unsigned numWires=wiresAt_.size();
        int tool=tm_->getTool();

        if (tool==ToolManager::MOVETOOL||
            tool==ToolManager::POINTERTOOL||
            tool==ToolManager::DELETETOOL||
            tool==ToolManager::WIRINGTOOL||
            tool==ToolManager::WIRING_LEFT||
            //tool==ToolManager::HELPTOOL||
            tool==ToolManager::EDITTOOL)

        {

            if (numWires>0)
            {
                //pic::logmsg()<<"MainComponent mouseMove:numWires>0 "<<this<<" x="<<e.x<<" y="<<e.y;
                if(!wireIsAt(selectedWire_,e.x,e.y))
                {
                    wireIndex_=0;
                    if (wiresAt_[wireIndex_]!=selectedWire_)
                    {
                        if (selectedWire_!=0)
                        {
                            juce::Rectangle <float> u;
                            selectedWire_->getPathBounds(u);
                            selectedWire_->setSelected(false);
                            repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
                        }

                        selectWire(wiresAt_[wireIndex_]);
                        juce::Rectangle <float> u;
                        selectedWire_->getPathBounds(u);
                        repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
                        int segmentHash=selectedWire_->getSegmentHashAt(e.x,e.y);

                        createTooltip(selectedWire_,segmentHash);
                    }
                }
            }
            else
            {
                if (selectedWire_!=0)
                {
                    juce::Rectangle <float> u;
                    selectedWire_->getPathBounds(u);
                    selectedWire_->setSelected(false);
                    selectedWire_=0;
                    repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
                }

                tooltip_=String::empty;
            }
        }

        if(tool==ToolManager::MOVETOOL)
        {
            if(selectedWire_==0)
            {
                moveToolMouseMove(e);
            }
        }

        else if(tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
        {
            if(selectedWire_==0)
            {
                wiringToolMouseMove(e);
            }
        }
//    }
//    else
//    {
//        pic::logmsg()<<"mouseMoved original component!=MainComponent";
//    }
}

void MainComponent::moveToolMouseMove(const MouseEvent& e)
{
    if(candidateMovable_!=0)
    {
        candidateMovable_->setMouseOver(false);
    }
    candidateMovable_=0;

    candidateMovable_=getMovableAt(e.x,e.y);
    if(candidateMovable_!=0)
    {
         candidateMovable_->setMouseOver(true);
    }
}

Movable* MainComponent::getMovableAt(int x, int y)
{
    Box * b =getBoxExactlyAt(x,y);
    if(b!=0)
    {
        return b->getTopLevelBox();    
    }
    else
    {
        RoutingElement* p=getRoutingElementExactlyAt(x,y);
        if(p!=0)
        {
            return dynamic_cast<Movable*>(p);
        }
        else
        {
            b=getBoxNear(x,y,20,5);
            if(b!=0)
            {
                return b->getTopLevelBox();
            }
            else
            {
                p=getRoutingElementNear(x,y,20,5);
                if(p!=0)
                {
                    Peg* m=dynamic_cast<Peg*>(p);
                    if(m!=0)
                    {
                        return m;
                    }
                    else
                    {
                        Trunk* t=dynamic_cast<Trunk*>(p);
                        if(t!=0)
                        {
                            return t;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

void MainComponent::wiringToolMouseMove(const MouseEvent& e)
{
    if(candidateWiringBox_!=0)
    {
        candidateWiringBox_->highlight(false);
    }
    candidateWiringBox_=0;

    Box * b =getBoxAt(e.x,e.y);
    if(b!=0)
    {
         b->doWiringHighlights(e.getEventRelativeTo(b));
         b->highlight(true);
         candidateWiringBox_=b;
// XXX cant get this working - seems to cause spurious mouseDrag event when do mouseDown
//                 int newTool=tm_->getTool();
//                 if(newTool==ToolManager::WIRINGTOOL)
//                 {
//                    candidateWiringBox_->getSrcPin()->selectWire();
//                 }
//                 else if(newTool==ToolManager::WIRING_LEFT)
//                 {
//                    candidateWiringBox_->getDstPin()->selectWire();
//                 }
    }
}

const MouseCursor MainComponent::getCursor()
{
    return tm_->getMouseCursor();
}

const MouseCursor MainComponent::getCursor(int cur)
{
    return tm_->getMouseCursor(cur);
}

void MainComponent::doMoveSubAssemblyGreater(int dx, int dy,String assemblyId,int assemblyIndex)
{

    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;
        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                if((t->getProps()->get_number("assemblyIndex"))>assemblyIndex)
                {
                    t->doAssemblyMove(dx,dy);
                }
            }
        }
    }
}

void MainComponent::doMoveSubAssemblyLess(int dx, int dy,String assemblyId,int assemblyIndex)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;

        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                if((t->getProps()->get_number("assemblyIndex"))<assemblyIndex)
                {
                    t->doAssemblyMove(dx,dy);
                }
            }
        }
    }
}

void MainComponent::doMoveCornerAssemblyGreater(int dx, int dy,String assemblyId,int assemblyIndex)
{
    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* t=iter->second;

        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                if((t->getProps()->get_number("assemblyIndex"))>=assemblyIndex)
                {
                    t->doAssemblyMove(dx,dy);
                }
            }
        }
    }
}

void MainComponent::doMoveCornerAssemblyLess(int dx, int dy,String assemblyId,int assemblyIndex)
{
    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* t=iter->second;

        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                if((t->getProps()->get_number("assemblyIndex"))<assemblyIndex)
                {
                    t->doAssemblyMove(dx,dy);
                }
            }
        }
    }
}

void MainComponent::moveSubAssemblyX(String assemblyId, int assemblyIndex, int direction,int pos,int dx)
{
    if ((direction==Trunk::LEFT_TO_RIGHT) && (pos==Trunk::RIGHT))
    {
        doMoveSubAssemblyGreater(dx,0,assemblyId,assemblyIndex);
        doMoveCornerAssemblyGreater(dx,0,assemblyId,assemblyIndex);
    }
    else if ((direction==Trunk::RIGHT_TO_LEFT) && (pos==Trunk::RIGHT))
    {
        doMoveSubAssemblyLess(dx,0,assemblyId,assemblyIndex);
        doMoveCornerAssemblyLess(dx,0,assemblyId,assemblyIndex);
    }
    else if((direction==Trunk::LEFT_TO_RIGHT)&& (pos==Trunk::LEFT))
    {
        doMoveSubAssemblyLess(-dx,0,assemblyId,assemblyIndex);
        doMoveCornerAssemblyLess(-dx,0,assemblyId,assemblyIndex);
    }
    else if((direction==Trunk::RIGHT_TO_LEFT)&& (pos==Trunk::LEFT))
    {
        doMoveSubAssemblyGreater(-dx,0,assemblyId,assemblyIndex);
        doMoveCornerAssemblyGreater(-dx,0,assemblyId,assemblyIndex);
    }
}

void MainComponent::moveSubAssemblyY(String assemblyId, int assemblyIndex, int direction,int pos, int dy)
{
    if ((direction==Trunk::TOP_TO_BOTTOM) && (pos==Trunk::BOTTOM))
    {
        doMoveSubAssemblyGreater(0,dy,assemblyId,assemblyIndex);
        doMoveCornerAssemblyGreater(0,dy,assemblyId,assemblyIndex);
    }
    else if ((direction==Trunk::BOTTOM_TO_TOP) && (pos==Trunk::BOTTOM))
    {
        doMoveSubAssemblyLess(0,dy,assemblyId,assemblyIndex);
        doMoveCornerAssemblyLess(0,dy,assemblyId,assemblyIndex);
    }
    else if((direction==Trunk::TOP_TO_BOTTOM)&& (pos==Trunk::TOP))
    {
        doMoveSubAssemblyLess(0,-dy,assemblyId,assemblyIndex);
        doMoveCornerAssemblyLess(0,-dy,assemblyId,assemblyIndex);
    }
    else if((direction==Trunk::BOTTOM_TO_TOP)&& (pos==Trunk::TOP))
    {
        doMoveSubAssemblyGreater(0,-dy,assemblyId,assemblyIndex);
        doMoveCornerAssemblyGreater(0,-dy,assemblyId,assemblyIndex);
    }
}

void MainComponent::moveAssembly(String assemblyId,int dx, int dy)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;

        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                t->doAssemblyMove(dx,dy);
            }
        }
    }

    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;

        if(tc!=0)
        {
            if(tc->getAssemblyId()==assemblyId)
            {
                tc->doAssemblyMove(dx,dy);
            }
        }
    }
}

void MainComponent::revertForegroundAssembly(String assemblyId)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;

        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                t->doRevertForegrounded();
            }
        }
    }

    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;

        if(tc!=0)
        {
            if(tc->getAssemblyId()==assemblyId)
            {
                tc->doSetForegrounded(true);
            }
        }
    }
}


void MainComponent::foregroundAssembly(String assemblyId,bool foregrounded, String id)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;

        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                t->doSetForegrounded(foregrounded,id);
            }
        }
    }

    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;

        if(tc!=0)
        {
            if(tc->getAssemblyId()==assemblyId)
            {
                tc->doSetForegrounded(foregrounded);
            }
        }
    }
}

void MainComponent::bringToFrontAssembly(String assemblyId)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;

        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                t->doBringToFront();
            }
        }
    }

    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;

        if(tc!=0)
        {
            if(tc->getAssemblyId()==assemblyId)
            {
                tc->doBringToFront();
            }
        }
    }
}

void MainComponent::toBackAssembly(String assemblyId)
{
    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;

        if(tc!=0)
        {
            if(tc->getAssemblyId()==assemblyId)
            {
                tc->toBack();
            }
        }
    }

    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;

        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                t->toBack();
            }
        }
    }


}

void MainComponent::toFrontAssembly(String assemblyId)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;

        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                t->toFront(true);
            }
        }
    }

    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;

        if(tc!=0)
        {
            if(tc->getAssemblyId()==assemblyId)
            {
                tc->toFront(true);
            }
        }
    }
}

void MainComponent::setAssemblyProps(String assemblyId)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;
        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                t->doAssemblySetProps();
            }
        }
    }
    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;

        if (tc!=0)
        {
            if(tc->getProps()->get_string("assemblyId")==assemblyId)
            {
                tc->doAssemblySetProps();
            }
        }
    }
}

void MainComponent::mouseDown (const MouseEvent& e)
{
    pic::logmsg()<<"MainComponent mouseDown "<<this<<" x="<<e.x<<" y="<<e.y;
   	ModifierKeys m=e.mods;
	if (m.isPopupMenu()==true)
    {
        // do nothing
    }
    else if (tm_->getTool()==ToolManager::MOVETOOL) 
    {
        moveToolMouseDown(e);
//    else if(testPath_.contains(e.x,e.y))
//    {
//        pic::logmsg()<<"in shape "<< e.x<<" "<<e.y;
//    }
    }

    else if(tm_->getTool()==ToolManager::SELECTTOOL)
    {
       mouseMode_=LASSO;
       lassoComponent_->beginLasso(e,this); 
    }
    else if(tm_->getTool()==ToolManager::POINTERTOOL||tm_->getTool()==ToolManager::HELPTOOL)
    {
        mouseMode_=DRAGCANVAS;
        pic::logmsg()<<"mousemode set to DRAGCANVAS";
        //setMouseCursor(tm_->getGrabCursor());
        setMouseCursor(getCursor(ToolManager::GRAB));
        updateMouseCursor();
        startCanvasDrag(e);
    }
    else if(tm_->getTool()==ToolManager::ZOOMTOOL)
    {   
        mouseMode_=DRAGZOOM;
        startZoomDrag(e); 
    }
    else if(tm_->getTool()==ToolManager::TRUNKTOOL)
    {
        mouseMode_=CREATE_TRUNK;
        startCreateTrunkDrag(e);
    }
    else if(tm_->getTool()==ToolManager::WIRINGTOOL)
    {
        if(candidateWiringBox_!=0)
        {
            if(candidateWiringBox_->isReversed())
            {
                dstPinMouseDown(e);
            }
            else
            {
                srcPinMouseDown(e);
            }
        }
    }
    else if(tm_->getTool()==ToolManager::WIRING_LEFT)
    {
        if(candidateWiringBox_!=0)
        {
            if(candidateWiringBox_->isReversed())
            {
                srcPinMouseDown(e);
            }
            else
            {
                dstPinMouseDown(e);
            }
        }
    }

}

void MainComponent::moveToolMouseDown (const MouseEvent& e)
{
    if(selectedWire_!=0)
    {
        pic::logmsg()<<"moveToolMouseDown: selectedWire!=0";
        setMouseCursor(getCursor(ToolManager::GRAB));
        updateMouseCursor();
        mouseMode_=DRAGWIRE;
        wireDragStarted_=false;
    }

    else if(candidateMovable_!=0)
    {
        candidateMovable_->mouseDown(e.getEventRelativeTo(dynamic_cast<Component*>(candidateMovable_)));
    }


    else
    {
        mouseMode_=DRAGCANVAS;
        pic::logmsg()<<"mousemode set to DRAGCANVAS";
        //setMouseCursor(tm_->getGrabCursor());
        setMouseCursor(getCursor(ToolManager::GRAB));
        updateMouseCursor();
        startCanvasDrag(e);
    }
}

void MainComponent::mouseDrag (const MouseEvent& e)
{

    if (mouseMode_==DRAGWIRE)
    {
        if(!wireDragStarted_)
        {
            startWireDrag(e);
        }
        else
        {
            wireDrag(e);
        }
    }
    else if (mouseMode_==DRAGCANVAS)
    {
        canvasDrag(e);
    }
    else if (mouseMode_==DRAGZOOM)
    {
        zoomDrag(e);
    }
    else if (mouseMode_==CREATE_TRUNK)
    {
        createTrunkDrag(e);
    }
    else if (mouseMode_==LASSO)
    {
        if (inViewPort(e.x,e.y))
        {
            stopAutoDrag();
        }
        else
        {
            autoDrag(e.x,e.y, false);
        }

        lassoComponent_->dragLasso(e);
    }
    else if(tm_->getTool()==ToolManager::WIRINGTOOL)
    {
        wiringToolMouseDrag(e);

    }
    else if(tm_->getTool()==ToolManager::WIRING_LEFT)
    {
        wiringLeftMouseDrag(e);
    }
    else if (tm_->getTool()==ToolManager::MOVETOOL)
    {
        moveToolMouseDrag(e);
    }
}

void MainComponent::wiringToolMouseDrag(const MouseEvent& e)
{
    if(candidateWiringBox_!=0)
    {
        if(candidateWiringBox_->isReversed())
        {
            DestinationPin* p=candidateWiringBox_->getDstPin();
            if(p->isConnecting())
            {
                dstPinMouseDrag(e);
            }
            else
            {
                dstPinMouseDisconnectDrag(e);
            }
        }
        else
        {
            SourcePin* p=candidateWiringBox_->getSrcPin();
            if(p->isConnecting())
            {
                srcPinMouseDrag(e);
            }
            else
            {
                srcPinMouseDisconnectDrag(e);
            }
        }
    }
}

void MainComponent::wiringLeftMouseDrag(const MouseEvent& e)
{
    if(candidateWiringBox_!=0)
    {
        if(candidateWiringBox_->isReversed())
        {
            SourcePin* p=candidateWiringBox_->getSrcPin();
            if(p->isConnecting())
            {
                srcPinMouseDrag(e);
            }
            else
            {
                srcPinMouseDisconnectDrag(e);
            }
        }
        else
        {
            DestinationPin* p=candidateWiringBox_->getDstPin();
            if(p->isConnecting())
            {
                dstPinMouseDrag(e);
            }
            else
            {
                dstPinMouseDisconnectDrag(e);
            }
        }
    }
}
    
void MainComponent::moveToolMouseDrag(const MouseEvent& e)
{
    if(candidateMovable_!=0)
    {
        candidateMovable_->mouseDrag(e.getEventRelativeTo(dynamic_cast<Component*>(candidateMovable_)));
    }
}

void MainComponent::mouseUp (const MouseEvent& e)
{
    pic::logmsg()<<"MainComponent mouseUp: mouseMode_="<<mouseMode_<<" tool="<<tm_->getTool();
   	ModifierKeys m=e.mods;
	if (m.isPopupMenu())
	{	
        showTools(e);
        mouseMode_=0;
	}

    else if (e.mouseWasClicked() && (mouseMode_==0||mouseMode_==DRAGZOOM||mouseMode_==DRAGCANVAS ||mouseMode_==LASSO ||mouseMode_==DRAGWIRE))
    {
        doMouseClick(e);
        mouseMode_=0;
    }
    
    else
    {

        if (mouseMode_==DRAGWIRE)
        {
            if(wireDragStarted_)
            {
                endWireDrag(e);
            }
            setMouseCursor(tm_->getMouseCursor());
        }
        else if (mouseMode_==DRAGCANVAS)
        {
            endCanvasDrag(e);
        }
        else if (mouseMode_==DRAGZOOM)
        {
            mouseMode_=0;
            endZoomDrag(e);
        }
        else if(mouseMode_==CREATE_TRUNK)
        {
            endCreateTrunkDrag(e);
        }
        else if(mouseMode_==LASSO)
        {
            lassoComponent_->endLasso();
        }
        else if (tm_->getTool()==ToolManager::WIRINGTOOL)
        {
            wiringToolMouseUp(e);
        }
        else if (tm_->getTool()==ToolManager::WIRING_LEFT)
        {
            wiringLeftMouseUp(e);
        }
        else if (tm_->getTool()==ToolManager::MOVETOOL)
        {
            moveToolMouseUp(e);
        }
    }

    mouseMode_=0;
    endThingMoved();
    doToolChangeIfRequired();
    doMoveToolChangeIfRequired();
}

void MainComponent::wiringToolMouseUp(const MouseEvent& e)
{
    srcPinMouseUp(e);
    candidateWiringBox_=0;

}
void MainComponent::wiringLeftMouseUp(const MouseEvent& e)
{
    dstPinMouseUp(e);
    candidateWiringBox_=0;
}

void MainComponent::moveToolMouseUp(const MouseEvent& e)
{
    pic::logmsg()<<"moveToolMouseUp: toolChangeRequired_= "<<toolChangeRequired_;
    if(candidateMovable_!=0)
    {
        candidateMovable_->mouseUp(e.getEventRelativeTo(dynamic_cast<Component*>(candidateMovable_)));
        candidateMovable_=0;
    }
}

Component* MainComponent::toolChangeComponent()
{
    Point<int> p=getMouseXYRelative();
    Component* cc=getComponentAt(p.getX(),p.getY());
    if(cc==0)
    {
        cc=this;
    }
    return cc;
}

void MainComponent::doToolChangeIfRequired()
{
    if(toolChangeRequired_)
    {
        tm_->setTool(storedTool_,toolChangeComponent());
        storedTool_=0;
        toolChangeRequired_=false;
    }
}

void MainComponent::doMoveToolChangeIfRequired()
{
    if(moveToolChangeRequired_)
    {
        storedTool_=tm_->getTool();
        tm_->setTool(ToolManager::MOVETOOL, toolChangeComponent());
        moveToolChangeRequired_=false;
    }
}

void MainComponent::mouseEnter(const MouseEvent& e)
{
    setMouseCursor(tm_->getMouseCursor());
}

void MainComponent::showTools(const MouseEvent& e)
{
    if (e.mouseWasClicked())
    {
        setMouseCursor(MouseCursor::NormalCursor);
        showPopupMenu(e);
    }	
}

void MainComponent::startCreateTrunkDrag(const MouseEvent& e)
{
    createTrunkReady_=true;
}

void MainComponent::createTrunkDrag(const MouseEvent& e)
{
    if (createTrunkReady_==true)
    {
        pic::logmsg()<<"Trunk Drag Start point = "<<e.x<< ","<<e.y;
        dummyTrunk_=new TrunkDrawingTool(e.x,e.y, zoomFactor_, false);
        createTrunkReady_=false;
    }
    else
    {
        if (inViewPort(e.x,e.y))
        {
            stopAutoDrag();
        }
        else
        {
            autoDrag(e.x,e.y, false);
        }

        dummyTrunk_->setCurrentPoint(e.x,e.y);
        juce::Rectangle <int> u;
        dummyTrunk_->getRectangle(u);
        //std::cout<<"x="<<u.getX()<<" y="<<u.getY()<<" width="<<u.getWidth()<<" height="<<u.getHeight()<<std::endl;
        repaint(u.expanded(20,20));
    }
}


void MainComponent::endCreateTrunkDrag(const MouseEvent& e)
{
    pic::logmsg()<<"endCreateTrunkDrag";
    
    if (dummyTrunk_!=0)
    {
        dummyTrunk_->endDrag(e.x,e.y);
        createTrunkAssembly(dummyTrunk_,model_->generateID(),0,0);
        juce::Rectangle <int> u;
        dummyTrunk_->getRectangle(u);

        delete dummyTrunk_;
        dummyTrunk_=0;
        repaint(u.expanded(20,20));
    }
}

void MainComponent::createTrunkAssembly(TrunkDrawingTool* tdt, String assemblyId,int offset, int cornerOffset)
{
    int n =tdt->getNumSections();
    int lowerBound=tdt->getLowerBound(zoomFactor_);
    if (n>0)
    {
        for (int i =lowerBound;i<n;i++)
        {
            TrunkSection* t=tdt->getSection(i);
            int orientation=t->getOrientation();

            int x=(float)t->getX()/zoomFactor_;
            int y=(float)t->getY()/zoomFactor_;
            int width=(float)t->getWidth()/zoomFactor_;
            int height=(float)t->getHeight()/zoomFactor_;

            int direction=t->getDirection();
            delete t;

            if (orientation==Trunk::HORIZONTAL)
            {
                x=x+10;
                y=y-25;
                width=width-20;
                height=50;
            }
            else
            {
                x=x-25;
                y=y+10;
                width=50;
                height=height-20;
            }

            model_->createTrunk(x,y,width,height,orientation,direction, assemblyId,(n-lowerBound)+offset, (i-lowerBound)+offset);
        }
    }
    
    int numCorners=tdt->getNumCorners();
    for (int i=lowerBound;i<numCorners;i++)
    {
        int x=(float)tdt->getCornerX(i)/zoomFactor_;
        int y=(float)tdt->getCornerY(i)/zoomFactor_;
        int cornerType=tdt->getCornerType(i);
        model_->createTrunkCorner(x,y,assemblyId,(i-lowerBound)+cornerOffset,cornerType);
    }
}

void MainComponent::createReverseTrunkAssembly(TrunkDrawingTool* tdt, String assemblyId,int offset, int cornerOffset)
{
    int n =tdt->getNumSections();
    int lowerBound=tdt->getLowerBound(zoomFactor_);
    if (n>0)
    {
        for (int i =lowerBound;i<n;i++)
        {
            TrunkSection* t=tdt->getSection(i);
            int orientation=t->getOrientation();

            int x=(float)t->getX()/zoomFactor_;
            int y=(float)t->getY()/zoomFactor_;
            int width=(float)t->getWidth()/zoomFactor_;
            int height=(float)t->getHeight()/zoomFactor_;

            int direction=t->getReverseDirection();
            delete t;

            if (orientation==Trunk::HORIZONTAL)
            {
                x=x+10;
                y=y-25;
                width=width-20;
                height=50;
            }
            else
            {
                x=x-25;
                y=y+10;
                width=50;
                height=height-20;
            }

            model_->createTrunk(x,y,width,height,orientation,direction, assemblyId,(n-lowerBound)+offset, ((n-1)-(i-lowerBound)+offset));
        }
    }
    
    for (int i=0;i<tdt->getNumCorners();i++)
    {
        int x=(float)tdt->getCornerX(i)/zoomFactor_;
        int y=(float)tdt->getCornerY(i)/zoomFactor_;
        int cornerType=tdt->getCornerType(i);
        model_->createTrunkCorner(x,y,assemblyId,(tdt->getNumCorners()-1)-(i+cornerOffset),cornerType);
    }
}
void MainComponent::startExtendTrunkMouseDrag(const MouseEvent& e)
{
    pic::logmsg()<<"statExtendMouseDrag";
    createTrunkReady_=true;
}

void MainComponent::extendTrunkMouseDrag(Trunk* t, const MouseEvent& me)
{
    pic::logmsg()<<"extendTrunkMouseDrag";
    MouseEvent e=me.getEventRelativeTo(this);

    if (createTrunkReady_==true)
    {
        int x=(t->getAppendX(me))*zoomFactor_;
        int y=(t->getAppendY(me))*zoomFactor_;;
        dx_=x-e.x;
        dy_=y-e.y;

        pic::logmsg()<<"Trunk Drag Start point = "<<x<< ","<<y;
        pic::logmsg()<<"dx="<<dx_<<" dy="<<dy_;
        dummyTrunk_=new TrunkDrawingTool(x,y, zoomFactor_,true);
        dummyTrunk_->setJoiningCornerParameters(t->getOrientation(),t->getX()+10,t->getY()+10);
        createTrunkReady_=false;
    }
    else
    {
        if (inViewPort(e.x+dx_,e.y+dy_))
        {
            stopAutoDrag();
        }
        else
        {
            autoDrag(e.x+dx_,e.y+dy_, false);
        }

        dummyTrunk_->setCurrentPoint(e.x+dx_,e.y+dy_);
        repaint();
    }

}

void MainComponent::extendTrunkMouseUp(Trunk* t,const MouseEvent& me )
{
    MouseEvent e=me.getEventRelativeTo(this);
    pic::logmsg()<<"extendTrunkMouseUp";
    if (dummyTrunk_!=0)
    {
        dummyTrunk_->endDrag(e.x+dx_,e.y+dy_);
        if(dummyTrunk_->getNumSections()>0)
        {
            appendTrunkAssembly(dummyTrunk_,t);
        }
        delete dummyTrunk_;
        dummyTrunk_=0;
        repaint();
    }
}

void MainComponent::appendTrunkAssembly(TrunkDrawingTool* tdt, Trunk* t)
{
    String assemblyId=t->getAssemblyId();
    int currentMaxIndex=t->getMaxAssemblyIndex();
    int currentIndex=t->getAssemblyIndex();

    if (currentMaxIndex==0) //  special case for one section trunk
    {
        appendToSingleSectionAssembly(tdt,t);
    }
    else
    {
        if (currentIndex==currentMaxIndex)
        {
            appendTrunkAssemblyAtEnd(tdt,t);
        }
        else if (currentIndex==0)
        {
            appendTrunkAssemblyAtStart(tdt, t);
        }
    }
}

void MainComponent::appendToSingleSectionAssembly(TrunkDrawingTool* tdt, Trunk* t)
{
    if(t->getOrientation()==Trunk::HORIZONTAL)
    {
        if (t->getDirection()==Trunk::LEFT_TO_RIGHT)
         {
             if (tdt->getJoiningCornerPosX()<t->getMidX())
             {
                  appendTrunkAssemblyAtStart(tdt,t);
             }
             else
             {
                  appendTrunkAssemblyAtEnd(tdt,t);
             }
         }
         else if (t->getDirection()==Trunk::RIGHT_TO_LEFT)
         {
             if (tdt->getJoiningCornerPosX()<t->getMidX())
             {
                  appendTrunkAssemblyAtEnd(tdt,t);
             }
             else
             {
                  appendTrunkAssemblyAtStart(tdt,t);
             }
         }
     }
    else if(t->getOrientation()==Trunk::VERTICAL)
    {
         if (t->getDirection()==Trunk::TOP_TO_BOTTOM)
         {
             if (tdt->getJoiningCornerPosY()<t->getMidY())
             {
                  appendTrunkAssemblyAtStart(tdt,t);
             }
             else
             {
                  appendTrunkAssemblyAtEnd(tdt,t);
             }
         }
         else if (t->getDirection()==Trunk::BOTTOM_TO_TOP)
         {
             if (tdt->getJoiningCornerPosY()<t->getMidY())
             {
                  appendTrunkAssemblyAtEnd(tdt,t);
             }
             else
             {
                  appendTrunkAssemblyAtStart(tdt,t);
             }
         }
    }

}

void MainComponent::appendTrunkAssemblyAtEnd(TrunkDrawingTool* tdt, Trunk* t)
{
    String assemblyId=t->getAssemblyId();
    int currentNumCorners=getNumCorners(assemblyId);
    int offset=t->getAssemblySize();
    updateAssemblySize(assemblyId, tdt->getNumSections());
    int x=(float)tdt->getJoiningCornerPosX()/zoomFactor_;
    int y=(float)tdt->getJoiningCornerPosY()/zoomFactor_;
    int cornerType=tdt->getJoiningCornerType(t->getOrientation(),t->getX()+10, t->getY()+10);
    createTrunkAssembly(tdt, assemblyId,offset,currentNumCorners+1);
    model_->createTrunkCorner(x,y,assemblyId,currentNumCorners,cornerType);
}

void MainComponent::appendTrunkAssemblyAtStart(TrunkDrawingTool* tdt, Trunk* t)
{
    String assemblyId=t->getAssemblyId();
    updateAssemblySize(assemblyId, tdt->getNumSections());
    updateAssemblyIndices(assemblyId,tdt->getNumSections());
    int offset=0;
    createReverseTrunkAssembly(tdt, assemblyId,offset,offset);
    int x=(float)tdt->getJoiningCornerPosX()/zoomFactor_;
    int y=(float)tdt->getJoiningCornerPosY()/zoomFactor_;
    int cornerType=tdt->getJoiningCornerType(t->getOrientation(),t->getX()+10, t->getY()+10);
    model_->createTrunkCorner(x,y,assemblyId,tdt->getNumCorners(),cornerType);
}

int MainComponent::getNumCorners(String assemblyId)
{
    int count=0;
    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;
        if (tc->getAssemblyId()==assemblyId)
        {
            count++;
        }
    }
    pic::logmsg()<<"TrunkAssembly "<<std::string(assemblyId.toUTF8())<<" has "<<count<<" corners";
    return count;
}

void MainComponent::updateAssemblySize(String assemblyId, int deltaSize)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;
        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                int size=t->getProps()->get_number("assemblySize");
                t->getProps()->set_number("assemblySize",size+deltaSize);
                pic::logmsg()<<"updating assemblySize to "<<size+deltaSize;
                t->save_props();
            }
        }
    }
}

void MainComponent::updateAssemblyIndices(String assemblyId, int deltaSize)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;
        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                int index=t->getProps()->get_number("assemblyIndex");
                t->getProps()->set_number("assemblyIndex",index+deltaSize);
                pic::logmsg()<<"updating assemblyIndex to "<<index+deltaSize;
                t->save_props();
            }
        }
    }

    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;
        if (tc->getAssemblyId()==assemblyId)
        {
           PropertyStore* props=tc->getProps();
           int cIndex=props->get_number("assemblyIndex");
           props->set_number("assemblyIndex",cIndex+deltaSize);
        }
    }
}

void MainComponent::mouseOverAssembly(String assemblyId, bool over)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;
        if (t!=0)
        {
            if(t->getProps()->get_string("assemblyId")==assemblyId)
            {
                t->doSetMouseOver(over);
            }
        }
    }

    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;
        if (tc->getAssemblyId()==assemblyId)
        {
           tc->doSetMouseOver(over);
        }
    }
}

void MainComponent::lassoAssembly(String assemblyId, bool lassoed)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;
        if (t!=0)
        {
            if(t->getAssemblyId()==assemblyId)
            {
                t->doSetLassoed(lassoed);
            }
        }
    }

    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;
        if (tc->getAssemblyId()==assemblyId)
        {
           tc->doSetLassoed(lassoed);
        }
    }
}

int MainComponent::getTool()
{
    return tm_->getTool();
}

void MainComponent::setTool(int tool)
{
    tm_->setTool(tool,0);
}


void MainComponent::doMouseClick(const MouseEvent& e)
{
    pic::logmsg()<<"doMouseClick: mouseMode= "<<mouseMode_<<" tool="<<tm_->getTool();
    switch (tm_->getTool())
    {
        case ToolManager::CREATETOOL:
           doCreate(e);
           break;

        case ToolManager::ZOOMTOOL:
           if (e.mods.isShiftDown())
           {
               doZoomInc(-0.1f,e.x,e.y,true);
           }
           else
           {
               doZoomInc(0.1f,e.x,e.y,true);
           }
           break; 

        case ToolManager::DELETETOOL:

            if (e.mods.isShiftDown())
            {
                cycleSelectedWire(e);
            }
            else
            { 
                deleteToolMouseClick(e);
            }
            break; 

        case ToolManager::HOOKTOOL:
            createHook(e.x,e.y);
            break;

        case ToolManager::EDITTOOL:
            if(selectedWire_!=0)
            {
                if (e.mods.isShiftDown())
                {
                    cycleSelectedWire(e);
                }
                else
                {
                    showWireProperties(selectedWire_, e.x, e.y);
                }
            }
            break;

        case ToolManager::POINTERTOOL:
            if(mouseMode_==DRAGCANVAS)
            {
                endCanvasDrag(e);            
            }

            if (e.mods.isShiftDown())
            {
                cycleSelectedWire(e);
            }
            clearSelectedItems();
            pointerToolMouseClick(e);
            break;

        case ToolManager::SELECTTOOL:
            clearSelectedItems();
            break;

        case ToolManager::WIRINGTOOL:
            if(candidateWiringBox_!=0)
            {
                candidateWiringBox_->getSrcPin()->doWiringToolMouseClick();
            }
            else
            {
                if (e.mods.isShiftDown())
                {
                    cycleSelectedWire(e);
                }
            }
            break;

        case ToolManager::WIRING_LEFT:
            if(candidateWiringBox_!=0)
            {
                candidateWiringBox_->getDstPin()->doWiringToolMouseClick();
            }
            else
            {
                if (e.mods.isShiftDown())
                {
                    cycleSelectedWire(e);
                }
            }
            break;

        // XXX for testing
        //case ToolManager::HELPTOOL:
        //    showOutlines(!showOutlines_);
            //model_->get_test_string(130000);
        //    break;

        case ToolManager::MOVETOOL:
            if(mouseMode_==DRAGCANVAS)
            {
                endCanvasDrag(e);
            }
            else if(mouseMode_==DRAGWIRE)
            {
                if (e.mods.isShiftDown())
                {
                    cycleSelectedWire(e);
                }
                setMouseCursor(tm_->getMouseCursor());
            }
            break;
    }
}

void MainComponent::pointerToolMouseClick(const MouseEvent& e)
{
    //if(isWireAt(e.x,e.y))
    if(selectedWire_!=0)
    {
        if (selectedWire_->isRevConnect())
        {
            pic::logmsg()<<selectedWire_->getId()<<" is reverse connection ";
        }   

        if(e.mods.isCommandDown())
        {
            findUpstream();
        }
        else
        {
            findDownstream();
        }
    }
}

void MainComponent::findDownstream()
{
    if(selectedWire_!=0)
    {
        String dstId=selectedWire_->getTopLevelDstId();
        Box* b = getBoxById(dstId);
        if(b!=0)
        {
            selectOnly(b);
            //if(!inViewPort(b->getX(),b->getY()))
            //{
                setViewPosition(b->getX()-0.5*vp_->getWidth(),b->getY()-0.5*vp_->getHeight(), true);
            //}
        }

    }
}

void MainComponent::findUpstream()
{
    if(selectedWire_!=0)
    {
        String srcId=selectedWire_->getTopLevelSrcId();
        Box* b = getBoxById(srcId);
        if(b!=0)
        {
            selectOnly(b);
            //if(!inViewPort(b->getX(),b->getY()))
            //{
                setViewPosition(b->getX()-0.5*vp_->getWidth(),b->getY()-0.5*vp_->getHeight(), true);
            //}
        }
    }
}

void MainComponent::cycleSelectedWire(const MouseEvent& e)
{
    if(selectedWire_!=0 && wiresAt_.size()>0)
    {
        unsigned max=wiresAt_.size();
        unsigned count=0;
        int route=0;
        int currentRoute=selectedWire_->getRouteHash();
        int segmentHash=selectedWire_->getSegmentHashAt(e.x,e.y);

        while(count<max)
        {
            wireIndex_++;
            if(wireIndex_>=wiresAt_.size())
            {
                wireIndex_=0;
            }
            count++;
            route=wiresAt_[wireIndex_]->getRouteHash();
            pic::logmsg()<<"wireIndex_="<<wireIndex_<<" route="<<route<<" count="<<count;
            if(route!=currentRoute)
            {
               if(wiresAt_[wireIndex_]->hasSegment(segmentHash)) 
               {
                    selectWire(wiresAt_[wireIndex_]);
                    createTooltip(selectedWire_,segmentHash);
                    break;
               }
            }
        }

        // XXX repaint rect defined by selectedWire and previously selected wire
        repaint();
    }
}

void MainComponent::showWorkspace(String tabName, String scope, String abs_id)
{
    pic::logmsg()<<"mainComponent showWorkspace tabname="<<tabName<<" scope="<<scope;
    wsf_->showWorkspace(tabName,scope,abs_id);
}

void MainComponent::deleteToolMouseClick(const MouseEvent& e)
{
    if(selectedWire_!=0)
    {
        Wire* w=selectedWire_;
        if (e.mods.isCommandDown())
        {
            bool doDelete=true;
            if(requiresDeleteConfirmation("Route"))
            {
                Anchor* anchor=new Anchor(e.x,e.y);
                addChildComponent(anchor);

                std::vector<Wire*>wires=getSameRouteWires(w);
                wires.push_back(w); 
                std::vector<String> wireNames;
                for(std::vector<Wire*>::iterator i=wires.begin();i!=wires.end();i++)
                {
                    wireNames.push_back((*i)->getDescription());    
                }


                DeleteRouteConfirmation* da=new DeleteRouteConfirmation(wireNames);
                DialogWindow::showModalDialog("Delete route infomation",da,anchor,Colour(0xffababab),true);
                if(da->dontShowAgain_&& da->okPressed_)
                {
                   setDeleteConfirmationRequired("Route"); 
                }

                doDelete=da->okPressed_;
                delete da;
                removeChildComponent(anchor);
                delete anchor;
            }

            if(doDelete)
            {
              unRouteWiresLike(w);
              repaint();
            }
        }

        else
        {
            bool doDelete=true;
            if(requiresDeleteConfirmation("Wire"))
            {
                Anchor* anchor=new Anchor(e.x,e.y);
                addChildComponent(anchor);

                std::vector<Wire*>wires=getSameRouteWires(w);
                wires.push_back(w); 
                std::vector<String> wireNames;
                for(std::vector<Wire*>::iterator i=wires.begin();i!=wires.end();i++)
                {
                    if((*i)->isLoose())
                    {
                       wireNames.push_back(getLooseWireDesc(*i)); 
                    }
                    else
                    {
                        wireNames.push_back((*i)->getDescription());    
                    }
                }

                DeleteWireConfirmation* da=new DeleteWireConfirmation(wireNames);
                DialogWindow::showModalDialog("Delete wire",da,anchor,Colour(0xffababab),true);
                if(da->dontShowAgain_&& da->okPressed_)
                {
                   setDeleteConfirmationRequired("Wire"); 
                }

                doDelete=da->okPressed_;
                delete da;
                removeChildComponent(anchor);
                delete anchor;
            }

            if(doDelete)
            {

              deleteWiresLike(w);
            }
        }
    }
    else
    {
        // XXX testing
        checkPendingWires();
        for (std::map<String,Wire*>::iterator iter=pendingWires_.begin();iter!=pendingWires_.end();iter++)
        {
            pic::logmsg()<<"         " <<std::string((iter->first).toUTF8());
        }
        repaint();
    }
}

void MainComponent::showWireProperties(Wire* w, int x, int y)
{
    pic::logmsg()<<"show wire properties ";
    std::vector<Wire*>wires=getSameRouteWires(w);
    wires.push_back(w); 

    WirePropertyPanel* wpp=new WirePropertyPanel(wires);
    DialogComponent* dc = new DialogComponent(wpp,1); 

    Anchor* anchor=new Anchor(x,y);
    addChildComponent(anchor);


    int h=getDialogHeight(wpp);
    dc->setSize(600,h);
    DialogWindow::showModalDialog("Connection details",dc,anchor,Colour (0xffababab),true);

    if(dc->okPressed())
    {
        std::vector<WireEditor*> editors=wpp->getEditors();
        for(std::vector<WireEditor*>::iterator i=editors.begin();i!=editors.end();i++)
        {
            WireEditor* ed=*i;
            if(ed->changed())
            {
               DestinationPin* dp=w->getUsingDP();
               if(dp!=0)
               {
                   Box* b=dp->findParentComponentOfClass<Box>();
                   if(b!=0)
                   {
                        Box* pb =dynamic_cast <Box *>(b->getParentComponent());
                        if(pb!=0)
                        {
                            pb->setUsingChanged(true);
                        }
                   }
               }
             
               changeWire(ed->getWire(),ed->getUsing(),ed->getFilter(),ed->getControl());
            }
            else
            {
                pic::logmsg()<<"wire editor not changed";
            }
        }
    }

    delete dc;
    removeChildComponent(anchor);
    delete anchor;
}

int MainComponent::getDialogHeight(Component* displayComponent)
{
    int h=displayComponent->getHeight();
    int heightLimit=600;
    int monitorHeight=getParentMonitorArea().getHeight()-60;
    if( heightLimit>monitorHeight)
    {
        heightLimit=monitorHeight;
    }

    if(h>heightLimit)
    {
        h=heightLimit;
    }
    else
    {
        h=h+70;
    }
//    pic::logmsg()<<"getDialogHeight h="<<h<<" original height="<<displayComponent->getHeight();
    return h;
}

void MainComponent::showProperties(Box* b)
{
    if(b!=0)
    {
        Atom* atom=b->getAtom();
        if(atom!=0)
        {
            PropertyEditor* p=new PropertyEditor(new Atom(*atom),tm_, model_->getValueMonitor(),b->isRig());
            dc_=new DialogComponent(p,2);
            dc_->setId(atom->get_id());
            dc_->setSize(dc_->getWidth(),getDialogHeight(p));
            DialogWindow::showModalDialog(atom->get_fulldesc(),dc_,b,Colour (0xffababab),true);
            model_->getValueMonitor()->removeListener();
            model_->getValueMonitor()->clear();
            delete p;
            delete dc_;
            dc_=0;
        }
        else
        {
            pic::logmsg()<<"showProperties:atom=0";
        }
    }
    else
    {
        pic::logmsg()<<"showProperties:box=0";
    }
}

void MainComponent::refreshProperties(String id)
{
    if(dc_!=0)
    {
        String dcId=dc_->getId();
        if(id.contains(dcId))
        {

           PropertyEditor* p_old=dynamic_cast<PropertyEditor*>(dc_->getViewedComponent());
           p_old->refresh();
           Atom* atom=p_old->getAtom();
           if(atom!=0)
           {
               DialogWindow* dw=dc_->findParentComponentOfClass<DialogWindow>();
               if(dw!=0)
               {
                   dw->setName(atom->get_fulldesc());
               }
           }
        }
        else
        {
           pic::logmsg()<<"id did not contain dcID - propertyEditor not refreshed";
        }

    }
}

void MainComponent::refreshProperties(Box* b)
{
    if(dc_!=0)
    {
        String dcId=dc_->getId();
        String id=b->getId();
        if(dcId.contains(id))
        {
           PropertyEditor* p_old=dynamic_cast<PropertyEditor*>(dc_->getViewedComponent());
           p_old->refresh();
        }
    }
}

void MainComponent::deleteWiresLike(Wire* w)
{
    if(w!=0)
    {
        std::vector<Wire*> v=getSameRouteWires(w);
        while(!v.empty())
        {
            Wire* wi=v.back();
            v.pop_back();
            deleteWire(wi);
        }
        deleteWire(w);
    }
}

void MainComponent::unRouteWiresLike(Wire* w)
{
    if(w!=0)
    {
        std::vector<Wire*> v=getSameRouteWires(w);
        while(!v.empty())
        {
            Wire* wi=v.back();
            v.pop_back();
            pic::logmsg()<<"unRoute wire "<<std::string(wi->getId().toUTF8());
            wi->removeAllPegs();
        }

        pic::logmsg()<<"unRoute wire "<<std::string(w->getId().toUTF8());
        w->removeAllPegs();
    }
}

void MainComponent::deleteConnectionsLike(Wire* w)
{
    if(w!=0)
    {
      int h=w->getRouteHash();
      for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
      {
           Wire* wi=iter->second;
           if(wi->getRouteHash()==h)
           {
                model_->delete_connection(wi->getId(),wi->get_dstUsing(),wi->get_srcFilter(),wi->get_control());
           }
      }
    }
}

void MainComponent::doCreate(const MouseEvent& e)
{
      model_->getAgentNames();
      createDialogX_=e.x;
      createDialogY_=e.y;
}

void MainComponent::createInstance(Atom* atom, String name)
{
    instanceParent_=atom;
    instanceName_=name;
    pic::logmsg()<<"MainComponent createInstance name="<<std::string(instanceName_.toUTF8());
    instanceParent_->get_instances();
}

void MainComponent::deleteInstance(Atom* atom)
{
    Atom* instanceParent=atom->get_parent();
    instanceParent->delete_instance(atom->get_id()); 
    delete instanceParent;
}

void MainComponent::instancesUpdated(const std::set<std::string> & ords)
{
    showCreateInstanceDialog(instanceParent_,ords);
}

void MainComponent::report_error(String err1, String err2)
{
    ErrorReportComponent* e=new ErrorReportComponent(err1, err2);

    DialogWindow::showModalDialog(String("Error"),e,getTopLevelComponent(),Colour (0xffababab),true);
    delete e; 
}

void MainComponent::showCreateInstanceDialog(Atom* parent,const std::set<std::string> & ords) 
{
    CreateInstanceComponent* ct =new CreateInstanceComponent(ords);
    Box* b = getBoxById(parent->get_id());

    DialogWindow::showModalDialog(String("Create new ")+instanceName_,ct,b,Colour (0xffababab),true);
    if(ct->okPressed_)
    {
        int ordinal=ct->getOrdinal();
        if(parent!=0)
        {
            parent->create_instance(ordinal);
        }
    }
    delete ct;
}

void MainComponent::agentsUpdated(const std::set<std::string> & agents)
{
    showCreateDialog(agents);
}

void MainComponent::cinfo_updated(String id, String path,const std::set<std::string> & cinfo)
{
    pic::logmsg()<<"MainComponent cinfo_updated for "<< std::string(id.toUTF8());
    if(dc_!=0)
    {
          PropertyEditor* p=dynamic_cast<PropertyEditor*>(dc_->getViewedComponent());
          if(p!=0)
          {
               p->cinfo_updated(id,path,cinfo);
          }
    }
}

void MainComponent::finfo_updated(String id, String path,const std::set<std::string> & finfo)
{
    pic::logmsg()<<"MainComponent finfo_updated for "<< std::string(id.toUTF8());
    if(dc_!=0)
    {
          PropertyEditor* p=dynamic_cast<PropertyEditor*>(dc_->getViewedComponent());
          if(p!=0)
          {
               p->finfo_updated(id,path,finfo);
          }
    }
}

void MainComponent::enumerate_updated(String id,String path, int nf, int nc)
{
    pic::logmsg()<<"MainComponent enumerate_updated for "<< std::string(id.toUTF8())<<" nf="<<nf<<" nc="<<nc;
    if(dc_!=0)
    {
          PropertyEditor* p=dynamic_cast<PropertyEditor*>(dc_->getViewedComponent());
          if(p!=0)
          {
               p->enumerate_updated(id,path,nf,nc);
          }
    }
}

void MainComponent::sourcekeys_updated(String id, String keys)
{
    if(dc_!=0)
    {
          PropertyEditor* p=dynamic_cast<PropertyEditor*>(dc_->getViewedComponent());
          if(p!=0)
          {
               p->sourcekeys_updated(id, keys);
          }
    }
}


void MainComponent::activated(String id)
{
    pic::logmsg()<<"MainComponent activated for "<< std::string(id.toUTF8());
    if(dc_!=0)
    {
          PropertyEditor* p=dynamic_cast<PropertyEditor*>(dc_->getViewedComponent());
          if(p!=0)
          {
               p->activated(id);
          }
    }
}

void MainComponent::current(String id, String cookie)
{
    pic::logmsg()<<"MainComponent current for "<< std::string(id.toUTF8());
    if(dc_!=0)
    {
          PropertyEditor* p=dynamic_cast<PropertyEditor*>(dc_->getViewedComponent());
          if(p!=0)
          {
               p->current(id,cookie);
          }
    }
}
void MainComponent::showCreateDialog(const std::set<std::string>& agentNames)
{
    CreateAgentComponent* ct=new CreateAgentComponent(agentNames, model_);
    if (currentCreateAgent_.isNotEmpty())
    {
        ct->setSelection(currentCreateAgent_);
    }
    Anchor* anchor=new Anchor(createDialogX_,createDialogY_);
    pic::logmsg()<<"createDialogX_="<<createDialogX_<<"   createDialogY_="<<createDialogY_;
    addChildComponent(anchor);

	DialogWindow::showModalDialog("Create an agent",ct,anchor,Colour (0xffababab),true);
    if(ct->okPressed_)
    {
        currentCreateAgent_=ct->getSelection();
        int ordinal=ct->getOrdinal();
        createAgentBox(createDialogX_,createDialogY_,currentCreateAgent_,ordinal); 
    }
    delete ct;
    removeChildComponent(anchor);
    delete anchor;
}

void MainComponent::showFindDialog()
{
    FindComponent* fc=new FindComponent(getTopLevelBoxNames());

    ProgressLayer* pl=findParentComponentOfClass<ProgressLayer>();
    DialogWindow::showModalDialog("Find",fc,pl,Colour(0xffababab),true);

    if(fc->okPressed_)
    {
        pic::logmsg()<< "highlight agent box with name"<<std::string(fc->getSelection().toUTF8());
        Box* b = getBoxByName(fc->getSelection());
        if(b!=0)
        {
            selectOnly(b);
            if(!inViewPort(b->getX(),b->getY()))
            {
                setViewPosition(b->getX()-0.5*vp_->getWidth(),b->getY()-0.5*vp_->getHeight(), true);
            }
        }
    }

    delete fc;
}

void MainComponent::showPreferencesDialog()
{
    PreferenceComponent* fc=new PreferenceComponent(this);
    DialogWindow::showModalDialog("Preferences",fc,vp_,Colour(0xffababab),true);

    if(fc->okPressed)
    {
        pic::logmsg()<< "Preferences ok pressed";
        setProperty("selectOnExpand",fc->getValue("selectOnExpand"));
        setProperty("enableMouseWheelZoom",fc->getValue("enableMouseWheelZoom"));
    }

    delete fc;
}

void MainComponent::mouseDoubleClick (const MouseEvent& e)
{
}

void MainComponent::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails &wheel)
{
    // XXX if shift id dowm pan

    if(getProperty("enableMouseWheelZoom",true))
    {
        startTimer(2,330);
        mouseWheeling_=true;

        //   XXX temporary use of X - should change to y
        //float zoomInc=wheel.deltaX*0.15f;
        float zoomInc=wheel.deltaY*0.15f;

        zoomTot_=zoomTot_+zoomInc;
        //pic::logmsg()<<"MouseWheelMove x= "<< e.x << " y= " << e.y <<" wheelIncrement="<<wheel.deltaX <<" zoomInc="<<zoomInc<<" zoomTot="<<zoomTot_<<" zoomFactor="<<zoomFactor_;
        if(zoomTot_>0.025)
        {
            stopTimer(4);
            doZoomInc(zoomTot_,e.x,e.y,false);
            zoomTot_=0.0f;
            startTimer(4,300);
        }
        else if (zoomTot_<-0.025)
        {
            stopTimer(4);
            doZoomInc(zoomTot_,e.x,e.y,false);
            zoomTot_=0.0f;
            startTimer(4,300);
        }
    }
}

void MainComponent::doZoom(float zoomFactor)
{
    doZoom(zoomFactor,0.5*getWidth(),0.5*getHeight()); 
}

void MainComponent::doZoom(float zoomFactor, int x, int y)
{
   if (zoomFactor<0.1f)
    {
        zoomFactor=0.1f;
    }
    else if (zoomFactor>1.0f)
    {
        zoomFactor=1.0f;
    }

    pic::logmsg()<<"doZoom: zoomFactor="<<zoomFactor;
    float oldZoomFactor=zoomFactor_;
  
    if (zoomFactor!=oldZoomFactor)
    {
// zoom to supplied x,y
        int offsetX=x-vp_->getViewPositionX();
        int offsetY=y-vp_->getViewPositionY();
        double m=zoomFactor/oldZoomFactor;

        int xdash=(double)x*m;
        int ydash=(double)y*m;
//            pic::logmsg()<<"x="<<x<<" xdash="<<xdash<<" m="<<m;
//            pic::logmsg()<<"y="<<y<<" ydash="<<ydash<<" m="<<m;

//  can't do this - doesnt allow zoom out at edge of canvas - probably need bufferzone
//            if(xdash-offsetX>=0 && ydash-offsetY>=0)
//            {
            zoomFactor_=zoomFactor;
            zoomChildren();

//                pic::logmsg()<<"new canvas size "<<trueWidth_*zoomFactor_<< " " << trueHeight_*zoomFactor_;
            setSize(floor(((float)trueWidth_*zoomFactor_)+0.5), floor(((float)trueHeight_*zoomFactor_)+0.5));
            displayLayer_->setBounds(0,0,getWidth(),getHeight());

            setViewPosition(xdash-offsetX,ydash-offsetY,false);
//                pic::logmsg()<<"setViewPosition "<<xdash-offsetX<<" "<<ydash-offsetY;
            repaint();
//            }

    }
    else
    {
        pic::logmsg()<<"min zoomfactor";
    }
}


void MainComponent::doZoomInc(float zoomInc, int x, int y, bool save)
{
//    pic::logmsg()<<"MainComponent doZoom x="<<x<<" y="<<y;

    doZoom(zoomFactor_+zoomInc, x,y);
//    pic::logmsg()<<"MainComponent doZoomInc: zoomFactor set to "<<zoomFactor_;
    if(save)
    {
        setWorkspaceProps();
    }
 }

void MainComponent::setWorkspaceProps()
{
    int offsetX=vp_->getViewPositionX();
    int offsetY=vp_->getViewPositionY();
    if(props_!=0)
    {
        pic::logmsg()<<"set in existing props "<<props_;
        props_->set_number("zoomfactor",zoomFactor_);
        props_->set_number("viewoffsetx",offsetX);
        props_->set_number("viewoffsety",offsetY);
        save_props(props_->get_string("id"));
    }
    else
    {
        props_=model_->create_store("Workspace","root");
        pic::logmsg()<<"create in new props "<<props_;
        props_->set_number("zoomfactor",zoomFactor_);
        props_->set_number("viewoffsetx",offsetX);
        props_->set_number("viewoffsety",offsetY);
        model_->init_store(props_);
        save_props(props_->get_string("id"));
    }
}

void MainComponent::zoomChildren()
{
    for (std::map<String,Box*>::iterator iter=boxes_.begin();iter!=boxes_.end();iter++)
    {
        Box* b=iter->second;
        if (b!=0)
        {
            b->doZoom(zoomFactor_);
        }
    }

    for (std::map<String,Peg*>::iterator iter=pegs_.begin();iter!=pegs_.end();iter++)
    {
        Peg* p=iter->second;
        if (p!=0)
        {
            p->doZoom(zoomFactor_);
        }
    }

    for (std::map<String,SourcePin*>::iterator iter=loose_sps_.begin();iter!=loose_sps_.end();iter++)
    {
        SourcePin* sp=iter->second;
        if (sp!=0)
        {
            sp->doZoom(zoomFactor_);
        }
    }

    for (std::map<String,DestinationPin*>::iterator iter=loose_dps_.begin();iter!=loose_dps_.end();iter++)
    {
        DestinationPin* dp=iter->second;
        if (dp!=0)
        {
            dp->doZoom(zoomFactor_);
        }
    }


    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;
        if (t!=0)
        {
            t->doZoom(zoomFactor_);
        }
    }

    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* tc=iter->second;
        if (tc!=0)
        {
            tc->doZoom(zoomFactor_);
        }
    }

    if (dragPin_!=0)
    {
        dragPin_->doZoom(zoomFactor_);
    }
}

float MainComponent::getZoomFactor()
{
    return zoomFactor_;
}

void MainComponent::changeWire(Wire* w, String u, String f, String c)
{
    changeWires_.insert(std::pair<String,ChangeWireProperties*>(w->getId(),new ChangeWireProperties(w->getId(),w->getSrcId(),w->getDstId(),u,f,c)));
    std::list<StoredRouteElement> route;
    storeRoute(w,route);
    saveRoute(w->getId(),route);
    deleteWire(w);
}

void MainComponent::changeWirePart2(String wireId)
{
     std::map<String, ChangeWireProperties*>::iterator i=changeWires_.find(wireId);
     if(i!=changeWires_.end())
     {
        ChangeWireProperties* cw=i->second;
        create_connection(cw);
        changeWires_.erase(i);
        delete cw;
     }
}

void MainComponent::looseConnectionRemoved(Wire* w)
{
    if(w->isLoose())
    {
        if (selectedWire_==w)
        {
            w->setSelected(false);
            selectedWire_=0;
        }

        SourcePin* srcPin=w->getSrcPin();
        DestinationPin* dstPin=w->getDstPin();
        if(!dstPin->isLoosePin())
        {
            updateDstBox(w);
        }

        removeCachedWire(w);

        if(srcPin->isLoosePin())
        {
            removeLoosePin(srcPin);
        }
        if(dstPin->isLoosePin())
        {
            removeLoosePin(dstPin);
        }

        // XXX needs area repaint
        //  need to deal with selectedWire also
        repaint();
    }
}

void MainComponent::deleteWire(Wire* w)
{
    if(w->isLoose())
    {
        looseConnectionRemoved(w);
    }
    else
    {
        pic::logmsg()<<"MainComponent deleteWire "<<std::string(w->getId().toUTF8())<<" control="<<std::string(w->get_control().toUTF8());
        model_->delete_connection(w->getId(),w->get_dstUsing(),w->get_srcFilter(),w->get_control());
    }
}

void  MainComponent::removeLoosePins(Wire* w)
{
    if(w->isLoose())
    {
        SourcePin* sp=w->getSrcPin();
        DestinationPin* dp=w->getDstPin();
        if (dp!=0)
        {
            if( dp->isLoosePin())
            {
                int n=loose_dps_.erase(dp->getId());
                pic::logmsg() <<"erased "<<n<<" loose destination pins";
                PropertyStore* props=dp->getProps();
                model_->remove_store(props);
                removeChildComponent(dp);
            }
        }

        if(sp!=0)
        {
            if(sp->isLoosePin())
            {
                int n=loose_sps_.erase(sp->getId());
                pic::logmsg()<<"erased "<<n<<" loose source pins";
                PropertyStore* props=sp->getProps();
                model_->remove_store(props);
                removeChildComponent(sp);
            }
        }
    }
}

void MainComponent::showPopupMenu(const MouseEvent& e)
{
    int result=tm_->getToolsMenu().show();
    if (result!=0)
    {
        tm_->setTool(result, toolChangeComponent());
    }
    else
    {
        setMouseCursor(tm_->getMouseCursor());
    }
}

void MainComponent::listWiresAt(int x, int y)
{
    int tol=5;
    bool foregroundOnly=hasForegroundedItem();
    bool showWire=true;
    std::vector<Wire*> revwires;
    wiresAt_.clear();
    int gs=getGridNumber(((float)x)/zoomFactor_,((float)y)/zoomFactor_);
    std::map<int,std::list<Wire*> >::iterator i=gridSquareMap_.find(gs);
    if(i!=gridSquareMap_.end())
    {
       std::list<Wire*> wires=i->second;

       for(std::list<Wire*>::iterator iter=wires.begin();iter!=wires.end();iter++)
       {
            Wire* w=*iter;
            if (foregroundOnly)
            {
                showWire=w->isForegrounded();
            }
            if((!w->isHidden()) && showWire)
            {
                Path p=w->getWirePath();

                if(p.intersectsLine(Line<float>(x-tol,y,x+tol,y)))
                {
                    if(w->isRevConnect())
                    {
                        revwires.push_back(w);
                    }
                    else
                    {
                        wiresAt_.push_back(w);
                    }
                }
                else if(p.intersectsLine(Line<float>(x,y-tol,x,y+tol)))
                {
                    if(w->isRevConnect())
                    {
                        revwires.push_back(w);
                    }
                    else
                    {
                        wiresAt_.push_back(w);
                    }

                }
            }
       }
    }

    for(std::vector<Wire*>::iterator iter=revwires.begin();iter!=revwires.end();iter++)
    {
        wiresAt_.push_back(*iter);
    }

}

bool MainComponent::wireIsAt(Wire* targetWire, int x, int y)
{
    if(targetWire==0)
    {
        return false;
    }

    for(std::vector<Wire*>::iterator iter=wiresAt_.begin();iter!=wiresAt_.end();iter++)
    {
        if((*iter)==targetWire)
        {
            return true;
        }
    }
    
    return false;
}

void MainComponent::selectWire(Wire* w)
{
    if (selectedWire_!=0)
    {
        selectedWire_->setSelected(false);
        selectedWire_=0;
    }
    if(w!=0)
    {
        w->setSelected(true);
        selectedWire_=w;
    }
}

Box* const MainComponent::getBoxAt(int x, int y)
{
    Box* b =getBoxExactlyAt(x,y);
    if(b==0)
    {
        return getBoxNear(x,y,20,5);
    }
    else
    {
        return b;
    }
}

Box* const MainComponent::getBoxNear(int x, int y, int tol, int step)
{
    int t=step;
    Box* b;
    
    while(t<=tol)
    {
        b=getBoxExactlyAt(x+t,y);     
        if(b!=0)
        {
            return b;
        }

        b=getBoxExactlyAt(x+(0.7*t),y-(0.7*t));     
        if(b!=0)
        {
            return b;
        }


        b=getBoxExactlyAt(x,y-t);     
        if(b!=0)
        {
            return b;
        }

        b=getBoxExactlyAt(x-(0.7*t),y-(0.7*t));     
        if(b!=0)
        {
            return b;
        }

        b=getBoxExactlyAt(x-t,y);     
        if(b!=0)
        {
            return b;
        }

        b=getBoxExactlyAt(x-(0.7*t),y+(0.7*t));     
        if(b!=0)
        {
            return b;
        }


        b=getBoxExactlyAt(x,y+t);     
        if(b!=0)
        {
            return b;
        }

        b=getBoxExactlyAt(x+(0.7*t),y+(0.7*t));     
        if(b!=0)
        {
            return b;
        }

        t=t+step;
    }
    return 0;
}

Box* const MainComponent::getBoxExactlyAt(int x, int y)
{
    for (std::map<String,Box*>::iterator iter=boxes_.begin();iter!=boxes_.end();iter++)
        {
            Box* b=iter->second;
            if (b!=0 && b->isTopLevel())
            {
                if (b->contains(juce::Point<int> (x-b->getX(),y-b->getY())))
                {
                    //pic::logmsg()<<"getBoxExactlyAt x="<<x<<" y="<<y<<" box name="<<b->getName().toUTF8()<<" getX="<<b->getX()<<" getY="<<b->getY();
                    return b->getBoxAt(x-b->getX(),y-b->getY());
                }
            }
        }
    return 0;
}



std::vector<Wire*> MainComponent::getSameRouteWires(Wire * w)
{
    std::vector<Wire*> v;
    for (std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
    {
        Wire* w2=iter->second;
        if(w2!=w)
        {
            if(w->getRouteHash()==w2->getRouteHash())
            {
                v.push_back(w2);
            }
        }
    }

    return v;
}

void MainComponent::startWireDrag(const MouseEvent& e)
{
    pic::logmsg()<<"startWireDrag";
    wireDragStarted_=true;
    dragWires_.clear();
    overElement_=0;
    dragWires_=getSameRouteWires(selectedWire_);
    pic::logmsg()<<"start wire drag "<<std::string(selectedWire_->getId().toUTF8())<<": route shared by: ";
    for(std::vector<Wire*>::iterator i=dragWires_.begin();i!=dragWires_.end();i++)
    {
        pic::logmsg()<<"    "<<std::string((*i)->getId().toUTF8());
    }

    Wire* w =selectedWire_;

    if (w!=0)
    {
        paintOverMode_=QUICK;
        juce::Point<float> nP=juce::Point<float>();
        w->getWirePath().getNearestPoint(juce::Point<float> (e.x,e.y),nP);
        int tol=35*zoomFactor_;
        Peg* p=getPegNear(nP.getX(),nP.getY(),tol);
        if (p!=0)
        {
            pic::logmsg()<<"Near to Peg !";
            if(w->routedVia(p))
            {
                pic::logmsg()<<"Routed via peg";
                unhookPeg_=p;
            }
        }

        w->addHook(new Hook(nP.getX(),nP.getY(),this));
        w->pathChanged();
        w->preDraw();
        for(std::vector<Wire*>::iterator i=dragWires_.begin();i!=dragWires_.end();i++)
        {
            (*i)->addHook(new Hook(nP.getX(),nP.getY(),this));
            (*i)->pathChanged();
            (*i)->preDraw();
        }
        //XXX
        repaint();
    }
}

void MainComponent::wireDrag(const MouseEvent& e)
{
    RoutingElement* t=getRoutingElementExactlyAt(e.x,e.y);
    if(t==0)
    {
        if(overElement_!=0)
        {
            pic::logmsg()<<" **** exiting routingElement";
            overElement_->setMouseOver(false);
            overElement_=0;
        }
    }
    else
    {
        if(overElement_==0)
        {
            pic::logmsg()<<"***** entering routingElement";
            overElement_=t;
            overElement_->setMouseOver(true);
        }
    }

    doWireDrag(e,selectedWire_);
    for(std::vector<Wire*>::iterator i=dragWires_.begin();i!=dragWires_.end();i++)
    {
        doWireDrag(e,*i);
    }
    juce::Rectangle <float> u;
    selectedWire_->getPathChangeBounds(u);
    repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
}

void MainComponent::doWireDrag(const MouseEvent& e, Wire* w)
{
    if (w!=0)
    {
      Hook* h=w->getHook();
      
      h->setPos(e.x,e.y);
      if (!inCentralViewPort(e.x,e.y))
      {
          autoComponentDrag(e.x,e.y,h);
      }
      else
      {
         stopAutoDrag();
      }

      if (unhookPeg_!=0 && e.y<unhookPeg_->getY())
      {
        w->removePeg(unhookPeg_);
      }

        w->pathChanged();
        w->preDraw();
    }

}

void MainComponent::endWireDrag(const MouseEvent& e)
{
    paintMode_=FULL;
    paintOverMode_=FULL;
    paintForegroundMode_=FULL;
    if (selectedWire_!=0)
    {
        pic::logmsg() <<"endWireDrag: selectedWire_!=0: size of dragWires="<<dragWires_.size();


        Path areaPath=selectedWire_->getHookedArea();
        testPath_=areaPath;
        Peg* pp =droppedOnPeg(e);        

        doEndWireDrag(e, selectedWire_,pp);

        for(std::vector<Wire*>::iterator i=dragWires_.begin();i!=dragWires_.end();i++)
        {
            doEndWireDrag(e,*i,pp);
        }

        unhookPeg_=0;
        selectedWire_=0;
        repaint();
    }

    setMouseCursor(tm_->getMouseCursor());
    stopAutoDrag();
}

void MainComponent::doEndWireDrag(const MouseEvent& e,Wire* w, Peg* pp)
{
    pic::logmsg()<<"doEndWireDrag";
    if (pp!=0 && pp==unhookPeg_)
    {
        pic::logmsg()<<"drop peg=unhookpeg";
        w->setSelected(false);
        w->removeHook();
        w->routeChanged(false,false);
    }
    else
    {
        w->setSelected(false);
        w->removeHook();
        // check if position is on a Peg
        // position encloses a peg
        if(pp!=0)
        {
            pic::logmsg()<<"dropped on peg";

            w->addRoutingElement(pp, 1, -1); 
        }
        else
        {
            // if it is need to associate the peg with the wire path
            RoutingElement* p=dynamic_cast <RoutingElement *>(getComponentAt(e.x,e.y));
            if ((p!=0) && (p->isWireDragTarget()))
            {
                pic::logmsg()<<"dropped on routingElement";
                Component* const c=dynamic_cast <Component *>(p);
                if (c!=0)
                {
                    int input=p->getInputAt(e.x-c->getX(),e.y-c->getY());
                    if(input==999 ||input==-999)
                    {
                        input=1;
                    }

                    // if wire is revconnect need to swap input and output
                    if(w->isRevConnect())
                    {
                        w->addRoutingElement(p,-1*input, input); 
                    }
                    else
                    {
                        w->addRoutingElement(p, input, -1*input); 
                    }
                }
            }
            else
            {
                w->routeChanged(false,false);
            }
        }
    }
    w->preDraw();
}

bool MainComponent::doHighLights(const MouseEvent& e)
{
    MouseEvent ev=e.getEventRelativeTo(this);
    int x2=ev.x;
    int y2=ev.y;
 
    Box* const b2=getBoxAt(x2,y2);
    if (b2!=0 && !(isCircular(b2)))
    {
        Highlightable* const h=dynamic_cast<Highlightable *>(b2);
        //pic::logmsg()<<"dragging over highlightable ";
        if ((highlight_!=0) && (highlight_!=h))
        {
            highlight_->highlight(false);
        }
        highlight_=h;
        highlight_->highlight(true);
        highlightPin(dragPin_,true);
        highlightPin(dragSrcPin_,true);
        return true;
    }
    else
    {
        clearHighlight();
        highlightPin(dragPin_,false);
        highlightPin(dragSrcPin_,false);
        return false;
    }
}

bool MainComponent::isCircular(Box* b)
{
    String id=b->getTopLevelBox()->getId();
    if(dragPin_!=0)
    {
        Wire* w=getWireForDstPin(dragPin_);
        if(w!=0)
        {
           if(w->getId().containsIgnoreCase(id))
           {
                return true;
           }
        }
    }

    if(dragSrcPin_!=0)
    {
        Wire* w=getWireForSrcPin(dragSrcPin_);
        if(w!=0)
        {
           if(w->getId().containsIgnoreCase(id))
           {
                return true;
           }
        }
    }

    return false;
}


void MainComponent::highlightPin(DestinationPin* d, bool hl)
{
    if (d!=0)
    {
        d->highlight(hl);
    }
}

void MainComponent::highlightPin(SourcePin* d, bool hl)
{
    if (d!=0)
    {
        d->highlight(hl);
    }
}

void MainComponent::selectWireForPin(DestinationPin* d, bool hl)
{
    if (d!=0)
    {
        Wire* w =getWireForDstPin(d);
        if(w!=0)
        {
            w->setSelected(hl);
        }
    }
}

void MainComponent::selectWireForPin(SourcePin* d, bool hl)
{
    if (d!=0)
    {
        Wire* w =getWireForSrcPin(d);
        if(w!=0)
        {
            w->setSelected(hl);
        }
    }
}

void MainComponent::create_connection(ChangeWireProperties* cw)
{
    model_->create_connection(cw->getSrcId(),cw->getDstId(), cw->getUsing(), cw->getFilter(), cw->getControl());
}

void MainComponent::connection_created(String id)
{
    if (wires_.count(id)==0)
    {
        Connection* c = model_->get_connection(id);
        if(!(c->is_hidden()))
        {
            //pic::logmsg()<<"connection_created "<<id.toUTF8();
            addWire(c);
        }
        else
        {
            //pic::logmsg()<<"connection has hidden protocol - not added "<<id.toUTF8();
            delete c;
        }
    }
}

void MainComponent::addWire(Connection* c)
{
//    pic::logmsg()<<"addWire id="<< c->get_id().toUTF8();
    String id=c->get_id();

    Box* b =getBoxById(c->output());
    if(b!=0)
    {
        if(!b->isSingleInput())
        {
            if(b->isExpanded() && b->hasInputBoxes())
            {
                b->refreshExpanded();
            }
        }
    }

    Wire* w=new Wire(c,model_,this);
    wires_.insert(std::pair<String,Wire*>(c->get_id(),w));
    pic::logmsg() <<"Wire added:"<<std::string(w->getId().toUTF8()) << "size of wires_="<<wires_.size();

    // call addDrawingWire first otherwise doStoredRoute may add it and the subsequent
    // call to addDrawingWire may add a second one with the same route.
    addDrawingWire(w);
    doStoredRoute(id,w);

    juce::Rectangle <float> u;
    w->getPathBounds(u);
    repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));

    DestinationPin* dp=w->getUsingDP();
    if(dp!=0)
    {
        Box* b=dp->findParentComponentOfClass<Box>();
        if(b!=0)
        {
            if(b->getUsingChanged())
            {
                if(b->isExpanded())
                {
                    b->refreshExpanded();
                }
                else
                {
                    b->refreshExpand();
                }
                b->setUsingChanged(false);
                b->repaint();
            }

            b->refreshButton();
        }
    }
}

String MainComponent::getLoosePinId()
{
    loosePinCount_++;
    return "LoosePin"+String(loosePinCount_);
}

void MainComponent::addLooseWire(String srcId, String dstId )
{
    String id=srcId+":"+dstId;
    model_->createLooseWire(id);
}

void MainComponent::looseWireAdded(String id)
{
    Wire* w = new Wire(id,model_,this,model_->get_store(id));
    wires_.insert(std::pair<String,Wire*>(id,w));
    pic::logmsg()<<"Loose Wire added:"<<std::string(w->getId().toUTF8()) << " size of wires_="<<wires_.size();
    //std::cout<<"Loose Wire added:"<<std::string(w->getId().toUTF8()) << " size of wires_="<<wires_.size()<<std::endl;
    doStoredRoute(id,w);

    w->preAdd();
    int h=w->getRouteHash();
    pic::logmsg() <<"     hash="<<h;
    if(h!=0)
    {
        looseWires_.insert(std::pair<String,Wire*>(id,w));
        w->preDraw();
        cacheWire(w);
        foregroundWire(w);
        repaint();
    }
    else
    {
        pic::logmsg()<<"src or dst pins for loosewire not loaded yet- added to pending wires";
        w->setPending(true);
        pendingWires_.insert(std::pair<String,Wire*>(w->getId(),w));
        if (w->getUsingDP()!=0 || w->getUsingSP()!=0)
        {
            cacheWire(w);     
        }
    }
}

void MainComponent::looseWireChanged(String id)
{
    routingChanged(id);
}

void MainComponent::doStoredRoute(String id, Wire* w)
{
        
    std::list<StoredRouteElement> route;
    if(w->isRevConnect())
    {
        getMatchingReverseStoredRoute(id,route);
        if(!route.empty())
        {
           if(route.front().isRevConnect())
           {
              addStoredRoute(w,route);
           }
           else
           {
             addReverseStoredRoute(w,route);
           }
        }

    }
    else
    {
        getMatchingStoredRoute(id,route);
        if(!route.empty())
        {
            addStoredRoute(w,route);
        }
    }

}

void MainComponent::getMatchingStoredRoute(String id,std::list<StoredRouteElement>& route )
{

//    pic::logmsg()<<"getMatchingStoredRoute";
//    for (std::map<String,std::list<StoredRouteElement> >::iterator iter=storedRouteMap_.begin();iter!=storedRouteMap_.end();iter++)
//    {
//        pic::logmsg()<<"   storedRouteid="<<std::string(iter->first.toUTF8());
//    }
//
    std::map<String,std::list<StoredRouteElement> >::iterator i=storedRouteMap_.find(id);
    if(i!=storedRouteMap_.end())
    {
        route=i->second;
    }
    else
    {
        String srcId=id.upToFirstOccurrenceOf(":",false,false);
        String dstId=id.fromFirstOccurrenceOf(":",false,false);

        for (std::map<String,std::list<StoredRouteElement> >::iterator iter=storedRouteMap_.begin();iter!=storedRouteMap_.end();iter++)
        {
            String storedSrcId=(iter->first).upToFirstOccurrenceOf(":",false,false);
            String storedDstId=(iter->first).fromFirstOccurrenceOf(":",false,false);
            if (srcId.contains(storedSrcId) && dstId.contains(storedDstId))
            {
                pic::logmsg()<<"parent match";
                route=iter->second;
                return;
            }
        }
    }
}

void MainComponent::getMatchingReverseStoredRoute(String id,std::list<StoredRouteElement>& route )
{
    std::map<String,std::list<StoredRouteElement> >::iterator i=storedRouteMap_.find(id);
    if(i!=storedRouteMap_.end())
    {
        route=i->second;
    }
    else
    {
        String srcId=id.upToFirstOccurrenceOf(":",false,false);
        String dstId=id.fromFirstOccurrenceOf(":",false,false);

        for (std::map<String,std::list<StoredRouteElement> >::iterator iter=storedRouteMap_.begin();iter!=storedRouteMap_.end();iter++)
        {
            String storedSrcId=(iter->first).upToFirstOccurrenceOf(":",false,false);
            String storedDstId=(iter->first).fromFirstOccurrenceOf(":",false,false);
            if (srcId.contains(storedDstId) && dstId.contains(storedSrcId))
            {
                pic::logmsg()<<"reverse parent match";
                route=iter->second;
                return;
            }
        }
    }
}

void MainComponent::addStoredRoute(Wire* w, std::list<StoredRouteElement> route)
{
    std::list<StoredRouteElement>::iterator iter=route.begin();
    while(iter!=route.end())
    {
        StoredRouteElement  sr=*iter;
        sr.tick();
        startTimer(6,2000);

        RoutingElement* r=getRoutingElement(sr.getId());
        if(r!=0)
        {
            w->addRoutingElement(r, sr.getInput(), sr.getOutput());
            pic::logmsg()<<"addStoredRoute:input="<<sr.getInput()<<" output="<<sr.getOutput();
        }

        iter++;
    }
    //remove the route from the storedRouteMap
    storedRouteMap_.erase(w->getId());
}

void MainComponent::addReverseStoredRoute(Wire* w, std::list<StoredRouteElement> route)
{

    std::list<StoredRouteElement>::reverse_iterator iter=route.rbegin();
    while(iter!=route.rend())
    {
        StoredRouteElement  sr=*iter;
        sr.tick();
        startTimer(6,2000);
        RoutingElement* r=getRoutingElement(sr.getId());
        if(r!=0)
        {
            w->addRoutingElement(r, sr.getOutput(), sr.getInput());
            pic::logmsg()<<"addReverseStoredRoute:input="<<sr.getOutput()<<" output="<<sr.getInput();
        }

        iter++;
    }
//    remove the route from the storedRouteMap
    storedRouteMap_.erase(w->getId());
}


int MainComponent::getNumWires()
{
    return wires_.size();
}

int MainComponent::getNumPendingWires()
{
    return pendingWires_.size();
}

void MainComponent::storeRoute(Wire* w, std::list<StoredRouteElement>& route)
{
    pic::logmsg()<<"*****storeRoute:  wire= "<<std::string(w->getId().toUTF8());
    PropertyStore* props=w->getPropertyStore();
    if (props!=0)
    {
        if (props->has_list("routing"))
        {
            for (unsigned i=0; i<props->list_getsize("routing");i++)
            {
                String sid=props->get_list("routing",i);
                RoutingElement* r=getRoutingElement(sid);
                if (r!=0)
                {
                    int input=getInput(r,w);
                    int output=getOutput(r,w);
                    pic::logmsg() <<"StoreRoute: id="<<std::string(sid.toUTF8())<<" input="<<input<<" output="<<output;
                    StoredRouteElement sr=StoredRouteElement(sid,input,output,w->isRevConnect());
                    startTimer(6,2000);
                    pic::logmsg()<<std::string(sid.toUTF8())<<" index="<<i;
                    route.push_back(sr);
                }
            }
        }
    }
}

void MainComponent::saveRoute(String id, std::list<StoredRouteElement> route)
{
    
    pic::logmsg()<<"saveRoute"<<std::string(id.toUTF8());
//    std::map<String, std::list<StoredRouteElement> >::iterator i=storedRouteMap_.find(id);
//    if(i!=storedRouteMap_.end())
//    {
//        pic::logmsg()<<"route with this id already in map: route.empty="<<route.empty();
//    }
//
    int n=storedRouteMap_.erase(id);
    if(n!=0)
    {
        pic::logmsg()<<"SaveRoute: erased route with same id from storedRouteMap";
    }

    if(!route.empty())
    {
        storedRouteMap_.insert(std::pair<String,std::list<StoredRouteElement> >(id,route));
    }
}

String MainComponent::getWireId(SourcePin* srcPin,DestinationPin* dstPin)
{
    String srcId=srcPin->getId();
    String dstId=dstPin->getId();
    return srcId+":"+dstId;
}

void MainComponent::updateDstBox(Wire* w)
{
    DestinationPin* dp=w->getUsingDP();
    if(dp!=0)
    {
       Box* b=dp->findParentComponentOfClass<Box>();
       if(b!=0)
       {
           b->refreshButton();
           b->setFree(true);
       }
    }
}

void MainComponent::connectionRemoved(String wireId)
{
//    pic::logmsg()<<"connectionRemoved: wireMoveList contains "<<wireMoveList_.size()<<" wires ";
//    std::cout<<"connectionRemoved: wireMoveList contains "<<wireMoveList_.size()<<" wires "<<std::endl;
//    pic::logmsg()<<"connectionRemoved:"<<wires_.count(wireId)<<" wireId "<<wireId.toUTF8();
//    std::cout<<"connectionRemoved:"<<wires_.count(wireId)<<" wireId "<<wireId.toUTF8()<<std::endl;
    Wire* w=getWireById(wireId);
    if(w!=0)
    {
        if(w->isPending())
        {

            removeCachedWire(w);
            w->removeFromPins();

            removePendingWire(w);
            wires_.erase(wireId);
//            int n=wires_.erase(wireId);
//          pic::logmsg() <<"erased "<<n<<" wires";
//            std::cout <<"erased "<<n<<" wires"<<std::endl;
//            std::cout <<"size of wires_="<<wires_.size()<<std::endl;
        }
        else
        {
            if(selectedWire_==w)
            {
                w->setSelected(false);
                selectedWire_=0;
            }
            updateDstBox(w);
            removeForegroundWire(w);
            removeLooseWire(w); 

            wires_.erase(wireId);
//            int n=wires_.erase(wireId);
//            pic::logmsg() <<"erased "<<n<<" wires";
//            std::cout <<"erased "<<n<<" wires"<<std::endl;
//            pic::logmsg() <<"size of wires_="<<wires_.size();
//            std::cout <<"size of wires_="<<wires_.size()<<std::endl;
//            std::cout<<"connectionRemoved before call to removeDrawingWire "<<w<<std::endl;

            removeDrawingWire(w);
            // get the toplevel src and dst boxes and remove w from their caches
            removeCachedWire(w);
            unForegroundBoxes(w);
            changeWirePart2(wireId);
            w->removeFromPins();
        }

        delete w;
        repaint();
    }
    else
    {
        pic::logmsg() <<"wire not found for wireId="<<std::string(wireId.toUTF8());
        //std::cout<<"wire not found for wireId="<<wireId.toUTF8()<<std::endl;
    }
}

void MainComponent::clearHighlight()
{
    if(highlight_!=0)
    {
        highlight_->highlight(false);
        highlight_=0;
    }
}

void MainComponent::startCanvasDrag(const MouseEvent& e)
{
    setBufferedToImage(true);
    pic::logmsg()<<"startCanvasDrag";
    if (vp_!=0)
    {
        mouseDownX_=vp_->getViewPositionX();
        mouseDownY_=vp_->getViewPositionY();
    }
    else
    {
        pic::logmsg() <<"vp_=0";
    }
}

void MainComponent::canvasDrag(const MouseEvent& e)
{
    int x2=mouseDownX_-e.getDistanceFromDragStartX();
    int y2=mouseDownY_-e.getDistanceFromDragStartY();

        if (inViewPort(e.x,e.y))
        {
//            pic::logmsg()<<"canvasdrag calling stopAutodrag";
            stopAutoDrag();
            doCanvasDrag(x2,y2);
        }
        else
        {
            autoDrag(e.x,e.y,true);
        }
}
void MainComponent::autoComponentDrag(int x,int y,AutoDragable* b)
{
    autoDragComp_=b;
    autoDrag(x,y,false);
}

void MainComponent::autoDrag(int x, int y,bool canvas)
{
//   pic::logmsg()<<"autodrag"; 
   if (canvas)
   {
       autoDragDirection_=getDragDirection(x,y);
   }
   else
   {
       int mx=getMouseXYRelative().getX();
       int my=getMouseXYRelative().getY();
       autoDragDirection_=getReverseDragDirection(mx,my);
   }
   if (autoDragMode_==false)
   {
        autoDragMode_=true;
        startTimer(1,10);
   }
}

int MainComponent::getDragDirection(int x, int y)
{
    if((x-40)<(vp_->getViewPositionX()))
    {
        return WEST;
    }
    else if ((x+40)>(vp_->getViewPositionX()+vp_->getViewWidth()))
    {
        return EAST;
    }
    else if ((y-40)<(vp_->getViewPositionY()))
    {
        return NORTH;
    }
    else if ((y+40)>(vp_->getViewPositionY()+vp_->getViewHeight()))
    {
        return SOUTH;
    }
    return 0;
}
int MainComponent::getReverseDragDirection(int x, int y)
{
    if((x-40)<(vp_->getViewPositionX()))
    {
        return EAST;
    }
    else if ((x+40)>(vp_->getViewPositionX()+vp_->getViewWidth()))
    {
        return WEST;
    }
    else if ((y-40)<(vp_->getViewPositionY()))
    {
        return SOUTH;
    }
    else if ((y+40)>(vp_->getViewPositionY()+vp_->getViewHeight()))
    {
        return NORTH;
    }
    return 0;
}

bool MainComponent::inCentralViewPortX(int x,int tol)
{
    if(x>tol && x<(getWidth()-tol))
    {
        return (((x-tol)-vp_->getViewPositionX())>0)&&((x+tol)<(vp_->getViewPositionX()+vp_->getViewWidth()));
    }
    else
    {
        return true;
    }
}

bool MainComponent:: inCentralViewPortY(int y, int tol)
{
    if((y>tol)&&(y<getHeight()-tol))
    {
        return ((((y-tol)-vp_->getViewPositionY())>0)&&((y+tol)<(vp_->getViewPositionY()+vp_->getViewHeight())));
    }
    else
    {
        return true;
    }
}

bool MainComponent::inViewPortX(int x)
{
    return ((x-vp_->getViewPositionX())>0)&&(x<(vp_->getViewPositionX()+vp_->getViewWidth()));
}

bool MainComponent:: inViewPortY(int y)
{
    return (((y-vp_->getViewPositionY())>0)&&(y<(vp_->getViewPositionY()+vp_->getViewHeight())));
}

bool MainComponent::offCanvas(int x, int y,int w, int h)
{
//    pic::logmsg()<<"offCanvas: x="<<x<<" getWidth()="<<getWidth()<<" y="<<y<<" getHeight()="<<getHeight();
    return x<=0||(x+w)>=getWidth()||y<=0||(y+h)>=getHeight();
}

bool MainComponent::inViewPort(int x, int y)
{
    return inViewPortX(x) && inViewPortY(y);
}

bool MainComponent::inCentralViewPort(int x, int y)
{
    return inCentralViewPortX(x,40) && inCentralViewPortY(y,40);
}


void MainComponent::doCanvasDrag(int x2, int y2)
{
    double viewWidth=-0.5*vp_->getViewWidth();
    double viewHeight=-0.5*vp_->getViewHeight();
    double portWidth=-0.5*vp_->getWidth();
    double portHeight=-0.5*vp_->getHeight();

    if (x2<viewWidth)
    {
        x2=viewWidth;
    }
    if (y2<=viewHeight)
    {
        y2=viewHeight;
    }

    if (x2>(getWidth()+portWidth))
    {
        x2=getWidth()+portWidth;
    }

    if (y2>(getHeight()+(portHeight)))
    {
        y2=getHeight()+(portHeight);
    }

    setViewPosition(x2,y2,false);
}

void MainComponent::setViewPosition(int x, int y,bool save)
{
    if(vp_!=0)
    {
//        pic::logmsg()<<"setViewPosition: x="<<x<<" y="<<y;
        vp_->setViewPosition(x,y);
    }
    if(save)
    {
        setWorkspaceProps();
    }
}

void MainComponent::endCanvasDrag(const MouseEvent& e)
{
    pic::logmsg()<<"endCanvasDrag";
    setMouseCursor(tm_->getMouseCursor());
    if (autoDragMode_==true)
    {
        stopAutoDrag();
    }
    setWorkspaceProps();
    setBufferedToImage(false);
    repaint();
}

void MainComponent::srcPinMouseDown(const MouseEvent& e)
{
    pic::logmsg()<<"srcPinMouseDown";
    eventOriginPin_=0;
    dragSrcPinReady_=true;

    Box* b;
    if(e.originalComponent==this)
    {
        b=candidateWiringBox_;
    }
    else
    {
        SourcePin* const p =dynamic_cast <SourcePin *>(e.originalComponent);
        if(p!=0)
        {
            b =getBoxById(p->getId()); 
        }
        else
        {
            b =dynamic_cast <Box *>(e.originalComponent);
        }
    }

    if(b!=0)
    {
        b->getSrcPin()->setMouseMode(e);
        eventOriginPin_=b->getSrcPin();
        Atom* a =b->getAtom();
        if(a!=0)
        {
            if(a->has_protocol("metronome"))
            {
                showMetronomeOutputs(true,true);
            }
            else if(a->has_protocol("controller"))
            {
                showControllerOutputs(true,true);
            }
        }
    }
    else
    {
        pic::logmsg()<<"srcPinMouseDown - no Box";
    }
}

void MainComponent::showMetronomeOutputs(bool shouldBeShown, bool saveSetting)
{
//    pic::logmsg()<<"showMetronomeOutputs "<<shouldBeShown<<"  "<<saveSetting;
    hideMetronomeOutputs_=!(shouldBeShown);
    if(saveSetting)
    {
        setProperty("showMetronomeOutputs",shouldBeShown);
    }
    mm_->setShowMetronomes(shouldBeShown);
}

void MainComponent::showControllerOutputs(bool shouldBeShown, bool saveSetting)
{
    hideControllerOutputs_=!(shouldBeShown);
    if(saveSetting)
    {
        setProperty("showControllerOutputs",shouldBeShown);
    }
    mm_->setShowControllers(shouldBeShown);
}


void MainComponent::srcPinMouseDisconnectDrag(const MouseEvent& e)
{
    pic::logmsg()<<"srcPinMouseDisconnectDrag";
    MouseEvent ev=e.getEventRelativeTo(this);
    int x=ev.x;
    int y=ev.y;
    if (dragSrcPinReady_)
    {
        initSrcPinDisconnectDrag(e);
        doHighLights(e);
        repaint();
    }
  
    else
    {
        int xp=x-(12*zoomFactor_);
        int yp=y-(14*zoomFactor_);

        loosePinDrag(dragSrcPin_,xp,yp,e);
    }
}

void MainComponent::initSrcPinDisconnectDrag(const MouseEvent& e)
{
    MouseEvent ev=e.getEventRelativeTo(this);
    int x=ev.x;
    int y=ev.y;

    SourcePin* p;
    if(e.originalComponent==this)
    {
        p=candidateWiringBox_->getSrcPin();
    }
    else
    {
        p=dynamic_cast <SourcePin *>(e.originalComponent);
        if(p==0)
        {
            p=(dynamic_cast<Box *>(e.originalComponent))->getSrcPin();
        }
    }

    Wire* w =getWireForSrcPin(p);
    if (w!=0)
    {
        DestinationPin* dstPin=w->getDstPin();
        if (dstPin!=0)
        {
            String did=w->getUsingDpId();
            if (dstPin->isLoosePin())
            {
                deleteWire(w);
                pic::logmsg()<<"completely loose wire removed";
                // XXX currently wire is removed - drag ends.  Should be able to have wire loose at both ends
            }
            else
            {
                std::list<StoredRouteElement> route;
                storeRoute(w,route);

                deleteConnectionsLike(w);
                createSourcePin(x-(12.0f*zoomFactor_),y-(14.0f*zoomFactor_));
                dragSrcPin_=getSourcePinAt(x-(12.0f*zoomFactor_),y-(14.0f*zoomFactor_)); 
                
                if(dragSrcPin_!=0)
                {
                    saveRoute(dragSrcPin_->getId()+":"+did,route);
                    addLooseWire(dragSrcPin_->getId(),did);
                    clearWireMoveList();
                    Wire* wl=getWireForSrcPin(dragSrcPin_);
                    if(wl!=0)
                    {
                        addToWireMoveList(wl);
                    }
                }

                dragSrcPinReady_=false;
            }
        }
    }
}

void MainComponent::srcPinMouseDrag(const MouseEvent& e)
{
    //pic::logmsg()<<"srcPinMouseDrag";
    MouseEvent ev=e.getEventRelativeTo(this);
    int x=ev.x;
    int y=ev.y;

    if (dragSrcPinReady_)
    {
        SourcePin* p;
        if(e.originalComponent==this)
        {
            p=candidateWiringBox_->getSrcPin();
        }
        else
        {
            p=dynamic_cast <SourcePin *>(e.originalComponent);
            if(p==0)
            {
                p=(dynamic_cast<Box *>(e.originalComponent))->getSrcPin();
            }
        }

        if(!p->wiringAllowed())
        {
            return;
        }

        createDestinationPin(x-(12.0f*zoomFactor_),y-(14.0f*zoomFactor_));
        dragPin_=getDestinationPinAt(x-(12.0f*zoomFactor_),y-(14.0f*zoomFactor_)); 

        if(dragPin_!=0)
        {
            addLooseWire(p->getId(),dragPin_->getId());
            clearWireMoveList();
            Wire* w=getWireForDstPin(dragPin_);
            if(w!=0)
            {
                addToWireMoveList(w);
            }
        }

        dragSrcPinReady_=false;
        doHighLights(e);
        repaint();
   }
    
    else
    {
        int xp=x-(12.0f*zoomFactor_);
        int yp=y-(14.0f*zoomFactor_);

        loosePinDrag(dragPin_,xp,yp,e);
    }
}

void MainComponent::loosePinDrag(SourcePin* sp, int x, int y,const MouseEvent& e)
{
    if (sp!=0)
    {
        sp->doLoosePinDrag(x,y);
        doHighLights(e);
        thingMoved();
    }
}

void MainComponent::loosePinDrag(DestinationPin* dp, int x, int y,const MouseEvent& e)
{
    if (dp!=0)
    {
        dp->doLoosePinDrag(x,y);
        doHighLights(e);
        thingMoved();
    }
}

void MainComponent::routeThroughPeg(DestinationPin* dp, int x, int y)
{
    Wire* w=getWireForDstPin(dp);
    routeThroughPegBack(w,x,y);
}

void MainComponent::routeThroughPeg(SourcePin* sp, int x, int y)
{
        Wire* w=getWireForSrcPin(sp);
        routeThroughPegFront(w,x,y);
}

Peg* MainComponent::getPegNear(int x, int y,int tol)
{
     for (std::map<String,Peg*>::iterator iter=pegs_.begin();iter!=pegs_.end();iter++)
     {
        Peg* pp=iter->second;
        if (pp!=0)
        {
            int px=pp->getX()+(pp->getWidth()*0.5);
            int py=pp->getY()+(pp->getHeight()*0.5);

            //if((x>px-tol) && (x<px+tol) && (y>py-tol) && (y<py+tol))
            // XXX workaround for peg unhooking bug
            // only unhook from right
            if((x>px) && (x<px+tol) && (y>py-tol) && (y<py+tol))
            {
                return pp;
            }
        }
    }
    return 0;
}

RoutingElement* MainComponent::getRoutingElementAt(int x, int y)
{
    return getRoutingElementExactlyAt(x,y);
}

RoutingElement* MainComponent::getRoutingElementExactlyAt(int x, int y)
{
    for (std::map<String,Peg*>::iterator iter=pegs_.begin();iter!=pegs_.end();iter++)
    {
        Peg* pp=iter->second;
        if (pp!=0)
        {
            if(pp->contains(juce::Point<int> (x-pp->getX(),y-pp->getY())))
            {
                return pp;
            }
        }
    }

    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;
        if (t!=0)
        {
            if(t->contains(juce::Point<int> (x-t->getX(),y-t->getY())))
            {
                return t;
            }
        }
    }

    return 0;
}       

RoutingElement* MainComponent::getRoutingElementNear(int x, int y, int tol, int step)
{
    int t=step;
    RoutingElement* b;
    
    while(t<=tol)
    {
        b=getRoutingElementExactlyAt(x+t,y);     
        if(b!=0)
        {
            return b;
        }

        b=getRoutingElementExactlyAt(x+(0.7*t),y-(0.7*t));     
        if(b!=0)
        {
            return b;
        }


        b=getRoutingElementExactlyAt(x,y-t);     
        if(b!=0)
        {
            return b;
        }

        b=getRoutingElementExactlyAt(x-(0.7*t),y-(0.7*t));     
        if(b!=0)
        {
            return b;
        }

        b=getRoutingElementExactlyAt(x-t,y);     
        if(b!=0)
        {
            return b;
        }

        b=getRoutingElementExactlyAt(x-(0.7*t),y+(0.7*t));     
        if(b!=0)
        {
            return b;
        }

        b=getRoutingElementExactlyAt(x,y+t);     
        if(b!=0)
        {
            return b;
        }

        b=getRoutingElementExactlyAt(x+(0.7*t),y+(0.7*t));     
        if(b!=0)
        {
            return b;
        }

        t=t+step;
    }
    return 0;
}


bool MainComponent::isRoutingElementNear(RoutingElement* p, int x, int y, int tol)
{
    Component* const c=dynamic_cast <Component *>(p);
    if(c!=0)
    {
        juce::Rectangle <int> r= c->getBoundsInParent();

        Trunk* const t=dynamic_cast <Trunk *>(p);
        if(t!=0)
        {
           if(t->getOrientation()==Trunk::HORIZONTAL)
           {
                r.expand(0,tol/2);
           }
           else
           {
                r.expand(tol/2,0);
           }
        }
        r.expand(tol/2,tol/2);
        return r.contains(x,y);
    }

    return false;
}

Trunk* MainComponent::getTrunkAt(int x, int y)
{
    for (std::map<String,Trunk*>::iterator iter=trunks_.begin();iter!=trunks_.end();iter++)
    {
        Trunk* t=iter->second;
        if (t!=0)
        {
            if(t->contains(juce::Point<int> (x-t->getX(),y-t->getY())))
            {
                return t;
            }
        }
    }
    return 0;
}

SourcePin* MainComponent::getSourcePinAt(int x, int y)
{
    pic::logmsg()<<"getSourcePinAt "<<x<<" "<<y;
    SourcePin* p=0;
    for (std::map<String,SourcePin*>::iterator iter=loose_sps_.begin();iter!=loose_sps_.end();iter++)
    {
        SourcePin* t=iter->second;
        if (t!=0)
        {
            //pic::logmsg()<<t->getX();
            //pic::logmsg()<<t->getWidth();
            if(t->contains(juce::Point<int> (x-t->getX(),y-t->getY())))
            {
                pic::logmsg()<<"   Matching pin";
                p=t;
            }
        }
    }

    if(p==0)
    {
        pic::logmsg()<<"cant find source pin at "<<x <<","<<y;
    }
    return p;

}

DestinationPin* MainComponent::getDestinationPinAt(int x, int y)
{
    pic::logmsg()<<"getDestinationPinAt "<<x<<" "<<y;
    DestinationPin* p=0;

    for (std::map<String,DestinationPin*>::iterator iter=loose_dps_.begin();iter!=loose_dps_.end();iter++)
    {
        DestinationPin* t=iter->second;
        if (t!=0)
        {
            //pic::logmsg()<<t->getX();
            //pic::logmsg()<<t->getWidth();
            if(t->contains(juce::Point<int> (x-t->getX(),y-t->getY())))
            {
                pic::logmsg()<<"   Matching pin";
                p=t;
            }
        }
    }
    if(p==0)
    {
        pic::logmsg()<<"cant find destination pin at "<<x <<","<<y;
    }
    return p;
}

TrunkCorner*  MainComponent::getTrunkCornerAt(int x, int y)
{
    for (std::map<String,TrunkCorner*>::iterator iter=corners_.begin();iter!=corners_.end();iter++)
    {
        TrunkCorner* t=iter->second;
        if (t!=0)
        {
            if(t->contains(juce::Point<int> (x-t->getX(),y-t->getY())))
            {
                return t;
            }
        }
    }
    return 0;
}

void MainComponent:: routeThroughPegBack(Wire* w, int x, int y)
{
    TrunkCorner* tc=getTrunkCornerAt(x,y);
    if(tc!=0)
    {
        pic::logmsg()<<"over trunkCorner";
    }

    RoutingElement* p=getRoutingElementExactlyAt(x,y);
    if(p!=0)
    {
        pic::logmsg()<<"also over routingElement";
    }

//  Try to prevent it being too easy for pin to fall out of trunk:
    if(p==0 && currentRoutingElement_!=0)
    {
       int tol=50;
       if (isRoutingElementNear(currentRoutingElement_,x,y,tol))
       {
            p=currentRoutingElement_;
            pic::logmsg()<<"near routingElement";
       }
    }

    if(p==0||p!=currentRoutingElement_)
    {
        routeThroughBack_=true;
        currentRoutingElement_=p;
    }

    if(p!=0)
    {
        if (routeThroughBack_==true)
        {
            if (w->routedVia(p))
            {
                // XXX
                pic::logmsg()<<"RouteThroughPegBack: already routed via routingelement";
                //w->removePeg(p);
            }
            else
            {
                Component* const c=dynamic_cast <Component *>(p);
                if (c!=0)
                {
                    int input=p->getInputAt(x-c->getX(),y-c->getY());
                    int output=p->getOutputAt(x-c->getX(),y-c->getY());
                    if(input==-999)
                    {
                       input=1; 
                    }
                    pic::logmsg()<<"AddRoutingElement input="<<input<<" output= "<<output;
                    w->addRoutingElementAtEnd(p, input, output);
                }
            }
            routeThroughBack_=false;
        }

        else
        {
            Component* const c=dynamic_cast <Component *>(p);
            if (c!=0)
            {
                p->setOutput(w->getId(),x-c->getX(), y-c->getY()); 
            }
        }
    }
    
}


void MainComponent:: routeThroughPegFront(Wire* w, int x, int y)
{
    RoutingElement* p=getRoutingElementExactlyAt(x,y);

//  Try to prevent it being too easy for pin to fall out of trunk:
    if(p==0 && currentRoutingElement_!=0)
    {
       int tol=50;
       if (isRoutingElementNear(currentRoutingElement_,x,y,tol))
       {
            p=currentRoutingElement_;
       }
    }


    if(p==0 || p!=currentRoutingElement_)
    {
        routeThroughFront_=true;
        currentRoutingElement_=p;
    }

    if(p!=0)
    {
        if(routeThroughFront_==true)
        {
            if (w->routedVia(p))
            {
                // XXX
                pic::logmsg()<<"RouteThroughPegFront: already routed via routingelement";
                //w->removePeg(p);
            }
            else
            {
                Component* const c=dynamic_cast <Component *>(p);
                if (c!=0)
                {
                    int input=p->getInputAt(x-c->getX(),y-c->getY());
                    int output=p->getOutputAt(x-c->getX(),y-c->getY());
                    if(input==999)
                    {
                        input=1;
                    }
                    w->addRoutingElementAtStart(p, input,output);
                }
            }
            routeThroughFront_=false;
        }
        else
        {
            Component* const c=dynamic_cast <Component *>(p);
            if (c!=0)
            {
                p->setInput(w->getId(),x-c->getX(), y-c->getY()); 
            }
        }
    }
}


void MainComponent::srcPinMouseUp(const MouseEvent& e)
{
    pic::logmsg()<<"srcPinMouseUp";
    dragSrcPinReady_=false;
    currentRoutingElement_=0;
    stopAutoDrag();
    endThingMoved();

    if (dragSrcPin_!=0)
    {
        dragSrcPin_->doSetPositionProps();
        MouseEvent ev=e.getEventRelativeTo(this);
        int x=ev.x;
        int y=ev.y;
        dropLoosePin(dragSrcPin_,x,y);
        dragSrcPin_=0;
        clearHighlight();
    }

    if (dragPin_!=0)
    {
        dragPinReady_=false;
        dragPin_->doSetPositionProps();
        MouseEvent ev=e.getEventRelativeTo(this);
        int x=ev.x;
        int y=ev.y;
        dropLoosePin(dragPin_,x,y);
        dragPin_=0;
        clearHighlight();
    }
    if(eventOriginPin_!=0)
    {
        eventOriginPin_->onDragEnd();
        eventOriginPin_=0;
    }
}

void MainComponent::dstPinMouseDown(const MouseEvent& e)
{
    pic::logmsg()<<"dstPinMouseDown ";
    eventOriginPin_=0;
    dragPinReady_=true;
    Box* b;
    if(e.originalComponent==this)
    {
        b=candidateWiringBox_;
    }
    else
    {
        DestinationPin* const p =dynamic_cast <DestinationPin *>(e.originalComponent);
        if(p!=0)
        {
            b =getBoxById(p->getId()); 
        }
        else
        {
            b =dynamic_cast <Box *>(e.originalComponent);
        }
    }

    if(b!=0)
    {
        b->getDstPin()->setMouseMode(e);
        eventOriginPin_=b->getDstPin();
    }
    else
    {
        pic::logmsg()<<"dstPinMouseDown - no Box";
    }

}

void MainComponent::dstPinMouseDrag(const MouseEvent& e)
{
    MouseEvent ev=e.getEventRelativeTo(this);
    int x=ev.x;
    int y=ev.y;
    if (dragPinReady_)
    {
        DestinationPin* p; 
        if(e.originalComponent==this)
        {
            p=candidateWiringBox_->getDstPin();
        }
        else
        {
            p=dynamic_cast <DestinationPin *>(e.originalComponent);
            if(p==0)
            {
                p=(dynamic_cast<Box *>(e.originalComponent))->getDstPin();
            }
        }

        createSourcePin(x-(12.0f*zoomFactor_),y-(14.0f*zoomFactor_));
        dragSrcPin_=getSourcePinAt(x-(12.0f*zoomFactor_),y-(14.0f*zoomFactor_)); 
        if(dragSrcPin_!=0)
        {
            addLooseWire(dragSrcPin_->getId(),p->getId());

            clearWireMoveList();
            Wire* w=getWireForSrcPin(dragSrcPin_);
            if(w!=0)
            {
                addToWireMoveList(w);
            }
        }
        dragPinReady_=false;
        doHighLights(e);
        repaint();
    }
    
    else
    {
        int xp=x-(12.0f*zoomFactor_);
        int yp=y-(14.0f*zoomFactor_);

        loosePinDrag(dragSrcPin_,xp,yp,e);
    }
}

void MainComponent::dstPinMouseDisconnectDrag(const MouseEvent& e)
{
    pic::logmsg()<<"dstPinMouseDisconnectDrag ";
    MouseEvent ev=e.getEventRelativeTo(this);
    int x=ev.x;
    int y=ev.y;
    if (dragPinReady_)
    {
        initDstPinDisconnectDrag(e);
        doHighLights(e);
        repaint();
    }
  
    else
    {
        pic::logmsg()<< "set dragPin_ position";
        int xp=x-(12*zoomFactor_);
        int yp=y-(14*zoomFactor_);

        loosePinDrag(dragPin_,xp,yp,e);
    }
}

void MainComponent::initDstPinDisconnectDrag(const MouseEvent& e)
{
    MouseEvent ev=e.getEventRelativeTo(this);
    int x=ev.x;
    int y=ev.y;

    DestinationPin* p;
    if(e.originalComponent==this)
    {
        p=candidateWiringBox_->getDstPin();
    }
    else
    {
        p=dynamic_cast <DestinationPin *>(e.originalComponent);
        if(p==0)
        {
            p=(dynamic_cast<Box *>(e.originalComponent))->getDstPin();
        }
    }

    Wire* w =getWireForDstPin(p);
    if (w!=0)
    {
        SourcePin* srcPin=w->getSrcPin();
        if (srcPin!=0)
        {
            String sid=w->getUsingSpId();
            if(srcPin->isLoosePin())
            {
                deleteWire(w);
                pic::logmsg()<<"completely loose wire removed";
                // XXX currently wire is removed -drag ends. Should be able to have wire loose at both ends
            }
            else
            {
                std::list<StoredRouteElement> route;
                storeRoute(w,route);
                deleteConnectionsLike(w);
                createDestinationPin(x-(12.0f*zoomFactor_),y-(14.0f*zoomFactor_));
                dragPin_=getDestinationPinAt(x-(12.0f*zoomFactor_),y-(14.0f*zoomFactor_)); 
                if(dragPin_!=0)
                {
                    saveRoute(sid+":"+dragPin_->getId(),route);
                    addLooseWire(sid,dragPin_->getId());
                    clearWireMoveList();
                    Wire* wl=getWireForDstPin(dragPin_);
                    if(wl!=0)
                    {
                        addToWireMoveList(wl);
                    }
                }

                dragPinReady_=false;
            }
        }
    }
    else
    {
        pic::logmsg()<<"Wire for destinationPin "<<std::string(p->getId().toUTF8())<<"==0";
    }

}

void MainComponent::dstPinMouseUp(const MouseEvent& e)
{
    pic::logmsg()<<"dstPinMouseUp ";
    dragPinReady_=false;
    currentRoutingElement_=0;
    stopAutoDrag();
    endThingMoved();
    if (dragSrcPin_!=0)
    {
        dragSrcPin_->doSetPositionProps();
        MouseEvent ev=e.getEventRelativeTo(this);
        int x=ev.x;
        int y=ev.y;
        dropLoosePin(dragSrcPin_,x,y);
        dragSrcPin_=0;
        clearHighlight();
    }

    if (dragPin_!=0)
    {
        dragPin_->doSetPositionProps();
        MouseEvent ev=e.getEventRelativeTo(this);
        int x=ev.x;
        int y=ev.y;
        dropLoosePin(dragPin_,x,y);
        dragPin_=0;
        clearHighlight();
    }
    if(eventOriginPin_!=0)
    {
        eventOriginPin_->onDragEnd();
        eventOriginPin_=0;
    }
}

void MainComponent::timerCallback(int timerId)
{
//    std::cout<<"timerCallback "<<timerId<<" autoDragComp="<<autoDragComp_<<std::endl;
    if(timerId==1)
    {
        int x=vp_->getViewPositionX();
        int y=vp_->getViewPositionY();
        int dx=0;
        int dy=0;
        if (autoDragDirection_==WEST)
        {
            dx=10;
        }
        else if (autoDragDirection_==EAST)
        {
            dx=-10;
        }
        
        else if (autoDragDirection_==NORTH)
        {
            dy=10;
        }
        else if (autoDragDirection_==SOUTH)
        {
            dy=-10;
        }
        else
        {
            pic::logmsg()<<"no autoDragDirection";
        }

        mouseDownX_=mouseDownX_+dx;
        mouseDownY_=mouseDownY_+dy;

        doCanvasDrag(x+dx,y+dy);
        if (autoDragComp_!=0)
        {
            autoDragComp_->doAutoDrag(dx,dy);
        }
        else
        {
            pic::logmsg()<<"autoDragComp=0";
        }
    }

    else if(timerId==2)
    {
        mouseWheeling_=false;
        stopTimer(2);
        setWorkspaceProps();
        repaint();
    }

    else if (timerId==3)
    {
        thingMovedFullDraw();
        stopTimer(3);
    }
    else if (timerId==4)
    {
        zoomDragFullDraw();
        stopTimer(4);
    }
    else if (timerId==5)
    {
        thingChangedFullDraw();
        stopTimer(5);
    }
    else if(timerId==6)
    {
        for (std::map<String,std::list<StoredRouteElement> >::iterator iter=storedRouteMap_.begin();iter!=storedRouteMap_.end();)
        {
            if (iter->second.front().expired(1500))
            {
                storedRouteMap_.erase(iter++);
            }
            else
            {
                ++iter;
            }
        }

        stopTimer(6);
    }

}

void MainComponent::stopAutoDrag()
{
    if (autoDragMode_)
    {
        pic::logmsg()<<"MainComponent::stopAutoDrag";
//        std::cout<<"********stopAutoDrag"<<std::endl;
        autoDragMode_=false;
        autoDragComp_=0;
        stopTimer(1); 
    }
}

Peg* MainComponent::droppedOnPeg(const MouseEvent& e)
{
//    pic::logmsg()<<"droppedOnPeg";
    // if directly over a peg - return that peg

    Peg* p=0;

    p=dynamic_cast <Peg*>(getComponentAt(e.x,e.y));
    if(p!=0)
    {
        return p;
    }

    for (std::map<String,Peg*>::iterator iter=pegs_.begin();iter!=pegs_.end();iter++)
    {
        Peg* pp=iter->second;

        if (pp!=0)
        {
            if (testPath_.contains(pp->getX(),pp->getY()))
            {
                pic::logmsg()<<"area contains peg at "<<pp->getX()<<" "<<pp->getY();
                if(p==0)
                {
                   p=pp;
                }
                else if (pp->getY()<p->getY())
                {
                    p=pp;
                }
            }
        }
    }
//    pic::logmsg()<<"returning "<<p;
    return p;
}

void MainComponent::startZoomDrag(const MouseEvent& e)
{
    if (vp_!=0)
    {
        mouseDownX_=(float)e.x/zoomFactor_;
        zoomDragX_=mouseDownX_;
        mouseDownY_=(float)e.y/zoomFactor_;;
    }
    else
    {
        pic::logmsg() <<"vp_=0";
    }
}

void MainComponent::zoomDrag(const MouseEvent& e)
{
    int x2=mouseDownX_-e.getDistanceFromDragStartX();
    //int y2=mouseDownY_-e.getDistanceFromDragStartY();

    if ((zoomDragX_-x2)>0)
    {
        stopTimer(4);
        doZoomInc(0.025f,mouseDownX_*zoomFactor_,mouseDownY_*zoomFactor_,false);
        startTimer(4,300);
    }
    else if((zoomDragX_-x2<0))
    {
        stopTimer(4);
        doZoomInc(-0.025f,mouseDownX_*zoomFactor_,mouseDownY_*zoomFactor_,false);
        startTimer(4,300);
    }

    zoomDragX_=x2;
}

void MainComponent::zoomDragFullDraw()
{
    //pic::logmsg()<<"zoomDragFullDraw()";
    overrideDraft_=true; 
    foregroundWireOverrideDraft_=true;
    repaint();
}

void MainComponent::endZoomDrag(const MouseEvent& e)
{
    doZoomInc(0.0,mouseDownX_*zoomFactor_,mouseDownY_*zoomFactor_,true);

    if (autoDragMode_==true)
    {
        stopAutoDrag();
    }
    repaint();
}

void MainComponent::save_props(String id)
{
    pic::logmsg() <<"MainComponent:save_props: id="<<std::string(id.toUTF8())<<" model_="<<model_;
    if (model_!=0)
    {
        model_->save_store(id);
    }
}

void MainComponent:: trunkInputMouseDown(const MouseEvent& e)
{
    dragInputReady_=true;
    TrunkInput * input=dynamic_cast<TrunkInput *> (e.originalComponent);
    String id =input->getID();
    Wire* w=getWireById(id);
    if(w!=0)
    {
        selectWire(w);
        juce::Rectangle <float> u;
        w->getPathBounds(u);
        input->getTrunk()->repaint();
        repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
    }
}

void MainComponent::trunkInputMouseEnter(const MouseEvent& e)
{
    TrunkInput * input=dynamic_cast<TrunkInput *> (e.originalComponent);
    String id =input->getID();
    Wire* w=getWireById(id);
    if(w!=0)
    {
        selectWire(w);
        juce::Rectangle <float> u;
        w->getPathBounds(u);
        input->getTrunk()->repaint();
        repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
    }
}

void MainComponent::doWasInput(Trunk* t, int x, int y, String id)
{
    TrunkInput* tin=t->getInput(id);
    if(tin!=0)
    {
        currentInput_=tin;
    }
    else
    {
        pic::logmsg()<<"wire not in new section"; 
        currentInput_=0;

        int input= t->getInputAt(x-t->getX(),y-t->getY());
        int output= t->getOutputAt(x-t->getX(),y-t->getY());
        pic::logmsg() <<"was an input- addRoutingElement: input= "<<input <<"output="<<output;
        Wire * w=getWireById(id);
        if(w!=0)
        {
            int hash=w->getRouteHash();
            for(std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
            {
                if((iter->second)->getRouteHash()==hash)
                {
                    if((iter->second)->isRevConnect())
                    {
                        iter->second->insertRoutingElementAfter(t,currentTrunk_,output,input);
                    }
                    else
                    {
                        iter->second->insertRoutingElementBefore(t,currentTrunk_,input,output);
                    }
                }
            }
        }

        currentInput_=t->getInput(id);
    }
}

void MainComponent::doWasOutput(Trunk* t, int x, int y, String id)
{
    TrunkInput* tout=t->getOutput(id);
    if(tout!=0)
    {
        currentInput_=tout;
    }
    else
    {
        pic::logmsg()<<"wire not in new section"; 
        currentInput_=0;
        int input= t->getInputAt(x-t->getX(),y-t->getY());
        int output= t->getOutputAt(x-t->getX(),y-t->getY());
        pic::logmsg() <<"was an output- addRoutingElement: input= "<<input <<"output="<<output;
        Wire * w=getWireById(id);
        if(w!=0)
        {
            int hash=w->getRouteHash();
            for(std::map<String,Wire*>::iterator iter=wires_.begin();iter!=wires_.end();iter++)
            {
                if((iter->second)->getRouteHash()==hash)
                {
                    if((iter->second)->isRevConnect())
                    {
                        iter->second->insertRoutingElementBefore(t,currentTrunk_,output,input);
                    }
                    else
                    {
                        iter->second->insertRoutingElementAfter(t,currentTrunk_,input,output);
                    }
                }
            }
        }

        currentInput_=t->getOutput(id);
    }
}

void MainComponent::doOtherTrunkInputMouseDrag(Trunk* t, int x, int y)
{
    pic::logmsg()<<"Over different trunk section";
    
    if(currentInput_!=0)
    {
        String id=currentInput_->getID();
        bool wasInput=currentInput_->isInput();
        currentInput_->onDrag(x-currentTrunk_->getX(),y-currentTrunk_->getY());
        currentInput_->onMoveEndTest();

        if (wasInput)
        {
            doWasInput(t,x,y,id);
        }
        else
        {
            doWasOutput(t,x,y,id);
        }

    }
    currentTrunk_=t;

}

void MainComponent::doCurrentTrunkInputMouseDrag(Trunk* t, int x, int y)
{
    if(currentInput_!=0)
    {
        currentInput_->onDrag(x-t->getX(),y-t->getY());
        thingChanged();
    }
}

void MainComponent::doTrunkInputMouseDrag(Trunk* t, int x, int y)
{
    if (t!=0 && currentTrunk_->isAssemblyNeighbour(t))
    {
        dragInput_->setTopLeftPosition(x,y);

        if (t==currentTrunk_)
        {
            doCurrentTrunkInputMouseDrag(t,x,y);
        }

        else
        {
            doOtherTrunkInputMouseDrag(t,x,y);
        }
    }

}

void MainComponent::trunkInputMouseDrag(const MouseEvent& e)
{
    MouseEvent ev=e.getEventRelativeTo(this);
    int x=ev.x;
    int y=ev.y;
    if(dragInputReady_)
    {
        currentInput_=dynamic_cast<TrunkInput *> (e.originalComponent);
        dragInput_=new DummyInput(x,y,zoomFactor_);
        addChildComponent(dragInput_);
        currentTrunk_=currentInput_->getTrunk();
        dragInputReady_=false;
    }
    else
    {
        RoutingElement * p =getRoutingElementExactlyAt(x,y);
        if(p==0)
        {
//            pic::logmsg()<<"not over routingelement x="<<x<<" y="<<y;
            if (currentTrunk_!=0)
            {
                doTrunkInputMouseDrag(currentTrunk_,x,y);
            }
        }
        else
        {
//            pic::logmsg()<<"trunkInputMouseDrag x="<<x<<" y="<<y;
            Trunk* t =dynamic_cast<Trunk *>(p);
            doTrunkInputMouseDrag(t,x,y);
        }
    }
}

void MainComponent:: trunkInputMouseUp(const MouseEvent& e)
{
    if (dragInput_!=0)
    {
        pic::logmsg()<<"trunkInputMouseUp: dragInput_!=0";
        removeChildComponent(dragInput_);
        delete dragInput_;

        MouseEvent ev=e.getEventRelativeTo(this);
        int x=ev.x;
        int y=ev.y;

        Trunk* t=getTrunkAt(x,y);
        if(t!=0 && t->isAssemblyNeighbour(currentTrunk_))
        {
            currentInput_->onMoveEnd();
        }
        else
        {
            pic::logmsg()<<"drag ended off the current trunk section or neighbours: apertureIndex="<<currentInput_->getApertureIndex();
            pic::logmsg()<<"**** Need to adjust apertureIndex";
            MouseEvent evtrunk=e.getEventRelativeTo(currentTrunk_);
            currentInput_->setPos(evtrunk.x,evtrunk.y,false); 
            thingChanged();
        }

        currentTrunk_=0;
        currentInput_=0;
        dragInput_=0;

        selectedWire_->setSelected(false);
    }
    endThingChanged();  
}

void MainComponent::trunkInputMouseExit(const MouseEvent& e)
{
    juce::Rectangle <float> u;
    selectedWire_->getPathBounds(u);
    selectedWire_->setSelected(false);
    TrunkInput * input=dynamic_cast<TrunkInput *> (e.originalComponent);
    if(input!=0)
    {
        input->getTrunk()->repaint();
    }
    repaint((u.getSmallestIntegerContainer()).expanded(50*zoomFactor_,50*zoomFactor_));
 
}

bool MainComponent::isConnectionFrom(Wire* w,String srcAgent)
{
    Box* b =getTopLevelBox(w->getUsingSP());
    if(b!=0)
    {
        return b->getAtom()->has_protocol(srcAgent);
    }
    return false;
}


