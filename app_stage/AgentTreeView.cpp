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
 
#include "AgentTreeView.h"

AgentTreeViewItem::AgentTreeViewItem(XmlElement* const xml_) : xml (xml_)
{
    if (xml != 0)
    {
        XmlElement *tip_xml = xml->getChildByName("tip");

        if (tip_xml)
        {
            tip = tip_xml->getAllSubText();
        }
        else
        {
            XmlElement *help_xml = xml->getChildByName("help");

            if(help_xml)
            {
                tip = help_xml->getAllSubText();
            }
        }
    }
}

    
int AgentTreeViewItem::getItemWidth() const
{
    // get width from xml
    // if it doesn't exist then return -1 (fill tree width)
    return xml->getIntAttribute ("width", -1);
}
    
String AgentTreeViewItem::getUniqueName() const
{
    // return string to identify this item
    if (xml != 0)
        return xml->getStringAttribute("name");
    else
        return String::empty;
}

bool AgentTreeViewItem::mightContainSubItems()
{
    // determine if a [+] should be drawn
    
    if (xml != 0 && xml->getFirstChildElement() != 0)
        return (xml->getFirstChildElement()->hasTagName("atom") ||
                xml->getFirstChildElement()->hasTagName("agent"));
    return false;
}

void AgentTreeViewItem::paintItem (Graphics& g, int width, int height)
{
    if (xml != 0)
    {
        // if this item is selected, fill it with a background colour..
        if (isSelected())
            //g.fillAll (Colours::blue.withAlpha (0.3f));
            g.fillAll(Colour(0xff387fd7));
        
        if(xml->getStringAttribute("enabled")=="true")
            g.setColour(Colour(0xff000000));
        else
            g.setColour(Colour(0xff777777));
            
        g.setFont (height * 0.7f);
        
        // draw the xml element's tag name..
        g.drawText (xml->getStringAttribute("name"),
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);
    }
}

void AgentTreeViewItem::itemOpennessChanged (bool isNowOpen)
{
    if (isNowOpen)
    {
        // if we've not already done so, we'll now add the tree's sub-items. You could
        // also choose to delete the existing ones and refresh them if that's more suitable
        // in your app.
        if (getNumSubItems() == 0)
        {
            // create and add sub-items to this node of the tree, corresponding to
            // each sub-element in the XML..
            
            if (xml != 0)
            {
                forEachXmlChildElement (*xml, child)
                {
                    // is child another atom? (can be an element of atom meta data)
                    if(child->hasTagName("atom") || child->hasTagName("agent"))
                        addSubItem (new AgentTreeViewItem (child));
                }
            }
        }
    }
    else
    {
        // in this case, we'll leave any sub-items in the tree when the node gets closed,
        // though you could choose to delete them if that's more appropriate for
        // your application.
    }
}

juce::var AgentTreeViewItem::getDragSourceDescription()
{
    if(xml->getNumChildElements()==0)   return "";
    
	if(xml->getFirstChildElement()->hasTagName("atom") ||
                xml->getFirstChildElement()->hasTagName("agent"))
		return "";
	else
		// return an xml string for the dragged item
		return xml->createDocument("", true, false);
}

bool AgentTreeViewItem::isInterestedInDragSource (const DragAndDropTarget::SourceDetails &)
{
    return false;
}
    

