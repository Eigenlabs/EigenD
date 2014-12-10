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

#ifndef __MAINCOMP__
#define __MAINCOMP__
#include <piw/piw_tsd.h>
#include "juce.h"

#include <map>
#include <list>
#include <set>
#include <vector>
#include "WorkbenchColours.h"
#include "ToolManager.h"
#include "Pin.h"
#include "Wire.h"
#include "HelpViewComponent.h"
#include "CreateAgentComponent.h"
#include "CreateInstanceComponent.h"
#include "DeleteInstanceComponent.h"
#include "FindComponent.h"
#include "RoutingElement.h"
#include "Box.h"
#include "sizes.h"
#include "PathCalculator.h"
#include "Hook.h"
#include "Peg.h"
#include "Trunk.h"
#include <iostream>
#include "utils.h"
#include "MenuManager.h"
#include "DialogComponent.h"
#include "WirePropertyPanel.h"
#include "DeleteWireConfirmation.h"
#include "DeleteRouteConfirmation.h"
#include "PreferenceManager.h"
#include "PreferenceComponent.h"
#include "ErrorReportComponent.h"
#include "ProgressLayer.h"

class MenuManager;
class MainComponent;
class Trunk;
class TrunkCorner;
class TrunkDrawingTool;
class Peg;
class Box;
class Workspace;
class Wire;
class ToolManager;
class TrunkInput;
class Pin;

class ChangeWireProperties
{
public: 
    ChangeWireProperties(String id, String srcId, String dstId,String u, String filter, String c);
    ~ChangeWireProperties();
    String getId();
    String getSrcId();
    String getDstId();
    String getUsing();
    String getFilter();
    String getControl();

private:
   String id_;
   String sid_;
   String did_;
   String u_;
   String f_;
   String c_;
};

class StoredRouteElement
{
public:
    StoredRouteElement(String id, int input, int output, bool revconnect);
    ~StoredRouteElement();
    String getId();
    int getInput();
    int getOutput();
    bool isRevConnect();
    bool expired(int lifetime);
    void tick();

private:
    String id_;
    int input_;
    int output_;
    bool revconnect_;
    int createTime_;
};

class StoredDropPin
{
public:
    StoredDropPin(Wire* w, Pin* p);
    ~StoredDropPin(){};
    Wire* getWire();
    Pin* getPin();

private:
    Wire* w_;
    Pin* p_;
};


class SelectedItems: public SelectedItemSet<Selectable*>
{
public:
    void itemSelected (Selectable* item);
    void itemDeselected(Selectable* item);

};

class WBScrollListener: public ScrollBar::Listener, public Timer
{
public:
    WBScrollListener(Viewport* comp);
    virtual void scrollBarMoved(ScrollBar* scrollBarThatHasMoved, double newRangeStart);
    virtual void timerCallback();

private:
    Viewport* comp_;
};

class WBViewport: public Viewport
{
public:
    WBViewport(const String& name);
    virtual void parentSizeChanged();
private:
    WBScrollListener* hbarListener_;
    WBScrollListener* vbarListener_;
};

class Anchor: public Component
{
public:
    Anchor(int x, int y);
};

class DummyBox: public Component,public Timer
{
public:
    DummyBox(int x, int y, float zoomFactor);
    void paint (Graphics& g);
    virtual void timerCallback();
private:
    float zoomFactor_;
};

class DummyInput: public Component
{
public:
    DummyInput (int x, int y, float zoomFactor);
    void paint (Graphics& g);

};

class DisplayLayer:public Component
{

public:
    DisplayLayer(MainComponent* mc);
    void paint(Graphics& g);

private:
    MainComponent* mc_;
};

class WorkspaceFactory
{
public:
    virtual ~WorkspaceFactory(){};
    virtual void showWorkspace(String name, String scope, String id)=0;
    virtual void nameChanged(String name, String abs_id)=0;
    virtual void remove(String abs_id)=0;
    virtual void removeCurrentTab()=0;
    virtual void removeTab(String name)=0;
};


class MainComponent  : public Component,
                       public TooltipClient,
                       public MultiTimer,
                       public WorkspaceListener,
                       public LassoSource<Selectable*>,
                       public ChangeListener,
                       public PreferenceManager

                       
{
public:
    MainComponent (ToolManager* tm, Workspace* model, MenuManager* mm, HelpViewComponent* hv, WorkspaceFactory* wsf);
    ~MainComponent();

    void setViewport(WBViewport* vp);
    Component* getDisplayLayer();
    void deleteBox(Box* b);
    void contractBox(Box* b, bool clearExpandedList);
    void deleteTopLevelBox(Box* b);
    void deleteAssemblyElement(RoutingElement* p);
    void deleteRoutingElement(RoutingElement* p);
    void deleteAssembly(String id);
    void doDeleteRoutingElement(RoutingElement* p);
    void createAgentBox(int x,int y, String agentType, int ordinal);
    void connectionRemoved(String wireId);
    void deleteAgent(Box* b);
    void showHelpText(String help);

    virtual void agentAdded(String id);
    virtual void connection_created(String id);
    virtual void agentRemoved(String id);
    virtual void instanceRemoved(String id);
    virtual void portRemoved(String id);
    virtual void nameChanged(String id);
    virtual void instanceName(String id, String name);
    virtual void checkConnections(String id, std::set<String> conset );
    virtual void agentsUpdated(const std::set<std::string> & agents);
    virtual void instancesUpdated(const std::set<std::string> & ords);

    virtual void cinfo_updated(String s, String path, const std::set<std::string> & cinfo);
    virtual void finfo_updated(String s, String path, const std::set<std::string> & finfo);
    virtual void enumerate_updated(String s,String path, int nf,int nc);
    virtual void activated(String s);
    virtual void current(String id, String cookie);
    virtual void sourcekeys_updated(String id, String keys);
    virtual void report_error(String err1, String err2);

    void deleteWire(Wire* w);
    void changeWire(Wire* w, String u, String f, String c);
    void changeWirePart2(String id);
    void createHook(int x,int y);
    void createSourcePin(int x, int y);
    void createDestinationPin(int x, int y);

    void create_connection(ChangeWireProperties* cw);
	void paint (Graphics& g);
	void paintOverChildren (Graphics& g);
    void resized();

	void showTools(const MouseEvent& e);

	void startWireDrag(const MouseEvent& e);
	void endWireDrag(const MouseEvent& e);
	void wireDrag(const MouseEvent& e);

    Movable* getMovableAt(int x, int y);
    Box* const getBoxAt(int x, int y);
    Box* const getBoxExactlyAt(int x, int y);
    Box* const getBoxNear(int x, int y, int tol, int step);
    SourcePin* getSourcePinAt(int x, int y);
    DestinationPin* getDestinationPinAt(int x, int y);
    RoutingElement* getRoutingElementAt(int x, int y);
    RoutingElement* getRoutingElementExactlyAt(int x, int y);
    RoutingElement* getRoutingElementNear(int x, int y, int tol, int step);
    TrunkCorner* getTrunkCornerAt(int x, int y);
    Trunk* getTrunkAt(int x, int y);
    Peg* getPegNear(int x, int y, int tol);

    void removeLoosePin(DestinationPin* p);
    void removeLoosePin(SourcePin* p);
    void selectWire(Wire* w);
    void showPopupMenu(const MouseEvent& e);
    void dropLoosePin(const MouseEvent& e);
    void dropLoosePin(DestinationPin* p, int x, int y);
    void dropLoosePin(SourcePin* p, int x, int y);
    void doDropPin(String sid, String did, Wire* w, DestinationPin* p);
    void doDropPin(String sid, String did, Wire* w, SourcePin* p);

    void dstPinMouseDown(const MouseEvent& e);
    void dstPinMouseUp(const MouseEvent& e);
    void dstPinMouseDisconnectDrag(const MouseEvent& e);
    void dstPinMouseDrag(const MouseEvent& e);

    void srcPinMouseDown(const MouseEvent& e);
    void srcPinMouseUp(const MouseEvent& e);
    void srcPinMouseDrag(const MouseEvent& e);
    void srcPinMouseDisconnectDrag(const MouseEvent& e);

    SourcePin* getSrcPinById(String id);
    DestinationPin* getDstPinById(String id, Wire* w);

    void removeWireFromRoutingElement(String id, RoutingElement * r);
    void removeRouteFromRoutingElement(String id, RoutingElement * r);
    void removeRouteFromTrunkAssembly(String id, String assemblyId);

    Box* getBoxById(String id);
    Box* getBoxByName(String name);

    void mouseEnter (const MouseEvent& e);
    void mouseDown (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseMove (const MouseEvent& e);
    void mouseDoubleClick (const MouseEvent& e);
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails &wheel);

    float getZoomFactor();
    void doZoomInc(float zoomInc,int x, int y, bool save);
    void doZoom(float zoomFactor, int x, int y);
    void doZoom(float zoomFactor);

    bool doHighLights(const MouseEvent& e);
    bool isCircular(Box* b);
    void startCanvasDrag(const MouseEvent& e);
    void canvasDrag(const MouseEvent& e);
    void endCanvasDrag(const MouseEvent& e);
    void doCanvasDrag(int x, int y);
    void startZoomDrag(const MouseEvent& e);
    void zoomDrag(const MouseEvent& e);
    void zoomDragFullDraw();
    void endZoomDrag(const MouseEvent& e);

    void startCreateTrunkDrag(const MouseEvent& e);
    void createTrunkDrag(const MouseEvent& e);
    void endCreateTrunkDrag(const MouseEvent& e);

    bool inViewPortX(int x);
    bool inViewPortY(int y);
    bool inCentralViewPortX(int x,int tol);
    bool inCentralViewPortY(int y,int tol);

    bool inViewPort(int x, int y);
    bool inCentralViewPort(int x, int y);
    bool offCanvas(int x, int y, int w, int h);
    bool inAutoDragRegion(int x,int y);
    void autoDrag(int x, int y,bool canvas);
    void autoComponentDrag(int x, int y,AutoDragable* b);
    void stopAutoDrag();
    virtual void timerCallback(int timerId);
    int getDragDirection(int x, int y);
    int getReverseDragDirection(int x, int y);

    Path getWirePath(int x1, int y1,int x2,int y2,float lengthFactor);
    Path getLinearWirePath(int x1, int y1,int x2,int y2);
    int conditionX(int x, int w);
    int conditionY(int y,int h);
    void clearHighlight();

    String getWireId(SourcePin* srcPin,DestinationPin* dstPin);
    void deleteWiresForBox(Box* b);
    String getLoosePinId();
    void addLooseWire(String srcId, String dstId);
    void addWire(Connection* con);
    Wire* getWireForDstPin(DestinationPin * dp);
    Wire* getWireForSrcPin(SourcePin * dp);
    Wire* getWireById(String wid);
    virtual bool keyPressed(const KeyPress &key);
    virtual bool keyStateChanged(bool isKeyDown);
    Peg* droppedOnPeg(const MouseEvent& e);

    void highlightPin(DestinationPin* d, bool hl);
    void highlightPin(SourcePin* d, bool hl);
    void menuItemSelected(int menuItemID, int topLevelMenuIndex);
    int getDefaultX();
    int getDefaultY();

    void doMouseClick(const MouseEvent& e);
    void doCreate(const MouseEvent& e);
    int getTool();
    void setTool(int);
    const MouseCursor getCursor();
    const MouseCursor getCursor(int cur);
    void save_props(String id);

    virtual void hookAdded(String id);
    virtual void horizontalTrunkAdded(String id);
    virtual void verticalTrunkAdded(String id);
    virtual void trunkCornerAdded(String id);
    virtual void routingAdded(String id);
    virtual void sourcePinAdded(String id);
    virtual void destinationPinAdded(String id);
    virtual void looseWireAdded(String id);
    virtual void propertiesAdded(String id);

    virtual void agentPropertyChanged(String id);
    virtual void hookChanged(String id);
    virtual void horizontalTrunkChanged(String id);
    virtual void verticalTrunkChanged(String id);
    virtual void trunkCornerChanged(String id);
    virtual void routingChanged(String id);
    virtual void sourcePinChanged(String id);
    virtual void destinationPinChanged(String id);
    virtual void looseWireChanged(String id);
    virtual void propertiesChanged(String id);

    virtual void hookRemoved(String id);
    virtual void horizontalTrunkRemoved(String id);
    virtual void verticalTrunkRemoved(String id);
    virtual void trunkCornerRemoved(String id);
    virtual void routingRemoved(String id);
    virtual void propertiesRemoved(String id);
    
    RoutingElement* getRoutingElement(String id);

    void storeRoute(Wire* w, std::list<StoredRouteElement>& route);
    void saveRoute(String id, std::list<StoredRouteElement> route);
    void addStoredRoute(Wire* w, std::list<StoredRouteElement> route);
    void addReverseStoredRoute(Wire* w, std::list<StoredRouteElement> route);

    void loosePinDrag(DestinationPin * dp, int x, int y,const MouseEvent& e);
    void loosePinDrag(SourcePin * sp, int x, int y,const MouseEvent& e);
    void routeThroughPeg(DestinationPin* dp, int x, int y);
    void routeThroughPeg(SourcePin* sp, int x, int y);
    void routeThroughPegFront(Wire* w, int x, int y);
    void routeThroughPegBack(Wire* w, int x, int y);
    int getInput(RoutingElement* r, Wire* w);
    int getOutput(RoutingElement* r, Wire* w);

    void createTrunkAssembly(TrunkDrawingTool* tdt, String assemblyId, int offset,int cornerOffset);
    void createReverseTrunkAssembly(TrunkDrawingTool* tdt, String assemblyId, int offset,int cornerOffset);
    void appendTrunkAssembly(TrunkDrawingTool* tdt, Trunk* t);
    void moveAssembly(String assemblyId, int dx, int dy);
    void moveSubAssemblyX(String assemblyId, int assemblyIndex, int direction,int pos,int dx);
    void moveSubAssemblyY(String assemblyId, int assemblyIndex, int direction,int pos,int dy);
    void mouseOverAssembly(String assemblyId, bool over);
    void lassoAssembly(String assemblyId, bool over);

    void setAssemblyProps(String assemblyId);
    void trunkInputMouseDown(const MouseEvent& e);
    void trunkInputMouseDrag(const MouseEvent& e);
    void trunkInputMouseUp(const MouseEvent& e);
    void trunkInputMouseEnter(const MouseEvent& e);
    void trunkInputMouseExit(const MouseEvent& e);
    virtual String getTooltip();
    void addBox(String id,Box* box);
    void removeBox(String id);
    void testRemoveBox(String id);
    void thingMoved();
    void thingMovedFullDraw();
    void thingChangedFullDraw();
    void thingChanged();
    void endThingMoved();
    void endThingChanged();
    void clearWireMoveList();
    void addToWireMoveList(Wire* w);

    void initRoutingElementMove(RoutingElement* r);
    void initAssemblyMove(String id);
    void initTrunkInputMove(String id);
    void initSourcePinMove(String id);
    void initDestinationPinMove(String id);
    
    void addForegroundWire(Wire* w);
    void removeForegroundWire(Wire* w);
    void removeLooseWire(Wire* w);
    void removePendingWire(Wire* w);
    void addDrawingWire(Wire* w);
    void removeDrawingWire(Wire* w);
    void changeDrawingWire(Wire* w);

    void startExtendTrunkMouseDrag(const MouseEvent& e);
    void extendTrunkMouseDrag(Trunk* t, const MouseEvent& e);
    void extendTrunkMouseUp(Trunk* t,const MouseEvent& e );
    void updateAssemblySize(String assemblyId, int newSize);
    void updateAssemblyIndices(String assemblyId, int newSize);
    int getNumCorners(String assemblyId);

    void appendTrunkAssemblyAtEnd(TrunkDrawingTool* tdt, Trunk* t);
    void appendTrunkAssemblyAtStart(TrunkDrawingTool* tdt, Trunk* t);
    void appendToSingleSectionAssembly(TrunkDrawingTool* tdt, Trunk* t);

    void selectWireForPin(DestinationPin* d, bool hl);
    void selectWireForPin(SourcePin* d, bool hl);
    void showCreateDialog(const std::set<std::string>& agentNames);
    void showFindDialog();
    void showPreferencesDialog();
    virtual void handleCommandMessage(int commandId);
    Wire* getWireWithSameRoute(Wire* w);
    void cacheWire(Wire* w);
    void removeCachedWire(Wire* w);
    Box* getTopLevelSrcBox(Wire* w);
    Box* getTopLevelDstBox(Wire* w);
    Box* getTopLevelBox(DestinationPin* dp);
    Box* getTopLevelBox(SourcePin* sp);
    void checkPendingWires();
    void checkPendingWires(String aid);
    void drawForegroundWires(Graphics& g);
    bool hideMetronomeOutputs();
    bool hideControllerOutputs();
    String get_fulldesc(String id);
    std::vector<String> getTopLevelBoxNames();
    bool isRoutingElementNear(RoutingElement* p, int x, int y, int tol);
    void createInstance(Atom* a, String name);
    void deleteInstance(Atom* a);
    void get_connections(String id);
    bool isConnectionFrom(Wire* w, String agentType);
    virtual SelectedItemSet<Selectable*> & getLassoSelection();
    virtual void findLassoItemsInArea(Array<Selectable*>&itemsFound,const juce::Rectangle<int>& area);
    void initSelectedItemMove();
    void moveSelectedGroup(int dx, int dy);
    void setSelectedGroupProps();
    void clearSelectedItems();
    void clearSelectedItemsIfOne();
    void selectOnly(Selectable* b);
    void addToSelection(Selectable* b);
    void addToSelectionBasedOnModifiers(Selectable* b,const ModifierKeys& mods);
    void deselect(Selectable* b);
    void showProperties(Box* b);
    void refreshProperties(Box* b);
    void refreshProperties(String id);
    int getGridNumber(int,int);
    void clearGridMap(Wire*);
    void updateGridMap(int, Wire*);
    bool hasForegroundWires();
    bool hasForegroundedItem();
    void moveObscuredBoxes(Box*);
    void returnObscuredBoxes(Box*);
    virtual void changeListenerCallback(ChangeBroadcaster* source);
    bool showOutlines();
    void clearCandidateWiringBox(Box* b);

    void moveToolMouseMove(const MouseEvent& e);
    void moveToolMouseDown(const MouseEvent& e);
    void moveToolMouseDrag(const MouseEvent& e);
    void moveToolMouseUp(const MouseEvent& e);

    void wiringToolMouseMove(const MouseEvent& e);
    void wiringToolMouseDrag(const MouseEvent& e);
    void wiringToolMouseUp(const MouseEvent& e);

    void wiringLeftMouseDrag(const MouseEvent& e);
    void wiringLeftMouseUp(const MouseEvent& e);
    void deleteToolMouseClick(const MouseEvent& e);
    void pointerToolMouseClick(const MouseEvent& e);
    void findUpstream();
    void findDownstream();
 
    void foregroundAssembly(String assemblyId,bool foregrounded,String id); 
    void revertForegroundAssembly(String assemblyId);
    void bringToFrontAssembly(String assemblyId); 
    void toBackAssembly(String assemblyId); 
    void toFrontAssembly(String assemblyId); 

    bool requiresDeleteConfirmation(String t);
    void setDeleteConfirmationRequired(String t);

    virtual void setProperty(String,bool);
    virtual bool getProperty(String,bool);

    void setViewPosition(int x, int y,bool save);
    bool isSelected(Selectable*);
    void showWorkspace(String tabName,String scope, String id);
    Workspace* getWorkspace();
    int getNumWires();
    int getNumPendingWires();
    void doToolChangeIfRequired();
    void doMoveToolChangeIfRequired();
    void stopMonitor(Atom* atom);
    void setDraftMode(bool);
    void insertLooseWire(Wire* w);
    void repaintWire(Wire* w);
    void repaintReRoutedWire(Wire* w);
    void repaintSelectedWire();
    Wire* getSelectedWire();
    void getTemporaryPosition(int& x, int& y);
    virtual void connectionPossible(String sid, String did, bool possible);

private:
    int mouseDownX_;
	int mouseDownY_;
    int zoomDragX_;
    float zoomTot_;
    String sourceId;
	Box* box;
    AutoDragable* autoDragComp_;
    Highlightable* highlight_;
    SelectedItems selectedItems_;
    Wire* selectedWire_;
    Path testPath_;
    DestinationPin* dragPin_;
    SourcePin* dragSrcPin_;
    Pin* eventOriginPin_;
    bool dragPinReady_;
    bool dragSrcPinReady_;
    bool dragInputReady_;
	MainComponent (const MainComponent&);
    const MainComponent& operator= (const MainComponent&);
    float zoomFactor_;
    int trueWidth_;
    int trueHeight_;
    int nx_; 
    int ny_;
    int mouseMode_;
    int paintMode_;
    int paintOverMode_;
    int paintForegroundMode_;
    bool autoDragMode_;
    int autoDragDirection_;
    int dx_;
    int dy_;
    static const int DRAGCANVAS=1;
    static const int DRAGWIRE=6;
    static const int DRAGZOOM=7;
    static const int CREATE_TRUNK=8;
    static const int NORTH=1;
    static const int SOUTH=2;
    static const int WEST=3;
    static const int EAST=4;
    static const int FULL=0;
    static const int QUICK=1;
    static const int LASSO=12;

    WBViewport* vp_;
    ToolManager* tm_;
    MenuManager* mm_;
    HelpViewComponent* hv_;
    int connectorDragX_;
    int connectorDragY_;
    Workspace* model_;

    std::map<String,Wire*> wires_;
    std::map<String,Box*> boxes_;
    std::map<String,DestinationPin*> loose_dps_;
    std::map<String,SourcePin*> loose_sps_;
    std::map<String,Peg*> pegs_;
    std::map<String,Trunk*> trunks_;
    std::map<String,TrunkCorner*> corners_;
    
    std::map<String,Wire*>foregroundWires_;
    std::map<String,Wire*>looseWires_;
    std::map<String,Wire*>normalWires_;
    std::map<String,Wire*>pendingWires_;
    std::map<int,int>routes_;
    std::map<String,ChangeWireProperties*>changeWires_;

    std::map<int,std::list<Wire*> >gridSquareMap_;

    int loosePinCount_;
    std::map<String,std::list<StoredRouteElement> >storedRouteMap_;
    std::map<String,StoredDropPin*> dropPinMap_;
    bool routeThroughBack_;
    bool routeThroughFront_;
    RoutingElement* currentRoutingElement_;
    Peg* unhookPeg_;
    bool createTrunkReady_;
    TrunkDrawingTool* dummyTrunk_;
    DummyInput* dragInput_;
    TrunkInput* currentInput_;
    Trunk* currentTrunk_;
    void doMoveSubAssemblyGreater(int dx, int dy, String assemblyId, int assemblyIndex);
    void doMoveSubAssemblyLess(int dx, int dy, String assemblyId, int assemblyIndex);
    void doMoveCornerAssemblyGreater(int dx, int dy, String assemblyId, int assemblyIndex);
    void doMoveCornerAssemblyLess(int dx, int dy, String assemblyId, int assemblyIndex);
    SourcePin * getLooseSourcePinById(String id);
    DestinationPin * getLooseDestinationPinById(String id);
    String getParentId(String id);
    String tooltip_;
//    Wire* toggledWire_;

    std::set<Wire*> wireMoveList_;
    std::vector<Wire*> getSameRouteWires(Wire* w);
    std::vector<Wire*> dragWires_;
    std::vector<Wire*> wiresAt_;

	void doWireDrag(const MouseEvent& e, Wire* w);
	void doEndWireDrag(const MouseEvent& e, Wire* w, Peg* p);
    int createDialogX_;
    int createDialogY_;
    String currentCreateAgent_;
    bool hideMetronomeOutputs_;
    bool hideControllerOutputs_;
    DisplayLayer* displayLayer_;
    RoutingElement* overElement_;
    Atom* instanceParent_;
    String instanceName_;
    void showCreateInstanceDialog(Atom* a, const std::set<std::string>& ords);
    void showDeleteInstanceDialog(Atom* a, const std::set<std::string>& ords);
    void initDstPinDisconnectDrag(const MouseEvent& e);
    void initSrcPinDisconnectDrag(const MouseEvent& e);
    void removeLoosePins(Wire* w);
    void looseConnectionRemoved(Wire* w);
    void updateDstBox(Wire* w);
    void getMatchingStoredRoute(String id,std::list<StoredRouteElement>& route );
    void getMatchingReverseStoredRoute(String id,std::list<StoredRouteElement>& route );
    void doStoredRoute(String id, Wire* w);
    void deleteWiresLike(Wire* w);
    void unRouteWiresLike(Wire* w);
    void deleteConnectionsLike(Wire* w);
    void showWireProperties(Wire* w, int, int);
    LassoComponent<Selectable*>* lassoComponent_;
    DialogComponent* dc_;
    bool spaceKeyDown_;
    int storedTool_;
    int getDialogHeight(Component*);
    void foregroundWire(Wire* w);
    void foregroundBoxes(Wire* w);
    void unForegroundBoxes(Wire* w);
    bool animated_;
    bool showOutlines_;
    void zoomChildren();
    bool wireDragStarted_;
    Box* candidateWiringBox_;
    Movable* candidateMovable_;
    void listWiresAt(int x, int y);
    bool wireIsAt(Wire* w,int x, int y);
    unsigned wireIndex_;
    PropertyStore* props_;
    void setWorkspaceProps();
    WorkspaceFactory* wsf_;
    void showOutlines(bool);
    void showMetronomeOutputs(bool,bool);
    void showControllerOutputs(bool,bool);
    bool toolChangeRequired_;
    bool moveToolChangeRequired_;
    bool mouseWheeling_;
    void decrementRoutes(int h, Wire* w, int c);
    void removeUniqueRoute(int h, Wire* w);
    void addRoute(int h, Wire* w);
    void removeOldRoute(int oldh, Wire* w);
    void removeOldRoute1(int oldh);
    void doLeftAlign();
    void doTopAlign();
    Component* toolChangeComponent();
    void getWireDescription(Wire*, String&,String&,String&,String&,String&);
    String getLooseWireDesc(Wire* w);
    void createTooltip(Wire* w,int segmentHash);
    bool overrideDraft_;
    bool foregroundWireOverrideDraft_;
    int pathQuality_;
    void cycleSelectedWire(const MouseEvent& e);
    void doTrunkInputMouseDrag(Trunk*, int, int);
    void doCurrentTrunkInputMouseDrag(Trunk*, int, int);
    void doOtherTrunkInputMouseDrag(Trunk*, int, int);
    void doWasInput(Trunk*, int, int, String);
    void doWasOutput(Trunk*, int, int, String);
};

#endif
