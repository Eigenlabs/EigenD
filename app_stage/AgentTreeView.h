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
 
#ifndef __AGENT_TREE_VIEW_H__
#define __AGENT_TREE_VIEW_H__

#include "juce.h"

class AgentTreeViewItem : public TreeViewItem
{
public:
    AgentTreeViewItem (XmlElement* const xml_);
    ~AgentTreeViewItem() {}

    //--------------------------------------------------------------------------
    // Overidden TreeViewItem functions
    
    int getItemWidth() const;
    String getUniqueName() const;
    bool mightContainSubItems();
    void paintItem (Graphics& g, int width, int height);
    void itemOpennessChanged (bool isNowOpen);
    juce::var getDragSourceDescription();
    bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails &);
    String getTooltip() { return tip; }

    //--------------------------------------------------------------------------
    void setXml(XmlElement* const xml_) { xml = xml_; }
    XmlElement* getXml() { return xml; }
    
private:
    // the xml for the tree item
    XmlElement* xml;
    String tip;

};

/*
class AgentTreeView : public TreeView {
    TreeView
};
*/



#endif // __AGENT_TREE_VIEW_H__



