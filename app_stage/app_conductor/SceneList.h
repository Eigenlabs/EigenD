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

#ifndef __SCENELIST__
#define __SCENELIST__

#include "juce.h"
#include "Conductor.h"
#include "ClipList.h"
#include "SceneViewWidget.h"

class SceneList:public ClipList,
                public DragAndDropTarget,
                public SceneListener
{
public:
    SceneList(SceneBackend* backend, int mode);
    ~SceneList();
    void setMode(int);
    void sceneChanged(int mode);
    void removeFromScene();
    void duplicate();
    virtual bool isInterestedInDragSource(const SourceDetails &dragSourceDetails);
    virtual void itemDropped(const SourceDetails &dragSourceDetails);
    virtual void listBoxItemDoubleClicked(int row, const MouseEvent& e);
    virtual void listBoxItemClicked(int row, const MouseEvent& e);
    String getSceneName();

private:
    SceneBackend* backend_;
    int mode_;
    String sceneName_;
};

#endif
