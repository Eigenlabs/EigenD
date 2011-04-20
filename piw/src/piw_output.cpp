
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

#include <piw/piw_output.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <piw/piw_fastdata.h>
#include <picross/pic_ilist.h>
#include <picross/pic_ref.h>

#include <vector>

#define CHILDRENPERNODE (((BCTLINK_SMALLTREE)<=250)?(BCTLINK_SMALLTREE):250)

piw::splitter_node_t::splitter_node_t(): piw::server_t(PLG_SERVER_RO|PLG_SERVER_TRANSIENT|PLG_SERVER_RTRANSIENT), extension_(0)
{
    set_data(piw::data_t());
}

piw::splitter_node_t::~splitter_node_t()
{
    tracked_invalidate();
    close_server();
    signals_.clear();

    if(extension_)
    {
        delete extension_;
        extension_=0;
    }
}

void piw::splitter_node_t::close_server()
{
    if(open())
    {
        for(std::map<unsigned, pic::weak_t<piw::server_t> >::iterator i = signals_.begin(); i!=signals_.end(); i++)
        {
            if(i->second.isvalid())
            {
                i->second->close_server();
            }
        }
    }

    if(extension_)
    {
        extension_->close_server();
    }

}

void piw::splitter_node_t::server_opened()
{
    piw::server_t::server_opened();

    for(std::map<unsigned, pic::weak_t<piw::server_t> >::iterator i = signals_.begin(); i!=signals_.end(); i++)
    {
        if(i->second.isvalid() && !i->second->open())
        {
            child_add(i->first+1,i->second.ptr());
        }
    }

    if(extension_ && !extension_->open())
    {
        child_add(254,extension_);
    }
}

void piw::splitter_node_t::add_signal(unsigned v, piw::server_t *s)
{
    if(v>=CHILDRENPERNODE)
    {
        if(!extension_)
        {
            extension_=new splitter_node_t();

            if(open() && !closing())
            {
                child_add(254,extension_);
            }
        }

        extension_->add_signal(v-CHILDRENPERNODE,s);
        return;
    }

    remove_signal(v);

    signals_.insert(std::make_pair(v,s));

    if(open() && !closing())
    {
        child_add(v+1,s);
    }
}

void piw::splitter_node_t::remove_signal(unsigned v)
{
    if(v>=CHILDRENPERNODE)
    {
        if(extension_)
        {
            extension_->remove_signal(v-CHILDRENPERNODE);
            return;
        }
    }

    std::map<unsigned, pic::weak_t<piw::server_t> >::iterator i = signals_.find(v);

    if(i!=signals_.end())
    {
        if(open() && i->second.isvalid())
        {
            i->second->close_server();
        }

        signals_.erase(i);
    }
}

namespace
{
    struct splitter_sig_t : piw::server_t, piw::fastdata_t, pic::atomic_counted_t, virtual public pic::lckobject_t
    {
        splitter_sig_t();
        ~splitter_sig_t() { tracked_invalidate(); }
    };

    struct splitter_wire_t : piw::wire_t, piw::event_data_sink_t
    {
        splitter_wire_t(const piw::event_data_source_t &es, piw::splitter_t::impl_t *root);
        ~splitter_wire_t() { invalidate(); }
        void wire_closed() { delete this; }
        void invalidate();

        void event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &b);
        static int adder__(void *, void *, void *);
        static int remover__(void *, void *, void *);

        void add_signal(unsigned s, piw::splitter_node_t *r);
        void remove_signal(unsigned s, piw::splitter_node_t *r);

        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &o,const piw::dataqueue_t &n);

        piw::event_data_source_t source_;
        piw::splitter_t::impl_t *root_;
        pic::flipflop_t<pic::lckvector_t<pic::ref_t<splitter_sig_t> >::lcktype> children_;
        unsigned voice_;

        piw::dataholder_nb_t event_;
    };
}

struct piw::splitter_t::impl_t : piw::root_t
{
    impl_t(piw::splitter_t *ctl): root_t(0), ctl_(ctl)
    {
    } 

    ~impl_t() { invalidate(); }

    void root_closed() { invalidate(); }
    void root_opened() { root_clock(); root_latency(); }
    void root_latency() { ctl_->set_latency(get_latency()); }
    void root_clock() { ctl_->set_clock(get_clock()); }

    void add_signal(unsigned s, piw::splitter_node_t *r)
    {
        remove_signal(s);

        if(roots_.size()<=s)
        {
            roots_.resize(s+1);
        }

        roots_[s] = r;

        for(unsigned i=0;i<wires_.size();i++)
        {
            if(wires_[i])
            {
                wires_[i]->add_signal(s,r);
            }
        }
    }

    void remove_signal(unsigned s)
    {
        if(roots_.size()>s && roots_[s].isvalid())
        {
            for(unsigned i=0;i<wires_.size();i++)
            {
                if(wires_[i])
                {
                    wires_[i]->remove_signal(s,roots_[s].ptr());
                }
            }

            roots_[s].clear();
        }
    }

    wire_t *root_wire(const piw::event_data_source_t &es)
    {
        return new splitter_wire_t(es,this);
    }

    void invalidate()
    {
        root_t::disconnect();

        for(unsigned i=0;i<wires_.size();i++)
        {
            if(wires_[i])
            {
                delete wires_[i];
            }
        }
    }

    piw::splitter_t *ctl_;
    std::vector<pic::weak_t<splitter_node_t> > roots_;
    std::vector<splitter_wire_t *> wires_;
};

void splitter_wire_t::event_start(unsigned seq,const piw::data_nb_t &id,const piw::xevent_data_buffer_t &b)
{
    PIC_WARN(id.time()!=0ULL);
    piw::tsd_protect_t p;

    pic::flipflop_t<pic::lckvector_t<pic::ref_t<splitter_sig_t> >::lcktype>::guard_t g(children_);
    const pic::lckvector_t<pic::ref_t<splitter_sig_t> >::lcktype &c(g.value());

    for(unsigned i=1; i<c.size(); ++i)
    {
        if(c[i].isvalid())
        {
            piw::dataqueue_t q(b.signal(i));
            if(q.isvalid())
            {
                //pic::logmsg() << "output signal " << i;
                bct_fastdata_t *f(c[i].ptr());
                bct_fastdata_host_send_fast(f,id.lend(),q.lend());
            }
        }
    }

    event_.set_nb(id);
}

void splitter_wire_t::event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &o,const piw::dataqueue_t &n)
{
    PIC_WARN(t!=0ULL);
    piw::tsd_protect_t p;

    pic::flipflop_t<pic::lckvector_t<pic::ref_t<splitter_sig_t> >::lcktype>::guard_t g(children_);
    const pic::lckvector_t<pic::ref_t<splitter_sig_t> >::lcktype &c(g.value());

    if(s>0 && s<c.size() && c[s].isvalid())
    {
        //pic::logmsg() << "output signal " << i;
        bct_fastdata_t *f(c[s].ptr());
        bct_fastdata_host_send_fast(f,event_.get().restamp(t).lend(),n.lend());
    }
}

bool splitter_wire_t::event_end(unsigned long long t)
{
    pic::flipflop_t<pic::lckvector_t<pic::ref_t<splitter_sig_t> >::lcktype>::guard_t g(children_);
    const pic::lckvector_t<pic::ref_t<splitter_sig_t> >::lcktype &c(g.value());

    event_.set_nb(piw::makenull_nb(t));
    piw::dataqueue_t q;

    for(unsigned i=1; i<c.size(); ++i)
    {
        if(c[i].isvalid())
        {
            c[i]->send_fast(event_,q);
        }
    }

    return true;
}

splitter_sig_t::splitter_sig_t(): piw::fastdata_t(PLG_FASTDATA_SENDER)
{
    tsd_fastdata(this);
    set_source(this);
}

splitter_wire_t::splitter_wire_t(const piw::event_data_source_t &es, piw::splitter_t::impl_t *root): source_(es), root_(root)
{
    int fi = -1;
    unsigned j;

    for(unsigned i=0;i<root_->wires_.size();i++)
    {
        if(root_->wires_[i])
        {
            if(root_->wires_[i]->source_.path()==es.path())
            {
                delete root_->wires_[i];
                fi=i;
                break;
            }
        }
        else
        {
            if(fi<0) fi=i;
        }
    }

    if(fi<0)
    {
        fi = root_->wires_.size();
        root_->wires_.resize(fi+1);
    }

    voice_=fi;

    children_.alternate().resize(root->roots_.size());

    root_->wires_[voice_]=this;

    for(j=0;j<root->roots_.size();j++)
    {
        piw::splitter_node_t *n = root_->roots_[j].ptr();

        if(n)
        {
            pic::ref_t<splitter_sig_t> sig = pic::ref(new splitter_sig_t);
            children_.alternate()[j]=sig;
            n->add_signal(voice_,sig.ptr());
        }
    }

    children_.exchange();
    subscribe_and_ping(source_);
}

void splitter_wire_t::add_signal(unsigned s, piw::splitter_node_t *r)
{
    if(children_.alternate().size() < s+1)
    {
        children_.alternate().resize(s+1);
    }

    pic::ref_t<splitter_sig_t> sig = pic::ref(new splitter_sig_t);
    children_.alternate()[s]=sig;
    r->add_signal(voice_,sig.ptr());

    children_.exchange();
}

void splitter_wire_t::remove_signal(unsigned s, piw::splitter_node_t *r)
{
    children_.alternate()[s].clear();
    children_.exchange();

    r->remove_signal(voice_);
}

void splitter_wire_t::invalidate()
{
    unsigned j;

    unsubscribe();

    wire_t::disconnect();

    if(root_)
    {
        root_->wires_[voice_]=0;

        for(j=0;j<root_->roots_.size();j++)
        {
            piw::splitter_node_t *n = root_->roots_[j].ptr();

            if(n)
            {
                n->remove_signal(voice_);
            }
        }

        root_ = 0;
    }
}

piw::splitter_t::splitter_t() : impl_(new impl_t(this)) {}
piw::splitter_t::~splitter_t() { delete impl_; }
piw::cookie_t piw::splitter_t::cookie() { return cookie_t(impl_); }
void piw::splitter_t::add_signal(unsigned s, piw::splitter_node_t *r) { impl_->add_signal(s,r); }
void piw::splitter_t::remove_signal(unsigned s) { impl_->remove_signal(s); }
unsigned piw::splitter_t::get_latency() { return impl_->get_latency(); }
bct_clocksink_t *piw::splitter_t::get_clock() { return impl_->get_clock(); }
