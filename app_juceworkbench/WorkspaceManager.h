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

#ifndef __WSMANAGER__
#define __WSMANAGER__

#include <piw/piw_tsd.h>
#include "juce.h"
#include "epython.h"
#include "WorkbenchFrame.h"
#include "ToolManager.h"
#include "MainComponent.h"
#include "Workspace.h"
#include "workbench.h"
#include "MenuManager.h"
#include "HelpViewComponent.h"
#include <iostream>
#include <map>
#include "ProgressLayer.h"
#include "WBTabbedComponent.h"

class BackendFactory
{
    public:
    virtual ~BackendFactory(){};
    virtual epython::PythonBackend *getBackend()=0;

};

class WorkspaceManager: public WorkspaceFactory,
                        public KeyListener
{

public:
    WorkspaceManager( BackendFactory* factory, WorkbenchFrame* frame, MenuManager* mm);
    ~WorkspaceManager(){};
    void  setRootWorkspaceComponent();
    void  addWorkspaceComponent(String name, String scope,String abs_id);
    void  showWorkspace(String name, String scope, String id);
    void nameChanged(String name, String abs_id);
    void remove(String abs_id);
    void removeCurrentTab();
    void removeTab(String name);
    virtual bool keyStateChanged(bool isKeyDown, Component* originatingComponent);
    virtual bool keyPressed(const KeyPress &key, Component* originatingComponent);

private:
    WBTabbedComponent* tc_;
    //TabbedComponent* tc_;
    ToolManager* tm_;
    MenuManager* mm_;
    HelpViewComponent* hv_;
    BackendFactory* factory_;
    int getIndexToShow(String name);
    std::map<String,String> scopes_;
    std::map<String,String> ids_;
    std::map<String,Workspace*> workspaces_;
    std::map<String,MainComponent*> maincomponents_;
};

#endif
