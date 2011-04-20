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

#include "sng_mirror.h"

#define KICK_TIMER 180000
#define VERBOSE false

pi::state::worker_t::worker_t(worker_t *p, unsigned char name, const noderef_t &sink, unsigned cflags): piw::client_t(cflags), idset_(false), parent_(p), name_(name), sink_(sink)
{
}

pi::state::worker_t::~worker_t()
{
    close_client();
}

void pi::state::worker_t::save_template(noderef_t sink, const mapref_t &mapping)
{
    unsigned n;

    populate();

    while((n=sink->enum_children(0)) != 0)
    {
        sink->erase_child(n);
    }

    piw::data_t d = mapping->substitute(get_data().make_normal());
    sink->set_data(d);

    for(std::map<unsigned char,worker_t *>::iterator i=_clients.begin(); i!=_clients.end(); i++)
    {
        i->second->save_template(sink->get_child(i->first),mapping);
    }
}


void pi::state::worker_t::start_save()
{
    if(sink_.isvalid())
    {
        if(writeable() || !parent_)
        {
            sink_->set_data(get_data().make_normal());
        }

        for(unsigned char n=sink_->enum_children(0); n!=0; n=sink_->enum_children(n))
        {
            if(!child_get(&n,1))
            {
                sink_->erase_child(n);
            }
        }
    }
}

void pi::state::worker_t::save()
{
    for(std::map<unsigned char,worker_t *>::iterator i=_clients.begin(); i!=_clients.end(); i++)
    {
        i->second->save();
    }

    start_save();
}

piw::term_t pi::state::worker_t::add_diff(int ord,noderef_t snap,const mapref_t &map)
{
    piw::term_t node;

    populate();

    if(rtransient())
    {
        return node;
    }

    if(writeable())
    {
        piw::data_t d = snap->get_data();

        if(map.isvalid())
        {
            d = map->substitute(d);
        }

        if(get_data().compare(d,false)!=0)
        {
            if(node.type()==PIW_TERM_NULL)
            {
                node = piw::term_t("n",0);
                node.append_arg(piw::term_t(piw::makelong(ord,0)));
            }

            node.append_arg(piw::term_t(d));
        }
    }

    if(!dynlist())
    {
        for(unsigned char n=snap->enum_children(0); n!=0; n=snap->enum_children(n))
        {
            std::map<unsigned char,worker_t *>::iterator i;
            piw::term_t cs;

            if((i=_clients.find(n))!=_clients.end())
            {
                cs=i->second->add_diff(n,snap->get_child(n),map);
            }

            if(cs.type()!=PIW_TERM_NULL)
            {
                if(node.type()==PIW_TERM_NULL)
                {
                    node = piw::term_t("n",0);
                    node.append_arg(piw::term_t(piw::makelong(ord,0)));
                }

                node.append_arg(cs);
            }
        }

        return node;
    }

    if(node.type()==PIW_TERM_NULL)
    {
        node = piw::term_t("n",0);
        node.append_arg(piw::term_t(piw::makelong(ord,0)));
    }

    bool delta = false;

    for(unsigned char n=snap->enum_children(0); n!=0; n=snap->enum_children(n))
    {
        std::map<unsigned char,worker_t *>::iterator i;
        piw::term_t cs;

        if((i=_clients.find(n))!=_clients.end())
        {
            cs=i->second->add_diff(n,snap->get_child(n),map);

            if(cs.type()!=PIW_TERM_NULL)
            {
                delta=true;
            }
            else
            {
                cs = piw::term_t("p",0);
                cs.append_arg(piw::term_t(piw::makelong(n,0)));
            }
        }
        else
        {
            cs=full_state(n,snap->get_child(n),map);
            delta = true;
        }

        node.append_arg(cs);
    }

    if(!delta)
    {
        std::map<unsigned char,worker_t *>::iterator i;
        for(i=_clients.begin(); i!=_clients.end(); i++)
        {
            unsigned char n = i->first;
            if(snap->enum_children(n-1)!=n)
            {
                delta = true;
                break;
            }
        }
    }

    if(!delta)
    {
        return piw::term_t();
    }

    return node;
}

piw::term_t pi::state::worker_t::full_state(int n, noderef_t snap, const mapref_t &map)
{
    piw::term_t node("n",0);

    populate();

    node.append_arg(piw::term_t(piw::makelong(n,0)));

    piw::data_t d(snap->get_data());

    if(map.isvalid())
    {
        d = map->substitute(d);
    }
    node.append_arg(piw::term_t(d));

    for(unsigned char c=snap->enum_children(0); c!=0; c=snap->enum_children(c))
    {
        node.append_arg(full_state(c,snap->get_child(c),map));
    }

    return node;
}

unsigned pi::state::worker_t::hflags() { return get_host_flags(); }
bool pi::state::worker_t::rtransient() { return (hflags()&PLG_SERVER_RTRANSIENT)!=0; }
bool pi::state::worker_t::transient() { return (hflags()&PLG_SERVER_TRANSIENT)!=0 || (hflags()&PLG_SERVER_RTRANSIENT)!=0; }
bool pi::state::worker_t::writeable() { return (hflags()&PLG_SERVER_RO)==0 && !transient(); }
bool pi::state::worker_t::dynlist() { return (hflags()&PLG_SERVER_LIST)!=0; }

void pi::state::worker_t::populate()
{
    if(rtransient())
    {
        return;
    }

    unsigned char n;
    std::map<unsigned char,worker_t *>::iterator i;

    for(n=enum_child(0); n!=0; n=enum_child(n))
    {
        if(!child_get(&n,1))
        {
            noderef_t cn;

            if(sink_.isvalid())
            {
                cn = sink_->get_child(n);
            }

            worker_t *c = new worker_t(this,n,cn,0);

            try
            {
                child_add(&n,1,c);
            }
            catch(...)
            {
                pic::logmsg() << "child node disappeared";
                delete c;
                continue;
            }

            _clients.insert(std::make_pair(n,c));
            c->myid();
        }
    }


restart:

    for(i=_clients.begin(); i!=_clients.end(); i++)
    {
        n = i->first;

        if(!child_get(&n,1))
        {
            try
            {
                delete i->second;
            }
            CATCHLOG()

            goto restart;
        }
    }
}

void pi::state::worker_t::client_tree()
{
    client_t::client_tree();
    populate();
}

void pi::state::worker_t::close_client()
{
    if(parent_)
    {
        parent_->_clients.erase(name_);
        parent_ = 0;

        if(sink_.isvalid())
        {
            sink_->erase();
        }
    }

    std::map<unsigned char,worker_t *>::iterator i;

    while((i=_clients.begin())!=_clients.end())
    {
        delete i->second;
    }

    client_t::close_client();

}

void pi::state::worker_t::client_data(const piw::data_t &d)
{
    client_t::client_data(d);

    if(writeable() || !parent_)
    {
        if(sink_.isvalid())
        {
            sink_->set_data(d.make_normal());
        }
    }
}

void pi::state::worker_t::client_opened()
{
    piw::client_t::client_opened();
    populate();
    myid();

    if(writeable() || !parent_)
    {
        if(sink_.isvalid())
        {
            sink_->set_data(get_data().make_normal());
        }
    }

}

std::string pi::state::worker_t::myid()
{
    if(idset_)
    {
        return id_;
    }

    if(open())
    {
        id_=id();
        idset_=true;
    }

    return "<closed>";
}

pi::state::manager_t::manager_t(const noderef_t &sink): worker_t(0,0,sink,PLG_CLIENT_SYNC), saving_(false)
{
}

pi::state::manager_t::~manager_t()
{
    tracked_invalidate();
}

void pi::state::manager_t::save_template(const noderef_t &sink, const mapref_t &mapping)
{
    PIC_ASSERT(mapping.isvalid());
    worker_t::save_template(sink,mapping);
}

void pi::state::manager_t::manager_checkpoint()
{
}

piw::term_t pi::state::manager_t::get_diff(const noderef_t &snap, const mapref_t &mapping)
{
    return add_diff(0,snap,mapping);
}

void pi::state::manager_t::client_sync()
{
    piw::client_t::client_sync();
    manager_checkpoint();

    if(!saving_)
    {
        saving_=true;
        worker_t::save();
    }
}

int pi::state::manager_t::gc_clear()
{
    piw::client_t::gc_clear();
    return 0;
}

int pi::state::manager_t::gc_traverse(void *v, void *a)
{
    int r;
    if((r=piw::client_t::gc_traverse(v,a))!=0) return r;
    return 0;
}
