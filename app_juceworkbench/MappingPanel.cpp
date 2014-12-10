/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
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
//[/Headers]

#include "MappingPanel.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MappingPanel::MappingPanel (Atom* atom, String mapping, Component* editor)
    : atom_(atom), mapping_(mapping), editor_(editor)
{

    //[UserPreSize]
    //[/UserPreSize]

    setSize (370, 1000);


    //[Constructor] You can add your own custom stuff here..

    doDisplayMapping(mapping);
    //[/Constructor]
}

MappingPanel::~MappingPanel()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MappingPanel::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MappingPanel::resized()
{
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void MappingPanel::addElement()
{
    //std::cout<<"MappingPanel:addElement"<<std::endl;
    XYMapComponent* element=new XYMapComponent(0,elements_.size()*24);
    elements_.push_back(element);
    addAndMakeVisible(element);
    setSize(370,elements_.size()*24);
    checkRevertable();
}

void MappingPanel::removeElement(XYMapComponent* element)
{
    //std::cout<<"MappingPanel::removeElement"<<std::endl;
    for(std::list<XYMapComponent*>::iterator i=elements_.begin();i!=elements_.end();++i)
    {
        (*i)->removing=true;
    }

    removeChildComponent(element);
    elements_.remove(element);

    int count=0;
    for(std::list<XYMapComponent*>::const_iterator i=elements_.begin();i!=elements_.end();++i)
    {
        (*i)->setTopLeftPosition(0,count*24);
        count++;
    }
    delete element;
    setSize(370,elements_.size()*24);
    updateMapping();
    checkRevertable();
}

void MappingPanel::updateMapping()
{
    mapping_=makeMappingString();
    //std::cout<<"MappingPanel::updateMapping to "<<mapping_.toUTF8()<<std::endl;
    atom_->set_value(mapping_);

    //checkRevertable();
}

String MappingPanel::makeMappingString()
{
    String mapping="[";
    for(std::list<XYMapComponent*>::iterator i=elements_.begin();i!=elements_.end();++i)
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

String MappingPanel::makeDisplayedMappingString()
{
    String mapping="[";
    for(std::list<XYMapComponent*>::iterator i=elements_.begin();i!=elements_.end();++i)
    {
            mapping=mapping+(*i)->makeMappingString();
            mapping=mapping+",";
    }
    mapping=mapping.upToLastOccurrenceOf(",",false,true)+"]";
    return mapping;
}


void MappingPanel::checkRevertable()
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

void MappingPanel::setUpstreamKeys(String upstream)
{
    //std::cout<<"MappingPanel::setUpStreamKeys "<<upstream.toUTF8()<<std::endl;
    elements_.clear();
    deleteAllChildren();
    String s=upstream.removeCharacters("[]");
    while( s.isNotEmpty())
    {
        String x=s.upToFirstOccurrenceOf(",",false,false);
        s=s.fromFirstOccurrenceOf(",",false,false);
        String y=s.upToFirstOccurrenceOf(",",false,false);
        s=s.fromFirstOccurrenceOf(",",false,false);
        //std::cout<<"x="<<x.toUTF8()<<" y="<<y.toUTF8()<<std::endl;
        XYMapComponent* element=new XYMapComponent(0,elements_.size()*24);
        element->setInputValues(x,y);
        elements_.push_back(element);
        addAndMakeVisible(element);
        setSize(370,elements_.size()*24);
    }

    mapping_=makeMappingString();
    editor_->postCommandMessage(1);
}

void MappingPanel::clearMapping()
{
    mapping_=("[]");
    doDisplayMapping(mapping_);
    atom_->set_value(mapping_);
}

void MappingPanel::doDisplayMapping(String mapping)
{
    for(std::list<XYMapComponent*>::iterator i=elements_.begin();i!=elements_.end();++i)
    {
        (*i)->removing=true;
    }

    elements_.clear();
    deleteAllChildren();
    if(mapping.equalsIgnoreCase("[]"))
    {
        elements_.push_back(new XYMapComponent(0,0));
        setSize(370,elements_.size()*24);
        for(std::list<XYMapComponent*>::const_iterator i=elements_.begin();i!=elements_.end();++i)
        {
            addAndMakeVisible(*i);
        }
    }
    else
    {
        String s=mapping.removeCharacters("[]");
        while( s.isNotEmpty())
        {
            String xin=s.upToFirstOccurrenceOf(",",false,false);
            s=s.fromFirstOccurrenceOf(",",false,false);
            String yin=s.upToFirstOccurrenceOf(",",false,false);
            s=s.fromFirstOccurrenceOf(",",false,false);
            //std::cout<<"xin="<<xin.toUTF8()<<" yin="<<yin.toUTF8()<<std::endl;

            String xout=s.upToFirstOccurrenceOf(",",false,false);
            s=s.fromFirstOccurrenceOf(",",false,false);
            String yout=s.upToFirstOccurrenceOf(",",false,false);
            s=s.fromFirstOccurrenceOf(",",false,false);
            //std::cout<<"xout="<<xout.toUTF8()<<" yout="<<yout.toUTF8()<<std::endl;

            XYMapComponent* element=new XYMapComponent(0,elements_.size()*24);
            element->setInputValues(xin,yin);
            element->setOutputValues(xout,yout);
            elements_.push_back(element);
            addAndMakeVisible(element);
            setSize(370,elements_.size()*24);
        }
    }
    editor_->postCommandMessage(0);

}

void MappingPanel::displayMapping(String mapping)
{
//    std::cout<<"MappingPanel:displayMapping displayedMapping="<<std::string(mapping_.toUTF8())<<" eigend mapping="<<std::string(mapping.toUTF8())<<std::endl;
    if (!mapping.equalsIgnoreCase(mapping_))
    {
        doDisplayMapping(mapping);
    }
}

void MappingPanel::revertMapping(String mapping)
{
//    std::cout<<"MappingPanel:revertMapping displayedMapping="<<std::string(makeDisplayedMappingString().toUTF8())<<" eigend mapping="<<std::string(mapping.toUTF8())<<std::endl;
    if (!mapping.equalsIgnoreCase(makeDisplayedMappingString()))
    {
        doDisplayMapping(mapping);
    }
}



//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MappingPanel" componentName=""
                 parentClasses="public Component" constructorParams="Atom* atom, String mapping, Component* editor"
                 variableInitialisers="atom_(atom), mapping_(mapping), editor_(editor)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="370" initialHeight="1000">
  <BACKGROUND backgroundColour="ffffff"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
