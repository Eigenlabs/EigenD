
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

#include "rig_connector.h"
#include <memory>

struct rig::connector_t::impl_t: piw::client_t
{
    impl_t(rig::connector_t *r, piw::server_t *parent, unsigned index, const piw::d2d_nb_t &filter): connector_(r), parent_(parent), index_(index), filter_(filter), output_(PLG_SERVER_TRANSIENT|PLG_SERVER_RO), bridge_(0)
    {
        parent_->child_add(index_,&output_);
    }

    ~impl_t()
    {
        output_.close_server();
        close_client();
    }

    void client_opened()
    {
        piw::client_t::client_opened();
        populate();

        output_.set_flags(get_host_flags());
        output_.set_data(get_data());

        if((get_host_flags()&PLG_SERVER_FAST)!=0)
        {
            bridge_ = new piw::fastdata_t();
            piw::tsd_fastdata(bridge_);
            output_.set_source(bridge_);
            set_sink(bridge_);
        }
    }

    void close_client()
    {
        piw::client_t::close_client();

        output_.set_flags(PLG_SERVER_TRANSIENT|PLG_SERVER_RO);
        output_.set_data(piw::data_t());

        if(bridge_)
        {
            clear_sink();
            output_.clear_source();
            bridge_->close_fastdata();
            delete bridge_;
            bridge_ = 0;
        }

        std::map<unsigned char, impl_t *>::iterator ci;

        while((ci = children_.begin()) != children_.end())
        {
            impl_t *p = ci->second;
            children_.erase(ci);
            delete p;
        }
    }

    void client_tree()
    {
        piw::client_t::client_tree();
        populate();
    }

    void client_data(const piw::data_t &d)
    {
        output_.set_data(get_data());
    }

    void populate()
    {
        unsigned char i = enum_child(0);

        while(i)
        {
            if(!child_get(&i,1))
            {
                std::auto_ptr<impl_t> p(new impl_t(connector_,&output_,i,filter_));
                child_add(i, p.get());
                children_[i] = p.release();
            }

            i = enum_child(i);
        }

        std::map<unsigned char, impl_t *>::iterator ci, ce;

    prune:

        ci = children_.begin();
        ce = children_.end();

        for(; ci != ce; ++ci)
        {
            i = ci->first;

            if(!child_get(&i,1))
            {
                impl_t *p = ci->second;
                children_.erase(ci);
                delete p;
                goto prune;
            }
        }
    }

    rig::connector_t *connector_;
    piw::server_t *parent_;
    unsigned index_;
    piw::d2d_nb_t filter_;
    std::map<unsigned char, impl_t *> children_;
    piw::server_t output_;
    piw::fastdata_t *bridge_;
};

rig::connector_t::connector_t(rig::output_t *parent,unsigned index, const piw::d2d_nb_t &filter): piw::client_t(PLG_CLIENT_CLOCK)
{
    impl_ = new impl_t(this,parent,index,filter);
}

rig::connector_t::~connector_t()
{
    close_client();
    delete impl_;
}

void rig::connector_t::client_clock()
{
    piw::client_t::client_clock();
}

void rig::connector_t::client_opened()
{
    piw::client_t::client_opened();
    clone(impl_);
}

void rig::connector_t::close_client()
{
    piw::client_t::close_client();
    impl_->close_client();
}

int rig::connector_t::gc_traverse(void *v, void *a) const
{
    return impl_->filter_.gc_traverse(v,a);
}

int rig::connector_t::gc_clear()
{
    pic::indirect_clear(impl_->filter_);
    return 0;
}

rig::output_t::output_t(): piw::server_t(PLG_SERVER_RO|PLG_SERVER_TRANSIENT|PLG_SERVER_RTRANSIENT)
{
}

rig::output_t::~output_t()
{
}
