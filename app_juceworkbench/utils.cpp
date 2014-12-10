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

#include "utils.h"

//  XXX put these convertion functions somewhere else
String stdToJuceString(const std::string& stdString)
{
    return String::fromUTF8(stdString.c_str());
}

std::string juceToStdString(const String& juceString)
{
    return std::string(juceString.toUTF8());
}

bool sortChildren(NamedId a1, NamedId a2)
{
    return sortNames(a1.name,a2.name);
}

bool sortNames(String a, String b)
{
    return ((renumber(a)).compare(renumber(b)))<0;
}

String renumber(String s)
{
    if(s.isEmpty())
    {
        return "zzzzzz";
    }
    int idigit=s.indexOfAnyOf("0123456789",0,true);
    if(idigit<0)
    {
        return s;
    }
    else
    {
        String namepart=s.substring(0,idigit);
        return namepart+((String("000000")+s.substring(idigit)).getLastCharacters(4));
    }
}
