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

#ifndef __BOX__
# define __BOX__
#include <piw/piw_tsd.h>
#include "juce.h"
#include "ToolManager.h"
#include "PropertyEditor.h"
#include "SingleInputEditor.h"
#include "MainComponent.h"
#include "DialogComponent.h"
#include "DeleteAgentConfirmation.h"
#include "DeleteInstanceConfirmation.h"
#include "HelpComponent.h"
#include "CustomCursor.h"
#include "RoutingElement.h"
#include "sizes.h"
#include "Wire.h"
#include <list> 
#include <vector>
#include <iostream>
#include <map>
#include <set>

class SourcePin;
class DestinationPin;
class ReverseSourcePin;
class ReverseDestinationPin;
class MainComponent;
class Wire;

class ExpandButton :public ImageButton
{
public:
    ExpandButton(String);
    ~ExpandButton(){};

protected:
    virtual void clicked(const ModifierKeys &modifiers);
};

class BoxPos
{

public:
    BoxPos(String,int, int);
    ~BoxPos(){};
    String getId();
    int getX();
    int getY();

private:
    String id_;
    int x_;
    int y_;
};

class Box  :public Component,
            public SettableTooltipClient,
            public Button::Listener,
            public Selectable,
            public Highlightable,
            public Movable
{
public:
Box(Agent* agent,String id,int boxType,float zoomFactor);
Box(Atom* atom,String name, String id,int childNo,int boxType,float zoomFactor,int level);
Box(String name, String id,int childNo,int boxType,float zoomFactor,int level, bool revconnect);

~Box();
static const int TOPLEVEL=0;
static const int CHILD=1;
static const int SINGLE_INPUT=2;

bool isTopLevel();
bool isMoving();
bool isChild();
bool isSingleInput();
void printWireInfo();
Atom* getAtom();
void setUsingChanged(bool);
bool getUsingChanged();
int getOverallTrueHeight();
int getOverallBoxCount();
void layout();
void doZoom(float zoomFactor);
void postLayout(int x, int y);
void zoomChildBoxes();
void testZoomChildBoxes();
void parentLayout();
virtual void buttonClicked (Button* buttonThatWasClicked);
SourcePin* getSrcPin();
DestinationPin* getDstPin();
ReverseSourcePin* getRevSrcPin();
ReverseDestinationPin* getRevDstPin();

void paintOverChildren (Graphics& g);
void paint (Graphics& g);
void topLevelPaint (Graphics& g);
bool hitTest(int x,int y);
void mouseDown (const MouseEvent& e);
void mouseUp (const MouseEvent& e);
void mouseEnter(const MouseEvent& e);
void mouseExit(const MouseEvent& e);
void mouseDrag (const MouseEvent& e);
void mouseMove (const MouseEvent& e);
void mouseDoubleClick (const MouseEvent& e);
void doMouseClick (const MouseEvent& e);
void showProperties();
void showConnectionInfo();
String getId();
void doAutoDrag(int dx,int dy);
void highlight(bool on);
void setTrueY(int y);
void setTrueHeight(int h);
int getTrueX();
int getTrueY();
int getTrueHeight();
void addChildBox(Atom* atom);
void addInputBox(String u);
Box* const getBoxAt(int x, int y);
Box * getBoxById(String id);
Box * getTopLevelBox();
void onDelete();
void onContract(bool clearExpandedList);
void doContract(bool clearExpandedList);
void doClickContract(bool clearExpandedList);
String getTooltip();
void setTooltip(const String& newTooltip){};
float getTextIndent();
void nameChanged();
void changed(PropertyStore* p);
void addSrcWire(Wire* w);
void addDstWire(Wire* w);
void removeSrcWire(Wire* w);
void removeDstWire(Wire* w);
void invalidateWires();
void forceInvalidateWires();
void initBoxMove();
void removeDrawingWires();
void preDrawWires();
void getSrcWires(std::map<String,Wire*>& m);
void getDstWires(std::map<String,Wire*>& m);
int getTrueBoxIndent();
int getSrcX();
int getSrcY();
int getDstX();
int getDstY();

bool hasInputBoxes();
bool isFree();
void setFree(bool f);
Box* getFreeInputBox();
Box* getNamedInputBox(String name);
void changeName(String name);
String get_fulldesc();
void invalidateSingleInputBoxes();
bool isExpanded();
void setSelected(bool selected);
void setMouseOver(bool over);
void refresh();
void refreshExpanded();
void refreshExpand();
void refreshButton();
bool canCreate();
void setph(float ph);
void setpy(float py);
PropertyStore* get_props();

void lasso(bool lassoed);
void doMove(const MouseEvent& e);
void doSelectedGroupMove(int dx, int dy);
void doSetPositionProps();
void setTrueBounds();
void initMove();
void autoDragIfRequired(int x, int y, int mx, int my);
bool isForegrounded();
virtual void setForeground(bool, bool);
void bringToFront();
void setExpanded();

void addObscuredBox(String id);
std::set<String> getObscuredBoxIds();
int getObscuredBoxTrueX();
int getObscuredBoxTrueY();
void clearObscuredBoxIds();
void setAutoPositioned(bool);
bool isAutoPositioned();
void cachePosition(String id);
void returnToManualPosition();
void returnToManualPosition(String id);

bool inVisibleArea(int x, int y);
bool inChildVisibleArea(int x, int y);
bool childHitTest(int x, int y);
bool posInBox(int x, int y);
void setZoomFactor(float zoomFactor);
void doWiringHighlights(const MouseEvent& e);
void clearHighlights();

void pointerToolMouseClick(const MouseEvent& e);
void selectToolMouseClick(const MouseEvent& e);
void deleteToolMouseClick(const MouseEvent& e);
void editToolMouseClick(const MouseEvent& e);
void createToolMouseClick(const MouseEvent& e);

void moveToolMouseUp(const MouseEvent& e);

void helpToolMouseEnter(const MouseEvent& e);
void moveToolMouseEnter(const MouseEvent& e);
void deleteToolMouseEnter(const MouseEvent& e);
void editToolMouseEnter(const MouseEvent& e);
void createToolMouseEnter(const MouseEvent& e);
void pointerToolMouseEnter(const MouseEvent& e);
void wiringToolMouseEnter(const MouseEvent& e);

void setCreateButtonTooltip(String name);
void setCreateInstanceName(String name);
bool wiringAllowed();
void expandButtonClicked(const ModifierKeys& mods);
bool connectedToSelectedBox();
void connectedBoxForegroundWires();
void foregroundWires(bool shouldBeForegrounded);
bool isReversed();
void doExpand(bool, bool saveprops);
bool isRig();
void refreshPinVisibility();
void doSetPositionProps(int x, int y);
void setScreenPos(int x, int y);

private:

    void doSetup(Atom* atom,String name, String id,float zoomFactor,int level);
    void doSetup(String name, String id,float zoomFactor,int level);
    bool revconnect_;
    void setupButtons();
    void setupSourcePins();
    void setupDestinationPins();
    bool isExpandable();
    void doExpandableSetup();
    void doNonExpandableSetup();
    void doExpandedButton();
    void doExpandableButton();
    void doNonExpandableButton();
    void setExpandedProps();
    void zoomPins();
    void zoomButton();
    void sizeButton();
    Atom* atom_;
	int mouseDownX;
    bool expanded;
    int mouseDownY;

	SourcePin* srcPin_;
    ReverseSourcePin* revSrcPin_;
	DestinationPin* dstPin_;
    ReverseDestinationPin* revDstPin_;

    String id_;

    ExpandButton* expandButton;
    TextButton* createButton_;
    TextButton* deleteButton_;
    TextButton* rigButton_;

	MainComponent* getMainPanel() const throw();
    int trueWidth_;
    int trueHeight_;
    int trueVisibleHeight_;
    int trueX_;
    int trueY_;
    float zoomFactor_;
    std::list<Box *> boxes_;
    int level_;
    PropertyStore* props_;
    bool move_init_;
    std::map<String,Wire*> srcWires_;
    std::map<String,Wire*> dstWires_;
    bool free_;
    int boxType_;
    bool selected_;
    bool mouseOver_;
    bool expandable_;
    bool rig_;
    void showNameEditor();
    String truncateName(String name, int w, Graphics& g);
    bool foregrounded_;

    void doMoveObscuredBoxes();
    void doReturnObscuredBoxes();
    int  obscuredBoxTrueX_;
    int  obscuredBoxTrueY_;
    bool autoPositioned_;
    std::vector<BoxPos*>obscuredBoxPositions_;
    bool wasObscuredBy(String id);
    void doReturnToManualPosition(int x, int y);
    void clearStoredPositions();

    bool isOverPin(int x,int y);
    void rigButtonClicked();
    void createButtonClicked();
    void deleteButtonClicked();
    String createInstanceName_;
    std::set<String> obscuredBoxIds_;
    void doForegroundOff();
    bool usingChanged_;
    float titleBoxHeight_;
    void doSrcPinVisibility();
    void doDstPinVisibility();
    void checkPos();
    Colour getColour(String);
    Colour getMovingColour(String);
    bool moving_;
}; // end of class Box

#endif
