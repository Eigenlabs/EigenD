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

#ifndef __HOOK__
#define __HOOK__
#include <piw/piw_tsd.h>
#include "RoutingElement.h"
#include "MainComponent.h"
#include <iostream>
#include "juce.h"

class Hook : public RoutingElement,
             public AutoDragable
// This is temporary hook used for dragging the wire
{
public:

Hook(int x, int y,MainComponent* mc);
~Hook();
void setPos(int x,int y);
int getX();
int getY();
virtual int getXin(String id);
virtual int getYin(String id);
virtual int getXout(String id);
virtual int getYout(String id);
virtual String get_id();
virtual String get_hash(String wireId);
virtual Path getPathFrom(RoutingElement* other,String id);
virtual void doZoom(float zoomFactor);
void doAutoDrag(int dx,int dy);
virtual PropertyStore* getProps();
virtual int getInputAt(int x, int y);
virtual int getOutputAt(int x, int y);


private:
int x_;
int y_;
MainComponent* mc_;
}; // end of class Hook
#endif

