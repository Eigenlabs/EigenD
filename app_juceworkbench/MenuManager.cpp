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

#include "MenuManager.h"

MenuManager::MenuManager(ApplicationCommandManager* manager,WBTabbedComponent* tc)
//MenuManager::MenuManager(ApplicationCommandManager* manager,TabbedComponent* tc)
{
    manager_=manager;
    tc_=tc;
    showTalkers_=false;
    showMetronomes_=true;
    showControllers_=false;
    manager_->registerAllCommandsForTarget(this);
    manager_->setFirstCommandTarget(this);
}

MenuManager::~MenuManager()
{

}

ApplicationCommandTarget *MenuManager::getNextCommandTarget()
{
    return 0;
}

void MenuManager::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] = { 
        commandFileAbout,
        commandFilePreferences,
        commandEditFind,
        commandEditCloseTab,
    };

    commands.addArray (ids, numElementsInArray (ids));
}

void MenuManager::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    const String generalCategory ("General");

    switch (commandID)
    {
        case commandFileAbout:
            result.setInfo ("About Workbench", "Program Information", generalCategory, 0);
            break;
        case commandFilePreferences:
            result.setInfo("Preferences...","Set preferences", generalCategory,0);
            break;
        case commandEditFind:
            result.setInfo ("Find", "Locate an agent on the diagram", generalCategory, 0);
            result.addDefaultKeypress ('f', ModifierKeys::commandModifier);
            break;
        case commandEditCloseTab:
            result.setInfo ("Close Tab", "Close the current tab", generalCategory, 0);
    }
}

bool MenuManager::perform (const InvocationInfo& info)
{
    int msg;
    Component* c=0;
    Viewport* vp;
    switch (info.commandID)
    {
        case commandFileAbout:
            msg=1020;
            c=tc_->getCurrentContentComponent();
            vp=dynamic_cast<Viewport*>(c->getChildComponent(0));
            if(vp!=0)
            {
                vp->getTopLevelComponent()->postCommandMessage(msg);
            }
            break;

        case commandFilePreferences:
            msg=1021;
            c=tc_->getCurrentContentComponent();
            vp=dynamic_cast<Viewport*>(c->getChildComponent(0));
            if(vp!=0)
            {
                vp->getViewedComponent()->postCommandMessage(msg);
            }
            break;


        case commandEditFind:
            msg=1011;
            c=tc_->getCurrentContentComponent();
            vp=dynamic_cast<Viewport*>(c->getChildComponent(0));
            if(vp!=0)
            {
                vp->getViewedComponent()->postCommandMessage(msg);
            }
            break;

        case commandEditCloseTab:
        // XXX
        //std::cout<<"close tab"<<std::endl;
        msg=1012;
        c=tc_->getCurrentContentComponent();
        vp=dynamic_cast<Viewport*>(c->getChildComponent(0));
        if(vp!=0)
        {
            vp->getViewedComponent()->postCommandMessage(msg);
        }
        break;
    }

    return true;
}


void MenuManager::setShowMetronomes(bool show)
{
    showMetronomes_=show;
    menuItemsChanged();
}

void MenuManager::setShowControllers(bool show)
{
    showControllers_=show;
    menuItemsChanged();
}

StringArray MenuManager::getMenuBarNames()
{
    const wchar_t* const names[]={L"File",L"Edit",L"View",0};
    return StringArray((const wchar_t**) names);
}

PopupMenu MenuManager::getMenuForIndex(int topLevelMenuIndex,const String &menuName)
{
    PopupMenu menu;
    if(topLevelMenuIndex==0)
    {
        menu.addCommandItem(manager_,commandFileAbout);
        menu.addCommandItem(manager_,commandFilePreferences);
    }

    else if(topLevelMenuIndex==1)
    {
        menu.addCommandItem(manager_,commandEditFind);
        menu.addCommandItem(manager_,commandEditCloseTab);
    }
    else if(topLevelMenuIndex==2)
    {
        menu.addItem(2,String("Metronome Outputs"),true,showMetronomes_);
        menu.addItem(3,String("Controller Outputs"),true,showControllers_);
        PopupMenu subMenu;
        subMenu.addItem(112,"20 %");
        subMenu.addItem(118,"30 %");
        subMenu.addItem(124,"40 %");
        subMenu.addItem(130,"50 %");
        subMenu.addItem(136,"60 %");
        subMenu.addItem(142,"70 %");
        subMenu.addItem(148,"80 %");
        subMenu.addItem(154,"90 %");
        subMenu.addItem(160,"100 %");
        subMenu.addItem(166,"110 %");
        subMenu.addItem(172,"120 %");
        subMenu.addItem(178,"130 %");
        subMenu.addItem(184,"140 %");
        subMenu.addItem(190,"150 %");
        subMenu.addItem(196,"160 %");
        subMenu.addItem(200,"170 %");
        menu.addSubMenu("Set Zoom Level",subMenu);
    }
    return menu;
}

void MenuManager::menuItemSelected(int menuItemID,int topLevelMenuIndex)
{
    std::cout<<"menuItemSelected="<<menuItemID<<" topLevelMenuIndex="<<topLevelMenuIndex<<std::endl;
    int msg=0;

    if(menuItemID==2 && topLevelMenuIndex==2)
    {
        if(showMetronomes_)
        {
            msg=1003;
        }
        else
        {
            msg=1004;
        }
        Component* c=tc_->getCurrentContentComponent();
        Viewport* vp=dynamic_cast<Viewport*>(c->getChildComponent(0));
        if(vp!=0)
        {
            vp->getViewedComponent()->postCommandMessage(msg);
        } 
        showMetronomes_=!showMetronomes_;
    }
    else if(menuItemID==3 && topLevelMenuIndex==2)
    {
        if(showControllers_)
        {
            msg=1005;
        }
        else
        {
            msg=1006;
        }
        Component* c=tc_->getCurrentContentComponent();
        Viewport* vp=dynamic_cast<Viewport*>(c->getChildComponent(0));
        if(vp!=0)
        {
            vp->getViewedComponent()->postCommandMessage(msg);
        }
        showControllers_=!showControllers_;
    }
    else if (menuItemID>100 && menuItemID<=200 && topLevelMenuIndex==2)
    {
        std::cout<<"set zoom level of all tabs to "<<((float)(menuItemID-100)/100.0f)<<std::endl;
        for(int i=0;i<tc_->getNumTabs();i++)
        {
            Component* c=tc_->getTabContentComponent(i);
            Viewport* vp=dynamic_cast<Viewport*>(c->getChildComponent(0));
            if(vp!=0)
            {
                vp->getViewedComponent()->postCommandMessage(2000+menuItemID);
            }
        }
    }
}
