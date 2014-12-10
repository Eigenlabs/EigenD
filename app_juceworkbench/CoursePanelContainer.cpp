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

#include "CoursePanelContainer.h"


CoursePanelContainer::CoursePanelContainer (Atom* atom, String mapping, Component* editor)
    : atom_(atom), mapping_(mapping), editor_(editor)
{

    setSize (450, 1000);

    doDisplayMapping(mapping);
}

CoursePanelContainer::~CoursePanelContainer()
{
}

void CoursePanelContainer::paint (Graphics& g)
{
}

void CoursePanelContainer::resized()
{
}

void CoursePanelContainer::addElement()
{
    CoursePanel* element=new CoursePanel(0,elements_.size()*200);
    elements_.push_back(element);
    addAndMakeVisible(element);
    setSize(450,elements_.size()*200);
    checkRevertable();
}

void CoursePanelContainer::removeElement(CoursePanel* element)
{
    element->removing=true;
    removeChildComponent(element);
    elements_.remove(element);

    int count=0;
    for(std::list<CoursePanel*>::const_iterator i=elements_.begin();i!=elements_.end();++i)
    {
        (*i)->setTopLeftPosition(0,count*200);
        count++;
    }
    delete element;
    setSize(450,elements_.size()*200);
    updateMapping();
    checkRevertable();
}

void CoursePanelContainer::updateMapping()
{
    mapping_=makeMappingString();
    pic::logmsg()<<"CoursePanelContainer: updateMapping  value for atom="<<std::string(mapping_.toUTF8());    
    atom_->set_value(mapping_);
}

String CoursePanelContainer::makeMappingString()
{
    String mapping="[";
    for(std::list<CoursePanel*>::iterator i=elements_.begin();i!=elements_.end();++i)
    {
        //if(((*i)->isEmpty())==false)
        if((*i)->isValid())
        {
            mapping=mapping+(*i)->makeMappingString();
            mapping=mapping+",";
        }
    }
    mapping=mapping.upToLastOccurrenceOf(",",false,true)+"]";
    return mapping;
}

String CoursePanelContainer::makeDisplayedMappingString()
{
    String mapping="[";
    for(std::list<CoursePanel*>::iterator i=elements_.begin();i!=elements_.end();++i)
    {
            mapping=mapping+(*i)->makeMappingString();
            mapping=mapping+",";
    }
    mapping=mapping.upToLastOccurrenceOf(",",false,true)+"]";
    return mapping;
}


void CoursePanelContainer::checkRevertable()
{
    String mapping=makeDisplayedMappingString();

    if(!(mapping.equalsIgnoreCase(mapping_)))
    {
        //std::cout<<"Mapping editor is revertable"<<std::endl;
        editor_->postCommandMessage(1);
    }
    else
    {
        //std::cout<<"Mapping editor is not revertable"<<std::endl;
        editor_->postCommandMessage(0);
    }
}

void CoursePanelContainer::clearMapping()
{
    mapping_=("[]");
    doDisplayMapping(mapping_);
    atom_->set_value(mapping_);
}

void CoursePanelContainer::doDisplayMapping(String mapping)
{
    for(std::list<CoursePanel*>::iterator i=elements_.begin();i!=elements_.end();++i)
    {
        (*i)->removing=true;
    }

    elements_.clear();
    deleteAllChildren();
    if(mapping.equalsIgnoreCase("[]"))
    {
        elements_.push_back(new CoursePanel(0,0));
        setSize(450,elements_.size()*200);
        for(std::list<CoursePanel*>::const_iterator i=elements_.begin();i!=elements_.end();++i)
        {
            addAndMakeVisible(*i);
        }
    }
    else
    {
        String s=mapping.fromFirstOccurrenceOf("[",false,false);
        s=s.upToLastOccurrenceOf("]",false,false);
        while(s.isNotEmpty())
        {
            int indent=0;
//              std::cout<<"analysing"<<std::string(s.toUTF8())<<std::endl;
            for (int i=0;i<s.length();i++)
            {
//                  std::cout<<std::string(s.substring(i,i+1).toUTF8())<<indent<<std::endl;
                if(s.substring(i,i+1)=="[")
                {
                    indent++;
                }
                else if(s.substring(i,i+1)=="]")
                {
                    indent--;
                    if(indent==0)
                    {
                        pic::logmsg()<<"course string="<<std::string((s.substring(0,i+1)).toUTF8());
                        CoursePanel* element=new CoursePanel(0,elements_.size()*200);
                        element->setValues(s.substring(0,i+1));
                        elements_.push_back(element);
                        addAndMakeVisible(element);
                        setSize(450,elements_.size()*200);
                        s=(s.substring(i+1)).fromFirstOccurrenceOf("[",true,false);
                        break;
                    }
                }
//                  std::cout<<"   indent="<<indent<<std::endl;
            }
        }
    }
    editor_->postCommandMessage(0);

}

String CoursePanelContainer::getMapping()
{
    return atom_->get_value();
}


void CoursePanelContainer::changed()
{
    displayMapping(getMapping());
}

void CoursePanelContainer::displayMapping(String mapping)
{
    pic::logmsg()<<"CoursePanelContainer:displayMapping displayedMapping="<<std::string(mapping_.toUTF8())<<" eigend mapping="<<std::string(mapping.toUTF8());
    if (!mapping.equalsIgnoreCase(mapping_)) 
    {
        doDisplayMapping(mapping);
    }
}

void CoursePanelContainer::revertMapping(String mapping)
{
    pic::logmsg()<<"CoursePanelContainer:revertMapping displayedMapping="<<std::string(makeDisplayedMappingString().toUTF8())<<" eigend mapping="<<std::string(mapping.toUTF8());
    if (!mapping.equalsIgnoreCase(makeDisplayedMappingString())) 
    {
        doDisplayMapping(mapping);
    }
}

