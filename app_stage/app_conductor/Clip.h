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

#ifndef __CLIP__
#define __CLIP__
 
#include "juce.h"

class Clip
{
    public:
        Clip(XmlElement* e);
        ~Clip(){};
        String getName();
//        String getLabelText();
        XmlElement* toXml();
        virtual String getCategory();
        String getId();
    private:
        XmlElement* xml_;

};

class SceneClip: public Clip
{
    public:
        SceneClip(XmlElement* e);
        ~SceneClip(){};
        virtual String getCategory();
};

class ArrangementClip: public Clip
{
    public:
        ArrangementClip(XmlElement* e);
        ~ArrangementClip(){};
        virtual String getCategory();
};

#endif
