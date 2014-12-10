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

#ifndef __PEG__
#define __PEG__

#include <piw/piw_tsd.h>
#include "juce.h"
#include <map>
#include <iostream>
#include "PathCalculator.h"
#include "RoutingElement.h"
#include "ToolManager.h"
#include "MainComponent.h"
#include "DeletePegConfirmation.h"

class MainComponent;
class Peg  : public Component,
             public RoutingElement,
             public Selectable,
             public Movable
{
public:
    Peg(PropertyStore* props, float zoomFactor);
    ~Peg();

    virtual int getXin(String id);
    virtual int  getYin(String id);
    virtual int  getXout(String id);
    virtual int  getYout(String id);
    virtual String get_id();
    virtual String get_hash(String wireId);
    virtual Path getPathFrom(RoutingElement * r,String id);
    virtual void setForegrounded(bool foregrounded, String id);
    virtual void setSelected(bool selected, String id);
    virtual void addId(String id, int input,int output);
    virtual void removeId(String id);
    void doZoom(float zoomFactor);
    void doAutoDrag(int dx,int dy);
    virtual int getInputAt(int x, int y);
    virtual int getOutputAt(int x, int y);

    virtual PropertyStore* getProps();
    virtual void changed(PropertyStore* props);
    virtual void setMouseOver(bool over);

    void paint (Graphics& g);
    void resized();
    void mouseMove (const MouseEvent& e);
    void mouseEnter (const MouseEvent& e);
    void mouseExit (const MouseEvent& e);
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);

    void doMove(const MouseEvent& e);
    void lasso(bool lassoed);
    void doSelectedGroupMove(int dx, int dy);
    void doSetPositionProps();
    void initMove();
    void autoDragIfRequired(int x, int y, int mx, int my);
    void pointerToolMouseClick(const MouseEvent& e);
    void selectToolMouseClick(const MouseEvent& e);
    void deleteToolMouseClick(const MouseEvent& e);
    void hookToolMouseClick(const MouseEvent& e);
    virtual bool isWireDragTarget(){return true;};
    virtual bool isSticky();
    virtual void setSticky(bool shouldBeSticky);
    virtual void bringToFront();

private:
    int mouseDownX;
    int mouseDownY;
    String testId_;
    int getXin(String,bool);
    int getXout(String,bool);
    int getLeft();
    int getRight();
    std::map<String, int> directionMap_;
//    static const int LEFT_TO_RIGHT=0;
//    static const int RIGHT_TO_LEFT=1;

    MainComponent* getMainPanel() const throw()
    {
        return findParentComponentOfClass<MainComponent>();
    }

    bool mouseOver_;
    void doMouseClick(const MouseEvent& e);
    void doRemoveId(String);
    bool shouldBeHidden();
    bool foregrounded_;
    bool selected_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Peg);
};


#endif 
