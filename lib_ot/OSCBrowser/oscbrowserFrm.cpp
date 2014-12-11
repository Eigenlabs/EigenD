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

#include "wx/wx.h"
#include "wx/stattext.h"
#include "wx/tokenzr.h"

#include "oscbrowserApp.h"
#include "oscbrowserFrm.h"

#include <stdexcept>

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------
// the application icon
#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#include "oscbrowser.xpm"
#endif

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
	// menu items
	kOscbrowser_Quit = 1,
	kOscbrowser_About,
    kOscbrowser_Resolve,
    kOscbrowser_numitems
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWindows
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
IMPLEMENT_CLASS(oscbrowserFrame, wxFrame)
BEGIN_EVENT_TABLE(oscbrowserFrame, wxFrame)
EVT_MENU(kOscbrowser_Quit,  oscbrowserFrame::OnQuit)
EVT_MENU(kOscbrowser_About, oscbrowserFrame::OnAbout)
END_EVENT_TABLE()


// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// frame constructor
//-----------------------------------------------------------------------------------------
oscbrowserFrame::oscbrowserFrame(oscbrowserApp& application,const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame((wxFrame *)NULL, -1, title, pos, size/*,(wxSYSTEM_MENU |wxMINIMIZE_BOX|wxCLOSE_BOX|wxCAPTION|wxCLIP_CHILDREN)*/ )
{
	SetIcon(wxICON(oscbrowser));

    // menu -------------------------------------------
	wxMenu *fileMenu = new wxMenu("", wxMENU_TEAROFF);
	wxMenu *helpMenu = new wxMenu("", wxMENU_TEAROFF);

	fileMenu->Append(kOscbrowser_Quit, _("&Quit\tCtrl-Q"), _("Quit this program"));
	helpMenu->Append(kOscbrowser_About, _("&About...\tAlt-A"), _("Show about dialog"));

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, _("&File"));
	menuBar->Append(helpMenu, _("&Help"));
	SetMenuBar(menuBar);

    // status -------------------------------------------
	CreateStatusBar(2);
    SetStatusText(_("Oscbrowser :: Remy Muller - 2006"));

    wxBoxSizer* topsizer		= new wxBoxSizer(wxVERTICAL);
    wxPanel *panel = new wxPanel(	this,wxID_ANY,wxDefaultPosition,wxDefaultSize,
									wxTAB_TRAVERSAL|wxNO_BORDER);
	panel->SetSizer(topsizer);

    // services -------------------------------------------
    {
    wxStaticBoxSizer* servicesizer	= new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Services"));
    tree = new wxTreeCtrl(panel,wxID_ANY,wxDefaultPosition,wxSize(400,400),wxTR_HIDE_ROOT|wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT,wxDefaultValidator,wxT("OSCServices"));
    tree->AddRoot("");
    servicesizer->Add(tree,0, wxEXPAND|wxALIGN_CENTER, 5);
    topsizer->Add(servicesizer,3, wxEXPAND|wxALIGN_CENTER|wxALL, 2);
	}

	topsizer->SetSizeHints( this );   // set size hints to honour minimum size

    browser = new OscUdpZeroConfBrowser(this);
}

//-----------------------------------------------------------------------------------------
// frame destructor
//-----------------------------------------------------------------------------------------
oscbrowserFrame::~oscbrowserFrame()
{
    if(browser)
        delete browser;

    for(std::map<string,OscUdpZeroConfResolver*>::iterator i=resolvers.begin();
        i != resolvers.end(); ++i)
    {
        if(i->second)
            delete i->second;
    }
}


//-----------------------------------------------------------------------------------------
// event handlers
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
//if token exist return childitem else append token
wxTreeItemId AppendItem(wxTreeCtrl *tree, const wxTreeItemId& parent,const wxString& text)
{
    wxTreeItemId item;
    wxTreeItemIdValue cookie;
    item = tree->GetFirstChild(parent,cookie);

    while(item.IsOk())
    {
        wxString itemtext = tree->GetItemText(item);
        
        if(itemtext.Cmp(text) == 0)
        {
            return item;
        }
        item = tree->GetNextChild(parent,cookie);
    }

    //text not found: really append the item
    return tree->AppendItem(parent,text);
}
//-----------------------------------------------------------------------------------------
wxTreeItemId MyFindItem(wxTreeCtrl *tree, const wxTreeItemId& parent,const wxString& text)
{
    wxTreeItemId item;
    wxTreeItemIdValue cookie;
    item = tree->GetFirstChild(parent,cookie);

    while(item.IsOk())
    {
        wxString itemtext = tree->GetItemText(item);
        
        if(itemtext.Cmp(text) == 0)
        {
            return item;
        }
        item = tree->GetNextChild(parent,cookie);
    }

    item.Unset();
    return item;
}
//-----------------------------------------------------------------------------------------
void oscbrowserFrame::OnResolveService(const char *fullName,const char *hostTarget,int port,const char *txtRecord)
{
    wxStringTokenizer tokenizer(fullName,".",wxTOKEN_STRTOK);
    
    wxString service(wxT(""));
    wxString type(wxT(""));
    wxString domain(wxT(""));

    if(tokenizer.HasMoreTokens())
        service = tokenizer.GetNextToken();

    if(tokenizer.HasMoreTokens())
        type = tokenizer.GetNextToken();
    type += ".";
    if(tokenizer.HasMoreTokens())
        type += tokenizer.GetNextToken();
    type += ".";

    if(tokenizer.HasMoreTokens())
        domain = tokenizer.GetNextToken();
    domain += ".";
    
    wxTreeItemId item = tree->GetRootItem();   
    item = AppendItem(tree,item,domain);       
    tree->SetItemHasChildren(item);
    item = AppendItem(tree,item,type);         
    tree->SetItemHasChildren(item);
    item = AppendItem(tree,item,service);      
    tree->SetItemHasChildren(item);

    wxString ipstr(hostTarget);
    wxString portstr;
    portstr << port;

    item = AppendItem(tree,item,ipstr+wxT(":")+portstr);
}
//-----------------------------------------------------------------------------------------
void oscbrowserFrame::OnAddService(const char *name,const char *type,const char *domain)
{
    wxTreeItemId item = tree->GetRootItem();

    if(!item.IsOk())
    {
        item = tree->AddRoot("");
        tree->Expand(item);
    }

    item = AppendItem(tree,item,domain);
    tree->SetItemBold(item);
    tree->SetItemHasChildren(item);
    tree->Expand(item);

    item = AppendItem(tree,item,type);
    //tree->SetItemBold(item);
    tree->SetItemHasChildren(item);
    tree->Expand(item);

    item = AppendItem(tree,item,name);
    tree->SetItemBold(item);

    resolvers[name] = new OscUdpZeroConfResolver(name,type,domain,this);
}
//-----------------------------------------------------------------------------------------
void oscbrowserFrame::OnRemoveService(const char *name,const char *type,const char *domain)
{
    wxTreeItemId rootitem = tree->GetRootItem(); if(!rootitem.IsOk()) return;

    wxTreeItemId domainitem = MyFindItem(tree,rootitem,wxString(domain));    
    if(!domainitem.IsOk()) return;

    wxTreeItemId typeitem = MyFindItem(tree,domainitem,type); 
    if(!typeitem.IsOk()) return;
    
    wxTreeItemId nameitem = MyFindItem(tree,typeitem,name);      
    if(!nameitem.IsOk()) return;

    tree->Delete(nameitem);

    if(resolvers.count(name))
        if(resolvers[name])
            delete resolvers[name];
}

//-----------------------------------------------------------------------------------------
void oscbrowserFrame::OnQuit(wxCommandEvent& event)
{
	// TRUE is to force the frame to close
	Close(TRUE);
}

//-----------------------------------------------------------------------------------------
void oscbrowserFrame::OnAbout(wxCommandEvent& event)
{
	// called when help - about is picked from the menu or toolbar
	wxString msg;
	msg.Printf(wxT(
        "OSCBrowser (2006)                                               \n"
        "  by Remy Muller                                             \n"
        "  remy.muller@ircam.fr                                       \n"
        "                                                             \n"
        "this is an Open Sound Control testing tool                   \n"
        "http://www.opensoundcontrol.org/                             \n"
        "                                                             \n"
        "it uses the following libraries:                             \n"
        "wxWidgets                                                    \n"
        "  http://www.wxwindows.org/                                  \n"
        "  by Dr Robert Roebling, Dr Vadim Zeitlin, Dr Stefan Csomor, \n"
        "  Dr Julian Smart, Vaclav Slavik, Robin Dunn ...             \n"
        "                                                             \n"
        "oscpack                                                      \n"
        "  http://www.audiomulch.com/~rossb/code/oscpack/             \n"
        "  by Ross Bencina                                            \n"
        "                                                             \n"));
	wxMessageBox(msg, wxT("About oscbrowser"), wxOK | wxICON_INFORMATION, this);
}
