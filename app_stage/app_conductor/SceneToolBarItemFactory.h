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

#ifndef __SCENE_TOOLBAR_BUTTON__
#define __SCENE_TOOLBAR_BUTTON__

#include "juce.h"
#include "ImageResources.h"

class SceneToolbarButton: public ToolbarButton
{
public:
    SceneToolbarButton(int, const String&);
    ~SceneToolbarButton(){};
    virtual void paintButtonArea(Graphics& g, int width, int height, bool isMouseOver, bool isMouseDown);
    virtual void paintButton(Graphics& g, bool,bool);
};

class SceneToolbarItemFactory: public ToolbarItemFactory
{
public:
    SceneToolbarItemFactory(ButtonListener*);
    ~SceneToolbarItemFactory(){};
    virtual void getAllToolbarItemIds(Array <int>& ids);
    virtual void getDefaultItemSet(Array<int>& ids);
    virtual ToolbarItemComponent* createItem(const int itemId);
    static const int PLAYING=1;
    static const int LAST=2;

    static const int SCENE_BUTTON_GROUP=10;

    enum clipItemIds
    {
        playing=PLAYING,
        last=LAST
    };

    SceneToolbarButton* playingButton_;
    SceneToolbarButton* lastButton_;

private:
    void setupButtons();
    SceneToolbarButton* setupButton(int, String,Drawable*,Drawable*);
    ButtonListener* listener_;
};
#endif
