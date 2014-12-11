#include "wx/wx.h"

#include "oscbrowserApp.h"
#include "oscbrowserFrm.h"
#include "resource.h"

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. oscbrowserApp
// and not wxApp)
IMPLEMENT_APP(oscbrowserApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
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

//-----------------------------------------------------------------------------------------
oscbrowserApp::oscbrowserApp()
{
}

//-----------------------------------------------------------------------------------------
oscbrowserApp::~oscbrowserApp()
{
}

//-----------------------------------------------------------------------------------------
// 'Main program' equivalent: the program execution "starts" here
//-----------------------------------------------------------------------------------------
bool oscbrowserApp::OnInit()
{
	// create the main application window
	oscbrowserFrame *frame = new oscbrowserFrame(*this,wxT("Oscbrowser"),wxPoint(50, 50), wxSize(640, 640));

	// and show it (the frames, unlike simple controls, are not shown when
	// created initially)
	frame->Show(TRUE);
	SetTopWindow(frame);

	// success: wxApp::OnRun() will be called which will enter the main message
	// loop and the application will run. If we returned FALSE here, the
	// application would exit immediately.
	return TRUE;
}

