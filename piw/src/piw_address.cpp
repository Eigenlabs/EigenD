
/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include <piw/piw_address.h>
#include <pibelcanto/link.h>
#include <piembedded/pie_parse.h>
#include <piembedded/pie_string.h>

#include <cstring>
#include <string>
#include <sstream>

std::string piw::address2server(const std::string &address)
{
    char server[BCTLINK_GROUP_SIZE+1];
    unsigned char path[255];
    pie_strreader_t r;

    pie_readstr_init(&r,address.c_str(),address.length());

    if(pie_parseaddress(server,BCTLINK_GROUP_SIZE+1,path,255,pie_readstr,&r)<0)
    {
        PIC_THROW("can't parse address");
    }

    return std::string(server);
}

unsigned piw::address2pathbuf(const std::string &address, unsigned char path[255])
{
    char server[BCTLINK_GROUP_SIZE+1];
    pie_strreader_t r;
    int x;

    pie_readstr_init(&r,address.c_str(),address.length());

    if((x=pie_parseaddress(server,BCTLINK_GROUP_SIZE+1,path,255,pie_readstr,&r))<0)
    {
        PIC_THROW("can't parse address");
    }

    return x;
}
