
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

#include <piw/piw_multiplexer.h>
#include <piw/piw_aggregator.h>
#include <piw/piw_gate.h>
#include <piw/piw_tsd.h>
#include <piw/piw_policy.h>
#include <piw/piw_status.h>

namespace
{
    typedef piw::multiplexer_t::impl_t mimpl_t;

    struct input_t: virtual pic::lckobject_t
    {
        input_t(mimpl_t *root, unsigned label_);
        ~input_t();

        mimpl_t *root_;
        unsigned label_;
        piw::gate_t gate_;
        piw::change_nb_t change_;
        bool enabled_;
    };
};

struct piw::multiplexer_t::impl_t: virtual pic::tracked_t, virtual pic::lckobject_t
{
    impl_t(const piw::cookie_t &dst, piw::clockdomain_ctl_t *clk): statusmixer_(dst), aggregator_(statusmixer_.cookie(),clk), active_(-1)
    {
    }

    ~impl_t()
    {
        tracked_invalidate();
    }

    piw::cookie_t get_input(unsigned label)
    {
        clear_input(label,false);
        pic::lckvector_t<input_t *>::lcktype &v(inputs_.alternate());


        if(label>=v.size())
        {
            v.resize(label+1);
        }

        input_t *i = new input_t(this,label);
        v[label]=i;
        inputs_.exchange();
        piw::tsd_fastcall(__chk,this,&label);

        return i->gate_.cookie();
    }

    void clear_input(unsigned label, bool chk)
    {
        pic::lckvector_t<input_t *>::lcktype &v(inputs_.alternate());

        if(label>=v.size())
        {
            return;
        }

        input_t *i = v[label];

        if(i)
        {
            v[label]=0;
            inputs_.exchange();
            delete i;

            if(chk)
            {
                piw::tsd_fastcall(__chk,this,&label);
            }

        }
    }

    static int __chk(void *self_, void *label_)
    {
        impl_t *self = (impl_t *)self_;
        pic::flipflop_t<pic::lckvector_t<input_t *>::lcktype>::guard_t g(self->inputs_);
        const pic::lckvector_t<input_t *>::lcktype &v(g.value());
        int l=*(unsigned *)label_;

        if(l==self->active_)
        {
            if(!v[l])
            {
                self->active_=-1;
                return 0;
            }

            if(!v[l]->enabled_)
            {
                v[l]->change_(piw::makebool_nb(true,piw::tsd_time()));
                v[l]->enabled_=true;
                return 0;
            }

        }

        return 0;
    }

    static int __gate(void *self_, void *a_, void *t_)
    {
        impl_t *self = (impl_t *)self_;
        pic::flipflop_t<pic::lckvector_t<input_t *>::lcktype>::guard_t g(self->inputs_);
        const pic::lckvector_t<input_t *>::lcktype &v(g.value());

        self->statusmixer_.autoupdate(false);

        int a = *(long*)a_;
        unsigned long long t = *(unsigned long long*)t_;
        
        if(!t) t=piw::tsd_time();

        if(a != self->active_)
        {
            if(self->active_>=0)
            {
                input_t *i = v[self->active_];

                if(i)
                {
                    i->change_(piw::makebool_nb(false,t));
                    i->enabled_=false;
                }
            }

            self->active_=a;

            if(a>=0)
            {
                input_t *i = v[a];

                if(i)
                {
                    i->change_(piw::makebool_nb(true,t+1));
                    i->enabled_=true;
                }
            }

            self->statusmixer_.update();
        }

        self->statusmixer_.autoupdate(true);

        return 0;
    }

    void gate(const piw::data_nb_t &d)
    {
        if(d.is_long())
        {
            long a = d.as_long();
            unsigned long long t = d.time();
            piw::tsd_fastcall3(__gate,this,(void *)&a,(void *)&t);
        }
    }

    piw::statusmixer_t statusmixer_;
    piw::aggregator_t aggregator_;
    pic::flipflop_t<pic::lckvector_t<input_t *>::lcktype> inputs_;
    int active_;
};

input_t::input_t(mimpl_t *root, unsigned label): root_(root), label_(label), gate_(root->aggregator_.get_filtered_output(label,piw::null_filter()),false), enabled_(false)
{
    change_=gate_.gate();
}

input_t::~input_t()
{
    root_->aggregator_.clear_output(label_);
}

piw::multiplexer_t::multiplexer_t(const piw::cookie_t &dst, piw::clockdomain_ctl_t *clk): impl_(new impl_t(dst,clk)) {}
piw::multiplexer_t::~multiplexer_t() { delete impl_; }

piw::cookie_t piw::multiplexer_t::get_input(unsigned n) { return impl_->get_input(n); }
void piw::multiplexer_t::clear_input(unsigned n) { impl_->clear_input(n,true); }
piw::change_nb_t piw::multiplexer_t::gate_input() { return piw::change_nb_t::method(impl_,&impl_t::gate); }
