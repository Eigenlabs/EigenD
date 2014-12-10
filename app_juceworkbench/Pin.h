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

#ifndef __PIN__
#define __PIN__

#include <piw/piw_tsd.h>
#include "juce.h"
#include "WorkbenchColours.h"
#include "RoutingElement.h"
#include "ToolManager.h"
#include "MainComponent.h"
#include "Wire.h"
#include "sizes.h"
#include <iostream>
#include <map>

class MainComponent;
class Wire;
class SourcePin;
class DestinationPin;

class Pin: public Component,
           public RoutingElement,
           public AutoDragable
{
public:

    Pin(String id, int x, int y, float zoomFactor,bool loose);
    Pin(PropertyStore* p, float zoomFactor);
//    virtual ~Pin(){std::cout<<"pin destructor "<<id_<<"  "<<this<<std::endl;};
    virtual ~Pin(){};
    String getId();

    virtual void paint (Graphics& g){};

    virtual int getXin(String id)=0;
    virtual int getYin(String id)=0;
    virtual int getXout(String id)=0;
    virtual int getYout(String id)=0;
    virtual String get_id();
    virtual String get_hash(String wireId);
    virtual void setSelected(bool s, String id);
    virtual PropertyStore* getProps();
    virtual int getInputAt(int x, int y);
    virtual int getOutputAt(int x, int y);
    virtual void changed(PropertyStore* props);

    virtual void mouseExit (const MouseEvent& e);
    virtual void mouseEnter (const MouseEvent& e)=0;
    virtual void mouseDown(const MouseEvent& e)=0;
    virtual void mouseUp(const MouseEvent& e)=0;
    virtual void mouseDrag (const MouseEvent& e)=0;

    virtual void doAutoDrag(int dx,int dy);

    bool isLoosePin();
    bool wiringAllowed();
    Path getPathFrom(RoutingElement* other, String id);

    virtual void doLoosePinDrag(int x,int y)=0;
    virtual bool isRevConnect()=0;
    MainComponent* getMainPanel() const throw();
    void doZoom(float zoomFactor);
    void highlight(bool on);
    void addWire(Wire* w);
    void removeWire(Wire* w);
    void doRemoveWire(Wire* w);
    Wire* getWire();
    void setMouseMode(const MouseEvent& e);
    bool isConnecting();
    void selectWire();
    void doWiringToolMouseClick();
    void doSetPositionProps();
    void setForegrounded(bool foregrounded, String id);
    virtual bool isDragging(){return false;};
    void onDragEnd();
    void setDragging(bool);
    int getNumWires();

protected:
    static const int DISCONNECTING=1;
    static const int CONNECTING=2;
    std::map<String,Wire*> wires_;
    void highlightParentBox(bool val);
    virtual bool hitTest(int x,int y)=0;
    bool highlight_;
    String id_;
    int mouseDownX_;
    int mouseDownY_;
    int dx_;
    int dy_;
    bool loose_;
    bool selected_;
    int mouseMode_;
    unsigned currentIndex_;
    bool dragging_;
    bool needsRefresh_;
    void refreshBox();
};   // end of class Pin

class DestinationPin: public Pin
{

public:
    DestinationPin(String id, int x, int y, float zoomFactor,bool loose);
    DestinationPin(PropertyStore* p, float zoomFactor);

    virtual void paint (Graphics& g);

    virtual bool isDragging();
    virtual int getXin(String id);
    virtual int getYin(String id);
    virtual int getXout(String id);
    virtual int getYout(String id);

    virtual void mouseEnter (const MouseEvent& e);
    virtual void mouseDown(const MouseEvent& e);
    virtual void mouseUp(const MouseEvent& e);
    virtual void mouseDrag (const MouseEvent& e);
    virtual void doLoosePinDrag(int x,int y);
    virtual bool isRevConnect();

protected:
    virtual bool hitTest(int x,int y);
};

class SourcePin: public Pin
{

public:
    SourcePin(String id, int x, int y, float zoomFactor,bool loose);
    SourcePin(PropertyStore* p, float zoomFactor);

    virtual void paint (Graphics& g);

    virtual bool isDragging();
    virtual int getXin(String id);
    virtual int getYin(String id);
    virtual int getXout(String id);
    virtual int getYout(String id);

    virtual void mouseEnter (const MouseEvent& e);
    virtual void mouseDown(const MouseEvent& e);
    virtual void mouseUp(const MouseEvent& e);
    virtual void mouseDrag (const MouseEvent& e);
    virtual void doLoosePinDrag(int x,int y);
    virtual bool isRevConnect();

protected:
    virtual bool hitTest(int x,int y);

//private:
//    Path getOutlinePath();
};

class ReverseDestinationPin: public DestinationPin
{

public:
    ReverseDestinationPin(String id, int x, int y, float zoomFactor,bool loose);
    ReverseDestinationPin(PropertyStore* p, float zoomFactor);

    virtual void paint (Graphics& g);

    virtual int getXin(String id);
    virtual int getYin(String id);
    virtual int getXout(String id);
    virtual int getYout(String id);
    virtual bool isRevConnect();

protected:
    virtual bool hitTest(int x,int y);
};

class ReverseSourcePin: public SourcePin
{

public:
    ReverseSourcePin(String id, int x, int y, float zoomFactor,bool loose);
    ReverseSourcePin(PropertyStore* p, float zoomFactor);
    virtual void paint (Graphics& g);

    virtual int getXin(String id);
    virtual int getYin(String id);
    virtual int getXout(String id);
    virtual int getYout(String id);
    virtual bool isRevConnect();
protected:
    virtual bool hitTest(int x,int y);
};

#endif

