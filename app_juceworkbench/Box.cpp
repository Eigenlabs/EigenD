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

#include "Box.h"
#include <cmath>

ExpandButton::ExpandButton(String name):ImageButton(name)
{
}

void ExpandButton::clicked(const ModifierKeys &modifiers)
{
    pic::logmsg()<<"expand button clicked";
    Box* b=findParentComponentOfClass<Box>();
    b->expandButtonClicked(modifiers);
}

BoxPos::BoxPos(String id, int x, int y):id_(id),x_(x),y_(y)
{
}

String BoxPos::getId()
{
    return id_;
}

int BoxPos::getX()
{
    return x_;
}

int BoxPos::getY()
{
    return y_;
}

Box::Box(Agent* agent,String id,int boxType,float zoomFactor)
    :expandButton(0), createButton_(0), deleteButton_(0), rigButton_(0)
{
    props_=agent->get_store();
    trueWidth_=BOXWIDTH;
    trueVisibleHeight_=BOXHEIGHT;
    zoomFactor_=zoomFactor;
    free_=true;
    selected_=false;
    foregrounded_=false;
    expandable_=false;
    rig_=agent->is_workspace();
    boxType_=boxType;
    usingChanged_=false;

    trueX_=props_->get_number("x");
    trueY_=props_->get_number("y");

    int x=ceil(((float)trueX_*zoomFactor)-0.5);
    int y=ceil(((float)trueY_*zoomFactor)-0.5);
    trueHeight_=trueVisibleHeight_;
    setBounds(x,y,ceil(((float)trueWidth_*zoomFactor_)-0.5),ceil(((float)trueHeight_*zoomFactor_)-0.5));

    String name=agent->get_root()->get_desc();
    doSetup(agent->get_root(),name,id,zoomFactor,TOPLEVEL);
    move_init_=false;

    if(props_->get_number("expanded")==1)
    {
        expanded=true;
    }
    else
    {
        expanded=false;
    }

    pic::logmsg()<<"Construct box "<<this <<" screenpos= ("<<getX()<<","<<getY()<<")"<<"  true pos=("<<trueX_<<","<<trueY_<<")zoomfactor="<<zoomFactor;
    checkPos();
}

Box::Box(Atom* atom,String name,String id,int childNo,int boxType,float zoomFactor=1,int level=0)
    :expandButton (0), createButton_(0), deleteButton_(0), rigButton_(0)
{
    props_=0;
    trueWidth_=BOXWIDTH;
    trueVisibleHeight_=BOXHEIGHT;
    zoomFactor_=zoomFactor;
    free_=true;
    selected_=false;
    foregrounded_=false;
    expandable_=false;
    rig_=false;
    boxType_=boxType;
    usingChanged_=false;

    trueX_=0;
    trueY_=childNo*trueVisibleHeight_;
    trueHeight_=trueVisibleHeight_;
    setBounds(0,(float)trueY_*zoomFactor,(float)trueWidth_*zoomFactor,(float)trueHeight_*zoomFactor);
    doSetup(atom,name,id,zoomFactor,level);
    move_init_=false;
}

Box::Box(String name,String id,int childNo,int boxType,float zoomFactor=1,int level=0, bool revconnect=false)
    :expandButton (0), createButton_(0), deleteButton_(0), rigButton_(0)
{
    props_=0;
    trueWidth_=BOXWIDTH;
    trueVisibleHeight_=BOXHEIGHT;
    zoomFactor_=zoomFactor;
    free_=true;
    selected_=false;
    foregrounded_=false;
    expandable_=false;
    rig_=false;
    boxType_=boxType;
    usingChanged_=false;
    revconnect_=revconnect; 

    trueX_=0;
    trueY_=childNo*trueVisibleHeight_;

    trueHeight_=trueVisibleHeight_;

    setBounds(0,(float)trueY_*zoomFactor_,(float)trueWidth_*zoomFactor_, (float)trueHeight_*zoomFactor_);
    atom_=0;
    doSetup(name,id,zoomFactor,level);
    move_init_=false;
}

void Box::doSetup(Atom* atom,String name,String id,float zoomFactor=1,int level=0)
{
    atom_=atom;
    revconnect_=false;
    if(atom_!=0 && atom_->has_protocol("revconnect"))
    {
        revconnect_=true;
    }


    doSetup(name,id,zoomFactor,level);
}

void Box::doSetup(String name,String id,float zoomFactor=1,int level=0)
{
	setName(name);
    expanded=false;
    level_=level;
    highlight_=false;
    mouseOver_=false;
    lassoed_=false;
    autoPositioned_=false;
    obscuredBoxTrueX_=0;
    obscuredBoxTrueY_=0;
    createInstanceName_=String::empty;

    id_=id;

    setupSourcePins();
    setupDestinationPins();
    setupButtons();
    titleBoxHeight_=(float)trueVisibleHeight_*zoomFactor_;
    moving_=false;
}

void Box::setupSourcePins()
{
    srcPin_=new SourcePin(id_,0,0,1.0f,false);
    if (isTopLevel())
    {
        srcPin_->setBounds(getWidth()-(16.0f*zoomFactor_), 0, 16.0f*zoomFactor_,getHeight());
    }
    else
    {
        srcPin_->setBounds(getWidth()-(30.0f*zoomFactor_), 0, 16.0f*zoomFactor_,getHeight());
    }
    addChildComponent(srcPin_);

    revSrcPin_=new ReverseSourcePin(id_,0,0,1.0f,false);

    revSrcPin_->setBounds((float)(getTrueBoxIndent())*zoomFactor_, 0, 16.0f*zoomFactor_,getHeight());
    addChildComponent(revSrcPin_);
    doSrcPinVisibility();
}

void Box::doSrcPinVisibility()
{
    if(revconnect_)
    {
        if((atom_!=0) && atom_->is_output())
        {
            revSrcPin_->setVisible(true);
        }

        srcPin_->toBack();
    }
    else
    {
        if((atom_!=0) && atom_->is_output())
        {
            srcPin_->setVisible(true);
        }

        revSrcPin_->toBack();
    }
}


void Box::setupDestinationPins()
{
    dstPin_=new DestinationPin(id_,0,0,1.0f,false);

    dstPin_->setBounds((float)(getTrueBoxIndent())*zoomFactor_, 0, 16.0f*zoomFactor_,getHeight());
    addChildComponent(dstPin_);

    revDstPin_=new ReverseDestinationPin(id_,0,0,1.0f,false);
    if (isTopLevel())
    {
        revDstPin_->setBounds(getWidth()-(16.0f*zoomFactor_), 0, 16.0f*zoomFactor_,getHeight());
    }
    else
    {
        revDstPin_->setBounds(getWidth()-(30.0f*zoomFactor_), 0, 16.0f*zoomFactor_,getHeight());
    }
    addChildComponent(revDstPin_);
    doDstPinVisibility();
}

void Box::doDstPinVisibility()
{
    if(revconnect_)
    {
        if((atom_!=0) && atom_->is_input())
        {
            revDstPin_->setVisible(true);
        }
        if(isSingleInput())
        {
            revDstPin_->setVisible(true);
        }

        dstPin_->toBack();
    }
    else
    {
        if((atom_!=0) && atom_->is_input())
        {
            dstPin_->setVisible(true);
        }
        if(isSingleInput())
        {
            dstPin_->setVisible(true);
        }

        revDstPin_->toBack();
    }
}

void Box::refreshPinVisibility()
{
    doSrcPinVisibility();
    doDstPinVisibility();
}

void Box::setupButtons()
{
    addAndMakeVisible(expandButton=new ExpandButton(String::empty));
    if (isExpandable())
    {
        doExpandableSetup();
    }

    else
    {
        doNonExpandableSetup();
    }

    if(canCreate())
    {
        pic::logmsg()<<"Box canCreate "<<std::string(getId().toUTF8());
        if(atom_!=0)
        {
            atom_->get_instanceName();
        }

        addAndMakeVisible (createButton_ = new TextButton ("createButton"));
        createButton_->setTooltip ("Create");
        createButton_->setButtonText ("+");
        createButton_->addListener (this);
        createButton_->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));
    }

    if (atom_!=0)
    {
        if(atom_->has_protocol("remove"))
        {
            addAndMakeVisible (deleteButton_ = new TextButton ("deleteButton"));
            deleteButton_->setTooltip ("Delete "+getName());
            deleteButton_->setButtonText ("-");
            deleteButton_->addListener (this);
            deleteButton_->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));
        }
    }
    if (rig_)
    {
        addAndMakeVisible (rigButton_ = new TextButton ("rigButton"));
        rigButton_->setTooltip ("Display the internal agents and wiring of this rig");
        rigButton_->setButtonText ("Open");
        rigButton_->addListener (this);
        rigButton_->setColour (TextButton::buttonColourId, Colour (0xffaeaeae));

    }
    sizeButton();

}

Box::~Box()
{	
    if (atom_!=0)
    {
        delete atom_;
    }
    clearStoredPositions();
    //std::cout<<"Box destructor "<<getId()<<"  "<<this<<std::endl;
}

bool Box::isRig()
{
    return rig_;
}

bool Box::isReversed()
{
    return revconnect_;
}

void Box::setCreateInstanceName(String name)
{
    createInstanceName_=name;
    setCreateButtonTooltip(createInstanceName_);
}

void Box::setCreateButtonTooltip(String name)
{
    createButton_->setTooltip("Create new "+ name);
}


int Box::getTrueX()
{
    return trueX_;
}

int Box::getTrueY()
{
    return trueY_;
}
int Box::getTrueHeight()
{
    return trueHeight_;
}

void Box::addObscuredBox(String id)
{
    std::set<String>::iterator i;
    i=obscuredBoxIds_.find(id);
    if(i==obscuredBoxIds_.end())
    {
        obscuredBoxIds_.insert(id);
    }
}

void Box::cachePosition(String id)
{
    obscuredBoxTrueX_=trueX_;
    obscuredBoxTrueY_=trueY_;
    BoxPos* bp=new BoxPos(id,trueX_,trueY_);
    obscuredBoxPositions_.push_back(bp); 
}

std::set<String> Box::getObscuredBoxIds()
{
    return obscuredBoxIds_;
}

int Box::getObscuredBoxTrueX()
{
    return obscuredBoxTrueX_;
}

int Box::getObscuredBoxTrueY()
{
    return obscuredBoxTrueY_;
}
void Box::clearObscuredBoxIds()
{
    obscuredBoxIds_.clear();
}

void Box::setAutoPositioned(bool val)
{
    autoPositioned_=val;
}

bool Box::isAutoPositioned()
{
    return autoPositioned_;
}

bool Box::isForegrounded()
{
    return foregrounded_;
}

bool Box::wiringAllowed()
{
    if(atom_!=0)
    {
        return !(atom_->has_protocol("hidden-connection"));

    }
    return true;
}

void Box::lasso(bool lassoed)
{
    if(lassoed)
    {
        setSelected(true);
    }
    else
    {
        setSelected(false);
    }

    lassoed_=lassoed;
    repaint();
}

PropertyStore* Box::get_props()
{
    return props_;
}

void Box::refresh()
{
    pic::logmsg()<<"Box "<<std::string(getId().toUTF8())<<" refresh";
//    std::cout<<"Box "<<std::string(getId().toUTF8())<<" refresh"<<std::endl;
    refreshButton();

    if(isSingleInput())
    {
        Box* const b =dynamic_cast <Box *>(getParentComponent());
        if (b!=0)
        {
            b->refreshExpanded();
        }
    }
}

void Box::refreshButton()
{
    if(!expandable_ && isExpandable())
    {
        doExpandableSetup();
        repaint();
    }
    else if(expandable_ && !isExpandable())
    {
        doNonExpandableSetup();
        repaint();
    }
}

void Box::refreshExpanded()
{
    pic::logmsg()<<"Box "<<std::string(getId().toUTF8())<<" refreshExpanded: expanded="<<expanded ;
    if(expanded)
    {
        expanded=!expanded;
        doContract(false);

        if(isExpandable())
        {
            pic::logmsg()<<"   isExpandable="<<isExpandable();
            expanded=!expanded;
            doExpand(true,true);
        }   
    }

    refreshButton();
}

void Box::refreshExpand()
{
    if(isExpandable())
    {
        pic::logmsg()<<"   isExpandable="<<isExpandable();
        expanded=!expanded;
        doExpand(true,true);
    } 

    refreshButton();
}

bool Box::canCreate()
{
    if(atom_!=0)
    {
        return atom_->has_protocol("create");
    }
    return false;    
}

bool Box::isExpanded()
{
    return expanded;
}

bool Box::isExpandable()
{
    if(atom_==0)
    {
        return false;
    }

    return (atom_->child_count()>0)||(atom_->get_numInputs()>0);
}

String Box::get_fulldesc()
{
    if(atom_!=0)
    {
        return atom_->get_fulldesc();
    }
    return String();
}

void Box::doExpandableSetup()
{
    expandable_=true;
    expandButton->addListener(this);
    doExpandableButton();
}

void Box::doNonExpandableSetup()
{
    expandable_=false;
    doNonExpandableButton();
}

void Box::doExpandedButton()
{
   Image img1=ImageCache::getFromMemory(CustomCursor::down_arrow_png,CustomCursor::down_arrow_pngSize);
   Image img2=ImageCache::getFromMemory(CustomCursor::right_arrow_png,CustomCursor::right_arrow_pngSize);
   expandButton->setImages(false,true,true,img1,1.000f, Colour(0x0),img1,1.000f,Colour(0x0),img2,1.000f,Colour(0x0));
}

void Box::doExpandableButton()
{
    Image img1=ImageCache::getFromMemory(CustomCursor::down_arrow_png,CustomCursor::down_arrow_pngSize);
    Image img2=ImageCache::getFromMemory(CustomCursor::right_arrow_png,CustomCursor::right_arrow_pngSize);
    expandButton->setImages(false,true,true,img2,1.000f, Colour(0x0),img2,1.000f,Colour(0x0),img1,1.000f,Colour(0x0));

}

void Box::doNonExpandableButton()
{
    Image img1=ImageCache::getFromMemory(CustomCursor::dot_png,CustomCursor::dot_pngSize);
    Image img2=ImageCache::getFromMemory(CustomCursor::dot_png,CustomCursor::dot_pngSize);
    expandButton->setImages(false,true,true,img1,1.000f, Colour(0x0),img1,1.000f,Colour(0x0),img2,1.000f,Colour(0x0));
}

bool Box::isTopLevel()
{
    return boxType_==TOPLEVEL;
}

bool Box::isChild()
{
    return boxType_==CHILD;
}

bool Box::isSingleInput()
{
    return boxType_==SINGLE_INPUT;
}

int Box::getSrcX()
{
    if (isTopLevel())
    {
        return trueX_+trueWidth_-16;
    }
    else
    {
        return getTopLevelBox()->getTrueX()+(trueWidth_-16-14);
    }
}

int Box::getSrcY()
{

    Box* const b =dynamic_cast <Box *>(getParentComponent());
    if (b!=0)
    {
        return trueY_+b->getSrcY();
    }
    return trueY_;
}

int Box::getDstX()
{
    return getTopLevelBox()->getTrueX()+getTrueBoxIndent();
}

int Box ::getDstY()
{
    Box* const b =dynamic_cast <Box *>(getParentComponent());
    if (b!=0)
    {
        return trueY_+b->getDstY();
    }
    return trueY_;
}

void Box::addSrcWire(Wire* w)
{
    if(isTopLevel())
    {
        srcWires_.insert(std::pair<String, Wire*>(w->getId(),w));
    }
    else
    {
        pic::logmsg()<<"attempt to add srcwire to non-toplevelbox";
    }
}

void Box::addDstWire(Wire* w)
{
    if(isTopLevel())
    {
        dstWires_.insert(std::pair<String, Wire*>(w->getId(),w));
    }
    else
    {
        pic::logmsg()<<"attempt to add dstwire to non-toplevelbox";
    }
}

void Box::connectedBoxForegroundWires()
{
    for (std::map<String,Wire*>::iterator iter=srcWires_.begin();iter!=srcWires_.end();iter++)
    {
        Wire* w=iter->second;
        if((!w->isHidden())||(!w->isPending()))
        {
            DestinationPin* dp=w->getUsingDP();
            if(dp!=0 && !dp->isLoosePin())
            {
                Box* b=dp->findParentComponentOfClass<Box>();
                if (b!=0)
                {
                    Box* tlb=b->getTopLevelBox();
                    if(getMainPanel()->isSelected(tlb))
                    {
                        tlb->foregroundWires(true);
                    }
                }
           }
        }        
    }

    for (std::map<String,Wire*>::iterator iter=dstWires_.begin();iter!=dstWires_.end();iter++)
    {
        Wire* w=iter->second;
        if((!w->isHidden())||(!w->isPending()))
        {
            SourcePin* sp=w->getUsingSP();
            if(sp!=0 && !sp->isLoosePin())
            {
                Box* b=sp->findParentComponentOfClass<Box>();
                if (b!=0)
                {
                    Box* tlb=b->getTopLevelBox();
                    if(getMainPanel()->isSelected(tlb))
                    {
                        tlb->foregroundWires(true);
                    }
                }
           }
        }  
    }
}

bool Box::connectedToSelectedBox()
{
    std::set<String> boxlist;
    for (std::map<String,Wire*>::iterator iter=srcWires_.begin();iter!=srcWires_.end();iter++)
    {
        Wire* w=iter->second;
        if(!w->isPending())
        {
            boxlist.insert(w->getTopLevelDstId());
        }
    }
    for (std::map<String,Wire*>::iterator iter=srcWires_.begin();iter!=srcWires_.end();iter++)
    {
        Wire* w=iter->second;
        if(!w->isPending())
        {
            boxlist.insert(w->getTopLevelSrcId());
        }
    }

    for (std::set<String>::iterator iter=boxlist.begin();iter!=boxlist.end();iter++)
    {
        Box* b=getMainPanel()->getBoxById(*iter);
        if(b!=0)
        {
          if(getMainPanel()->isSelected(b))
          {
              return true;
          }
        }
    }
    return false;
}

void Box::removeSrcWire(Wire* w)
{
    if(isTopLevel())
    {
        //int n=srcWires_.erase(w->getId());
        srcWires_.erase(w->getId());
        //pic::logmsg()<<"removed "<<n<<" wires from srcWires_ in box "<< getId().toUTF8();
    }
    else
    {
        pic::logmsg()<< "attempt to remove srcWire from non-topLevel box";
    }
}

void Box::removeDstWire(Wire* w)
{
    if(isTopLevel())
    {
        dstWires_.erase(w->getId());
        //int n=dstWires_.erase(w->getId());
        //pic::logmsg()<<"removed "<<n<<" wires from dstWires_ in box "<< getId().toUTF8();
    }
    else
    {
        pic::logmsg()<< "attempt to remove dstWire from non-topLevel box";
    }
}

void Box::initMove()
{
    initBoxMove();
}

void Box::initBoxMove()
{
    for (std::map<String,Wire*>::iterator iter=srcWires_.begin();iter!=srcWires_.end();iter++)
    {
        Wire* w=iter->second;
        if(!w->isPending())
        {
            getMainPanel()->addToWireMoveList(w);
        }
    }
    for (std::map<String,Wire*>::iterator iter=dstWires_.begin();iter!=dstWires_.end();iter++)
    {
        Wire* w=iter->second;
        if(!w->isPending())
        {
            getMainPanel()->addToWireMoveList(w);
        }
    }
}

void Box::changed(PropertyStore* props)
{
    if (props!=0)
    {
        pic::logmsg()<<"Box propertyChanged"<<std::string(getId().toUTF8());
        props_=props;
        getMainPanel()->clearWireMoveList();
        initBoxMove();
        trueX_=props_->get_number("x");
        trueY_=props_->get_number("y");

        int x=ceil(((float)trueX_*zoomFactor_)-0.5);
        int y=ceil(((float)trueY_*zoomFactor_)-0.5);
        setTopLeftPosition(x,y);
        getMainPanel()->thingMoved();
        getMainPanel()->endThingMoved();

        if(props->get_number("expanded")==1 && !isExpanded())
        {
            expanded=true;
            doExpand(false,false);
        }
        else if(props->get_number("expanded")==0 && isExpanded())
        {
            expanded=false;
            doContract(false);
        }
    }
}

void Box::nameChanged()
{
    if (atom_!=0)
    {
        String name=atom_->get_desc();
        pic::logmsg()<<"Box::nameChanged "<<std::string(getId().toUTF8())<<" "<<std::string(name.toUTF8());
        setName(name);

        if(!isTopLevel())
        {
            Box* const b =dynamic_cast <Box *>(getParentComponent());
            if (b!=0)
            {
                b->refreshExpanded();
            }
        }
        else
        {
            repaint();
        }
    }
}

float Box::getTextIndent()
{
    return 0.2;
}

int Box::getTrueBoxIndent()
{
    return level_*HALFBOXHEIGHT;
}

String Box::getTooltip()
{
    if(getMainPanel()->getTool()==ToolManager::HELPTOOL)
    {
        if(atom_!=0)
        {
            String tooltip= atom_->get_tooltip();
            if(tooltip.isNotEmpty())
            {
                String tip=getName()+ ": "+tooltip;
                return tip;
            }
        }
    }

    return getName(); 
}

void Box::onDelete()
{
    if(!isTopLevel())
    {
        PropertyStore* props=getTopLevelBox()->get_props();
        if(props!=0)
        {
            props->remove_list_item("expandedlist",getId());
            getMainPanel()->save_props(props->get_string("id"));
        }
    }

    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box * b=(*iter); 
        if (b!=0)
        {
            getMainPanel()->deselect(b);
            getMainPanel()->clearCandidateWiringBox(b);
            getMainPanel()->removeBox(b->getId());
            b->onDelete();
        }
    }   
    deleteAllChildren();
}

void Box::onContract(bool clearExpandedList)
{
    if(!isTopLevel() && isExpanded() && clearExpandedList)
    {
        PropertyStore* props=getTopLevelBox()->get_props();
        if(props!=0)
        {
            props->remove_list_item("expandedlist",getId());
            getMainPanel()->save_props(props->get_string("id"));
        }
    }


    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box * b=(*iter); 
        if (b!=0)
        {
            getMainPanel()->deselect(b);
            getMainPanel()->clearCandidateWiringBox(b);
            getMainPanel()->testRemoveBox(b->getId());
            b->onContract(clearExpandedList);
        }
    }   
    deleteAllChildren();
}

int Box::getOverallTrueHeight()
{
    int h=trueVisibleHeight_;
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box * b=(*iter); 
        if (b!=0)
        {
            h=h+b->getOverallTrueHeight();
        }
    }
    return h;
}

int Box::getOverallBoxCount()
{
    int n=1;
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box * b=(*iter); 
        if (b!=0)
        {
            n=n+b->getOverallBoxCount();
        }
    }
    return n;
}

void Box::postLayout(int x,int y)
{
    float hh=(float)trueHeight_*zoomFactor_;
    int h=ceil(hh-0.5);
    int w=ceil(((float)trueWidth_*zoomFactor_)-0.5);

    setBounds(x,y,w,h);

    zoomButton();
    zoomPins();
    if(boxes_.size()>0)
    {
        zoomChildBoxes();
    }
    repaint();       
}

void Box::doZoom(float zoomFactor)
{

    if(isTopLevel())
    {
        zoomFactor_=zoomFactor;
        int x=ceil(((float)trueX_*zoomFactor_)-0.5);
        int y=ceil(((float)trueY_*zoomFactor_)-0.5);
        postLayout(x,y);
        //pic::logmsg()<<"doZoom box "<<this <<" screenpos= ("<<getX()<<","<<getY()<<")"<<"  true pos=("<<trueX_<<","<<trueY_<<") zoomFactor="<<zoomFactor_;
        checkPos();
    }
}

void Box::zoomChildBoxes()
{
    int w=std::floor((float)trueWidth_*zoomFactor_);
    int h=getHeight();
    int nb=0;
    int sbh=0;
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box* bb=(*iter);
        if(bb!=0)
        {
            nb=nb+bb->getTrueHeight()/trueVisibleHeight_;
        }
    }
    sbh=h/(nb+1);
    int r=h%(nb+1);
    int nextY=sbh;

    float deltaE=0.0f;
    float carryOver=0.0f;
    float de=0.0f;
    if (r!=0)
    {
        deltaE=(float)r/(float)(nb+1);
        de=deltaE-std::floor(deltaE);
        if(de<0.5)
        {
            nextY=nextY+std::floor(deltaE);
            carryOver=de;
        }
        else
        {
            nextY=nextY+ceil(deltaE);
            carryOver=deltaE-ceil(deltaE);
        }
    }
    titleBoxHeight_=nextY;
    //pic::logmsg()<<"Box zoom "<<getName().toUTF8()<<" sbh="<<sbh<<" r="<<r<<" deltaE="<<deltaE;

    unsigned count=1;
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box* bb=(*iter);
        if(bb!=0)
        {
            bb->setZoomFactor(zoomFactor_);
            int sh=sbh*((bb->getTrueHeight()/trueVisibleHeight_));
            int adj=0;
            float a=0.0f;
            if(r!=0)
            {
                a=((bb->getTrueHeight()/trueVisibleHeight_)*deltaE)+carryOver;
                float da=a-std::floor(a);
                if(da<0.5)
                {
                    adj=std::floor(a);
                    carryOver=a-(float)adj; 
                }
                else
                {
                    adj=ceil(a);
                    carryOver=da-ceil(da);
                }
            }

            sh=sh+adj;
            //pic::logmsg()<<"       "<<bb->getName().toUTF8()<<" sh="<<sh<<" a="<<a<<" adj="<<adj<<" carryOver="<<carryOver;

            // pic::logmsg()<<"       "<<" y set to "<<nextY<<" height set to "<<sh;
            if ((count==boxes_.size()) && (carryOver>0.999))
            {
                sh=sh+1;
            }
            else if((count==boxes_.size())&& (carryOver<-0.999))
            {
                sh=sh-1;
            }

            bb->setBounds(0,nextY,w,sh);
            nextY=nextY+sh;
            bb->zoomButton();
            bb->zoomPins();
            count++;
        }
    }

    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box * bb=(*iter); 
        if (bb!=0)
        {
            bb->zoomChildBoxes();
        }
    }

}

void Box::setZoomFactor(float zoomFactor)
{
    zoomFactor_=zoomFactor;
}

void Box::zoomButton()
{
    sizeButton();
}

void Box::sizeButton()
{
    if(expandButton!=0)
    {
        if (isTopLevel())
        {
            expandButton->setBounds(ceil(16.0f*zoomFactor_),ceil(5.6f*zoomFactor_),ceil(20.0f*zoomFactor_),ceil(16.8*zoomFactor_));
        }
        else
        {
            expandButton->setBounds(ceil((float)(16+getTrueBoxIndent())*zoomFactor_),ceil(5.6f*zoomFactor_),ceil(20.0f*zoomFactor_),ceil(16.8f*zoomFactor_));
        }
    }

    if (createButton_!=0)
    {
        float dx=18.0;
        int x=trueWidth_-50;
        int y=0.1f*(trueVisibleHeight_*zoomFactor_);
        int dy=0.4f*(trueVisibleHeight_*zoomFactor_);
        createButton_->setBounds((float)x*zoomFactor_,y,dx*zoomFactor_,dy);
    }

    if (deleteButton_!=0)
    {
        float dx=18.0;
        int x =trueWidth_-50;
        int y=0.5f*(trueVisibleHeight_*zoomFactor_);
        int dy=0.4f*(trueVisibleHeight_*zoomFactor_);
        deleteButton_->setBounds((float)x*zoomFactor_,y,dx*zoomFactor_,dy);
    }

    if (rigButton_!=0)
    {
        float dx=38.0;
        int x=trueWidth_-60;
        int y=0.2f*((float)trueVisibleHeight_*zoomFactor_);
        int dy=((float)trueVisibleHeight_*zoomFactor_)-(2*y);
        rigButton_->setBounds(((float)x*zoomFactor_),y,dx*zoomFactor_,dy);
    }

}

void Box::zoomPins()
{
    float h=(float)trueVisibleHeight_*zoomFactor_;
    if(h>getHeight())
    {
        h=getHeight();
    }
    if(boxes_.size()==0)
    {
        h=getHeight();
    }

    if (isTopLevel())
    {
        srcPin_->setBounds(getWidth()-(16.0f*zoomFactor_), 0, ceil(16.0f*zoomFactor_),h);
        revDstPin_->setBounds(getWidth()-(16.0f*zoomFactor_), 0, ceil(16.0f*zoomFactor_),h);

    }
    else
    {
        srcPin_->setBounds(getWidth()-std::floor(30.0f*zoomFactor_), 0, ceil(16.0f*zoomFactor_),h);
        revDstPin_->setBounds(getWidth()-std::floor(30.0f*zoomFactor_), 0, ceil(16.0f*zoomFactor_),h);
    }

    dstPin_->setBounds((float)(16+getTrueBoxIndent())*zoomFactor_-std::floor(16.0f*zoomFactor_), 0, ceil(16.0f*zoomFactor_),h);
    revSrcPin_->setBounds((float)(16+getTrueBoxIndent())*zoomFactor_-std::floor(16.0f*zoomFactor_), 0, ceil(16.0f*zoomFactor_),h);

}

void Box::setTrueY(int y)
{
    trueY_=y;
}

void Box::setTrueHeight(int h)
{
    trueHeight_=h;
}

void Box::clearHighlights()
{
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box * bb=(*iter); 
        if (bb!=0)
        {
            bb->highlight(false);
            bb->clearHighlights();
        }
    }
}

void Box::layout()
{
    int dh=trueVisibleHeight_;
    int cumul_h=dh;

    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box * bb=(*iter); 
        if (bb!=0)
        {
            int nBox=bb->getOverallBoxCount();
            int trueY=cumul_h;
            bb->setTrueY(trueY);
            int trueHeight=nBox*dh;
            bb->setTrueHeight(trueHeight);
            cumul_h=cumul_h+trueHeight;
        }
    }

    int nBox=getOverallBoxCount();
    trueHeight_=nBox*dh;

    parentLayout();
    if(isTopLevel())
    {
        postLayout(getX(),getY());
    }
}

void Box::parentLayout()
{
    Box* const b =dynamic_cast <Box *>(getParentComponent());
    if (b!=0)
    {
        b->layout();
    }
}

Box * Box::getTopLevelBox()
{
    if(isTopLevel())
    {
        return this;
    }
    else
    {
        Box* b =dynamic_cast<Box *>(getParentComponent());
        if (b!=0)
        {
            return b->getTopLevelBox();
        }
    }
    return 0;
}

void Box::addInputBox(String s)
{
        int numChildBoxes=boxes_.size();
        String name ="No channel set";
        String id= getId()+ "["+s+"]";
        pic::logmsg()<<"adding single InputBox "<<std::string(name.toUTF8())<<" "<<std::string(id.toUTF8());
        Box* b= new Box (name,id,(numChildBoxes+1),Box::SINGLE_INPUT,zoomFactor_,level_+1, revconnect_);
        boxes_.push_back(b);
        addAndMakeVisible (b);
        trueHeight_=(boxes_.size()+1)*trueVisibleHeight_;
        setSize((float)trueWidth_*zoomFactor_,(float)trueHeight_*zoomFactor_); 
        layout();
}

void Box::addChildBox(Atom* atom)
{
    if(atom!=0)
    {
        int numChildBoxes=boxes_.size();
        String name =atom->get_desc();
        String id= atom->get_id();
        Box* b= new Box (atom,name,id,(numChildBoxes+1),Box::CHILD,zoomFactor_,level_+1);
        getMainPanel()->addBox(id,b);
        boxes_.push_back(b);
        addAndMakeVisible (b);
        trueHeight_=(boxes_.size()+1)*trueVisibleHeight_;
        setSize((float)trueWidth_*zoomFactor_,(float)trueHeight_*zoomFactor_); 
        pic::logmsg()<<"adding ChildBox "<<std::string(name.toUTF8());
        layout();

        PropertyStore* props=b->getTopLevelBox()->get_props();
        if(props!=0)
        {
            if(props->has_list_item("expandedlist",b->getId()))
            {
                b->setExpanded();    
            }
        }
    }
}

void Box::doExpand(bool bringToFront, bool saveProps)
{
   
   if(bringToFront)
   {
       getTopLevelBox()->bringToFront();
   }

   pic::logmsg()<<"Box "<<std::string(getId().toUTF8())<<" expanded";
   doExpandedButton();

   if (atom_!=0)
   {
       unsigned n=atom_->child_count();
       pic::logmsg()<<"child_count ="<<n;
       if(n>0)
       {
           for(unsigned i=0; i<n ;i++)
           {
                addChildBox(atom_->get_child(i));
           }

       }

       else if (atom_->is_input()) 
       {
         pic::logmsg()<<"is_input";
         int numInputs=atom_->get_numInputs();
         pic::logmsg()<<"num_inputs="<<numInputs;
         for(int i =1; i<=numInputs;i++)
         {
             addInputBox(String(i));
         }
       }
    }

    getTopLevelBox()->invalidateSingleInputBoxes();
    getTopLevelBox()->invalidateWires();
    getTopLevelBox()->preDrawWires();
    if(saveProps)
    {
        setExpandedProps();
    }

    if(bringToFront)
    {
        getTopLevelBox()->doMoveObscuredBoxes();
    }
}

void Box::doMoveObscuredBoxes()
{
    if(isTopLevel())
    {
        getMainPanel()->moveObscuredBoxes(this);
    }
}

void Box::doReturnObscuredBoxes()
{
    if(isTopLevel())
    {
        getMainPanel()->returnObscuredBoxes(this);
    }
}

void Box:: invalidateWires()
{
    pic::logmsg()<<"invalidateWires";
    for (std::map<String,Wire*>::iterator iter=srcWires_.begin();iter!=srcWires_.end();iter++)
    {
        Wire* w=iter->second;
        w->setForcedChange(true);
        w->routeChanged(true,false);
        w->setForcedChange(false);
    }

    for (std::map<String,Wire*>::iterator iter=dstWires_.begin();iter!=dstWires_.end();iter++)
    {
        Wire* w=iter->second;
        w->setForcedChange(true);
        w->routeChanged(false,true);
        w->setForcedChange(false);
    }

}

void Box:: forceInvalidateWires()
{
//    pic::logmsg()<<"forceInvalidateWires";
    for (std::map<String,Wire*>::iterator iter=srcWires_.begin();iter!=srcWires_.end();iter++)
    {
        Wire* w=iter->second;
//        pic::logmsg()<<"    ForceInvalidating "<<w->getId().toUTF8();
//        std::cout<<"    ForceInvalidating "<<w->getId().toUTF8()<<std::endl;
        w->forceRouteChanged(true,false);
    }

    for (std::map<String,Wire*>::iterator iter=dstWires_.begin();iter!=dstWires_.end();iter++)
    {
        Wire* w=iter->second;
//        pic::logmsg()<<"    ForceInvalidating "<<w->getId().toUTF8();
//        std::cout<<"    ForceInvalidating "<<w->getId().toUTF8()<<std::endl;
        w->forceRouteChanged(false,true);
    }
}

void Box::doClickContract(bool clearExpandedList)
{
    pic::logmsg()<<"Box doClickContract"<<std::string(getId().toUTF8())<<" contracted";
    doContract(clearExpandedList);
}

void Box::doContract(bool clearExpandedList)
{

    pic::logmsg()<<"Box "<<std::string(getId().toUTF8())<<" contracted";
    doExpandableButton();

    if((atom_!=0) && atom_->is_output())
    {
        srcPin_->setVisible(true);
    }
    else
    {
        srcPin_->setVisible(false);
    }

    if((atom_!=0) && atom_->is_input())
    {
        dstPin_->setVisible(true);
        
    }
    else
    {
        dstPin_->setVisible(false);
    }

    boxes_.clear();

    getTopLevelBox()->removeDrawingWires();

    for (int i =getNumChildComponents();--i>=0;)
    {
        Box* const bb =dynamic_cast <Box *>(getChildComponent (i));
        if (bb!=0)
        {
            getMainPanel()->contractBox(bb, clearExpandedList);
        }
    }

    getTopLevelBox()->invalidateSingleInputBoxes();
    getTopLevelBox()->forceInvalidateWires();

    trueHeight_ =trueVisibleHeight_;
    setSize(ceil(((float)trueWidth_*zoomFactor_)-0.5),ceil(((float)trueHeight_*zoomFactor_)-0.5));
    parentLayout();

    getTopLevelBox()->preDrawWires();
    if(clearExpandedList)
    {
        setExpandedProps();
    }

    if(isTopLevel())
    {
        doReturnObscuredBoxes();
    }
}

void Box::preDrawWires()
{
    for (std::map<String,Wire*>::iterator iter=srcWires_.begin();iter!=srcWires_.end();iter++)
    {
        Wire* w=iter->second;
        w->preDraw();
    }

    for (std::map<String,Wire*>::iterator iter=dstWires_.begin();iter!=dstWires_.end();iter++)
    {
        Wire* w=iter->second;
        w->preDraw();
    }
    // XXX
    getMainPanel()->repaint();
}

void Box::printWireInfo()
{
    pic::logmsg()<<"Box "<<std::string(getId().toUTF8())<<"printWireInfo: No. of srcWires="<<srcWires_.size()<<"No. of dstWires="<<dstWires_.size();
}

void Box::removeDrawingWires()
{
//    pic::logmsg()<<"Box "<<getId().toUTF8()<<"removeDrawingWires: No. of srcWires="<<srcWires_.size()<<"No. of dstWires="<<dstWires_.size();
    for (std::map<String,Wire*>::iterator iter=srcWires_.begin();iter!=srcWires_.end();iter++)
    {
       Wire* w=iter->second;
       if (w->isLoose())
       {
           getMainPanel()->removeLooseWire(w);
       }
       else
       {
           getMainPanel()->removeDrawingWire(w);
       }
    }

    for (std::map<String,Wire*>::iterator iter=dstWires_.begin();iter!=dstWires_.end();iter++)
    {
       Wire* w=iter->second;
       if (w->isLoose())
       {
           getMainPanel()->removeLooseWire(w);
       }
       else
       {
           getMainPanel()->removeDrawingWire(w);
       }
    }
}
void Box::setExpanded()
{
    expanded=true;
    doExpand(false,true);
}

void Box::buttonClicked (Button* buttonThatWasClicked)
{

    if(buttonThatWasClicked==createButton_)
    {
        createButtonClicked();
    }
    else if(buttonThatWasClicked==deleteButton_)
    {
        deleteButtonClicked();
    }
    else if(buttonThatWasClicked==rigButton_)
    {
        rigButtonClicked();
    }
}

void Box::expandButtonClicked(const ModifierKeys& mods)
{
    setUsingChanged(false);
    if(isExpandable())
    {
        expanded=!expanded;
        if (expanded)
        {
            doExpand(true,true);

            if(getMainPanel()->getProperty("selectOnExpand",true ) && isTopLevel())
            {
                if(!selected_)
                {
                    getMainPanel()->addToSelectionBasedOnModifiers(this,mods);
                }
            }

        }
        else
        {
            doClickContract(true);
        }
    }
}

void Box::rigButtonClicked()
{
    if(rig_)
    {
       getMainPanel()->showWorkspace(getName(),atom_->get_scope(),atom_->get_absoluteID()); 
    }
}

void Box::createButtonClicked()
{
    getMainPanel()->createInstance(atom_, createInstanceName_);
}

void Box::deleteButtonClicked()
{
    pic::logmsg()<<"deleteInstance id="<<std::string(getId().toUTF8());
    bool doDelete=true;
    if(getMainPanel()->requiresDeleteConfirmation("Instance"))
    {
        DeleteInstanceConfirmation* da=new DeleteInstanceConfirmation(getName());
        DialogWindow::showModalDialog("Delete port",da,this,Colour(0xffababab),true);
        if(da->dontShowAgain_&& da->okPressed_)
        {
           getMainPanel()->setDeleteConfirmationRequired("Instance"); 
        }

        doDelete=da->okPressed_;
        delete da;
    }

    if(doDelete)
    {
        getMainPanel()->deleteInstance(atom_);
    }
}

void Box::setForeground(bool shouldBeForegrounded,bool includeConnections)
{
    foregrounded_=shouldBeForegrounded;
    if(foregrounded_)
    {
       setAlwaysOnTop(true);
       getMainPanel()->getDisplayLayer()->toBack();
       if(includeConnections)
       {
           foregroundWires(true);
       }
    }

    else
    {
       setAlwaysOnTop(false);
       toBehind(getMainPanel()->getDisplayLayer());
       if(includeConnections)
       {
           foregroundWires(false);
       }
    }
    //XXX
    getMainPanel()->repaint();
}

void Box::foregroundWires(bool shouldBeForegrounded)
{
    std::set<String> boxlist;
    for (std::map<String,Wire*>::iterator iter=srcWires_.begin();iter!=srcWires_.end();iter++)
    {
        Wire* w=iter->second;
        if(!w->isPending())
        {
            boxlist.insert(w->getTopLevelDstId());
            w->setForegrounded(shouldBeForegrounded);
        }
//      //if its a loose wire - should foreground the loosepin
    }
    for (std::map<String,Wire*>::iterator iter=dstWires_.begin();iter!=dstWires_.end();iter++)
    {
        Wire* w=iter->second;
        if(!w->isPending())
        {
            boxlist.insert(w->getTopLevelSrcId());
            w->setForegrounded(shouldBeForegrounded);
        }
//      //if its a loose wire - should foreground the loosepin
    }

    for (std::set<String>::iterator iter=boxlist.begin();iter!=boxlist.end();iter++)
    {
        Box* b=getMainPanel()->getBoxById(*iter);
        if(b!=0)
        {
            if(shouldBeForegrounded)
            {
                b->setForeground(shouldBeForegrounded,false);
            }
            else
            {
                if( (!(getMainPanel()->isSelected(b))) && (!(b->connectedToSelectedBox())))
                {
                    b->setForeground(shouldBeForegrounded,false);
                }
            }
        }
    }

    // XXX
    getMainPanel()->repaint();
}

bool Box::hasInputBoxes()
{
//   pic::logmsg()<<"hasInputBoxes:";
   if (atom_!=0)
   {
//    pic::logmsg()<<boxes_.size();
//    pic::logmsg()<<" "<<atom_->child_count();
       return (boxes_.size()>0) && (atom_->child_count()==0); 
   }
//   pic::logmsg()<<"atom_==0";
   return false;
}

Box* Box::getFreeInputBox()
{
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box* b=(*iter);
        if ((b!=0) && b->isFree())
        {
            b->setFree(false);
            return b;
        }
    }
    return 0;
}

Box* Box::getNamedInputBox(String name)
{
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box* b=(*iter);
        if ((b!=0) && b->getName()==name)
        {
            return b;
        }
    }
    return 0;
}

void Box:: setFree(bool f)
{
    if(!free_ && f)
    {
        setName("");
        repaint();
    }
    free_=f;
}

bool Box::isFree()
{
    return free_;
}

ReverseSourcePin* Box::getRevSrcPin()
{
    return revSrcPin_;
}

SourcePin* Box::getSrcPin()
{
    if(revconnect_)
    {
        return revSrcPin_;
    }

    return srcPin_;
}

ReverseDestinationPin* Box::getRevDstPin()
{
    return revDstPin_;
}

DestinationPin* Box::getDstPin()
{
    if(revconnect_)
    {
        return revDstPin_;
    }

    return dstPin_;
}

void Box::setSelected(bool selected)
{
    selected_=selected;
    if (selected)
    {
        setForeground(true,true);
    }
    else
    {
        if(isAutoPositioned())
        {
            returnToManualPosition();
        }
        
        setForeground(false,true);
    }
    repaint();
}

void Box::returnToManualPosition(String id)
{
    // XXX
    // is the id in obscuredBoxPositions_ ?
    if(wasObscuredBy(id))
    {
        String storedId=String::empty;
        int storedX=0;
        int storedY=0;
        do
        {
            BoxPos* bp=obscuredBoxPositions_.back();
            obscuredBoxPositions_.pop_back();
            storedId=bp->getId(); 
            storedX=bp->getX(); 
            storedY=bp->getY(); 
            delete bp;
        }
        while(!(storedId.equalsIgnoreCase(id)));
        doReturnToManualPosition(storedX,storedY);
    }
}

bool Box::wasObscuredBy(String id)
{
    for(std::vector<BoxPos*>::iterator i=obscuredBoxPositions_.begin();i!=obscuredBoxPositions_.end();i++)
    {
        if((*i)->getId().equalsIgnoreCase(id))
        {
            return true;
        }
    }
    return false;
}

void Box::returnToManualPosition()
{
    if(!obscuredBoxPositions_.empty())
    {
        BoxPos* bp=obscuredBoxPositions_.front();
        doReturnToManualPosition(bp->getX(),bp->getY());

        while(!(obscuredBoxPositions_.empty()))    
        {
            bp=obscuredBoxPositions_.back();
            obscuredBoxPositions_.pop_back();
            delete bp; 
        }
    }
}

void Box::clearStoredPositions()
{
    BoxPos* bp;
    while(!(obscuredBoxPositions_.empty()))    
    {
        bp=obscuredBoxPositions_.back();
        obscuredBoxPositions_.pop_back();
        delete bp; 
    }
}

void Box::doReturnToManualPosition(int x, int y)
{
    pic::logmsg()<<"doReturnToManualPosition x="<<x<<"  y="<<y;
    trueX_=x;
    trueY_=y;
    initBoxMove();
    setTopLeftPosition(floor(((float)trueX_*zoomFactor_)+0.5),floor(((float)trueY_*zoomFactor_)+0.5));
    doSetPositionProps();
    getMainPanel()->endThingMoved();
}

void Box::setScreenPos(int x, int y)
{
    setTopLeftPosition(x,y);
    trueX_=floor(((float)getX()/zoomFactor_)+0.5);
    trueY_=floor(((float)getY()/zoomFactor_)+0.5);
    doSetPositionProps();
}

bool Box::hitTest(int x, int y)
{
    if(inVisibleArea(x,y))
    {
       return true;
    }
    else
    {
      return childHitTest(x,y);
    }
}

bool Box::isOverPin(int x, int y)
{
    if(isTopLevel())
    {
       if((x<16.0f*zoomFactor_)||(x>(getWidth()-16.0f*zoomFactor_))) 
       {
            int miny=0.357*trueVisibleHeight_*zoomFactor_;
            int maxy=miny +(float)8*zoomFactor_;
            if((y>miny)&&(y<maxy))
            {
                return true;
            }
       }
    }
    else
    {
        float boxIndent=((float)getTrueBoxIndent())*zoomFactor_;
        if((x>boxIndent) && (x<boxIndent+16.0f*zoomFactor_) )
        {
            int miny=0.357*trueVisibleHeight_*zoomFactor_;
            int maxy=miny +(float)8*zoomFactor_;
            if((y>miny)&&(y<maxy))
            {
                if(dstPin_->isShowing())
                {
                    return true;
                }
            }
        }
        else if((x>(getWidth()-16.0f*zoomFactor_)-0.5f*(trueVisibleHeight_*zoomFactor_)))
        {
           int miny=0.357*trueVisibleHeight_*zoomFactor_;
            int maxy=miny +(float)8*zoomFactor_;
            if((y>miny)&&(y<maxy))
            {
                if(srcPin_->isShowing())
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Box::inVisibleArea(int x, int y)
{
    if(isTopLevel())
    {
       bool xInside= (x>(16.0f*zoomFactor_) && x<(getWidth()-16.0f*zoomFactor_));
       bool yInside= (y<trueVisibleHeight_*zoomFactor_);
       return ((xInside && yInside)||isOverPin(x,y));
    }
    else
    {
        float boxIndent=((float)getTrueBoxIndent())*zoomFactor_;
        int xmin=boxIndent+0.08*getWidth();
        int xmax=(getWidth()-16.0f*zoomFactor_)-0.5f*(trueVisibleHeight_*zoomFactor_);
        int ymin=0;
        float ymax=(float)trueVisibleHeight_*zoomFactor_;
        bool xInside= (x>xmin) && (x<xmax);
        bool yInside= (y>ymin) && ((float)y<=ymax);
        return ((xInside && yInside)||isOverPin(x,y));
    }
}

bool Box::inChildVisibleArea(int x, int y)
{
    float boxIndent=((float)getTrueBoxIndent())*zoomFactor_;
    int xmin=boxIndent+0.08*getWidth();
    int xmax=(getWidth()-16.0f*zoomFactor_)-0.5f*(trueVisibleHeight_*zoomFactor_);
    int ymin=getY();
    float ymax=getY()+((float)trueVisibleHeight_*zoomFactor_);
    bool xInside= (x>xmin) && (x<xmax);
    bool yInside= (y>ymin) && ((float)y<=ymax);
    return (xInside && yInside)||(isOverPin(x,y-getY()));
}

bool Box::posInBox(int x, int y)
{
    int xmin=getX();
    int xmax=getX()+getWidth();
    int ymin=getY();
    int ymax=getY()+getHeight();
    bool xInside=(x>xmin) && (x<xmax);
    bool yInside=(y>ymin) && (y<ymax);
    return xInside && yInside;
}

bool Box::childHitTest(int x, int y)
{
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box* childBox=*iter;
        if(childBox->posInBox(x,y))
        {
            if(childBox->inChildVisibleArea(x,y))
            {
                return true;
            }
            else
            {
                return childBox->childHitTest(x-childBox->getX(),y-childBox->getY());
            }
        }
    }
    return false;
}

Colour Box::getMovingColour(String description)
{
    if(description.equalsIgnoreCase("background"))
    {
        if(mouseOver_||highlight_)
        {
            return Colour(0xbbacd0ed);
            //g.setColour(Colour(0x88acd0ed));
        }
        else
        {
            return Colour(0xbbd6d6d6);
            //g.setColour(Colour(0x88d6d6d6));
        }
    }
    else if(description.equalsIgnoreCase("text"))
    {
        return Colour(0xbb343434);
    }
    else if(description.equalsIgnoreCase("topLevelBackground1"))
    {
        if(selected_||lassoed_)
        {
            return Colour(0xbbacd0ed);
        }
        else if(mouseOver_||highlight_)
        {
            return Colour(0xbbacd0ed);
        }
        else
        {
            return Colour(0xbbd6d6d6);
        }
    }
    else if(description.equalsIgnoreCase("topLevelBackground2"))
    {
        if(selected_||lassoed_)
        {
            return Colour(0xbb53a2e6);
        }
        else if(mouseOver_||highlight_)
        {
            return Colour(0xbbacd0ed);
        }
        else
        {
            return Colour(0xbbafafaf);
        }
    }

    return Colour(0xbbafafaf);
}

bool Box::isMoving()
{
    if(isTopLevel())
    {
        return moving_;
    }

    return getTopLevelBox()->isMoving();
}

Colour Box::getColour(String description)
{
    if(isMoving())
    {
        return getMovingColour(description);
    }
    else if(description.equalsIgnoreCase("background"))
    {
        if(mouseOver_||highlight_)
        {
            return Colour(0xffacd0ed);
            //g.setColour(Colour(0x88acd0ed));
        }
        else
        {
            return Colour(0xffd6d6d6);
            //g.setColour(Colour(0x88d6d6d6));
        }
    }
    else if(description.equalsIgnoreCase("text"))
    {
        return Colour(0xff343434);
    }
    else if(description.equalsIgnoreCase("topLevelBackground1"))
    {
        if(selected_||lassoed_)
        {
            return Colour(0xffacd0ed);
        }
        else if(mouseOver_||highlight_)
        {
            return Colour(0xffacd0ed);
        }
        else
        {
            return Colour(0xffd6d6d6);
        }
    }
    else if(description.equalsIgnoreCase("topLevelBackground2"))
    {
        if(selected_||lassoed_)
        {
            return Colour(0xff53a2e6);
        }
        else if(mouseOver_||highlight_)
        {
            return Colour(0xffacd0ed);
        }
        else
        {
            return Colour(0xffafafaf);
        }
    }

    return Colour(0xffafafaf);
}


void Box::paint (Graphics& g)
{

    if(isTopLevel())
    {
        topLevelPaint(g);
    }
    else
    {
        if(getMainPanel()->showOutlines())
        {
            g.setColour (Colours::black);
            g.drawRect(0,0,getWidth(),getHeight(),1);
        }
        else
        {

            g.setColour(getColour("background"));
            float h=(float)trueVisibleHeight_*zoomFactor_;
            if(h>getHeight())
            {
                h=getHeight();
            }
            if(boxes_.size()==0)
            {
                h=getHeight();
            }
            float boxX=(float)(16+getTrueBoxIndent())*zoomFactor_;
            float boxW=getWidth()-(float)((46+getTrueBoxIndent()))*zoomFactor_;
            g.fillRoundedRectangle (boxX, 0.0f, boxW, h, 3.0f);
            g.setColour(getColour("text"));
            g.drawRoundedRectangle(boxX, 0.0f,boxW , h, 3.0f,(1.0f*zoomFactor_));


            if (highlight_)
            {
                g.setFont(Font(20.0f*zoomFactor_,Font::bold));
            }
            else
            {
                g.setFont(17.0f*zoomFactor_);
            }

            int leftTextBorder=15;
            int rightTextBorder=2;
            if(createButton_!=0 || deleteButton_!=0)
            {
                rightTextBorder=20;
            }

            int availableWidth=boxW-((leftTextBorder + rightTextBorder)*zoomFactor_);
            int textX=boxX+(leftTextBorder*zoomFactor_);

            g.drawText (truncateName(getName(),availableWidth,g),textX, h/14.0f,availableWidth , 24.0f*zoomFactor_,Justification::horizontallyCentred, true);
        }
    }
}

void Box::paintOverChildren(Graphics& g)
{
    if (!isTopLevel())
    {
        float h=(float)trueVisibleHeight_*zoomFactor_;
        if(h>getHeight())
        {
            h=getHeight();
        }
        if(boxes_.size()==0)
        {
            h=getHeight();
        }

        g.setColour(getColour("text"));
        g.drawRoundedRectangle((float)(16+getTrueBoxIndent())*zoomFactor_, 0.0f,getWidth()-(float)((46+getTrueBoxIndent()))*zoomFactor_ , h, 3.0f,(1.0f*zoomFactor_));
    }
    else
    {
        float titleBoxHeight;
        if (boxes_.size()==0)
        {
            titleBoxHeight=getHeight();
        }
        else
        {
             titleBoxHeight=titleBoxHeight_;
        }

        float h=titleBoxHeight-1;
        if (h>getHeight())
        {
            h=getHeight()-1;
        }
        g.setColour(getColour("text"));
        g.drawRoundedRectangle(16.0f*zoomFactor_, 1, getWidth()-(32.0f*zoomFactor_), h, 0.5f*titleBoxHeight,1.0f*zoomFactor_);

    }
}

void Box::topLevelPaint(Graphics& g)
{
    if(getMainPanel()->showOutlines())
    {
        g.setColour (Colours::red);
        g.drawRect(0,0,getWidth(),getHeight(),1);
    }
    float titleBoxHeight;
    if (boxes_.size()==0)
    {
        titleBoxHeight=getHeight();
    }
    else
    {
         titleBoxHeight=titleBoxHeight_;
    }

    g.setGradientFill (ColourGradient (getColour("topLevelBackground1"),0.0f, 0.25f*titleBoxHeight,getColour("topLevelBackground2"),(float) 0.0f, 0.75f*titleBoxHeight,false));
    float h1=titleBoxHeight;;
    if (h1>getHeight())
    {
        h1=getHeight();
    }

    g.fillRoundedRectangle (16.0f*zoomFactor_, 0, getWidth()-(32.0f*zoomFactor_),h1 , 0.5f*titleBoxHeight);

    float h=titleBoxHeight-1;
    if (h>getHeight())
    {
        h=getHeight()-1;
    }
    g.setColour(getColour("text"));
    g.drawRoundedRectangle(16.0f*zoomFactor_, 1, getWidth()-(32.0f*zoomFactor_), h, 0.5f*titleBoxHeight,1.0f*zoomFactor_);

    if (highlight_)
    {
        g.setFont(Font(20.0f*zoomFactor_,Font::bold));
    }
    else
    {
        g.setFont(20.0f*zoomFactor_);
    }

    int availableWidth=(trueWidth_-32-48)*zoomFactor_;
    g.drawText (truncateName(getName(),availableWidth,g),(16.0+24.0)*zoomFactor_, 2.0f*zoomFactor_, availableWidth, 24.0f*zoomFactor_,Justification::horizontallyCentred, true);

}

String Box::truncateName(String name, int availableWidth,Graphics& g)
{
    Font f=g.getCurrentFont();
    int l =name.length();
    if (l>=2)
    {
        int i=(2*l/3);
        String s=name.substring(0,i);
        String e=name.substring(i);
        String t=s+e;
        
        while((f.getStringWidth(t)>availableWidth) && (e.length()>2) && (s.length()>2))
        {
            e=e.substring(1);
            s=s.dropLastCharacters(1);
            t=s+"..."+e;
        }
        return t;
    }
    return name;
}

void Box::changeName(String name)
{
    setName(name);
    repaint();
}

void Box::mouseDown (const MouseEvent& e)
{
	mouseDownX=getX();
	mouseDownY=getY();
    int tool =getMainPanel()->getTool();

    if (tool==ToolManager::MOVETOOL)
    {
        if(getMainPanel()->getSelectedWire()!=0)
        {
            getMainPanel()->mouseDown(e.getEventRelativeTo(getMainPanel()));
        }
        else if (isTopLevel())
        {
            if(lassoed_)
            {
                getMainPanel()->initSelectedItemMove();
                bringToFront();
            }
            else
            {
                getMainPanel()->addToSelectionBasedOnModifiers(this, e.mods);
                bringToFront();
                if(!move_init_)
                {
                    getMainPanel()->clearWireMoveList();
                    initBoxMove();
                    move_init_=true;
                }
                getMainPanel()->thingMoved();
            }
        }
        else
        {
            getTopLevelBox()->mouseDown(e.getEventRelativeTo(getTopLevelBox()));
        }   
    }

    else if(tool==ToolManager::ZOOMTOOL)
    {
        getMainPanel()->mouseDown(e.getEventRelativeTo(getMainPanel()));
    }

    else if(tool==ToolManager::WIRINGTOOL)
    {
        if(srcPin_->isShowing())
        {
            srcPin_->mouseDown(e.getEventRelativeTo(srcPin_));
        }
        else if(revDstPin_->isShowing())
        {
            revDstPin_->mouseDown(e.getEventRelativeTo(revDstPin_));
        }
    }
    else if(tool==ToolManager::WIRING_LEFT)
    {
        if(dstPin_->isShowing())
        {
            dstPin_->mouseDown(e.getEventRelativeTo(dstPin_));
        }
        else if(revSrcPin_->isShowing())
        {
            revSrcPin_->mouseDown(e.getEventRelativeTo(revSrcPin_));
        }
    }

}

void Box::mouseUp (const MouseEvent& e)
{
    move_init_=false;
	ModifierKeys m=e.mods;
	if (m.isPopupMenu())
	{	
        getMainPanel()->showTools(e);
	}
    else
    {
        if (e.mouseWasClicked())
        {
            doMouseClick(e);
        }

        else if (getMainPanel()->getTool()==ToolManager::MOVETOOL)
        {
            moveToolMouseUp(e);
        }       

        else if (getMainPanel()->getTool()==ToolManager::ZOOMTOOL)
        {
            getMainPanel()->mouseUp(e.getEventRelativeTo(getMainPanel()));
        }

        else if(getMainPanel()->getTool()==ToolManager::WIRINGTOOL)
        {
            if(srcPin_->isShowing())
            {
                srcPin_->mouseUp(e.getEventRelativeTo(srcPin_));
            }
            else if(revDstPin_->isShowing())
            {
                revDstPin_->mouseUp(e.getEventRelativeTo(revDstPin_));
            }

        }

        else if(getMainPanel()->getTool()==ToolManager::WIRING_LEFT)
        {
            if(dstPin_->isShowing())
            {
                dstPin_->mouseUp(e.getEventRelativeTo(dstPin_));
            }
            else if(revSrcPin_->isShowing())
            {
                revSrcPin_->mouseUp(e.getEventRelativeTo(revSrcPin_));
            }

        }

        if(isTopLevel())
        {
            moving_=false;
        }
    }
}   

void Box::moveToolMouseUp(const MouseEvent& e)
{
    if(getMainPanel()->getSelectedWire()!=0)
    {
        getMainPanel()->mouseUp(e.getEventRelativeTo(getMainPanel()));
    }

    else if(isTopLevel())
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
            pic::logmsg()<<"Box mouseup "<<props_;
            setAutoPositioned(false);
            setTrueBounds();
        }
        getMainPanel()->clearSelectedItemsIfOne();
        bringToFront(); 

        pic::logmsg()<<"moveToolMouseUp for box "<<this <<" screenpos= ("<<getX()<<","<<getY()<<")"<<"  true pos=("<<trueX_<<","<<trueY_<<") zoomFactor="<<zoomFactor_;
        checkPos();

    }
    else
    {
        getTopLevelBox()->mouseUp(e.getEventRelativeTo(getTopLevelBox()));
    }
    getMainPanel()->doToolChangeIfRequired();

}

void Box::checkPos()
{
    int testX=ceil(((float)trueX_*zoomFactor_)-0.5);
    int testY=ceil(((float)trueY_*zoomFactor_)-0.5);
    if((testX-getX())!=0)
    {
        pic::logmsg()<<"*** x failed "<<testX-getX()<<" (float)trueX_*zoomFactor="<<(float)trueX_*zoomFactor_<<" ceil="<<ceil((float)trueX_*zoomFactor_);
    }
    if((testY-getY())!=0)
    {
        pic::logmsg()<<"*** y failed "<<testY-getY()<<" (float)trueY_*zoomFactor="<<(float)trueY_*zoomFactor_<<" ceil="<<ceil((float)trueY_*zoomFactor_);
;
    }
}

void Box::doMouseClick(const MouseEvent& e)
{
   getTopLevelBox()->bringToFront();

    pic::logmsg() <<"click on Box "<<std::string(getId().toUTF8())<<" name="<<std::string(getName().toUTF8());
    if(isTopLevel())
    {
        pic::logmsg()<<"    screenpos= ("<<getX()<<","<<getY()<<")"<<"  true pos=("<<trueX_<<","<<trueY_<<") zoomFactor="<<zoomFactor_;
    }
    if(atom_!=0)
    {
        //pic::logmsg() <<" has hidden-connection protocol="<<atom_->has_protocol("hidden-connection");
        if(isTopLevel())
        {
            pic::logmsg() <<"dstWires_ size= "<<dstWires_.size();
            pic::logmsg() <<"srcWires_ size= "<<srcWires_.size();
            pic::logmsg() <<"wires_.size= "<<getMainPanel()->getNumWires();
            pic::logmsg() <<"pendingWires_ size"<<getMainPanel()->getNumPendingWires();
        }
        pic::logmsg() <<"srcPin wires_.size= "<<srcPin_->getNumWires();
        pic::logmsg() <<"dstPin wires_.size= "<<dstPin_->getNumWires();
        pic::logmsg() <<"revSrcPin wires_.size= "<<revSrcPin_->getNumWires();
        pic::logmsg() <<"revDstPin wires_.size= "<<revDstPin_->getNumWires();
    }

    int tool=getMainPanel()->getTool();

    switch(tool)
    {
        case ToolManager::MOVETOOL:
            moveToolMouseUp(e);
            break;

        case ToolManager::POINTERTOOL:
            pointerToolMouseClick(e);
            break;

        case ToolManager::SELECTTOOL:
            selectToolMouseClick(e);
            break;

        case ToolManager::ZOOMTOOL:
            getMainPanel()->doMouseClick(e.getEventRelativeTo(getMainPanel()));
            break;

        case ToolManager::DELETETOOL:
            deleteToolMouseClick(e);
            break;

        case ToolManager::EDITTOOL:
            editToolMouseClick(e);
            break;

        case ToolManager::CREATETOOL:
            createToolMouseClick(e);
            break;
    }
}

void Box::pointerToolMouseClick(const MouseEvent& e)
{
    selectToolMouseClick(e);
}


void Box::selectToolMouseClick(const MouseEvent& e)
{
    if(isTopLevel())
    {
        ModifierKeys m=e.mods;
        if(selected_ && !(m.isCommandDown()||m.isShiftDown()))
        {
            getMainPanel()->deselect(this);
        }
        else
        {
              getMainPanel()->addToSelectionBasedOnModifiers(this,m);
        }
        bringToFront();
    }
    else
    {
        Box* b=getTopLevelBox();
        if(b!=0)
        {
            b->selectToolMouseClick(e.getEventRelativeTo(b));
        }
    }
}

void Box::deleteToolMouseClick(const MouseEvent& e)
{
    if(isTopLevel())
    {
        bool doDelete=true;
        if(getMainPanel()->requiresDeleteConfirmation("Agent"))
        {
            DeleteAgentConfirmation* da=new DeleteAgentConfirmation(getName());
            DialogWindow::showModalDialog("Delete agent",da,this,Colour(0xffababab),true);
            if(da->dontShowAgain_&& da->okPressed_)
            {
               getMainPanel()->setDeleteConfirmationRequired("Agent"); 
            }

            doDelete=da->okPressed_;
            delete da;
        }

        if(doDelete)
        {
            if (expanded)
            {
                expanded=!expanded;
                doContract(false);
            }
            getMainPanel()->deleteAgent(this);
        }
    }
    else
    {
        Box* parentBox=findParentComponentOfClass<Box> ();
        if(parentBox!=0)
        {
            if(parentBox->canCreate())
            {
                bool doDelete=true;
                if(getMainPanel()->requiresDeleteConfirmation("Instance"))
                {
                    DeleteInstanceConfirmation* da=new DeleteInstanceConfirmation(getName());
                    DialogWindow::showModalDialog("Delete port",da,this,Colour(0xffababab),true);
                    if(da->dontShowAgain_&& da->okPressed_)
                    {
                       getMainPanel()->setDeleteConfirmationRequired("Instance"); 
                    }

                    doDelete=da->okPressed_;
                    delete da;
                }

                if(doDelete)
                {
                    Atom* parent=parentBox->getAtom();
                    parent->delete_instance(atom_->get_id());
                }
            }
        }
    }

}

void Box::editToolMouseClick(const MouseEvent& e)
{
    if (isSingleInput())
    {
      showConnectionInfo();
    }
    else
    {
      showProperties();
    }
}

void Box::createToolMouseClick(const MouseEvent& e)
{
    if(canCreate())
    {
        getMainPanel()->createInstance(atom_,createInstanceName_);
    }
}

void Box::mouseDoubleClick(const MouseEvent& e)
{
    if (getMainPanel()->getTool()==ToolManager::EDITTOOL)
    {
        showProperties();
    }
}

void Box::setTrueBounds()
{
    trueX_=floor(((float)getX()/zoomFactor_)+0.5);
    trueY_=floor(((float)getY()/zoomFactor_)+0.5);
    pic::logmsg()<<"SetTrueBounds for box "<<this <<" screenpos= ("<<getX()<<","<<getY()<<")"<<"  true pos=("<<trueX_<<","<<trueY_<<") zoomFactor="<<zoomFactor_;
    checkPos();
    doSetPositionProps();
}

void Box::getSrcWires(std::map<String, Wire*>& m)
{
    m=srcWires_;
}

void Box::getDstWires(std::map<String, Wire*>& m)
{
    m=dstWires_;
}

Atom* Box::getAtom()
{
    return atom_;
}


Box* const Box::getBoxAt(int x, int y)
{
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
        {
            Box * b=(*iter); 
            if (b!=0)
            {
                if (b->contains(juce::Point<int> (x-b->getX(),y-b->getY())))
                {
                    return b->getBoxAt(x-b->getX(),y-b->getY());
                }
            }
        }
    return this;
}


Box*  Box:: getBoxById(String id)
{
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box* b=*(iter);
        if (b!=0)
        {
            if (b->getId()==id)
            {
                return b;
            }

            Box* bb= b->getBoxById(id);
            if (bb!=0)
            {
                return bb;
            }
        }
    }
    return 0;
}

void Box::invalidateSingleInputBoxes()
{
    for (std::list<Box*>::const_iterator iter=boxes_.begin();iter!=boxes_.end();++iter)
    {
        Box* b=*(iter);
        if(b!=0)
        {
            if(b->isSingleInput())
            {
                 b->setFree(true);   
            }
            else
            {
                b->invalidateSingleInputBoxes();
            }
        }
    }
}


void Box::mouseMove(const MouseEvent& e)
{
    int tool =getMainPanel()->getTool();
    if(tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
    {
        doWiringHighlights(e);
// XXX cant get this working - seems to cause spurious mouseDrag event when do mouseDown see also mainComponent mouseMove
//        int newTool=getMainPanel()->getTool();
//        if(newTool==ToolManager::WIRINGTOOL)
//        {
//           srcPin_->selectWire();
//        }
//        else if(newTool==ToolManager::WIRING_LEFT)
//        {
//           dstPin_->selectWire();
//        }
    }
    else
    {
      getParentComponent()->mouseMove(e.getEventRelativeTo(getParentComponent()));
    }
}

void Box::mouseEnter(const MouseEvent& e)
{

    setMouseCursor(getMainPanel()->getCursor());

    int tool =getMainPanel()->getTool();

    if (tool==ToolManager::MOVETOOL)
    {
        moveToolMouseEnter(e);
    }

    else if(tool==ToolManager::DELETETOOL)
    {
        deleteToolMouseEnter(e);
    }

    else if(tool==ToolManager::EDITTOOL)
    {
        editToolMouseEnter(e);
    }

    else if (tool==ToolManager::CREATETOOL)
    {
        createToolMouseEnter(e);
    }
    else if(tool==ToolManager::POINTERTOOL)
    {
        pointerToolMouseEnter(e);
    }
    else if(tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
    {
        wiringToolMouseEnter(e);
    }
    else if(tool==ToolManager::HELPTOOL)
    {
        helpToolMouseEnter(e);
    }

}
void Box::helpToolMouseEnter(const MouseEvent& e)
{
    getTopLevelBox()->setMouseOver(true);    
    if(atom_!=0)
    {
        getMainPanel()->showHelpText(atom_->get_helptext());
    }
}

void Box::moveToolMouseEnter(const MouseEvent& e)
{
    getTopLevelBox()->setMouseOver(true);    
}

void Box::deleteToolMouseEnter(const MouseEvent& e)
{
    if(isTopLevel())
    {
        getTopLevelBox()->setMouseOver(true);    
    }
    else
    {
        Box* parentBox=findParentComponentOfClass<Box>();
        if(parentBox!=0)
        {
            if(parentBox->canCreate())
            {
                 setMouseOver(true);
            }
        }
    }
}

void Box::editToolMouseEnter(const MouseEvent& e)
{
    setMouseOver(true); 

}

void Box::createToolMouseEnter(const MouseEvent& e)
{
    if(canCreate())
    {
        setMouseOver(true);
    }

}

void Box::pointerToolMouseEnter(const MouseEvent& e)
{
    if(isTopLevel())
    {
        setMouseOver(true);
    }

}

void Box::wiringToolMouseEnter(const MouseEvent& e)
{
    getTopLevelBox()->clearHighlights();
    doWiringHighlights(e);
    highlight(true);
}

void Box::doWiringHighlights(const MouseEvent& e)
{

    int tool =getMainPanel()->getTool();

    if(revconnect_)
    {
        if(revSrcPin_->isVisible()&& !revDstPin_->isVisible())
        {
            if(tool!=ToolManager::WIRING_LEFT)
            {
                e.originalComponent->setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRING_LEFT));
                getMainPanel()->setTool(ToolManager::WIRING_LEFT);
            }
        }
        else if(revDstPin_->isVisible() && !revSrcPin_->isVisible())
        {

            if(tool!=ToolManager::WIRINGTOOL)
            {
                e.originalComponent->setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRINGTOOL));
                getMainPanel()->setTool(ToolManager::WIRINGTOOL);
            }
        }

        else if(revDstPin_->isVisible() && revSrcPin_->isVisible())
        {
            if(e.x<getWidth()*0.5)
            {
                if(tool!=ToolManager::WIRING_LEFT)
                {
                    e.originalComponent->setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRING_LEFT));
                    getMainPanel()->setTool(ToolManager::WIRING_LEFT);
                }
            }
            else if(e.x>=getWidth()*0.5)
            {
                if(tool!=ToolManager::WIRINGTOOL)
                {
                    e.originalComponent->setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRINGTOOL));
                    getMainPanel()->setTool(ToolManager::WIRINGTOOL);
                }
            }
        }

    }
    else
    {
        if(dstPin_->isVisible()&& !srcPin_->isVisible())
        {
            if(tool!=ToolManager::WIRING_LEFT)
            {
                //setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRING_LEFT));
                e.originalComponent->setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRING_LEFT));
                getMainPanel()->setTool(ToolManager::WIRING_LEFT);
            }
            //highlight(true);
            //dstPin_->selectWire();
        }
        else if(srcPin_->isVisible() && !dstPin_->isVisible())
        {

            if(tool!=ToolManager::WIRINGTOOL)
            {
                //setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRINGTOOL));
                e.originalComponent->setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRINGTOOL));
                getMainPanel()->setTool(ToolManager::WIRINGTOOL);
            }
            //highlight(true);
            //srcPin_->selectWire();
        }

        else if(srcPin_->isVisible() && dstPin_->isVisible())
        {
            if(e.x<getWidth()*0.5)
            {
                if(tool!=ToolManager::WIRING_LEFT)
                {
                    e.originalComponent->setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRING_LEFT));
                    //setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRING_LEFT));
                    getMainPanel()->setTool(ToolManager::WIRING_LEFT);
                }
                //highlight(true);
                //dstPin_->selectWire();
            }
            else if(e.x>=getWidth()*0.5)
            {
                if(tool!=ToolManager::WIRINGTOOL)
                {
                    //setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRINGTOOL));
                    e.originalComponent->setMouseCursor(getMainPanel()->getCursor(ToolManager::WIRINGTOOL));
                    getMainPanel()->setTool(ToolManager::WIRINGTOOL);
                }
                //highlight(true);
                //srcPin_->selectWire();
            }
        }
    }
}

void Box::mouseExit(const MouseEvent& e)
{
    int tool =getMainPanel()->getTool();
    if (tool==ToolManager::MOVETOOL)
    {
        getTopLevelBox()->setMouseOver(false);    
        //getMainPanel()->deselect(this);
    }
    else if(tool==ToolManager::DELETETOOL)
    {
        if(isTopLevel())
        {
            getTopLevelBox()->setMouseOver(false);    
        }
        else
        {
            setMouseOver(false);
        }
    }
    else if(tool==ToolManager::EDITTOOL)
    {
        setMouseOver(false);
    }
    else if(tool==ToolManager::CREATETOOL)
    {
        setMouseOver(false);
    }
    else if(tool==ToolManager::POINTERTOOL)
    {
        setMouseOver(false);
    }
    else if(tool==ToolManager::HELPTOOL)
    {
        setMouseOver(false);
    }

    else if(tool==ToolManager::WIRINGTOOL||tool==ToolManager::WIRING_LEFT)
    {
        highlight(false);
    }
}

void Box::setMouseOver(bool over)
{
    mouseOver_=over;
    repaint();
}

void Box::mouseDrag (const MouseEvent& e)
{
    int tool =getMainPanel()->getTool();

    if (tool==ToolManager::MOVETOOL)
    {
        if(getMainPanel()->getSelectedWire()!=0)
        {
            getMainPanel()->mouseDrag(e.getEventRelativeTo(getMainPanel()));
        }

        else if (isTopLevel())
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
       
                setTopLeftPosition(x,y);

                trueX_=floor(((float)getX()/zoomFactor_)+0.5);
                trueY_=floor(((float)getY()/zoomFactor_)+0.5);
     
                if ((!getMainPanel()->offCanvas(x,y,getWidth(),getHeight())) && (!getMainPanel()->inCentralViewPort(x,y)))
                {
                    getMainPanel()->autoComponentDrag(x,y,this);
                }
                else
                {
                    //pic::logmsg()<<"Box mousedrag calling stopAutoDrag()";
                    getMainPanel()->stopAutoDrag();
                }

                getMainPanel()->thingMoved();
            }
        }
        else
        {
            //pic::logmsg() <<"box mousedrag";
            getTopLevelBox()->mouseDrag(e.getEventRelativeTo(getTopLevelBox()));
        }
    }

    else if (tool==ToolManager::ZOOMTOOL)
    {
       getMainPanel()->mouseDrag(e.getEventRelativeTo(getMainPanel()));
    }

    else if(tool==ToolManager::WIRINGTOOL)
    {
        if(srcPin_->isShowing())
        {
            srcPin_->mouseDrag(e.getEventRelativeTo(srcPin_));
        }
        else if(revDstPin_->isShowing())
        {
            revDstPin_->mouseDrag(e.getEventRelativeTo(revDstPin_));
        }
    }

    else if(tool==ToolManager::WIRING_LEFT)
    {
        if(dstPin_->isShowing())
        {
            dstPin_->mouseDrag(e.getEventRelativeTo(dstPin_));
        }
        else if(revSrcPin_->isShowing())
        {
            revSrcPin_->mouseDrag(e.getEventRelativeTo(revSrcPin_));
        }
    }

}

void Box::doMove(const MouseEvent& e)
{
    if(isTopLevel())
    {
        moving_=true;
    }

    int x=mouseDownX+e.getDistanceFromDragStartX();
    int y=mouseDownY+e.getDistanceFromDragStartY();

    x=getMainPanel()->conditionX(x,getWidth());
    y=getMainPanel()->conditionY(y,getHeight());
    int dx=x-getX();
    int dy=y-getY();
    getMainPanel()->moveSelectedGroup(dx,dy);
    autoDragIfRequired(getX(),getY(), e.x+getX(), e.y+getY());
}

void Box::doSelectedGroupMove(int dx, int dy)
{
    if(isTopLevel())
    {
        moving_=true;
    }

    toFront(false);
    int x=getX()+dx;
    int y=getY()+dy;
    trueX_=floor(((float)x/zoomFactor_)+0.5);
    trueY_=floor(((float)y/zoomFactor_)+0.5);
    setAutoPositioned(false);
    setTopLeftPosition(x, y);
}

void Box::doSetPositionProps()
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

void Box::setExpandedProps()
{
    if (props_!=0)
    {
        if(expanded)
        {
            if( props_->get_number("expanded")!=1)
            {
                props_->set_number("expanded",1);
                getMainPanel()->save_props(props_->get_string("id"));
            }
        }
        else
        {
            props_->set_number("expanded",0);
            props_->remove_list("expandedlist");
            getMainPanel()->save_props(props_->get_string("id"));
        }

    }
    else
    {
        PropertyStore* props=getTopLevelBox()->get_props();
        if(props!=0)
        {
            if(expanded)
            {
                if(!props->has_list_item("expandedlist",getId()))
                {
                    props->add_list_item("expandedlist",getId());
                    getMainPanel()->save_props(props->get_string("id"));
                }
            }
            else
            {
                props->remove_list_item("expandedlist",getId());
                getMainPanel()->save_props(props->get_string("id"));
            }
        }
    }
}

void Box::autoDragIfRequired(int x, int y, int mx, int my)
{
    if ((!getMainPanel()->offCanvas(mx,my,getWidth(),getHeight())) && (!getMainPanel()->inCentralViewPort(mx,my)))
    {
        getMainPanel()->autoComponentDrag(x,y,this);
    }
    else
    {
        //pic::logmsg()<<"Box autodragIfRrequired calling stopAutoDrag";
        getMainPanel()->stopAutoDrag();
    }
}


void Box::doAutoDrag(int dx,int dy)
{
    //pic::logmsg() <<"Box doAutoDrag";
    mouseDownX=mouseDownX+dx;
    mouseDownY=mouseDownY+dy;
    int x=getX()+dx;
    int y=getY()+dy;
    x=getMainPanel()->conditionX(x,getWidth());
    y=getMainPanel()->conditionY(y,getHeight());

    if(getMainPanel()->offCanvas(x,y,getWidth(),getHeight()))
    {
//        getMainPanel()->stopAutoDrag();
    }
    else
    {
        if (lassoed_)
        {
            getMainPanel()->moveSelectedGroup(dx,dy);    
        }
        else
        {
            setTopLeftPosition(x,y);
            trueX_=floor(((float)getX()/zoomFactor_)+0.5);
            trueY_=floor(((float)getY()/zoomFactor_)+0.5);
            getMainPanel()->thingMoved();
        }
    }
}

void Box::showProperties()
{
    if (atom_!=0)
    {
        getMainPanel()->showProperties(this);
    }
}

void Box::showConnectionInfo()
{
    pic::logmsg()<<"show connection infomation for box "<<std::string(getId().toUTF8());

    if(isSingleInput())
    {
        Wire* w=dstPin_->getWire();
        if(w!=0)
        {
            pic::logmsg()<<"src_filter="<<std::string(w->get_srcFilter().toUTF8());
            String srcName=getMainPanel()->get_fulldesc(w->getSrcId());
            SingleInputEditor* editorPanel=new SingleInputEditor(w,srcName);
            String title=String::empty;

            Box* parentBox=findParentComponentOfClass<Box> ();
            if(parentBox!=0)
            {
                String fullname=parentBox->get_fulldesc();
                title = "Input editor: "+fullname;
            }

            DialogWindow::showModalDialog(title,editorPanel,this,Colour (0xffababab),true);
            
            if(editorPanel->changed())
            {
                pic::logmsg()<<"SingleInputEditor values were changed";
                if(parentBox!=0)
                {
                    parentBox->setUsingChanged(true);
                }

                String u=editorPanel->getUsing();
                String f=editorPanel->getFilter();
                String c=editorPanel->getControl();
                getMainPanel()->changeWire(w,u,f,c);
            }
           
            delete editorPanel;
        }
    }
}

void Box::setUsingChanged(bool val)
{
    usingChanged_=val;
}

bool Box::getUsingChanged()
{
    return usingChanged_;
}


String Box::getId()
{
    return id_;
}

void Box::highlight(bool highlight)
{
//
//    if (highlight)
//    {
//        pic::logmsg()<<"Highlight on "<<getName().toUTF8();
//    }
//    else
//    {
//        pic::logmsg()<<"Highlight off "<<getName().toUTF8();
//    }
//
    getTopLevelBox()->bringToFront();
    highlight_=highlight;
    repaint(); 
}

void Box::bringToFront()
{
   if(isTopLevel())
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
}

MainComponent* Box::getMainPanel() const throw()
{
    return findParentComponentOfClass <MainComponent> ();
}


