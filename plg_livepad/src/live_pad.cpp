
#include "live_pad.h"
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
#include <piw/piw_keys.h>
#include <picross/pic_ref.h>

#include <lib_lo/lo/lo.h>

#define OUT_LIGHT 1
#define OUT_MASK SIG1(OUT_LIGHT)

#define IN_KEY 1
#define IN_PRESSURE 2
#define IN_ROLL 3
#define IN_YAW 4



namespace livepad_plg
{

//////////// _pad_t::impl_t  declaration
// this is based on latch implemenation
struct live_wire_t:
    piw::wire_t,
    piw::wire_ctl_t,
    piw::event_data_sink_t,
    piw::event_data_source_real_t,
    virtual public pic::lckobject_t,
    pic::element_t<>
{
    live_wire_t(live_pad_t::impl_t *p, const piw::event_data_source_t &);
    ~live_wire_t() { invalidate(); }

    void wire_closed() { delete this; }
    void invalidate();
    void event_start(unsigned,const piw::data_nb_t &, const piw::xevent_data_buffer_t &);
    bool event_end(unsigned long long);
    void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);
    void process(unsigned, const piw::data_nb_t &, unsigned long long);
    void ticked(unsigned long long f, unsigned long long t);
    void source_ended(unsigned);

    live_pad_t::impl_t *root_;
    piw::xevent_data_buffer_t::iter_t input_;
    piw::xevent_data_buffer_t output_;

    unsigned long long last_from_;
};

struct light_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::counted_t
{
	light_wire_t(live_pad_t::impl_t *impl);
	~light_wire_t();
	void source_ended(unsigned seq);
	void updateLights();

	live_pad_t::impl_t *root_;
	piw::xevent_data_buffer_t output_;
};



//////////// live_pad_t::impl_t  declaration
// this is based on latch, please look there for general comments on eigenD usage
struct live_pad_t::impl_t :
	    piw::root_t,
	    piw::root_ctl_t,
	    piw::clocksink_t,
	    virtual pic::lckobject_t
{
    impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c, const std::string &sendhost, const std::string &sendport, const std::string &recvport);
    ~impl_t();
     void clocksink_ticked(unsigned long long f, unsigned long long t);
    void add_ticker(live_wire_t *w);
    void del_ticker(live_wire_t *w);
    void invalidate();
    piw::wire_t *root_wire(const piw::event_data_source_t &es);
    void root_closed();
    void root_opened();
    void root_clock();
    void root_latency();
    live_model::live_view_t& view() { return *view_;}
    void set_window(unsigned top, unsigned height, unsigned left, unsigned width);
    void refresh();
    void play();
    void stop();
    void play_scene(int scene);
    void shutdown();
    void undo();
    void redo();

    pic::lckmap_t<piw::data_t, live_wire_t *>::lcktype children_;
    pic::ilist_t<live_wire_t> tickers_;
    bct_clocksink_t *up_;
    osc_server_t *server_;
    osc_client_t *client_;
    pic::ref_t<light_wire_t> light_wire_;
    live_model::live_model_t model_;
    live_model::live_view_t *view_;
};


struct model_update
{
   	model_update(live_model::live_view_t& v, osc_client_t *clt, int t, int c,live_model::clip_t::state_t s): v_(v),client_(clt), t_(t), c_(c), s_(s) {}
   	live_model::live_view_t& v_;
	osc_client_t *client_;
	int t_,  c_;
	live_model::clip_t::state_t s_;
};


static int __updateModel(void *f_, void * d_)
{
	light_wire_t *f = (light_wire_t *)f_;
	model_update *d = (model_update*) d_;
//	pic::logmsg() << "__updateModel track: " << d->t_ << "clip:" << d->c_ << "state:" << d->s_;

	d->v_.changeState(d->t_,d->c_,d->s_);
	f->updateLights();

	// we dont get told explicity of end of recording, but if get another clip triggered it should have ended!
	live_model::clip_t *lastRec=d->v_.getRecordingClip();
	if(lastRec!=NULL)
	{
		if (lastRec->track_!=d->t_ && lastRec->clip_ != d->c_)
		{
			d->client_->send(new live_clip_info_message_t(lastRec->track_,lastRec->clip_));
		}
	}

	delete d;
	return 0;
}



///////////// clip_info_handler_t
struct clip_info_handler_t : public live_clip_info_handler_t
{
	clip_info_handler_t(light_wire_t *wire) : wire_(wire){}

	live_model::clip_t::state_t stateToModel(state_t s)
	{
		switch(s)
		{
			case EMPTY : return live_model::clip_t::CLIP_EMPTY;
			case HAS_CLIP: return live_model::clip_t::CLIP_HAS_CLIP;
			case PLAYING: return live_model::clip_t::CLIP_PLAYING;
			case TRIGGERED: return live_model::clip_t::CLIP_TRIGGERED;
		}
		return live_model::clip_t::CLIP_EMPTY;
	}
	void processClip(int track,int clip, state_t state)
	{
//		pic::logmsg() << "clip_info_handler_t::processClip track: " << track << "clip:" << clip << "state:" << state;
		piw::tsd_fastcall(__updateModel,wire_,new model_update(wire_->root_->view(), wire_->root_->client_, track,clip,stateToModel(state)) );
	}
	light_wire_t *wire_;
};

///////////// clip_info_handler_t
struct track_info_handler_t : public live_track_info_handler_t
{
	track_info_handler_t(light_wire_t *wire) : wire_(wire){}

	live_model::clip_t::state_t stateToModel(state_t s)
	{
		switch(s)
		{
			case EMPTY : return live_model::clip_t::CLIP_EMPTY;
			case HAS_CLIP: return live_model::clip_t::CLIP_HAS_CLIP;
			case PLAYING: return live_model::clip_t::CLIP_PLAYING;
			case TRIGGERED: return live_model::clip_t::CLIP_TRIGGERED;
		}
		return live_model::clip_t::CLIP_EMPTY;
	}
	void processClip(int track,bool armed, int clip, state_t state, float length)
	{
		// pic::logmsg() << "track_info_handler_t::processClip track: " << track << "clip:" << clip << "state:" << state << "length" << length;
		if(length>=1.0)
		{
			piw::tsd_fastcall(__updateModel,wire_,new model_update(wire_->root_->view(), wire_->root_->client_, track,clip,live_model::clip_t::CLIP_RECORDING));
		}
		else
		{
			piw::tsd_fastcall(__updateModel,wire_,new model_update(wire_->root_->view(), wire_->root_->client_, track,clip,stateToModel(state)) );
		}
	}
	light_wire_t *wire_;
};


///////////// live_name_clip_handler_t
// this is just used to get the initial info, as we are not interested in names.

struct clip_check
{
	clip_check(live_model::live_view_t& v, int t, int c): v_(v), t_(t), c_(c) {}
	live_model::live_view_t& v_;
	int t_,  c_;
};


static int __checkClip(void *f_, void * d_)
{
	osc_client_t *client = (osc_client_t *)f_;
	clip_check *d = (clip_check*) d_;
//	pic::logmsg() << "__checkClip track: " << d->t_ << "clip:" << d->c_ ;

	if(d->v_.getState(d->t_,d->c_)==live_model::clip_t::CLIP_EMPTY)
	{
		client->send(new live_clip_info_message_t(d->t_,d->c_));
	}

	// we dont get told explicity of end of recording, but is being recorded the other one should have stopped!
	live_model::clip_t *lastRec=d->v_.getRecordingClip();
	if(lastRec!=NULL)
	{
		if (lastRec->track_!=d->t_ && lastRec->clip_ != d->c_)
		{
			client->send(new live_clip_info_message_t(lastRec->track_,lastRec->clip_));
		}
	}

	delete d;
	return 0;
}

struct name_clip_handler_t : public live_name_clip_handler_t
{
	name_clip_handler_t(live_model::live_view_t& view, osc_client_t* client) : client_(client), view_(view){}
	void processClip(int track, int clip, const std::string& name, int colour)
	{
		// pic::logmsg() << "name_clip_handler_t::processClip track: " << track << "clip:" << clip << "name:" << name;
		piw::tsd_fastcall(__checkClip,client_,new clip_check(view_, track,clip) );
	}
	osc_client_t *client_;
	live_model::live_view_t& view_;
};

struct track_check
{
	track_check(live_model::live_view_t& v, int t): v_(v), t_(t) {}
	live_model::live_view_t& v_;
	int t_;
};


static int __checkTrack(void *f_, void * d_)
{
	osc_client_t *client = (osc_client_t *)f_;
	track_check *d = (track_check*) d_;
//	pic::logmsg() << "checkTrack track: " << d->t_;


	// we dont get told explicity of end of recording, but is being recorded the other one should have stopped!
	live_model::clip_t *lastRec=d->v_.getRecordingClip();
	if(lastRec!=NULL)
	{
		client->send(new live_clip_info_message_t(lastRec->track_,lastRec->clip_));
	}

	delete d;
	return 0;
}

struct armed_handler_t : public live_arm_handler_t
{
	armed_handler_t(live_model::live_view_t& view, osc_client_t* client) : client_(client), view_(view){}
	void processArm(int track, bool armed)
	{
		// pic::logmsg() << "armed_handler_t::processArm track: " << track << "armed:" << armed;

		// later model will keep track of track status, but at the moment we are only interested in
		// if a recording has been deactivated
		piw::tsd_fastcall(__checkTrack,client_,new track_check(view_, track) );
	}
	osc_client_t *client_;
	live_model::live_view_t& view_;
};


struct window
{
	window(osc_client_t* c,live_model::live_view_t &v, unsigned top, unsigned height, unsigned left, unsigned width)
		: c_(c), v_(v), t_(top), h_(height), l_(left), w_(width) {};
	osc_client_t *c_;
	live_model::live_view_t &v_;
	int t_,h_,l_,w_;
};


static int __set_window(void *w_, void * d_)
{
	light_wire_t *w=(light_wire_t*) w_;
	window *d= (window*) d_;
	// pic::logmsg() << "__set_window "<< d->t_ << "," << d->h_ << "," << d->l_ << "," << d->w_;
	d->v_.setWindow(d->t_, d->h_, d->l_, d->w_);
	d->c_->send(new live_selection_message_t(d->l_,d->t_, d->w_, d->h_));
	w->updateLights();
	delete d;
    return 0;
}

static int __refresh(void *w_, void * d_)
{
	light_wire_t *w=(light_wire_t*) w_;
	window *d= (window*) d_;
	// pic::logmsg() << "__refresh";
	d->v_.clear();
	d->v_.setWindow(d->v_.clipOffset(), d->v_.clipSize(), d->v_.trackOffset(), d->v_.trackSize());
	d->c_->send(new live_selection_message_t( d->v_.trackOffset(), d->v_.clipOffset(), d->v_.trackSize(), d->v_.clipSize()));
	d->c_->send(new live_name_clip_message_t);
	w->updateLights();

	delete d;
    return 0;
}


struct startup_handler_t : public live_startup_handler_t
{
	startup_handler_t(light_wire_t *wire) : wire_(wire){}
	void process()
	{
		// pic::logmsg() << "armed_handler_t::processArm track: " << track << "armed:" << armed;
		piw::tsd_fastcall(__refresh, wire_, new window(wire_->root_->client_,wire_->root_->view(), 0, 0, 0, 0));
	}

	light_wire_t *wire_;
};


struct shutdown_handler_t : public live_shutdown_handler_t
{
	shutdown_handler_t(light_wire_t *wire) : wire_(wire){}
	void process()
	{
		// pic::logmsg() << "armed_handler_t::processArm track: " << track << "armed:" << armed;
		piw::tsd_fastcall(__refresh, wire_, new window(wire_->root_->client_,wire_->root_->view(), 0, 0, 0, 0));
	}

	light_wire_t *wire_;
};


/// live_pad_t::impl_t implementation
live_pad_t::impl_t::impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c,
							const std::string &send_host,  const std::string &send_port, const std::string &recv_port) :
	root_t(0), up_(0),
	server_( new osc_server_t(recv_port)), client_(new osc_client_t(send_host,send_port)),
	view_(new live_model::live_view_t(model_,0,50,0,25))
{
	connect(c);
	cd->sink(this,"liveinput");
	tick_enable(true);

	set_clock(this);
	light_wire_ = pic::ref(new light_wire_t(this));
	connect_wire(light_wire_.ptr(), light_wire_->source());

	client_->osc_startup();
	server_->osc_startup();
	server_->registerHandler(new clip_info_handler_t(light_wire_.ptr()));
	server_->registerHandler(new track_info_handler_t(light_wire_.ptr()));
	server_->registerHandler(new name_clip_handler_t(view(), client_));
	server_->registerHandler(new armed_handler_t(view(), client_));
	server_->registerHandler(new startup_handler_t(light_wire_.ptr()));
	server_->registerHandler(new shutdown_handler_t(light_wire_.ptr()));
	client_->send(new live_name_clip_message_t());
}

live_pad_t::impl_t::~impl_t()
{
	// pic::logmsg() << "live_pad_t::impl_t::~impl_t";
	client_->osc_shutdown();
	server_->osc_shutdown();
	tracked_invalidate();
	invalidate();
	delete client_;
	delete server_;
	delete light_wire_.ptr();
	light_wire_.assign(0);
	delete view_;
}


void live_pad_t::impl_t::set_window(unsigned top, unsigned height, unsigned left, unsigned width)
{
//	pic::logmsg() << "live_pad_t::impl_t::set_window "<< top << "," << height << "," << left << "," << width;
	piw::tsd_fastcall(__set_window, light_wire_.ptr(), new window(client_,view(), top, height, left, width));
}



void live_pad_t::impl_t::refresh()
{
//	pic::logmsg() << "live_pad_t::impl_t::refresh";
	piw::tsd_fastcall(__refresh, light_wire_.ptr(), new window(client_,view(), 0, 0, 0, 0));
}

void live_pad_t::impl_t::play()
{
//	pic::logmsg() << "live_pad_t::impl_t::play";
	client_->send(new live_play_message_t());
}

void live_pad_t::impl_t::stop()
{
//	pic::logmsg() << "live_pad_t::impl_t::refresh";
	client_->send(new live_stop_message_t());
}

void live_pad_t::impl_t::play_scene(int scene)
{
//	pic::logmsg() << "live_pad_t::impl_t::play_scene" << scene;
	client_->send(new live_play_scene_message_t(scene));
}

void live_pad_t::impl_t::shutdown()
{
	// pic::logmsg() << "live_pad_t::impl_t::shutdown";
	client_->osc_shutdown();
	server_->osc_shutdown();
}

void live_pad_t::impl_t::undo()
{
//	pic::logmsg() << "live_pad_t::impl_t::undo";
	client_->send(new live_undo_message_t());
}


void live_pad_t::impl_t::redo()
{
//	pic::logmsg() << "live_pad_t::impl_t::redo";
	client_->send(new live_redo_message_t());
}




void live_pad_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
	live_wire_t *w;

	for(w=tickers_.head(); w!=0; w=tickers_.next(w))
	{
		w->ticked(f,t);
	}
}

void live_pad_t::impl_t::add_ticker(live_wire_t *w)
{
	if(!tickers_.head())
	{
		tick_suppress(false);
	}

	tickers_.append(w);
}

void live_pad_t::impl_t::del_ticker(live_wire_t *w)
{
	tickers_.remove(w);

	if(!tickers_.head())
	{
		tick_suppress(true);
	}

}

void live_pad_t::impl_t::invalidate()
{
	tick_disable();

	pic::lckmap_t<piw::data_t,live_wire_t *>::lcktype::iterator ci;
	while((ci=children_.begin())!=children_.end())
	{
		delete ci->second;
	}
}

piw::wire_t *live_pad_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
   pic::lckmap_t<piw::data_t,live_wire_t *>::lcktype::iterator ci;

	if((ci=children_.find(es.path()))!=children_.end())
	{
		delete ci->second;
	}

	return new live_wire_t(this, es);
}

void live_pad_t::impl_t::root_closed() { invalidate(); }

void live_pad_t::impl_t::root_opened() { root_clock(); root_latency(); }

void live_pad_t::impl_t::root_clock()
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

void live_pad_t::impl_t::root_latency()
{
	set_latency(get_latency());
}

int modelToLightState(live_model::clip_t::state_t s)
{
	switch(s)
	{
		case live_model::clip_t::CLIP_EMPTY : return BCTSTATUS_OFF;
		case live_model::clip_t::CLIP_HAS_CLIP: return BCTSTATUS_UNKNOWN;
		case live_model::clip_t::CLIP_PLAYING: return BCTSTATUS_ACTIVE;
		case live_model::clip_t::CLIP_TRIGGERED: return BCTSTATUS_ACTIVE;
		case live_model::clip_t::CLIP_RECORDING: return BCTSTATUS_INACTIVE;
	}
	return BCTSTATUS_BLINK;
}


void updateLightBuffer(live_model::live_view_t &view, piw::xevent_data_buffer_t& outputbuffer)
{
    piw::statusset_t status;

    std::list<live_model::clip_t*> clips=view.clips();
    std::list<live_model::clip_t*>::iterator c;

    for(c=clips.begin();c!=clips.end(); c++)
    {
    	live_model::clip_t *clip=*c;
//		pic::logmsg() << "updateLightBuffer: clip:" << clip->clip_ << "track:" << clip->track_ << " x,y " << 2-(clip->clip_+1) << "," << (clip->track_+1);
        status.insert(piw::statusdata_t(false,piw::coordinate_t(clip->clip_-view.clipOffset()+1,clip->track_-view.trackOffset()+1),modelToLightState(clip->state_)));
	}
    piw::data_nb_t buffer = piw::statusbuffer_t::make_statusbuffer(status);
    outputbuffer.add_value(OUT_LIGHT,buffer);
}



live_wire_t::live_wire_t(live_pad_t::impl_t  *p, const piw::event_data_source_t &es):
    piw::event_data_source_real_t(es.path()),
    root_(p),
    last_from_(0)
{
    root_->children_.insert(std::make_pair(path(),this));

    root_->connect_wire(this, source());

    subscribe_and_ping(es);
}

static int __wire_invalidator(void *w_, void *_)
{
	live_wire_t *w = (live_wire_t *)w_;
    if(w->root_)
    {
        w->root_->del_ticker(w);
    }
    return 0;
}

void live_wire_t::invalidate()
{
    source_shutdown();

    unsubscribe();

    piw::tsd_fastcall(__wire_invalidator, this, 0);

    if(root_)
    {
        root_->children_.erase(path());
        root_ = 0;
    }
}

void live_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    output_ = piw::xevent_data_buffer_t(OUT_MASK,PIW_DATAQUEUE_SIZE_NORM);

    input_ = b.iterator();

    unsigned long long t = id.time();
    last_from_ = t;

	piw::data_nb_t d;
	if(input_->latest(IN_KEY,d,t))
	{
		process(IN_KEY,d,t);
	}

    source_start(seq,id,output_);

    root_->add_ticker(this);
}

void live_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    input_->set_signal(s,n);
    input_->reset(s,t);
    ticked(last_from_,t);
}

bool live_wire_t::event_end(unsigned long long t)
{
    ticked(last_from_,t);
    root_->del_ticker(this);
    return source_end(t);
}

void live_wire_t::ticked(unsigned long long f, unsigned long long t)
{
    last_from_ = t;

	piw::data_nb_t d;
	unsigned s;
	while(input_->next(SIG1(IN_KEY),s,d,t))
	{
		process(s,d,t);
	}
}

void live_wire_t::source_ended(unsigned seq)
{
    event_ended(seq);
}


void live_wire_t::process(unsigned s, const piw::data_nb_t &d, unsigned long long t)
{
//	pic::logmsg() << "live_wire_t::process:" << s;
    float column, row, course, key;
    piw::hardness_t hardness;
    if(IN_KEY==s && piw::decode_key(d,&column,&row,&course,&key,&hardness))
    {
    	if(hardness==piw::KEY_LIGHT)
    	{
    		return;

    	}
    	if(row > root_->view().trackSize() || column > root_->view().clipSize())
    	{
    		return;
    	}
    	int track = (int) row - 1 + root_->view().trackOffset();
    	int clip = (int) column - 1 + root_->view().clipOffset();

//    	pic::logmsg() << "live_wire_t::process " << s  << "track" << track << "clip:" << clip << "hard" << hardness;
    	if(root_->view().getState(track,clip)==live_model::clip_t::CLIP_EMPTY)
    	{
			int prevArmed = root_->view().armTrack(track);
			if (track!=prevArmed)
			{
				if(prevArmed!=-1)
				{
					root_->client_->send(new live_arm_message_t(prevArmed,false));
				}
				root_->client_->send(new live_arm_message_t(track,true));
			}
    	}

    	live_model::clip_t *lastRec=root_->view().getRecordingClip();
    	if(lastRec!=NULL)
    	{
    		root_->client_->send(new live_clip_info_message_t(lastRec->track_,lastRec->clip_));
    	}

    	root_->client_->send(new live_clip_slot_message_t(track,clip));
    }
}


light_wire_t::light_wire_t(	live_pad_t::impl_t *impl) : event_data_source_real_t(piw::pathone(1,0)), root_(impl)
{
	unsigned long long t = piw::tsd_time();
	output_ = piw::xevent_data_buffer_t(OUT_MASK,PIW_DATAQUEUE_SIZE_TINY);
	source_start(0,piw::pathone_nb(1,t),output_);
}

light_wire_t::~light_wire_t()
{
	// pic::logmsg() << "light_wire_t::~light_wire_t";
	source_shutdown();
}

void light_wire_t::source_ended(unsigned seq)
{
//	pic::logmsg() << "light_wire_t::source_ended";
	source_end(piw::tsd_time());
}

void light_wire_t::updateLights()
{
	updateLightBuffer(root_->view(),output_);
}



//////////// live_pad_t
live_pad_t::live_pad_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &output,const std::string &sendhost, const std::string &sendport, const std::string &recvport)
	: impl_(new impl_t(d,output,sendhost,sendport,recvport))
{
}

live_pad_t::~live_pad_t()
{
	delete impl_;
}

piw::cookie_t live_pad_t::cookie()
{
    return piw::cookie_t(impl_);
}


void live_pad_t::set_window(unsigned top, unsigned height, unsigned left, unsigned width)
{
//	pic::logmsg() << "live_pad_t::set_window "<< top << "," << height << "," << left << "," << width;
	if ( top > 0 && height > 0 && left > 0 && width >0)
	{
		impl_->set_window(top-1,height,left-1,width);
	}
}

void live_pad_t::refresh()
{
//	pic::logmsg() << "live_pad_t::set_window ";
	impl_->refresh();
}

void live_pad_t::play()
{
//	pic::logmsg() << "live_pad_t::play ";
	impl_->play();
}

void live_pad_t::stop()
{
//	pic::logmsg() << "live_pad_t::stop ";
	impl_->stop();
}

void live_pad_t::play_scene(int scene)
{
//	pic::logmsg() << "live_pad_t::play_scene " << scene;
	if (scene > 0)
	{
		impl_->play_scene(scene-1);
	}
	else
	{
		stop();
	}
}

void live_pad_t::shutdown()
{
	// pic::logmsg() << "live_pad_t::shutdown ";
	impl_->shutdown();
}

void live_pad_t::undo()
{
//	pic::logmsg() << "live_pad_t::undo ";
	impl_->undo();
}

void live_pad_t::redo()
{
//	pic::logmsg() << "live_pad_t::redo ";
	impl_->redo();
}



}; //namespace livepad_plg


