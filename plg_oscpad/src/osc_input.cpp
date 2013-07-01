
#include "osc_input.h"
#include "osc_client.h"
#include "osc_server.h"
#include "live_osc.h"
#include "live_model.h"
#include <piw/piw_cfilter.h>
#include <piw/piw_data.h>
#include <piw/piw_status.h>

#include <picross/pic_ref.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <picross/pic_ref.h>

#include <lib_lo/lo/lo.h>

#define OUT_LIGHT 1
#define OUT_MASK SIG1(OUT_LIGHT)

#define IN_KEY 1
#define IN_PRESSURE 2
#define IN_ROLL 3
#define IN_YAW 4



namespace oscpad_plg
{



//////////// osc_input_t::impl_t  declaration
struct osc_wire_t:
    piw::wire_t, // used to terminate the upstream wire
    piw::wire_ctl_t, // used to manage the downstream wire
    piw::event_data_sink_t, // receive events from the upstream wire
    piw::event_data_source_real_t, // used to generate events going downstream
    virtual public pic::lckobject_t, // stop this object being paged out
    pic::element_t<> // allow this object to be part of an intrusive list (pic_ilist.h)
{
    osc_wire_t(osc_input_t::impl_t *p, const piw::event_data_source_t &);
    ~osc_wire_t() { invalidate(); }

    void wire_closed() { delete this; }
    void invalidate();
    void event_start(unsigned,const piw::data_nb_t &, const piw::xevent_data_buffer_t &);
    bool event_end(unsigned long long);
    void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);
    void process(unsigned, const piw::data_nb_t &, unsigned long long);
    void ticked(unsigned long long f, unsigned long long t);
    void source_ended(unsigned);

    osc_input_t::impl_t *root_; // just to keep track of the root instance that this wire belongs to
    piw::xevent_data_buffer_t::iter_t input_; // an iterator to interact with the upstream's data source buffer
    piw::xevent_data_buffer_t output_; // buffer that is used by this wire's own data source for the output

    unsigned long long last_from_; // remember the last time the wire ticked
};

struct light_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::counted_t
{
	light_wire_t(osc_input_t::impl_t *impl);
	~light_wire_t();
	void source_ended(unsigned seq);
	void updateLights();

	osc_input_t::impl_t *root_;
	piw::xevent_data_buffer_t output_;
};



//////////// osc_input_t::impl_t  declaration
struct osc_input_t::impl_t :
	    piw::root_t, // root_t is the bundle endpoint
	    piw::root_ctl_t, // root_ctl_t manages the bundle endpoint together with its wires
	    piw::clocksink_t, // act as a clock sink
	    virtual pic::lckobject_t // stop this object being paged out
{
    impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c, const std::string &sendhost, const std::string &sendport, const std::string &recvport);
    ~impl_t();
     void clocksink_ticked(unsigned long long f, unsigned long long t);
    void add_ticker(osc_wire_t *w);
    void del_ticker(osc_wire_t *w);
    void invalidate();
    piw::wire_t *root_wire(const piw::event_data_source_t &es);
    void root_closed();
    void root_opened();
    void root_clock();
    void root_latency();
    live_model_t& model() { return model_;}

    pic::lckmap_t<piw::data_t, osc_wire_t *>::lcktype children_; // the existing wires in the bundle
    pic::ilist_t<osc_wire_t> tickers_; // the wires that are actively ticking, hence which have an event started
    bct_clocksink_t *up_; // the upstream clocksink
    osc_server_t *server_;
    osc_client_t *client_;
    pic::ref_t<light_wire_t> light_wire_;
    live_model_t model_;
};


struct model_update
{
   	model_update(live_model_t& m, int t, int c,live_model_t::clip_state_t s): m_(m), t_(t), c_(c), s_(s) {}
   	live_model_t& m_;
	int t_,  c_;
	live_model_t::clip_state_t s_;
};


static int __updateModel(void *f_, void * d_)
{
	light_wire_t *f = (light_wire_t *)f_;
	model_update *d = (model_update*) d_;
	d->m_.changeState(d->t_,d->c_,d->s_);
	f->updateLights();
	delete d;
	return 0;
}



///////////// clip_info_handler_t
struct clip_info_handler_t : public live_clip_info_handler_t
{
	clip_info_handler_t(light_wire_t *wire) : wire_(wire){}

	live_model_t::clip_state_t stateToModel(state_t s)
	{
		switch(s)
		{
			case live_clip_info_handler_t::EMPTY : return live_model_t::CLIP_EMPTY;
			case live_clip_info_handler_t::HAS_CLIP: return live_model_t::CLIP_HAS_CLIP;
			case live_clip_info_handler_t::PLAYING: return live_model_t::CLIP_PLAYING;
			case live_clip_info_handler_t::TRIGGERED: return live_model_t::CLIP_TRIGGERED;
		}
		return live_model_t::CLIP_EMPTY;
	}
	void processClip(int track,int clip, state_t state)
	{
//		pic::logmsg() << "clip_info_handler_t::processClip track: " << track << "clip:" << clip << "state:" << state;
		tsd_fastcall(__updateModel,wire_,new model_update(wire_->root_->model(), track,clip,stateToModel(state)) );
	}
	light_wire_t *wire_;
};

///////////// clip_info_handler_t
struct track_info_handler_t : public live_track_info_handler_t
{
	track_info_handler_t(light_wire_t *wire) : wire_(wire){}

	live_model_t::clip_state_t stateToModel(state_t s)
	{
		switch(s)
		{
			case EMPTY : return live_model_t::CLIP_EMPTY;
			case HAS_CLIP: return live_model_t::CLIP_HAS_CLIP;
			case PLAYING: return live_model_t::CLIP_PLAYING;
			case TRIGGERED: return live_model_t::CLIP_TRIGGERED;
		}
		return live_model_t::CLIP_EMPTY;
	}
	void processClip(int track,bool armed, int clip, state_t state, float length)
	{
//		pic::logmsg() << "track_info_handler_t::processClip track: " << track << "clip:" << clip << "state:" << state << "length" << length;
		if(length>=1.0)
		{
			tsd_fastcall(__updateModel,wire_,new model_update(wire_->root_->model(), track,clip,live_model_t::CLIP_RECORDING));
		}
		{
			tsd_fastcall(__updateModel,wire_,new model_update(wire_->root_->model(), track,clip,stateToModel(state)) );
		}
	}
	light_wire_t *wire_;
};


/// osc_input_t::impl_t implementation
osc_input_t::impl_t::impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c,
							const std::string &send_host,  const std::string &send_port, const std::string &recv_port) :
	root_t(0), up_(0),
	server_( new osc_server_t(recv_port)), client_(new osc_client_t(send_host,send_port))
{
	connect(c);
	cd->sink(this,"oscinput");
	tick_enable(true);

	set_clock(this);
	light_wire_ = pic::ref(new light_wire_t(this));
	connect_wire(light_wire_.ptr(), light_wire_->source());

	client_->osc_startup();
	server_->osc_startup();
	server_->registerHandler(new clip_info_handler_t(light_wire_.ptr()));
	server_->registerHandler(new track_info_handler_t(light_wire_.ptr()));
}

osc_input_t::impl_t::~impl_t()
{
	client_->osc_shutdown();
	server_->osc_shutdown();
	tracked_invalidate();
	invalidate();
}

/*
 * Called to tick the clocksink (in fast thread)
 */
void osc_input_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
	osc_wire_t *w;

	for(w=tickers_.head(); w!=0; w=tickers_.next(w))
	{
		w->ticked(f,t);
	}
}

/*
 * Called when an event has started on a wire to allow it to be added
 * to the list of wires that need to be processed during a clock tick.
 */
void osc_input_t::impl_t::add_ticker(osc_wire_t *w)
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
void osc_input_t::impl_t::del_ticker(osc_wire_t *w)
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
void osc_input_t::impl_t::invalidate()
{
	tick_disable();

	pic::lckmap_t<piw::data_t,osc_wire_t *>::lcktype::iterator ci;
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
piw::wire_t *osc_input_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
   pic::lckmap_t<piw::data_t,osc_wire_t *>::lcktype::iterator ci;

	if((ci=children_.find(es.path()))!=children_.end())
	{
		delete ci->second;
	}

	return new osc_wire_t(this, es);
}

/*
 * Called by root_ctl when the root endpoint is being closed.
 */
void osc_input_t::impl_t::root_closed() { invalidate(); }

/*
 * Called by root_ctl when a root endpoint is being opened.
 */
void osc_input_t::impl_t::root_opened() { root_clock(); root_latency(); }

/*
 * Called when the clocking relationship might have changed.
 */
void osc_input_t::impl_t::root_clock()
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
void osc_input_t::impl_t::root_latency()
{
	set_latency(get_latency());
}

int modelToLightState(live_model_t::clip_state_t s)
{
	switch(s)
	{
		case live_model_t::CLIP_EMPTY : return BCTSTATUS_OFF;
		case live_model_t::CLIP_HAS_CLIP: return BCTSTATUS_INACTIVE;
		case live_model_t::CLIP_PLAYING: return BCTSTATUS_ACTIVE;
		case live_model_t::CLIP_TRIGGERED: return BCTSTATUS_MIXED;
		case live_model_t::CLIP_RECORDING: return BCTSTATUS_BLINK;
	}
	return BCTSTATUS_BLINK;
}


void updateLightBuffer(live_model_t &theModel, piw::xevent_data_buffer_t& outputbuffer)
{
    piw::statusset_t status;

    std::list<live_model_t::clip_t*> clips=theModel.clips();
    std::list<live_model_t::clip_t*>::iterator c;

    for(c=clips.begin();c!=clips.end(); c++)
    {
    	live_model_t::clip_t *clip=*c;
//		pic::logmsg() << "updateLightBuffer: clip:" << clip->clip_ << "track:" << clip->track_ << " x,y " << 2-(clip->clip_+1) << "," << (clip->track_+1);
        status.insert(piw::statusdata_t(false,piw::coordinate_t(clip->clip_+1,(clip->track_+1)),modelToLightState(clip->state_)));
	}
    piw::data_nb_t buffer = piw::statusbuffer_t::make_statusbuffer(status);
    outputbuffer.add_value(OUT_LIGHT,buffer);
}



/// osc_wire_t

osc_wire_t::osc_wire_t(osc_input_t::impl_t  *p, const piw::event_data_source_t &es):
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
	osc_wire_t *w = (osc_wire_t *)w_;
    if(w->root_)
    {
        w->root_->del_ticker(w);
    }
    return 0;
}

/*
 * Properly clean up the wire
 */
void osc_wire_t::invalidate()
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
void osc_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    // Create a new buffer that handles 3 signals
    output_ = piw::xevent_data_buffer_t(OUT_MASK,PIW_DATAQUEUE_SIZE_NORM);

    // Obtain an iterator to access the incoming data buffer
    input_ = b.iterator();

    // Initialize the last time this wire ticked
    unsigned long long t = id.time();
    last_from_ = t;

//    // Process any data that has been sent along by the buffer at event start
//    piw::data_nb_t d;
//    for(int s = 1; s <= 3; ++s)
//    {
//        if(input_->latest(s,d,t))
//        {
//            process(s,d,t);
//        }
//    }

	piw::data_nb_t d;
	if(input_->latest(IN_KEY,d,t))
	{
		process(IN_KEY,d,t);
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
void osc_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
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
bool osc_wire_t::event_end(unsigned long long t)
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
void osc_wire_t::ticked(unsigned long long f, unsigned long long t)
{
    last_from_ = t;

//    piw::data_nb_t d;
//    unsigned s;
//
//    while(input_->next(SIG3(1,2,3),s,d,t))
//    {
//        process(s,d,t);
//    }

	piw::data_nb_t d;
	unsigned s;
	while(input_->next(SIG1(IN_KEY),s,d,t))
	{
		process(s,d,t);
	}

}

/*
 * Received when the upstream data source has ended.
 */
void osc_wire_t::source_ended(unsigned seq)
{
    // Tell the event data sink that we're done with it
    event_ended(seq);
}



void osc_wire_t::process(unsigned s, const piw::data_nb_t &d, unsigned long long t)
{
//	pic::logmsg() << "osc_wire_t::process:" << s;
    float column, row, course, key;
    piw::hardness_t hardness;
    if(IN_KEY==s && piw::decode_key(d,&column,&row,&course,&key,&hardness))
    {
    	int track = (int) row - 1;
    	int clip = (int) column - 1;

//    	pic::logmsg() << "osc_wire_t::process " << s  << "track" << track << "clip:" << clip;
    	int prevArmed = root_->model().armTrack(track);
    	if (track!=prevArmed)
    	{
    		if(prevArmed!=-1)
    		{
    		   	root_->client_->send(new live_arm_message_t(prevArmed,false));
    		}
		   	root_->client_->send(new live_arm_message_t(track,true));
    	}

    	live_model_t::clip_t *lastRec=root_->model().getRecordingClip();
    	if(lastRec!=NULL)
    	{
    		root_->client_->send(new live_clip_info_message_t(lastRec->track_,lastRec->clip_));
    	}

    	root_->client_->send(new live_clip_slot_message_t(track,clip));
    }
}


light_wire_t::light_wire_t(	osc_input_t::impl_t *impl) : event_data_source_real_t(piw::pathone(1,0)), root_(impl)
{
	unsigned long long t = piw::tsd_time();
	output_ = piw::xevent_data_buffer_t(OUT_MASK,PIW_DATAQUEUE_SIZE_TINY);
	source_start(0,piw::pathone_nb(1,t),output_);
}

light_wire_t::~light_wire_t()
{
	source_shutdown();
}

void light_wire_t::source_ended(unsigned seq)
{
	source_end(piw::tsd_time());
}

void light_wire_t::updateLights()
{
	updateLightBuffer(root_->model(),output_);
}



//////////// osc_input_t
osc_input_t::osc_input_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &output,const std::string &sendhost, const std::string &sendport, const std::string &recvport)
	: impl_(new impl_t(d,output,sendhost,sendport,recvport))
{
}

osc_input_t::~osc_input_t()
{
	delete impl_;
}

piw::cookie_t osc_input_t::cookie()
{
    return piw::cookie_t(impl_);
}


}; //namespace oscpad_plg


