
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

#include <piw/piw_dumper.h>

static std::string type2str(unsigned char type)
{
    switch(type)
    {
        case BCTVTYPE_NULL:     return "null"; break;
        case BCTVTYPE_PATH:     return "path"; break;
        case BCTVTYPE_STRING:   return "string"; break;
        case BCTVTYPE_DOUBLE:   return "double"; break;
        case BCTVTYPE_FLOAT:    return "float"; break;
        case BCTVTYPE_INT:      return "int"; break;
        case BCTVTYPE_BOOL:     return "bool"; break;
        case BCTVTYPE_BLOB:     return "blob"; break;
    }

    return "unknown";
}

void piw::dump_client(piw::client_t *client, unsigned flags, bool log)
{
    unsigned char name;
    piw::client_t child;
    piw::fastdata_t fastdata;
    pic::msg_t msg;

    if(!client->open()) return;

    if(flags & PIW_DUMP_ADDRESS)
    {
        msg << client->servername() << '#';
    }

    msg << client->path();
    
    if(flags & PIW_DUMP_FLAGS)
    {
        if(client->host_flags & PLG_SERVER_RO)
        {
            msg << "/ro";
        }
        else
        {
            msg << "/rw";
        }

        if(client->host_flags & PLG_SERVER_LIST)
        {
            msg << "/list";
        }

        if(client->host_flags & PLG_SERVER_RTRANSIENT)
        {
            msg << "/rtx";
        }

        if(client->host_flags & PLG_SERVER_TRANSIENT)
        {
            msg << "/tx";
        }
    }

    if(flags & (PIW_DUMP_SDATA|PIW_DUMP_SSCALAR|PIW_DUMP_SVECTOR|PIW_DUMP_STIME|PIW_DUMP_STYPE))
    {
        piw::data_t sd = client->get_data();

        if(flags & PIW_DUMP_STYPE)
        {
            msg << ' ' << type2str(sd.type());
        }

        if(flags & PIW_DUMP_STIME)
        {
            msg << " @" << sd.time();
        }

        if(flags & PIW_DUMP_SDATA)
        {
            msg << ' ' << sd;
        }

        if((flags&PIW_DUMP_SSCALAR) && !sd.is_null())
        {
            msg << " <" << sd.as_array_lbound() << ':' << sd.as_array_rest() << ':' << sd.as_array_ubound() << "> " << sd.as_norm();
        }
    }

    if(flags&PIW_DUMP_ID)
    {
        piw::data_nb_t id;

        if(client->host_flags & PLG_SERVER_FAST)
        {
            tsd_fastdata(&fastdata);
            client->set_sink(&fastdata);
            id = fastdata.current(true);
        }

        if(id.is_null())
        {
            msg << " id=null";
        }
        else
        {
            msg << " id=" << id;
        }
    }

    if(flags & (PIW_DUMP_FDATA|PIW_DUMP_FSCALAR|PIW_DUMP_FVECTOR|PIW_DUMP_FTIME|PIW_DUMP_FTYPE))
    {
        piw::data_nb_t fd;

        if(client->host_flags & PLG_SERVER_FAST)
        {
            tsd_fastdata(&fastdata);
            client->set_sink(&fastdata);
            fd = fastdata.current(false);
        }

        if(flags & PIW_DUMP_FTYPE)
        {
            msg << ' ' << type2str(fd.type());
        }

        if(flags & PIW_DUMP_FTIME)
        {
            msg << " @" << fd.time();
        }

        if(flags & PIW_DUMP_FDATA)
        {
            msg << ' ' << fd;
        }

        if((flags&PIW_DUMP_FSCALAR) && !fd.is_null())
        {
            msg << " <" << fd.as_array_lbound() << ':' << fd.as_array_rest() << ':' << fd.as_array_ubound() << "> " << fd.as_norm();
        }
    }

    if(log)
    {
        pic::log(msg);
    }
    else
    {
        pic::print(msg);
    }

    for(name=client->enum_child(0); name>0; name=client->enum_child(name))
    {
        client->child_add(&name,1,&child);
        dump_client(&child,flags,log);
        child.close_client();
    }
}
