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

#include "BrowseTreeView.h"

    BrowseTreeViewItem::BrowseTreeViewItem (Atom* atom, String parentpath,int nf, int nc): atom_(atom)
    {
//        pic::logmsg()<<"creating BrowseTreeViewItem: "<< this<< " parent= "<<std::string(parentpath.toUTF8());
        tip_=String::empty; 
        displayText_="Fetching...";
        nf_=nf;
        nc_=nc;
        expandable_=false;
        parentpath_=parentpath;
        empty_=true;
        cookie_=String::empty;
    }


    int BrowseTreeViewItem::getItemWidth() const
    {
    // if it doesn't exist then return -1 (fill tree width)
        return -1;
 
    }

    void BrowseTreeViewItem::itemSelectionChanged(bool isNowSelected)
    {
        if(!mightContainSubItems() && isNowSelected)
        {
//            pic::logmsg()<<"itemSelected "<<std::string(displayText_.toUTF8())<<"  cookie_="<<std::string(cookie_.toUTF8());
            atom_->activate(parentpath_,cookie_);
        }
    }

    String BrowseTreeViewItem::getUniqueName() const
    {
        return displayText_;
    }

    bool BrowseTreeViewItem::mightContainSubItems()
    {
        return expandable_;
    }

    void BrowseTreeViewItem::paintItem (Graphics& g, int width, int height)
    {
        g.setFont (height * 0.7f);
        g.setColour(Colour(0xffffffff));
        if (isSelected())
        {
            g.setColour(Colour(0xff00ff00));
        }
        g.drawText (displayText_,
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);
    }

    void BrowseTreeViewItem::itemOpennessChanged (bool isNowOpen)
    {
        if (isNowOpen)
        {
            // if we've not already done so, we'll now add the tree's sub-items. You could
            // also choose to delete the existing ones and refresh them if that's more suitable
            // in your app.
            if (getNumSubItems() == 0)
            {
                for(int i=0; i<nf_;i++)
                {
                     addSubItem (new BrowseTreeViewItem (atom_,getItemIdentifierString(),0,0));
                }

                for(int i=0; i<nc_;i++)
                {
                     addSubItem (new BrowseTreeViewItem (atom_,getItemIdentifierString(),0,0));
                }

//                pic::logmsg()<<"Item "<<this<<" "<<std::string(getUniqueName().toUTF8())<<": no of subitems="<<getNumSubItems();
                atom_->finfo(getItemIdentifierString());
                atom_->cinfo(getItemIdentifierString());
            }
        }
        else
        {
            clearSubItems();
        }
    }

    var BrowseTreeViewItem::getDragSourceDescription()
    {
    // XXX
        return String::empty;
    }

    bool BrowseTreeViewItem::isInterestedInDragSource(const DragAndDropTarget::SourceDetails &)
    {
        return false;
    }

    String BrowseTreeViewItem::getTooltip()
    {
        return tip_;
    }

    void BrowseTreeViewItem::fillChildFinfo(const std::set<std::string>& finfo)
    {
        std::set<std::string>::const_iterator i = finfo.begin();
        while(i!=finfo.end())
        {
            String s=String::fromUTF8(i->c_str());
//            pic::logmsg()<<"fillChildFinfo "<<std::string(s.toUTF8())<<" getUniqueName="<<std::string(getUniqueName().toUTF8());
            String k=s.upToFirstOccurrenceOf("&&",false,true);
            String v=s.fromLastOccurrenceOf("&&",false,true);

            BrowseTreeViewItem* subItem=getEmptySubItem();
            if(subItem!=0)
            {
                subItem->setFinfo(k,v);
            }
            else
            {
                pic::logmsg()<<"   subItem==0";
            }
            i++;
        }

    }

    void BrowseTreeViewItem::fillChildCinfo(const std::set<std::string>& cinfo)
    {
        std::set<std::string>::const_iterator i = cinfo.begin();
        while(i!=cinfo.end())
        {
            String s=String::fromUTF8(i->c_str());
//            pic::logmsg()<<"fillChildCinfo "<<std::string(s.toUTF8());
            BrowseTreeViewItem* subItem=getEmptySubItem();
            if(subItem!=0)
            {
                subItem->setCinfo(s);
            }
            else
            {
                pic::logmsg()<<"   subItem==0";
            }

            i++;
        }
    }

    BrowseTreeViewItem* BrowseTreeViewItem::getEmptySubItem()
    {
//        pic::logmsg()<<"getEmptySubItem: "<<this<<" "<<std::string(getUniqueName().toUTF8()) <<" no of sub items="<<getNumSubItems();
        for(int i =0;i<getNumSubItems();i++)
        {
            BrowseTreeViewItem* subItem=dynamic_cast<BrowseTreeViewItem*>(getSubItem(i));
//            pic::logmsg()<<"got sub item: parentPath="<<std::string(parentpath_.toUTF8());
            if(subItem->isEmpty())
            {
                return subItem;
            }
        }
        return 0;
    }

    bool BrowseTreeViewItem::isEmpty()
    {
        return empty_;
    }

    void BrowseTreeViewItem::setEnumerate(int nf,int nc)
    {
        nf_=nf;
        nc_=nc;
        setExpandable(true);
        repaintItem();
    }

    void BrowseTreeViewItem::setCinfo(String s)
    {
       setDisplayText(s);
       atom_->enumerate(getItemIdentifierString());
       empty_=false;
       repaintItem();
    }

    void BrowseTreeViewItem::setFinfo(String k,String v)
    {
       setDisplayText(v);
       setCookie(k);
       setExpandable(false);
       empty_=false;
       repaintItem();
    }

    void BrowseTreeViewItem::setCookie(String k)
    {
        cookie_=k;
    }

    void BrowseTreeViewItem::setDisplayText(String s)
    {
        displayText_=s;
    }

    void BrowseTreeViewItem::setExpandable(bool expandable)
    {
        expandable_=expandable;
    }


