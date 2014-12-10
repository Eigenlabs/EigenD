
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



#include "fng_finger.h"
#include <piw/piw_tsd.h>
#include <piw/piw_scaler.h>
#include <piw/piw_keys.h>

#include <picross/pic_ilist.h>
#include <picross/pic_ref.h>
#include <bitset>
#include <algorithm>
#include <cmath>

//output clock period for the newly generated note in uS
#define OUTPUT_PERIOD 500
#define MAX_POLYPHONY 20


#define FINGER_DEBUG 0

typedef std::pair<fng::fingering_t::f_pattern_t, float> keyID_t;
typedef pic::lcklist_t<keyID_t>::nbtype keys_down_t;

typedef std::pair<float, float> output_weight_t;
typedef std::pair<fng::fingering_t::f_pattern_t, output_weight_t> output_fingering_t;
typedef pic::lcklist_t<output_fingering_t>::nbtype output_weights_t;

typedef std::pair<unsigned, output_weight_t> output_poly_fingering_t;
typedef pic::lcklist_t<output_poly_fingering_t>::nbtype output_poly_weights_t;

namespace
{

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // input_wire_t: the input wires, polyphonic wires coming from the keyboard
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    struct input_wire_t: piw::wire_t, piw::event_data_sink_t, pic::element_t<>
    {
        input_wire_t(fng::finger_t::impl_t *i, unsigned id, const piw::event_data_source_t &);
        ~input_wire_t();

        void wire_closed() { delete this; }
        void invalidate();

        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b);
        void event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n);
        bool event_end(unsigned long long t);

        fng::finger_t::impl_t *impl_;

        unsigned wire_id_;
        unsigned seq_;
        std::string kpath_;
        unsigned kcourse_;
        unsigned kkey_;
        float breath_;

        piw::xevent_data_buffer_t b_;
    };

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // output_wire: the output wires, one extra wire is created per polyphony modifier
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    struct output_wire: virtual pic::counted_t, virtual pic::lckobject_t, piw::event_data_source_real_t
    {
        output_wire(fng::finger_t::impl_t *i, const piw::data_t &path);
        ~output_wire() { source_shutdown(); }

        void startup(const piw::data_nb_t &id,unsigned long long t);
        bool shutdown(unsigned long long time);
        void source_ended(unsigned seq);
        void add_value(unsigned sig, const piw::data_nb_t &data);

        fng::finger_t::impl_t *impl_;
        piw::wire_ctl_t main_wire_;
        piw::xevent_data_buffer_t buffer_;
    };

}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// fingererer impl struct and functions
//
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct fng::finger_t::impl_t: piw::decode_ctl_t, virtual pic::lckobject_t, piw::clocksink_t
{
    impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &);
    ~impl_t();

    piw::wire_t *wire_create(const piw::event_data_source_t &es);

    void set_clock(bct_clocksink_t *c) { if(up_) remove_upstream(up_); up_=c; if(up_) add_upstream(up_); }
    void set_latency(unsigned l) { }
    void clocksink_ticked(unsigned long long f, unsigned long long t);
    void set_poly(unsigned poly);
    void tick_once(const fng::fingering_t &, unsigned long long time);

    pic::ilist_t<input_wire_t> active_;
    piw::decoder_t main_decoder_;
    piw::root_ctl_t main_encoder_;
    piw::xevent_data_buffer_t pressure_signals_;
    piw::xevent_data_buffer_t::iter_t pressure_iter_;
    input_wire_t *input_wires_[MAX_SIGNALS];
    std::vector<pic::ref_t<output_wire> > output_wires_;

    bct_clocksink_t *up_;
    unsigned long long from_;
    float active_key_pressures[MAX_COURSES][MAX_KEYS];
    pic::flipflop_t<fng::fingering_t> fingering_flipflop_;

    bool notes_running_[MAX_POLYPHONY];
    bool poly_notes_still_running_[MAX_POLYPHONY];
    unsigned poly_voices_;
    piw::data_nb_t event_id_;
};


void fng::finger_t::impl_t::set_poly(unsigned poly)
{
    // setup total available output voices, one for each possible note
    while(poly_voices_<poly)
    {
        output_wires_.resize(poly_voices_+1);
        output_wires_[poly_voices_]=pic::ref(new output_wire(this,piw::pathone(poly_voices_+1,0)));
        main_encoder_.connect_wire(&output_wires_[poly_voices_]->main_wire_,output_wires_[poly_voices_]->source());
        notes_running_[poly_voices_]=false;
        poly_voices_++;
    }
    //notes_running_[0] = false;
}

fng::finger_t::impl_t::impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c):  main_decoder_(this), up_(0), poly_voices_(0)
{
    pressure_iter_ = pressure_signals_.iterator();
    poly_voices_=0;

    for(unsigned i=0;i<MAX_SIGNALS;i++)
    {
        input_wires_[i] = 0;
    }

    cd->sink(this,"fingerer");
    tick_enable(true);
    main_encoder_.set_clock(this);
    main_encoder_.connect(c);
}

fng::finger_t::impl_t::~impl_t()
{
    main_decoder_.shutdown();
    tick_disable();
}

piw::wire_t *fng::finger_t::impl_t::wire_create(const piw::event_data_source_t &es)
{
    unsigned id = main_decoder_.wire_count();

    if(id >= MAX_SIGNALS)
    {
        return 0;
    }

    return new input_wire_t(this,id,es);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// main wire functions
//
// the input wires, polyphonic wires coming from the keyboard
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

input_wire_t::input_wire_t(fng::finger_t::impl_t *impl, unsigned id, const piw::event_data_source_t &es) : impl_(impl), wire_id_(id)
{
    impl_->input_wires_[wire_id_] = this;
    subscribe(es);
}

input_wire_t::~input_wire_t()
{
    invalidate();
    impl_->input_wires_[wire_id_] = 0;
}

void input_wire_t::invalidate()
{
    unsubscribe();
    disconnect();
}

void fng::finger_t::impl_t::tick_once(const fng::fingering_t &fingering, unsigned long long t)
{
//    pic::logmsg() << "NEW TICK --------------------------------------------------------------------" ;

    input_wire_t *w;

    const fng::fingering_t::table_t fingering_ = fingering.get_table();
    const fng::fingering_t::table_t modifiers_ = fingering.get_modifiers();
    const fng::fingering_t::table_t additions_ = fingering.get_additions();
    const fng::fingering_t::table_t polyphonies_ = fingering.get_polyphonies();
    const fng::fingering_t::range_t range_ = fingering.get_range();
    float breath_threshold_ = fingering.get_breath_threshold();

//for every currently active key, update the active keys tables
    float roll_ = 0.0;
    float yaw_ = 0.0;
    float breath_pressure_ = 0.0;
    int roll_count_ = 0;
    int yaw_count_ = 0;

    fng::fingering_t::f_pattern_t active_keys_;
    keys_down_t keys_down_;
    keys_down_t::const_iterator key_down;

    for(w=active_.head(); w!=0; w=active_.next(w))
    {
        piw::data_nb_t d;
        if(w->b_.signal(5).latest(d,0,t))
        {
            if(d.as_norm() > 0.0)
            {
                breath_pressure_=d.as_norm();
            }
            else
            {
                breath_pressure_=0.0;
            }
            continue;
        }
        else if(w->b_.signal(1).latest(d,0,t))
        {
            if(d.as_norm() < range_.first)
            {
                continue;
            }
            active_keys_.set(((w->kcourse_-1) * MAX_KEYS) + w->kkey_-1, true);
            keys_down_.push_back(keyID_t());
            keys_down_.back().first.set(((w->kcourse_-1) * MAX_KEYS) + w->kkey_-1, true);
            keys_down_.back().second = std::min(range_.second * d.as_norm(), (float)1.0); 
            active_key_pressures[w->kcourse_-1][w->kkey_-1] = d.as_norm();
            if(w->b_.signal(2).latest(d,0,t))
            {
                roll_ += d.as_norm();
                roll_count_++;
            }
            if(w->b_.signal(3).latest(d,0,t))
            {
                yaw_ += d.as_norm();
                yaw_count_++;
            }
        }
        else
        {
            continue;
        }
    }
    if(roll_count_ !=0)
    {
        roll_ = roll_/roll_count_;
    }
    if(yaw_count_ !=0)
    {
        yaw_ = yaw_/yaw_count_;
    }

    output_fingering_t output_weight_;
    output_weights_t output_weights_;
    pic::lcklist_t<output_fingering_t>::nbtype::iterator ow_; 
    pic::lcklist_t<output_fingering_t>::nbtype::reverse_iterator row_; 

    output_fingering_t output_modifier_;
    output_weights_t output_additions_;
    output_weights_t output_modifiers_;

    output_poly_weights_t output_polys_;
    pic::lcklist_t<output_poly_fingering_t>::nbtype::iterator pow_; 

    float weight_ = 1.0;
    float last_weight_ = 1.0;

    bool valid_fingering_ = false;
    bool got_full_fingering_ = false;

    for(unsigned i=0; i<fingering_.size(); i++)
    {
        //find the largest active fingering 
        if((fingering_[i].first.first & active_keys_) == fingering_[i].first.first)
        {
            //For all the possible subfingerings calculate the weights
            for(unsigned j=0; j<fingering_[i].second.size(); j++)
            {
                //multiply all the pressures to give a starting weight 
                weight_ = 1.0;
                for(key_down=keys_down_.begin(); key_down!=keys_down_.end(); ++key_down)
                {
                    if((fingering_[i].second[j]->first & key_down->first) != 0)
                    {
                        weight_ = weight_ * key_down->second;
                    }
                }
                //if our weight is 1 ish, ignore all smaller fingerings in this list
                if(fabs(1.0 - weight_) < 0.01)
                {
                    got_full_fingering_=true;
                } 
                last_weight_ = 0;
                if(output_weights_.size() != 0)
                {
                    for(row_ = output_weights_.rbegin(); row_ != output_weights_.rend(); ++row_)
                    {
                        if((row_->first & fingering_[i].second[j]->first) == fingering_[i].second[j]->first)
                        {
                            last_weight_ = row_->second.first;
                        }
                    }
                }
                else
                {
                    last_weight_ = 0;
                }
                weight_ = weight_ * (1.0 - last_weight_);
                output_weights_.push_back(output_fingering_t());
                output_weights_.back().second.first = weight_;  //fingering weighting
                output_weights_.back().second.second = fingering_[i].second[j]->second; //target scale inc
                output_weights_.back().first = fingering_[i].second[j]->first; //bitmap fingering
                if(got_full_fingering_)
                {
                    break;
                }
            }
            valid_fingering_ = true;
            break;
        }
    }            
   
    if(valid_fingering_ && (fabs(breath_pressure_) > breath_threshold_))
    {
//        pic::logmsg() << "Got valid fingering " << event_id_;  

        //find all the additions
        for(unsigned i=0; i<additions_.size(); i++)
        {
            if((additions_[i].first.first & active_keys_) == additions_[i].first.first)
            {
                for(unsigned j=0; j<additions_[i].second.size(); j++)
                {
                    weight_ = 1.0;
                    for(key_down=keys_down_.begin(); key_down!=keys_down_.end(); ++key_down)
                    {
                        if((additions_[i].second[j]->first & key_down->first) != 0)
                        {
                            weight_ = weight_ * key_down->second;
                        }
                    }
                    last_weight_ = 0;
                    if(output_additions_.size() != 0)
                    {
                        for(row_ = output_additions_.rbegin(); row_ != output_additions_.rend(); ++row_)
                        {
                            if((row_->first & additions_[i].second[j]->first) == additions_[i].second[j]->first)
                            {
                                last_weight_ = row_->second.first;
                            }
                        }
                    }
                    else
                    {
                        last_weight_ = 0;
                    }
                    weight_ = weight_ * (1.0 - last_weight_);
                    output_additions_.push_back(output_fingering_t());
                    output_additions_.back().second.first = weight_;  //addition weighting
                    output_additions_.back().second.second = additions_[i].second[j]->second; //target addition inc
                    output_additions_.back().first = additions_[i].second[j]->first; //bitmap fingering
                }
            }
        }
        //find all the modifiers
        for(unsigned i=0; i<modifiers_.size(); i++)
        {
            if((modifiers_[i].first.first & active_keys_) == modifiers_[i].first.first)
            {
                for(unsigned j=0; j<modifiers_[i].second.size(); j++)
                {
                    weight_ = 1.0;
                    for(key_down=keys_down_.begin(); key_down!=keys_down_.end(); ++key_down)
                    {
                        if((modifiers_[i].second[j]->first & key_down->first) != 0)
                        {
                            weight_ = weight_ * key_down->second;
                        }
                    }
                    last_weight_ = 0;
                    if(output_weights_.size() != 0)
                    {
                        for(row_ = output_weights_.rbegin(); row_ != output_weights_.rend(); ++row_)
                        {
                            if((row_->first & modifiers_[i].second[j]->first) == modifiers_[i].second[j]->first)
                            {
                                last_weight_ = row_->second.first;
                            }
                        }
                    }
                    else
                    {
                        last_weight_ = 0;
                    }

                    weight_ = weight_ * (1.0 - last_weight_);
                    output_modifiers_.push_back(output_fingering_t());
                    output_modifiers_.back().second.first = weight_;  //modifier weighting
                    output_modifiers_.back().second.second = modifiers_[i].second[j]->second; //target modifier inc
                    output_modifiers_.back().first = modifiers_[i].second[j]->first; //bitmap fingering
                }
            }
        }
        //find all the polyphonic additions
        for(unsigned i=0; i<polyphonies_.size(); i++)
        {
            if((polyphonies_[i].first.first & active_keys_) == polyphonies_[i].first.first)
            {
                for(key_down=keys_down_.begin(); key_down!=keys_down_.end(); ++key_down)
                {
                    if((polyphonies_[i].first.first & key_down->first) != 0)
                    {
                        weight_ = key_down->second;
                    }
                }
                output_polys_.push_back(output_poly_fingering_t());
                output_polys_.back().first = i;
                output_polys_.back().second.first = weight_;  //polyphony pressure
                output_polys_.back().second.second = polyphonies_[i].second[0]->second; //target polyphony inc
            }
        }

        //Output the notes 

        float divisor_ = 0.0;
        int divisor_count_ = 0;
        float output_key_ = 0.0;
        float output_addition_ = 0.0;
        float output_mod_ = 0.0;

        //start a new output event for main note if needed
        if(notes_running_[0] == false)
        {
            output_wires_[0]->startup(piw::pathappend_nb(event_id_, 1),event_id_.time());
            notes_running_[0] = true;
        }

        //calculate the weighted mean of the active fingerings
        for(ow_=output_weights_.begin(); ow_!=output_weights_.end(); ++ow_)
        {
            divisor_ += ow_->second.first;
            output_key_ = output_key_ + (ow_->second.first * ow_->second.second);
        }
        output_key_ = output_key_/divisor_;

        //weight the effects of valid additions
        divisor_ = 0.0;
        divisor_count_ = 0;
        for(ow_=output_additions_.begin(); ow_ != output_additions_.end(); ++ow_)
        {
//            pic::logmsg() << "calculating addition:" << ow_->first << " , offset:" << ow_->second.second << ", weight:" << ow_->second.first;  
            divisor_ += ow_->second.first;
            output_addition_ = output_addition_ + (ow_->second.first * ow_->second.second);
            divisor_count_++;
        }
        if(divisor_count_ > 1)
        {
            output_addition_ = output_addition_/divisor_;
        }
        output_key_ = output_key_ + output_addition_;

        
        //weight the effects of valid modifiers
        divisor_ = 0.0;
        divisor_count_ = 0;
        for(ow_=output_modifiers_.begin(); ow_ != output_modifiers_.end(); ++ow_)
        {
            output_mod_ = output_mod_ + (ow_->second.first * ow_->second.second);
            divisor_ += ow_->second.first;
            divisor_count_++;
        }
        if(divisor_count_ > 1)
        {
            output_mod_ = output_mod_/divisor_;
        }
        else if(divisor_count_ == 0)
        {
            output_mod_ = 0.0;
        }

//        pic::logmsg() << "playing note: " << output_key_ << " pressure: " << breath_pressure_ << " modifier: " << output_mod_ << " roll:" << roll_ << " yaw:" << yaw_;  

        //main output voice
        output_wires_[0]->add_value(1,piw::makefloat_bounded_nb(1.0,0.0,0.0,breath_pressure_,t));
        output_wires_[0]->add_value(2,piw::makefloat_bounded_nb(1.0,-1.0,0.0,roll_,t));
        output_wires_[0]->add_value(3,piw::makefloat_bounded_nb(1.0,-1.0,0.0,yaw_,t));
        output_wires_[0]->add_value(4,piw::makekey(1,output_key_,1,output_key_,piw::KEY_SOFT,t)); 
        output_wires_[0]->add_value(5,piw::makefloat_bounded_units_nb(BCTUNIT_STEPS,96,-96,0,output_mod_,t));


        //add the polyphonic notes
        for(unsigned i=1; i<poly_voices_; i++)
        {
            poly_notes_still_running_[i]=false;
        }
        float op_ = 1.0;
        float poly_pressure_ = 0.0;
        unsigned voice_ = 0;
        for(pow_=output_polys_.begin(); pow_ != output_polys_.end(); ++pow_)
        {

            voice_=pow_->first+1;
            if(!notes_running_[voice_])
            {
                output_wires_[voice_]->startup(piw::pathappend_nb(event_id_, voice_+1),event_id_.time());
                notes_running_[voice_] = true;
            }
            op_ =  output_key_ + pow_->second.second;
            poly_pressure_ =  pow_->second.first * breath_pressure_;
            output_wires_[voice_]->add_value(1,piw::makefloat_bounded_nb(1.0,0.0,0.0,poly_pressure_,t)); 
            output_wires_[voice_]->add_value(2,piw::makefloat_bounded_nb(1.0,0.0,0.0,roll_,t));
            output_wires_[voice_]->add_value(3,piw::makefloat_bounded_nb(1.0,0.0,0.0,yaw_,t));
            output_wires_[voice_]->add_value(4,piw::makekey(1,op_,1,op_,piw::KEY_SOFT,t)); 
            output_wires_[voice_]->add_value(5,piw::makefloat_bounded_units_nb(BCTUNIT_STEPS,96,-96,0,output_mod_,t));
            poly_notes_still_running_[voice_]=true;

        }
        //shutdown running poly wires where the notes are no longer active 
        for(unsigned i=1; i<poly_voices_; i++)
        {
            if(notes_running_[i] && !poly_notes_still_running_[i])
            {
               output_wires_[i]->shutdown(t);
               notes_running_[i]=false;
            }
        }
    }
    else
    {
        for(unsigned i=0; i<poly_voices_; i++) {
            if(notes_running_[i])
            {
               output_wires_[i]->shutdown(t);
               notes_running_[i]=false;
            }
        }

        notes_running_[0]=false;
    }
}

void fng::finger_t::impl_t::clocksink_ticked(unsigned long long from, unsigned long long to)
{
    pic::flipflop_t<fng::fingering_t>::guard_t finger_guard(fingering_flipflop_);

    const fng::fingering_t &fingering(finger_guard.value());

#if FINGER_DEBUG>0
//    pic::logmsg() << "clock ticked " << from << " to " << to;
#endif // FINGER_DEBUG>0

    if(!from_)
    {
        from_ = from;
    }

    while(from_ <= to)
    {
        tick_once(fingering, from_);
        from_ += OUTPUT_PERIOD;
    }
}

void input_wire_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    seq_=seq;

    piw::data_nb_t d;
    float course_in,key_in;

    kcourse_ = 1;
    kkey_ = 1;

    if(b.latest(4,d,id.time()) && piw::decode_key(d,0,0,&course_in,&key_in))
    {
        kcourse_ = course_in;
        kkey_ = key_in;
    }
    if(b.latest(5,d,id.time()))
    {
//        pic::logmsg() << "breath event start: wire " << this << " voice ";
        breath_=d.as_norm();
    }

    if(kkey_==0 || kkey_>MAX_KEYS)
    {
        return;
    }

    if(kcourse_==0 || kcourse_>MAX_COURSES)
    {
        return;
    }

    // store note info in case the wire is reallocated later
    b_ = b;

    impl_->pressure_signals_.set_signal(wire_id_+1,b.signal(1));
    impl_->pressure_iter_->reset(wire_id_+1,id.time());

    impl_->from_ = id.time();
    impl_->tick_suppress(false);

    impl_->active_.append(this);

    if(impl_->notes_running_[0] == false)
    {
        impl_->event_id_ = id;
    }
#if FINGER_DEBUG>0
//    pic::logmsg() << "event start: wire " << this << " voice ";
#endif // FINGER_DEBUG>0
}

void input_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    if(s==1)
    {
        impl_->pressure_signals_.set_signal(wire_id_+1,n);
        impl_->pressure_iter_->reset(wire_id_+1,t);
    }
}

bool input_wire_t::event_end(unsigned long long t)
{
    // a key has been released
#if FINGER_DEBUG>0
//    pic::logmsg() << "event end: wire " << this << " voice ";
#endif // FINGER_DEBUG>0

    // remove from ilist
    impl_->pressure_signals_.set_signal(wire_id_+1,piw::dataqueue_t());
    remove();

    if(!impl_->active_.head())
    {
//        pic::logmsg() << "Shutting down all events.";
        for(unsigned i=0; i<impl_->poly_voices_; i++)
        {
            if(impl_->notes_running_[i])
            {
//               pic::logmsg() << "Shutting down poly output wire:" << i;
               impl_->output_wires_[i]->shutdown(t);
               impl_->notes_running_[i]=false;
            }
        }
//        pic::logmsg() << "Supressing clock tick.";
        impl_->tick_suppress(true);
    }

    return true;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// voice functions
//
// create a wires for the output polyphony
// new notes cause an end event and a start event with the same timestamp
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

output_wire::output_wire(fng::finger_t::impl_t *i, const piw::data_t &path): piw::event_data_source_real_t(path), impl_(i)
{
    buffer_ = piw::xevent_data_buffer_t(SIG5(1,2,3,4,5),PIW_DATAQUEUE_SIZE_NORM);
}

void output_wire::add_value(unsigned sig, const piw::data_nb_t &data)
{
    buffer_.add_value(sig,data);
}

void output_wire::startup(const piw::data_nb_t &id,unsigned long long t)
{
#if FINGER_DEBUG>0
    pic::logmsg() << "output wire startup" << (void *)this << " starting " << id << " " << id.time();
#endif
    source_start(0,id,buffer_);
}

void output_wire::source_ended(unsigned seq)
{
}

bool output_wire::shutdown(unsigned long long time)
{
#if FINGER_DEBUG>0
    pic::logmsg() << "output wire shutdown " << (void *)this << " " << time;
#endif // FINGER_DEBUG>0

    source_end(time);
    buffer_ = piw::xevent_data_buffer_t(SIG5(1,2,3,4,5),PIW_DATAQUEUE_SIZE_NORM);
    return true;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
// fingerer interface class
//
//
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

fng::finger_t::finger_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c): impl_(new impl_t(cd,c))
{
}

fng::finger_t::~finger_t()
{
    delete impl_;
}

void fng::finger_t::set_fingering(const fng::fingering_t &f)
{
    impl_->set_poly(f.get_poly());
    impl_->fingering_flipflop_.set(f);
}

piw::cookie_t fng::finger_t::cookie()
{
    return impl_->main_decoder_.cookie();
}
