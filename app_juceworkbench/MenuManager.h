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

#ifndef __MENUMAN__
#define __MENUMAN__
#include <piw/piw_tsd.h>
#include "juce.h"
#include "WBTabbedComponent.h"

class WBTabbedComponent;

enum WorkbenchCommands
{
    commandFileAbout = 1000,
    commandFilePreferences,
    commandEditFind,
    commandEditCloseTab
};


class MenuManager: public MenuBarModel, public ApplicationCommandTarget
{
    public:
        MenuManager(ApplicationCommandManager* manager,WBTabbedComponent* tc);
        //MenuManager(ApplicationCommandManager* manager,TabbedComponent* tc);
        ~MenuManager();
        StringArray getMenuBarNames();
        PopupMenu getMenuForIndex(int topLevelMenuIndex,const String &menuName);
        void menuItemSelected(int menuItemID,int topLevelMenuIndex);

        void setShowMetronomes(bool show);
        void setShowControllers(bool show);

        void getAllCommands (Array <CommandID>& commands);
        void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result);
        ApplicationCommandTarget *getNextCommandTarget();
        bool perform (const InvocationInfo& info);
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MenuManager)
    private:

        bool showTalkers_;
        bool showMetronomes_;
        bool showControllers_;
        WBTabbedComponent* tc_;
        //TabbedComponent* tc_;
        ApplicationCommandManager* manager_;
};
#endif
