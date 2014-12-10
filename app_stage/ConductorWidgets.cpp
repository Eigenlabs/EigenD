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

#include "ConductorWidgets.h"

ConductorClipManagerWidget::ConductorClipManagerWidget(WidgetComponent *widget): EigenRPCWidget(widget)
{
    addAndMakeVisible(display_ = new ClipManagerWidget(this));
}

void ConductorClipManagerWidget::changeTag(XmlElement *e)
{
    String xml = e->createDocument("");
    String result;

    if(!getWidget()->doServerRpc("change_tag",xml,result))
    {
        std::cout<<"rpc change_tag failed"<<std::endl;
        // XXX test
        //clipManagerBackendChanged();
    }
}

void ConductorClipManagerWidget::addTag(XmlElement *e)
{
    String xml = e->createDocument("");
    String result;

    if(!getWidget()->doServerRpc("add_tag",xml,result))
    {
        std::cout<<"rpc add_tag failed"<<std::endl;
        // XXX test
        //clipManagerBackendChanged();
    }
}

void ConductorClipManagerWidget::addCategory()
{
    String result;
    if(!getWidget()->doServerRpc("add_category","",result))
    {
        std::cout<<"rpc add_category failed"<<std::endl;
        // XXX test
        //clipManagerBackendChanged();
    }
}

void ConductorClipManagerWidget::removeTag(XmlElement *e)
{
    String xml = e->createDocument("");
    String result;

    if(!getWidget()->doServerRpc("remove_tag",xml,result))
    {
        std::cout<<"rpc remove_tag failed"<<std::endl;
        // XXX test
        //clipManagerBackendChanged();
    }
}


void ConductorClipManagerWidget::changeTagsForClip(XmlElement *e)
{
    String xml = e->createDocument("");
    String result;

    if(!getWidget()->doServerRpc("change_tags_for_clip",xml,result))
    {
        std::cout<<"rpc change_tags_for_clip failed"<<std::endl;
        // XXX test
        //clipManagerBackendChanged();
    }
}

XmlElement* ConductorClipManagerWidget::getColumnCategories()
{
    String result;
    std::cout << "calling get column categories " << "\n";

    if(!getWidget()->doServerRpc("get_column_categories","",result))
    {
        std::cout<<"rpc get_column_categories failed"<<std::endl;
        // XXX test
        //clipManagerBackendChanged();
        return new XmlElement("CATEGORIES");
    }

    std::cout << "get column categories tags " << result << "\n";

    XmlDocument doc(result);
    return doc.getDocumentElement();
}

XmlElement* ConductorClipManagerWidget::getAllTags()
{
    String result;
    std::cout << "calling get all tags " << "\n";

    if(!getWidget()->doServerRpc("get_all_tags","",result))
    {
         // XXX test
        //clipManagerBackendChanged();
        std::cout<<"rpc get_all_tags failed"<<std::endl;
        return new XmlElement("TAGS");
    }

    std::cout << "get all tags " << result << "\n";

    XmlDocument doc(result);
    return doc.getDocumentElement();
}

XmlElement* ConductorClipManagerWidget::getSelectedClips(XmlElement* e)
{
    String result;
    String xml = e->createDocument("");

    if(!getWidget()->doServerRpc("get_selected_clips",xml,result))
    {
        std::cout<<"rpc get_selected_clips failed"<<std::endl;
         // XXX test
        clipManagerBackendChanged();
        return new XmlElement("CLIPS");
    }

    XmlDocument doc(result);
    return doc.getDocumentElement();
}

void ConductorClipManagerWidget::addToClipPool(XmlElement *e, int index)
{
    e->setAttribute("param_index",index);
    String xml = e->createDocument("");
    e->removeAttribute("param_index");

    String result;

    if(!getWidget()->doServerRpc("add_to_clip_pool",xml,result))
    {
        std::cout<<"rpc add_to_clip_pool failed"<<std::endl;
         // XXX test
        //clipManagerBackendChanged();
    }
}

void ConductorClipManagerWidget::valueChanged(const WidgetValueMessage *vm)
{
    std::cout<< "clip manager valueChanged"<<std::endl;
    if(vm->data != current_)
    {
        current_ = vm->data;
        clipManagerBackendChanged();
        std::cout << " called clipManagerBackendChanged"<<std::endl;
    }
}


ConductorClipPoolWidget::ConductorClipPoolWidget(WidgetComponent *widget): EigenRPCWidget(widget)
{
    addAndMakeVisible(display_ = new ClipPoolWidget(this));
}

void ConductorClipPoolWidget::viewClip(XmlElement* e)
{
}

void ConductorClipPoolWidget::addToArrangement(XmlElement* e, int)
{
}

void ConductorClipPoolWidget::addToScene(XmlElement* e, int)
{
}

void ConductorClipPoolWidget::removeFromClipPool(XmlElement* e)
{
}

XmlElement* ConductorClipPoolWidget::getClipPoolClips()
{
    //XXX initially just read from file to get it working
    XmlDocument doc(File("~/conductor_model/clip_pool.xml"));
    XmlElement* e=(doc.getDocumentElement());
    return e;
}

void ConductorClipPoolWidget::addToClipPool(XmlElement*, int)
{
}

void ConductorClipPoolWidget::valueChanged(const WidgetValueMessage *vm)
{
}


ConductorArrangementViewWidget::ConductorArrangementViewWidget(WidgetComponent *widget): EigenRPCWidget(widget)
{
    addAndMakeVisible(display_ = new ArrangementViewWidget(this,ArrangementViewWidget::PLAYING));
}

XmlElement* ConductorArrangementViewWidget::getArrangement(int mode)
{
    //XXX initially just read from file to get it working
    XmlDocument doc(File("~/conductor_model/arrangement1.xml"));
    XmlElement* e=(doc.getDocumentElement());
    return e;
}

void ConductorArrangementViewWidget::addToArrangement(XmlElement* e, int)
{
}

void ConductorArrangementViewWidget::removeFromArrangement(XmlElement* e, int mode)
{
}

void ConductorArrangementViewWidget::valueChanged(const WidgetValueMessage *vm)
{
}

void ConductorSceneViewWidget::viewClip(XmlElement *e)
{
}

void ConductorArrangementViewWidget::viewClip(XmlElement *e)
{
}


ConductorSceneViewWidget::ConductorSceneViewWidget(WidgetComponent *widget): EigenRPCWidget(widget)
{
    addAndMakeVisible(display_ = new SceneViewWidget(this,SceneViewWidget::PLAYING));
}

XmlElement* ConductorSceneViewWidget::getScene(int mode)
{
    //XXX initially just read from file to get it working
    XmlDocument doc(File("~/conductor_model/scene1.xml"));
    XmlElement* e=(doc.getDocumentElement());
    return e;
}

void ConductorSceneViewWidget::addToScene(XmlElement* e, int)
{
}

void ConductorSceneViewWidget::removeFromScene(XmlElement* e, int mode)
{
}

void ConductorSceneViewWidget::valueChanged(const WidgetValueMessage *vm)
{
}

#include "app_conductor/ArrangementList.cpp"
#include "app_conductor/ArrangementViewWidget.cpp"
#include "app_conductor/Backend.cpp"
#include "app_conductor/Clip.cpp"
#include "app_conductor/ClipBox.cpp"
#include "app_conductor/ClipEditorPanel.cpp"
#include "app_conductor/ClipList.cpp"
#include "app_conductor/ClipManagerList.cpp"
#include "app_conductor/ClipManagerPanel.cpp"
#include "app_conductor/ClipManagerWidget.cpp"
#include "app_conductor/ClipPanel.cpp"
#include "app_conductor/ClipPoolFactory.cpp"
#include "app_conductor/ClipPoolList.cpp"
#include "app_conductor/ClipPoolWidget.cpp"
#include "app_conductor/SceneToolbarItemFactory.cpp"
#include "app_conductor/EditAttributePanel.cpp"
#include "app_conductor/ImageResources.cpp"
#include "app_conductor/SceneList.cpp"
#include "app_conductor/ScenePanel.cpp"
#include "app_conductor/SceneViewWidget.cpp"
#include "app_conductor/SetList.cpp"
#include "app_conductor/SetViewWidget.cpp"
#include "app_conductor/TagTable.cpp"
