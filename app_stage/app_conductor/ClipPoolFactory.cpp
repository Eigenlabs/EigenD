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

#include "ClipPoolFactory.h"
 
ClipPoolFactory::ClipPoolFactory(ButtonListener* listener)
{
    listener_=listener;
    setupButtons();
}

void ClipPoolFactory::getAllToolbarItemIds(Array <int>& ids)
{
    ids.add(audio_clip);
    ids.add(instrument_clip);
    ids.add(talker_clip);
    ids.add(scene_clip);
}

void ClipPoolFactory:: getDefaultItemSet(Array<int>& ids)
{
    ids.add(audio_clip);
    ids.add(instrument_clip);
    ids.add(talker_clip);
    ids.add(scene_clip);
}

ToolbarItemComponent* ClipPoolFactory::createItem(const int itemId)
{
    switch (itemId)
    {
        case audio_clip:
            return audioButton_;
            break;
        case instrument_clip:
            return instrumentButton_;
            break;
        case talker_clip:
            return talkerButton_;
            break;
        case scene_clip:
            return sceneButton_;
            break;
    }

    return 0;
}

void ClipPoolFactory::setupButtons()
{
   Drawable* dr=0;
   Drawable* dr2=0;

   dr=Drawable::createFromImageData(ImageResources::audio_button_off_png, ImageResources::audio_button_off_pngSize);
   dr2=Drawable::createFromImageData(ImageResources::audio_button_on_png, ImageResources::audio_button_on_pngSize);
   audioButton_=setupButton(AUDIO_CLIP,"Audio clips",  dr,dr2);
   audioButton_->setToggleState(true,false);

   dr=Drawable::createFromImageData(ImageResources::instrument_button_off_png, ImageResources::instrument_button_off_pngSize);
   dr2=Drawable::createFromImageData(ImageResources::instrument_button_on_png, ImageResources::instrument_button_on_pngSize);
   instrumentButton_=setupButton(INSTRUMENT_CLIP,"Instrument clips",  dr,dr2);

   dr=Drawable::createFromImageData(ImageResources::talker_button_off_png, ImageResources::talker_button_off_pngSize);
   dr2=Drawable::createFromImageData(ImageResources::talker_button_on_png, ImageResources::talker_button_on_pngSize);
   talkerButton_=setupButton(TALKER_CLIP,"Talker clips",  dr,dr2);

   dr=Drawable::createFromImageData(ImageResources::scene_button_off_png, ImageResources::scene_button_off_pngSize);
   dr2=Drawable::createFromImageData(ImageResources::scene_button_on_png, ImageResources::scene_button_on_pngSize);
   sceneButton_=setupButton(SCENE_CLIP,"Scene clips",  dr,dr2);

}

ToolbarButton* ClipPoolFactory::setupButton(int tool, String label, Drawable* img1, Drawable* img2)
{
    ToolbarButton* tbb =new ToolbarButton(tool,label,img1,img2);
    tbb->setRadioGroupId(CLIP_POOL_BUTTON_GROUP);
    tbb->setClickingTogglesState(true);
    tbb->addListener(listener_);
    tbb->setTooltip(label);
    return tbb;
}


