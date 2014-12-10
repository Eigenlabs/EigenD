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

#ifndef __WIRE__
#define __WIRE__

#include <piw/piw_tsd.h>
#include "juce.h"
#include "WorkbenchColours.h"
#include "MainComponent.h"
#include "RoutingElement.h"
#include "Pin.h"
#include <iostream>
#include <list>

class Hook;
class DestinationPin;
class MainComponent;
class SourcePin;
class Connection;
class Workspace;
class Wire;

class Segment
{
public:
    Segment(Path p, int hash, Wire* parent);
    ~Segment(){};
    void draw(Graphics& g, float zoomFactor);
    Path getPath();
    int getHash();

private:
    Path p_;
    int hash_;
    Wire* parent_;
};

class Wire 

{
public:	

    Wire(String id, Workspace* w,MainComponent* mc,PropertyStore* p);
    Wire(Connection* c, Workspace* w, MainComponent* mc);
    ~Wire();

    String getId();
    String getUsingId();
    String getSrcId();
    String getDstId();
    String getTopLevelDstId();
    String getTopLevelSrcId();
    void getSegments(bool, std::vector<Segment*> &s);

    DestinationPin * getDstPin();
    SourcePin * getSrcPin();
    void addHook(Hook* hook);
    Hook* getHook();
    void removeHook();
    int getNumRoutingElements();
    RoutingElement*  getLastRoutingElement();
    void addRoutingElement(RoutingElement* p, int input, int output);
    void addRoutingElementAtEnd(RoutingElement* p, int intput, int output);
    void addRoutingElementAtStart(RoutingElement* p, int input, int output);
    void insertRoutingElementBefore(RoutingElement* newElement, RoutingElement* existingElement, int input, int output);
    void insertRoutingElementAfter(RoutingElement* newElement, RoutingElement* existingElement, int input, int output);

    void  getConnectionPath(std::vector<Segment*> &p);
    void  getHookPath(std::vector<Segment*> &p, RoutingElement* posPeg, RoutingElement* r1, bool& hooktodo);
    void  getEndHookPath(std::vector<Segment*> &p, RoutingElement* dp, RoutingElement* r1, bool& hooktodo);
    void removePeg(RoutingElement* p);
    void removeAllPegs();
    Colour getWireColour();
    Path getWirePath();
    void getPathChangeBounds(juce::Rectangle <float>& u);
    void getPathBounds(juce::Rectangle <float>& u);
    Path getHookedArea();
    void setSelected(bool selected);
    void setForegrounded(bool shouldBeForegrounded);
    void preDraw();
    void preAdd();
    bool shouldNotBeLoaded();
    bool isHidden();
    bool routedVia(RoutingElement* p);
    bool isSelected();
    bool isForegrounded();
    bool isLoose();
    void foregroundOn();
    void foregroundOff();
    PropertyStore* getPropertyStore();
    void setChanged();
    void pathChanged();
    void routeChanged(bool srcChanged, bool dstChanged);
    void forceRouteChanged(bool srcChanged, bool dstChanged);
    void forceRouteChanged();
    DestinationPin* getUsingDP();
    SourcePin* getUsingSP();
    bool hasSameRoute(Wire* w);
    int getRouteHash();
    int getOldRouteHash();
    void addRouting();
    void changeRouting();
    void removeRouting();
    void set_dstUsing(String u);
    String get_dstUsing();
    void set_srcFilter(String f);
    String get_srcFilter();
    void set_control(String f);
    String get_control();
    bool isForcedChange();
    void setForcedChange(bool forced);
    String getDescription();
    bool isRevConnect();
    void removeFromPins();
    void setPending(bool shouldBePending);
    bool isPending();
    String getUsingDpId();
    String getUsingSpId();
    int getSegmentHashAt(int x, int y);
    bool hasSegment(int hash);

private:
    String id_;
	bool selected_;
    bool foregrounded_;
    Hook* hook_;
    std::list<RoutingElement *> pegs_;
    std::vector<Segment *> fullSegments_;
    std::vector<Segment *> oldSegments_;
    std::vector<Segment *> draftSegments_;
    Path getFullPath();
    Path getOldPath();
    RoutingElement * hookPos_;
    DestinationPin* using_dp_;
    SourcePin* using_sp_;
    bool dp_ready_;
    bool sp_ready_;
    PropertyStore* props_;
    Workspace* workspace_;
    MainComponent* mc_;
    Connection* connection_;
    String sid_;
    String did_;
    bool loose_;
    bool needsRouteCalculation_;
    void calculateRoute();
    void calculateGrids();
    int hashCode_;
    int oldHashCode_;
    void calcHashCode();
    String dstUsing_;
    String srcFilter_;
    String control_;
    bool forcedChange_;
    Path getOldWirePath();
    void clearGridMap();
    void updateGridMap(int);
    void updateRoutingProps();
    bool pending_;
    void clearRoute();
    int getHash(RoutingElement*, RoutingElement*);
    int getToHookHash(RoutingElement*);
    int getFromHookHash(RoutingElement*);
    void deleteRoute();
    bool metronomeOutput_;
    bool controllerOutput_;
};   // end of class Wire

#endif

