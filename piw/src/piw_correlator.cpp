
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

#include <piw/piw_correlator.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_thing.h>

#include <picross/pic_stl.h>

#define BUFFER_STARTING   0 // not started yet
#define BUFFER_STARTED    1 // started
#define BUFFER_ENDING     2 // ended
#define BUFFER_SHORT      3 // start and end in same tick
#define BUFFER_LINGERING  4 // releasing
#define BUFFER_LINGERING2 5 // releasing
#define BUFFER_DONE       6 // finished

#define CHECKTIME(x)  t1=tsd_time(); if((t1-t0)>1000ULL) pic::logmsg() << "******** correlator " << (x) << " time " << (t1-t0); t0 = t1;

#define MAX_VOICES 500

namespace
{
    typedef piw::correlator_t::impl_t impl_t;

    struct correlator_input_t;
    struct correlator_default_t;
    struct correlator_voice_t;
    struct correlator_buffer_t;

    struct iid_t
    {
        iid_t(unsigned iid, const piw::data_t &path, int pri=-1);
        iid_t(const iid_t &id);
        iid_t &operator=(const iid_t &id);
        bool operator==(const iid_t &id) const;
        bool operator<(const iid_t &id) const;

        unsigned iid_;
        piw::data_t path_;
        int pri_;
    };

    struct correlator_source_t: pic::element_t<0>, piw::fastdata_t, virtual public pic::lckobject_t
    {
        correlator_source_t(impl_t *root, unsigned signal, unsigned signame, unsigned iid,const piw::data_t &path, const piw::d2d_nb_t &flt, piw::fastdata_t *data, int priority, unsigned type);
        virtual ~correlator_source_t() { tracked_invalidate(); }

        virtual void source_detached(correlator_buffer_t *voice, bool reset_queue) = 0;
        virtual bool input_ticked(unsigned long long f, unsigned long long t, unsigned long sr, unsigned bs) = 0;

        static int __plumb(void *input_, void *) { correlator_source_t *input = (correlator_source_t *)input_; input->startup_fast(); return 0; }
        static int __unplumb(void *input_, void *) { correlator_source_t *input = (correlator_source_t *)input_; input->shutdown_fast(); return 0; }

        void plumb() { piw::tsd_fastcall(__plumb,this,0); }
        void unplumb() { piw::tsd_fastcall(__unplumb,this,0); }

        void activate_input();
        void deactivate_input();

        bool fastdata_receive_data(const piw::data_nb_t &d);
        void idle(bool subscribe);

        void startup_fast();
        virtual void shutdown_fast();

        impl_t *root_;
        unsigned signal_;
        unsigned signame_;
        unsigned iid_;
        piw::data_t path_;
        piw::d2d_nb_t filter_;
        int priority_;
        unsigned type_;
    };

    struct correlator_buffer_t: pic::element_t<0>, pic::element_t<1>, pic::element_t<2>, virtual pic::lckobject_t
    {
        correlator_buffer_t(correlator_voice_t *voice, unsigned long long t);

        void detach_source(correlator_source_t *, unsigned long long, bool);
        void set_signal(unsigned s, unsigned long long t, const piw::dataqueue_t &q,bool linger);
        void detach_sources();

        correlator_voice_t *voice_;
        pic::lckvector_t<correlator_source_t *>::nbtype inputs_;
        unsigned refcount_;
        unsigned refcount_input_;
        unsigned state_;
        piw::xevent_data_buffer_t event_;
        unsigned long long start_time_, end_time_;
    };

    struct correlator_voice_t: piw::event_data_source_real_t, piw::wire_ctl_t,  pic::element_t<0>, pic::element_t<1>, virtual public pic::lckobject_t
    {
        correlator_voice_t(impl_t *root, unsigned id);
        ~correlator_voice_t() { invalidate(); }

        void process_event();
        void source_ended(unsigned seq);
        void invalidate();

        static int __invalidate(void *, void *);

        impl_t *root_;
        unsigned id_;
        piw::dataholder_nb_t current_id_, output_id_;
        pic::ilist_t<correlator_buffer_t,1> buflist_;
        bool free_;
    };

    struct correlator_default_t: correlator_source_t, virtual public pic::lckobject_t
    {
        correlator_default_t(impl_t *root, unsigned signal, unsigned signame, unsigned iid, const piw::data_t &path, int pri, unsigned type, const piw::converter_ref_t &cvt, const piw::d2d_nb_t &flt, piw::fastdata_t *data);
        ~correlator_default_t();

        static int __insert(void *self_, void *item_);
        static int __erase(void *self_, void *item_);

        void erase_defaultbyid();
        void shutdown_fast();
        bool input_ticked(unsigned long long f, unsigned long long t, unsigned long sr, unsigned bs);
        void set_queue(unsigned long long t,const piw::dataqueue_t &q,bool linger);
        void default_attached(correlator_buffer_t *voice, unsigned long long time);
        void source_detached(correlator_buffer_t *voice, bool reset_queue);
        void release_voices(unsigned long long time);
        bool fastdata_receive_event(const piw::data_nb_t &d, const piw::dataqueue_t &q);
        void dump_buffers();

        piw::converter_ref_t converter_;
        piw::dataholder_nb_t current_id_;

        pic::lcklist_t<correlator_buffer_t *>::nbtype buffers_;
        piw::dataqueue_t queue_;
    };

    struct correlator_input_t: correlator_source_t, virtual public pic::lckobject_t
    {
        correlator_input_t(impl_t *root, unsigned signal, unsigned signame, unsigned iid, const piw::data_t &path, const piw::converter_ref_t &cvt, const piw::d2d_nb_t &flt, piw::fastdata_t *data);
        ~correlator_input_t();

        void shutdown_fast();

        bool input_ticked(unsigned long long f, unsigned long long t, unsigned long sr, unsigned bs);
        bool fastdata_receive_event(const piw::data_nb_t &d, const piw::dataqueue_t &q);
        void source_detached(correlator_buffer_t *voice, bool reset_queue);

        void set_queue(unsigned long long t,const piw::dataqueue_t &q);

        correlator_buffer_t *buffer_;
        piw::converter_ref_t converter_;
    };

    piw::data_t voice_id(unsigned id)
    {
        return piw::pathtwo(1+id/255,1+id%255,0);
    }
}

struct piw::correlator_t::impl_t: piw::clocksink_t, piw::thing_t, virtual public pic::lckobject_t
{
    void default_ready(correlator_default_t *d, const piw::data_nb_t &id);
    correlator_buffer_t *allocate_voice(unsigned long long t,const piw::data_nb_t &id, correlator_input_t *input);
    void clocksink_ticked(unsigned long long f, unsigned long long t);
    correlator_default_t *best_default(unsigned,const piw::data_nb_t &);

    impl_t(piw::clockdomain_ctl_t *d, const std::string &sigmap, const piw::d2d_nb_t &evtmap, const piw::cookie_t &c,unsigned thr,unsigned poly);
    ~impl_t();

    void trash_inputs();
    void trash_voices();
    void enable_ticking();
    void activate_input(correlator_source_t *input);
    void deactivate_input(correlator_source_t *input);
    bool add_voice();
    void thing_trigger_slow();
    void plumb_default(unsigned signal, unsigned signame, unsigned iid, const piw::data_t &path, int pri, unsigned type, piw::fastdata_t *data, const piw::converter_ref_t &cvt, const piw::d2d_nb_t &flt);
    void unplumb_default(unsigned signal, unsigned iid, const piw::data_t &path, int pri);
    void plumb_input(unsigned signal, unsigned signame, unsigned iid, const piw::data_t &path, piw::fastdata_t *data, const piw::converter_ref_t &cvt, const piw::d2d_nb_t &flt);
    void unplumb_input(unsigned signal, unsigned iid, const piw::data_t &path);
    void clock_plumbed(unsigned signal, bool status);
    int lookup(unsigned name);
    static int __adder(void *self_, void *voice_);
    void set_latency(unsigned signal, unsigned iid, unsigned latency);
    void remove_latency(unsigned signal, unsigned iid);
    int gc_traverse(void *v, void *a) const;
    int gc_clear();

    void check_time(correlator_source_t *s, unsigned long long t);
    void dump_defaults();

    bool killed_;
    std::vector<correlator_voice_t *> voices_;
    pic::lckmap_t<piw::data_nb_t,correlator_voice_t *,piw::path_less>::nbtype voicemap_; // fast
    pic::lckmap_t<piw::data_nb_t,correlator_voice_t *,piw::path_less>::nbtype voiceout_; // fast
    std::vector<std::map<iid_t,correlator_input_t *> > inputs_; // slow
    pic::lckvector_t<pic::lckmap_t<iid_t,correlator_default_t *>::nbtype >::nbtype defaults_; // fast
    pic::lckvector_t<pic::lckmultimap_t<piw::data_nb_t,correlator_default_t *,piw::path_less>::nbtype >::nbtype defaults_byid_; // fast
    std::vector<unsigned> sigmap_; // static
    std::map<unsigned,unsigned> backmap_; // static
    std::map<std::pair<unsigned,unsigned>,unsigned> latencies_; // slow
    piw::root_ctl_t output_;
    pic::ilist_t<correlator_source_t,0> active_inputs_; // fast
    pic::ilist_t<correlator_voice_t,1> queue_;
    pic::ilist_t<correlator_voice_t,0> freelist_;

    piw::d2d_nb_t mapper_;
    bool enabled_;
    unsigned freecount_;
    unsigned threshold_;

    unsigned poly_;
};

bool correlator_source_t::fastdata_receive_data(const piw::data_nb_t &d)
{
    //pic::logmsg() << "source " << (void *) this << " signal " << signame_ << " going active";
    activate_input();
    return false;
}

void correlator_source_t::idle(bool subscribe)
{
    //pic::logmsg() << "source " << (void *) this << " signal " << signame_ << " idling";
    deactivate_input();

    if(subscribe)
    {
        fastdata_subscribe();
    }
}

void correlator_source_t::activate_input()
{
    //pic::logmsg() << "source " << (void *) this << " signal " << signame_ << " going active";
    root_->activate_input(this);
}

void correlator_source_t::deactivate_input()
{
    //pic::logmsg() << "source " << (void *) this << " signal " << signame_ << " going inactive";
    root_->deactivate_input(this);
}

correlator_buffer_t::correlator_buffer_t(correlator_voice_t *voice, unsigned long long t): voice_(voice), inputs_(voice_->root_->inputs_.size()), refcount_(1), refcount_input_(1), state_(BUFFER_STARTING)
{
    start_time_=t;

    for(unsigned i=0;i<inputs_.size();i++)
    {
        inputs_[i]=0;
    }
}

void piw::correlator_t::impl_t::check_time(correlator_source_t *s, unsigned long long t)
{
    long long et = (long long)t;
    long long ct = (long long)current_tick();
    long dt = (et-ct)/1000LL;
    if(abs(dt)>50)
        pic::logmsg() << "******** correlator: signal " << s->signame_ << " start " << abs(dt) << "ms " << ((dt>0)?"after":"before") << " tick" << " (start " << et << " tick " << ct << ")";
}

void piw::correlator_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
    //unsigned long long t0 = tsd_time();
    //unsigned long long t1;

    unsigned  long sr = get_sample_rate();
    unsigned bs = get_buffer_size();

    unsigned count=0;
    correlator_voice_t *qi;
    while((qi=queue_.pop_front())!=0)
    {
        count++;
        qi->process_event();
    }
    //if(count) pic::logmsg() << "processed " << count << " events";

    //CHECKTIME(1)

    pic::ilist_t<correlator_source_t,0> l;
    correlator_source_t *i;

    //pic::logmsg() << "ticking inputs:";
    while((i=active_inputs_.pop_front())!=0)
    {
        //pic::logmsg() << "ticking " << (void *)i;
        if(i->input_ticked(f,t,sr,bs))
        {
            l.append(i);
        }
        else
        {
            //pic::logmsg() << (void *)i << " unticking";
        }
    }

    //CHECKTIME(2)

    active_inputs_.takeover(l);

    if(!queue_.head() && !active_inputs_.head())
    {
        tick_suppress(true);
        enabled_=false;
//        pic::logmsg() << "clock suppressed";
    }

    //CHECKTIME(3)

}

correlator_buffer_t *piw::correlator_t::impl_t::allocate_voice(unsigned long long t,const piw::data_nb_t &id, correlator_input_t *input)
{
    pic::lckmap_t<piw::data_nb_t,correlator_voice_t *,piw::path_less>::nbtype::iterator i = voicemap_.find(id);

    correlator_buffer_t *b;
    correlator_voice_t *v;
    piw::data_nb_t idout;

    unsigned s = input->signal_;

    if(i!=voicemap_.end())
    {
        v = i->second;
        b = v->buflist_.head();

        if(b)
        {
            if(b->state_==BUFFER_STARTED || b->state_==BUFFER_STARTING)
            {
                if(b->refcount_input_>0)
                {
                    //pic::logmsg() << "attaching input for " << id << " to voice " << (void *)v << " buffer " << (void *)b << " t=" << b->start_time_;

                    if(b->inputs_[s])
                    {
                        if(b->inputs_[s]->priority_<0)
                        {
                            //pic::logmsg() << "already input for " << id << " buffer " << (void *)b << " iid " << b->inputs_[s]->iid_ << " path " << b->inputs_[s]->path_;
                            return 0;
                        }

                        b->inputs_[s]->source_detached(b,false);
                        b->inputs_[s]=0;
                    }

                    b->refcount_++;
                    b->refcount_input_++;
                    return b;
                }

                //pic::logmsg() << "ditching latched event for " << id;

                if(b->state_==BUFFER_STARTED)
                {
                    b->state_=BUFFER_ENDING;
                }
                else
                {
                    b->state_=BUFFER_SHORT;
                }

                b->end_time_=t;
            }
        }

        //pic::logmsg() << "end buffer for " << id << " to voice " << (void *)v << " buffer " << (void *)b;
        idout = v->output_id_;
    }
    else
    {
        idout = mapper_(id);
        //pic::logmsg() << "new voice for " << id << "->" << idout;

        if(!idout.is_path())
        {
            //pic::logmsg() << "mapper skipped (1) id=" << id;
            return 0;
        }

        if((i=voiceout_.find(idout))!=voiceout_.end())
        {
            //pic::logmsg() << "mapper skipped (2) id=" << id;
            return 0;
        }

        v = freelist_.pop_front();

        if(!v)
        {
            pic::logmsg() << "no free input buffer";
            return 0;
        }

        freecount_--;

        if(freecount_<threshold_)
        {
            trigger_slow();
        }

        v->current_id_.set_nb(id);
        v->output_id_.set_nb(idout);
        voicemap_.insert(std::make_pair(id,v));
        voiceout_.insert(std::make_pair(idout,v));
    }

    b = new correlator_buffer_t(v,t);
    v->buflist_.prepend(b);
    b->inputs_[s]=input;
    queue_.append(v);
    enable_ticking();

    //pic::logmsg() << "added buffer, voice " << (void *)v << ' ' << id << "->" << idout << " buffer " << (void *)b << " t=" << t << " vt= " << b->start_time_;

    return b;
}

correlator_default_t *piw::correlator_t::impl_t::best_default(unsigned s, const piw::data_nb_t &id_)
{
    const pic::lckmultimap_t<piw::data_nb_t,correlator_default_t *,piw::path_less>::nbtype &m(defaults_byid_[s]);

    if(m.size()==0)
    {
        return 0;
    }

    if(m.size()==1)
    {
        pic::lckmultimap_t<piw::data_nb_t,correlator_default_t *,piw::path_less>::nbtype::const_iterator iter = m.begin();
        if(iter->first.compare_path(id_)==0)
        {
            return iter->second;
        }
    }

    const unsigned char *p = id_.as_path();
    int l = id_.as_pathlen();

    for(;l>=0;l--)
    {
        std::pair<
            pic::lckmultimap_t<piw::data_nb_t,correlator_default_t *,piw::path_less>::nbtype::const_iterator,
            pic::lckmultimap_t<piw::data_nb_t,correlator_default_t *,piw::path_less>::nbtype::const_iterator> iter(m.equal_range(piw::makepath_nb(p,l)));

        correlator_default_t *d = 0;

        for(; iter.first!=iter.second; ++iter.first)
        {
            correlator_default_t *i = iter.first->second;
            if(!d || i->priority_<d->priority_)
            {
                d=i;
            }
        }

        if(d)
        {
            return d;
        }
    }

    return 0;
}

void piw::correlator_t::impl_t::dump_defaults()
{
    unsigned n = defaults_.size();
    for(unsigned i=0; i<n; i++)
    {
        pic::lckmap_t<iid_t,correlator_default_t *>::nbtype::iterator di,de;
        di = defaults_[i].begin();
        de = defaults_[i].end();
        for(; di!=de; ++di)
        {
            correlator_default_t *def = di->second;
            //pic::logmsg() << "default " << (void *)def << " signal " << def->signame_;
            def->dump_buffers();
        }
    }
}

void piw::correlator_t::impl_t::default_ready(correlator_default_t *d, const piw::data_nb_t &id)
{
    unsigned s = d->signal_;
    unsigned idl = id.as_pathlen();
    const unsigned char *idp = id.as_path();

    defaults_byid_[s].insert(std::make_pair(id,d));

    //pic::logmsg() << "default ready " << id << " " << d->signame_;
    //dump_defaults();

    pic::lckmap_t<piw::data_nb_t,correlator_voice_t *,piw::path_less>::nbtype::iterator i;

    for(i=voicemap_.begin(); i!=voicemap_.end(); i++)
    {
        correlator_voice_t *v = i->second;
        correlator_buffer_t *b = v->buflist_.head();

        //pic::logmsg() << "checking " << v->current_id_;

        if(b==0)
        {
            //pic::logmsg() << "checking " << v->current_id_ << " no buffers";
            continue;
        }

        if(b->state_==BUFFER_DONE || b->state_==BUFFER_ENDING)
        {
            //pic::logmsg() << "checking " << v->current_id_ << " done buffer";
            continue;
        }

        if(b->state_==BUFFER_LINGERING || b->state_==BUFFER_LINGERING2)
        {
            if(d->type_!=INPUT_LINGER)
            {
                //pic::logmsg() << "checking " << v->current_id_ << " lingering buffer " << d->type_ << ' ' << d->signame_;
                continue;
            }

            //pic::logmsg() << "considering " << v->current_id_ << " lingering buffer ";
        }

        if(b->inputs_[s] && b->inputs_[s]->priority_<d->priority_)
        {
            //pic::logmsg() << "checking " << v->current_id_ << " bad pri";
            continue;
        }

        unsigned vidl = v->current_id_.get().as_pathlen();
        const unsigned char *vidp = v->current_id_.get().as_path();

        if(idl>vidl || memcmp(idp,vidp,idl)!=0)
        {
            //pic::logmsg() << "checking " << v->current_id_ << " bad path";
            continue;
        }

        if(b->inputs_[s])
        {
            b->inputs_[s]->source_detached(b,false);
        }

        b->inputs_[s]=d;
        d->default_attached(b,id.time());
    }
}

void correlator_voice_t::source_ended(unsigned seq)
{
    correlator_buffer_t *b = buflist_.tail();

    if(b && (b->state_==BUFFER_LINGERING || b->state_==BUFFER_LINGERING2))
    {
        b->state_=BUFFER_DONE;
        root_->queue_.append(this);
        root_->enable_ticking();
        //pic::logmsg() << "deferred end complete " << current_id_;
    }
    else
    {
        //pic::logmsg() << "deferred end ignored " << current_id_ << " " << (void *)b << " " << (b?b->state_:1000);
    }
}

void correlator_voice_t::process_event()
{

restart:

    if(!buflist_.head())
    {
        pic::lckmap_t<piw::data_nb_t,correlator_voice_t *,piw::path_less>::nbtype::iterator i;

        //pic::logmsg() << "freeing voice for " << current_id_ << " voice " << (void *)this;

        i=root_->voicemap_.find(current_id_);
        root_->voicemap_.erase(i);
        i=root_->voiceout_.find(output_id_);
        root_->voiceout_.erase(i);

        root_->freelist_.append(this);
        root_->freecount_++;
        return;
    }

    correlator_buffer_t *b = buflist_.tail();

    switch(b->state_)
    {
        case BUFFER_STARTED:
            return;

        case BUFFER_DONE:
            //pic::logmsg() << "ditching done buffer " << current_id_ << " voice " << (void *)this << " buffer " << (void *)b;
            //b->buffer_ticked();

            b->detach_sources();
            //pic::logmsg() << "delete buffer " << (void *)b;
            delete b;
            goto restart;

        case BUFFER_LINGERING2:
            if(!buflist_.prev(b))
            {
                return;
            }

            //pic::logmsg() << "linger->done " << current_id_ << " voice " << (void *)this << " buffer " << (void *)b;
            b->state_=BUFFER_DONE;
            goto restart;

        case BUFFER_LINGERING:
            //pic::logmsg() << "lingering buffer " << current_id_ << " voice " << (void *)this << " buffer " << (void *)b;
            for(unsigned s=0;s<b->inputs_.size();s++)
            {
                correlator_source_t *cs = b->inputs_[s];

                if(cs && cs->type_!=INPUT_LINGER)
                {
                    cs->source_detached(b,false);
                    b->inputs_[s]=0;
                }
            }

            b->state_=BUFFER_LINGERING2;
            goto restart;

        case BUFFER_STARTING:
            //pic::logmsg() << "starting event " << current_id_ << " voice " << (void *)this << " buffer " << (void *)b << " t=" << b->start_time_;

            for(unsigned s=0;s<b->inputs_.size();s++)
            {
                if(!b->inputs_[s])
                {
                    correlator_default_t *d = root_->best_default(s,current_id_);

                    if(d)
                    {
                        b->inputs_[s] = d;
                        d->default_attached(b,b->start_time_);
                        //pic::logmsg() << "default " << (void *)d << " attached at start " << current_id_ << " sig:" << d->signame_ << " type:" << d->type_;
                    }
                }
            }
            b->state_=BUFFER_STARTED;
            //pic::logmsg() << "start with " << (void *)b; b->event_.dump(false);
            source_start(0,output_id_.get().restamp(b->start_time_),b->event_);
            return;

        case BUFFER_ENDING:
            //pic::logmsg() << "ending event " << current_id_ << " voice " << (void *)this << " buffer " << (void *)b;

            if(source_end(b->end_time_))
            {
                b->state_=BUFFER_DONE;
            }
            else
            {
                //pic::logmsg() << "deferred buffer release";
                b->state_=BUFFER_LINGERING;
            }

            goto restart;

        case BUFFER_SHORT:
            //pic::logmsg() << "aborting event " << current_id_ << " voice " << (void *)this << " buffer " << (void *)b;
            for(unsigned s=0;s<b->inputs_.size();s++)
            {
                if(!b->inputs_[s])
                {
                    correlator_default_t *d = root_->best_default(s,current_id_);

                    if(d)
                    {
                        b->inputs_[s] = d;
                        d->default_attached(b,b->start_time_);
                        //pic::logmsg() << "default attached at start (short) " << current_id_ << " sig:" << d->signame_ << " type:" << d->type_;
                    }
                }
            }

            //pic::logmsg() << "short with " << (void *)b; b->event_.dump(false);
            source_start(0,output_id_.get().restamp(b->start_time_),b->event_);

            if(source_end(b->end_time_))
            {
                b->state_=BUFFER_DONE;
            }
            else
            {
                //pic::logmsg() << "deferred buffer release";
                b->state_=BUFFER_LINGERING;
            }

            goto restart;
    }
}

void correlator_buffer_t::detach_source(correlator_source_t *input, unsigned long long t, bool reset_queue)
{
    //pic::logmsg() << "detach source " << (void *)input << " " << input->signame_ << " refc " << refcount_ << " from " << (void *)this << " " << input->type_;

    unsigned s = input->signal_;
    correlator_voice_t *v = voice_;
    impl_t *root = v->root_;

    input->source_detached(this,reset_queue);
    inputs_[s]=0;

    if(input->type_==INPUT_MERGE)
    {
        return;
    }

    if(input->type_==INPUT_LINGER)
    {
        correlator_default_t *d = root->best_default(s,v->current_id_);

        if(d)
        {
            inputs_[s] = d;
            d->default_attached(this,0);
        }

        return;
    }

    if(input->type_==INPUT_INPUT)
    {
        --refcount_input_;
    }

    if(--refcount_>0)
    {
        return;
    }

    if(piw::tsd_killed())
    {
        return;
    }

    if(state_==BUFFER_STARTED)
    {
        state_=BUFFER_ENDING;
    }
    else
    {
        state_=BUFFER_SHORT;
    }

    end_time_=t;
    root->queue_.append(voice_);
    root->enable_ticking();

    //pic::logmsg() << "state -> " << state_;
}

void correlator_buffer_t::set_signal(unsigned s,unsigned long long t,const piw::dataqueue_t &q,bool linger)
{
    piw::dataqueue_t oq = event_.signal(s);
    event_.set_signal(s,q);

    if(!voice_)
    {
        return;
    }

    if(state_==BUFFER_STARTED)
    {
        voice_->source_buffer_reset(s,t,oq,q);
        return;
    }

    if(linger)
    {
        if(state_!=BUFFER_DONE)
        {
            voice_->source_buffer_reset(s,t,oq,q);
        }
        return;
    }
}

correlator_voice_t::correlator_voice_t(impl_t *root, unsigned id): piw::event_data_source_real_t(voice_id(id)), root_(root), id_(id)
{
}

piw::correlator_t::correlator_t(piw::clockdomain_ctl_t *d, const std::string &sigmap, const piw::d2d_nb_t &evtmap, const cookie_t &c,unsigned thr,unsigned poly) : impl_(new impl_t(d,sigmap,evtmap,c,thr,poly)) { }

piw::correlator_t::~correlator_t()
{
    tracked_invalidate();
    delete impl_;
}

void piw::correlator_t::set_latency(unsigned signal,unsigned iid, unsigned latency)
{
    impl_->set_latency(signal,iid,latency);
}

void piw::correlator_t::remove_latency(unsigned signal,unsigned iid)
{
    impl_->remove_latency(signal,iid);
}

void piw::correlator_t::plumb_input(unsigned name, unsigned iid, const piw::data_t &path, int pri, unsigned type, piw::fastdata_t *data, const piw::converter_ref_t &cvt, const piw::d2d_nb_t &flt)
{
    int i=impl_->lookup(name);

    if(i>=0)
    {
        if(pri<0)
            impl_->plumb_input(i,name,iid,path,data,cvt,flt);
        else
            impl_->plumb_default(i,name,iid,path,pri,type,data,cvt,flt);
    }
}

void piw::correlator_t::unplumb_input(unsigned name, unsigned iid, const piw::data_t &path, int pri)
{
    int i=impl_->lookup(name);

    if(i>=0)
    {
        if(pri<0)
            impl_->unplumb_input(i,iid,path);
        else
            impl_->unplumb_default(i,iid,path,pri);
    }
}

void piw::correlator_t::kill() { impl_->killed_ = true; }
void piw::correlator_t::clock_plumbed(unsigned signal,bool status) { impl_->clock_plumbed(signal,status); }
piw::clocksink_t *piw::correlator_t::clocksink() { return impl_; }

correlator_source_t::correlator_source_t(impl_t *root, unsigned signal, unsigned signame, unsigned iid,const piw::data_t &path, const piw::d2d_nb_t &flt, piw::fastdata_t *data, int priority,unsigned type): root_(root), signal_(signal), signame_(signame), iid_(iid), path_(path), filter_(flt), priority_(priority), type_(type)
{
    piw::tsd_fastdata(this);
    set_upstream(data);
    enable(true,false,false);
}

void correlator_source_t::startup_fast()
{
    suppress(false,true);
}

void correlator_source_t::shutdown_fast()
{
    suppress(true,false);
    remove();
}

bool correlator_input_t::input_ticked(unsigned long long f, unsigned long long t, unsigned long sr, unsigned bs)
{
    if(!buffer_ || !converter_.isvalid())
    {
        return false;
    }

    //pic::logmsg() << "ticking input converter";
    int rc = converter_->ticked(f,t,sr,bs);
    if(rc>0)
    {
        return true;
    }

    bool wakeup = (rc==0);
    idle(wakeup);

    return false;
}

void correlator_default_t::erase_defaultbyid()
{
    if(!current_id_.get().is_path())
        return;

    pic::lckmultimap_t<piw::data_nb_t,correlator_default_t *,piw::path_less>::nbtype &m(root_->defaults_byid_[signal_]);

    std::pair<
        pic::lckmultimap_t<piw::data_nb_t,correlator_default_t *,piw::path_less>::nbtype::iterator,
        pic::lckmultimap_t<piw::data_nb_t,correlator_default_t *,piw::path_less>::nbtype::iterator> iter;

    iter = m.equal_range(current_id_);
    for(; iter.first!=iter.second; ++iter.first)
    {
        correlator_default_t *i = iter.first->second;
        if(i==this)
        {
            m.erase(iter.first);
            return;
        }
    }
}

bool correlator_default_t::fastdata_receive_event(const piw::data_nb_t &d, const piw::dataqueue_t &q)
{
    //pic::logmsg() << "default: " << (void *)this << " " << signame_ << " iid " << iid_ << " path " << path_ << " event " << d;

    if(current_id_.get().is_path())
    {
        //pic::logmsg() << (void *)this << "release voices";
        erase_defaultbyid();
        current_id_.clear_nb();
        release_voices(d.time());
    }

    piw::data_nb_t nd = filter_(d);
    if(!nd.is_path())
    {
        return false;
    }

    if(converter_.isvalid())
    {
        piw::dataqueue_t oq = converter_->convert(q,d.time());
        if(!oq.isvalid())
        {
            pic::logmsg() << "converter rejected " << signame_;
            return false;
        }

        set_queue(d.time(),oq,false);
        activate_input();
    }
    else
    {
        set_queue(d.time(),q,false);
    }

    //pic::logmsg() << (void *)this << "attach voice";
    current_id_.set_nb(nd);
    root_->default_ready(this,current_id_);

    return false;
}

void correlator_default_t::set_queue(unsigned long long t,const piw::dataqueue_t &q,bool linger)
{
    queue_ = q;
    pic::lcklist_t<correlator_buffer_t *>::nbtype::iterator i,e;
    i = buffers_.begin();
    e = buffers_.end();

    for(; i!=e; i++)
    {
        correlator_buffer_t *b = *i;
        b->set_signal(signame_,t,q,linger);
        //pic::logmsg() << "(default) buffer " << (void *)b << " setq for " << signame_; q.dump(false);
    }
    root_->enable_ticking();
}

bool correlator_input_t::fastdata_receive_event(const piw::data_nb_t &d, const piw::dataqueue_t &q)
{
    //pic::logmsg() << "input: " << (void *)this << " " << signame_ << " iid " << iid_ << " path " << path_ << " event " << d;

    if(buffer_)
    {
        if(converter_.isvalid())
        {
            converter_->flush();
        }

        //pic::logmsg() << "input " << signame_ << " detaching from buffer " << (void *)buffer_;
        buffer_->detach_source(this,d.time(),false);
        buffer_=0;
    }

    if(!d.is_path())
    {
        return false;
    }

    piw::data_nb_t nd = filter_(d);

    if(!nd.is_path())
    {
        return false;
    }

    //root_->check_time(this,d.time());

    buffer_=root_->allocate_voice(d.time(),nd,this);

    if(buffer_)
    {
        //pic::logmsg() << "allocated voice " << (void *)buffer_ << " for " << d << "->" << nd << " t=" << d.time() << " vt=" << buffer_->start_time_;
        if(converter_.isvalid())
        {
            //pic::logmsg() << "input " << signame_ << " converting";
            piw::dataqueue_t oq = converter_->convert(q,d.time());
            if(!oq.isvalid())
            {
                pic::logmsg() << "converter rejected " << signame_;
                return false;
            }

            set_queue(d.time(),oq);
            activate_input();
        }
        else
        {
            set_queue(d.time(),q);
        }
    }
    return false;
}

void correlator_input_t::shutdown_fast()
{
    correlator_source_t::shutdown_fast();

    if(buffer_)
    {
        buffer_->detach_source(this,piw::tsd_time(),true);
        buffer_=0;
    }
}

correlator_input_t::correlator_input_t(impl_t *root, unsigned signal, unsigned signame, unsigned iid, const piw::data_t &path, const piw::converter_ref_t &cvt, const piw::d2d_nb_t &flt, piw::fastdata_t *data):
    correlator_source_t(root,signal,signame,iid,path,flt,data,-1,INPUT_INPUT), buffer_(0), converter_(cvt)
{
    root_->inputs_[signal_].insert(std::make_pair(iid_t(iid_,path_,-1),this));
    plumb();
}

correlator_input_t::~correlator_input_t()
{
    tracked_invalidate();
    unplumb();
    root_->inputs_[signal_].erase(iid_t(iid_,path_,-1));
}

bool correlator_default_t::input_ticked(unsigned long long f, unsigned long long t, unsigned long sr, unsigned bs)
{
    if(!converter_.isvalid())
    {
        return false;
    }

    //pic::logmsg() << "ticking default converter";
    int rc = converter_->ticked(f,t,sr,bs);
    if(rc>0)
    {
        return true;
    }

    bool wakeup = (rc==0);
    idle(wakeup);

    return false;
}

void correlator_input_t::set_queue(unsigned long long t,const piw::dataqueue_t &q)
{
    buffer_->set_signal(signame_,t,q,false);
    //pic::logmsg() << "(input) buffer " << (void *)(buffer_) << " setq for " << signame_; q.dump(false);
    root_->enable_ticking();
}

void correlator_input_t::source_detached(correlator_buffer_t *buffer, bool reset_queue)
{
    if(reset_queue)
    {
        buffer->set_signal(signame_,0,piw::dataqueue_t(),false);
    }
    buffer=0;
    root_->deactivate_input(this);
}

void correlator_default_t::source_detached(correlator_buffer_t *buffer, bool reset_queue)
{
    //pic::logmsg() << "detaching buffer " << (void *)buffer << " from " << (void *)this;
    //dump_buffers();
    buffers_.remove(buffer);
    if(reset_queue)
    {
        buffer->set_signal(signame_,0,piw::dataqueue_t(),false);
    }
    //dump_buffers();


    if(buffers_.empty())
    {
        //pic::logmsg() << (void *)this << " source detached";
        root_->deactivate_input(this);
    }
}

void correlator_default_t::dump_buffers()
{
    pic::msg_t m(pic::logmsg());
    m << "buffers: ";
    pic::lcklist_t<correlator_buffer_t *>::nbtype::iterator i,e;
    i = buffers_.begin();
    e = buffers_.end();

    for(; i!=e; i++)
    {
        correlator_buffer_t *b = *i;
        m << (void *)b << " ";
    }
}

void correlator_default_t::default_attached(correlator_buffer_t *buffer, unsigned long long time)
{
    buffers_.push_back(buffer);

    if(!time && queue_.isvalid())
    {
        time = queue_.current().time();
    }

    buffer->set_signal(signame_,time,queue_,(type_==INPUT_LINGER));
    //pic::logmsg() << "(default_attached) buffer " << (void *)(buffer) << " attached for " << signame_ << " " << (void *)this; queue_.dump(false);
    //dump_buffers();

    if(type_==INPUT_LATCH)
    {
        buffer->refcount_++;
    }

    root_->enable_ticking();

    if(converter_.isvalid())
    {
        activate_input();
    }
}

void correlator_default_t::release_voices(unsigned long long time)
{
    correlator_buffer_t *b;

    while(!buffers_.empty())
    {
        b = buffers_.front();
        b->detach_source(this,time,false);
    }
}

void correlator_default_t::shutdown_fast()
{
    correlator_source_t::shutdown_fast();
    release_voices(piw::tsd_time());
}

int correlator_default_t::__insert(void *self_, void *)
{
    correlator_default_t *self = (correlator_default_t *)self_;
    self->root_->defaults_[self->signal_].insert(std::make_pair(iid_t(self->iid_,self->path_,self->priority_),self));

    return 0;
}

int correlator_default_t::__erase(void *self_, void *)
{
    correlator_default_t *self = (correlator_default_t *)self_;
    self->root_->defaults_[self->signal_].erase(iid_t(self->iid_,self->path_,self->priority_));
    self->erase_defaultbyid();
    return 0;
}

correlator_default_t::correlator_default_t(impl_t *root, unsigned signal, unsigned signame, unsigned iid, const piw::data_t &path, int priority, unsigned type, const piw::converter_ref_t &cvt, const piw::d2d_nb_t &flt, piw::fastdata_t *data):
    correlator_source_t(root,signal,signame,iid,path,flt,data,priority,type), converter_(cvt)
{
    piw::tsd_fastcall(__insert,this,0);
    plumb();
}

correlator_default_t::~correlator_default_t()
{
    tracked_invalidate();
    unplumb();
    piw::tsd_fastcall(__erase,this,0);
}

int piw::correlator_t::gc_traverse(void *v, void *a) const
{
    return impl_->gc_traverse(v,a);
}

int piw::correlator_t::gc_clear()
{
    return impl_->gc_clear();
}

piw::correlator_t::impl_t::impl_t(piw::clockdomain_ctl_t *d, const std::string &sigmap, const piw::d2d_nb_t &evtmap, const piw::cookie_t &c,unsigned thr,unsigned poly): killed_(false), inputs_(sigmap.size()), defaults_(sigmap.size()), defaults_byid_(sigmap.size()), sigmap_(sigmap.size()), mapper_(evtmap), enabled_(false), freecount_(0), poly_(poly)
{
    piw::tsd_thing(this);

    threshold_=0;

    for(unsigned i=0;i<sigmap.size();i++)
    {
        sigmap_[i] = (unsigned char)sigmap[i];
        backmap_.insert(std::make_pair(sigmap_[i],i));
    }

    output_.connect(c);
    d->sink(this,"correlator");
    tick_enable(false);
    output_.set_clock(this);

    threshold_=thr;
}

int piw::correlator_t::impl_t::__adder(void *self_, void *voice_)
{
    impl_t *self = (impl_t *)self_;
    correlator_voice_t *voice = (correlator_voice_t *)voice_;
    self->freelist_.append(voice);
    self->freecount_++;
    return 0;
}

void piw::correlator_t::impl_t::trash_inputs()
{
    for(unsigned s=0;s<sigmap_.size();s++)
    {
        std::map<iid_t,correlator_input_t *> &i(inputs_[s]);
        std::map<iid_t,correlator_input_t *>::iterator ii;

        while((ii=i.begin())!=i.end())
        {
            delete ii->second;
        }

        pic::lckmap_t<iid_t,correlator_default_t *>::nbtype &d(defaults_[s]);
        pic::lckmap_t<iid_t,correlator_default_t *>::nbtype::iterator di;

        while((di=d.begin())!=d.end())
        {
            delete di->second;
        }
    }
}

void piw::correlator_t::impl_t::trash_voices()
{
    for(unsigned i=0;i<voices_.size();i++)
    {
        delete voices_[i];
    }
}

piw::correlator_t::impl_t::~impl_t()
{
    tracked_invalidate();
    tick_disable();
    close_sink();
    close_thing();
    trash_inputs();
    trash_voices();
}

void piw::correlator_t::impl_t::enable_ticking()
{
    if(!enabled_)
    {
        enabled_=true;
        tick_suppress(false);
        //pic::logmsg() << "clock unsuppressed";
    }
    else
    {
        //pic::logmsg() << "clock already unsuppressed";
    }
}

void piw::correlator_t::impl_t::activate_input(correlator_source_t *input)
{
    //pic::logmsg() << "activate_input " << (void *)input;
    active_inputs_.append(input);
    enable_ticking();
}

void piw::correlator_t::impl_t::deactivate_input(correlator_source_t *input)
{
    //pic::logmsg() << "deactivate_input " << (void *)input;
    input->remove();
}

bool piw::correlator_t::impl_t::add_voice()
{
    if(poly_ && voices_.size()>=poly_)
        return false;

    if(voices_.size()>=MAX_VOICES)
        return false;

    unsigned id = voices_.size();
    correlator_voice_t *v = new correlator_voice_t(this,id+1);
    output_.connect_wire(v,v->source());
    voices_.resize(id+1);
    voices_[id]=v;
    piw::tsd_fastcall(__adder,this,v);
    return true;
}

void piw::correlator_t::impl_t::thing_trigger_slow()
{
    unsigned f=freecount_;

    if(f<threshold_)
    {
        pic::logmsg() << "adding " << threshold_-f << " correlator voices free=" << f << " threshold=" << threshold_;

        while(f<threshold_)
        {
            if(!add_voice())
            {
                pic::logmsg() << "maximum voice count reached: " << voices_.size();
                return;
            }
            f++;
        }
    }

    pic::logmsg() << "voice count now " << voices_.size();
}

void piw::correlator_t::impl_t::plumb_default(unsigned signal, unsigned signame, unsigned iid, const piw::data_t &path, int pri, unsigned type, piw::fastdata_t *data, const piw::converter_ref_t &cvt, const piw::d2d_nb_t &flt)
{
    //pic::logmsg() << "default sig:" << signame << " fastdata:" << data;
    unplumb_default(signal,iid,path,pri);
    new correlator_default_t(this,signal,signame,iid,path,pri,type,cvt,flt,data);
}

void piw::correlator_t::impl_t::unplumb_default(unsigned signal, unsigned iid, const piw::data_t &path, int pri)
{
    pic::lckmap_t<iid_t,correlator_default_t *>::nbtype::iterator i = defaults_[signal].find(iid_t(iid,path,pri));

    if(i!=defaults_[signal].end())
    {
        delete i->second;
    }
}

void piw::correlator_t::impl_t::plumb_input(unsigned signal, unsigned signame, unsigned iid, const piw::data_t &path, piw::fastdata_t *data, const piw::converter_ref_t &cvt, const piw::d2d_nb_t &flt)
{
    unplumb_input(signal,iid,path);

    unsigned np = inputs_[signal].size()+1;
    unsigned np2 = np;

    if(poly_)
    {
        np2 = std::min(np2,poly_);
    }

    while(voices_.size() < np2)
    {
        //pic::logmsg() << "adding voice, current=" << voices_.size() << " np=" << np << " poly=" << poly_ << " signal=" << signal;
        if(!add_voice())
        {
            pic::logmsg() << "maximum voice count " << voices_.size() << " reached";
            break;
        }
    }

    new correlator_input_t(this,signal,signame,iid,path,cvt,flt,data);
    //pic::logmsg() << "poly now " << voices_.size();
}

void piw::correlator_t::impl_t::unplumb_input(unsigned signal, unsigned iid, const piw::data_t &path)
{
    std::map<iid_t,correlator_input_t *> &m(inputs_[signal]);
    std::map<iid_t,correlator_input_t *>::iterator i = m.find(iid_t(iid,path,-1));

    if(i!=m.end())
    {
        delete i->second;
    }
}

void piw::correlator_t::impl_t::clock_plumbed(unsigned signal, bool status)
{
}

int piw::correlator_t::impl_t::lookup(unsigned name)
{
    std::map<unsigned,unsigned>::iterator i;

    if((i=backmap_.find(name))==backmap_.end())
    {
        return -1;
    }

    return i->second;
}

void piw::correlator_t::impl_t::set_latency(unsigned signal, unsigned iid, unsigned latency)
{
    std::map<std::pair<unsigned,unsigned>,unsigned>::iterator i;
    std::pair<unsigned,unsigned> k(signal,iid);

    if((i=latencies_.find(k))!=latencies_.end())
    {
        i->second=latency;
    }
    else
    {
        latencies_.insert(std::make_pair(k,latency));
    }

    unsigned maxlatency=0;

    for(i=latencies_.begin(); i!=latencies_.end(); i++)
    {
        if(i->second>maxlatency) maxlatency=i->second;
    }

    //pic::logmsg() << "latency now " << maxlatency;
    set_sink_latency(maxlatency);
    output_.set_latency(maxlatency);
}

void piw::correlator_t::impl_t::remove_latency(unsigned signal, unsigned iid)
{
    std::map<std::pair<unsigned,unsigned>,unsigned>::iterator i;
    std::pair<unsigned,unsigned> k(signal,iid);

    latencies_.erase(k);

    unsigned maxlatency=0;

    for(i=latencies_.begin(); i!=latencies_.end(); i++)
    {
        if(i->second>maxlatency) maxlatency=i->second;
    }

    //pic::logmsg() << "latency now " << maxlatency;
    set_sink_latency(maxlatency);
}

int piw::correlator_t::impl_t::gc_traverse(void *v, void *a) const
{
    int r;

    if((r=mapper_.gc_traverse(v,a))!=0)
    {
        return r;
    }

    for(unsigned signal=0;signal<sigmap_.size();signal++)
    {
        const std::map<iid_t,correlator_input_t *> &im(inputs_[signal]);
        std::map<iid_t,correlator_input_t *>::const_iterator ii;

        for(ii=im.begin(); ii!=im.end(); ii++)
        {
            if((r=ii->second->filter_.gc_traverse(v,a))!=0)
            {
                return r;
            }
        }

        const pic::lckmap_t<iid_t,correlator_default_t *>::nbtype &dm = defaults_[signal];
        pic::lckmap_t<iid_t,correlator_default_t *>::nbtype::const_iterator di;

        for(di=dm.begin(); di!=dm.end(); di++)
        {
            if((r=di->second->filter_.gc_traverse(v,a))!=0)
            {
                return r;
            }
        }
    }

    return 0;
}

int piw::correlator_t::impl_t::gc_clear()
{
    mapper_.gc_clear();

    for(unsigned signal=0;signal<sigmap_.size();signal++)
    {
        std::map<iid_t,correlator_input_t *> &im(inputs_[signal]);
        std::map<iid_t,correlator_input_t *>::iterator ii;

        for(ii=im.begin(); ii!=im.end(); ii++)
        {
            ii->second->filter_.gc_clear();
        }

        const pic::lckmap_t<iid_t,correlator_default_t *>::nbtype &dm = defaults_[signal];
        pic::lckmap_t<iid_t,correlator_default_t *>::nbtype::const_iterator di;

        for(di=dm.begin(); di!=dm.end(); di++)
        {
            di->second->filter_.gc_clear();
        }
    }

    return 0;
}

void correlator_buffer_t::detach_sources()
{
    for(unsigned s=0;s<inputs_.size();s++)
    {
        if(inputs_[s])
        {
            inputs_[s]->source_detached(this,false);
            inputs_[s]=0;
        }
    }
}

int correlator_voice_t::__invalidate(void *v_, void *)
{
    correlator_buffer_t *b;
    correlator_voice_t *v = (correlator_voice_t *)v_;

    while((b=v->buflist_.head())!=0)
    {
        b->detach_sources();
        delete b;
    }

    return 0;
}

void correlator_voice_t::invalidate()
{
    source_shutdown();
    disconnect();
    piw::tsd_fastcall(__invalidate,this,0);
}

iid_t::iid_t(unsigned iid, const piw::data_t &path, int pri): iid_(iid), path_(path), pri_(pri)
{
}

iid_t::iid_t(const iid_t &id): iid_(id.iid_), path_(id.path_), pri_(id.pri_)
{
}

iid_t &iid_t::operator=(const iid_t &id)
{
    iid_=id.iid_;
    path_=id.path_;
    pri_=id.pri_;
    return *this;
}

bool iid_t::operator==(const iid_t &id) const
{
    if(iid_!=id.iid_)
    {
        return false;
    }
    
    if(pri_!=id.pri_)
    {
        return false;
    }

    if(path_!=id.path_)
    {
        return false;
    }
    
    return true;
}

bool iid_t::operator<(const iid_t &id) const
{
    if(iid_<id.iid_) return true;
    if(iid_>id.iid_) return false;
    if(pri_<id.pri_) return true;
    if(pri_>id.pri_) return false;
    if(path_<id.path_) return true;
    if(path_>id.path_) return false;
    return false;
}
