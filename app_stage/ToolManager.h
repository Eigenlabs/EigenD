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

#ifndef __TOOL_MANAGER__
#define __TOOL_MANAGER__

#include "MainComponent.h"
#include "CustomCursor.h"
#include "ToolbarIcons.h"

#include "juce.h"

/*
class TooltipToolbarButton: public ToolbarButton
{
public:
    TooltipToolbarButton(const int itemId, const String  &labelText, Drawable  *const normalImage, Drawable  *const toggledOnImage, const String &tooltip)
    : ToolbarButton(itemId, labelText, normalImage, toggledOnImage)
    {
        tooltip_ = tooltip;
    }

    virtual const String getTooltip()
    {
        return tooltip_;    
    }
    
private:
    String tooltip_;
    
};
*/
/*
class TooltipToolbarButton: public ToolbarButton, SettableTooltipClient
{
};
 */
 
class ToolManager: public ToolbarItemFactory,
                   public Button::Listener
{
public:

    ToolManager(MainComponent* mainComponent, Toolbar* toolbarComponent);
    ~ToolManager() {};

    void setTool(CommandID toolID);
    
    const static MouseCursor getMouseCursor(CommandID toolMode);

    void buttonClicked (Button* buttonThatWasClicked);

    virtual void getAllToolbarItemIds(Array <int>& ids);

    virtual void getDefaultItemSet(Array<int>& ids);
    
    virtual ToolbarItemComponent* createItem(const int itemId);

private:

    int currentTool_;

    MainComponent* mainComponent_;
    Toolbar* toolbarComponent_;
};



#endif  // __TOOL_MANAGER__


