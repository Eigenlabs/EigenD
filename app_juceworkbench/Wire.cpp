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

#include "Wire.h"

Segment::Segment(Path p,int hash,Wire* parent)
{
    p_=p;
    hash_=hash;
    parent_=parent;
}

void Segment::draw(Graphics& g, float zoomFactor)
{
    float x1=0.0;
    float x2=0.0;
    float y1=0.0;
    float y2=0.0;
    if(!p_.isEmpty())
    {
        g.setColour(parent_->getWireColour());
//        g.strokePath(p_,PathStrokeType(2.0f),AffineTransform::scale(zoomFactor,zoomFactor));
        Path p=p_;
        p.applyTransform(AffineTransform::scale(zoomFactor,zoomFactor));
//        pic::logmsg()<<"Segment draw";
        bool firstPoint=true;
        int count=0;
        for(Path::Iterator i (p);i.next();)
        {
            switch (i.elementType)
            {
            case Path::Iterator::startNewSubPath:
//                pic::logmsg()<<"     startNewSubPath x="<<i.x1<<" y="<<i.y1;
                if(firstPoint)
                {
                    x1=(float)i.x1;
                    y1=(float)i.y1;
                    firstPoint=false;
                }
                count=0;
                break;
            case Path::Iterator::lineTo:
                if (count==2)
                {
//                    pic::logmsg()<<"     lineTo x="<<i.x1<<" y="<<i.y1;
                    x2=(float)i.x1;
                    y2=(float)i.y1;
//                    pic::logmsg()<<"     drawingline x1="<<x1<<" y1="<<y1<<" x2="<<x2<<" y2="<<y2;
                    g.drawLine(x1,y1,x2,y2,4.0f*zoomFactor);
                    x1=x2;
                    y1=y2;
                }
                count++;
                break;
            default:
                break;
            }
        }
    }
}

Path Segment::getPath()
{
    return p_;
}

int Segment::getHash()
{
    return hash_;
}

Wire::Wire(Connection* c,Workspace* w, MainComponent* mc)
{
    connection_=c;
    sid_=c->input();
    did_=c->output();
    id_= sid_+":"+did_;
    selected_=false;
    foregrounded_=false;
    hook_=0;
    hookPos_=0;
    props_=c->get_props();
    workspace_=w;
    using_dp_=0;
    using_sp_=0;
    dp_ready_=false;
    sp_ready_=false;
    mc_=mc;
    loose_=false;
    hashCode_=0;
    oldHashCode_=0;
    needsRouteCalculation_=true;
    dstUsing_="";
    srcFilter_="";

    forcedChange_=true;
    pending_=false;

    if (props_!=0)
    {
       if (props_->has_list("routing")) 
       {
           for(unsigned i=0;i<props_->list_getsize("routing");i++)
           {
                String rid=props_->get_list("routing",i);
                RoutingElement* r= mc_->getRoutingElement(rid);
                if(r!=0)
                {
                    pegs_.push_back(r);
                }
           }
       }
    }
    String u=connection_->get_using();
    if(u.isNotEmpty())
    {
    //    pic::logmsg() <<"**** using "<<u.toUTF8();
        set_dstUsing(u);
    }

    String f=connection_->get_master_filter();
    if(f.isNotEmpty())
    {
    //    pic::logmsg() <<"**** filter "<<f.toUTF8();
        set_srcFilter(f);
    }

    String ctrl=connection_->get_master_control();
    if(ctrl.isNotEmpty())
    {
    //    pic::logmsg() <<"**** control "<<ctrl.toUTF8();
        set_control(ctrl);
    }
    metronomeOutput_=false;
    controllerOutput_=false;
}

Wire::Wire(String id,Workspace* w, MainComponent* mc, PropertyStore* props)
{
    id_=id;
    sid_=id.upToFirstOccurrenceOf(":",false,false);
    did_=id.fromFirstOccurrenceOf(":",false,false);

    selected_=false;
    foregrounded_=false;
    hook_=0;
    hookPos_=0;
    props_=props;
    workspace_=w;
    using_dp_=0;
    using_sp_=0;
    dp_ready_=false;
    sp_ready_=false;
    mc_=mc;
    loose_=true;
    hashCode_=0;
    oldHashCode_=0;
    needsRouteCalculation_=true;
    dstUsing_="";
    srcFilter_="";
    control_="";
    forcedChange_=false;
    connection_=0;
    pending_=false;

    if (props_!=0)
    {
       if (props_->has_list("routing")) 
       {
           for(unsigned i=0;i<props_->list_getsize("routing");i++)
           {
                String rid=props_->get_list("routing",i);
                RoutingElement* r= mc_->getRoutingElement(rid);
                if(r!=0)
                {
                    pegs_.push_back(r);
                }
           }
       }
    }
    metronomeOutput_=false;
    controllerOutput_=false;
}

Wire::~Wire()
{	
    //pic::logmsg()<<"Wire destructor"<<id_.toUTF8();
    clearGridMap();
    std::list<RoutingElement *>::iterator iter=pegs_.begin();
    deleteRoute();
    while (iter!=pegs_.end())
    {
        (*iter)->removeId(id_);
        iter++;
    }
   
    if (props_!=0)
    {
        workspace_->remove_store(props_);
    }

    if(connection_!=0)
    {
        delete connection_;
    }
    //std::cout<<"end Wire destructor"<<id_.toUTF8()<<std::endl;
}

void Wire::setPending(bool shouldBePending)
{
    pending_=shouldBePending;
}

bool Wire::isPending()
{
    return pending_;
}

void Wire::removeFromPins()
{
    if(using_dp_!=0)
    {
        using_dp_->removeWire(this);
    }

    if (using_sp_!=0)
    {
       using_sp_->removeWire(this);
    }
}

String Wire::getDescription()
{
    if(connection_!=0)
    {
        return connection_->getDescription();
    }
    return String::empty;
}

String Wire::getTopLevelDstId()
{
    String wid=getId();
    String topLevelDstId=wid.fromFirstOccurrenceOf(":",false,true);
    return topLevelDstId.upToFirstOccurrenceOf(">",true,true);
}

String Wire::getTopLevelSrcId()
{
    String wid=getId();
    return wid.upToFirstOccurrenceOf(">",true,true);
}

void Wire::set_dstUsing(String u)
{
    dstUsing_=u;
}

String Wire::get_dstUsing()
{
    return dstUsing_;
}

void Wire::set_srcFilter(String f)
{
    srcFilter_=f;
}

String Wire::get_srcFilter()
{
    return srcFilter_;
}

void Wire::set_control(String c)
{
    control_=c;
}

String Wire::get_control()
{
    return control_;
}

void Wire::getPathBounds(juce::Rectangle <float>& u)
{
    u =(getWirePath().getBounds());
}

void Wire::getPathChangeBounds(juce::Rectangle <float>& u)
{
    u =(getWirePath().getBounds()).getUnion(getOldWirePath().getBounds());
}

void Wire::addRouting()
{
    pic::logmsg()<< "Wire "<<std::string(getId().toUTF8())<<" addRouting";
    //std::cout<< "Wire "<<getId().toUTF8()<<" addRouting"<<std::endl;
    PropertyStore* p= workspace_->get_store(id_);
    if (p!=0)
    {
        props_=p;
        pegs_.clear();
        if (props_->has_list("routing")) 
        {
           for(unsigned i=0;i<props_->list_getsize("routing");i++)
           {
                String rid=props_->get_list("routing",i);
                RoutingElement* r= mc_->getRoutingElement(rid);
                if(r!=0)
                {
                    pegs_.push_back(r);
                }
                else
                {
                    pic::logmsg()<<"routingElement not found";
                }
           }
           routeChanged(true,true); 
        }       
    }
    preDraw();
    mc_->repaintWire(this);
}

void Wire::changeRouting()
{
    pic::logmsg()<< "Wire "<<std::string(getId().toUTF8())<<" changeRouting";
    PropertyStore* p= workspace_->get_store(id_);
    if (p!=0)
    {
        props_=p;
        if (props_->has_list("routing")) 
        {
           pegs_.clear();
           for(unsigned i=0;i<props_->list_getsize("routing");i++)
           {
                String rid=props_->get_list("routing",i);
                RoutingElement* r= mc_->getRoutingElement(rid);
                if(r!=0)
                {
                    pegs_.push_back(r);
                }
                else    
                {
                    pic::logmsg()<<"routing element not found";
                }
           }
           routeChanged(true,true); 
        }       
    }
    preDraw();
    mc_->repaintReRoutedWire(this);
}

void Wire:: removeRouting()
{
    pic::logmsg()<< "Wire "<<std::string(getId().toUTF8())<<" removeRouting";
    pegs_.clear();
    routeChanged(true,true);
    workspace_->remove_store(props_); 
    props_=0;
    mc_->repaintWire(this);
}

DestinationPin* Wire::getUsingDP()
{
    return using_dp_;
}

SourcePin* Wire::getUsingSP()
{
    return using_sp_;
}

bool Wire::isSelected()
{
    return selected_;
}

bool Wire::isLoose()
{
    return loose_;
}

bool Wire::isForegrounded()
{
    return foregrounded_;
}

void Wire::foregroundOn()
{
    foregrounded_=true;
}

void Wire::foregroundOff()
{
    foregrounded_=false;
}

PropertyStore* Wire::getPropertyStore()
{
    return props_;
}

String Wire::getId()
{
    return id_;
}

String Wire::getUsingId()
{
    if(using_sp_!=0 && using_dp_!=0)
    {
        return using_sp_->getId()+":"+ using_dp_->getId();
    }
    return String();
}

String Wire::getUsingSpId()
{
    if(using_sp_!=0 && using_dp_!=0)
    {
        return using_sp_->getId();
    }
    return String();
}

String Wire::getUsingDpId()
{
    if(using_sp_!=0 && using_dp_!=0)
    {
        return using_dp_->getId();
    }
    return String();
}

int Wire::getRouteHash()
{
    return hashCode_;
}

int Wire::getOldRouteHash()
{
    return oldHashCode_;
}


void Wire::calcHashCode()
{
    oldHashCode_=hashCode_;
    String h=String();
    if(isRevConnect())
    {
        h=getUsingDpId();
        if(h.isNotEmpty())
        {
            for (std::list<RoutingElement*>::reverse_iterator iter=pegs_.rbegin();iter!=pegs_.rend();++iter)
            {
               h=h+(*iter)->get_reverse_hash(id_);
            }
        }
        h=h+getUsingSpId();
    }

    else
    {
        h=getUsingSpId();
        if(h.isNotEmpty())
        {
            for (std::list<RoutingElement*>::const_iterator iter=pegs_.begin();iter!=pegs_.end();++iter)
            {
               h=h+(*iter)->get_hash(id_);
            }
        }
        h=h+getUsingDpId();
    }

    hashCode_= h.hashCode();
}

bool Wire::isRevConnect()
{
   if(using_sp_!=0 && using_dp_!=0)
   {
       if(isLoose())
       {
           return (getDstPin()->isRevConnect()||getSrcPin()->isRevConnect());
       }
       else
       {
           return (getDstPin()->isRevConnect() && getSrcPin()->isRevConnect());
       }
   }
   return false;
}

String Wire::getSrcId()
{
    return sid_;
}

String Wire::getDstId()
{
    return did_;
}

DestinationPin * Wire::getDstPin()
{
    if (!dp_ready_)
    {
        using_dp_= mc_->getDstPinById(did_,this);
        if(using_dp_!=0)
        {
            using_dp_->addWire(this);
            dp_ready_=true;
        }
    }

    return using_dp_;
}   

SourcePin* Wire::getSrcPin()
{
    if (!sp_ready_)
    {

        using_sp_=mc_->getSrcPinById(sid_);
        if (using_sp_!=0)
        {
            using_sp_->addWire(this);
            sp_ready_=true;
        }
    }
    return using_sp_;
}

void Wire::addHook(Hook* hook)
{
    hook_=hook;
    hookPos_=0;
}

Hook* Wire::getHook()
{
    return hook_;
}
void Wire::removeHook()
{
    hook_=0;
}
int Wire::getNumRoutingElements()
{
    return pegs_.size();
}

RoutingElement*  Wire::getLastRoutingElement()
{
    if (pegs_.empty())
    {
        return 0;
    }
    else
    {
        std::list<RoutingElement *>::iterator iter=pegs_.end();
        iter--;
        return (*iter); 
    }
}

bool Wire::routedVia(RoutingElement* p)
{
    std::list<RoutingElement *>::iterator iter=pegs_.begin();
    while(iter!=pegs_.end())
    {
        if( (*iter)->get_id()==p->get_id()) 
        {
            return true;
        }
        iter++;
    }
    return false;
}

void Wire::insertRoutingElementBefore(RoutingElement* newElement, RoutingElement* existingElement, int input, int output)
{
    std::list<RoutingElement *>::iterator iter=pegs_.begin();
    while(iter!=pegs_.end())
    {
        if( (*iter)->get_id()==existingElement->get_id()) 
        {
                pegs_.insert(iter,newElement);  
                newElement->addId(id_, input,output);
                break;
        }
        iter++;
    }

    if(foregrounded_)
    {
        newElement->setForegrounded(true,id_);
    }

    updateRoutingProps();
}

void Wire::insertRoutingElementAfter(RoutingElement* newElement, RoutingElement* existingElement, int input, int output)
{
    std::list<RoutingElement *>::iterator iter=pegs_.begin();
    while(iter!=pegs_.end())
    {
        if( (*iter)->get_id()==existingElement->get_id()) 
        {
                iter++;
                pegs_.insert(iter,newElement);  
                newElement->addId(id_, input,output);
                break;                
        }
        iter++;
    }

    if(foregrounded_)
    {
        newElement->setForegrounded(true,id_);
    }

    updateRoutingProps();
}


void Wire::addRoutingElementAtEnd(RoutingElement* p, int input, int output)
{
    
    if (!routedVia(p))
    {
        pegs_.push_back(p);
        p->addId(id_, input,output);

        if(foregrounded_)
        {
            p->setForegrounded(true,id_);
        }

        updateRoutingProps();
    }
}

void Wire::addRoutingElementAtStart(RoutingElement* p, int input, int output)
{
    if (!routedVia(p))
    {
        pegs_.push_front(p);
        p->addId(id_, input, output);

        if(foregrounded_)
        {
            p->setForegrounded(true,id_);
        }

        updateRoutingProps();
    }
}
void Wire::addRoutingElement(RoutingElement* p, int input, int output)
{

    //  XXX can only go through each routing element once at present
    if (!routedVia(p))
    {
        bool added=false;
        if (pegs_.empty())
        {
            pegs_.push_back(p);
            pic::logmsg()<<"pegs was empty- added to end ";
        }
        else
        {
            std::list<RoutingElement *>::iterator iter=pegs_.begin();
            while (iter!=pegs_.end())
            {
                if ((*iter)==hookPos_)
                {
                    iter=pegs_.insert(iter,p);
                    pic::logmsg()<<"inserted before Peg ";
                    added=true;
                    break;
                }
                else
                {
                    iter++;
                }
            }
            if (added==false)
            {
                pegs_.push_back(p);
                pic::logmsg()<<"added to end  ";
            }
        }
       
        p->addId(id_,input, output);

        if(foregrounded_)
        {
            p->setForegrounded(true,id_);
        }

        updateRoutingProps();
    }
}

void Wire::updateRoutingProps()
{
    routeChanged(false,false);
    if(props_==0)
    {
       props_=workspace_->create_store("Connection", getId()); 
       workspace_->init_store(props_); 
    }

    if(props_!=0)
    {
        pic::logmsg() <<"Wire updateRoutingProps: pegs_.size()="<<pegs_.size();
        props_->remove_list("routing");
        std::list<RoutingElement *>::iterator iter=pegs_.begin();
        unsigned index=0;
        while (iter!=pegs_.end())
        {

           RoutingElement* re=(*iter);
           props_->set_list("routing",index,re->get_id());
           pic::logmsg() <<"routing list index added"<< index;
           index ++;
           iter++;
        }
 
        //std::cout<<"wire updateRoutingProps"<<getId().toUTF8()<<std::endl;
        workspace_->save_store(id_);
    }
    preDraw();
}

Path Wire::getHookedArea()
{
//    areaPath.addPath(hook_->getPathFrom(getDstPin(),id_));
//    areaPath.addPath(getDstPin()->getPathFrom(getSrcPin(),id_));
//    areaPath.addPath(getSrcPin()->getPathFrom(hook_,id_));
//    areaPath.closeSubPath();
//    return areaPath;
//XXX test-try with simple triangle
    RoutingElement* r1=0;
    RoutingElement* r2=0;
    if (pegs_.empty())
    {
        r1=getDstPin();
        r2=getSrcPin();
    }
    else
    {
// XXX get r1 and r2 - the routing elements either size of the hook
        bool found=false;
        std::list<RoutingElement *>::iterator iter=pegs_.begin();
        int count=0;
        while (iter!=pegs_.end())
        {
            if ((*iter)==hookPos_)
            {
                pic::logmsg()<<"position="<<count<<" of "<<pegs_.size();
                if(count==0)
                {
                    pic::logmsg() <<"found-count=0";
                    r1=hookPos_;
                    r2=getSrcPin();
                    found=true;
                    break;
                }
                else
                {
                    pic::logmsg() <<"found-count>0";
                    r2=hookPos_;
                    iter--;
                    r1=(*iter);
                    found=true;
                    break;
                }
                
            }
            else
            {
                pic::logmsg()<<"increment";
                count++;
                iter++;
            }

        }
        if (found==false)
        {
            pic::logmsg()<<"found=false";
            r1=getDstPin();
            r2=pegs_.back();
        }

    }

    Path areaPath;
    if (r1!=0 && r2!=0)
    {
        areaPath.startNewSubPath(hook_->getXin(id_),hook_->getYin(id_));
        areaPath.lineTo(r1->getXin(id_),r1->getYin(id_));
        areaPath.lineTo(r2->getXin(id_),r2->getYin(id_));
        areaPath.closeSubPath();
    }

    float zoomFactor=mc_->getZoomFactor();
    areaPath.applyTransform(AffineTransform::scale(zoomFactor,zoomFactor));

    return areaPath;
}

int Wire::getHash(RoutingElement* r1, RoutingElement* r2)
{
    if(isRevConnect())
    {
        return (r2->get_hash(id_)+r1->get_hash(id_)).hashCode();
    }
    return (r1->get_hash(id_)+r2->get_hash(id_)).hashCode();
}

int Wire::getToHookHash(RoutingElement* r)
{
    if(isRevConnect())
    {
        return ("hook"+r->get_hash(id_)).hashCode();
    }
    return (r->get_hash(id_)+"hook").hashCode();
}

int Wire::getFromHookHash(RoutingElement* r)
{
    if(isRevConnect())
    {
        return (r->get_hash(id_)+"hook").hashCode();
    }
    return ("hook"+r->get_hash(id_)).hashCode();
}

int Wire::getSegmentHashAt(int x, int y)
{
    int tol=5;
    float zoomFactor=mc_->getZoomFactor();
    for (std::vector<Segment*>::iterator i=fullSegments_.begin();i!=fullSegments_.end();i++)
    {
        Path p =(*i)->getPath();
        p.applyTransform(AffineTransform::scale(zoomFactor,zoomFactor));
        if(p.intersectsLine(Line<float>(x-tol,y,x+tol,y)))
        {
            return (*i)->getHash();
        }
        else if(p.intersectsLine(Line<float>(x,y-tol,x,y+tol)))
        {
            return (*i)->getHash();
        }
    }
    return 0;
}

bool Wire::hasSegment(int hash)
{
    for (std::vector<Segment*>::iterator i=fullSegments_.begin();i!=fullSegments_.end();i++)
    {
        if((*i)->getHash()==hash)
        {
            return true;
        }
    }
    return false;
}

void Wire::getConnectionPath(std::vector<Segment*> &p)
{
    if (getDstPin()==0)
    {
        return;
    }
    RoutingElement* r1=getSrcPin();

//    if(r1==0)
//    {
//        std::cout<<" ***** r1==0"<<std::endl;
//    }
//
    if (hook_==0 && pegs_.empty())
    {
       DestinationPin * dp=getDstPin();
//       std::cout<<"destinationPin="<<dp<<std::endl;
       p.push_back(new Segment(dp->getPathFrom(r1,id_),getHash(r1,dp),this));
    }
    else if (pegs_.empty())
    {
        Path p1=hook_->getPathFrom(r1,id_);
        Path p2=getDstPin()->getPathFrom(hook_,id_);

        p.push_back(new Segment(p1,getToHookHash(r1),this));
        p.push_back(new Segment(p2,getFromHookHash(getDstPin()),this));
    }
    else
    {
        bool hooktodo=true;

        for (std::list<RoutingElement*>::const_iterator iter=pegs_.begin();iter!=pegs_.end();++iter)
        {
            RoutingElement * posPeg=(*iter); 
            if (hook_!=0 && hooktodo==true)
            {
                getHookPath(p,posPeg,r1,hooktodo);
            }
            else
            {
                // if r1 and pospeg in the same trunk assembly - dont add the path
                // so that the wire doesnt show up on top of trunk corners
                bool intraTrunkConnection=false;
                String a2=posPeg->getAssemblyId();
                String a1=r1->getAssemblyId();
                if(a1.isNotEmpty() && a2.isNotEmpty())
                {
                    if(a1==a2)
                    {
                        intraTrunkConnection=true;
                    }
                }
                if(!intraTrunkConnection)
                {
                    Path p1=posPeg->getPathFrom(r1,id_);
                    p.push_back(new Segment(p1,getHash(r1,posPeg),this));
                }
            }
            r1=posPeg;
        }

        if (hook_!=0 && hooktodo==true)
        {
            getEndHookPath(p,getDstPin(),r1,hooktodo);
        }
        else
        {
            DestinationPin* dp=getDstPin();
            Path p3=dp->getPathFrom(r1,id_);
            p.push_back(new Segment(p3,getHash(r1,dp),this));
        }
    }
}

void Wire::getHookPath(std::vector<Segment*> &p,RoutingElement* posPeg, RoutingElement* r1, bool& hooktodo)
{
    int tol=5;
    int x=hook_->getXin(id_);
    int y=hook_->getYin(id_);
    Path p1=posPeg->getPathFrom(r1, id_);
    if ( p1.contains(x,y))
    {
        hookPos_=posPeg;
    }

    else if(p1.intersectsLine(Line<float>(x-tol,y,x+tol,y)))
    {
        hookPos_=posPeg;
    }
    else if(p1.intersectsLine(Line<float>(x,y-tol,x,y+tol)))
    {
        hookPos_=posPeg;
    }


    if (hookPos_==posPeg)
    {
        // XXX
        // if posPeg and r1 are sticky
        // hook_->setSticky(true)
        // else 
        // hook_->setSticky(false)
        // implement isSticky() in Hook

        p1=hook_->getPathFrom(r1,id_);
        Path p2=posPeg->getPathFrom(hook_,id_);

        p.push_back(new Segment(p1, getToHookHash(r1),this));
        p.push_back(new Segment(p2, getFromHookHash(posPeg),this));
        hooktodo=false;
    }
    else
    {
        // if r1 and pospeg in the same trunk assembly - dont add the path
        // so that the wire doesnt show up on top of trunk corners

        bool intraTrunkConnection=false;
        String a2=posPeg->getAssemblyId();
        String a1=r1->getAssemblyId();
        if(a1.isNotEmpty() && a2.isNotEmpty())
        {
            if(a1==a2)
            {
                intraTrunkConnection=true;
            }
        }
        if(!intraTrunkConnection)
        {
            p1=posPeg->getPathFrom(r1,id_);
            p.push_back(new Segment(p1, getHash(r1,posPeg),this));
        }
    }

}

void  Wire::getEndHookPath(std::vector<Segment*> &p, RoutingElement* dp, RoutingElement* r1, bool& hooktodo)
{
    int tol=5;
    int x=hook_->getXin(id_);
    int y=hook_->getYin(id_);

    Path p3=dp->getPathFrom(r1, id_);
    if ( p3.contains(x,y))
    {
        hookPos_=dp;
    }
    else if(p3.intersectsLine(Line<float>(x-tol,y,x+tol,y)))
    {
        hookPos_=dp;
    }
    else if(p3.intersectsLine(Line<float>(x,y-tol,x,y+tol)))
    {
        hookPos_=dp;
    }


       
    if (hookPos_==dp)
    {
        p3=hook_->getPathFrom(r1,id_);
        Path p4=dp->getPathFrom(hook_,id_);

        p.push_back(new Segment(p3, getToHookHash(r1),this));
        p.push_back(new Segment(p4,getFromHookHash(dp),this));
        hooktodo=false;
    }
    else
    {
        pic::logmsg()<<"***** Hook position not detected in any sub path";
        p3=dp->getPathFrom(r1,id_);
        p.push_back(new Segment(p3, getHash(r1,dp),this));
        
    }
}

bool Wire::hasSameRoute(Wire* other)
{
    bool val=(getRouteHash()==other->getRouteHash());
    return val;
}

void Wire::removeAllPegs()
{
    std::list<RoutingElement *>::iterator iter=pegs_.begin();
    while (iter!=pegs_.end())
    {
        (*iter)->removeId(id_);
        ++iter;
    } 
    pegs_.clear();

    updateRoutingProps();
}

void Wire::removePeg(RoutingElement* p)
{
    std::list<RoutingElement *>::iterator iter=pegs_.begin();
    if(p==hookPos_)
    {
        while(*iter!=hookPos_)
        {
            iter++;
        }
        if(*iter==hookPos_)
        {
            ++iter;
            if(iter==pegs_.end())
            {
                hookPos_=getDstPin();
            }
            else
            {
                hookPos_=*iter;
            }
        }
    }

    iter=pegs_.begin();
    bool pegRemoved=false;
    while (iter!=pegs_.end())
    {
        if ((*iter)==p)
        {
            pic::logmsg()<<"   removing peg";
            pegRemoved=true;
            iter=pegs_.erase(iter);
        }
        else
        {
            ++iter;
        }
    } 
    if (pegRemoved)
    {
        p->removeId(id_);
        updateRoutingProps();
    }
}

Colour Wire::getWireColour()
{
    if (selected_)
    {
        return Colour(selectedWireColour);
    }
    else if(isLoose())
    {
        return Colour(looseWireColour);
    }

    return Colour(wireColour);
}

Path Wire::getFullPath()
{
    Path p=Path();
    for (std::vector<Segment*>::const_iterator i=fullSegments_.begin();i!=fullSegments_.end();++i)
    {
        p.addPath((*i)->getPath());
    }
    return p;
}

Path Wire::getOldPath()
{
    Path p=Path();
    for (std::vector<Segment*>::const_iterator i=oldSegments_.begin();i!=oldSegments_.end();++i)
    {
        p.addPath((*i)->getPath());
    }

    return p;
}

Path Wire::getWirePath()
{
    Path testPath=getFullPath();
    float zoomFactor=mc_->getZoomFactor();
    testPath.applyTransform(AffineTransform::scale(zoomFactor,zoomFactor));
    return testPath;
}

Path Wire::getOldWirePath()
{
    Path testPath=getOldPath();
    float zoomFactor=mc_->getZoomFactor();
    testPath.applyTransform(AffineTransform::scale(zoomFactor,zoomFactor));
    return testPath;
}

void Wire::setSelected(bool selected)
{
//    if(selected)
//    {
//        pic::logmsg()<<"Wire "<<getId().toUTF8()<<" ["<< dstUsing_.toUTF8()<<"] selected";
//    }
//
    selected_=selected;
    if (getDstPin()!=0)
    {
        getDstPin()->setSelected(selected, String::empty);
    }
    if (getSrcPin()!=0)
    {
        getSrcPin()->setSelected(selected, String::empty);
    }

    std::list<RoutingElement *>::iterator iter=pegs_.begin();
    while (iter!=pegs_.end())
    {
        (*iter)->setSelected(selected,id_);
        iter++;
    }
}

void Wire::setForegrounded(bool shouldBeForegrounded)
{
    if(shouldBeForegrounded)
    {
        mc_->addForegroundWire(this);
        foregrounded_=true;
        std::list<RoutingElement *>::iterator iter=pegs_.begin();
        while (iter!=pegs_.end())
        {
            (*iter)->setForegrounded(true,id_);
            iter++;
        }
        if(getSrcPin()!=0)
        {
            getSrcPin()->setForegrounded(true,id_); 
        }
        if(getDstPin()!=0)
        {
            getDstPin()->setForegrounded(true,id_); 
        }
    }
    else
    {
        //std::cout<<"wire:setForegrounded false"<<std::endl;
        mc_->removeForegroundWire(this);
        foregrounded_=false;
        std::list<RoutingElement *>::iterator iter=pegs_.begin();
        while (iter!=pegs_.end())
        {
            (*iter)->setForegrounded(false,id_);
            iter++;
        }
        if(getSrcPin()!=0)
        {
            getSrcPin()->setForegrounded(false,id_); 
        }
        if(getDstPin()!=0)
        {
            getDstPin()->setForegrounded(false,id_); 
        }
    }
}

void Wire::preDraw()
{
    if (needsRouteCalculation_ )
    {
        clearRoute();
        calculateRoute();
        int oldHash=hashCode_;
        calcHashCode();
//        pic::logmsg()<<"Wire preDraw: id="<<getId().toUTF8();

        if(oldHash!=hashCode_)
        {
            if(isLoose())
            {
                mc_->insertLooseWire(this);
            }
            else
            {
                mc_->changeDrawingWire(this);
            }
        }
    }
}

void Wire::preAdd()
{
    if (needsRouteCalculation_ )
    {
        clearRoute();
        calculateRoute();
        calcHashCode();
//        pic::logmsg()<<"Wire preAdd: id="<<getId().toUTF8();
        metronomeOutput_=mc_->isConnectionFrom(this, "metronome");
        controllerOutput_=mc_->isConnectionFrom(this, "controller");
    }
}

void Wire::deleteRoute()
{
    for (std::vector<Segment*>::const_iterator i=oldSegments_.begin();i!=oldSegments_.end();++i)
    {
        delete *i;
    }
    oldSegments_.clear();

    for (std::vector<Segment*>::const_iterator i=fullSegments_.begin();i!=fullSegments_.end();++i)
    {
        delete *i;
    }
    fullSegments_.clear();

    for (std::vector<Segment*>::const_iterator i=draftSegments_.begin();i!=draftSegments_.end();++i)
    {
        delete *i;
    }
    draftSegments_.clear();
}

void Wire::clearRoute()
{
    for (std::vector<Segment*>::const_iterator i=oldSegments_.begin();i!=oldSegments_.end();++i)
    {
        delete *i;
    }
    oldSegments_.clear();

    for (std::vector<Segment*>::const_iterator i=fullSegments_.begin();i!=fullSegments_.end();++i)
    {
        oldSegments_.push_back(new Segment((*i)->getPath(),(*i)->getHash(),this));
    }

    for (std::vector<Segment*>::const_iterator i=fullSegments_.begin();i!=fullSegments_.end();++i)
    {
        delete *i;
    }
    fullSegments_.clear();

    for (std::vector<Segment*>::const_iterator i=draftSegments_.begin();i!=draftSegments_.end();++i)
    {
        delete *i;
    }
    draftSegments_.clear();

}

void Wire::calculateRoute()
{
    if(getDstPin()!=0 && getSrcPin()!=0)
    {
           mc_->setDraftMode(false);
           getConnectionPath(fullSegments_);
           mc_->setDraftMode(true);
           getConnectionPath(draftSegments_);

           if(selected_)
           {
               setSelected(true);
           }

           calculateGrids();

        needsRouteCalculation_=false;
    }
}

void Wire::calculateGrids()
{
    clearGridMap();
    int gn;
    std::set<int> gridsquares;
    for (std::vector<Segment*>::const_iterator j=fullSegments_.begin();j!=fullSegments_.end();++j)
    {
        for(Path::Iterator i ((*j)->getPath());i.next();)
        {
            // XXX If make simplification to PathCalculator for the linear (sticky hook) case this
            // requires modification, otherwise wire detection breaks: If its a linear path 
            // ie path has only one line segement
            // add grid squares for intermediate points along path
            switch (i.elementType)
            {
            case Path::Iterator::startNewSubPath:
                gn=mc_->getGridNumber(i.x1,i.y1); 
                break;
            case Path::Iterator::lineTo:
                gn=mc_->getGridNumber(i.x1,i.y1); 
                break;
            default:
                break;
            }
            std::set<int>::iterator iter;
            iter=gridsquares.find(gn);
            if(iter==gridsquares.end())
            {
                 updateGridMap(gn);
                 gridsquares.insert(gn);
            }
        }
    }
}

void Wire::clearGridMap()
{
    mc_->clearGridMap(this);
}

void Wire::updateGridMap(int gs)
{
    mc_->updateGridMap(gs,this);
}

bool Wire::shouldNotBeLoaded()
{
    if(connection_!=0)
    {
        if(connection_->is_hidden())
        {
            return true;
        }
    }
    return false;
}

bool Wire::isHidden()
{
    if(metronomeOutput_)
    {
        return mc_->hideMetronomeOutputs();
    }
    else if(controllerOutput_)
    {
        return mc_->hideControllerOutputs();
    }
    else
    {
      return false;
    }
}

void Wire::getSegments(bool draft, std::vector<Segment*>& segments)
{
    if(draft)
    {
        segments=draftSegments_;
    }
    else
    {
        segments=fullSegments_;
    }
}

void Wire::routeChanged(bool srcChanged, bool dstChanged)
{
    pic::logmsg() <<"routeChanged Wire= "<<std::string(getId().toUTF8());
    //std::cout <<"routeChanged Wire= "<<getId().toUTF8()<<std::endl;
    if(srcChanged)
    {
        sp_ready_=false;
        if(using_sp_!=0)
        {
            using_sp_->doRemoveWire(this);
        }
        using_sp_=getSrcPin();
    }

    if(dstChanged)
    {
        dp_ready_=false;
        if(using_dp_!=0)
        {
            using_dp_->doRemoveWire(this);
        }
        using_dp_=getDstPin();
    }

    int oldHash=hashCode_;
    calcHashCode();
    if (oldHash!=hashCode_)
    {
        pic::logmsg()<<"      oldHash="<<oldHash<<" newHash= "<<hashCode_<<" changed "<<std::string(getId().toUTF8());
        if(isLoose())
        {
            mc_->insertLooseWire(this);
        }
        else
        {
            mc_->changeDrawingWire(this);
        }
    }
    else
    {
        pic::logmsg()<<"      oldHash="<<oldHash<<" newHash= "<<hashCode_<<" identical "<<std::string(getId().toUTF8());
    }

    clearRoute();
    needsRouteCalculation_=true;
}

void Wire::forceRouteChanged()
{
    forceRouteChanged(true,true);
}

void Wire::forceRouteChanged(bool srcChanged,bool dstChanged)
{
//    pic::logmsg() <<"forceRouteChanged "<<getId().toUTF8();
//    std::cout <<"forceRouteChanged "<<getId().toUTF8()<<" srcChanged="<<srcChanged<<" dstChanged="<<dstChanged<<std::endl;

    forcedChange_=true;
    if(srcChanged)
    {
//        std::cout<<"sp was "<<using_sp_<<std::endl;
        sp_ready_=false;
        using_sp_=getSrcPin();
//        std::cout<<"sp now "<<using_sp_<<std::endl;
    }
    if(dstChanged)
    {
//        std::cout<<"dp was "<<using_dp_<<std::endl;
        dp_ready_=false;
        using_dp_=getDstPin();
//        std::cout<<"dp now "<<using_dp_<<std::endl;
    }

    calcHashCode();
    if(isLoose())
    {
        mc_->insertLooseWire(this);
    }
    else
    {
        mc_->changeDrawingWire(this);
    }
    clearRoute();
    needsRouteCalculation_=true;
    forcedChange_=false;
}

bool Wire::isForcedChange()
{
    return forcedChange_;
}

void Wire::setForcedChange(bool forced)
{
    forcedChange_=forced;
}

void Wire::pathChanged()
{
//    clearRoute();
    needsRouteCalculation_=true;
}

void Wire::setChanged()
{
    dp_ready_=false;
    sp_ready_=false;
    using_sp_=0;
    using_dp_=0; 
    clearRoute();
    needsRouteCalculation_=true;
}
