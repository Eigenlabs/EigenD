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

#include "ClipManagerList.h"
 
ClipManagerList::ClipManagerList(ClipManagerBackend* backend)
            : ClipList("ClipManagerList"),backend_(backend)
{
    setSize (600, 228);
    selectedTags_=new XmlElement("TAGS");
    setMultipleSelectionEnabled(true);
    backend->addSelectionListener(this);
}

ClipManagerList::~ClipManagerList()
{

}

void ClipManagerList::selectionChanged()
{
    updateClips(selectedTags_);
}

void ClipManagerList::updateClips(XmlElement* e)
{
    std::cout<<"ClipManagerList - updateClips"<<std::endl;
    selectedTags_=e;
    XmlElement* clipData=backend_->getSelectedClips(e);
    refreshList(clipData);
}

void ClipManagerList::addToClipPool()
{
    std::cout<<"ClipManagerList-add to clipPool"<<std::endl;
    backend_->addToClipPool(getSelectedClips(),0);
}

void ClipManagerList::listBoxItemDoubleClicked(int row, const MouseEvent& e)
{
    std::cout<<"ClipManagerList doubleclicked"<<std::endl;
    ClipPanelSwitcher* p=findParentComponentOfClass<ClipPanelSwitcher>();
    if(p!=0)
    {
        p->switchPanel("clipEditor",v_[row]);
    }
}

bool ClipManagerList::isInterestedInDragSource(const SourceDetails &dragSourceDetails)
{
  
    return false;
}

void ClipManagerList::itemDropped(const SourceDetails &dragSourceDetails)
{

}


