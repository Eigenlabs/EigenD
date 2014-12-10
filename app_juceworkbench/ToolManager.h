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

#ifndef __TOOLMANAGER__
#define __TOOLMANAGER__
#include <piw/piw_tsd.h>
#include "juce.h"
#include <iostream>
#include "CustomCursor.h"

class ToolManager: public ToolbarItemFactory,
                   public Button::Listener
{
    public:
    ToolManager(Component* listener);
    ~ToolManager(){};
    void setTool(int tool,Component* c);
    int getTool();
    PopupMenu getToolsMenu();
    const MouseCursor getMouseCursor(int tool);
    const MouseCursor getMouseCursor();
    virtual void buttonClicked (Button* buttonThatWasClicked);

    static const int POINTERTOOL=1;
    static const int MOVETOOL=2;
    static const int ZOOMTOOL=3;
    static const int CREATETOOL=8;
    static const int DELETETOOL=9;
    static const int WIRINGTOOL=5;
    static const int EDITTOOL=4;
    static const int HOOKTOOL=6;
    static const int TRUNKTOOL=7;
    static const int SELECTTOOL=11;
    static const int HELPTOOL=12;

    static const int TOOLBUTTON_GROUP=10;

    static const int TRUNK_RESIZE_HOR=20;
    static const int TRUNK_RESIZE_VERT=21;
    static const int TRUNK_EXISTING=22;
    static const int WIRING_LEFT=23;

    static const int GRAB=30;
    static const int ZOOM_DOUBLE_CLICK=40;

    enum ToolManagerItemIds
    {
        move_tool=MOVETOOL,
        zoom_tool=ZOOMTOOL,
        create_tool=CREATETOOL,
        delete_tool=DELETETOOL,
        wiring_tool=WIRINGTOOL,
        edit_tool=EDITTOOL,
        hook_tool=HOOKTOOL,
        trunk_tool=TRUNKTOOL,
        pointer_tool=POINTERTOOL, 
        select_tool=SELECTTOOL,
        help_tool=HELPTOOL
    };

    virtual void getAllToolbarItemIds(Array <int>& ids);
    virtual void getDefaultItemSet(Array<int>& ids);
    virtual ToolbarItemComponent* createItem(const int itemId);

    ToolbarButton* moveButton_;
    ToolbarButton* zoomButton_;
    ToolbarButton* resizeButton_;
    ToolbarButton* createButton_;
    ToolbarButton* deleteButton_;
    ToolbarButton* wiringButton_;
    ToolbarButton* editButton_;
    ToolbarButton* hookButton_;
    ToolbarButton* trunkButton_;
    ToolbarButton* pointerButton_;
    ToolbarButton* selectButton_;
    ToolbarButton* helpButton_;

    void setPropertyValue(String name, bool setting);
    bool getPropertyValue(String name,bool defaultSetting);

    private:
    void changeCurrentTool(int tool);

    Component* listener_;
    int currentTool_;
    void setupButtons();
    ToolbarButton* setupButton(int tool,String label, Drawable* dr1, Drawable* dr2);
    juce::PropertiesFile ignores_;
    int clicktime_;
    bool clicked_;
};

#endif
