/*
 Copyright 2010-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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
#include <iostream>

using namespace std;

ToolManager::ToolManager(MainComponent* mainComponent, Toolbar* toolbarComponent) :
    mainComponent_(mainComponent), toolbarComponent_(toolbarComponent)
{
}

const MouseCursor ToolManager::getMouseCursor(CommandID toolMode)
{
    SystemStats::OperatingSystemType os = SystemStats::getOperatingSystemType();
    bool hidpi_cursor = (Desktop::getInstance().getDisplays().getMainDisplay().scale == 2.f && os != SystemStats::MacOSX_10_7);
    
    Image img;
    switch(toolMode)
    {
        case MainComponent::toolMove:
            return MouseCursor::DraggingHandCursor;
        
        case MainComponent::toolSize:
            return MouseCursor::UpDownLeftRightResizeCursor;
        
        case MainComponent::toolCreate:
            if (hidpi_cursor)
            {
                img=ImageCache::getFromMemory(CustomCursor::create_2x_png,CustomCursor::create_2x_pngSize);
                return MouseCursor(img,8,8,2.f);
            }
            else
            {
                img=ImageCache::getFromMemory(CustomCursor::create_png,CustomCursor::create_pngSize);
                return MouseCursor(img,8,8);
            }
        
        case MainComponent::toolDelete:
            if (hidpi_cursor)
            {
                img=ImageCache::getFromMemory(CustomCursor::delete_2x_png,CustomCursor::delete_2x_pngSize);
                return MouseCursor(img,8,8,2.f);
            }
            else
            {
                img=ImageCache::getFromMemory(CustomCursor::delete_png,CustomCursor::delete_pngSize);
                return MouseCursor(img,8,8);
            }
        
        case MainComponent::toolEdit:
            if (hidpi_cursor)
            {
                img=ImageCache::getFromMemory(CustomCursor::edit_2x_png,CustomCursor::edit_2x_pngSize);
                return MouseCursor(img,8,8,2.f);
            }
            else
            {
                img=ImageCache::getFromMemory(CustomCursor::edit_png,CustomCursor::edit_pngSize);
                return MouseCursor(img,8,8);
            }

        case MainComponent::toolHelp:
            if (hidpi_cursor)
            {
                img=ImageCache::getFromMemory(CustomCursor::help_2x_png,CustomCursor::help_2x_pngSize);
                return MouseCursor(img,8,8,2.f);
            }
            else
            {
                img=ImageCache::getFromMemory(CustomCursor::help_png,CustomCursor::help_pngSize);
                return MouseCursor(img,8,8);
            }
            
        default:
            return MouseCursor::NormalCursor;
    }

}



//==============================================================================

void ToolManager::getAllToolbarItemIds(Array <int>& ids)
{
    ids.add(MainComponent::toolPerform);
    ids.add(MainComponent::toolCreate);
    ids.add(MainComponent::toolEdit);
    //ids.add(MainComponent::toolMove);
    ids.add(MainComponent::toolSize);
    ids.add(MainComponent::toolDelete);
    ids.add(MainComponent::toolHelp);
}

void ToolManager:: getDefaultItemSet(Array<int>& ids)
{
    ids.add(MainComponent::toolPerform);
    ids.add(MainComponent::toolCreate);
    ids.add(MainComponent::toolEdit);
    //ids.add(MainComponent::toolMove);
    ids.add(MainComponent::toolSize);
    ids.add(MainComponent::toolDelete);
    ids.add(MainComponent::toolHelp);
}

ToolbarItemComponent* ToolManager::createItem(const int itemId)
{
    String label;
    String tooltip;
    Drawable* upImage=0;
    Drawable *downImage=0;
    switch (itemId)
    {
        case MainComponent::toolPerform:
            upImage=Drawable::createFromImageData(ToolbarIcons::usewidget_off_png, ToolbarIcons::usewidget_off_pngSize);
            downImage=Drawable::createFromImageData(ToolbarIcons::usewidget_on_png, ToolbarIcons::usewidget_on_pngSize);
            label="perform";
            tooltip="Perform";
            break;
        case MainComponent::toolCreate:
            upImage=Drawable::createFromImageData(ToolbarIcons::create_off_png, ToolbarIcons::create_off_pngSize);
            downImage=Drawable::createFromImageData(ToolbarIcons::create_on_png, ToolbarIcons::create_on_pngSize);
            label="create";
            tooltip="Create control";
            break;
        case MainComponent::toolEdit:
            upImage=Drawable::createFromImageData(ToolbarIcons::edit_off_png, ToolbarIcons::edit_off_pngSize);
            downImage=Drawable::createFromImageData(ToolbarIcons::edit_on_png, ToolbarIcons::edit_on_pngSize);
            label="edit";
            tooltip="Edit";
            break;
        case MainComponent::toolMove:
            upImage=Drawable::createFromImageData(ToolbarIcons::move_off_png, ToolbarIcons::move_off_pngSize);
            downImage=Drawable::createFromImageData(ToolbarIcons::move_on_png, ToolbarIcons::move_on_pngSize);
            label="move";
            tooltip="Move";
            break;
        case MainComponent::toolSize:
            upImage=Drawable::createFromImageData(ToolbarIcons::resize_off_png, ToolbarIcons::resize_off_pngSize);
            downImage=Drawable::createFromImageData(ToolbarIcons::resize_on_png, ToolbarIcons::resize_on_pngSize);
            label="resize";
            tooltip="Size and position";
            break;
        case MainComponent::toolDelete:
            upImage=Drawable::createFromImageData(ToolbarIcons::delete_off_png, ToolbarIcons::delete_off_pngSize);
            downImage=Drawable::createFromImageData(ToolbarIcons::delete_on_png, ToolbarIcons::delete_on_pngSize);
            label="delete";
            tooltip="Delete";
            break;
        case MainComponent::toolHelp:
            upImage=Drawable::createFromImageData(ToolbarIcons::help_off_png, ToolbarIcons::help_off_pngSize);
            downImage=Drawable::createFromImageData(ToolbarIcons::help_on_png, ToolbarIcons::help_on_pngSize);
            label="help";
            tooltip="Help";
            break;
        default:
            upImage=Drawable::createFromImageData(ToolbarIcons::usewidget_off_png, ToolbarIcons::usewidget_off_pngSize);
            downImage=Drawable::createFromImageData(ToolbarIcons::usewidget_on_png, ToolbarIcons::usewidget_on_pngSize);
            label="pointer";
            tooltip="Pointer tool";
            break;
    }
    
   
    ToolbarButton* tbb=new ToolbarButton(itemId,label,upImage,downImage);
    tbb->addListener(this);
    tbb->setTooltip(tooltip);
    
    return tbb;

}


void ToolManager::buttonClicked(Button* buttonThatWasClicked)
{
#if STAGE_BUILD==DESKTOP
    ToolbarItemComponent * itemClicked=dynamic_cast<ToolbarItemComponent*>(buttonThatWasClicked);

    mainComponent_->invokeDirectly(itemClicked->getItemId(), false);
#endif // STAGE_BUILD==DESKTOP
}


void ToolManager::setTool(CommandID toolID)
{
    
    // update toolbar buttons to behave as a radio group

    for(int i=0; i<toolbarComponent_->getNumItems(); i++)
    {
        ToolbarItemComponent* itemIter=dynamic_cast<ToolbarItemComponent*>(toolbarComponent_->getItemComponent(i));
        if(itemIter->getItemId()==toolID)
            itemIter->setToggleState(true, dontSendNotification);
        else
            itemIter->setToggleState(false, dontSendNotification);
    }
                
}


