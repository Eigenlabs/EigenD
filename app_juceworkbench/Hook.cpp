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

#include "Hook.h"

Hook::Hook(int x, int y, MainComponent* mc)
{
    x_=x;
    y_=y;
    mc_=mc;
}
Hook::~Hook()
{
}

int Hook::getInputAt(int x, int y)
{
    return 1;
}

int Hook::getOutputAt(int x, int y)
{
    return 1;
}

PropertyStore* Hook::getProps()
{
    return props_;
}

String Hook::get_id()
{
    return String();
}

String Hook::get_hash(String wireId)
{
    return get_id();
}

void Hook::setPos(int x,int y)
{
    x_=x;
    y_=y;
}
int Hook::getX()
{
    return x_;
}
int Hook::getY()
{
    return y_;
}

int Hook::getXin(String id)
{
    return (float)x_/mc_->getZoomFactor();
}

int Hook::getYin(String id)
{
    return (float)y_/ mc_->getZoomFactor();
}

int Hook::getXout(String id)
{
    return getXin(id);
}

int Hook::getYout(String id)
{
    return getYin(id);
}

Path Hook::getPathFrom(RoutingElement* other,String id)
{
    return mc_->getWirePath(other->getXout(id),other->getYout(id),getXin(id),getYin(id),1.1f);
}

void Hook::doZoom(float zoomFactor)
{
}

void Hook::doAutoDrag(int dx,int dy)
{
      setPos(getX()+dx,getY()+dy);
      mc_->repaint();
}
