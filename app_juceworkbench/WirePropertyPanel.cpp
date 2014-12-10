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

#include "WirePropertyPanel.h"

WirePropertyPanel::WirePropertyPanel(std::vector<Wire*> wires)
{
    wires_=wires;
    DuplicatePanel* dp_=new DuplicatePanel();
    addAndMakeVisible(dp_);
    if(wires.size()<3)
    {
      dp_->setVisible(false);  
    }

    for(std::vector<Wire*>::iterator i=wires_.begin();i!=wires_.end();i++)
    {
        addWireEditor((*i));
    }

    int ysize=editors_.size()*35;
    if (wires.size()>2)
    {
        ysize=ysize+35;
    }
    setSize (540, ysize);
}

WirePropertyPanel::~WirePropertyPanel()
{

}

void WirePropertyPanel::resized()
{
    int count=0;
    int offset=0;
    if (wires_.size()>2)
    {
        offset=35;
    }
    for(std::vector<WireEditor*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        (*i)->setTopLeftPosition(0,offset+35*count);
        count++;
    }
}

void WirePropertyPanel::handleCommandMessage(int commandId)
{
    switch(commandId)
    {
        case 0:
            enableAllUsing(false);
            break;
        case 1:
            enableAllUsing(true);
            break;
        case 2:
            enableAllFilter(false);
            break;
        case 3:
            enableAllFilter(true);
            break;
        case 4:
            controlUsing();
            break;
        case 5:
            controlFilter();
            break;
    }
}

void WirePropertyPanel::enableAllUsing(bool enable)
{
    int count=0;
    for(std::vector<WireEditor*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if(enable)
        {
            (*i)->setUsingController(false);
            (*i)->enableUsing(true);
        }
        else
        {

            if(count==0)
            {
                (*i)->setUsingController(true);
            }
            else
            {
                (*i)->enableUsing(false);
            }
        }
        count++;
    }
}

void WirePropertyPanel::enableAllFilter(bool enable)
{
    int count=0;
    for(std::vector<WireEditor*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if(enable)
        {
            
            (*i)->setFilterController(false);
            (*i)->enableFilter(true);
        }
        else
        {
            if(count==0)
            {
                (*i)->setFilterController(true); 
            }
            else
            {
                (*i)->enableFilter(false);
            }
        }
        count++;
    }
}

void WirePropertyPanel::controlUsing()
{
    WireEditor* c=editors_[0];
    String u=c->getUsing();
    int count=0;
    for(std::vector<WireEditor*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if (count>0)
        {
            (*i)->setUsingText(u.getIntValue());
        }
        count++;
    }
}

void WirePropertyPanel::controlFilter()
{
    WireEditor* c=editors_[0];
    String f=c->getFilter();
    int count=0;
    for(std::vector<WireEditor*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if (count>0)
        {
            (*i)->setFilterText(f);
        }
        count++;
    }

}
void WirePropertyPanel::addWireEditor(Wire* w)
{
      WireEditor* editor=new WireEditor(w);
      editors_.push_back(editor);
      addAndMakeVisible(editor);
}

std::vector<WireEditor*> WirePropertyPanel::getEditors()
{
    return editors_;
}
