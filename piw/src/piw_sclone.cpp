
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

#include <piw/piw_sclone.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>

#include <map>

namespace {
    struct first_filter_t
    {
        unsigned s_;

        first_filter_t(unsigned s): s_(s) {}

        first_filter_t(const first_filter_t &o): s_(o.s_) {}
        bool operator==(const first_filter_t &o) const { return s_==o.s_; }
        first_filter_t &operator=(const first_filter_t &o) { s_=o.s_; return *this; }

        bool operator()(const piw::data_t &x) const
        {
            return x.is_path() && x.as_pathlen()>0 && x.as_path()[0]==s_;
        }
    };

    struct sclone_wire_t;

    struct sclone_wire_t: piw::wire_t
    {
        sclone_wire_t(piw::sclone_t::impl_t *p, const piw::event_data_source_t &);
        ~sclone_wire_t() { invalidate(); }

        void add_output(unsigned name, piw::root_ctl_t *root)
        {
            std::map<unsigned,piw::wire_ctl_t *>::iterator oi;
            piw::wire_ctl_t *w;

            if((oi=sclones_.find(name))==sclones_.end())
            {
                w = new piw::wire_ctl_t();
                sclones_.insert(std::make_pair(name,w));
            }
            else
            {
                w = oi->second;
            }

            root->connect_wire(w,source_);
        }

        void del_outputs()
        {
            std::map<unsigned,piw::wire_ctl_t *>::iterator oi;

            while((oi=sclones_.begin())!=sclones_.end())
            {
                oi->second->disconnect();
                sclones_.erase(oi);
            }
        }

        void del_output(unsigned name)
        {
            std::map<unsigned,piw::wire_ctl_t *>::iterator oi;

            if((oi=sclones_.find(name))!=sclones_.end())
            {
                oi->second->disconnect();
                sclones_.erase(name);
            }
        }

        void wire_closed() { delete this; }
        void invalidate();

        piw::sclone_t::impl_t *parent_;
        piw::event_data_source_t source_;
        std::map<unsigned,piw::wire_ctl_t*> sclones_;
    };

    struct sclone_root_ctl_t: piw::root_ctl_t
    {
        sclone_root_ctl_t(const piw::d2b_t &f): filter_(f) {}

        piw::d2b_t filter_;
    };
};

struct piw::sclone_t::impl_t: root_t
{
    impl_t(): root_t(0) {}
    ~impl_t() { invalidate(); }

    void invalidate()
    {
        std::map<piw::data_t,sclone_wire_t *>::iterator ci;

        while((ci=children_.begin())!=children_.end())
        {
            delete ci->second;
        }
    }

    piw::wire_t *root_wire(const piw::event_data_source_t &es)
    {
        std::map<piw::data_t,sclone_wire_t *>::iterator ci;

        if((ci=children_.find(es.path()))!=children_.end())
        {
            delete ci->second;
        }

        sclone_wire_t *w = new sclone_wire_t(this,es);

        std::map<unsigned,sclone_root_ctl_t *>::iterator oi;
        for(oi=sclones_.begin(); oi!=sclones_.end(); oi++)
        {
            if(oi->second->filter_(es.path()))
            {
                w->add_output(oi->first,oi->second);
            }
        }

        return w;
    }

    void add_output(unsigned name, const piw::cookie_t cookie, const piw::d2b_t &filter)
    {
        del_output(name);

        sclone_root_ctl_t *r;

        PIC_ASSERT(sclones_.find(name)==sclones_.end());

        r = new sclone_root_ctl_t(filter);
        sclones_.insert(std::make_pair(name,r));
        r->set_clock(get_clock());
        r->set_latency(get_latency());
        r->connect(cookie);

        std::map<piw::data_t,sclone_wire_t *>::iterator ci;

        for(ci=children_.begin(); ci!=children_.end(); ci++)
        {
            if(!filter(ci->first))
            {
                ci->second->add_output(name,r);
            }
        }
    }

    void del_outputs()
    {
        std::map<unsigned,sclone_root_ctl_t *>::iterator oi;

        while((oi=sclones_.begin())!=sclones_.end())
        {
            oi->second->disconnect();
            sclones_.erase(oi);
        }

        std::map<piw::data_t,sclone_wire_t *>::iterator ci;

        for(ci=children_.begin(); ci!=children_.end(); ci++)
        {
            ci->second->del_outputs();
        }
    }

    void del_output(unsigned name)
    {
        std::map<unsigned,sclone_root_ctl_t *>::iterator oi;

        if((oi=sclones_.find(name))!=sclones_.end())
        {
            oi->second->disconnect();
            sclones_.erase(name);
        }

        std::map<piw::data_t,sclone_wire_t *>::iterator ci;

        for(ci=children_.begin(); ci!=children_.end(); ci++)
        {
            ci->second->del_output(name);
        }
    }

    void root_closed() { invalidate(); }
    void root_opened() { root_clock(); root_latency(); }

    void root_clock()
    {
        std::map<unsigned,sclone_root_ctl_t *>::iterator oi;

        for(oi=sclones_.begin(); oi!=sclones_.end(); oi++)
        {
            oi->second->set_clock(get_clock());
        }
    }

    void root_latency()
    {
        std::map<unsigned,sclone_root_ctl_t *>::iterator oi;

        for(oi=sclones_.begin(); oi!=sclones_.end(); oi++)
        {
            oi->second->set_latency(get_latency());
        }
    }

    int gc_traverse(void *v, void *a) const
    {
        std::map<unsigned,sclone_root_ctl_t *>::const_iterator oi;
        int r;

        for(oi=sclones_.begin(); oi!=sclones_.end(); oi++)
        {
            if((r=oi->second->filter_.gc_traverse(v,a))!=0)
            {
                return r;
            }
        }

        return 0;
    }

    int gc_clear()
    {
        std::map<unsigned,sclone_root_ctl_t *>::iterator oi;

        for(oi=sclones_.begin(); oi!=sclones_.end(); oi++)
        {
            oi->second->filter_.gc_clear();
        }

        return 0;
    }

    std::map<piw::data_t, sclone_wire_t *> children_;
    std::map<unsigned,sclone_root_ctl_t*> sclones_;
};

sclone_wire_t::sclone_wire_t(piw::sclone_t::impl_t *p, const piw::event_data_source_t &es): parent_(p), source_(es)
{
    parent_->children_.insert(std::make_pair(es.path(),this));
}

void sclone_wire_t::invalidate()
{
    if(parent_)
    {
        del_outputs();
        parent_->children_.erase(source_.path());
        parent_=0;
    }
}

piw::cookie_t piw::sclone_t::cookie()
{
    return piw::cookie_t(_root);
}

piw::sclone_t::sclone_t(): _root(new impl_t())
{
}

piw::sclone_t::~sclone_t()
{
    delete _root;
}

void piw::sclone_t::set_filtered_output(unsigned name, const piw::cookie_t &cookie, const piw::d2b_t &filter)
{
    _root->add_output(name,cookie,filter);
}

void piw::sclone_t::set_output(unsigned name, const piw::cookie_t &cookie)
{
    _root->add_output(name,cookie,piw::d2b_t());
}

void piw::sclone_t::clear_output(unsigned name)
{
    _root->del_output(name);
}

piw::d2b_t piw::first_filter(unsigned s)
{
    return piw::d2b_t::callable(first_filter_t(s));
}

int piw::sclone_t::gc_traverse(void *v, void *a) const
{
    return _root->gc_traverse(v,a);
}

int piw::sclone_t::gc_clear()
{
    return _root->gc_clear();
}
