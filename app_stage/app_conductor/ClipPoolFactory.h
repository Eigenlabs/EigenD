#ifndef CLIP_POOL_FACTORY
#define CLIP_POOL_FACTORY

#include "juce.h"
#include "ImageResources.h"

class ClipPoolFactory: public ToolbarItemFactory
{
public:
    ClipPoolFactory(ButtonListener*);
    ~ClipPoolFactory(){};
    virtual void getAllToolbarItemIds(Array <int>& ids);
    virtual void getDefaultItemSet(Array<int>& ids);
    virtual ToolbarItemComponent* createItem(const int itemId);
    static const int AUDIO_CLIP=1;
    static const int INSTRUMENT_CLIP=2;
    static const int TALKER_CLIP=3;
    static const int SCENE_CLIP=4;

    static const int CLIP_POOL_BUTTON_GROUP=10;

    enum clipItemIds
    {
        audio_clip=AUDIO_CLIP,
        instrument_clip=INSTRUMENT_CLIP,
        talker_clip=TALKER_CLIP,
        scene_clip=SCENE_CLIP
    };

    ToolbarButton* audioButton_;
    ToolbarButton* instrumentButton_;
    ToolbarButton* talkerButton_;
    ToolbarButton* sceneButton_;

private:
    void setupButtons();
    ToolbarButton* setupButton(int, String,Drawable*,Drawable*);
    ButtonListener* listener_;
};

#endif
