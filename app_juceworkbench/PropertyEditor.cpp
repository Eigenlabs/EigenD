/*
 Copyright 2012-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include "PropertyEditor.h"

PropertyEditor::PropertyEditor (Atom* atom, ToolManager* tm, ValueMonitor* vm, bool showNames)
{
    atom_=atom;
    showNames_=showNames;
    tm_=tm;
    vm_=vm;
    vm_->addListener(this);
    oldEditors_.clear();
    setupProperties(atom, true,-1, showNames);

    int ysize=0;
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        ysize=ysize+(*i)->getHeight();
    }
    setSize (384, ysize);
}

PropertyEditor::~PropertyEditor()
{
    deleteAllChildren();
    delete atom_;
}

void PropertyEditor::refresh()
{
    oldEditors_.clear();
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        oldEditors_.insert(std::pair<String,Component*>((*i)->getName(),*i));
    }
    removeAllChildren();
    editors_.clear();

    setupProperties(atom_, true,-1, showNames_);

    for(std::map<String,Component*>::iterator i=oldEditors_.begin();i!=oldEditors_.end();i++)
    {
        delete i->second;
    }
    oldEditors_.clear();

    int ysize=0;
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        ysize=ysize+(*i)->getHeight();
    }
    setSize (384, ysize);
    layout();
}

Atom* PropertyEditor::getAtom()
{
    return atom_;
}

bool PropertyEditor::getShowNames()
{
    return showNames_;
}

void PropertyEditor::setupProperties(Atom* atom, bool topLevel, int indentLevel, bool showNames)
{
    if(topLevel)
    {
        addNamePropertyEditor(atom,indentLevel);
    }

    std::vector<NamedId> ids;

    bool p=atom->hasEditableValue();
    atom->getChildProps(ids, showNames);

    if(!ids.empty())
    {
        if(!topLevel && !p)
        {
            addSeparator(atom->get_desc(),indentLevel);
        }
    }

    if(!topLevel && showNames)
    {
        addNamePropertyEditor(atom,indentLevel);
    }

    if(p)
    {
        addPropertyEditor(atom, indentLevel);
    }

    for(std::vector<NamedId>::iterator i=ids.begin();i!=ids.end();i++)
    {
        String id =(*i).id;
        Atom* d=atom_->getDescendant(id);
        setupProperties(d,false, indentLevel+1,showNames);
    }
}

void PropertyEditor::addPropertyEditor(Atom* child, int indentLevel)
{
    if (indentLevel<0)
    {
        indentLevel=0;
    }
    String name=child->get_desc();
    String value=child->get_value();
    String domain=child->get_domain();
    bool disabled=child->has_non_controller_master();

    if (name.isNotEmpty())
    {
        if (domain.startsWithIgnoreCase("bool"))
        {
            addBoolPropertyEditor(child,name,value,domain,disabled, indentLevel);
        }
        else if (domain.startsWithIgnoreCase("bfloat"))
        {
            addFloatPropertyEditor(child,name,value,domain,disabled,indentLevel);
        }
        else if (domain.startsWithIgnoreCase("bint"))
        {
            addIntPropertyEditor(child,name,value,domain,disabled,indentLevel);
        }
        else if(child->has_protocol("browse"))
        {
            addBrowseEditor(child,name,indentLevel);
        }
        else if (domain.startsWithIgnoreCase("string"))
        {
            if(child->has_protocol("mapper"))
            {
                addMappingEditor(child, indentLevel);
            }
            else if(child->has_protocol("course_editor"))
            {
                addCoursesEditor(child,indentLevel);
            }
            else
            {
                addStringPropertyEditor(child,name,value,domain,disabled, indentLevel);
            }
        }

        else if (domain.startsWithIgnoreCase("enum"))
        {
            addEnumPropertyEditor(child,name,value,domain,disabled, indentLevel);
        }
    }
}

void PropertyEditor::value_changed(String id)
{
    pic::logmsg()<<"PropertyEditor:value_changed "<<std::string(id.toUTF8());
    // call refresh on the editor with that id
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if( id==(*i)->getName())
        {
            AtomEditor* editor=dynamic_cast<AtomEditor*>(*i);
            if(editor!=0)
            {
                editor->refreshValue();
            }
        }
    }

}

void PropertyEditor::addSeparator(String name, int indentLevel)
{
    if(indentLevel<0)
    {
        indentLevel=0;
    }
    PropertySeparator* separator=new PropertySeparator(name);
    separator->setIndent(indentLevel*20);
    editors_.push_back(separator);
    addAndMakeVisible(separator);
}

void PropertyEditor::addNamePropertyEditor(Atom* atom, int indentLevel)
{
    if (indentLevel<0)
    {
        indentLevel=0;
    }
    NamePropertyEditor* editor;
    Component* c=getEditor(atom->get_id()+"name_editor");
    if(c!=0)
    {
        editor=dynamic_cast<NamePropertyEditor*>(c);
    }
    else
    {
        editor=new NamePropertyEditor(atom);
        editor->setName(atom->get_id()+"name_editor");
    }

    editor->setIndent(indentLevel*20);
    editors_.push_back(editor);
    addAndMakeVisible(editor);
}

Component* PropertyEditor::getEditor(String id)
{
    Component* c=0;
    std::map<String,Component*>::iterator i=oldEditors_.find(id);
    if(i!=oldEditors_.end())
    {
       c=i->second;
       oldEditors_.erase(id);
    }
    return c;
}

void PropertyEditor::addBrowseEditor(Atom* atom,String name, int indentLevel)
{
    BrowseEditor* editor;
    Component* c=getEditor(atom->get_id());
    if(c!=0)
    {
        editor=dynamic_cast<BrowseEditor*>(c);
    }
    else
    {
        editor=new BrowseEditor(atom,name);
        editor->setName(atom->get_id());
    }
    editor->setIndent(indentLevel*20);
    editors_.push_back(editor);
    addAndMakeVisible(editor);
}

void PropertyEditor::addMappingEditor(Atom* atom, int indentLevel)
{
    StringMappingPropertyEditor* editor;
    Component* c=getEditor(atom->get_id());
    if(c!=0)
    {
        editor=dynamic_cast<StringMappingPropertyEditor*>(c);
    }
    else
    {
        editor=new StringMappingPropertyEditor(atom, tm_);
        editor->setName(atom->get_id());
        vm_->add(atom);
    }
    editor->setIndent(indentLevel*20);
    editors_.push_back(editor);
    addAndMakeVisible(editor);
}

void PropertyEditor::addKeyToCourseEditor(Atom* atom, int indentLevel)
{
    KeyToCoursePropertyEditor* editor;
    Component* c=getEditor(atom->get_id());
    if(c!=0)
    {
        editor=dynamic_cast<KeyToCoursePropertyEditor*>(c);
    }
    else
    {
        editor=new KeyToCoursePropertyEditor(atom, tm_);
        editor->setName(atom->get_id());
        vm_->add(atom);
    }
    editor->setIndent(indentLevel*20);
    editors_.push_back(editor);
    addAndMakeVisible(editor);
}

void PropertyEditor::addCoursesEditor(Atom* atom, int indentLevel)
{
    CoursesPropertyEditor* editor;
    Component* c=getEditor(atom->get_id());
    if(c!=0)
    {
        editor=dynamic_cast<CoursesPropertyEditor*>(c);
    }
    else
    {
        editor=new CoursesPropertyEditor(atom, tm_);
        editor->setName(atom->get_id());
        vm_->add(atom);
    }
    editor->setIndent(indentLevel*20);
    editors_.push_back(editor);
    addAndMakeVisible(editor);
}

void PropertyEditor::addBoolPropertyEditor(Atom* atom,String name, String value, String domain, bool disabled, int indentLevel)
{
    BoolPropertyEditor* editor;
    Component* c=getEditor(atom->get_id());
    if(c!=0)
    {
        editor=dynamic_cast<BoolPropertyEditor*>(c);
    }
    else
    {
        editor=new BoolPropertyEditor();
        pic::logmsg()<<"created BoolPropertyEditor";
        editor->setAtom(atom);
        editor->setName(atom->get_id());
        editor->setItem(name,value.equalsIgnoreCase(String("true")));
        editor->setDisabled(disabled);
        vm_->add(atom);
    }
    editor->setIndent(indentLevel*20);
    editors_.push_back(editor);
    addAndMakeVisible(editor);
}

void PropertyEditor::addFloatPropertyEditor(Atom* atom,String name,String value, String domain, bool disabled, int indentLevel)
{
    FloatPropertyEditor* editor;
    Component* c=getEditor(atom->get_id());
    if(c!=0)
    {
        editor=dynamic_cast<FloatPropertyEditor*>(c);
    }
    else
    {
        editor=new FloatPropertyEditor();
        editor->setAtom(atom);
        editor->setName(atom->get_id());
        float min=getMin(domain);
        float max=getMax(domain);
        editor->setRange(min,max);
        editor->setItem(name,value.getFloatValue());
        editor->setDisabled(disabled);
        vm_->add(atom);
    }
    editor->setIndent(indentLevel*20);
    editors_.push_back(editor);
    addAndMakeVisible(editor);
}

void PropertyEditor::addIntPropertyEditor(Atom* atom,String name, String value, String domain, bool disabled, int indentLevel)
{
    IntPropertyEditor* editor;
    Component* c=getEditor(atom->get_id());
    if(c!=0)
    {
        editor=dynamic_cast<IntPropertyEditor*>(c);
    }
    else
    {
        editor=new IntPropertyEditor();
        editor->setAtom(atom);
        editor->setName(atom->get_id());
        int  min=(int)getMin(domain);
        int  max=(int)getMax(domain);
        editor->setRange(min,max);
        editor->setItem(name,value.getIntValue());
        editor->setDisabled(disabled);
        vm_->add(atom);
    }
    editor->setIndent(indentLevel*20);
    editors_.push_back(editor);
    addAndMakeVisible(editor);
}

void PropertyEditor::addStringPropertyEditor(Atom* atom,String name, String value,String domain,bool disabled, int indentLevel)
{
    StringPropertyEditor* editor;
    Component* c=getEditor(atom->get_id());
    if(c!=0)
    {
        editor=dynamic_cast<StringPropertyEditor*>(c);
    }
    else
    {
        editor=new StringPropertyEditor();
        editor->setAtom(atom);
        editor->setName(atom->get_id());
        editor->setItem(name,value);
        editor->setDisabled(disabled);
        vm_->add(atom);
    }
    editor->setIndent(indentLevel*20);
    editors_.push_back(editor);
    addAndMakeVisible(editor);
}

void PropertyEditor::addEnumPropertyEditor(Atom* atom,String name, String value,String domain,bool disabled, int indentLevel)
{
    EnumPropertyEditor* editor;
    Component* c=getEditor(atom->get_id());
    if(c!=0)
    {
        editor=dynamic_cast<EnumPropertyEditor*>(c);
    }
    else
    {
        editor=new EnumPropertyEditor();
        editor->setAtom(atom);
        editor->setItem(name,value);
        editor->setDisabled(disabled);
        vm_->add(atom);
    }
    editor->setIndent(indentLevel*20);
    editors_.push_back(editor);
    addAndMakeVisible(editor);
}

float PropertyEditor::getMin(String domain)
{
    String min=domain.fromFirstOccurrenceOf("(",false,true);
    min=min.upToFirstOccurrenceOf(",",false,true);
    return min.getFloatValue();
}

float PropertyEditor::getMax(String domain)
{
    String max=domain.fromFirstOccurrenceOf(",",false,true);
    max=max.upToFirstOccurrenceOf(",",false,true);
    return max.getFloatValue();
}

void PropertyEditor::resized()
{
    layout();
}
void PropertyEditor::layout()
{
//    pic::logmsg()<<"PropertyEditor: layout";
    int ypos=0;
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        (*i)->setTopLeftPosition((*i)->getX(),ypos);
        ypos=(*i)->getHeight()+ypos;
    }

    repaint();
}

void PropertyEditor::childBoundsChanged(Component* child)
{
    layout();
}

void PropertyEditor::enumerate_updated(String id,String path, int nf, int nc)
{
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if( id==(*i)->getName())
        {
            BrowseEditor* b=dynamic_cast<BrowseEditor*>(*i);
            b->enumerate_updated(path,nf,nc);
        }
    }
}

void PropertyEditor::sourcekeys_updated(String id, String keys)
{
    //std::cout<<"sourcekeys_updated "<<id.toUTF8()<<" "<<keys.toUTF8()<<std::endl;
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if( id==(*i)->getName())
        {
            StringMappingPropertyEditor* b=dynamic_cast<StringMappingPropertyEditor*>(*i);
            b->sourcekeys_updated(keys);
        }
    }

}

void PropertyEditor::activated(String id)
{
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if( id==(*i)->getName())
        {
            BrowseEditor* b=dynamic_cast<BrowseEditor*>(*i);
            b->activated();
        }
    }
}

void PropertyEditor::current(String id, String cookie)
{
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if( id==(*i)->getName())
        {
            BrowseEditor* b=dynamic_cast<BrowseEditor*>(*i);
            b->current(cookie);
        }
    }
}


void PropertyEditor::finfo_updated(String id,String path, const std::set<std::string>& finfo)
{
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if( id==(*i)->getName())
        {
            BrowseEditor* b=dynamic_cast<BrowseEditor*>(*i);
            b->finfo_updated(path,finfo);
        }
    }
}

void PropertyEditor::cinfo_updated(String id, String path,const std::set<std::string>& cinfo)
{
    for(std::vector<Component*>::iterator i=editors_.begin();i!=editors_.end();i++)
    {
        if( id==(*i)->getName())
        {
            BrowseEditor* b=dynamic_cast<BrowseEditor*>(*i);
            b->cinfo_updated(path,cinfo);
        }
    }
}
