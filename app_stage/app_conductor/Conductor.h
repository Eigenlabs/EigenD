#ifndef __CONDUCTOR__
#define __CONDUCTOR__

#include "juce.h"
#include "Backend.h"
#include <set>

class Conductor: public ArrangementBackend, public SceneBackend, public ClipPoolBackend, public ClipManagerBackend, public SetBackend
{
public:

    Conductor();
    ~Conductor();

    ClipManagerBackend* getClipManagerBackend(){return this;};
    ClipPoolBackend* getClipPoolBackend(){return this;};
    SceneBackend* getSceneBackend(){return this;};
    ArrangementBackend* getArrangementBackend(){return this;};
    SetBackend* getSetBackend(){return this;};

    XmlElement* getAllTags();
    XmlElement* getSelectedClips(XmlElement* e);
    XmlElement* getColumnCategories();

    XmlElement* getClipPoolClips();
    XmlElement* getScene(int mode);
    XmlElement* getArrangement(int mode);
    XmlElement* getSet();

    void addSelectionListener(SelectionListener*);
    void addClipPoolListener(ClipPoolListener*);
    void addSceneListener(SceneListener*);
    void addArrangementListener(ArrangementListener*);
    void addSetListener(SetListener*);

    void addToClipPool(XmlElement*, int);
    void removeFromClipPool(XmlElement* e);
    void addToScene(XmlElement* e, int);
    void addToArrangement(XmlElement* e,int);
    void removeFromScene(XmlElement* e, int);
    void removeFromArrangement(XmlElement* e, int);
    void addToSet(XmlElement* e, int);
    void removeFromSet(XmlElement* e);
    void play(XmlElement* e);

    void changeTagsForClip(XmlElement*);
    void addTag(XmlElement*);
    void removeTag(XmlElement*);
    void changeTag(XmlElement*);
    void viewClip(XmlElement*);

    void addCategory();


private:
    std::set<String> selectedTags_;
};


#endif

