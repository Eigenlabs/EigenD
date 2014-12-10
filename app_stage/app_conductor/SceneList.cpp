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

#include "SceneList.h"

SceneList::SceneList (SceneBackend* backend, int mode)
    : ClipList("SceneList"),backend_(backend),mode_(mode),sceneName_(String::empty)
{
    setSize (600, 400);
    sceneChanged(mode_);
    backend_->addSceneListener(this);
}

SceneList::~SceneList()
{
}

void SceneList::setMode(int mode)
{
    if(mode!=mode_)
    {
        deselectAllRows();
        mode_=mode;
    }
}

String SceneList::getSceneName()
{
    return sceneName_;
}

void SceneList::sceneChanged(int mode)
{
    if(mode==mode_)
    {
        std::cout<<"SceneList- sceneChanged mode="<<mode_<<std::endl;
        XmlElement* clipData=backend_->getScene(mode_);
        if(clipData!=0)
        {
            sceneName_=clipData->getStringAttribute("name");
            SceneViewWidget* svw=findParentComponentOfClass<SceneViewWidget>();
            if(svw!=0)
            {
                svw->setSceneName(sceneName_);
            }
        }
//        std::cout<<"name of Scene="<<clipData->getStringAttribute("name")<<std::endl;

        refreshList(clipData);
    }
}

void SceneList::removeFromScene()
{
    std::cout<<"SceneList- removeFromScene"<<std::endl;
    XmlElement* e= getSelectedClips();
    backend_->removeFromScene(e, mode_);
}

void SceneList::duplicate()
{
    std::cout<<"SceneList - duplicate"<<std::endl;
    // XXX
}

void SceneList::listBoxItemClicked(int row, const MouseEvent& e)
{
}


void SceneList::listBoxItemDoubleClicked(int row, const MouseEvent& e)
{
    std::cout<<"SceneList - doubleClicked"<<std::endl;
    if((row>=0) && (row<(int)v_.size()))
    {
        Clip* c=v_[row];
        if(c!=0)
        {
            backend_->viewClip(v_[row]->toXml());
        }
    }
}

bool SceneList::isInterestedInDragSource(const SourceDetails &dragSourceDetails)
{
    String source=dragSourceDetails.description.toString();

    std::cout<<"sceneList - isinterestedInDragSource source="<<source<<std::endl;
    if(source.equalsIgnoreCase("ClipPoolList")||source.equalsIgnoreCase("SceneList"))
    {
        ClipList::onInterestedInDragSource(dragSourceDetails);
        return true;
    }

    return false;
}

void SceneList::itemDropped(const SourceDetails &dragSourceDetails)
{
    String source=dragSourceDetails.description.toString();
    if(source.equalsIgnoreCase("ClipPoolList"))
    {
        std::cout<<"sceneList - ClipPoolList itemDropped"<<std::endl;
        Component* c=dragSourceDetails.sourceComponent.get();
        if(c!=0)
        {
            ClipList* cl=dynamic_cast<ClipList*>(c);
            if(cl!=0)
            {
                Point <int> p=dragSourceDetails.localPosition;
                backend_->addToScene(cl->getSelectedClips(),getInsertionIndexForPosition(p.getX(),p.getY()));
            }
        }
    }
    else if(source.equalsIgnoreCase("SceneList"))
    {
        ClipList::onDropSelf(dragSourceDetails);
    }
}


