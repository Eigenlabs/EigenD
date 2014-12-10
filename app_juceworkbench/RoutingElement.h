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

#ifndef __ROUTINGELEMENT__
#define __ROUTINGELEMENT__
#include "juce.h"
#include "Workspace.h"
#include <set>

class Movable
{
public:
    Movable(){};
    virtual ~Movable(){};
    virtual void mouseDown (const MouseEvent& e)=0;
    virtual void mouseUp (const MouseEvent& e)=0;
    virtual void mouseDrag (const MouseEvent& e)=0;
    virtual void setMouseOver(bool over){};
};

class AutoDragable
{
public:
    AutoDragable(){};
    virtual ~AutoDragable(){};
    virtual void doAutoDrag(int x,int y)=0;
};

class Selectable: public AutoDragable
{
public:
    Selectable(){};
    virtual ~Selectable(){};
    virtual void doSelectedGroupMove(int dx, int dy)=0;
    virtual void doSetPositionProps()=0;
    virtual void lasso(bool lassoed)=0;
    virtual void initMove()=0;
    virtual void setForeground(bool, bool){};
    
protected:
    bool lassoed_;
};

class Highlightable
{
public:
    Highlightable(){};
    virtual ~Highlightable(){};
    virtual void highlight(bool on)=0;
protected:
    bool highlight_;
};


class RoutingElement
{

public:
    RoutingElement(){};
    virtual int getXin(String id)=0;
    virtual int  getYin(String id)=0;
    virtual int  getXout(String id)=0;
    virtual int  getYout(String id)=0;
    virtual Path getPathFrom(RoutingElement* r, String id)=0;
    virtual void addId(String id, int input, int output){};
    virtual void removeId(String id){};
    virtual void setSelected(bool selected, String id){};
    virtual void setForegrounded(bool foregrounded, String id){};
    virtual ~RoutingElement(){};
    virtual void doZoom(float zoomFactor)=0;
    virtual PropertyStore* getProps()=0;
    virtual String get_id()=0;
    virtual int getInputAt(int x, int y)=0;
    virtual int getOutputAt(int x, int y)=0;
    virtual void setOutput(String id, int x, int y){};
    virtual void setInput(String id, int x, int y){};
    virtual void changed(PropertyStore* p){};
    virtual String get_hash(String wire_id)=0;
    virtual String get_reverse_hash(String wire_id){return get_hash(wire_id);};
    virtual String getAssemblyId(){return String::empty;};
    virtual void setMouseOver(bool over){};
    virtual bool isWireDragTarget(){return false;};
    virtual void setSticky(bool shouldBeSticky){};
    virtual bool isSticky(){return false;};
    virtual void bringToFront(){};

protected:
    int trueWidth_;
    int trueHeight_;
    int trueX_;
    int trueY_;
    float zoomFactor_;
    PropertyStore* props_;
    bool move_init_;
    std::set<String> wireIds_;
    bool sticky_;

}; // end of class RoutingElement

#endif
