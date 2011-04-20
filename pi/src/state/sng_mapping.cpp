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

#include "sng_mapping.h"
#include <picross/pic_log.h>

void pi::state::mapping_t::add(const char *from, const char *to)
{
    mapping_.insert(std::make_pair(from,to));
}

std::string pi::state::mapping_t::render() const
{
    std::ostringstream s;

    for(std::map<std::string,std::string>::const_iterator m=mapping_.begin(); m!=mapping_.end(); m++)
    {
        s << m->first << "->" << m->second << ' ';
    }

    return s.str();
}

piw::data_t pi::state::mapping_t::substitute(const piw::data_t &data) const
{
    if(data.is_string())
    {
        return substitute_string__(data);
    }

    if(data.is_dict())
    {
        return substitute_dict__(data);
    }

    return data;
}

std::string pi::state::mapping_t::substitute_string(const std::string &str) const
{
    std::string s2(str);

    if(substitute_stdstr__(s2))
    {
        return s2;
    }
    else
    {
        return str;
    }
}

piw::data_t pi::state::mapping_t::substitute_dict__(const piw::data_t &data) const
{
    piw::data_t d2 = piw::dictnull(data.time());
    unsigned l = data.as_dict_nkeys();

    for(unsigned i=0;i<l;i++)
    {
        piw::data_t v = data.as_dict_value(i);
        std::string k = data.as_dict_key(i);
        v = substitute(v);
        d2 = dictset(d2,k,v);
    }

    return d2;
}

piw::data_t pi::state::mapping_t::substitute_string__(const piw::data_t &data) const
{
    std::string s(data.as_string());

    if(substitute_stdstr__(s))
    {
        return piw::makestring(s,data.time());
    }
    else
    {
        return data;
    }
}

bool pi::state::mapping_t::substitute_stdstr__(std::string &s) const
{
    std::map<std::string,std::string>::const_iterator m;
    std::string::size_type i,k;
    bool rep = false;
    std::string::size_type j=0;

    for(;;)
    {

        i=s.find('<',j);
        k=s.find('|',j);

        if(i==std::string::npos && k==std::string::npos)
        {
            break;
        }

        if(i==std::string::npos)
        {
            i=k;
        }
        else
        {
            if(k!=std::string::npos && k<i)
            {
                i=k;
            }
        }

        char ch = s[i];
        bool addr;

        if(ch=='<')
        {
            ch='>';
            addr=true;
        }
        else
        {
            addr=false;
        }

        if((k=s.find(ch,i+1)) == std::string::npos)
        {
            j=i+1;
            continue;
        }

        k++;

        if((m=mapping_.find(s.substr(i,k-i)))!=mapping_.end())
        {
            s.replace(i,k-i,m->second);
            j=i+m->second.length();
            rep=true;
        }
        else
        {
            if(exclusive_ && addr)
            {
                s.replace(i,k-i,"<>");
                j=i+2;
                rep=true;
            }
            else
            {
                j=k;
            }
        }
    }

    return rep;
}

