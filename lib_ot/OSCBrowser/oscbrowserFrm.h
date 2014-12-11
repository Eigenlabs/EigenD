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

#ifndef OSCBROWSER_FRAME_H
#define OSCBROWSER_FRAME_H

#include "wx/button.h"
#include "wx/listctrl.h"
#include "wx/textctrl.h"
#include "wx/treectrl.h"

#include "OscUdpZeroConfService.h"
#include "OscUdpZeroConfBrowser.h"
#include "OscUdpZeroConfResolver.h"
#include <map>

// Define a new frame type: this is going to be our main frame
class oscbrowserFrame : public wxFrame
                      , public OSCBrowseListener
                      , public OSCResolveListener
{
    OscUdpZeroConfBrowser *browser;

    wxTreeCtrl *tree;

    std::map<string,OscUdpZeroConfResolver*> resolvers;

public:
	oscbrowserFrame(oscbrowserApp& application,const wxString& title, const wxPoint& pos, const wxSize& size);
	virtual ~oscbrowserFrame();

	// event handlers (these functions should _not_ be virtual)
	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

    // OSC Browser
    void OnAddService(const char *name,const char *type,const char *domain);
    void OnRemoveService(const char *name,const char *type,const char *domain);

    // OSC Resolver
    void OnResolveService(const char *fullName,const char *hostTarget,int port,const char *txtRecord);

private:
	DECLARE_CLASS(oscbrowserFrame)
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};

#endif
