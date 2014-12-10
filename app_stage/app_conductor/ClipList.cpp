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

#include "ClipList.h"

ClipList::ClipList(String name)
    :ListBox(name),numRows_(0)
{
    selectionChanged_=false;
    setColour(backgroundColourId,Colour(0x00ffffff));
    setModel(this);
}

ClipList::~ClipList()
{
}

void ClipList::selectedRowsChanged(int row)
{
    std::cout<<getName()<<" ClipList- selectedRows changed "<<row<<std::endl;
    selectionChanged_=true;
    updateContent();

    if((row>=0) && (row<(int)v_.size()))
    {
        Clip* c=v_[row];
        if(c!=0)
        {
            ClipSelector* svw=findParentComponentOfClass<ClipSelector>();
            if(svw!=0)
            {
                svw->clipSelected(c);
            }

        }
    }
    else if(row==-1)
    {
        ClipSelector* svw=findParentComponentOfClass<ClipSelector>();
        if(svw!=0)
        {
            svw->clipSelected(0);
        }
    }

}

void ClipList::backgroundClicked(const MouseEvent &e)
{
    deselectAllRows();
}

var ClipList::getDragSourceDescription(const SparseSet<int>& currentlySelectedRows)
{
    return getName();
}


void ClipList::listBoxItemClicked(int row, const MouseEvent&e)
{
//    supposed to allow clicking on selected row to un-select
//    but doesn't work with multiple row selection enabled.
//    Disabled for present.

//    if(!selectionChanged_)
//    {
//        std::cout<<"ClipList row clicked: row="<<row<<std::endl;
//        deselectRow(row);
//    }
//    selectionChanged_=false;
}

void ClipList::refreshList(XmlElement* clipData)
{
    if(clipData!=0)
    {
        // XXX delete the clips in the vector
        v_.clear();
        numRows_=0;

        XmlElement* e=clipData->getFirstChildElement();

        while(e!=0)
        {
            if(e->hasTagName("CLIP"))
            {
                v_.push_back(new Clip(e));
                numRows_++;
            }
            else if(e->hasTagName("SCENE"))
            {
                v_.push_back(new SceneClip(e));
                numRows_++;
            }
            else if(e->hasTagName("ARRANGEMENT"))
            {
                v_.push_back(new ArrangementClip(e));
                numRows_++;
            }

            e=e->getNextElement();
        }
        
        std::cout<<getName()<<" size of vector="<<v_.size()<<std::endl;
    //        deselectAllRows();
        updateContent();
    }
}

Component* ClipList::refreshComponentForRow(int rowNumber, bool isRowSelected, Component* existingComponentToUpdate)
{
//    std::cout<<getName()<<" refreshComponentForRow "<<rowNumber<<" component="<<existingComponentToUpdate<<std::endl;
    if((unsigned)rowNumber>=v_.size())
    {
        return 0;
    }
    deleteAndZero(existingComponentToUpdate);
    ClipBox* b=new ClipBox(v_[rowNumber]);
    if(isRowSelected)
    {
        b->setSelected(true);
    }
    b->setInterceptsMouseClicks(false,false);

    return b; 
}

int ClipList::getNumRows()
{
//    std::cout<<"getNumRows "<<numRows_<<std::endl;
    return numRows_;
}

void ClipList::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    //std::cout<<"ClipList::paintListBoxItem "<<rowNumber<<std::endl;
}

XmlElement* ClipList::getClipsForRows(const SparseSet<int>& rows)
{
    XmlElement* e =new XmlElement("Clips");
    std::cout<<"rows.size() "<<rows.size()<<std::endl;
    for(int i =0;i<rows.size();i++)
    {
        std::cout<<"   selected row="<<rows[i]<<std::endl;
        e->addChildElement(v_[rows[i]]->toXml()); 
    }
    return e;
}

XmlElement* ClipList::getSelectedClips()
{

    return getClipsForRows(getSelectedRows());
}

void ClipList::onInterestedInDragSource(const DragAndDropTarget::SourceDetails &dragSourceDetails)
{
        Point <int> p=dragSourceDetails.localPosition;
        int row=getInsertionIndexForPosition(p.getX(),p.getY());

        if(p.getY()>0 && p.getX()>0)
        {
        std::cout<<getName()<<" - isinterestedInDragSource  x="<<p.getX()<<" y="<<p.getY()<<" insertionIndex="<<row<<std::endl;
            if(p.getY()>(0.5*getHeight()))
            {
                int showRow=row+2;
                if(showRow<=getNumRows())
                {
                    scrollToEnsureRowIsOnscreen(showRow);
                }
            }
            else
            {
                int showRow=row-2;
                if(showRow<0)
                {
                    showRow=0;
                }
                scrollToEnsureRowIsOnscreen(showRow);
            }
        }

}

void ClipList::onDropSelf(const DragAndDropTarget::SourceDetails &dragSourceDetails)
{
        std::cout<<getName()<<" reorder list"<<std::endl;
        Component* c=dragSourceDetails.sourceComponent.get();
        if(c!=0)
        {
            ClipList* cl=dynamic_cast<ClipList*>(c);
            if(cl!=0)
            {
                cl->getSelectedRows();
                const SparseSet<int>& sourceRows=cl->getSelectedRows();
                for(int i =0;i<sourceRows.size();i++)
                {
                    std::cout<<"   source row="<<sourceRows[i]<<std::endl;
                }
            }
        }

        Point <int> p=dragSourceDetails.localPosition;
        
        std::cout<<getName()<<" reorder list: insertionIndex="<<getInsertionIndexForPosition(p.getX(),p.getY())<<std::endl;

    // XXX notify backend of new order

}

