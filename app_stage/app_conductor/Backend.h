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

#ifndef __BACKEND__
#define __BACKEND__

#include "juce.h"

class SelectionListener
{
    public:
        SelectionListener(){};
        virtual ~SelectionListener(){};
        virtual void selectionChanged()=0;
};

class SceneListener
{
    public:
        SceneListener(){};
        virtual ~SceneListener(){};
        virtual void sceneChanged(int mode)=0;
};

class ClipPoolListener
{
    public:
        ClipPoolListener(){};
        virtual ~ClipPoolListener(){};
        virtual void clipPoolChanged()=0;
};

class ArrangementListener
{
    public:
        ArrangementListener(){};
        virtual ~ArrangementListener(){};
        virtual void arrangementChanged(int mode)=0;
};

class SetListener
{
    public:
        SetListener(){};
        virtual ~SetListener(){};
        virtual void setChanged()=0;
};


class ClipManagerBackend
{
public:
    ClipManagerBackend();
    ~ClipManagerBackend(){};
    virtual void changeTagsForClip(XmlElement*) = 0;
    virtual void addTag(XmlElement*)=0;
    virtual void removeTag(XmlElement*)=0;
    virtual void changeTag(XmlElement*)=0;
    virtual void addCategory()=0;
    virtual XmlElement* getColumnCategories() = 0;
    virtual XmlElement* getAllTags() = 0;
    virtual XmlElement* getSelectedClips(XmlElement*) = 0;
    virtual void addToClipPool(XmlElement*, int index) = 0;

    void addSelectionListener(SelectionListener *l) { selectionListener_.add(l); }
    void removeSelectionListener(SelectionListener *l) { selectionListener_.remove(l); }
    void clipManagerBackendChanged() { selectionListener_.call(&SelectionListener::selectionChanged); }

private:
    ListenerList<SelectionListener> selectionListener_;
};

class ClipPoolBackend
{
public:

    ClipPoolBackend();
    ~ClipPoolBackend(){};

    virtual void addToScene(XmlElement* e,int) = 0;
    virtual void addToArrangement(XmlElement* e, int)=0;//which arrangement?
    virtual void removeFromClipPool(XmlElement* e) = 0;
    virtual XmlElement* getClipPoolClips() = 0;
    virtual void addToClipPool(XmlElement*,int) = 0;
    virtual void viewClip(XmlElement*)=0;

    void addClipPoolListener(ClipPoolListener *l) { clipPoolListener_.add(l); }
    void removeClipPoolListener(ClipPoolListener *l) { clipPoolListener_.remove(l); }
    void clipPoolBackendChanged() { clipPoolListener_.call(&ClipPoolListener::clipPoolChanged); }

private:
    ListenerList<ClipPoolListener> clipPoolListener_;
};

class SceneBackend
{
public:
    SceneBackend();
    ~SceneBackend(){};
    virtual XmlElement* getScene(int mode) = 0;
    virtual void addToScene(XmlElement* e,int index) = 0;// which scene?
    virtual void removeFromScene(XmlElement* e, int mode) = 0;
    virtual void viewClip(XmlElement*)=0;

    void addSceneListener(SceneListener *l) { sceneListener_.add(l); }
    void removeSceneListener(SceneListener *l) { sceneListener_.remove(l); }
    void sceneBackendChanged(int mode) { sceneListener_.call(&SceneListener::sceneChanged,mode); }

private:
    ListenerList<SceneListener> sceneListener_;

};

class ArrangementBackend
{
public:
    ArrangementBackend();
    ~ArrangementBackend(){};

    virtual XmlElement* getArrangement(int mode) = 0;
    virtual void addToArrangement(XmlElement* e, int index) = 0;
    virtual void removeFromArrangement(XmlElement* e, int mode) = 0;
    virtual void viewClip(XmlElement*)=0;

    void addArrangementListener(ArrangementListener *l) { arrangementListener_.add(l); }
    void removeArrangementListener(ArrangementListener *l) { arrangementListener_.remove(l); }
    void arrangementBackendChanged(int mode) { arrangementListener_.call(&ArrangementListener::arrangementChanged,mode); }

private:
    ListenerList<ArrangementListener> arrangementListener_;

};

class SetBackend
{
public:
    SetBackend();
    ~SetBackend(){};
    virtual XmlElement* getSet() = 0;
    virtual void addToSet(XmlElement* e, int index) = 0;
    virtual void removeFromSet(XmlElement* e) = 0;
    virtual void play(XmlElement* e) = 0;
    virtual void viewClip(XmlElement*)=0;

    void addSetListener(SetListener *l) { setListener_.add(l); }
    void removeSetListener(SetListener *l) { setListener_.remove(l); }
    void setBackendChanged() { setListener_.call(&SetListener::setChanged); }

private:
    ListenerList<SetListener> setListener_;
};

#endif

