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

#ifndef __TRUNK__
#define __TRUNK__

#include <piw/piw_tsd.h>
#include "juce.h"
#include <iostream>
#include "WorkbenchColours.h"
#include "TrunkDrawing.h"
#include "TrunkInput.h"
#include "ToolManager.h"
#include "MainComponent.h"
#include "WorkbenchColours.h"
#include "DeleteTrunkConfirmation.h"
#include <vector>
#include <set>
#include <algorithm>

Path getTrueCorner(int x,int y, int cornerType);

class TrunkInput;

class TrunkSection
{
public:
    TrunkSection(int x1, int y1,int x2,int y2, float zoomFactor);
    int getX();
    int getY();
    int getWidth();
    int getHeight();
    int getSectionLength();
    int getOrientation();
    int getDirection();
    int getReverseDirection();

private:
    int x1_;
    int y1_;
    int x2_;
    int y2_;
    float zoomFactor_;
};

class TrunkDrawingTool
{
public:

    TrunkDrawingTool(int x, int y,float zoomFactor, bool append);
    void draw(Graphics& g);
    void addPoint(int x, int y);
    void removePoint();
    int getLastX();
    int getLastY();
    void setCurrentPoint(int x,int y);
    void endDrag(int x, int y);
    int getNumSections();
    TrunkSection* getSection(unsigned index);
    int getNumCorners();
    int getCornerX(unsigned index);
    int getCornerY(unsigned index);
    int getCornerType(unsigned i);
    bool sameAsLastPoint(int x,int y, int tol);
    int getStandardTolerance();
    int getEndDragTolerance();
    void setMovedOffLastPoint();
    int getLowerBound(float zoomFactor);
    int getJoiningCornerPosX();
    int getJoiningCornerPosY();
    int getJoiningCornerType(int orientation, int x, int y);
    int getInitialJoiningCornerType();
    void setJoiningCornerParameters(int orientation, int x, int y);
    void getRectangle(juce::Rectangle <int>& u);  

private:
    float zoomFactor_;
    int currentX_;
    int currentY_;
    int orientation_;
    bool orientationSet_;
    std::vector <int> pointsX_;
    std::vector <int> pointsY_;
    bool dragOrientationChanged(int x, int y);
    bool movedOffLastPoint_;
    void drawTrunk(Graphics& g, int x1, int y1, int x2, int y2);
    void drawCorner(Graphics& g, int x, int y,int cornerType);
    Path getCornerPath(int x, int y, int cornerType);
    bool showJoiningCorner_;
    int joiningCornerType_;
    int appendTrunkOrientation_;
    int appendTrunkX_;
    int appendTrunkY_;
    int mx_;
    int my_;

};

class TrunkCorner:public Component
{
public:
    TrunkCorner(PropertyStore* p, float zoomFactor);
    void paint (Graphics& g);
    void doZoom(float zoomFactor);
    void doAssemblyMove(int dx, int dy);
    void changed(PropertyStore* p);
    String getAssemblyId();
    void doAssemblySetProps();
    PropertyStore*  getProps();
    void mouseMove (const MouseEvent& e);
    void mouseEnter (const MouseEvent& e);
    void mouseExit (const MouseEvent& e);
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    virtual void setMouseOver(bool over);
    void doSetMouseOver(bool over);
    //void setLassoed(bool lassoed);
    void doSetLassoed(bool lassoed);
    void doSetForegrounded(bool foregrounded);
    void doMouseClick(const MouseEvent& e);
    void bringToFront();
    void doBringToFront();

    static const int WEST_NORTH=1;
    static const int WEST_SOUTH=2;
    static const int EAST_NORTH=3;
    static const int EAST_SOUTH=4;
    static const int NORTH_EAST=3;
    static const int NORTH_WEST=1;
    static const int SOUTH_EAST=4;
    static const int SOUTH_WEST=2;

private:
    int mouseDownX;
    int mouseDownY;
    int trueX_;
    int trueY_;
    float zoomFactor_;
    int trueWidth_;
    int trueHeight_;
    String assemblyId_;
    PropertyStore* props_;
    void save_props();
    void setCursor(const MouseEvent& e);
    void doMove(const MouseEvent& e);
    MainComponent* getMainPanel() const throw()
    {
        return findParentComponentOfClass <MainComponent>();
    }
    Path getCornerPath(int x, int y, int cornerType);
    bool move_init_;
    bool mouseOver_;
    bool lassoed_;
    bool foregrounded_; 
};

class Trunk  : public Component,
               public RoutingElement,
               //public AutoDragable
               public Selectable,
               public Movable
{

public:
    Trunk (PropertyStore* p, float zoomFactor, int orientation);
    ~Trunk();

    virtual int getXin(String id);
    virtual int getYin(String id);
    virtual int getXout(String id);
    virtual int getYout(String id);
    int getXinInternal(String id);
    int getYinInternal(String id);
    int getXoutInternal(String id);
    int getYoutInternal(String id);
    virtual String get_id();
    virtual String get_hash(String wireId);
    virtual String get_reverse_hash(String wireId);
    virtual Path getPathFrom(RoutingElement * r, String id);
    virtual void addId(String id, int input,int output);
    virtual void removeId(String id);
    virtual void setSelected(bool selected, String id);
    virtual void setForegrounded(bool foregrounded, String id);
    virtual bool isWireDragTarget(){return true;};
    void doSetForegrounded(bool foregrounded, String id);
    virtual void changed(PropertyStore* p);
    void toggleSelected(String id);
    void doZoom(float zoomFactor);
    void doAutoDrag(int dx,int dy);
    void removeWire(String id);
    void setCursor(const MouseEvent& e);
    int getTool();
    virtual PropertyStore* getProps();
    void save_props();
    float getYInputRange();
    float getXInputRange();
    int getApertureX(int index);
    int getApertureY(int index);

    int getApertureIndex(int x, int y,bool endsAllowed);
    int getNumSections();
    virtual int getInputAt(int x, int y);
    virtual int getOutputAt(int x, int y);
    virtual void setOutput(String id, int x, int y);
    virtual void setInput(String id, int x, int y);

    void paint (Graphics& g);
    void resized();
    void mouseMove (const MouseEvent& e);
    void mouseEnter (const MouseEvent& e);
    void mouseExit (const MouseEvent& e);
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    void setMouseOver(bool over);
    void doSetMouseOver(bool over);
    void setLassoed(bool lassoed);
    void doSetLassoed(bool lassoed);

    TrunkInput* getInput(String id);
    TrunkInput* getOutput(String id);
    static const int HORIZONTAL=0;
    static const int VERTICAL=1;
    static const int LEFT_TO_RIGHT=0;
    static const int RIGHT_TO_LEFT=1;
    static const int TOP_TO_BOTTOM=0;
    static const int BOTTOM_TO_TOP=1;
    static const int LEFT=0;
    static const int RIGHT=1;
    static const int TOP=0;
    static const int BOTTOM=1;

    void doLassoMove(const MouseEvent&e);
    void doAssemblyMove(int dx, int dy);
    void doAssemblySetProps();
    int getOrientation();
    MainComponent* getMainPanel() const throw()
    {
        return findParentComponentOfClass<MainComponent>();
    }
    void doRemoveInput(TrunkInput* t);
    bool isAssemblyNeighbour(Trunk* t);
    Colour getWireColour(String wireId);
    bool isAssemblyEnd();
    void setInputTopLeftPosition(TrunkInput* t, int x, int y);
    void saveInputProps(TrunkInput* t);
    void getInputsLike(TrunkInput* t, std::vector<TrunkInput*>& inputs);
    int getRouteHash(TrunkInput* t);
    bool isRevConnect(TrunkInput* t);
    void setInputIndex(TrunkInput* t, int apertureIndex);
    int getDirection();
    bool isFirstSection();
    bool isLastSection();
    int getAssemblyIndex();
    int getAssemblySize();
    int getMaxAssemblyIndex();
    virtual String getAssemblyId();
    int getAppendX(const MouseEvent& e);
    int getAppendY(const MouseEvent& e);
    int getMidX();
    int getMidY();

    void lasso(bool lassoed);
    void doSelectedGroupMove(int dx, int dy);
    void doSetPositionProps();
    void initMove();
    void autoDragIfRequired(int x, int y, int mx, int my);

    void moveToolMouseDown(const MouseEvent& e);
    void trunkToolMouseDown(const MouseEvent& e);
    void deleteToolMouseClick(const MouseEvent& e);
    void selectToolMouseClick(const MouseEvent& e);
    void pointerToolMouseClick(const MouseEvent& e);
    virtual void bringToFront();
    void doBringToFront();
    void doRevertForegrounded();
  
private:
    static const int MOVING=1;
    static const int RESIZINGLEFT=2;
    static const int RESIZINGRIGHT=3;
    static const int RESIZINGTOP=4;
    static const int RESIZINGBOTTOM=5;
    static const int DRAGWIRE=6;
    static const int RESIZING=7;
    static const int EXTEND=8;

    int mouseDownX;
    int mouseDownY;
    int mouseDownWidth;
    int mouseDownHeight;
    String selected_;
    String foregrounded_;
    int orientation_;
    bool extending_;
    bool inEZone_;
    int mouseMode;
    Trunk (const Trunk&);
    const Trunk& operator= (const Trunk&);
    void doMove(const MouseEvent& e);
    void doResize(const MouseEvent& e);
    bool deleteOnDragEnd_;
    bool isPerpendicularMovement(const MouseEvent& e);
    bool inExtendZone(const MouseEvent& e);
    void checkEZone(const MouseEvent& e);
    bool mouseOver_;
    int getMinWidth();
    int getMinHeight();
    void doAtResizeLimit();

    int getTrueTrunkLength();
    float getTrueBorderWidth();
    float getTrueApertureWidth();

    int getTrueTrunkThickness();
    int getApertureLat(int index);
    int getApertureLong(int index);
    int doGetApertureIndex(int lng, int lat, bool endsAllowed);
    bool onLeftBorder(int x);
    bool onRightBorder(int x);

    int resizeLeft(int x, int newWidth);
    int resizeTop(int y, int newHeight);
    int resizeBottom(int newHeight);
    int resizeRight(int newWidth);

    void autoResizeBottom(int dy);
    void autoResizeTop(int dy);
    void autoResizeRight(int dx);
    void autoResizeLeft(int dx);

    void zoomInputs();
    void repositionInputs(bool);
    void createInput(String id, int index);
    void createOutput(String id, int index);

    void drawTrunkWire(Graphics& g, String id, float pos, const Colour col);
    void drawTrunkWires(Graphics& g);
    int toTrue(int);
    void doMouseClick (const MouseEvent& e);
//    void bringToFront();
};


#endif
