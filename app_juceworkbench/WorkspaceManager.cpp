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

#include "WorkspaceManager.h"

WorkspaceManager::WorkspaceManager(BackendFactory* factory, WorkbenchFrame* frame, MenuManager* mm)
{
   factory_=factory;
   tm_=frame->get_tm();
   tc_=frame->get_tabbedComponent();
   tc_->addRemoveTabListener(this);
   frame->addKeyListener(this);
   hv_=frame->get_helpView();
   mm_=mm;
}

bool WorkspaceManager::keyStateChanged(bool isKeyDown, Component* originatingComponent)
{
    Component* c=tc_->getCurrentContentComponent();
    Viewport* vp=dynamic_cast<Viewport*>(c->getChildComponent(0));
    if(vp!=0)
    {
        MainComponent* mc= dynamic_cast<MainComponent*>(vp->getViewedComponent());
        if(mc!=0)
        {
            return mc->keyStateChanged(isKeyDown);
        }
    }

    return false;
}

bool WorkspaceManager::keyPressed(const KeyPress &key, Component* originatingComponent)
{
    Component* c=tc_->getCurrentContentComponent();
    Viewport* vp=dynamic_cast<Viewport*>(c->getChildComponent(0));
    if(vp!=0)
    {
        MainComponent* mc= dynamic_cast<MainComponent*>(vp->getViewedComponent());
        if(mc!=0)
        {
            return mc->keyPressed(key);
        }
    }

    return false;
}

void WorkspaceManager::showWorkspace(String name, String scope, String abs_id)
{
    pic::logmsg()<<"WorkspaceManager:showWorkspace name= "<<std::string(name.toUTF8())<<" scope="<<std::string(scope.toUTF8())<<" abs_id="<<std::string(abs_id.toUTF8());
    String currentName=tc_->getCurrentTabName();
    std::map<String,String >::iterator i=scopes_.find(currentName);
    if(i!=scopes_.end())
    {
        scope =(i->second)+"."+scope; 
        name=(i->second)+":"+name;
    }
    int indexToShow=getIndexToShow(name);

    if(indexToShow>-1)
    {
        tc_->setCurrentTabIndex(indexToShow,false);
    }
    else
    {
        addWorkspaceComponent(name,scope,abs_id);

        pic::logmsg()<<"WorkspaceManager:workspace added name= "<<std::string(name.toUTF8())<<" scope="<<std::string(scope.toUTF8())<<" abs_id="<<std::string(abs_id.toUTF8());
        ids_.insert(std::pair<String,String>(abs_id,name));
        scopes_.insert(std::pair<String,String>(name,scope));
        tc_->setCurrentTabIndex(getIndexToShow(name),false);
    }
}

void WorkspaceManager::removeTab(String tabName)
{
    if(tabName!="Main workspace")
    {
        String abs_id=String::empty;

        for(std::map<String,String>::iterator i=ids_.begin();i!=ids_.end();i++)
        {
            if(i->second==tabName)
            {
                abs_id=i->first;
            }
        }
        if(abs_id.isNotEmpty())
        {
            int indexToRemove=getIndexToShow(tabName);
            if(indexToRemove>-1)
            {
                std::map<String,Workspace*>::iterator i=workspaces_.find(abs_id);
                if(i!=workspaces_.end())
                {
                    Workspace* w=(i->second);
                    w->quit();
                    workspaces_.erase(abs_id);
                    delete w;
                }
                tc_->removeTab(indexToRemove);
                ids_.erase(abs_id);
                scopes_.erase(tabName);
            }
        }
    }

    else
    {
        //std::cout<<"cant remove main workspace"<<std::endl;
    }

}

void WorkspaceManager::removeCurrentTab()
{
//    std::cout<<"Workspacemanager::remove current tab"<<std::endl;
    String tabName=tc_->getCurrentTabName();
    removeTab(tabName);
}


void WorkspaceManager::remove(String abs_id)
{
    String tabName=String::empty;
    std::map<String,String>::iterator i=ids_.find(abs_id);
    if(i!=ids_.end())
    {
        tabName=i->second;
    }
    if(tabName.isNotEmpty())
    {
           pic::logmsg()<<"WorkspaceManager:remove tabname "<<std::string(tabName.toUTF8());

        int indexToRemove=getIndexToShow(tabName);
        if(indexToRemove>-1)
        {
            std::map<String,Workspace*>::iterator i=workspaces_.find(abs_id);
            if(i!=workspaces_.end())
            {
                Workspace* w=i->second;
                w->quit();
                workspaces_.erase(abs_id);
                delete w;
            }

            tc_->removeTab(indexToRemove);
            ids_.erase(abs_id);
            scopes_.erase(tabName);
        }
    }
}

void WorkspaceManager::nameChanged(String newName, String abs_id)
{ 
    pic::logmsg()<<"WorkspaceManager::nameChanged to "<<newName<<" abs_id="<<abs_id;
    //find the tab name associated with the abs_id
    String oldName=String::empty;
    std::map<String,String>::iterator i=ids_.find(abs_id);
    if(i!=ids_.end())
    {
        oldName=i->second;
    }
    
    pic::logmsg()<<"WorkspaceManager::tabName was "<<oldName;
    if(oldName.isNotEmpty())
    {
        int indexToChange=getIndexToShow(oldName);
        String scope=String::empty;;
        std::map<String,String >::iterator iter=scopes_.find(oldName);
        if(iter!=scopes_.end())
        {
            scope =(iter->second); 
        }

        //change it in the map
        
        ids_.erase(abs_id);
        scopes_.erase(oldName);

        String currentName=tc_->getCurrentTabName();
        std::map<String,String >::iterator i=scopes_.find(currentName);
        if(i!=scopes_.end())
        {
            newName=(i->second)+":"+newName;
        }
        pic::logmsg()<<"WorkspaceManager::new tabName is "<<newName<<" indexToChange is"<<indexToChange;

        ids_.insert(std::pair<String,String>(abs_id,newName));
        scopes_.insert(std::pair<String,String>(newName,scope));

        tc_->setTabName(indexToChange,newName);
        tc_->getTabContentComponent(indexToChange)->setName(newName);  
    }
}

int WorkspaceManager::getIndexToShow(String name)
{
    int indexToShow=-1;
    for(int i=0;i<tc_->getNumTabs();i++)
    {
        pic::logmsg()<<"WorkspaceManager::tabNames "<<std::string(tc_->getTabContentComponent(i)->getName().toUTF8())<<" name to match "<<std::string(name.toUTF8());
        if(tc_->getTabContentComponent(i)->getName()==name)
        {
            indexToShow=i;
            break;
        }
    }
    return indexToShow;
}

void  WorkspaceManager::setRootWorkspaceComponent()
{
    addWorkspaceComponent("Main workspace","","main");
}

void  WorkspaceManager::addWorkspaceComponent(String name, String scope, String abs_id)
{
    Workspace* wsp=0;
    MainComponent* mc=0;
    wsp=new Workspace(factory_->getBackend());
    pic::logmsg()<<std::string(name.toUTF8())<<" workspace created"<<wsp;
    workspaces_.insert(std::pair<String,Workspace*>(abs_id,wsp));
    mc=new MainComponent(tm_,wsp,mm_,hv_,this);
    pic::logmsg()<<"MainComponent created "<<mc;
    maincomponents_.insert(std::pair<String,MainComponent*>(abs_id,mc));
    wsp->addListener(mc);

    WBViewport* viewport = new WBViewport(name);
    viewport->setScrollBarThickness (16);
    mc->setViewport(viewport);
    viewport->setViewedComponent(mc);
    viewport->setViewPosition(0.5*viewport->getWidth(),0.5*viewport->getHeight());
    viewport->getHorizontalScrollBar()->setAutoHide(false);
    viewport->getVerticalScrollBar()->setAutoHide(false);

    ProgressLayer* progressLayer = new ProgressLayer(name);
    progressLayer->addAndMakeVisible(viewport);
    tc_->addTab(name,Colour(0xffe7e7e7),progressLayer,true,-1);

    wsp->initialise_backend(scope);
    mc->doZoom(0.6);
}

