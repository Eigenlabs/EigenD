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

#ifndef __BROWSE_TREE_VIEW_H__
#define __BROWSE_TREE_VIEW_H__
#include <piw/piw_tsd.h>
#include "juce.h"
#include "BrowseEditor.h"
#include "Workspace.h"

class BrowseTreeViewItem :public TreeViewItem
{

public:
    BrowseTreeViewItem (Atom* atom,String parentpath, int nf,int nc);
    ~BrowseTreeViewItem() {};

    //--------------------------------------------------------------------------
    // Overidden TreeViewItem functions
    
    int getItemWidth() const;
    String getUniqueName() const;
    bool mightContainSubItems();
    void paintItem (Graphics& g, int width, int height);
    void itemOpennessChanged (bool isNowOpen);
    var getDragSourceDescription();
    bool isInterestedInDragSource(const DragAndDropTarget::SourceDetails &);
    String getTooltip();
//    void itemClicked(const MouseEvent &e);
    void itemSelectionChanged(bool isNowSelected);
    // ---

    void fillChildFinfo(const std::set<std::string>&);
    void fillChildCinfo(const std::set<std::string>&);
    void setCinfo(String s);
    void setFinfo(String k,String v);
    void setEnumerate(int nf, int nc);
    void setDisplayText(String s);
    void setExpandable(bool);
//    void setPath(String p);
    bool isEmpty();
    BrowseTreeViewItem* getEmptySubItem();

private:
    String tip_;
    Atom* atom_;
    int nc_;
    int nf_;
    String displayText_;
    bool expandable_;
//    String path_;
    String parentpath_;
    bool empty_;
    String cookie_;
    void setCookie(String k);
};


#endif

