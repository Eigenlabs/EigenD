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

#ifndef __CLIPLIST__
#define __CLIPLIST__
 
#include "juce.h"
#include "ClipBox.h"
#include  "Clip.h"
#include "ClipSelector.h"

class ClipList:public ListBox,
                public ListBoxModel
{

public:
    ClipList(String nam);
    ~ClipList();
    virtual int getNumRows();
    virtual void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);
    virtual void selectedRowsChanged(int lastRowSelected);
//    virtual String getTooltipForRow(int row){return String::empty};
    virtual void listBoxItemDoubleClicked(int row, const MouseEvent& e){};
    virtual void backgroundClicked(const MouseEvent& e);
    virtual void listBoxItemClicked(int row,const MouseEvent& e);
    virtual Component* refreshComponentForRow(int rowNumber, bool isRowSelected, Component* existingComponentToUpdate);
    virtual var getDragSourceDescription(const SparseSet<int>& currentlySelectedRows);
    void refreshList(XmlElement* clipData);
    XmlElement* getSelectedClips();
    XmlElement* getClipsForRows(const SparseSet<int>& rows);

protected:
    std::vector<Clip*> v_;
    void onInterestedInDragSource(const DragAndDropTarget::SourceDetails &dragSourceDetails);
    void onDropSelf(const DragAndDropTarget::SourceDetails &dragSourceDetails);

private: 

    int numRows_;
    bool selectionChanged_;
};

#endif
