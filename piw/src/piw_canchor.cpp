
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

#include <piw/piw_canchor.h>
#include <piw/piw_tsd.h>
#include <piw/piw_address.h>
#include <picross/pic_error.h>
#include <picross/pic_log.h>

struct piw::canchor_t::impl_t: piw::client_t, piw::thing_t
{
    impl_t();
    ~impl_t();

    bool set_address(const std::string &a);
    std::string get_address() { return address_; }
    void set_client(client_t *);
    void set_slow_client(client_t *);
    void client_sync();
    void client_closed();
    void thing_trigger_slow();
    void synchronise();

    bool address_ok();

    pic::weak_t<client_t> client_;
    std::string address_;
    std::string server_;
    unsigned char path_[255];
    unsigned pathlen_;
    bool fast_;
};

piw::canchor_t::impl_t::impl_t(): piw::client_t(PLG_CLIENT_SYNC)
{
    tsd_thing(this);
    address_="";
}

piw::canchor_t::impl_t::~impl_t()
{
    if(client_.isvalid() && client_->open()) client_->close_client();
    close_client();
    tracked_invalidate();
}

bool piw::canchor_t::impl_t::address_ok()
{
    if(server_.length()>2 && server_[0]=='<' && server_[server_.length()-1]=='>')
    {
        return true;
    }

    return false;
}

bool piw::canchor_t::impl_t::set_address(const std::string &a)
{
    if(address_ != a)
    {
        address_=a;
        if(client_.isvalid() && client_->open()) client_->close_client();
        close_client();
        server_=address2server(a);
        pathlen_=address2pathbuf(a,path_);
        synchronise();
    }

    return address_ok();
}

void piw::canchor_t::impl_t::set_slow_client(client_t *c)
{
    if(client_.isvalid() && client_->open()) client_->close_client();
    client_=c;
    fast_=false;
    synchronise();
}

void piw::canchor_t::impl_t::set_client(client_t *c)
{
    if(client_.isvalid() && client_->open()) client_->close_client();
    client_=c;
    fast_=true;
    synchronise();
}

void piw::canchor_t::impl_t::client_closed()
{
    piw::client_t::client_closed();
    trigger_slow();
}

void piw::canchor_t::impl_t::thing_trigger_slow()
{
    piw::thing_t::thing_trigger_slow();
    synchronise();
}

void piw::canchor_t::impl_t::client_sync()
{
    piw::client_t::client_sync();
    synchronise();
}

void piw::canchor_t::impl_t::synchronise()
{
    if(piw::tsd_killed())
    {
        return;
    }

    if(!client_.isvalid())
    {
        return;
    }

    if(!address_ok())
    {
        if(client_->open()) client_->close_client();
        close_client();
        return;
    };

    if(!piw::client_t::open())
    {
        tsd_client(server_.c_str(),this,fast_);
        return;
    }

    if(client_->open())
    {
        return;
    }

    if(child_exists_str(path_, pathlen_))
    {
        child_add(path_,pathlen_,client_.ptr());
    }
}

piw::canchor_t::canchor_t()
{
    impl_ = new piw::canchor_t::impl_t();
}

piw::canchor_t::~canchor_t()
{
    delete impl_;
}

bool piw::canchor_t::set_address(const piw::data_nb_t &a)
{
    std::string s(a.as_string());
    return impl_->set_address(s);
}

std::string piw::canchor_t::get_address()
{
    return impl_->get_address();
}

bool piw::canchor_t::set_address_str(const std::string &a)
{
    return impl_->set_address(a);
}

void piw::canchor_t::set_slow_client(client_t *c)
{
    impl_->set_slow_client(c);
}

void piw::canchor_t::set_client(client_t *c)
{
    impl_->set_client(c);
}

piw::subcanchor_t::subcanchor_t(): piw::client_t(PLG_CLIENT_SYNC), pathlen_(-1)
{
    tsd_thing(this);
}

piw::subcanchor_t::~subcanchor_t()
{
    tracked_invalidate();
    if(client_.isvalid() && client_->open()) client_->close_client();
    close_client();
}

bool piw::subcanchor_t::clear_path()
{
    if(pathlen_>=0)
    {
        pathlen_=-1;
        if(client_.isvalid() && client_->open()) client_->close_client();
        close_client();
        client_sync();
    }

    return false;
}

bool piw::subcanchor_t::set_path(const std::string &a)
{
    int l = a.length();

    if(l != pathlen_ || path_ != a)
    {
        path_=a;
        pathlen_=l;
        if(client_.isvalid() && client_->open()) client_->close_client();
        close_client();
        client_sync();
    }

    return (pathlen_>=0);
}

void piw::subcanchor_t::set_client(client_t *c)
{
    if(client_.isvalid() && client_->open()) client_->close_client();
    client_=c;
    synchronise();
}

void piw::subcanchor_t::close_client()
{
    piw::client_t::close_client();
    if(client_.isvalid() && client_->open()) client_->close_client();
}

void piw::subcanchor_t::thing_trigger_slow()
{
    piw::thing_t::thing_trigger_slow();
    synchronise();
}

void piw::subcanchor_t::client_sync()
{
    piw::client_t::client_sync();
    synchronise();
}

void piw::subcanchor_t::synchronise()
{
    if(!client_.isvalid())
    {
        return;
    }

    if(pathlen_<0)
    {
        if(client_->open()) client_->close_client();
        close_client();
        return;
    };

    if(!piw::client_t::open())
    {
        return;
    }

    if(!client_->open())
    {
        if(child_exists_str((const unsigned char *)path_.c_str(), pathlen_))
        {
            child_add((const unsigned char *)path_.c_str(),pathlen_,client_.ptr());
        }
    }
}
