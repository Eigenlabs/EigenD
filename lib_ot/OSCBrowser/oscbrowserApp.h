/*
    This file is part of OSCMap. Copyright (c) 2005 Rémy Muller. 

    OSCMap is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef OSCBROWSER_APP_H
#define OSCBROWSER_APP_H

// Define a new application type, each program should derive a class from wxApp
class oscbrowserApp : public wxApp
{
public:
    oscbrowserApp();
    virtual ~oscbrowserApp();

    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

DECLARE_APP(oscbrowserApp)

#endif
