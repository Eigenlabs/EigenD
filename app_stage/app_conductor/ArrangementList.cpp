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

#include "ArrangementList.h"

ArrangementList::ArrangementList (ArrangementBackend* backend, int mode)
    : ClipList("ArrangementList"),backend_(backend), mode_(mode),arrangementName_(String::empty)
{
    setSize (600, 400);
    arrangementChanged(mode_);
    backend_->addArrangementListener(this);
}

ArrangementList::~ArrangementList()
{
}

String ArrangementList::getArrangementName()
{
    return arrangementName_;
}

void ArrangementList::setMode(int mode)
{
    mode_=mode;
}

void ArrangementList::listBoxItemDoubleClicked(int row, const MouseEvent& e)
{
    std::cout<<"ArrangementList - doubleClicked"<<std::endl;
    if((row>=0) && (row<(int)v_.size()))
    {
        Clip* c=v_[row];
        if(c!=0)
        {
            backend_->viewClip(v_[row]->toXml());
        }
    }
}

void ArrangementList::listBoxItemClicked(int row, const MouseEvent& e)
{
}

void ArrangementList::arrangementChanged(int mode)
{
    if(mode==mode_)
    {
        std::cout<<"ArrangementPanel- arrangementChanged mode="<<mode_<<std::endl;
        XmlElement* clipData=backend_->getArrangement(mode_);

        if(clipData!=0)
        {
            arrangementName_=clipData->getStringAttribute("name");
            ArrangementViewWidget* avw=findParentComponentOfClass<ArrangementViewWidget>();
            if(avw!=0)
            {
                avw->setArrangementName(arrangementName_);
            }

            refreshList(clipData);
        }
        else
        {
            std::cout<<"clipData==0"<<std::endl;
        }
    }
}

void ArrangementList::removeFromArrangement()
{
    std::cout<<"ArrangementList- removeFromArrangement"<<std::endl;
    XmlElement* e= getSelectedClips();
    backend_->removeFromArrangement(e,mode_);
}

void ArrangementList::duplicate()
{
    // XXX
    std::cout<<"ArrangementList - duplicate"<<std::endl;
}

bool ArrangementList::isInterestedInDragSource(const SourceDetails &dragSourceDetails)
{
    String source=dragSourceDetails.description.toString();
    if(source.equalsIgnoreCase("ClipPoolList")||source.equalsIgnoreCase("ArrangementList"))
    {
        ClipList::onInterestedInDragSource(dragSourceDetails);
        return true;
    }

    return false;
}

void ArrangementList::itemDropped(const SourceDetails &dragSourceDetails)
{
    String source=dragSourceDetails.description.toString();
    if(source.equalsIgnoreCase("ClipPoolList"))
    {
        std::cout<<"ArrangementList - ClipPoolList itemDropped"<<std::endl;
        Component* c=dragSourceDetails.sourceComponent.get();
        if(c!=0)
        {
            ClipList* cl=dynamic_cast<ClipList*>(c);
            if(cl!=0)
            {
                Point <int> p=dragSourceDetails.localPosition;
                backend_->addToArrangement(cl->getSelectedClips(),getInsertionIndexForPosition(p.getX(),p.getY()));
            }
        }
    }
    else if(source.equalsIgnoreCase("ArrangementList"))
    {
        ClipList::onDropSelf(dragSourceDetails);
    }

}

