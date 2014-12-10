#include "Conductor.h"

Conductor::Conductor()
{
}

XmlElement* Conductor::getAllTags()
{
    XmlDocument doc(File("~/conductor_model/all_tags.xml"));
    return doc.getDocumentElement();
}

XmlElement* Conductor::getColumnCategories()
{
    //XXX read from file and save new ones if defined
    XmlElement* columnCategories=new XmlElement("CATEGORIES");
    
    XmlElement* c1=new XmlElement(String("CATEGORY"));
    c1->setAttribute("Name","Type");
    columnCategories->addChildElement(c1);

    XmlElement* c2=new XmlElement(String("CATEGORY"));
    c2->setAttribute("Name","Style");
    columnCategories->addChildElement(c2);

    XmlElement* c3=new XmlElement(String("CATEGORY"));
    c3->setAttribute("Name","Instrument");
    columnCategories->addChildElement(c3);

    XmlElement* c4=new XmlElement(String("CATEGORY"));
    c4->setAttribute("Name","Other");
    columnCategories->addChildElement(c4);

    XmlElement* c5=new XmlElement(String("CATEGORY"));
    c5->setAttribute("Name","Another");
    columnCategories->addChildElement(c5);

    return columnCategories;
}

XmlElement* Conductor::getClipPoolClips()
{
    //XXX initially just read from file to get it working
    XmlDocument doc(File("~/conductor_model/clip_pool.xml"));
    XmlElement* e=(doc.getDocumentElement());
    return e;
}

XmlElement* Conductor::getScene(int mode)
{
    //XXX initially just read from file to get it working
     if(mode==0)
    {
        XmlDocument doc(File("~/conductor_model/currently_playing_scene.xml"));
        XmlElement* e=(doc.getDocumentElement());
        return e;

    }
    else if(mode==1)
    {
        XmlDocument doc(File("~/conductor_model/last_viewed_scene.xml"));
        XmlElement* e=(doc.getDocumentElement());
        return e;
    }
    return 0;
}

XmlElement* Conductor::getArrangement(int mode)
{
    //XXX initially just read from file to get it working
     if(mode==0)
    {
        XmlDocument doc(File("~/conductor_model/currently_playing_arrangement.xml"));
        XmlElement* e=(doc.getDocumentElement());
        std::cout<<"Conductor - getArrangement mode=0"<<e->createDocument("");
        return e;

    }
    else if(mode==1)
    {
        XmlDocument doc(File("~/conductor_model/last_viewed_arrangement.xml"));
        XmlElement* e=(doc.getDocumentElement());
        std::cout<<"Conductor - getArrangement mode=1"<<e->createDocument("");
        return e;
    }
    return 0;

}

XmlElement* Conductor::getSet()
{
//XXX initially just read from file to get it working
    XmlDocument doc(File("~/conductor_model/set.xml"));
    XmlElement* e=(doc.getDocumentElement());
    std::cout<<"Conductor - getSet"<<e->createDocument("");
    return e;
}

XmlElement* Conductor::getSelectedClips(XmlElement* arg)
{
    XmlElement* selectedClips=new XmlElement("CLIPS");
    XmlDocument doc(File("~/conductor_model/clips.xml"));
    XmlElement* e=(doc.getDocumentElement())->getFirstChildElement();
    while (e!=0)
    {
        if(e->hasTagName("CLIP"))
        {
                std::set<String> clipTags;
                XmlElement* t=e->getChildByName("TAG");
                while(t!=0)
                {
                    clipTags.insert(t->getStringAttribute("name"));
                    t=t->getNextElement();
                }
            bool missing=false;
            for(std::set<String>::iterator i=selectedTags_.begin();i!=selectedTags_.end();i++)
            {
                std::set<String>::iterator j=clipTags.find((*i));
                if(j==clipTags.end())
                {
                    missing=true;
                }
            }

            if (!missing)
            {
                XmlElement* c = new XmlElement(*e);
                selectedClips->addChildElement(c);
            }
        }

        e=e->getNextElement();
    }
    return selectedClips;
}

void Conductor::viewClip(XmlElement* e)
{
    // XXX
    std::cout<<"Conductor - viewClip "<<e->createDocument("");
// XXX 
// check which type of clip this is 
// get the appropriate widget to display it
    if(e->getTagName().equalsIgnoreCase("Scene"))
    {
        std::cout<<"change SceneView"<<std::endl;
        // set the last viewed scene to this scene
        // if the scene viewer is in last-viewed mode then show it
        sceneBackendChanged(1);
    }
    else if(e->getTagName().equalsIgnoreCase("Arrangement"))
    {
        std::cout<<"change ArrangementView"<<std::endl;
        // set the last viewed arrangement to this scene
        // if the arrangement viewer is in last-viewed mode then show it
        arrangementBackendChanged(1);
    }
    else
    {
        XmlElement* c=e->getChildByName("Tag");
        while (c!=0)
        {
            if(c->hasAttribute("name") && c->hasAttribute("category"))
            {
                if(c->getStringAttribute("category").equalsIgnoreCase("Type"))
                {
                    if(c->getStringAttribute("name").equalsIgnoreCase("Arrangement"))
                    {
                    // arrangementBackendChanged
                        std::cout<<"change ArrangementView"<<std::endl;
                        break;
                    }
                    else if(c->getStringAttribute("name").equalsIgnoreCase("Scene"))

                    {
                        std::cout<<"change SceneView"<<std::endl;
                        // set the last viewed scene to this scene
                        // if the scene viewer is in last-viewed mode then show it
                        sceneBackendChanged(1);
                        break;
                    }
                }
            }
            c=c->getNextElementWithTagName("Tag");
        }
    }

    delete e;

}

void Conductor::addToClipPool(XmlElement* e, int index)
{
    // XXX
    std::cout<<"Conductor addToClipPool at index "<<index<<e->createDocument("")<<std::endl;
    delete e;
    clipPoolBackendChanged();
}

void Conductor::removeFromClipPool(XmlElement* e)
{
    // XXX
    std::cout<<"Conductor removecFromClipPool "<<e->createDocument("");
    delete e;

    clipPoolBackendChanged();
}

void Conductor::removeFromScene(XmlElement* e, int mode)
{
    // XXX
    std::cout<<"Conductor removecFromScene "<<e->createDocument("");
    delete e;
    sceneBackendChanged(mode);
}

void Conductor::removeFromArrangement(XmlElement* e, int mode)
{
    // XXX
    std::cout<<"Conductor removecFromArrangement "<<e->createDocument("");
    delete e;
    arrangementBackendChanged(mode);
}

void Conductor::removeFromSet(XmlElement* e)
{
    // XXX
    std::cout<<"Conductor removecFromSet "<<e->createDocument("");
    delete e;
    setBackendChanged();
}

void Conductor::play(XmlElement* e)
{
    // XXX
    std::cout<<"Conductor play "<<e->createDocument("");
    delete e;
    // change currently playing
}

void Conductor::addToScene(XmlElement* e,int index)
{
    // XXX
    // add to currently playing scene or create one if none
    std::cout<<"Conductor addToScene at index "<<index<<e->createDocument("");
    delete e;
    int playmode=0;
    sceneBackendChanged(playmode);
}

void Conductor::addToArrangement(XmlElement* e, int index)
{
    // XXX assumes mode=0

    std::cout<<"Conductor addToArrangement at index "<<index<<e->createDocument("");
    delete e;
    arrangementBackendChanged(0);
}

void Conductor::addToSet(XmlElement* e, int index)
{
    std::cout<<"Conductor addToSet at index "<<index<<e->createDocument("");
    delete e;
    setBackendChanged();
}


void Conductor::changeTagsForClip(XmlElement* e)
{
    std::cout<<"Conductor changeTagsforClip "<<e->createDocument("");
    delete e;
}

void Conductor::addTag(XmlElement* e)
{
    std::cout<<"Conductor addTag "<<e->createDocument("");
    delete e;
//  XXX add new tag to list of all tags
//  notify clipEditorPanel that list has changed.
}

void Conductor::addCategory()
{
    std::cout<<"Conductor addCategory"<<std::endl;
}

void Conductor::removeTag(XmlElement* e)
{
    std::cout<<"Conductor removeTag "<<e->createDocument("");
    delete e;
//  XXX remove tag from list of all tags
//  notify clipEditorPanel that list has changed.
}


void Conductor::changeTag(XmlElement* e)
{
    std::cout<<"Conductor changeTag "<<e->createDocument("");
    delete e;
//  XXX change tag in list of all tags
//  notify clipEditorPanel that list has changed.
}


