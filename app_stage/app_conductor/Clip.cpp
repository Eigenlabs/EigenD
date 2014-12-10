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

#include "Clip.h"
 
SceneClip::SceneClip(XmlElement* e):Clip(e)
{
}

String  SceneClip::getCategory()
{
    return "Scene";
}

ArrangementClip::ArrangementClip(XmlElement* e):Clip(e)
{
}

String  ArrangementClip::getCategory()
{
    return "Arrangement";
}


Clip::Clip(XmlElement* e)
{
    xml_=new XmlElement(*e);
}

String Clip::getName()
{
    return xml_->getStringAttribute("name");
}

String Clip::getId()
{
    return xml_->getStringAttribute("uuid");
}

//String Clip::getLabelText()
//{
//    String labelText=String::empty;
//    XmlElement* c= xml_->getChildByName("Label");
//    while (c!=0)
//    {
//        if(c->hasAttribute("name"))
//        {
//            if(labelText.isNotEmpty())
//            {
//                labelText=labelText+"\n";
//            }
//            labelText=labelText+c->getStringAttribute("name");
//        }
//
//        c=c->getNextElementWithTagName("Label");
//    }
//
//    return labelText;
//}
//

XmlElement* Clip::toXml()
{
    return new XmlElement(*xml_);
}

String Clip::getCategory()
{
    XmlElement* c=xml_->getChildByName("Tag");
    while (c!=0)
    {
        if(c->hasAttribute("name") && c->hasAttribute("category"))
        {
            if(c->getStringAttribute("category").equalsIgnoreCase("Type"))
            {
                return c->getStringAttribute("name");
            }
        }
        c=c->getNextElementWithTagName("Tag");
    }
    return String::empty;
}
