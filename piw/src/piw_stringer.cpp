
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

/*
 * piw_stringer.cpp: makes courses monophonic, liked stringed instruments
 */


#include <piw/piw_stringer.h>
#include <piw/piw_tsd.h>
#include <piw/piw_scaler.h>

#include <picross/pic_ilist.h>
#include <picross/pic_ref.h>
#include <vector>
#include <list>

#define VSTATE_IDLE      0
#define VSTATE_ACTIVE    1

#define STRINGER_DEBUG 0
// flag to activate one mono note per chaff path per course
// 0 = mono per course
// 1 = mono per cource per path (chaff), e.g. mono each take from the recorder
#define STRINGER_USE_PATH 1

namespace
{
    struct main_wire_t;
    struct voice_t;

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // voice_t: the output wires, one wire is created per course going to the instrument
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    struct voice_t: pic::element_t<0>, pic::element_t<1>, virtual pic::counted_t, virtual pic::lckobject_t, piw::event_data_source_real_t
    {
        voice_t(piw::stringer_t::impl_t *i, const piw::data_t &path): piw::event_data_source_real_t(path), impl_(i), state_(VSTATE_IDLE), owner_(0) {}
        ~voice_t() { source_shutdown(); }

        void startup(main_wire_t *owner,const piw::data_nb_t &id,std::string path, unsigned course, unsigned relkey, unsigned abskey,const piw::xevent_data_buffer_t &b, unsigned long long t);
        bool shutdown(unsigned long long time);

        void source_ended(unsigned seq);

        void add_wire(main_wire_t *wire);
        void remove_wire(main_wire_t *wire, unsigned long long t);

        piw::stringer_t::impl_t *impl_;
        piw::data_nb_t id_;
        unsigned state_;

        piw::wire_ctl_t main_wire_;

        unsigned long long etime_;
        main_wire_t *owner_;
        piw::xevent_data_buffer_t buffer_;

        // list of active wires
        std::list<main_wire_t *> active_wires_;

        unsigned course_,abskey_,relkey_;
        std::string path_;
    };

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // main_wire_t: the input wires, polyphonic wires coming from the keyboard
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    struct main_wire_t: piw::wire_t, piw::event_data_sink_t, pic::element_t<>
    {
        main_wire_t(piw::stringer_t::impl_t *i, const piw::event_data_source_t &);
        ~main_wire_t() { invalidate(); }

        void wire_closed() { delete this; }
        void invalidate();

        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        bool event_end(unsigned long long t);

        void detach_voice(voice_t *voice);

        piw::stringer_t::impl_t *impl_;
        // voice the wire belongs to
        voice_t *voice_;
        unsigned seq_;

        piw::data_nb_t id_;
        std::string kpath_;
        unsigned kcourse_;
        unsigned kkey_;
        unsigned knum_;
        piw::xevent_data_buffer_t b_;

    };

}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// stringer impl struct and functions
//
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct piw::stringer_t::impl_t: piw::decode_ctl_t, virtual pic::lckobject_t
{
    impl_t(const piw::cookie_t &);
    ~impl_t();

    piw::wire_t *wire_create(const piw::event_data_source_t &es);

    void set_clock(bct_clocksink_t *c) { main_encoder_.set_clock(c); }
    void set_latency(unsigned l) { main_encoder_.set_latency(l); }

    voice_t *allocate_voice(main_wire_t *owner,const piw::data_nb_t &id, std::string path, unsigned course, unsigned relkey, unsigned abskey,const piw::xevent_data_buffer_t &b)
    {
        // allocate a note from a course to a voice
#if STRINGER_DEBUG>0
        pic::logmsg() << "allocate voice rel=" << relkey << " abs=" << abskey;
#endif // STRINGER_DEBUG>0

        pic::lckmap_t<std::pair<std::string,unsigned>,voice_t *>::nbtype::iterator i;

        // allocate if stringer enabled, otherwise just create a new voice for every input wire
        if(enable_)
        {
            // is there already a note from this course of this path on a voice?
#if STRINGER_DEBUG>0
            pic::logmsg() << "check for voice on course";
#endif // STRINGER_DEBUG>0
            // find in path
            if((i=pathcourse2voice_.find(std::make_pair(path,course)))!=pathcourse2voice_.end())
            {
                // then startup the new key on the voice
                i->second->startup(owner,id,path,course,relkey,abskey,b,id.time());
                return i->second;
            }
        }

        // otherwise, course empty, get a free voice
        voice_t *voice=free_queue_.head();

        if(enable_)
        {
            // store the assignment of the course to the voice
#if STRINGER_DEBUG>0
            pic::logmsg() << "insert voice on course";
#endif // STRINGER_DEBUG>0
            pathcourse2voice_.insert(std::make_pair(std::make_pair(path,course),voice));
        }

        // start up the key on the voice
        voice->startup(owner,id,path,course,relkey,abskey,b,id.time());

        // return voice to wire
        return voice;
    }

    void free_voice(voice_t *voice,unsigned long long t)
    {
        // shutdown the voice when all notes have finished
#if STRINGER_DEBUG>0
        pic::logmsg() << "free voice";
#endif // STRINGER_DEBUG>0


        // remove voice from a course
        // try this even when not enabled to remove any voices that
        // might have been assigned to courses when it was enabled
        // (can disable when notes are being held down!)
        pathcourse2voice_.erase(std::make_pair(voice->path_, voice->course_));

        voice->shutdown(t);
    }

    piw::data_nb_t nextid(const piw::data_nb_t &);

    void set_poly(unsigned poly);

    void enable(bool enable) { enable_ = enable; }

    unsigned poly_;

    // queues of free and busy output voices
    pic::ilist_t<voice_t,0> free_queue_;
    pic::ilist_t<voice_t,1> busy_queue_;
    // list of output voices
    std::vector<pic::ref_t<voice_t> > voices_;

    // path and course number to voice map
    pic::lckmap_t<std::pair<std::string,unsigned>,voice_t *>::nbtype pathcourse2voice_;

    pic::ilist_t<main_wire_t> active_;

    piw::decoder_t main_decoder_;
    piw::root_ctl_t main_encoder_;

    unsigned id_;

    bool enable_;
};

piw::stringer_t::impl_t::impl_t(const piw::cookie_t &c): poly_(0), main_decoder_(this), id_(1), enable_(true)
{
    main_encoder_.connect(c);
}

piw::stringer_t::impl_t::~impl_t()
{
}

piw::wire_t *piw::stringer_t::impl_t::wire_create(const piw::event_data_source_t &es)
{
    set_poly(main_decoder_.wire_count()+1);
    return new main_wire_t(this,es);
}

static int setup_voice__(void *impl_, void *voice_)
{
    piw::stringer_t::impl_t *impl = (piw::stringer_t::impl_t *)impl_;
    voice_t *voice = (voice_t *)voice_;
    //
    impl->free_queue_.append(voice);
    return 0;
}

void piw::stringer_t::impl_t::set_poly(unsigned poly)
{
    // setup total available output voices
    while(poly_<poly)
    {
        voices_.resize(poly_+1);
        voices_[poly_]=pic::ref(new voice_t(this,piw::pathone(poly_+1,0)));
        main_encoder_.connect_wire(&voices_[poly_]->main_wire_,voices_[poly_]->source());
        piw::tsd_fastcall(setup_voice__,this,voices_[poly_].ptr());
        poly_++;
    }

}

piw::data_nb_t piw::stringer_t::impl_t::nextid(const piw::data_nb_t &d_)
{
    piw::data_nb_t d(d_);

    if(id_)
    {
        d = piw::pathappend_chaff_nb(d,id_++);
        if(id_>255)
            id_=1;
    }

    return d;
}






// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// voice functions
//
// create a wires for the output polyphony
// new notes cause an end event and a start event with the same timestamp
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

void voice_t::startup(main_wire_t *owner,const piw::data_nb_t &id,std::string path, unsigned course, unsigned relkey, unsigned abskey,const piw::xevent_data_buffer_t &b,unsigned long long t)
{
    if(owner_)
    {
        // already a wire on the voice
        owner_->detach_voice(this);
        // end the current downstream event
#if STRINGER_DEBUG>0
        pic::logmsg() << "stringer ending downstream at " << id.time();
#endif // STRINGER_DEBUG>0
        // end source and start source at the same time
        source_end(t);
        // restamp the id time with the current time
        // stops the event falling behind the current time in the correlator
        id_ = id_.restamp(t);
    }
    else
    {
        // no owner yet, so new voice
        id_=id;
    }

    owner_=owner;
    path_=path;
    course_=course;
    abskey_=abskey;
    relkey_=relkey;

    buffer_ = piw::xevent_data_buffer_t(SIG1(5),5);
    buffer_.merge(b,SIG5(1,2,3,4,6));
    // send the abskey out on the key output
    buffer_.add_value(5,piw::makelong_bounded_nb(1000,0,0,abskey,id_.time()));
//    buffer_.add_value(5,piw::makelong_bounded_nb(1000,0,0,abskey,t));

    // start the new downstream event
#if STRINGER_DEBUG>0
    pic::logmsg() << "stringer starting downstream at " << id_.time();
#endif // STRINGER_DEBUG>0
    source_start(0,id_,buffer_);

    if(state_==VSTATE_IDLE)
    {
        state_=VSTATE_ACTIVE;
        impl_->free_queue_.remove(this);
        impl_->busy_queue_.append(this);
    }

#if STRINGER_DEBUG>0
    pic::logmsg() << "voice " << (void *)this << " starting " << id_ << ' ' << abskey_;
#endif // STRINGER_DEBUG>0
}

void voice_t::source_ended(unsigned seq)
{
    impl_->busy_queue_.remove(this);
    impl_->free_queue_.append(this);
    owner_=0;
}

bool voice_t::shutdown(unsigned long long time)
{
#if STRINGER_DEBUG>0
    pic::logmsg() << "shutdown " << (void *)this << " " << state_ << ' ' << id_ << ' ' << time;
#endif // STRINGER_DEBUG>0

    PIC_ASSERT(state_==VSTATE_ACTIVE);

    state_=VSTATE_IDLE;
    etime_=time+1;

    if(source_end(etime_))
    {
#if STRINGER_DEBUG>0
        pic::logmsg() << "voice " << (void *)this << " freed ";
#endif // STRINGER_DEBUG>0
        impl_->busy_queue_.remove(this);
        impl_->free_queue_.append(this);
        owner_=0;
        return true;
    }

    return false;
}

void voice_t::add_wire(main_wire_t *wire)
{
    // put this at the head of the list of active wires
    active_wires_.push_front(wire);
    // it will be playing now from the stringer allocating the course to the voice

#if STRINGER_DEBUG>0
    for (std::list<main_wire_t *>::iterator it=active_wires_.begin(); it!=active_wires_.end(); ++it)
        pic::logmsg() << *it;
#endif // STRINGER_DEBUG>0
}

void voice_t::remove_wire(main_wire_t *wire, unsigned long long t)
{

#if STRINGER_DEBUG>0
    for (std::list<main_wire_t *>::iterator it=active_wires_.begin(); it!=active_wires_.end(); ++it)
        pic::logmsg() << *it;
#endif // STRINGER_DEBUG>0

    // if the wire at the head then stop playing it
    // and play the next in list
#if STRINGER_DEBUG>0
    pic::logmsg() << "remove_wire";
#endif // STRINGER_DEBUG>0

    if(active_wires_.front()==wire)
    {
#if STRINGER_DEBUG>0
        pic::logmsg() << "at head";
#endif // STRINGER_DEBUG>0
        // remove head
        active_wires_.pop_front();
        // if no more wires then free voice
        if(active_wires_.empty())
        {
            impl_->free_voice(this,t);
        }
        else
        {
            // play next
            main_wire_t *next = active_wires_.front();
            startup(next,next->id_,next->kpath_,next->kcourse_,next->kkey_,next->knum_,next->b_,t);
        }
    }
    else
    {
#if STRINGER_DEBUG>0
        pic::logmsg() << "remove other";
#endif // STRINGER_DEBUG>0
        // remove from list
        active_wires_.remove(wire);
    }

#if STRINGER_DEBUG>0
    for (std::list<main_wire_t *>::iterator it2=active_wires_.begin(); it2!=active_wires_.end(); ++it2)
        pic::logmsg() << *it2;
#endif // STRINGER_DEBUG>0

}


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// main wire functions
//
// the input wires, polyphonic wires coming from the keyboard
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

main_wire_t::main_wire_t(piw::stringer_t::impl_t *impl, const piw::event_data_source_t &es) : impl_(impl), voice_(0)
{
    subscribe(es);
}

void main_wire_t::invalidate()
{
    unsubscribe();
    disconnect();
}

void main_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(voice_) voice_->source_buffer_reset(s,t,o,n);
}

void main_wire_t::detach_voice(voice_t *voice)
{
#if STRINGER_DEBUG>0
    pic::logmsg() << "detach voice";
#endif // STRINGER_DEBUG>0
    remove();
}

void main_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    seq_=seq;

    unsigned knum=0;
    unsigned kcourse=0;
    unsigned kkey=0;

#if STRINGER_USE_PATH==1
    unsigned chaff_len=id.as_pathchafflen();
    //pic::logmsg() << "id = " << id << " chaff_len=" << chaff_len;

    std::string chaff=id.as_pathstr();
    chaff.resize(chaff_len);
#else
    std::string chaff("path");
#endif // STRINGER_USE_PATH==1

    piw::data_nb_t d;
    if(b.latest(6,d,id.time()) && d.is_tuple() && d.as_tuplelen() >= 6)
    {
        knum = d.as_tuple_value(3).as_long();

        piw::data_nb_t muskey = d.as_tuple_value(4);
        kcourse = unsigned(muskey.as_tuple_value(0).as_float());
        kkey = unsigned(muskey.as_tuple_value(1).as_float());
    }

#if STRINGER_DEBUG>0
    pic::logmsg() << "event start key=" << knum << " course=" << kcourse << " key=" << kkey;
#endif // STRINGER_DEBUG>0

    // assign to a voice according to the course this note belongs to
    // and start playing the voice (will be at head of active wire list so play it)
    voice_ = impl_->allocate_voice(this,id,chaff,kcourse,kkey,knum,b);

    // store note info in case the wire is reallocated later
    id_ = id;
    kpath_ = chaff;
    kcourse_ = kcourse;
    kkey_ = kkey;
    knum_ = knum;
    b_ = b;

    // put this at the head of the list of active wires
    voice_->add_wire(this);
}

bool main_wire_t::event_end(unsigned long long t)
{
    // a key has been released
#if STRINGER_DEBUG>0
    pic::logmsg() << "event end: wire " << this << " voice " << voice_;
#endif // STRINGER_DEBUG>0

    // remove from ilist
    remove();

    // remove this from the list of active wires in the voice
    // will stop playing if it's the head
    voice_->remove_wire(this, t);

    return true;
}



// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// stringer interface class
//
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

piw::stringer_t::stringer_t(const piw::cookie_t &c): impl_(new impl_t(c))
{
}

piw::stringer_t::~stringer_t()
{
    delete impl_;
}

piw::cookie_t piw::stringer_t::data_cookie()
{
    return impl_->main_decoder_.cookie();
}

void piw::stringer_t::enable(bool b)
{
    impl_->enable(b);
}
