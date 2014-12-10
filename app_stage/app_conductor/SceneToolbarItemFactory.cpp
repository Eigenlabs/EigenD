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

#include "SceneToolbarItemFactory.h"

SceneToolbarButton::SceneToolbarButton(int id, const String& label)
                        :ToolbarButton(id,label,0,0)
{

}

void SceneToolbarButton::paintButtonArea(Graphics& g, int width, int height, bool isMouseOver, bool isMouseDown)
{
//    std::cout<<"SceneToolbarButton-paintButtonArea"<<std::endl;
//    ToolbarButton::paintButtonArea(g,width,height,isMouseOver,isMouseDown);
}

void SceneToolbarButton::paintButton(Graphics& g, bool isMouseOver, bool isMouseDown)
{
//    std::cout<<"SceneToolbarButton-paintButton"<<std::endl;

    if(getToggleState())
    {
        g.fillAll(Colours::lightgrey);
        g.setColour(Colour(0xff7cf8ed));
        g.fillRect(4,4,getWidth()-8,getHeight()-8);
        g.setColour(Colours::darkgrey);
        g.drawSingleLineText(getButtonText(),4,0.6*getHeight());


    }
    else
    {
        g.fillAll(Colours::lightgrey);
        g.setColour(Colours::white);
        g.fillRect(4,4,getWidth()-8,getHeight()-8);
        g.setColour(Colours::lightgrey);
        g.drawSingleLineText(getButtonText(),4,0.6*getHeight());
    }

}

SceneToolbarItemFactory::SceneToolbarItemFactory(ButtonListener* listener)
{
    listener_=listener;
    setupButtons();
}

void SceneToolbarItemFactory::getAllToolbarItemIds(Array <int>& ids)
{
    ids.add(playing);
    ids.add(last);
}

void SceneToolbarItemFactory:: getDefaultItemSet(Array<int>& ids)
{
    ids.add(playing);
    ids.add(last);
}

ToolbarItemComponent* SceneToolbarItemFactory::createItem(const int itemId)
{
    switch (itemId)
    {
        case playing:
            return playingButton_;
            break;
        case last:
            return lastButton_;
            break;
    }

    return 0;
}

void SceneToolbarItemFactory::setupButtons()
{
   Drawable* dr=0;
   Drawable* dr2=0;

   playingButton_=setupButton(PLAYING,"Playing",  dr,dr2);
   playingButton_->setToggleState(true,false);

   lastButton_=setupButton(LAST,"Last",  dr,dr2);

}

SceneToolbarButton* SceneToolbarItemFactory::setupButton(int tool, String label, Drawable* img1, Drawable* img2)
{
    SceneToolbarButton* tbb =new SceneToolbarButton(tool,label);
    tbb->setRadioGroupId(SCENE_BUTTON_GROUP);
    tbb->setClickingTogglesState(true);
    tbb->addListener(listener_);
    tbb->setTooltip(label);
    return tbb;
}


