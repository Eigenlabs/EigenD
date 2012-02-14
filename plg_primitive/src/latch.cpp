
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

#include "latch.h"

#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <picross/pic_ilist.h>
#include <cmath>
#include <map>

namespace
{
   /* 
    * Created by latch_t::impl_t to terminate each wire in the bundle.
    * When we have an event running, we add ourselves to a list of
    * active wires (across all outputs) for servicing during each
    * clock tick.
    */ 
    struct latch_wire_t: 
        piw::wire_t, // used to terminate the upstream wire
        piw::wire_ctl_t, // used to manage the downstream wire
        piw::event_data_sink_t, // receive events from the upstream wire
        piw::event_data_source_real_t, // used to generate events going downstream
        virtual public pic::lckobject_t, // stop this object being paged out
        pic::element_t<> // allow this object to be part of an intrusive list (pic_ilist.h)
    {
        latch_wire_t(prim::latch_t::impl_t *p, const piw::event_data_source_t &);
        ~latch_wire_t() { invalidate(); }

        void wire_closed() { delete this; }
        void invalidate();
        void event_start(unsigned,const piw::data_nb_t &, const piw::xevent_data_buffer_t &);
        bool event_end(unsigned long long);
        void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);
        void process(unsigned, const piw::data_nb_t &, unsigned long long);
        void ticked(unsigned long long f, unsigned long long t);
        void source_ended(unsigned);

        prim::latch_t::impl_t *root_; // just to keep track of the root instance that this wire belongs to
        piw::xevent_data_buffer_t::iter_t input_; // an iterator to interact with the upstream's data source buffer
        piw::xevent_data_buffer_t output_; // buffer that is used by this wire's own data source for the output

        unsigned long long last_from_; // remember the last time the wire ticked
        piw::data_nb_t last_value_[3]; // remember the last values so that the can be send when the latch is triggered
    };
};

/*
 * Implements the output for the data stream that have been processed by the latch,
 * as well as the required handling for the wires, clocking and latch configuration.
 */
struct prim::latch_t::impl_t: 
    piw::root_t, // root_t is the bundle endpoint
    piw::root_ctl_t, // root_ctl_t manages the bundle endpoint together with its wires
    piw::clocksink_t, // act as a clock sink
    virtual pic::lckobject_t // stop this object being paged out
{
    impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c):
        root_t(0),
        up_(0),
        minimum_(0.5f),
        controller_(1)
    {
        // Connect the root control to the downstream cookie.
        connect(c);

        // create our clock sink in the domain passed by the agent.
        cd->sink(this,"latch");

        // Enable the clock sink, but in suppressed mode.
        // Clocksinks can be enabled and disabled only from
        // the slow thread.  For efficiency, an enabled clock
        // can be suppressed.
        //
        // We unsuppress the clock when we get an event,
        // and suppress it when there's nothing to do.
        //
        // Beware: tick_enable(false) will actually start
        // the clock (enable with suppression=false)
        tick_enable(true);

        // Set the clocksink for the root_t bundle endpoint.
        set_clock(this);
    }

    ~impl_t()
    {
        // root_t are objects wich can be tracked via
        // weak pointers.  Invalidate ourself here.
        tracked_invalidate();

        // Clean up
        invalidate();
    }

    /*
     * Called to tick the clocksink (in fast thread)
     */
    void clocksink_ticked(unsigned long long f, unsigned long long t)
    {
        latch_wire_t *w;

        for(w=tickers_.head(); w!=0; w=tickers_.next(w))
        {
            w->ticked(f,t);
        }
    }

    /*
     * Called when an event has started on a wire to allow it to be added 
     * to the list of wires that need to be processed during a clock tick.
     */
    void add_ticker(latch_wire_t *w)
    {
        if(!tickers_.head())
        {
            tick_suppress(false);
        }
        
        tickers_.append(w);
    }

    /*
     * Called when an event ends on a wire to allow it to be removed
     * from the list of wires that need to be processed during a clock tick.
     */
    void del_ticker(latch_wire_t *w)
    {
        tickers_.remove(w);

        if(!tickers_.head())
        {
            tick_suppress(true);
        }
        
    }

    /*
     * Properly clean up the ticking and the created wires.
     */
    void invalidate()
    {
        tick_disable();

        pic::lckmap_t<piw::data_t,latch_wire_t *>::lcktype::iterator ci;
        while((ci=children_.begin())!=children_.end())
        {
            delete ci->second;
        }
    }

    /*
     * Called from root_t to create a new channel for events.  (aka a wire)
     * Each wire can carry a single event at any given time, the event_data_source
     * represents the source of events to which we should subscribe to receive events.
     */
    piw::wire_t *root_wire(const piw::event_data_source_t &es)
    {
       pic::lckmap_t<piw::data_t,latch_wire_t *>::lcktype::iterator ci;

        if((ci=children_.find(es.path()))!=children_.end())
        {
            delete ci->second;
        }

        return new latch_wire_t(this, es);
    }

    /*
     * Called by root_ctl when the root endpoint is being closed.
     */
    void root_closed() { invalidate(); }

    /*
     * Called by root_ctl when a root endpoint is being opened.
     */
    void root_opened() { root_clock(); root_latency(); }

    /*
     * Called when the clocking relationship might have changed.
     */
    void root_clock()
    {
        if(up_)
        {
            remove_upstream(up_);
            up_ = 0;
        }

        up_=get_clock();

        if(up_)
        {
            add_upstream(up_);
        }
    }

    /*
     * Called when the latency might have been changed by the root_ctl.
     */
    void root_latency()
    {
        set_latency(get_latency());
    }

    pic::lckmap_t<piw::data_t, latch_wire_t *>::lcktype children_; // the existing wires in the bundle
    pic::ilist_t<latch_wire_t> tickers_; // the wires that are actively ticking, hence which have an event started
    bct_clocksink_t *up_; // the upstream clocksink

    float minimum_; // the minimum threshold value for the latch
    unsigned controller_; // the controller signal number for the latch
};

/*
 * Latch wire constructor.
 */
latch_wire_t::latch_wire_t(prim::latch_t::impl_t *p, const piw::event_data_source_t &es):
    piw::event_data_source_real_t(es.path()),
    root_(p),
    last_from_(0)
{
    // Store ourself into the root's list of wires
    root_->children_.insert(std::make_pair(path(),this));

    // Connect ourselves to the upstream data source of the bundle endpoint
    root_->connect_wire(this, source());

    // Subscribe to the events of the upstream data source
    subscribe_and_ping(es);
}

/*
 * Static method to be called by the fast thread.
 */
static int __wire_invalidator(void *w_, void *_)
{
    latch_wire_t *w = (latch_wire_t *)w_;
    if(w->root_)
    {
        w->root_->del_ticker(w);
    }
    return 0;
}

/*
 * Properly clean up the wire
 */
void latch_wire_t::invalidate()
{
    // Shut down our own data source, no events will be sent through it anymore.
    source_shutdown();

    // Unsubscribe from the upstream data source
    unsubscribe();

    // Remove ourselves from the active list of wires
    piw::tsd_fastcall(__wire_invalidator, this, 0);

    // Remove ourselves from the root's list of wires
    if(root_)
    {
        root_->children_.erase(path());
        root_ = 0;
    }
}

/* 
 * Called when an event starts on this wire. (fast thread)
 *
 * The seq argument is a sequence number for this event on this wire.
 * It's used for delaying the end of an event, which this bundle never does.
 * 
 * The arguments id and b are the event id and the buffer containing the queues
 * for the different signals.
 */
void latch_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    // Create a new buffer that handles 3 signals 
    output_ = piw::xevent_data_buffer_t(SIG3(1,2,3),PIW_DATAQUEUE_SIZE_NORM);

    // Obtain an iterator to access the incoming data buffer
    input_ = b.iterator();

    // Initialize the last time this wire ticked
    unsigned long long t = id.time();
    last_from_ = t;

    // Reset the last values
    for(int i = 0; i < 3; ++i)
    {
        last_value_[i] = piw::makefloat_nb(0,t);
    }

    // Process any data that has been sent along by the buffer at event start
    piw::data_nb_t d;
    for(int s = 1; s <= 3; ++s)
    {
        if(input_->latest(s,d,t))
        {
            process(s,d,t);
        }
    }

    // Start our own data source (which is an event for downstream agents)
    source_start(seq,id,output_);

    // Add ourselves to the list of wires that have to be processed during a clock tick
    root_->add_ticker(this);
}

/*
 * Sometimes, during an event, a source might go away and be
 * substituted for another. This happens in particular with
 * something like a breath pipe which generates events only
 * when blown. The signal carrying default values will get
 * overridden by the new signal.
 */
void latch_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    input_->set_signal(s,n);
    input_->reset(s,t);
    ticked(last_from_,t);
}

/*
 * Called when an event ends on this wire. (fast thread)
 *
 * This does the required cleanup and sends out the last data that
 * was still present when the event ended.
 */
bool latch_wire_t::event_end(unsigned long long t)
{
    // Process the data that was still present at event end
    ticked(last_from_,t);

    // Remove ourselves from the wires that are processed during a clock tick
    root_->del_ticker(this);

    // End our own event on our own data source for downstream agents
    return source_end(t);
}

/*
 * Called from the clocksink object in our root a on clock tick (fast thread)
 *
 * The arguments from and to are the timestamps. Our duty is to process all
 * data up to and including 'to'
 */
void latch_wire_t::ticked(unsigned long long f, unsigned long long t)
{
    last_from_ = t;

    piw::data_nb_t d;
    unsigned s;

    while(input_->next(SIG3(1,2,3),s,d,t))
    {
        process(s,d,t);
    }
}

/*
 * Received when the upstream data source has ended.
 */
void latch_wire_t::source_ended(unsigned seq)
{
    // Tell the event data sink that we're done with it
    event_ended(seq);
}

/*
 * This implements the actual latch logic, it will be called when event data
 * needs to be processed.
 */
void latch_wire_t::process(unsigned s, const piw::data_nb_t &d, unsigned long long t)
{
    if(s == root_->controller_)
    {
        last_value_[s-1] = d;
        output_.add_value(s,d);
    }
    else
    {
        if(fabs(last_value_[root_->controller_-1].as_norm()) >= root_->minimum_)
        {
            last_value_[s-1] = d;
            output_.add_value(s,d);
        }
        else
        {
            output_.add_value(s,last_value_[s-1].restamp(piw::tsd_time()));
        }
    }
}

/*
 * Static methods that can be called from the fast thread.
 */
static int __set_minimum(void *i_, void *m_)
{
    prim::latch_t::impl_t *i = (prim::latch_t::impl_t *)i_;
    float m = *(float *)m_;
    i->minimum_ = m;
    return 0;
}

static int __set_controller(void *i_, void *c_)
{
    prim::latch_t::impl_t *i = (prim::latch_t::impl_t *)i_;
    unsigned c = *(unsigned *)c_;
    i->controller_ = c;
    return 0;
}

/*
 * See latch.h for the documentation on these methods
 */
piw::cookie_t prim::latch_t::cookie()
{
    return piw::cookie_t(impl_);
}

prim::latch_t::latch_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &cookie): impl_(new impl_t(cd, cookie))
{
}

prim::latch_t::~latch_t()
{
    delete impl_;
}

void prim::latch_t::set_minimum(float m)
{
    // Ensure that this actual state change is executed in the fast thread.
    piw::tsd_fastcall(__set_minimum,impl_,&m);
}

void prim::latch_t::set_controller(unsigned c)
{
    if(c<1) return;
    // Ensure that this actual state change is executed in the fast thread.
    piw::tsd_fastcall(__set_controller,impl_,&c);
}

