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

#ifndef __TRUNKINPUT__
#define __TRUNKINPUT__
#include <piw/piw_tsd.h>
#include "juce.h"
#include "ToolManager.h"
#include "Trunk.h"
#include "RoutingElement.h"
#include <iostream>

class Trunk;
class TrunkInput  : public Component,
                    public AutoDragable
{
public:
    TrunkInput (Trunk* t,String id,int apertureIndex,float zoomFactor, bool input);
    ~TrunkInput();

    void paint (Graphics& g);
    void resized();
    void moved();
    void mouseEnter (const MouseEvent& e);
    void mouseExit (const MouseEvent& e);
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    String getID();
    void doZoom(float zoomFactor);
    void reposition(bool);
    bool isInput();
    bool isOutput();
    void save_props();
    void setPos(int x, int y, bool endsAllowed);
    void setPos(int x, int y);
//    void doRemove();
    bool checkRemove(int apertureIndex,int x, int y);
    bool checkRemove2(int apertureIndex,int x, int y);
    //bool checkRemove3(int apertureIndex,int x, int y);
    int  getApertureIndex();
    void onMoveEnd();
    void onMoveEndTest();
    Trunk* getTrunk();
    void onDrag(int x, int y);
    void removeOnDragEnd();
    bool isDragging();
    void setApertureIndex(int apertureIndex);
    void doAutoDrag(int dx,int dy);
    int getConnectionX();
    int getConnectionY();
    int getTrueConnectionX();
    int getTrueConnectionY();


private:
    String id_;
    float zoomFactor_;
    int mouseDownX;
    int mouseDownY;
    int trueX_;
    int trueY_;
    float px_;
    float py_;
    Trunk* t_;
    int index_;
    bool input_;
    bool indexChanged_;
    bool dragging_;
    bool removeOnDragEnd_;
    bool move_init_;
    bool removeIfEqual(int index1, int index2);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrunkInput);
};

#endif
