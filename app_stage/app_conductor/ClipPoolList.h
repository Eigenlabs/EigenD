#ifndef CLIP_POOL_LIST
#define CLIP_POOL_LIST

#include "juce.h"
#include "Conductor.h"
#include "ClipList.h"

class ClipPoolList:public ClipList,
                    public DragAndDropTarget,
                    public ClipPoolListener
{

public:
    ClipPoolList (ClipPoolBackend* backend);
    ~ClipPoolList();
    void clipPoolChanged();
    void removeFromClipPool();
    void addToScene();
    void addToArrangement();
    virtual bool isInterestedInDragSource(const SourceDetails &dragSourceDetails);
    virtual void itemDropped(const SourceDetails &dragSourceDetails);
    virtual void listBoxItemDoubleClicked(int row, const MouseEvent& e);
    void filterByType(int);

private:
    ClipPoolBackend* backend_;
    String getTagName(int);
    int filterType_;
    bool hasCategoryTag(XmlElement*,String);
    XmlElement* filterData(XmlElement*);
};

#endif
