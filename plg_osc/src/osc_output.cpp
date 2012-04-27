
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

#include "osc_output.h"
#include "osc_transport.h"
#include <piw/piw_keys.h>

#include <lib_lo/lo/lo.h>
#include <map>

#define IN_KEY 1

// 
// Read the Roadmap in plg_osc/Roadmap for some background information
//

namespace
{
    struct osc_wire_t;

    //
    // An output.  Terminates a bundle and transmits all events on the OSC server
    // using the given name.
    //

    struct osc_output_t: piw::root_t // root_t is the bundle endpoint.
    {
        // construct the bundle termination.  Use prefix as the OSC port.

        osc_output_t(osc_plg::osc_server_t::impl_t *server, const std::string &prefix, bool fake_key, unsigned signals);
        ~osc_output_t();

        // called from root_t to create a new channel for events.  (aka a wire)
        // Each wire can carry a single event at any given time.
        // the event_data_source represents the source of events to which we
        // should subscribe to receive events.

        piw::wire_t *root_wire(const piw::event_data_source_t &);

        // called when upstream connects to us
        void root_opened();
        // called when upstream disconnects
        void root_closed();
        // called when upsream changes its clock
        void root_clock();

        void root_latency();

        // return a cookie used to connect to this termination
        // a cookie is just a weak pointer.
        piw::cookie_t cookie() { return piw::cookie_t(this); }

        // our server
        osc_plg::osc_server_t::impl_t *server_;
        // our OSC prefix
        std::string prefix_;
        // whether to fake a key channel
        bool fake_key_;
        // number of data signals to output
        unsigned signals_;
        // upstreams clock node
        bct_clocksink_t *upstream_;

        // a list of wires we've created
        std::vector<osc_wire_t *> wires_;
    };

    //
    // Created by osc_output to terminate each wire in the bundle.
    // When we have an event running, we add ourselves to a list
    // of active wires (across all outputs) for servicing during 
    // each clock tick.
    //

    struct osc_wire_t:
        piw::wire_t,  // used to terminate the wire
        piw::event_data_sink_t, // event subscriber, used to receive events
        pic::element_t<>, // allow this object to be part of an intrusive list (pic_ilist.h)
        virtual pic::lckobject_t // stop this object being paged out
    {
        // construct the wire.  Need the source to subscribe to, and the channel number.
        osc_wire_t(osc_output_t *output, unsigned index, const piw::event_data_source_t &es);
        ~osc_wire_t();

        // called via wire_t if upstream disconnects this wire.
        void wire_closed();

        // called on clock tick to flush our data queues (in fast thread)
        void ticked(unsigned long long from, unsigned long long to);

        // called when event starts (in fast thread)
        void event_start(unsigned seq, const piw::data_nb_t &id,const piw::xevent_data_buffer_t &b);
        // called when event ends (in fast thread)
        bool event_end(unsigned long long);
        // called when one signal source is substitued for another during the event
        void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);

        // send OSC data.  sends one packet, with the data which is current for time
        void send(unsigned long long time);

        // our output
        osc_output_t *output_;

        // our channel number
        unsigned index_;

        // iterator used to get data values in sequence across all signals
        piw::xevent_data_buffer_t::iter_t iterator_;

        // current event ID as a string, using a dataholder ensures that at construction
        // the allocation doesn't happen in another thread than the fast thread,
        // it has really nothing stored in it until it's set
        piw::dataholder_nb_t id_string_;

        // our full OSC path (including agent, output name, and channel number)
        char osc_path_[64];

        // the last time data was processed
        unsigned long long last_processed_;
    };

};

//
// The OSC server itself.
//
// The server creates outputs.  It also acts as a common clocksink
// for all the outputs, and services all active outputs on each tick
//

struct osc_plg::osc_server_t::impl_t:
    piw::clocksink_t, // act as a clock sink
    osc_thread_t      // act as OSC server
{
    // construct the OSC server, given the agent name and a clock domain from the agent
    impl_t(piw::clockdomain_ctl_t *d, const std::string &agent);
    ~impl_t();

    // called to tick the clocksink (in fast thread)
    void clocksink_ticked(unsigned long long f, unsigned long long t);

    // called by agent to create an output port in the server.
    piw::cookie_t create_output(const std::string &prefix, bool fake_key, unsigned signals);

    // remove the output
    void remove_output(const std::string &prefix);

    // remove a wire from the slow thread
    void deactivate_wire_slow(osc_wire_t *w);

    // add and remove wires from the fast thread.
    void activate_wire_fast(osc_wire_t *w);
    void deactivate_wire_fast(osc_wire_t *w);

    // all our outputs
    std::map<std::string,osc_output_t *> outputs_;

    // list of active wires (fast thread only)
    // this is intrusive list.
    pic::ilist_t<osc_wire_t> active_wires_;

    // the decimation rate in micro seconds
    unsigned decimation_;
};

osc_wire_t::osc_wire_t(osc_output_t *output, unsigned index, const piw::event_data_source_t &es): output_(output), index_(index), last_processed_(0)
{
    // build our URL.  Something like /keyboard_1/key/1
    PIC_ASSERT(output_->server_->build_channel_url(osc_path_,sizeof(osc_path_),output_->prefix_,index));

    // add ourself to the output
    output_->wires_[index_] = this;

    // subscribe our event_data_sink to the wires event source
    // if there's currently an event running, cause an event_started callback
    subscribe_and_ping(es);
}

osc_wire_t::~osc_wire_t()
{
    // disconnect this wire.
    disconnect();

    // unsubscribe the event data sink
    unsubscribe();

    // get us out of the active wire list
    output_->server_->deactivate_wire_slow(this);

    // remove from the output
    output_->wires_[index_] = 0;
}

//
// Called when an event starts on this wire. (fast thread)
//
// The seq argument is a sequence number for this event on this wire.
// It's used for delaying the end of an event, which this bundle never does.
// 
// The arguments id and b are the event id and the buffer containing the queues
// for the different signals.
//

void osc_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    // add us to the active list and enable the clock tick if necessary
    output_->server_->activate_wire_fast(this);

    // create an iterator
    // an iterator keeps track of a read position for each signal
    // in a buffer and lets us retrieve changes by time across
    // all the signals
    iterator_ = b.iterator();

    // build the string version of the event ID
    pic::msg_t msg;
    msg << id;
    id_string_.set_nb(piw::makestring_nb(msg.str().c_str(),id.time()));

    // send the initial values, at event start time
    send(id.time());
}

//
// Called from the core object on clock tick (fast thread)
//
// The arguments from and to are the timestamps. Our duty is to emit all
// data up to and including 'to'
//

void osc_wire_t::ticked(unsigned long long from, unsigned long long to)
{
    //
    // normal data_t objects are reference counted in a thread safe way.
    // data_nb_t objects are not thread safe, and are for fast thread only.
    // be careful.
    //
    piw::data_nb_t d;
    unsigned s;

    //
    // build a mask of the signals we want, ie, 4 signals will be 00001111
    //

    unsigned long long mask = (1ULL<<(output_->signals_))-1ULL;

    // get the signal number and value of the next change

    while(iterator_->next(mask,s,d,to))
    {
        // send the current values for the change time.
        // this will also reset the iterator so it wont
        // return any other changes for that time.
        send(d.time());
    }
}

//
// Send an OSC packet.
// Given a time, finds the latest data value up to and including
// the time for each signal in our event. Reset the iterator to
// just after that time for each signal so we dont emit more
// than 1 message for the same time.
//

void osc_wire_t::send(unsigned long long t)
{
    // don't send data until the decimation interval has expired
    // note that this is only done on processing real performance data
    // and not the data that's synthetically generated, for instance
    // at event end
    if(output_->server_->decimation_ &&
       last_processed_ + output_->server_->decimation_ > t)
    {
        return;
    }

    // initialize a new OSC message
    lo_message msg = lo_message_new();
    piw::data_nb_t d;

    // add the event id
    if(!id_string_.is_empty())
    {
        lo_message_add(msg,"s",id_string_.get().as_string());
    }

    // add the signal values
    for(unsigned i=1;i<=output_->signals_;i++)
    {
        // check for the last bit of data on the signal 
        // before or at time t
        if(iterator_->latest(i,d,t))
        {
            float column, row, course, key;
            piw::hardness_t hardness;
            if(IN_KEY==i && piw::decode_key(d,0,&column,&row,0,&course,&key,&hardness))
            {
                // fake the key number
                if(output_->fake_key_)
                {
                    lo_message_add(msg,"f",column);
                    lo_message_add(msg,"f",row);
                    lo_message_add(msg,"f",course);
                    lo_message_add(msg,"f",key);
                    lo_message_add(msg,"i",hardness);
                }
            }
            else
            {
                // output it as a float, in denormalised form.
                // data values are sent in denormalised form
                // ie, 345 hz, along with bounding information
                // which allows one to convert it to normalised
                // -1 -> 0 -> 1 or 0 -> 1 form.
                // Non numeric values have an arbitrary normalised form.
                // this function will use that to create a numeric
                // form if necessary.
                lo_message_add(msg,"f",d.as_denorm_float());
            }
        }

        // reset the signal in the iterator
        iterator_->reset(i,t+1);
    }

    // send the message
    output_->server_->osc_send_fast(osc_path_,msg);
    lo_message_free(msg);
    
    // store the last processing time for each wire
    last_processed_ = t;
}

//
// Called when an event end on this wire. (fast thread)
//
// This does the required cleanup and send out the last data that
// was still present when the event ended.
//

bool osc_wire_t::event_end(unsigned long long t)
{
    // get us off the active queue.
    output_->server_->deactivate_wire_fast(this);

    // send the last values
    send(t);

    // clear the event id
    id_string_.clear_nb();

    // clear the iterator
    iterator_.clear();

    // output end-of-event message
    // null event id, all 0 data
    lo_message msg = lo_message_new();
    lo_message_add(msg,"s","");

    for(unsigned i=1;i<=output_->signals_;i++)
    {
        if(IN_KEY==i && output_->fake_key_)
        {
            lo_message_add(msg,"f",0.f);
            lo_message_add(msg,"f",0.f);
            lo_message_add(msg,"f",0.f);
            lo_message_add(msg,"f",0.f);
            lo_message_add(msg,"i",0);
        }
        else
        {
            lo_message_add(msg,"f",0.f);
        }
    }

    output_->server_->osc_send_fast(osc_path_,msg);
    lo_message_free(msg);

    return true;
}

//
// Sometimes, during an event, a source might go away and be
// substituted for another. This happens in particular with
// something like a breath pipe which generates events only
// when blown. The signal carrying default values wil get
// overridden by the BP signal.
//

void osc_wire_t::event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &oq,const piw::dataqueue_t &nq)
{
    // replace the data queue for the signal in our iterator
    iterator_->set_signal(s,nq);
    // reset to the correct time
    iterator_->reset(s,t);
    // send an OSC packet
    send(t);
}

// called when upstream no longer wants this wire.
// just delete ourself, the dtor will take care
// of the rest.

void osc_wire_t::wire_closed()
{
    delete this;
}

//
// construct an output
// The output is the root of a bundle, responsible for creating carrier wires
// and dealing with clocks.
//

osc_output_t::osc_output_t(osc_plg::osc_server_t::impl_t *server, const std::string &prefix, bool fake_key, unsigned signals):
  piw::root_t(0), server_(server), prefix_(prefix), fake_key_(fake_key), signals_(signals), upstream_(0)
{
    // add ourself to the list of outputs in the server.
    server_->outputs_.insert(std::make_pair(prefix_,this));
}

osc_output_t::~osc_output_t()
{
    // root_t are objects wich can be tracked via
    // weak pointers.  Invalidate ourself here.

    tracked_invalidate();

    // disconnect the bundle root.
    disconnect();

    // remove ourself from the server
    server_->outputs_.erase(prefix_);

    // erase all wires
    for(unsigned i=0;i<wires_.size();i++)
    {
        if(wires_[i])
        {
            delete wires_[i];
        }
    }
}

//
// Called from the root when upstream needs another carrier wire.
//

piw::wire_t *osc_output_t::root_wire(const piw::event_data_source_t &es)
{
    unsigned i = 0;

    // find a free slot in the vector 

    for(i=0;i<wires_.size();i++)
    {
        if(!wires_[i])
        {
            goto found_slot;
        }
    }

    // or resize the vector

    wires_.resize(i+1);
    wires_[i] = 0;

found_slot:

    // construct the carrier.
    // it will add itself to the vector

    return new osc_wire_t(this,i,es);
}

// called when upstream disconnects.

void osc_output_t::root_closed()
{
    // the dtor will handle everything.
    delete this;
}

// called when the upstream clock 
// changes.  Each bundle can have
// one clock.  The server object
// is our clock.

void osc_output_t::root_clock()
{
    // fetch upstreams clock
    bct_clocksink_t *c = get_clock();

    if(c!=upstream_)
    {
        // remove the relationship between
        // old upstream clock and our server

        if(upstream_)
        {
            server_->remove_upstream(upstream_);
        }

        upstream_=c;

        // make the new relationship.  This
        // ensures we will tick after upstream
        // ticks and has filled our queues

        if(upstream_)
        {
            server_->add_upstream(upstream_);
        }
    }
}

//
// Called when upstream connects to us.
// just initialise the clock.
//

void osc_output_t::root_opened()
{
    root_clock();
}

void osc_output_t::root_latency()
{
}

//
// main server initialisation
//

osc_plg::osc_server_t::impl_t::impl_t(piw::clockdomain_ctl_t *d, const std::string &a): osc_thread_t(a), decimation_(0)
{
    // start the OSC threads.
    osc_startup();

    // create our clock sink in the domain passed by the agent.
    d->sink(this,"osc server");

    // enable the clock sink, but in suppressed mode.
    // clocksinks can be enabled and disabled only from
    // the slow thread.  For efficiency, an enabled clock
    // can be suppressed.

    // we unsuppress the clock when we get an event,
    // and suppress it when there's nothing to do.

    // beware: tick_enable(false) will actually start
    // the clock (enable with suppression=false)
    // sigh.

    tick_enable(true);
}

// 
// tear down the server
//

osc_plg::osc_server_t::impl_t::~impl_t()
{
    // shutdown the OSC thread.
    osc_shutdown();

    // disable the clock sink
    tick_disable();

    // and destroy it.
    close_sink();

    // erase all outputs
    std::map<std::string,osc_output_t *>::iterator i;

    while((i=outputs_.begin()) != outputs_.end())
    {
        // the dtor of each output will remove it from the list
        delete i->second;
    }
}

// create a new output

piw::cookie_t osc_plg::osc_server_t::impl_t::create_output(const std::string &prefix, bool fake_key, unsigned signals)
{
    std::map<std::string,osc_output_t *>::iterator i;

    // check for a duplicate output.
    // return a null cookie in that case (an event bucket)

    if((i=outputs_.find(prefix))!=outputs_.end())
    {
        return piw::cookie_t();
    }

    osc_output_t *o = new osc_output_t(this,prefix,fake_key,signals);
    return o->cookie();
}

// remove the output

void osc_plg::osc_server_t::impl_t::remove_output(const std::string &prefix)
{
    std::map<std::string,osc_output_t *>::iterator i;

    // just delete it if it exists.
    if((i=outputs_.find(prefix))!=outputs_.end())
    {
        osc_output_t *o = i->second;
        // dtor will erase it
        delete o;
    }

}

// function called in the fast thread to remove a wire 
// from the active queue

static int remover__(void *impl__, void *wire__)
{
    osc_plg::osc_server_t::impl_t *impl = (osc_plg::osc_server_t::impl_t *)impl__;
    osc_wire_t *wire = (osc_wire_t *)wire__;
    impl->active_wires_.remove(wire);
    return 0;
}

void osc_plg::osc_server_t::impl_t::deactivate_wire_slow(osc_wire_t *w)
{
    // call the remove function in the fast thread.
    piw::tsd_fastcall(remover__,this,w);
}

// fast thread wire activation.

void osc_plg::osc_server_t::impl_t::activate_wire_fast(osc_wire_t *w)
{
    // list is empty; we will unsuppress (start) the clocksink ticking

    if(!active_wires_.head())
    {
        tick_suppress(false);
    }

    // append to the list
    active_wires_.append(w);
}

// fast thread wire deactivation

void osc_plg::osc_server_t::impl_t::deactivate_wire_fast(osc_wire_t *w)
{
    // dont worry about shutting down the clock, 
    // the tick will shut the clock down if the active list is empty

    active_wires_.remove(w);
}

void osc_plg::osc_server_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
    osc_wire_t *w;

    // iterate the active list, ticking each active wire
    // ticks aren't allowed to manage the list
    for(w=active_wires_.head(); w!=0; w=active_wires_.next(w))
    {
        w->ticked(f,t);
    }

    // shut down the clock if necessary
    if(!active_wires_.head())
    {
        tick_suppress(true);
    }
}

/*
 * Static methods that can be called from the fast thread.
 */

static int __set_decimation(void *i_, void *d_)
{
    osc_plg::osc_server_t::impl_t *i = (osc_plg::osc_server_t::impl_t *)i_;
    unsigned d = *(unsigned *)d_;
    i->decimation_ = d*1000;
    return 0;
}

// 
// pimpl wrapper functions
//

osc_plg::osc_server_t::osc_server_t(piw::clockdomain_ctl_t *d, const std::string &agent): impl_(new impl_t(d,agent))
{
}

osc_plg::osc_server_t::~osc_server_t()
{
    delete impl_;
}

piw::cookie_t osc_plg::osc_server_t::create_output(const std::string &prefix,bool fake_key,unsigned signals)
{
    return impl_->create_output(prefix,fake_key,signals);
}

void osc_plg::osc_server_t::remove_output(const std::string &prefix)
{
    impl_->remove_output(prefix);
}

void osc_plg::osc_server_t::set_decimation(unsigned decimation)
{
    piw::tsd_fastcall(__set_decimation,impl_,&decimation);
}
