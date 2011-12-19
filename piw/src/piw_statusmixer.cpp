
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

#include <piw/piw_tsd.h>
#include <piw/piw_status.h>
#include <piw/piw_thing.h>
#include <piw/piw_ufilter.h>
#include <picross/pic_ref.h>
#include <picross/pic_ilist.h>

namespace
{
    static inline int c2int(unsigned char *c)
    {
        unsigned long cx = (c[0]<<8) | (c[1]);

        if(cx>0x7fff)
        {
            return ((long)cx)-0x10000;
        }

        return cx;
    }

    struct receiver_t: piw::ufilterfunc_t, virtual pic::lckobject_t, pic::element_t<0>
    {
        receiver_t(piw::statusmixer_t::impl_t *root): root_(root), status_data_(), active_(false)
        {
        }

        ~receiver_t()
        {
        }

        void ufilterfunc_changed(piw::ufilterenv_t *, void *);
        void ufilterfunc_start(piw::ufilterenv_t *,const piw::data_nb_t &);
        void ufilterfunc_data(piw::ufilterenv_t *,unsigned,const piw::data_nb_t &);
        void ufilterfunc_end(piw::ufilterenv_t *, unsigned long long);

        piw::statusmixer_t::impl_t *root_;
        piw::dataholder_nb_t status_data_;
        bool active_;
    };
};

struct piw::statusmixer_t::impl_t: virtual pic::lckobject_t, piw::ufilterctl_t, piw::ufilter_t
{
    impl_t(const piw::cookie_t &output): piw::ufilter_t(this,piw::cookie_t(0)), output_(piw::change_nb_t(),0,output), autoupdate_(true)
    {
        output_.autosend(false);
    }

    ~impl_t()
    {
    }

    ufilterfunc_t *ufilterctl_create(const piw::data_t &path)
    {
        return new receiver_t(this);
    }
    
    void update_status()
    {
        output_.clear();

        receiver_t *r;

        for(r=receivers_.head(); r; r=receivers_.next(r))
        {
            if(!r->active_)
            {
                continue;
            }

            piw::data_nb_t rd = r->status_data_;
            if(rd.is_null())
            {
                continue;
            }

            unsigned char* rs = ((unsigned char*)rd.as_blob());
            unsigned rl = rd.as_bloblen();

            while(rl>=5)
            {
                int kr = c2int(&rs[0]);
                int kc = c2int(&rs[2]);
                bool kmus = rs[4]>>7;
                unsigned kv = rs[4]&0x7f;

                if(kv!=BCTSTATUS_OFF)
                {
                    unsigned os = output_.get_status(kmus,kr,kc);

                    if(os!=kv)
                    {
                        if(os && os!=BCTSTATUS_OFF)
                        {
                            kv = BCTSTATUS_MIXED;
                        }

                        output_.set_status(kmus,kr,kc,kv);
                    }
                }

                rs+=5;
                rl-=5;
            }
        }
        
        output_.send();
    }

    void add_receiver(receiver_t *r)
    {
        receivers_.append(r);
        if (autoupdate_)
        {
            update_status();
        }
    }

    void del_receiver(receiver_t *r)
    {
        receivers_.remove(r);
        if (autoupdate_)
        {
            update_status();
        }
    }

    static int update__(void *self_, void *arg_)
    {
        impl_t *self = (impl_t *)self_;
        self->update_status();
        return 1;
    }

    static int autoupdate__(void *self_, void *autoupdate_)
    {
        bool autoupdate = *(bool *)autoupdate_;
        impl_t *self = (impl_t *)self_;

        self->autoupdate_=autoupdate;
        return 1;
    }

    void update()
    {
        piw::tsd_fastcall(update__,this,0);
    }

    void autoupdate(bool autoupdate)
    {
        piw::tsd_fastcall(autoupdate__,this,&autoupdate);
    }

    unsigned long long ufilterctl_thru() { return 0; }
    unsigned long long ufilterctl_inputs() { return SIG1(1); }
    unsigned long long ufilterctl_outputs() { return 0; }

    piw::statusbuffer_t output_;
    pic::ilist_t<receiver_t> receivers_;
    bool autoupdate_;
};

piw::statusmixer_t::statusmixer_t(const piw::cookie_t &output): root_(new impl_t(output))
{
}

piw::statusmixer_t::~statusmixer_t()
{
    delete root_;
}

void piw::statusmixer_t::update()
{
    root_->update();
}

void piw::statusmixer_t::autoupdate(bool autoupdate)
{
    root_->autoupdate(autoupdate);
}

piw::cookie_t piw::statusmixer_t::cookie()
{
    return root_->cookie();
}

void receiver_t::ufilterfunc_changed(piw::ufilterenv_t *, void *)
{
}

void receiver_t::ufilterfunc_start(piw::ufilterenv_t *e,const piw::data_nb_t &id)
{
    piw::data_nb_t d;

    if(e->ufilterenv_latest(1,d,id.time()))
    {
        status_data_.set_nb(d);
    }
 
    active_=true;
    root_->add_receiver(this);
}

void receiver_t::ufilterfunc_data(piw::ufilterenv_t *,unsigned s,const piw::data_nb_t &d)
{
    if(active_ && s==1)
    {
        status_data_.set_nb(d);
        if (root_->autoupdate_)
        {
            root_->update_status();
        }
    }
}

void receiver_t::ufilterfunc_end(piw::ufilterenv_t *, unsigned long long)
{
    if(active_)
    {
        root_->del_receiver(this);
        active_=false;
    }
}
