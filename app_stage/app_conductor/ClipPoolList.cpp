#include "ClipPoolList.h"

ClipPoolList::ClipPoolList (ClipPoolBackend* backend)
    : ClipList("ClipPoolList"), backend_(backend)
{

    filterType_=1;
    setSize (600, 400);
    clipPoolChanged();
    setMultipleSelectionEnabled(true);
    backend_->addClipPoolListener(this);
}

ClipPoolList::~ClipPoolList()
{
}

void ClipPoolList::addToScene()
{
    std::cout<<"ClipPoolList-add to scene"<<std::endl;
    backend_->addToScene(getSelectedClips(),0);
}

void ClipPoolList::addToArrangement()
{
    std::cout<<"ClipPoolList-add to arrangement"<<std::endl;
    backend_->addToArrangement(getSelectedClips(),0);
}

void ClipPoolList::removeFromClipPool()
{
    std::cout<<"SceneList- removeFromClipPool"<<std::endl;
    backend_->removeFromClipPool(getSelectedClips());
}

void ClipPoolList::listBoxItemDoubleClicked(int row, const MouseEvent& e)
{
    std::cout<<"ClipPoolList - doubleClicked"<<std::endl;
    if((row>=0) && (row<(int)v_.size()))
    {
        Clip* c=v_[row];
        if(c!=0)
        {
            backend_->viewClip(v_[row]->toXml());
        }
    }
}


void ClipPoolList::clipPoolChanged()
{
    std::cout<<"ClipPoolList- Clip pool changed"<<std::endl;
    XmlElement* clipData=backend_->getClipPoolClips();
    refreshList(filterData(clipData));
}

XmlElement* ClipPoolList::filterData(XmlElement* clipData)
{
    XmlElement* f =new XmlElement("Clips");
    XmlElement* e=clipData->getFirstChildElement();
    while(e!=0)
    {
        if(e->hasTagName("CLIP"))
        {
            if(hasCategoryTag(e,getTagName(filterType_)))
            {
               f->addChildElement(new XmlElement(*e)); 
            }
        }
        e=e->getNextElement();
    }
    return f;
}

String ClipPoolList::getTagName(int filter)
{
    switch (filter)
    {
        case 1:
            return "Audio";
        case 2:
            return "Instrument";
        case 3:
            return "Talker";
        case 4:
            return "Scene";
    }
    return "Audio";
}

bool ClipPoolList::hasCategoryTag(XmlElement* e, String name)
{
    XmlElement* c=e->getChildByName("Tag");
    while (c!=0)
    {
        if(c->hasAttribute("name") && c->hasAttribute("category"))
        {
            if(c->getStringAttribute("category").equalsIgnoreCase("Type"))
            {
                if(c->getStringAttribute("name").equalsIgnoreCase(name))
                {
                    return true;
                }
            }
        }
        c=c->getNextElementWithTagName("Tag");
    }
    return false;
}

bool ClipPoolList::isInterestedInDragSource(const SourceDetails &dragSourceDetails)
{
    String source=dragSourceDetails.description.toString();
    if(source.equalsIgnoreCase("ClipManagerList")||source.equalsIgnoreCase("ClipPoolList"))
    {
        ClipList::onInterestedInDragSource(dragSourceDetails);
        return true;
    }

    return false;
}

void ClipPoolList::itemDropped(const SourceDetails &dragSourceDetails)
{

    String source=dragSourceDetails.description.toString();

    if(source.equalsIgnoreCase("ClipManagerList"))
    {
        std::cout<<"ClipPoolList - clip manager clip dropped"<<std::endl;
        Component* c=dragSourceDetails.sourceComponent.get();
        if(c!=0)
        {
            ClipList* cl=dynamic_cast<ClipList*>(c);
            if(cl!=0)
            {
                Point <int> p=dragSourceDetails.localPosition;
                backend_->addToClipPool(cl->getSelectedClips(),getInsertionIndexForPosition(p.getX(),p.getY()));
            }
        }
    }
    else if(source.equalsIgnoreCase("ClipPoolList"))
    {
        ClipList::onDropSelf(dragSourceDetails);
    }
}

void ClipPoolList::filterByType(int n)
{
    std::cout<<"ClipPoolPanel filterByType "<<n<<std::endl;
    filterType_=n;
    deselectAllRows();
// XXX might want to cache the full data to avoid getting it again
//  or do the filtering in the agent?
    XmlElement* clipData=backend_->getClipPoolClips();
    refreshList(filterData(clipData));
}





