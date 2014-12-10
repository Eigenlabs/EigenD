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

#ifndef __CONDUCTOR_WIDGETS__
#define __CONDUCTOR_WIDGETS__

#include "EigenWidgets.h"
#include "app_conductor/Backend.h"
#include "app_conductor/ClipManagerWidget.h"
#include "app_conductor/ArrangementViewWidget.h"
#include "app_conductor/ClipPoolWidget.h"
#include "app_conductor/SceneViewWidget.h"

class ConductorClipManagerWidget: public EigenRPCWidget, public ClipManagerBackend
{
public:

    ConductorClipManagerWidget(WidgetComponent *widget);

    float getGraphicsWidth() { return 550.0; }
    float getGraphicsHeight() { return 310.0; }
    void resized() { display_->setBounds(getLocalBounds()); }

    void changeTagsForClip(XmlElement*);
    XmlElement* getColumnCategories();
    XmlElement* getAllTags();
    XmlElement* getSelectedClips(XmlElement*);
    void addToClipPool(XmlElement*, int index);
    void addTag(XmlElement *);
    void removeTag(XmlElement *);
    void changeTag(XmlElement *);

    void addCategory();

    void valueChanged(const WidgetValueMessage *vm);

private:
    std::set<String> selectedTags_;
    ScopedPointer<ClipManagerWidget> display_;
    WidgetData current_;
};

class ConductorClipPoolWidget: public EigenRPCWidget, public ClipPoolBackend
{
public:
    
    ConductorClipPoolWidget(WidgetComponent *widget);

    float getGraphicsWidth() { return 500.0; }
    float getGraphicsHeight() { return 300.0; }
    void resized() { display_->setBounds(getLocalBounds()); }

    void addToScene(XmlElement* e, int );
    void addToArrangement(XmlElement* e, int);
    void removeFromClipPool(XmlElement* e);
    XmlElement* getClipPoolClips();
    void addToClipPool(XmlElement*, int);
    void viewClip(XmlElement*);

    void valueChanged(const WidgetValueMessage *vm);

private:
    ScopedPointer<ClipPoolWidget> display_;
};

class ConductorArrangementViewWidget: public EigenRPCWidget, public ArrangementBackend
{
public:
    
    ConductorArrangementViewWidget(WidgetComponent *widget);

    float getGraphicsWidth() { return 500.0; }
    float getGraphicsHeight() { return 300.0; }
    void resized() { display_->setBounds(getLocalBounds()); }

    XmlElement* getArrangement(int mode);
    void addToArrangement(XmlElement* e, int index);
    void removeFromArrangement(XmlElement* e, int mode);
    void viewClip(XmlElement*);

    void valueChanged(const WidgetValueMessage *vm);
private:
    ScopedPointer<ArrangementViewWidget> display_;
};

class ConductorSceneViewWidget: public EigenRPCWidget, public SceneBackend
{
public:
    ConductorSceneViewWidget(WidgetComponent *widget);

    float getGraphicsWidth() { return 500.0; }
    float getGraphicsHeight() { return 300.0; }
    void resized() { display_->setBounds(getLocalBounds()); }

    XmlElement* getScene(int mode);
    void addToScene(XmlElement* e, int index);// which scene?
    void removeFromScene(XmlElement* e, int mode);
    void viewClip(XmlElement*);

    void valueChanged(const WidgetValueMessage *vm);
private:
    ScopedPointer<SceneViewWidget> display_;
};

#endif
