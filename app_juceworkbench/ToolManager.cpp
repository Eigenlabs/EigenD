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

#include "ToolManager.h"
#include "ejuce.h"
#include <picross/pic_resources.h>

File getGlobalDir()
{
    return ejuce::pathToFile(pic::global_library_dir()).getChildFile("Global");
}


ToolManager::ToolManager(Component* listener):ignores_(getGlobalDir().getChildFile("wb_ignores.xml"), juce::PropertiesFile::Options())

{
    listener_=listener;
    setupButtons();
    currentTool_=POINTERTOOL;
    clicktime_=0;
    clicked_=false;
}

void ToolManager::setTool(int tool, Component* c)
{
    if(tool==WIRING_LEFT)
    {
        ToolbarItemComponent* tbi=createItem(WIRINGTOOL);
        if(tbi!=0)
        {
            tbi->setToggleState(true,false);
        }
        else
        {
            pic::logmsg()<<"cannot create ToolbarItemComponent";
        }
    }
    else
    {
        ToolbarItemComponent* tbi=createItem(tool);
        if(tbi!=0)
        {
            tbi->setToggleState(true,false);
        }
        else
        {
            pic::logmsg()<<"cannot create ToolbarItemComponent";
        }
    }

    if (c!=0)
    {
        c->setMouseCursor(getMouseCursor(tool));
    }

    changeCurrentTool(tool);
}

const MouseCursor ToolManager::getMouseCursor()
{
    return getMouseCursor(currentTool_);
}

const MouseCursor ToolManager::getMouseCursor(int tool)
{
    Image  img;
    switch(tool)
    {
        case MOVETOOL:
            //img=ImageCache::getFromMemory(CustomCursor::movetool_png,CustomCursor::movetool_pngSize);
            img=ImageCache::getFromMemory(CustomCursor::movetool2_png,CustomCursor::movetool2_pngSize);
            return MouseCursor(img,8,8);
//            return MouseCursor::DraggingHandCursor;
            break;
        case CREATETOOL:
            img=ImageCache::getFromMemory(CustomCursor::create_cursor_png,CustomCursor::create_cursor_pngSize);
            return MouseCursor(img,6,6);
        case SELECTTOOL:
            // XXX
            return MouseCursor::NormalCursor;

        case ZOOMTOOL:
            img=ImageCache::getFromMemory(CustomCursor::zoom_cursor_png,CustomCursor::zoom_cursor_pngSize);
            return MouseCursor(img,6,6);

        case DELETETOOL:
            img=ImageCache::getFromMemory(CustomCursor::delete_cursor_png,CustomCursor::delete_cursor_pngSize);
            return MouseCursor(img,8,8);

        case WIRINGTOOL:
            img=ImageCache::getFromMemory(CustomCursor::wiring_right_cursor_png,CustomCursor::wiring_right_cursor_pngSize);
            return MouseCursor(img,4,5);

        case EDITTOOL:
            img=ImageCache::getFromMemory(CustomCursor::edit_cursor_png,CustomCursor::edit_cursor_pngSize);
            return MouseCursor(img,4,8);

        case HOOKTOOL:
            img=ImageCache::getFromMemory(CustomCursor::hook_cursor_png,CustomCursor::hook_cursor_pngSize);
            return MouseCursor(img,8,8);

        case TRUNKTOOL:
            img=ImageCache::getFromMemory(CustomCursor::conduit_cursor_png,CustomCursor::conduit_cursor_pngSize);
            return MouseCursor(img,8,8);

        case POINTERTOOL:
            return MouseCursor::NormalCursor;

        case GRAB:
            img=ImageCache::getFromMemory(CustomCursor::grabtool2_png,CustomCursor::grabtool2_pngSize);
            return MouseCursor(img,8,8);

        case TRUNK_RESIZE_HOR:
            img=ImageCache::getFromMemory(CustomCursor::conduit_size_hor_cursor_png,CustomCursor::conduit_size_hor_cursor_pngSize);
            return MouseCursor(img,8,8);

        case TRUNK_RESIZE_VERT:
            img=ImageCache::getFromMemory(CustomCursor::conduit_size_vert_cursor_png,CustomCursor::conduit_size_vert_cursor_pngSize);
            return MouseCursor(img,8,8);

        case TRUNK_EXISTING:
            img=ImageCache::getFromMemory(CustomCursor::conduit_existing_cursor_png,CustomCursor::conduit_existing_cursor_pngSize);
            return MouseCursor(img,8,8);
        case WIRING_LEFT:
            img=ImageCache::getFromMemory(CustomCursor::wiring_left_cursor_png,CustomCursor::wiring_left_cursor_pngSize);
            return MouseCursor(img,14,5);
        case HELPTOOL:
            img=ImageCache::getFromMemory(CustomCursor::help_cursor_png,CustomCursor::help_cursor_pngSize);
            return MouseCursor(img,7,9);

        default:
            return MouseCursor::NormalCursor;
    }

    return MouseCursor::NormalCursor;
}

void ToolManager::setPropertyValue(String name, bool setting )
{
    ignores_.setValue(juce::String("ignores.")+name,setting);
}

bool ToolManager::getPropertyValue(String name, bool defaultSetting)
{
    return ignores_.getBoolValue(juce::String("ignores."+name),defaultSetting);
}

int ToolManager::getTool()
{
    return currentTool_;
}

PopupMenu ToolManager::getToolsMenu()
{
    PopupMenu menu;
    Image img;

    if(currentTool_==POINTERTOOL)
        img=ImageCache::getFromMemory(CustomCursor::pointer_on_png,CustomCursor::pointer_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::pointer_off_png,CustomCursor::pointer_off_pngSize);
    menu.addItem(POINTERTOOL,"Pointer",true,false,img);


    if(currentTool_==MOVETOOL)
        img=ImageCache::getFromMemory(CustomCursor::move_on_png,CustomCursor::move_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::move_off_png,CustomCursor::move_off_pngSize);
    menu.addItem(MOVETOOL,"Move",true,false,img);

    if(currentTool_==SELECTTOOL)
        img=ImageCache::getFromMemory(CustomCursor::select_on_png,CustomCursor::select_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::select_off_png,CustomCursor::select_off_pngSize);
    menu.addItem(SELECTTOOL,"Select",true,false,img);

    if(currentTool_==ZOOMTOOL)
        img=ImageCache::getFromMemory(CustomCursor::zoom_on_png,CustomCursor::zoom_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::zoom_off_png,CustomCursor::zoom_off_pngSize);
    menu.addItem(ZOOMTOOL,"Zoom",true,false,img);

    if(currentTool_==EDITTOOL)
        img=ImageCache::getFromMemory(CustomCursor::edit_on_png,CustomCursor::edit_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::edit_off_png,CustomCursor::edit_off_pngSize);
    menu.addItem(EDITTOOL,"Edit",true,false,img);

    if((currentTool_==WIRINGTOOL)||(currentTool_==WIRING_LEFT))
        img=ImageCache::getFromMemory(CustomCursor::connect_on_png,CustomCursor::connect_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::connect_off_png,CustomCursor::connect_off_pngSize);
    menu.addItem(WIRINGTOOL,"Wiring",true,false,img);

    if(currentTool_==HOOKTOOL)
        img=ImageCache::getFromMemory(CustomCursor::hock_on_png,CustomCursor::hock_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::hock_off_png,CustomCursor::hock_off_pngSize);
    menu.addItem(HOOKTOOL,"Hook",true,false,img);

    if(currentTool_==TRUNKTOOL)
        img=ImageCache::getFromMemory(CustomCursor::conduit_on_png,CustomCursor::conduit_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::conduit_off_png,CustomCursor::conduit_off_pngSize);
    menu.addItem(TRUNKTOOL,"Trunk",true,false,img);

    if(currentTool_==CREATETOOL)
        img=ImageCache::getFromMemory(CustomCursor::create_on_png,CustomCursor::create_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::create_off_png,CustomCursor::create_off_pngSize);
    menu.addItem(CREATETOOL,"Create",true,false,img);

    if(currentTool_==DELETETOOL)
        img=ImageCache::getFromMemory(CustomCursor::delete_on_png,CustomCursor::delete_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::delete_off_png,CustomCursor::delete_off_pngSize);
    menu.addItem(DELETETOOL,"Delete",true,false,img);

    if(currentTool_==HELPTOOL)
        img=ImageCache::getFromMemory(CustomCursor::help_on_png,CustomCursor::help_on_pngSize);
    else
        img=ImageCache::getFromMemory(CustomCursor::help_off_png,CustomCursor::help_off_pngSize);
    menu.addItem(HELPTOOL,"Help",true,false,img);


    return menu;
}

void ToolManager::getAllToolbarItemIds(Array <int>& ids)
{
    ids.add(pointer_tool);
    ids.add(move_tool);
    ids.add(select_tool);
    ids.add(zoom_tool);
    ids.add(edit_tool);
    ids.add(wiring_tool);
    ids.add(hook_tool);
    ids.add(trunk_tool);
    ids.add(create_tool);
    ids.add(delete_tool);
    ids.add(help_tool);
}

void ToolManager:: getDefaultItemSet(Array<int>& ids)
{
    ids.add(pointer_tool);
    ids.add(move_tool);
    ids.add(select_tool);
    ids.add(zoom_tool);
    ids.add(edit_tool);
    ids.add(wiring_tool);
    ids.add(hook_tool);
    ids.add(trunk_tool);
    ids.add(create_tool);
    ids.add(delete_tool);
    ids.add(help_tool);
}

ToolbarItemComponent* ToolManager::createItem(const int itemId)
{
    switch (itemId)
    {
        case move_tool:
            return moveButton_;
            break;
        case select_tool:
            return selectButton_;
            break;
        case zoom_tool:
            return zoomButton_;
            break;
        case create_tool:
            return createButton_;
            break;
        case delete_tool:
            return deleteButton_;
            break;
        case wiring_tool:
            return wiringButton_;
            break;
        case edit_tool:
            return editButton_;
            break;
        case hook_tool:
            return hookButton_;
            break;
        case trunk_tool:
            return trunkButton_;
            break;
        case pointer_tool:
            return pointerButton_;
            break;
        case help_tool:
            return helpButton_;
            break;
    }

    return 0;
}

void ToolManager::buttonClicked(Button* buttonThatWasClicked)
{
    ToolbarItemComponent * tbi=dynamic_cast<ToolbarItemComponent*>(buttonThatWasClicked);
    int tool=tbi->getItemId();
    if(tool==ZOOMTOOL )
    {
        if(clicked_)
        {
            int t2=Time::currentTimeMillis();
            std::cout<<t2-clicktime_<<std::endl;
            if((t2-clicktime_)<500)
            {
                std::cout<<"Zoom Toolbar button double click "<<t2-clicktime_<<std::endl;
                listener_->postCommandMessage(ZOOM_DOUBLE_CLICK);
                clicked_=false;
            }
            else
            {
                clicktime_=t2;
            }
        }
        else
        {
            clicktime_=Time::currentTimeMillis();
            clicked_=true;
        }
    }
    else
    {
        clicked_=false;
    }

    changeCurrentTool(tool);
}

void ToolManager::changeCurrentTool(int tool)
{

    pic::logmsg()<<"tool set to "<<tool;
    if(tool!=currentTool_)
    {
        listener_->postCommandMessage(tool);
    }

    currentTool_=tool;
}

void ToolManager::setupButtons()
{
   Drawable* dr=0;
   Drawable* dr2=0;
#ifdef WIN32
   String commandKey="ctrl";
#else
   String commandKey="Cmd";
#endif
   dr=Drawable::createFromImageData(CustomCursor::pointer_off_png, CustomCursor::pointer_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::pointer_on_png, CustomCursor::pointer_on_pngSize);
   pointerButton_=setupButton(POINTERTOOL,"Pointer. ("+commandKey +" R)",  dr,dr2);
   pointerButton_->setToggleState(true,false);

   dr=Drawable::createFromImageData(CustomCursor::move_off_png, CustomCursor::move_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::move_on_png, CustomCursor::move_on_pngSize);
   moveButton_=setupButton(MOVETOOL,"Move tool: move agents, trunks and hooks",  dr,dr2);

   dr=Drawable::createFromImageData(CustomCursor::select_off_png, CustomCursor::select_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::select_on_png, CustomCursor::select_on_pngSize);
   selectButton_=setupButton(SELECTTOOL,"Select tool: drag to select agents, trunks and hooks within a rectangle",  dr,dr2);

   dr=Drawable::createFromImageData(CustomCursor::zoom_off_png, CustomCursor::zoom_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::zoom_on_png, CustomCursor::zoom_on_pngSize);
   zoomButton_=setupButton(ZOOMTOOL,"Zoom tool: click or drag right to zoom in. Shift-click or drag left to zoom out",  dr,dr2);

   dr=Drawable::createFromImageData(CustomCursor::edit_off_png, CustomCursor::edit_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::edit_on_png, CustomCursor::edit_on_pngSize);
   editButton_=setupButton(EDITTOOL,"Edit tool: click on an agent or port to display properties to edit. (" +commandKey +" E)",  dr,dr2);

   dr=Drawable::createFromImageData(CustomCursor::connect_off_png, CustomCursor::connect_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::connect_on_png, CustomCursor::connect_on_pngSize);
   wiringButton_=setupButton(WIRINGTOOL,"Wiring tool: Drag from an agent box to create a new wire. " + commandKey + "-drag to disconnect a wire. (" + commandKey + " W)",  dr,dr2);

   dr=Drawable::createFromImageData(CustomCursor::hock_off_png, CustomCursor::hock_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::hock_on_png, CustomCursor::hock_on_pngSize);
   hookButton_=setupButton(HOOKTOOL,"Hook tool: click anywhere on the canvas to create a wire hook at that position",  dr,dr2);

   dr=Drawable::createFromImageData(CustomCursor::conduit_off_png, CustomCursor::conduit_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::conduit_on_png, CustomCursor::conduit_on_pngSize);
   trunkButton_=setupButton(TRUNKTOOL,"Trunk tool: drag on the canvas to create a new trunk, or on a trunk to resize it",  dr,dr2);

   dr=Drawable::createFromImageData(CustomCursor::create_off_png, CustomCursor::create_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::create_on_png, CustomCursor::create_on_pngSize);
   createButton_=setupButton(CREATETOOL,"Create tool: click anywhere on the canvas to create a new agent at that position. ("+ commandKey +" A)",  dr,dr2);

   dr=Drawable::createFromImageData(CustomCursor::delete_off_png, CustomCursor::delete_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::delete_on_png, CustomCursor::delete_on_pngSize);
   deleteButton_=setupButton(DELETETOOL,"Delete tool: click on an agent, wire, trunk or hook to delete it. " + commandKey +"-click on a wire to remove it from hooks and trunks.  (" +commandKey+ " D)",  dr,dr2);

   dr=Drawable::createFromImageData(CustomCursor::help_off_png, CustomCursor::help_off_pngSize);
   dr2=Drawable::createFromImageData(CustomCursor::help_on_png, CustomCursor::help_on_pngSize);
   helpButton_=setupButton(HELPTOOL,"Help tool: displays help for agents and ports",  dr,dr2);
}

ToolbarButton* ToolManager::setupButton(int tool, String label, Drawable* img1, Drawable* img2)
{
    ToolbarButton* tbb =new ToolbarButton(tool,label,img1,img2);
    tbb->setRadioGroupId(TOOLBUTTON_GROUP);
    tbb->setClickingTogglesState(true);
    tbb->addListener(this);
    tbb->setTooltip(label);
    return tbb;
}



