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

#include "SetList.h"

SetList::SetList (SetBackend* backend)
    : ClipList("SetList"),backend_(backend)
{
    setSize (600, 400);
    setChanged();
    backend_->addSetListener(this);
}

SetList::~SetList()
{
}

void SetList::listBoxItemDoubleClicked(int row, const MouseEvent& e)
{
    std::cout<<"SetList - doubleClicked"<<std::endl;
    if((row>=0) && (row<(int)v_.size()))
    {
        Clip* c=v_[row];
        if(c!=0)
        {
            backend_->viewClip(v_[row]->toXml());
        }
    }
}

void SetList::listBoxItemClicked(int row, const MouseEvent& e)
{
}

void SetList::setChanged()
{
    std::cout<<"setList- setChanged "<<std::endl;
    XmlElement* clipData=backend_->getSet();

    if(clipData!=0)
    {
        refreshList(clipData);
    }
    else
    {
        std::cout<<"clipData==0"<<std::endl;
    }
}

void SetList::removeFromSet()
{
    std::cout<<"SetList- removeFromSet"<<std::endl;
    XmlElement* e= getSelectedClips();
    backend_->removeFromSet(e);
}

void SetList::play()
{
    std::cout<<"SetList- play"<<std::endl;
    XmlElement* e= getSelectedClips();
    backend_->play(e);
}

bool SetList::isInterestedInDragSource(const SourceDetails &dragSourceDetails)
{
    String source=dragSourceDetails.description.toString();
    if(source.equalsIgnoreCase("SetList"))
    {
        ClipList::onInterestedInDragSource(dragSourceDetails);
        return true;
    }

    return false;
}

void SetList::itemDropped(const SourceDetails &dragSourceDetails)
{
    String source=dragSourceDetails.description.toString();
    if(source.equalsIgnoreCase("SetList"))
    {
        ClipList::onDropSelf(dragSourceDetails);
    }

}

