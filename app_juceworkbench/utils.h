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

#ifndef __UTILS__
#define __UTILS__

#include "juce.h"

class NamedId
{
public:
    NamedId(String id, String name);
    ~NamedId();
    String id;
    String name;
};



String stdToJuceString(const std::string& stdString);
std::string juceToStdString(const String& juceString);
bool sortChildren(NamedId  s1, NamedId  s2);
bool sortNames(String  s1, String  s2);
String renumber(String s);

#endif
