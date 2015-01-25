
#include "midi_device.h"

#include <piw/piw_data.h>
#include <piw/piw_status.h>
#include <piw/piw_bundle.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_keys.h>

#include <picross/pic_ref.h>
#include <picross/pic_ref.h>
#include <picross/pic_time.h>

#include <lib_midi/midi_decoder.h>

// COLOURS: the colours used at the moment, are just about acceptable for ableton push
// there is an issue though, as the CC colours will need to be tailoured a bit for push,
// once there is a 'dedicated' push implementation,
// then the 'defaults' will be set to 0,32,64,127

// turn logging by setting LOG_SINGLE(x) x
#define  LOG_SINGLE(x)
//#define  LOG_SINGLE(x) x

// signals
#define OUT_KEY 1
#define OUT_STRIP_1 2
#define OUT_STRIP_2 3
#define OUT_BREATH 4
#define OUT_PEDAL_1 5
#define OUT_PEDAL_2 6
#define OUT_PEDAL_3 7
#define OUT_PEDAL_4 8

// sub keys within the main key bundle
#define OUT_S_KEY 1
#define OUT_S_PRESSURE 2
#define OUT_S_ROLL 3
#define OUT_S_YAW 4
#define OUT_KEY_MASK SIG4(OUT_S_KEY,OUT_S_PRESSURE,OUT_S_ROLL,OUT_S_YAW)



#define OUT_S_STRIP_REL 1
#define OUT_S_STRIP_ABS 2
#define OUT_STRIP_MASK SIG2(OUT_S_STRIP_REL,OUT_S_STRIP_ABS)


#define IN_MIDI 1
#define IN_LIGHT 2
#define OUT_MIDI 10
#define OUT_MASK SIG1(OUT_MIDI)



// constants
#define MAX_KEYS 128

#define MAX_COLS 2
#define NOTE 0
#define CC 1


namespace midi_device_plg
{

//////////// midi_device_t::impl_t  declaration

	struct midi_wire_t:
		piw::wire_t,
		piw::wire_ctl_t,
		piw::event_data_sink_t,
		piw::event_data_source_real_t,
		virtual public pic::lckobject_t,
		pic::element_t<>
	{
		midi_wire_t(midi_device_t::impl_t *p, const piw::event_data_source_t &);
		~midi_wire_t() { invalidate(); }

		void wire_closed() { delete this; }
		void invalidate();
		void event_start(unsigned,const piw::data_nb_t &, const piw::xevent_data_buffer_t &);
		bool event_end(unsigned long long);
		void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);
		void process(unsigned, piw::data_nb_t &, unsigned long long);
		void processLights(unsigned, piw::data_nb_t &, unsigned long long);
		void updateLights(unsigned key,unsigned value);
		void ticked(unsigned long long f, unsigned long long t);
		void source_ended(unsigned);

		midi_device_t::impl_t *root_;
		piw::xevent_data_buffer_t::iter_t input_;
		piw::xevent_data_buffer_t output_;

		unsigned long long last_from_;
		piw::statusledconvertor_t led_converter_;
		unsigned long long light_t_;
	};

	// fwd declsvel
	struct kwire_t;
	struct cwire_t;
	struct breath_t;
	struct strip_t;
	struct pedal_t;

	struct midi_output_wire_t;


    //////////// midi_device_t::impl_t  declaration
    struct midi_device_t::impl_t :
    	    piw::root_t,
    	    piw::root_ctl_t,
    	    piw::clocksink_t,
    		midi::mididecoder_t,
    	    virtual pic::lckobject_t
    {
        impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t & outputs);
        ~impl_t();
        void clocksink_ticked(unsigned long long f, unsigned long long t);
        void add_ticker(midi_wire_t *w);
        void del_ticker(midi_wire_t *w);
        void invalidate();
        piw::wire_t *root_wire(const piw::event_data_source_t &es);
        void root_closed();
        void root_opened();
        void root_clock();
        void root_latency();

        void create_wires();


    	// callbacks for midi_decoder
        void decoder_noteoff(unsigned channel, unsigned number, unsigned velocity);
        void decoder_noteon(unsigned channel, unsigned number, unsigned velocity);
        void decoder_polypressure(unsigned channel, unsigned number, unsigned value);
        void decoder_channelpressure(unsigned channel, unsigned value);
        void decoder_pitchbend(unsigned channel, unsigned value);
        void decoder_cc(unsigned channel, unsigned number, unsigned value);

        // not interested in these at this time
        void decoder_programchange(unsigned channel, unsigned value) {}

    	// from midi wire
        void decode_midi(const piw::data_nb_t &d, unsigned long long t);

        void sendMidiNote(unsigned long long t,unsigned note,unsigned vel);
        void sendMidiCC(unsigned long long t,unsigned cc,unsigned value);


        // helper funcions
        unsigned long updatePeriod() { return update_period_;}
        pic::ref_t<cwire_t> get_cwire(unsigned dev);


        pic::lckmap_t<piw::data_t, midi_wire_t *>::lcktype children_;
        pic::ilist_t<midi_wire_t> tickers_;
        bct_clocksink_t *up_;

        // device wires
        pic::ref_t<kwire_t> kwires_[MAX_COLS][MAX_KEYS];

        //TODO these should change to an array!
        pic::ref_t<strip_t> strip1_;
        pic::ref_t<strip_t> strip2_;
        pic::ref_t<breath_t> breath_;
        pic::ref_t<pedal_t> pedal1_;
        pic::ref_t<pedal_t> pedal2_;
        pic::ref_t<pedal_t> pedal3_;
        pic::ref_t<pedal_t> pedal4_;

        pic::ref_t<midi_output_wire_t> midi_output_;


        static const int INVALID_CC= -1; ; // this may change!

        static const unsigned PRESSURE=0;
        static const unsigned ROLL=1;
        static const unsigned YAW=2;
        static const unsigned BREATH=3;
        static const unsigned STRIP_1=4;
        static const unsigned STRIP_2=5;
        static const unsigned PEDAL_1=6;
        static const unsigned PEDAL_2=7;
        static const unsigned PEDAL_3=8;
        static const unsigned PEDAL_4=9;
        static const unsigned MAX_DEVICE=PEDAL_4+1;

        // behavioural controls
        unsigned  channel_; //  poly voice =0 , else = filter

        bool enable_notes_;    // noteon/noteoff
        bool enable_velocity_;  // velocity = pressure
        bool enable_control_notes_; // CC num = note, value = pressure/velocity
        unsigned  velocity_sample_;

        unsigned long update_period_;

        unsigned long long last_event_t_;

        int note_cc; // CC X, values is used as note num
        int note_pressure_cc; // CC X, value is used as pressure, used with note_cc
        int last_cc_note;
        int last_cc_note_pressure;

    	bool high_res_cc_; // are CCs 14 bit, LSB followed by MSB

        bool poly_at_[MAX_DEVICE];
        bool chan_at_[MAX_DEVICE];
        bool pb_[MAX_DEVICE];
        int  cc_[MAX_DEVICE];

        int  hires_cc_[MAX_DEVICE];  // for non standard hi res midi
        int  hires_pb_[MAX_DEVICE]; // for 21 bit pitchbend support i.e. pb+1 cc

        unsigned colour_map_[CLR_MIXED+1];
    };


struct midi_output_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t, virtual public pic::counted_t
{
	midi_output_wire_t(const piw::data_t &,midi_device_t::impl_t *);
	~midi_output_wire_t() { source_shutdown(); }

	void source_ended(unsigned seq) {source_end(piw::tsd_time());}

	void sendNote( unsigned long long t, unsigned channel, unsigned note, unsigned velocity);
	void sendCC( unsigned long long t, unsigned channel, unsigned num, unsigned value);

	piw::data_nb_t id_;
	piw::xevent_data_buffer_t output_;
};


struct cwire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t, virtual public pic::counted_t
{
	cwire_t(const piw::data_t &,midi_device_t::impl_t *);
	virtual ~cwire_t() { source_shutdown(); }

	void source_ended(unsigned seq) {}

	virtual void updateLsb(unsigned long long t, unsigned lsb);
	virtual void update(unsigned long long t,unsigned msb)=0;
	void release(bool v) { release_=v;}
	void relative(bool v) { relative_=v;}
	void bipolar(bool v) { bipolar_=v;}

	midi_device_t::impl_t * device_;
	piw::data_nb_t id_;
	piw::xevent_data_buffer_t output_;
	bool active_;
	float cur_val;
	unsigned long long last_update_;
	int lsb_;

	bool release_;
	bool relative_;
	bool bipolar_;
};

struct breath_t: cwire_t
{
	breath_t(const piw::data_t &d,midi_device_t::impl_t *k): cwire_t(d,k) {}
	void update(unsigned long long t, unsigned msb);
};

struct pedal_t : cwire_t
{
	pedal_t( const piw::data_t &d,midi_device_t::impl_t *k ) : cwire_t(d,k) {}
	void update(unsigned long long t, unsigned msb);
};

struct strip_t: cwire_t
{
	strip_t(const piw::data_t &d,midi_device_t::impl_t *k): cwire_t(d,k), origin_(0) {}
	void update(unsigned long long t, unsigned msb);
    unsigned origin_;
};

struct kwire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t, virtual public pic::counted_t
{
	kwire_t(unsigned,unsigned,unsigned,const piw::data_t &,midi_device_t::impl_t  *);
	~kwire_t() { source_shutdown(); device_ = 0; }

	void source_ended(unsigned seq) {}

	void updateKey(unsigned long long t, bool a, unsigned msb);
	void updateP(unsigned long long t, unsigned msb);
	void updateR(unsigned long long t, unsigned msb);
	void updateY(unsigned long long t, unsigned msb);

	void updateLsbP(unsigned long long t, unsigned lsb);
	void updateLsbR(unsigned long long t, unsigned lsb);
	void updateLsbY(unsigned long long t, unsigned lsb);

	unsigned index_,column_,row_;
	unsigned channel_;
	piw::data_nb_t id_;
	midi_device_t::impl_t *device_;
	bool running_;
	piw::xevent_data_buffer_t output_;
	float cur_pressure_, cur_roll_, cur_yaw_;
	unsigned long long last_update_pressure_,last_update_roll_,last_update_yaw_;
	int lsb_pressure_,lsb_roll_,lsb_yaw_;
};




//////////////// implemenations
static const unsigned MAX_7_BIT = (1 << 7);
static const unsigned MAX_14_BIT = (1 << 14);
static const unsigned MSB =  (1 << 7) ;


static const float CLIP_WINDOW=1.0;

static float __clip(float x,float l)
{
    if(l<0.0001)
    {
        return 0;
    }

    x = std::max(x,-l);
    x = std::min(x,l);
    x = x*1.0/l;
    x = std::max(x,-1.0f);
    x = std::min(x,1.0f);

    return x;
}


inline float calcUnipolar(int msb, int lsb,bool hires=false)
{
	if(!hires) return float(msb)/ float(MAX_7_BIT);
	return __clip(float(msb * MSB + (lsb>=0?lsb:msb) ) / float(MAX_14_BIT) ,CLIP_WINDOW);
}

inline float calcBipolar(int msb, int lsb, bool hires=false)
{
	float u = calcUnipolar(msb,lsb,hires);
	float x= __clip((u * 2.0) - 1.0,CLIP_WINDOW);
	//LOG_SINGLE(pic::logmsg() << "calc bipolar x " << x  << " u " << u ;)
	return x;
}



/// midi_device_t::impl_t implementation
midi_device_t::impl_t::impl_t(piw::clockdomain_ctl_t *cd,  const piw::cookie_t &outputs) :
	root_t(0), up_(0),
	channel_(0),
	enable_notes_(true),enable_velocity_(true), enable_control_notes_(false),
	velocity_sample_(4),
	update_period_(0),
	note_cc(-1),note_pressure_cc(-1),last_cc_note(-1),last_cc_note_pressure(0),
	high_res_cc_(false)
{
	for (unsigned d=0;d<MAX_DEVICE;d++)
	{
		poly_at_[d]=false;
		chan_at_[d]=false;
		pb_[d]=false;
		cc_[d]=-1;
		hires_cc_[d]=-1;
		hires_pb_[d]=-1;
	}
	// common defaults, must match init in py
	cc_[BREATH]=2;
	cc_[STRIP_1]=1;
	pb_[ROLL]=true;
	colour_map_[CLR_RED]=4;
	colour_map_[CLR_ORANGE]=87;
	colour_map_[CLR_OFF]=0;
	colour_map_[CLR_GREEN]=124;
	colour_map_[CLR_MIXED]=colour_map_[CLR_ORANGE];

	connect(outputs);
	create_wires();
	cd->sink(this,"mididevice");
	tick_enable(true);

	set_clock(this);
}

pic::ref_t<cwire_t> midi_device_t::impl_t::get_cwire(unsigned dev)
{
	//TODO : this is temporary, wil move to a control wire array in the future
	switch(dev)
	{
		case BREATH: return breath_;
		case STRIP_1: return strip1_;
		case STRIP_2: return strip2_;
		case PEDAL_1: return pedal1_;
		case PEDAL_2: return pedal2_;
		case PEDAL_3: return pedal3_;
		case PEDAL_4: return pedal4_;
	}
	return pedal4_;
}


void midi_device_t::impl_t::create_wires()
{
	for(unsigned c=0; c < MAX_COLS ;c++)
	{
		for(unsigned k=0; k < MAX_KEYS ;k++)
		{
			kwires_[c][k] = pic::ref(new kwire_t(k+1,c+1,k+1,piw::pathtwo(OUT_KEY,(c*MAX_KEYS)+k+1,0),this));
		}
	}

    //TODO, this path... does it match the OUT_X?. i suspect it might given the strip uses 2 positions
    strip1_ = pic::ref(new strip_t(piw::pathone(OUT_STRIP_1,0),this));
    strip2_ = pic::ref(new strip_t(piw::pathone(OUT_STRIP_2,0),this));
    breath_ = pic::ref(new breath_t(piw::pathone(OUT_BREATH,0),this));
    pedal1_ = pic::ref(new pedal_t(piw::pathone(OUT_PEDAL_1,0),this));
    pedal2_ = pic::ref(new pedal_t(piw::pathone(OUT_PEDAL_2,0),this));
    pedal3_ = pic::ref(new pedal_t(piw::pathone(OUT_PEDAL_3,0),this));
    pedal4_ = pic::ref(new pedal_t(piw::pathone(OUT_PEDAL_4,0),this));

    midi_output_ = pic::ref(new midi_output_wire_t(piw::pathone(OUT_MIDI,0),this));
}

midi_device_t::impl_t::~impl_t()
{
	LOG_SINGLE(pic::logmsg() << "midi_device_t::impl_t::~impl_t";)
	tracked_invalidate();
	invalidate();
}


void midi_device_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
	midi_wire_t *w;

	for(w=tickers_.head(); w!=0; w=tickers_.next(w))
	{
		w->ticked(f,t);
	}
}

void midi_device_t::impl_t::add_ticker(midi_wire_t *w)
{
	if(!tickers_.head())
	{
		tick_suppress(false);
	}

	tickers_.append(w);
}

void midi_device_t::impl_t::del_ticker(midi_wire_t *w)
{
	tickers_.remove(w);

	if(!tickers_.head())
	{
		tick_suppress(true);
	}

}

void midi_device_t::impl_t::invalidate()
{
	tick_disable();

	pic::lckmap_t<piw::data_t,midi_wire_t *>::lcktype::iterator ci;
	while((ci=children_.begin())!=children_.end())
	{
		delete ci->second;
	}
}

piw::wire_t *midi_device_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
   pic::lckmap_t<piw::data_t,midi_wire_t *>::lcktype::iterator ci;

	if((ci=children_.find(es.path()))!=children_.end())
	{
		delete ci->second;
	}

	return new midi_wire_t(this, es);
}

void midi_device_t::impl_t::root_closed() { invalidate(); }

void midi_device_t::impl_t::root_opened() { root_clock(); root_latency(); }

void midi_device_t::impl_t::root_clock()
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

void midi_device_t::impl_t::root_latency()
{
	set_latency(get_latency());
}



// callbacks from decoder
void midi_device_t::impl_t::decoder_noteon(unsigned c, unsigned number, unsigned velocity)
{
	unsigned channel = c + 1;
	LOG_SINGLE(pic::logmsg() << "decoder_noteon " << channel << " note " << number << " v " << velocity ;)

	if (!enable_notes_) return;

	if (channel_!=channel && channel_!=0) return ;

	unsigned long long t=last_event_t_;

	unsigned key=number;

	unsigned v = enable_velocity_? velocity : 0;

	kwires_[NOTE][key]->channel_=channel;

	// MSH : currently velocity is 7 bit only
	kwires_[NOTE][key]->updateKey(t,true,v);

	LOG_SINGLE(pic::logmsg() << "decoder_noteon key: " << key << " id : " << kwires_[NOTE][key]->id_ << " ch " << kwires_[NOTE][key]->channel_;)
}

void midi_device_t::impl_t::decoder_noteoff(unsigned c, unsigned number, unsigned velocity)
{
	unsigned channel = c + 1;
	LOG_SINGLE(pic::logmsg() << "decoder_noteoff " << channel << " note " << number << " v " << velocity ;)
	if (!enable_notes_) return;
	if (channel_!=channel && channel_!=0) return ;

	unsigned long long t=last_event_t_;
	unsigned key=number;
	kwires_[NOTE][key]->channel_=0;
	kwires_[NOTE][key]->updateKey(t,false,0);
}

void midi_device_t::impl_t::decoder_polypressure(unsigned c, unsigned number, unsigned value)
{
	unsigned channel = c + 1;
	LOG_SINGLE(pic::logmsg() << "decoder_polypressure " << channel << " note " << number << " value " << value;)

	if(channel_!=channel && channel_!=0) return ;

	unsigned long long t=last_event_t_;

	unsigned key=number;

	// MSH - poly pressure is 7 bit only

	if(poly_at_[PRESSURE])	kwires_[NOTE][key]->updateP(t,value);
	if(poly_at_[YAW])		kwires_[NOTE][key]->updateY(t,value);
	if(poly_at_[ROLL])		kwires_[NOTE][key]->updateR(t,value);

	// no sense in poly pressure for global controls
}

void midi_device_t::impl_t::decoder_channelpressure(unsigned c, unsigned value)
{
	unsigned channel = c + 1;
	LOG_SINGLE(pic::logmsg() << "decoder_channelpressure " << channel << " value " << value;)


	if(channel_!=channel && channel_!=0) return ;

	unsigned long long t=last_event_t_;

	// MSH - channel pressure is 7 bit only
	for (unsigned key=0;key<MAX_KEYS;key++)
	{
		if(kwires_[NOTE][key]->channel_==channel)
		{
			if(chan_at_[PRESSURE])	kwires_[NOTE][key]->updateP(t,value);
			if(chan_at_[YAW])		kwires_[NOTE][key]->updateY(t,value);
			if(chan_at_[ROLL])		kwires_[NOTE][key]->updateR(t,value);
		}
	}

	if(chan_at_[BREATH])  breath_->update(t,value);
	if(chan_at_[STRIP_1]) strip1_->update(t,value);
	if(chan_at_[STRIP_2]) strip2_->update(t,value);
	if(chan_at_[PEDAL_1]) pedal1_->update(t,value);
	if(chan_at_[PEDAL_2]) pedal2_->update(t,value);
	if(chan_at_[PEDAL_3]) pedal3_->update(t,value);
	if(chan_at_[PEDAL_4]) pedal4_->update(t,value);

}

void midi_device_t::impl_t::decoder_pitchbend(unsigned c, unsigned value)
{
	unsigned channel = c + 1;
	LOG_SINGLE(pic::logmsg() << "decoder_pitchbend " << channel << " value " << value;)

	if(channel_!=channel && channel_!=0) return ;

	unsigned long long t=last_event_t_;

	unsigned msb=value / MAX_7_BIT;
	unsigned lsb=value % MAX_7_BIT;

	for (unsigned key=0;key<MAX_KEYS;key++)
	{
		if(kwires_[NOTE][key]->channel_==channel)
		{
			if(pb_[PRESSURE]) kwires_[NOTE][key]->updateLsbP(t,lsb);
			if(pb_[YAW])	  kwires_[NOTE][key]->updateLsbY(t,lsb);
			if(pb_[ROLL])	  kwires_[NOTE][key]->updateLsbR(t,lsb);

			if(pb_[PRESSURE]) kwires_[NOTE][key]->updateP(t,msb);
			if(pb_[YAW])	  kwires_[NOTE][key]->updateY(t,msb);
			if(pb_[ROLL])	  kwires_[NOTE][key]->updateR(t,msb);
		}
	}

	if(pb_[BREATH])  breath_->updateLsb(t,lsb);
	if(pb_[STRIP_1]) strip1_->updateLsb(t,lsb);
	if(pb_[STRIP_2]) strip2_->updateLsb(t,lsb);
	if(pb_[PEDAL_1]) pedal1_->updateLsb(t,lsb);
	if(pb_[PEDAL_2]) pedal2_->updateLsb(t,lsb);
	if(pb_[PEDAL_3]) pedal3_->updateLsb(t,lsb);
	if(pb_[PEDAL_4]) pedal4_->updateLsb(t,lsb);

	if(pb_[BREATH])  breath_->update(t,msb);
	if(pb_[STRIP_1]) strip1_->update(t,msb);
	if(pb_[STRIP_2]) strip2_->update(t,msb);
	if(pb_[PEDAL_1]) pedal1_->update(t,msb);
	if(pb_[PEDAL_2]) pedal2_->update(t,msb);
	if(pb_[PEDAL_3]) pedal3_->update(t,msb);
	if(pb_[PEDAL_4]) pedal4_->update(t,msb);
}



void midi_device_t::impl_t::decoder_cc(unsigned c, unsigned number, unsigned value)
{
	unsigned channel = c + 1;
	LOG_SINGLE(pic::logmsg() << "decoder_cc " << channel << " cc " << number << " value " << value;)

	if(channel_!=channel && channel_!=0) return ;

	unsigned long long t=last_event_t_;

	// handle hires cc for keys, and aux controls
	if(high_res_cc_)
	{
		for (unsigned key=0;key<MAX_KEYS;key++)
		{
			if(kwires_[NOTE][key]->channel_==channel)
			{
				if(hires_cc_[PRESSURE]==number) kwires_[NOTE][key]->updateLsbP(t,value);
				if(hires_cc_[YAW]==number)	  kwires_[NOTE][key]->updateLsbY(t,value);
				if(hires_cc_[ROLL]==number)	  kwires_[NOTE][key]->updateLsbR(t,value);
			}
		}

		if(hires_cc_[BREATH]==number)  breath_->updateLsb(t,value);
		if(hires_cc_[STRIP_1]==number) strip1_->updateLsb(t,value);
		if(hires_cc_[STRIP_2]==number) strip2_->updateLsb(t,value);
		if(hires_cc_[PEDAL_1]==number) pedal1_->updateLsb(t,value);
		if(hires_cc_[PEDAL_2]==number) pedal2_->updateLsb(t,value);
		if(hires_cc_[PEDAL_3]==number) pedal3_->updateLsb(t,value);
		if(hires_cc_[PEDAL_4]==number) pedal4_->updateLsb(t,value);
	}

	// handle normal cc for keys, and aux controls
	for (unsigned key=0;key<MAX_KEYS;key++)
	{
		if(kwires_[NOTE][key]->channel_==channel)
		{
			if(cc_[PRESSURE]==number) kwires_[NOTE][key]->updateP(t,value);
			if(cc_[YAW]==number)	  kwires_[NOTE][key]->updateY(t,value);
			if(cc_[ROLL]==number)	  kwires_[NOTE][key]->updateR(t,value);
		}
	}

	if(cc_[BREATH]==number)  breath_->update(t,value);
	if(cc_[STRIP_1]==number) strip1_->update(t,value);
	if(cc_[STRIP_2]==number) strip2_->update(t,value);
	if(cc_[PEDAL_1]==number) pedal1_->update(t,value);
	if(cc_[PEDAL_2]==number) pedal2_->update(t,value);
	if(cc_[PEDAL_3]==number) pedal3_->update(t,value);
	if(cc_[PEDAL_4]==number) pedal4_->update(t,value);

	// handle CC as notes, where CC num = note, CC val = pressure/velocity
	if(enable_control_notes_)
	{
		if(value>0)
		{
			unsigned key=number;
			unsigned v = enable_velocity_? value : 0;
			kwires_[CC][key]->channel_=channel;
			kwires_[CC][key]->updateKey(t,true,v);
			LOG_SINGLE(pic::logmsg() << "decoder_cc notes on key: " << key << " id : " << kwires_[CC][key]->id_ << " ch " << kwires_[CC][key]->channel_;)
		}
		else
		{
			unsigned key=number;
			kwires_[CC][key]->channel_=0;
			kwires_[CC][key]->updateKey(t,false,0);
			LOG_SINGLE(pic::logmsg() << "decoder_cc notes off key: " << key << " id : " << kwires_[CC][key]->id_;)
		}
	}

	// CC as note!
	if (note_cc==number)
	{
		LOG_SINGLE(pic::logmsg() << "detected note change " << number << " v " << value << "last_cc_note_pressure " << last_cc_note_pressure;)
		if (last_cc_note !=value)
		{
			 LOG_SINGLE(pic::logmsg() << "detected note change2 " << number << " v " << value;)
			// note off for last note
			if(last_cc_note > 0)
			{
				 LOG_SINGLE(pic::logmsg() << "note to zero : off " << number << " v " << value;)
				kwires_[NOTE][last_cc_note]->channel_=0;
				kwires_[NOTE][last_cc_note]->updateKey(t,false,0);
				last_cc_note=0;
			}

			if(value>0)
			{
				// new note
				LOG_SINGLE(pic::logmsg() << "change note " << number << " v " << value;)
				if (last_cc_note_pressure<=0)
				{
					LOG_SINGLE(pic::logmsg() << "pres zero note off " << number << " v " << value;)
					kwires_[NOTE][value]->channel_=0;
					kwires_[NOTE][value]->updateKey(t,false,0);
				}
				else
				{
					LOG_SINGLE(pic::logmsg() << "note on " << number << " v " << value << "last_cc_note_pressure " << last_cc_note_pressure;)
					kwires_[NOTE][value]->channel_=channel;
					kwires_[NOTE][value]->updateKey(t,true,last_cc_note_pressure);
				}
			}

			last_cc_note=value;
		}
	}
	if (note_pressure_cc==number)
	{
		last_cc_note_pressure=value;
		LOG_SINGLE(pic::logmsg() << "pressure change " << number << " value " << value << "last_cc_note_pressure " << last_cc_note_pressure;)
		if(last_cc_note > 0)
		{
			LOG_SINGLE(pic::logmsg() << "update last note " << number << " value " << value << "last_cc_note_pressure " << last_cc_note_pressure;)
			if (last_cc_note_pressure==0)
			{
				LOG_SINGLE(pic::logmsg() << "turn off" << number << " value " << value << "last_cc_note_pressure " << last_cc_note_pressure;)
				kwires_[NOTE][last_cc_note]->channel_=0;
				kwires_[NOTE][last_cc_note]->updateKey(t,false,0);
			}
			else
			{
				kwires_[NOTE][last_cc_note]->updateP(t,last_cc_note_pressure);
			}
		}
	}
}

void midi_device_t::impl_t::decode_midi(const piw::data_nb_t &d, unsigned long long t)
{
	LOG_SINGLE(pic::logmsg() << "decode_midi " << "d " << d << " t " << t;)
	last_event_t_=t;
	unsigned length = d.as_bloblen();
	const unsigned char *buffer = (const unsigned char *)(d.as_blob());
	decoder_input(buffer, length);
	last_event_t_=0;
}


void midi_device_t::impl_t::sendMidiNote(unsigned long long t,unsigned note, unsigned vel)
{
	// MSH : fixed to channel 1 for output?
	midi_output_->sendNote(t,1,note,vel);
}

void midi_device_t::impl_t::sendMidiCC(unsigned long long t,unsigned num, unsigned val)
{
	// MSH : fixed to channel 1 for output?
	midi_output_->sendCC(t,1,num,val);
}


static const unsigned midi_device_col_length[MAX_COLS]= { MAX_KEYS, MAX_KEYS};

// midi wire
midi_wire_t::midi_wire_t(midi_device_t::impl_t  *p, const piw::event_data_source_t &es):
    piw::event_data_source_real_t(es.path()),
    root_(p),
    last_from_(0),
    led_converter_(MAX_COLS,midi_device_col_length),
	light_t_(0)
{
    root_->children_.insert(std::make_pair(path(),this));

    root_->connect_wire(this, source());

    subscribe_and_ping(es);
}

static int __midi_wire_invalidator(void *w_, void *_)
{
	midi_wire_t *w = (midi_wire_t *)w_;
    if(w->root_)
    {
        w->root_->del_ticker(w);
    }
    return 0;
}

void midi_wire_t::invalidate()
{
    source_shutdown();

    unsubscribe();

    piw::tsd_fastcall(__midi_wire_invalidator, this, 0);

    if(root_)
    {
        root_->children_.erase(path());
        root_ = 0;
    }
}

void midi_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    output_ = piw::xevent_data_buffer_t(OUT_MASK,PIW_DATAQUEUE_SIZE_TINY);

    input_ = b.iterator();

    unsigned long long t = id.time();
    last_from_ = t;

	piw::data_nb_t d;
	if(input_->latest(IN_MIDI,d,t))
	{
		process(IN_MIDI,d,t);
	}

	if(input_->latest(IN_LIGHT,d,t))
	{
		process(IN_LIGHT,d,t);
	}

    source_start(seq,id,output_);

    root_->add_ticker(this);
}

void midi_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    input_->set_signal(s,n);
    input_->reset(s,t);
    ticked(last_from_,t);
}

bool midi_wire_t::event_end(unsigned long long t)
{
    ticked(last_from_,t);
    root_->del_ticker(this);
    return source_end(t);
}

void midi_wire_t::ticked(unsigned long long f, unsigned long long t)
{
    last_from_ = t;

	piw::data_nb_t d;
	unsigned s;
	while(input_->next(SIG2(IN_MIDI,IN_LIGHT),s,d,t))
	{
		process(s,d,d.time());
	}
}

void midi_wire_t::source_ended(unsigned seq)
{
    event_ended(seq);
}

void midi_wire_t::process(unsigned s, piw::data_nb_t &d, unsigned long long t)
{
	LOG_SINGLE(pic::logmsg() << "midi_wire_t::process t=" << t << " s= " << s;)
    if(IN_MIDI==s)
    {
		root_->decode_midi(d,t);
    }
    else if (IN_LIGHT==s)
    {
    	LOG_SINGLE(pic::logmsg() << "midi_wire_t::process (light) t=" << t << " d= " << d;)
    	processLights(s,d,t);
    }
}
static void __updateLights(void* i, unsigned key, unsigned value)
{
	((midi_wire_t*) i)->updateLights(key,value);
}

void midi_wire_t::processLights(unsigned s, piw::data_nb_t &d, unsigned long long t)
{
	light_t_=t;
	led_converter_.update_leds(d,this,__updateLights);
}

void midi_wire_t::updateLights(unsigned key,unsigned value)
{
	unsigned colour = root_->colour_map_[value==CLR_MIXED?CLR_ORANGE:value];

	unsigned long long t=light_t_++;

	LOG_SINGLE(pic::logmsg() << "midi_wire_t::updateLights t=" << t << " k= " << key << " c= " << colour;)

	if(key<=127)
	{
		root_->sendMidiNote(t,key,colour);
	}
	else
	{
		root_->sendMidiCC(t,key-127,colour);
	}
}

// midi_output_wire_t
midi_output_wire_t::midi_output_wire_t(const piw::data_t &path, midi_device_t::impl_t  *k):
		piw::event_data_source_real_t(path), id_(piw::pathone_nb(1,0))
{
    k->connect_wire(this,source());
    output_ = piw::xevent_data_buffer_t(OUT_MASK,PIW_DATAQUEUE_SIZE_NORM);
    source_start(0,id_.restamp(piw::tsd_time()),output_);
}

void midi_output_wire_t::sendNote(unsigned long long t, unsigned channel,unsigned note, unsigned velocity)
{
	unsigned d0= 0x90+(channel-1);
	unsigned d1=note;
	unsigned d2=velocity;

	unsigned char *blob = 0;
	piw::data_nb_t midi_data = piw::makeblob_nb(piw::tsd_time(),3,&blob);

	blob[0] = (unsigned char)d0;
	blob[1] = (unsigned char)d1;
	blob[2] = (unsigned char)d2;
	LOG_SINGLE(pic::logmsg() << "midi_output_wire_t::sendNote d0=" << std::hex << (unsigned)d0 << " d1=" << (unsigned)d1 << " d2=" << (unsigned)d2 << " len=3 time=" << std::dec << t;)
	output_.add_value(OUT_MIDI,midi_data);
}

void midi_output_wire_t::sendCC(unsigned long long t, unsigned channel,unsigned num, unsigned v)
{
	unsigned d0= 0xB0+(channel-1);
	unsigned d1=num;
	unsigned d2=v;

	unsigned char *blob = 0;
	piw::data_nb_t midi_data = piw::makeblob_nb(piw::tsd_time(),3,&blob);

	blob[0] = (unsigned char)d0;
	blob[1] = (unsigned char)d1;
	blob[2] = (unsigned char)d2;
	LOG_SINGLE(pic::logmsg() << "midi_output_wire_t::sendCC d0=" << std::hex << (unsigned)d0 << " d1=" << (unsigned)d1 << " d2=" << (unsigned)d2 << " len=3 time=" << std::dec << t;)
	output_.add_value(OUT_MIDI,midi_data);
}


//////////// midi_device_t
midi_device_t::midi_device_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &outputs)
	: impl_(new impl_t(d,outputs))
{
}

midi_device_t::~midi_device_t()
{
	delete impl_;
}

piw::cookie_t midi_device_t::cookie()
{
    return piw::cookie_t(impl_);
}


void midi_device_t::enable_notes(bool v)
{
	if(impl_) impl_->enable_notes_=v;
}

// where we use CC as note, and CC value is pressure/velocity
void midi_device_t::enable_control_notes(bool v)
{
	if(impl_) impl_->enable_control_notes_=v;
}


void midi_device_t::enable_velocity(bool v)
{
	if(impl_) impl_->enable_velocity_=v;
}

void midi_device_t::enable_poly_at(unsigned dev,bool v)
{
	if(impl_ && dev < midi_device_t::impl_t::MAX_DEVICE) impl_->poly_at_[dev]=v;
}

void midi_device_t::set_colour(unsigned i,unsigned c)
{
	if(impl_ && i<CLR_MIXED) impl_->colour_map_[i]=c;
}


void midi_device_t::enable_chan_at(unsigned dev,bool v)
{
	if(impl_ && dev < midi_device_t::impl_t::MAX_DEVICE) impl_->chan_at_[dev]=v;
}

void midi_device_t::set_pb_map(unsigned dev,int v)
{
	LOG_SINGLE(pic::logmsg() << " midi_device_t::set_pb_map" << impl_->pb_[dev] << " v " << v;)
	if(impl_ && dev < midi_device_t::impl_t::MAX_DEVICE) impl_->pb_[dev]=v;
}

void midi_device_t::set_cc_map(unsigned dev,int v)
{
	LOG_SINGLE(pic::logmsg() << " midi_device_t::set_cc_map" << impl_->cc_[dev] << " v " << v;)

	if(impl_ && dev < midi_device_t::impl_t::MAX_DEVICE) impl_->cc_[dev]=v;
}

void midi_device_t::set_hires_pb_map(unsigned dev,int v)
{
	LOG_SINGLE(pic::logmsg() << " midi_device_t::set_hires_pb_map" << impl_->hires_pb_[dev] << " v " << v;)

	if(impl_ && dev < midi_device_t::impl_t::MAX_DEVICE) impl_->hires_pb_[dev]=v;
}

void midi_device_t::set_hires_cc_map(unsigned dev,int v)
{
	LOG_SINGLE(pic::logmsg() << " midi_device_t::set_hires_cc_map" << impl_->hires_cc_[dev] << " v " << v;)

	if(impl_ && dev < midi_device_t::impl_t::MAX_DEVICE) impl_->hires_cc_[dev]=v;
}


void midi_device_t::set_bipolar_map(unsigned dev,bool v)
{
	LOG_SINGLE(pic::logmsg() << " midi_device_t::set_release_map v " << v;)

	if(impl_ && dev>=midi_device_t::impl_t::BREATH && dev < midi_device_t::impl_t::MAX_DEVICE)
	{
		pic::ref_t<cwire_t> wire=impl_->get_cwire(dev);
		wire->bipolar(v);
	}
}

void midi_device_t::set_release_map(unsigned dev,bool v)
{
	LOG_SINGLE(pic::logmsg() << " midi_device_t::set_release_map v " << v;)

	if(impl_ && dev>=midi_device_t::impl_t::BREATH && dev < midi_device_t::impl_t::MAX_DEVICE)
	{
		pic::ref_t<cwire_t> wire=impl_->get_cwire(dev);
		wire->release(v);
	}
}

void midi_device_t::set_relative_map(unsigned dev,bool v)
{

	LOG_SINGLE(pic::logmsg() << " midi_device_t::set_relative_map v " << v;)

	if(impl_ && dev>=midi_device_t::impl_t::BREATH && dev < midi_device_t::impl_t::MAX_DEVICE)
	{
		pic::ref_t<cwire_t> wire=impl_->get_cwire(dev);
		wire->relative(v);
	}
}



void midi_device_t::set_note_cc(int v)
{
	if(impl_) impl_->note_cc=v;
}

void midi_device_t::set_note_cc_pressure(int v)
{
	if(impl_) impl_->note_pressure_cc=v;
}


void midi_device_t::channel(unsigned v)
{
	if(impl_) impl_->channel_=v;
}

void midi_device_t::velocity_sample(unsigned v)
{
	if(impl_) impl_->velocity_sample_=v;
}

void midi_device_t::enable_hires_cc(bool v)
{
	if(impl_) impl_->high_res_cc_=v;
}


void midi_device_t::set_data_freq(int v)
{
	if(impl_)
	{
		if(v==0)
		{
			impl_->update_period_=0;
		}
		else
		{
			impl_->update_period_ = (1000*1000 / v);
		}
	}
}

piw::data_t  midi_device_t::get_columnlen()
{
	piw::data_t cols = piw::tuplenull(0);
	for(int i=0;i<MAX_COLS;i++)
	{
		cols = piw::tupleadd(cols, piw::makelong(MAX_KEYS,0));
	}
    return cols;
}

piw::data_t  midi_device_t::get_columnoffset()
{
	piw::data_t cols = piw::tuplenull(0);
	for(int i=0;i<MAX_COLS;i++)
	{
		cols = piw::tupleadd(cols, piw::makelong(0,0));
	}
    return cols;
}

piw::data_t  midi_device_t::get_courselen()
{
	piw::data_t courses = piw::tuplenull(0);
	courses = piw::tupleadd(courses, piw::makelong(MAX_COLS*MAX_KEYS,0));
    return courses;
}

piw::data_t  midi_device_t::get_courseoffset()
{
    piw::data_t courses = piw::tuplenull(0);
    courses = piw::tupleadd(courses, piw::makefloat(0.0,0));
    return courses;
}


///////////////////////////////////////////////////////////
// control wires implementations
///////////////////////////////////////////////////////////




kwire_t::kwire_t(unsigned i, unsigned c, unsigned r, const piw::data_t &path, midi_device_t::impl_t  *k)
	: piw::event_data_source_real_t(path),
	index_(i), column_(c), row_(r),
	channel_(0),
	id_(piw::pathone_nb(i,0)),
	device_(k), running_(false),
	output_(OUT_KEY_MASK,PIW_DATAQUEUE_SIZE_NORM),
	cur_pressure_(0), cur_roll_(0), cur_yaw_(0),
	last_update_pressure_(0), last_update_roll_(0), last_update_yaw_(0),
	lsb_pressure_(-1),lsb_roll_(-1),lsb_yaw_(-1)
{
    k->connect_wire(this,source());
}


void kwire_t::updateKey(unsigned long long t, bool a, unsigned msb)
{
    float v=calcUnipolar(msb,-1,device_->high_res_cc_);
    LOG_SINGLE(pic::logmsg() << "kwire_t::updateKey start t " << t << " a " << a << " v " << v;)

    if(!a)
    {
    	if(running_)
    	{
			LOG_SINGLE(pic::logmsg() << "kwire_t::updateKey source end c " << column_ << " r " << row_ << " v " << v << " t " << t;)
			source_end(t);
    	}
		running_=false;
    }
    else
    {
    	// active
    	if(!running_)
    	{
    		LOG_SINGLE(pic::logmsg() << "kwire_t::updateKey source start c " << column_ << " r " << row_ << " t " << t << " id " << id_;)
			piw::hardness_t h=piw::KEY_LIGHT;
			if(v>=.25) h=piw::KEY_SOFT;
			if(v>=.5) h=piw::KEY_HARD;

			output_.add_value(OUT_S_KEY,piw::makekey(column_,row_,1,index_,h,t));
			output_.add_value(OUT_S_PRESSURE,piw::makefloat_bounded_nb(1,0,0,0,t));
			output_.add_value(OUT_S_ROLL,piw::makefloat_bounded_nb(1,-1,0,0.0,t));
			output_.add_value(OUT_S_YAW,piw::makefloat_bounded_nb(1,-1,0,0.0,t));
            source_start(0,id_.restamp(t),output_);

            // some agents are expecting N pressure samples, so we generate them here
            for(unsigned i=0;i<device_->velocity_sample_;i++)
            {
            	output_.add_value(OUT_S_PRESSURE,piw::makefloat_bounded_nb(1,0,0,v,t+i+1));
            }

            running_=true;
            lsb_pressure_=-1;
            lsb_roll_=-1;
            lsb_yaw_=-1;
            last_update_pressure_=0;
            last_update_roll_=0;
            last_update_yaw_=0;
    	}
//    	else
//    	{
//			output_.add_value(OUT_S_PRESSURE,piw::makefloat_bounded_nb(1,0,0,p,t));
//    	}
    }
}

void kwire_t::updateP(unsigned long long t, unsigned msb)
{
    float v=calcUnipolar(msb,lsb_pressure_,device_->high_res_cc_);
    LOG_SINGLE(pic::logmsg() << "updateP " << t << "col " << column_ << " row " << row_ << " ch " << channel_ << " value " << v;)

    if(!running_ || t < (last_update_pressure_ + device_->updatePeriod()))
    {
        return;
    }

    if(v!=cur_pressure_) output_.add_value(OUT_S_PRESSURE,piw::makefloat_bounded_nb(1,0,0,v,t));
    cur_pressure_=v;
    last_update_pressure_=t;
    lsb_pressure_=-1;
}

void kwire_t::updateR(unsigned long long t, unsigned msb)
{
    float v=calcBipolar(msb,lsb_roll_,device_->high_res_cc_);
    LOG_SINGLE(pic::logmsg() << "updateR " << t << "col " << column_ << " row " << row_ << " ch " << channel_ << " value " << v;)
    if(!running_ || t < (last_update_roll_ + device_->updatePeriod()))
    {
    	LOG_SINGLE(pic::logmsg() << "updateR:no update t" << t << " lu " << last_update_roll_ << " up " << device_->updatePeriod();)
    	return;
    }
    if(cur_roll_!=v) output_.add_value(OUT_S_ROLL,piw::makefloat_bounded_nb(1,-1,0,v,t));
    cur_roll_=v;
    last_update_roll_=t;
    lsb_roll_=-1;
}

void kwire_t::updateY(unsigned long long t, unsigned msb)
{
    float v=calcBipolar(msb,lsb_yaw_,device_->high_res_cc_);
	LOG_SINGLE(pic::logmsg() << "updateY " << t << "col " << column_ << " row " << row_ << " ch " << channel_ << " value " << v;)
    if(!running_ || t < (last_update_yaw_ + device_->updatePeriod()))
    {
        return;
    }

    if(cur_yaw_!=v) output_.add_value(OUT_S_YAW,piw::makefloat_bounded_nb(1,-1,0,v,t));
    cur_yaw_=v;
    last_update_yaw_=t;
    lsb_yaw_=-1;
}


void kwire_t::updateLsbP(unsigned long long t,unsigned lsb)
{
	LOG_SINGLE(pic::logmsg() << "updateLsb_P " << t <<  " lsb " << lsb;)

	if(!running_)
		return;

	if((last_update_pressure_ + device_->updatePeriod()) > t )
		return;

	lsb_pressure_=lsb;
}

void kwire_t::updateLsbR(unsigned long long t,unsigned lsb)
{
	LOG_SINGLE(pic::logmsg() << "updateLsb_R " << t <<  " lsb " << lsb;)

	if(!running_)
		return;

	if((last_update_roll_ + device_->updatePeriod()) > t )
		return;

	lsb_roll_=lsb;
}

void kwire_t::updateLsbY(unsigned long long t,unsigned lsb)
{
	LOG_SINGLE(pic::logmsg() << "updateLsb_Y " << t  << " lsb " << lsb;)

	if(!running_)
		return;

	if((last_update_yaw_ + device_->updatePeriod()) > t )
		return;

	lsb_yaw_=lsb;
}


cwire_t::cwire_t(const piw::data_t &path, midi_device_t::impl_t  *k):
		piw::event_data_source_real_t(path), device_(k), id_(piw::pathnull_nb(0)),
		active_(false),cur_val(0.0),lsb_(-1), release_(true), relative_(false), bipolar_(true)
{
    k->connect_wire(this,source());
}


void cwire_t::updateLsb(unsigned long long t,unsigned lsb)
{
	LOG_SINGLE(pic::logmsg() << "updateLsb " << t << " lsb " << lsb;)

	if(!active_)
		return;

	if(t < (last_update_ + device_->updatePeriod()) )
		return;

	lsb_=lsb;
}



void strip_t::update( unsigned long long t, unsigned msb)
{
    bool a = true;
    float v = 0.0;
    if(bipolar_)
    {
        v = calcBipolar(msb,lsb_,device_->high_res_cc_);
		a= !(msb == 64 && lsb_<1);  // hires lsb=0 or -1
    }
    else
    {
        v = calcUnipolar(msb,lsb_,device_->high_res_cc_);
		a= !( msb == 0 && (device_->high_res_cc_==false || lsb_<1));  // hires lsb=0 or -1
    }
    LOG_SINGLE(pic::logmsg() << "strip update " << t << " a " << a << " value " << v;)

	if(a != active_)
	{
		active_ = a;
		if(active_)
		{
			LOG_SINGLE(pic::logmsg() << "strip update :start " << t;)
			output_ = piw::xevent_data_buffer_t(OUT_STRIP_MASK,PIW_DATAQUEUE_SIZE_NORM);
			source_start(0,id_.restamp(t),output_);

			// TODO, implement 'resting time'
			// as we dont have 'touch on' then we have to assume origin is always 0,
			// but we could say after N samples, we automatically release, then we could use orgin
			origin_ = 0;
			last_update_=0;
		}
		else
		{
			LOG_SINGLE(pic::logmsg() << "strip update :end " << t;)
			output_.add_value(OUT_S_STRIP_REL,piw::makefloat_bounded_nb(1,-1,0,0,t));
			output_.add_value(OUT_S_STRIP_ABS,piw::makefloat_bounded_nb(1,-1,0,0,t));
			cur_val=0;
			source_end(t+1);
			return;
		}
	}

	if(!active_)
		return;

	if(cur_val==v)
		return;

	if(t < (last_update_ + device_->updatePeriod()))
	{
		LOG_SINGLE(pic::logmsg() << "strip update :ignoring " << t << " next "<< last_update_ + device_->updatePeriod() <<" up " << last_update_ <<  " per " << device_->updatePeriod();)
		return;
	}

	float rel = 0;
	if(relative_)
	{
		rel = v-cur_val;
	}
	else
	{
		rel	= origin_- v;
	}
	cur_val=v;
	lsb_=-1;
	last_update_=t;

	LOG_SINGLE(pic::logmsg() << "strip update :update " << t << " val " << v;)
	output_.add_value(OUT_S_STRIP_REL,piw::makefloat_bounded_nb(1,-1,0,rel,t));
	output_.add_value(OUT_S_STRIP_ABS,piw::makefloat_bounded_nb(1,-1,0,v,t));
}

void breath_t::update(unsigned long long t, unsigned msb)
{
    bool a = true;
    float v = 0.0;
    if(bipolar_)
    {
        v = calcBipolar(msb,lsb_,device_->high_res_cc_);
		a= !(msb == 64 && lsb_<1);  // hires lsb=0 or -1
    }
    else
    {
        v = calcUnipolar(msb,lsb_,device_->high_res_cc_);
		a= !( msb == 0 && (device_->high_res_cc_==false || lsb_<1));  // hires lsb=0 or -1
    }

	LOG_SINGLE(pic::logmsg() << "breath update " << t << " a " << a << " value " << v;)
	if(a != active_)
	{
		active_ = a;
		if(active_)
		{
			LOG_SINGLE(pic::logmsg() << "breath update:start " << t;)
			output_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_NORM);
			source_start(0,id_.restamp(t),output_);
			last_update_=0;
		}
		else
		{
			LOG_SINGLE(pic::logmsg() << "breath update:end " << t;)
			output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,0,t));
			cur_val=0;
			source_end(t);
			return;
		}
	}

	if(!active_)
		return;

	if(cur_val==v)
		return;

	if(t < (last_update_ + device_->updatePeriod()))
	{
		LOG_SINGLE(pic::logmsg() << "breath update :ignoring " << t << " next "<< last_update_ + device_->updatePeriod() <<" up " << last_update_ <<  " per " << device_->updatePeriod();)
		return;
	}

	float val= v;
	if(relative_)
	{
		val = v-cur_val;
	}
	cur_val=v;
	lsb_=-1;
	last_update_=t;

	LOG_SINGLE(pic::logmsg() << "breath update :update " << t << " val " << val;)
	output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,val,t));
}

void pedal_t::update(unsigned long long t, unsigned msb)
{
	//TODO, not quite sure if pedal is really -1 to 1, code looks like pedal can be configured to be -ve or +ve
	// im assume -1  to 1
    bool a = true;
    float v = 0.0;
    if(bipolar_)
    {
        v = calcBipolar(msb,lsb_,device_->high_res_cc_);
		a= !(msb == 64 && lsb_<1);  // hires lsb=0 or -1
    }
    else
    {
        v = calcUnipolar(msb,lsb_,device_->high_res_cc_);
		a= !( msb == 0 && (device_->high_res_cc_==false || lsb_<1));  // hires lsb=0 or -1
    }
	LOG_SINGLE(pic::logmsg() << "pedal update " << t << " a " << a << " value " << v;)

	float p=v;
	if( a != active_)
	{
		active_ = a;

		if(active_)
		{
			output_ = piw::xevent_data_buffer_t(1,PIW_DATAQUEUE_SIZE_NORM);
			source_start(0,id_.restamp(t),output_);
			last_update_=0;
		}
		else
		{
			output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,0,t));
			source_end(t);
			return;
		}
	}

	if(!active_)
		return;

	if(cur_val==p)
		return;

	if(t < (last_update_ + device_->updatePeriod()))
		return;

	cur_val=p;
	lsb_=-1;
	last_update_=t;

	output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,p,t));
}



}; //namespace midi_device_plg


