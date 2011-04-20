
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

#include <picross/pic_config.h>

#ifndef PI_WINDOWS
#include <unistd.h>
#else
#include <io.h>
#endif


#include <piembedded/pie_string.h>

#include <piagent/pia_scaffold.h>
#include <piw/piw_client.h>
#include <piw/piw_tsd.h>
#include <piw/piw_fastdata.h>

#include <string.h>

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

static void dump(piw::client_t *c)
{
    unsigned char name;
    piw::client_t child;
    piw::fastdata_t fastdata;

    if(!c->open()) return;

    std::cout << c->path() << ' ' << type2str(c->get_data().type());

    if(c->host_flags & PLG_SERVER_RO)
    {
        std::cout << "/ro";
    }
    else
    {
        std::cout << "/rw";
    }

    if(c->host_flags & PLG_SERVER_LIST)
    {
        std::cout << "/list";
    }

    if(c->host_flags & PLG_SERVER_TRANSIENT)
    {
        std::cout << "/tx";
    }

    std::cout << ' ' << piw::fullprinter_t<piw::data_t>(c->get_data());

    if(c->host_flags & PLG_SERVER_FAST)
    {
        tsd_fastdata(&fastdata);
        c->set_sink(&fastdata);
        std::cout << " id=" << piw::fullprinter_t<piw::data_nb_t>(fastdata.current(true));
        std::cout << " (" << piw::fullprinter_t<piw::data_nb_t>(fastdata.current(false)) << ")";
    }

    std::cout << '\n';

    for(name=c->enum_child(0); name>0; name=c->enum_child(name))
    {
        c->child_add(&name,1,&child);
        dump(&child);
        child.close_client();
    }

}

class bcat_client_t: public piw::client_t
{
    public:
        bcat_client_t(bool once): piw::client_t(PLG_CLIENT_SYNC), _once(once)
        {
        }

        void client_sync()
        {
            client_t::client_sync();

            std::cout << "==============================\n";

            dump(this);

            if(_once)
            {
                close_client();
            }
        }

    private:
        bool _once;
};

int main0(int ac, char **av)
{
    if(ac!=2)
    {
        PIC_THROW("usage: bcat server\n");
        return -1;
    }

    pia::scaffold_mt_t manager("bcatfast",1,pic::f_string_t(),pic::f_string_t(),false,false);
    pia::context_t entity = manager.context(pic::status_t(),pic::f_string_t());
	piw::tsd_setcontext(entity.entity());

    bcat_client_t client(true);
    piw::tsd_client(av[1],&client);
    entity.release();
    manager.wait();
    return 0;
}

int main(int ac, char **av)
{
    try
    {
        main0(ac,av);
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return -1;
    }
    catch(...)
    {
        std::cerr << "unknown exception\n";
        return -1;
    }

    return 0;
}
