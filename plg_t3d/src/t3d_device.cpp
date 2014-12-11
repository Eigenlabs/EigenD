#include "t3d_device.h"

#include <piw/piw_data.h>
#include <piw/piw_status.h>
#include <piw/piw_bundle.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_keys.h>

#include <picross/pic_ref.h>
#include <picross/pic_ref.h>
#include <picross/pic_time.h>

#include <math.h>

#include <lib_op/osc/OscReceivedElements.h>
#include <lib_op/osc/OscPacketListener.h>
#include <lib_op/ip/UdpSocket.h>
#include <lib_op/osc/OscOutboundPacketStream.h>
#include <lib_ot/zeroconf/NetService.h>


// t3d messages - quickino
// default port 3123 
// /t3d/dr i 200   data rate
// /t3d/frm ii 17149 291  frame ?
// /t3d/frm ii 17151 291
// /t3d/tch1 ffff 0.021484 0.034180 0.041260 53.000000      tchN R Y Z note
// /t3d/tch1 ffff 0.000000 0.000000 0.000000 0.000000

// IMPLEMENTATION NOTES/IDEAS

// generally absx/y are used to determine a key location, which is independant of note
// note is also output as a separate frequency
// the choice is the users, as to which is more suitable - (using freqeuncy, will use the note layout specified in the soundplane software)

// 3 modes


// A) TOUCH MODE = FALSE IMPLEMENTATION
// we use the initial touch position as a key, then just roll and yaw from there
// this is most 'eigenharp' like, and so compatible with all agents  (which continuous is not, see below)
// BUT it wont work if you start two touches from the same position  e.g. touch starts on C3, then slide to D4 (hold), start a second touch on C3 slide to D2
// (this will confused EigenD since, we will now use the same kwire for the second touch!)

// device key position from abs x/y - kwire, are  physical keys, kwire[row*column_size+column]
// since sometimes its useful in eigend to be able to filter by key number (id)
// current functionality is midi like ( and definitely subject to change)
// i.e. the intial touch defines which 'key' is pressed, then the roll/yaw are offsets from that position
// we will then allow scaling, i.e. does moving to the next virtual key boundary =-1/+1 roll/yaw  (eigenharp style!)
// OR is it relative to N keys. the advantage of the later is to allow a continuum style slide (30 keys = whole slide, and pitchbend range is set accordingly)


// TOUCH MODE KEY = TRUE / CONTINUOUS KEY = FALSE IMPLEMENTATION ( very experimental!!!!)
// kwires are actually touches, the starting key is calculated just at initial start
// then roll and yaw are relative to it.
// this quite like the eigenharp in that the event doesnt change key, so less likely to cause trouble the continuous keys
// BUT it does means its possible that two touches may start on the key and be simultaneously active,
// which may cause issues, but probably not, as we get this with splits and alike.


// TOUCH MODE KEY = TRUE / CONTINUOUS KEY = TRUE IMPLEMENTATION ( very experimental!!!!)
// this is more soundplane like, the kwires are used as touch wires.
// every time we move from key to key, we send out a new key message.
// this could very well confused many agents as they expect a key to start at the beginning of an event, and not change.
// i.e. pressure/roll/yaw are the variants
// it is most likely this mode would work best with agents that can ignore the key and instead use frequency directly
// kwires are the touches in progress, kwire[touch]
// likely issues with continous mode and other agents
// agents may only expect KEY on event begin, and even ignore subsequent KEY events
// agents may use the KEY event as 'note on', and so cause retriggering
// scaler, I fear may not recalculate frequency on key event


// x= column (across)  y =row (vertical)


// turn logging by setting LOG_SINGLE(x) x
#define  LOG_SINGLE(x) x

//input signals
#define IN_LIGHT 1
#define IN_MASK SIG1(IN_LIGHT)


// output signals
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
#define OUT_S_FREQ 5
#define OUT_KEY_MASK SIG5(OUT_S_KEY,OUT_S_PRESSURE,OUT_S_ROLL,OUT_S_YAW,OUT_S_FREQ)



#define OUT_S_STRIP_REL 1
#define OUT_S_STRIP_ABS 2
#define OUT_STRIP_MASK SIG2(OUT_S_STRIP_REL,OUT_S_STRIP_ABS)


// constants, later replace with properties
#define MAX_COLS 5
#define MAX_ROWS 30
#define MAX_KEYS ( MAX_COLS * MAX_ROWS )
#define MAX_TOUCHES MAX_KEYS


#define DEFAULT_PORT 3999



namespace t3d_device_plg
{
	enum t3d_ctrl_t
	{
		T_TOUCH,
		T_BREATH,
		T_STRIP1,
		T_STRIP2
	};

	struct t3d_message_t
	{
		t3d_ctrl_t type_;
		unsigned touchId;
		float note;
		float x;
		float y;
		float z;
	};


//////////// t3d_device_t::impl_t  declaration

	// fwd declsvel
	struct kwire_t;
	struct cwire_t;
	struct breath_t;
	struct strip_t;
	struct pedal_t;

	class t3d_listener_t: public osc::OscPacketListener,piw::thing_t
	{
	public:
		t3d_listener_t(t3d_device_t::impl_t* device) : device_(device) { piw::tsd_thing(this); };
		virtual void ProcessMessage( const osc::ReceivedMessage& m,  const IpEndpointName& remoteEndpoint );

		// thing to transfer to fast thread
		void thing_dequeue_fast(const piw::data_nb_t &d);
	private:
		t3d_device_t::impl_t* device_;
	};

	class t3d_server_t: public pic::thread_t
	{
	public:
		t3d_server_t(t3d_listener_t* listener);
		virtual ~t3d_server_t();

		// thread functions
		void thread_main();
		void thread_init();
		void thread_term();

		// start and stop the server
		void start();
		void stop();
		void port(unsigned port);

	private:
		unsigned port_;
		UdpListeningReceiveSocket *socket_;
		t3d_listener_t* listener_;
	};



	class ZeroConfServiceListener : public ZeroConf::NetServiceListener
	{
	public:
	  void willPublish(ZeroConf::NetService *pNetService) { LOG_SINGLE(pic::logmsg() << "ZeroConfServiceListener::willPublish";)}
	  void didNotPublish(ZeroConf::NetService *pNetService) { LOG_SINGLE(pic::logmsg() << "ZeroConfServiceListener::didNotPublish";)}
	  void didPublish(ZeroConf::NetService *pNetService)
	  {
		  LOG_SINGLE(pic::logmsg() << "didPublish " << "name: " << pNetService->getName() << " port: " << pNetService->getPort();)
	  }
	  void willResolve(ZeroConf::NetService *pNetService) { LOG_SINGLE(pic::logmsg() << "ZeroConfServiceListener::willResolve";)}
	  void didNotResolve(ZeroConf::NetService *NetService) { LOG_SINGLE(pic::logmsg() << "ZeroConfServiceListener::didNotResolve";)}
	  void didResolveAddress(ZeroConf::NetService *pNetService) { LOG_SINGLE(pic::logmsg() << "ZeroConfServiceListener::didResolveAddress";)}
	  void didUpdateTXTRecordData(ZeroConf::NetService *pNetService) { LOG_SINGLE(pic::logmsg() << "ZeroConfServiceListener::didUpdateTXTRecordData";)}
	  void didStop(ZeroConf::NetService *pNetService) { LOG_SINGLE(pic::logmsg() << "ZeroConfServiceListener::didStop";)}
	};

	struct light_wire_t:
		piw::wire_t,
		piw::wire_ctl_t,
		piw::event_data_sink_t,
		piw::event_data_source_real_t,
		virtual public pic::lckobject_t,
		pic::element_t<>
	{
		light_wire_t(t3d_device_t::impl_t *p, const piw::event_data_source_t &);
		~light_wire_t() { invalidate(); }

		void wire_closed() { delete this; }
		void invalidate();
		void event_start(unsigned,const piw::data_nb_t &, const piw::xevent_data_buffer_t &);
		bool event_end(unsigned long long);
		void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);

		void ticked(unsigned long long f, unsigned long long t);
		void source_ended(unsigned);

		t3d_device_t::impl_t *root_;
		piw::xevent_data_buffer_t::iter_t input_;
		piw::xevent_data_buffer_t output_;

		unsigned long long last_from_;
		unsigned long long light_t_;
	};


    //////////// t3d_device_t::impl_t  declaration
    // this is based on latch, please look there for general comments on eigenD usage
    struct t3d_device_t::impl_t :
    	    piw::root_t,
    	    piw::root_ctl_t,
    	    piw::clocksink_t,
    		piw::thing_t,
    	    virtual pic::lckobject_t
    {
        impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t & outputs);
        ~impl_t();
        void stop();
        void connect(unsigned,unsigned);

        void clocksink_ticked(unsigned long long f, unsigned long long t);
        void invalidate();
        void add_ticker(light_wire_t *w);
        void del_ticker(light_wire_t *w);
        piw::wire_t *root_wire(const piw::event_data_source_t &es);
        void root_closed();
        void root_opened();
        void root_clock();
        void root_latency();

        void create_wires();

        void clear_touches();
        void handle_message(unsigned long long t, const piw::data_nb_t &d);
        void handle_touch(unsigned long long t, unsigned touch, float note, float x, float y, float z);
        void handle_control(unsigned long long t, enum t3d_ctrl_t type, float x);
        void sendLED(unsigned key,unsigned value);


//		void processLights(piw::data_nb_t &d);
		void updateLights(unsigned key,unsigned value);
	    void thing_dequeue_slow(const piw::data_t &);
	    void createLEDConverter();



        // helper funcions
        unsigned long updatePeriod() { return update_period_;}

        pic::lckmap_t<piw::data_t, light_wire_t *>::lcktype children_;
        pic::ilist_t<light_wire_t> tickers_;
        bct_clocksink_t *up_;
		piw::statusledconvertor_t* led_converter_;

        // device wires, one 'per key'
        unsigned ledcols_[MAX_COLS];
        pic::ref_t<kwire_t> kwires_[MAX_KEYS];

        pic::ref_t<strip_t> strip1_;
        pic::ref_t<strip_t> strip2_;
        pic::ref_t<breath_t> breath_;
        pic::ref_t<pedal_t> pedal1_;
        pic::ref_t<pedal_t> pedal2_;
        pic::ref_t<pedal_t> pedal3_;
        pic::ref_t<pedal_t> pedal4_;


        t3d_listener_t  t3d_listener;
        t3d_server_t	t3d_server;
        UdpTransmitSocket *t3d_led_socket;

        // behavioural controls
        unsigned long update_period_;
        unsigned long long last_event_t_;
        unsigned int row_size_;
        unsigned int col_size_;
        bool whole_roll_;
        bool whole_yaw_;
        bool continuous_key_;
        bool touch_mode_;

        int touch_map_[MAX_TOUCHES]; // which touch is currently active on which key
    };


struct cwire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t, virtual public pic::counted_t
{
	cwire_t(const piw::data_t &,t3d_device_t::impl_t *);
	virtual ~cwire_t() { source_shutdown(); }

	void source_ended(unsigned seq) {}

	virtual void update(unsigned long long t,float v)=0;
	void release(bool v) { release_=v;}
	void relative(bool v) { relative_=v;}
	void bipolar(bool v) { bipolar_=v;}

	t3d_device_t::impl_t * device_;
	piw::data_nb_t id_;
	piw::xevent_data_buffer_t output_;
	bool active_;
	float cur_val;
	unsigned long long last_update_;

	bool release_;
	bool relative_;
	bool bipolar_;
};

struct breath_t: cwire_t
{
	breath_t(const piw::data_t &d,t3d_device_t::impl_t *k): cwire_t(d,k) {}
	void update(unsigned long long t, float v);
};

struct pedal_t : cwire_t
{
	pedal_t( const piw::data_t &d,t3d_device_t::impl_t *k ) : cwire_t(d,k) {}
	void update(unsigned long long t, float v);
};

struct strip_t: cwire_t
{
	strip_t(const piw::data_t &d,t3d_device_t::impl_t *k): cwire_t(d,k), origin_(0) {}
	void update(unsigned long long t, float v);
    unsigned origin_;
};

struct kwire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::lckobject_t, virtual public pic::counted_t
{
	kwire_t(unsigned i,unsigned c,unsigned r,const piw::data_t &,t3d_device_t::impl_t  *);
	~kwire_t() { source_shutdown(); device_ = 0; }

	void source_ended(unsigned seq) {}

	void updateKey(unsigned long long t, bool a, unsigned touch, float note, float x, float y, float z);
	void updateTouch(unsigned long long t, bool a, unsigned touch, float note, float x, float y, float z);

	unsigned course_note_,column_,row_;
	unsigned touch_;
	piw::data_nb_t id_;
	t3d_device_t::impl_t *device_;
	bool running_;
	piw::xevent_data_buffer_t output_;
	float cur_pressure_, cur_roll_, cur_yaw_,cur_freq_;
	unsigned long long last_update_;
};




//////////////// implemenations
//static const float CLIP_WINDOW=1.0;

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




/// t3d_device_t::impl_t implementation
t3d_device_t::impl_t::impl_t(piw::clockdomain_ctl_t *cd,  const piw::cookie_t &outputs) :
	root_t(0), up_(0),led_converter_(NULL),
    t3d_listener(this),
    t3d_server(&t3d_listener),
    t3d_led_socket(NULL),
	update_period_(0),
	row_size_(MAX_ROWS), col_size_(MAX_COLS),
	whole_roll_(false),whole_yaw_(false),continuous_key_(false),touch_mode_(false)
{
	for(int i=0;i<MAX_TOUCHES;i++)
	{
		touch_map_[i]=-1;
	}

	piw::root_ctl_t::connect(outputs);

	create_wires();

	cd->sink(this,"t3ddevice");
	tick_enable(true);

	set_clock(this);

	createLEDConverter();

	piw::tsd_thing(this);
}


void t3d_device_t::impl_t::createLEDConverter()
{
	for(int i=0;i<col_size_;i++)
	{
		ledcols_[i]=row_size_;
	}
	if(led_converter_!=NULL) delete led_converter_;
	led_converter_=new piw::statusledconvertor_t(col_size_,ledcols_);
}

t3d_device_t::impl_t::~impl_t()
{
	LOG_SINGLE(pic::logmsg() << "t3d_device_t::impl_t::~impl_t";)
	stop();
	tracked_invalidate();
	invalidate();
	delete led_converter_;
}

void t3d_device_t::impl_t::stop()
{
	LOG_SINGLE(pic::logmsg() << "t3d_device_t::impl_t::stop";)
	if(t3d_led_socket!=NULL)
	{
		delete t3d_led_socket;
		t3d_led_socket=NULL;
	}
	if(t3d_server.isrunning())
		t3d_server.stop();
}

void t3d_device_t::impl_t::connect(unsigned server_port,unsigned light_port)
{
	LOG_SINGLE(pic::logmsg() << "t3d_device_t::impl_t::connect";)

	if(t3d_led_socket!=NULL)
	{
		delete t3d_led_socket;
		t3d_led_socket=NULL;
	}
	if(light_port>0)
	{
		LOG_SINGLE(pic::logmsg() << "t3d_device_t::impl_t::connect light port" << light_port;)
		t3d_led_socket= new UdpTransmitSocket(IpEndpointName(light_port));
		// connect to light port
	}

	t3d_server.port(server_port);
	if(server_port>0)
	{
		t3d_server.start();
	}
}


void t3d_device_t::impl_t::sendLED(unsigned key, unsigned value)
{
	static const int BS=128;
	if(t3d_led_socket==NULL) return;
    char buffer[BS];
    osc::OutboundPacketStream p( buffer, BS );

    // col/row are from 1, and we want zero BUT, we also want the mid point
   float x=float(kwires_[key]->row_-0.5)/row_size_;
   float y=float(kwires_[key]->column_-0.5)/col_size_;

    p << osc::BeginBundleImmediate
      << osc::BeginMessage( "/t3d/led" )
      << x << y << (int) value
      << osc::EndMessage
      << osc::EndBundle;

    t3d_led_socket->Send( p.Data(), p.Size() );
}


void t3d_device_t::impl_t::create_wires()
{
    int k=0;
	for(unsigned c=0; c < col_size_ ;c++)
	{
		for(unsigned r=0; r < row_size_ ;r++,k++)
		{
			kwires_[k] = pic::ref(new kwire_t(k+1,c+1,r+1,piw::pathtwo(OUT_KEY,k+1,0),this));
			LOG_SINGLE(pic::logmsg() << "create wire "<<  k << " cn " << kwires_[k]->course_note_ << " c " << kwires_[k]->column_ << " r " << kwires_[k]->row_;)
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
}


void t3d_device_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
	light_wire_t *w;

	for(w=tickers_.head(); w!=0; w=tickers_.next(w))
	{
		w->ticked(f,t);
	}
}

void t3d_device_t::impl_t::add_ticker(light_wire_t *w)
{
	if(!tickers_.head())
	{
		tick_suppress(false);
	}

	tickers_.append(w);
}

void t3d_device_t::impl_t::del_ticker(light_wire_t *w)
{
	tickers_.remove(w);

	if(!tickers_.head())
	{
		tick_suppress(true);
	}

}

void t3d_device_t::impl_t::invalidate()
{
	tick_disable();

	pic::lckmap_t<piw::data_t,light_wire_t *>::lcktype::iterator ci;
	while((ci=children_.begin())!=children_.end())
	{
		delete ci->second;
	}
}

piw::wire_t *t3d_device_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
   pic::lckmap_t<piw::data_t,light_wire_t *>::lcktype::iterator ci;

	if((ci=children_.find(es.path()))!=children_.end())
	{
		delete ci->second;
	}

	return new light_wire_t(this, es);
}

void t3d_device_t::impl_t::root_closed() { invalidate(); }

void t3d_device_t::impl_t::root_opened() { root_clock(); root_latency(); }

void t3d_device_t::impl_t::root_clock()
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

void t3d_device_t::impl_t::root_latency()
{
	set_latency(get_latency());
}


void t3d_device_t::impl_t::handle_message(unsigned long long t, const piw::data_nb_t &d)
{
//	LOG_SINGLE(pic::logmsg() << "handle_message t " << t;)
	last_event_t_=t;
	t3d_message_t *pMsg = (t3d_message_t *)(d.as_blob());

	switch (pMsg->type_)
	{
		case T_TOUCH:
			handle_touch(t,pMsg->touchId,pMsg->note,pMsg->x,pMsg->y,pMsg->z);
			break;
		case T_BREATH:
		case T_STRIP1:
		case T_STRIP2:
			handle_control(t,pMsg->type_,pMsg->y);
			break;
	}
	last_event_t_=0;
}

void t3d_device_t::impl_t::clear_touches()
{
	LOG_SINGLE(pic::logmsg()  << "t3d_device_t::impl_t::clear_touches";)
	unsigned long long t = piw::tsd_time();
	for(int i=0;i<MAX_TOUCHES;i++)
	{
		if(touch_map_[i]>0.0)
		{
			handle_touch(t,i,0.0,0.0,0.0,0.0);
			touch_map_[i]=-1;
		}
	}
	//TODO clean up this!
	// reset row/columns
    int k=0;
	for(unsigned c=0; c < col_size_ ;c++)
	{
		for(unsigned r=0; r < row_size_ ;r++,k++)
		{
			kwires_[k]->course_note_=k+1;
			kwires_[k]->row_=r+1;
			kwires_[k]->column_=c+1;
//			LOG_SINGLE(pic::logmsg() << "clear_touches cn" << kwires_[k]->course_note_ << " c " << kwires_[k]->column_ << " r " << kwires_[k]->row_;)
		}
	}
	createLEDConverter();
}


void t3d_device_t::impl_t::handle_touch(unsigned long long t, unsigned touch, float note, float x, float y, float z)
{
//    LOG_SINGLE(pic::logmsg() << "t3d_device_t::impl_t::handle_touch t " << t << " touch " << touch << " note " << note << " x " << x << " y " << y << " z " << z;)

	bool a= (z != 0.0); // is this note active?

    if(touch_mode_)
    {
    	// we only use the touch map here, so we know if a touch is active or not.
    	int key=touch-1;
//		LOG_SINGLE(pic::logmsg() << "t3d_device_t::impl_t::handle_touch touch "<< key << " a " << a;)
		kwires_[key]->updateTouch(t,a,touch,note,x,y,z);
		touch_map_[touch-1]=touch;
		if(!a)
		{
			touch_map_[touch-1]=-1;
		}
    }
    else
    {
		int key=touch_map_[touch-1];

		if(key<0)
		{
			// new touch
			int r=x*row_size_;
			int c=y*col_size_;

			r = r<row_size_ ? r : r-1;  // x = 1.0, want it in r 4 not 5 (0-4 range)
			c = c<col_size_ ? c : c-1;

			key= (c * row_size_ ) + r;
			LOG_SINGLE(pic::logmsg() << "t3d_device_t::impl_t::handle_touch key " << key << " r " << r << " c " << c  << " rs " << row_size_  << " cs " << col_size_;)

			touch_map_[touch-1]=key;
		}

//		LOG_SINGLE(pic::logmsg() << "t3d_device_t::impl_t::handle_touch key "<< key << " a " << a;)
		kwires_[key]->updateKey(t,a,touch,note,x,y,z);

		if(!a)
		{
			touch_map_[touch-1]=-1;
		}
    }
}

void t3d_device_t::impl_t::handle_control(unsigned long long t, enum t3d_ctrl_t type, float x)
{
	switch (type)
	{
		case T_TOUCH:
			// separate case
			break;
		case T_BREATH:
			breath_->update(t,x);
			break;
		case T_STRIP1:
			strip1_->update(t,x);
			break;
		case T_STRIP2:
			strip2_->update(t,x);
			break;
	}
}

struct led_msg
{
	unsigned key;
	unsigned value;
};

void t3d_device_t::impl_t::updateLights(unsigned key,unsigned value)
{
	unsigned note=key;
	unsigned out_value = 0;
	switch (value)
	{
		case CLR_OFF: out_value=0;break;
		case CLR_RED: out_value=1;break;
		case CLR_GREEN: out_value=2;break;
		case CLR_ORANGE: out_value=3;break;
		case CLR_MIXED: out_value=4;break;
		default: out_value=1;
	}

	sendLED(note,out_value);
}


static void __updateLights(void* i, unsigned key, unsigned value)
{
	((t3d_device_t::impl_t*) i)->updateLights(key,value);
}


void t3d_device_t::impl_t::thing_dequeue_slow(const piw::data_t &d)
{
    piw::data_nb_t data = piw::data_nb_t::from_given((bct_data_t)d.give_copy());

	led_converter_->update_leds(data,this,__updateLights);
}


//////////// t3d_device_t
t3d_device_t::t3d_device_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &outputs)
	: impl_(new impl_t(d,outputs))
{
}

t3d_device_t::~t3d_device_t()
{
	delete impl_;
}

piw::cookie_t t3d_device_t::cookie()
{
    return piw::cookie_t(impl_);
}


void t3d_device_t::col_size(int v)
{
	if(impl_)
	{
		impl_->col_size_=v;
	}
	impl_->clear_touches(); // required to reset col/rows
}

void t3d_device_t::row_size(int v)
{
	if(impl_)
	{
		impl_->row_size_=v;
	}
	impl_->clear_touches(); // required to reset col/rows
}

void t3d_device_t::whole_roll(bool v)
{
	if(impl_)
	{
		impl_->whole_roll_=v;
	}
}

void t3d_device_t::whole_yaw(bool v)
{
	if(impl_)
	{
		impl_->whole_yaw_=v;
	}
}


static int continuous_key__(void *i_, void * v_)
{
	t3d_device_t::t3d_device_t::impl_t *i = (t3d_device_plg::t3d_device_t::impl_t *)i_;
	bool v= *((bool*) v_);
	if(v!=i->continuous_key_)
	{
		// first clear all key events which might be outstanding
		// using the existing kwire mapping
		i->clear_touches();
		// now change to new mode
		if(v==true) i->touch_mode_=true;  // touch mode is implicit when using continuous
		i->continuous_key_=v;
	}
    return 0;
}

void t3d_device_t::continuous_key(bool v)
{
	piw::tsd_fastcall(continuous_key__,impl_,&v);
}

static int touch_mode__(void *i_, void * v_)
{
	t3d_device_t::t3d_device_t::impl_t *i = (t3d_device_plg::t3d_device_t::impl_t *)i_;
	bool v= *((bool*) v_);
	if(v!=i->touch_mode_)
	{
		// first clear all key events which might be outstanding
		// using the existing kwire mapping
		i->clear_touches();
		// now change to new mode
		i->touch_mode_=v;
	}
    return 0;
}

void t3d_device_t::touch_mode(bool v)
{
	piw::tsd_fastcall(touch_mode__,impl_,&v);
}



piw::data_t  t3d_device_t::get_columnlen()
{
	piw::data_t courses = piw::tuplenull(0);
	for(int i=0;i<impl_->col_size_;i++)
	{
		courses = piw::tupleadd(courses, piw::makelong(impl_->row_size_,0));
	}
    return courses;
}

piw::data_t  t3d_device_t::get_columnoffset()
{
	piw::data_t courses = piw::tuplenull(0);
	for(int i=0;i<impl_->col_size_;i++)
	{
	    courses = piw::tupleadd(courses, piw::makelong(0,0));
	}

    return courses;
}

piw::data_t  t3d_device_t::get_courselen()
{
	piw::data_t courses = piw::tuplenull(0);
	courses = piw::tupleadd(courses, piw::makelong(impl_->col_size_*impl_->row_size_,0));
    return courses;
}

piw::data_t  t3d_device_t::get_courseoffset()
{
    piw::data_t courses = piw::tuplenull(0);
	courses = piw::tupleadd(courses, piw::makefloat(0,0));
    return courses;
}


static int stopper__(void *i_, void *)
{
	t3d_device_t::t3d_device_t::impl_t *i = (t3d_device_plg::t3d_device_t::impl_t *)i_;
	i->stop();
    return 0;
}

void t3d_device_t::stop()
{
	piw::tsd_fastcall(stopper__,impl_,0);
}

void t3d_device_t::connect(unsigned server_port,unsigned client_port)
{
	piw::tsd_fastcall(stopper__,impl_,0);
	impl_->connect(server_port,client_port);
}



///////////////////////////////////////////////////////////
// control wires implementations
///////////////////////////////////////////////////////////

kwire_t::kwire_t(unsigned i, unsigned c, unsigned r, const piw::data_t &path, t3d_device_t::impl_t  *k)
	: piw::event_data_source_real_t(path),
	course_note_(i), column_(c), row_(r),
	id_(piw::pathone_nb(i,0)),
	device_(k), running_(false),
	output_(OUT_KEY_MASK,PIW_DATAQUEUE_SIZE_NORM),
	cur_pressure_(0), cur_roll_(0), cur_yaw_(0),cur_freq_(0),
	last_update_(0)
{
    k->connect_wire(this,source());
}


void kwire_t::updateKey(unsigned long long t, bool a, unsigned touch, float note, float absx, float absy, float absz)
{
	// this method works on the first touch determines key, and this is then invariant
    LOG_SINGLE(pic::logmsg() << "kwire_t::updateKey start t " << t << " c " << column_ << " r " << row_ <<" a " << a << " touch " <<  touch << " note " << note << " absx " << absx << " absy " << absy << " z " << absz;)

    if(!a)
    {
    	if(running_)
    	{
    		LOG_SINGLE(pic::logmsg() << "kwire_t::updateKey source end t " << t << " a " << a << " touch " << touch <<  " note " << note << " absx " << absx << " absy " << absy << " z " << absz;)
			source_end(t);
    	}
		running_=false;
    }
    else
    {

    	// calculate, such that 1 cell is the boundary for roll/yaw, like a discrete key
    	// continuous the board surface is considered the boundary for roll and yaw
    	// discrete calcuate by comparing which cell absx/y would give us, and then compare against mid point of actual row/col
    	// continous, absx- (r-1+0.5)/nrow

    	float x=0.0,y=0.0,z=0.0;
    	z = ( absz < 0.00001 ? 0.0 : (absz > 1.0 ? 1.0 : absz));

    	if(device_->whole_roll_)
    		x = __clip(absx - ((row_ - 1 + 0.5 )/device_->row_size_) ,1.0);
    	else
    		x = __clip(((absx * device_->row_size_)  -  ( row_ - 1 + 0.5)) * 2.0, 1.0);

    	if(device_->whole_yaw_)
    	{
    		y = __clip(absy - ((column_ - 1 + 0.5 )/device_->col_size_) ,1.0);
    	}
    	else
    	{
    		y = __clip(((absy * device_->col_size_)  -  ( column_ - 1 + 0.5))    * 2.0, 1.0);
    	}


		LOG_SINGLE(pic::logmsg() << "kwire_t::updateKey update " << " a " << a << " touch " << touch <<  " note " << note << " absx " << absx << " absy " << absy << " z " << z <<  " c " << column_ << " r " << row_ <<  " roll " << x << " yaw " << y;)

    	float freq= 440.0*powf(2.0,(note-69.0)/12.0);

    	if(!running_) // first event
    	{
    		LOG_SINGLE(pic::logmsg() << "kwire_t::updateKey source start c " << column_ << " r " << row_ << " t " << t << " id " << id_;)
			piw::hardness_t h=piw::KEY_HARD;  // we always get a very light initial touchs .. so need to calibrate this  TODO
//			piw::hardness_t h=piw::KEY_LIGHT;
//			if(z>=.25) h=piw::KEY_SOFT;
//			if(z>=.5) h=piw::KEY_HARD;

			output_.add_value(OUT_S_KEY,piw::makekey(column_,row_,1,course_note_,h,t));
			output_.add_value(OUT_S_ROLL,piw::makefloat_bounded_nb(1,-1,0,x,t));
			output_.add_value(OUT_S_YAW,piw::makefloat_bounded_nb(1,-1,0,y,t));
			output_.add_value(OUT_S_PRESSURE,piw::makefloat_bounded_nb(1,0,0,z,t));
			output_.add_value(OUT_S_FREQ,piw::makefloat_bounded_units_nb(BCTUNIT_HZ,96000,0,0,freq,t));
            source_start(0,id_.restamp(t),output_);

            running_=true;
    	}
    	else // updates
    	{
    	    if( t < (last_update_ + device_->updatePeriod()))
    	    {
    	    	LOG_SINGLE(pic::logmsg() << "kwire_t::updateKey:no update t" << t << " lu " << last_update_ << " up " << device_->updatePeriod();)
    	    	return;
    	    }

    	    if(cur_roll_!=x) output_.add_value(OUT_S_ROLL,piw::makefloat_bounded_nb(1,-1,0,x,t));
    	    if(cur_yaw_!=y) output_.add_value(OUT_S_YAW,piw::makefloat_bounded_nb(1,-1,0,y,t));
      	    if(cur_pressure_!=z) output_.add_value(OUT_S_PRESSURE,piw::makefloat_bounded_nb(1,0,0,z,t));
			if(cur_freq_!=freq) output_.add_value(OUT_S_FREQ,piw::makefloat_bounded_units_nb(BCTUNIT_HZ,96000,0,0,freq,t));
    	}
        last_update_=t;
        cur_roll_=x;
 	    cur_yaw_=y;
 	    cur_pressure_=z;
 	    cur_freq_=freq;
    }
}


void kwire_t::updateTouch(unsigned long long t, bool a, unsigned touch, float note, float absx, float absy, float z)
{
	// THIS MAY BE ROLLED BACK INTO updateKey, depending on how different it becomes, especially regarding roll/yaw
	// at the moment, difference is we send out key on updates
	// course_note, row and column are calculated and updated every update

    LOG_SINGLE(pic::logmsg() << "kwire_t::updateTouch start t " << t << " a " << a << " touch " <<  touch << " note " << note << " absx " << absx << " absy " << absy << " z " << z;)

    if(!a)
    {
    	if(running_)
    	{
    		LOG_SINGLE(pic::logmsg() << "kwire_t::updateTouch source end t " << t << " a " << a << " touch " << touch <<  " note " << note << " absx " << absx << " absy " << absy << " z " << z;)
			source_end(t);
    	}
		running_=false;
    }
    else
    {

		int r=row_;
		int c=column_;
		int cn=course_note_;
		piw::hardness_t h=piw::KEY_LIGHT;

		// if this is the first event OR we are in continuous key update mode, then calc column, row and coursenote
		if( !running_ || device_->continuous_key_)
		{
	    	// calculate row and column, and course key  UpdateTouch
			int r=absy*device_->row_size_;
			int c=absx*device_->col_size_;
			int cn=0;
			r = r<device_->row_size_ ? r : r-1;  // x = 1.0, want it in r 4 not 5 (0-4 range)
			c = c<device_->col_size_ ? c : c-1;
			cn = ((c * device_->col_size_ ) + c) + 1;
			if(z>=.25) h=piw::KEY_SOFT;
			if(z>=.5) h=piw::KEY_HARD;
		}

    	// calculate, such that 1 cell is the boundary for roll/yaw, like a discrete key
    	// continuous the board surface is considered the boundary for roll and yaw
    	// discrete calcuate by comparing which cell absx/y would give us, and then compare against mid point of actual row/col
    	// continous, absx- (r-1+0.5)/nrow

    	float x=0.0,y=0.0;

    	if(device_->whole_roll_)
    		x = __clip(absx - ((c - 1 + 0.5 )/device_->col_size_) ,1.0);
    	else
    		x = __clip(((absx * device_->col_size_)  -  ( c - 1 + 0.5))    * 2.0, 1.0);

    	if(device_->whole_yaw_)
    		y = __clip(absy - ((r - 1 + 0.5 )/device_->row_size_) ,1.0);
    	else
    		y = __clip(((absy * device_->row_size_)  -  ( r - 1 + 0.5)) * 2.0, 1.0);

    	float freq= 440.0*powf(2.0,(note-69.0)/12.0);

		LOG_SINGLE(pic::logmsg() << "kwire_t::updateTouch update c " << c << " r " << r <<  " roll " << x << " yaw " << y;)

    	if(!running_) // first event
    	{
    		LOG_SINGLE(pic::logmsg() << "kwire_t::updateTouch source start c " << column_ << " r " << row_ << " t " << t << " id " << id_;)
			output_.add_value(OUT_S_KEY,piw::makekey(c,r,1,cn ,h,t));
			output_.add_value(OUT_S_ROLL,piw::makefloat_bounded_nb(1,-1,0,x,t));
			output_.add_value(OUT_S_YAW,piw::makefloat_bounded_nb(1,-1,0,y,t));
			output_.add_value(OUT_S_PRESSURE,piw::makefloat_bounded_nb(1,0,0,z,t));
			output_.add_value(OUT_S_FREQ,piw::makefloat_bounded_units_nb(BCTUNIT_HZ,96000,0,0,freq,t));
            source_start(0,id_.restamp(t),output_);

            running_=true;
    	}
    	else // updates
    	{
    	    if( t < (last_update_ + device_->updatePeriod()))
    	    {
    	    	LOG_SINGLE(pic::logmsg() << "kwire_t::updateTouch:no update t" << t << " lu " << last_update_ << " up " << device_->updatePeriod();)
    	    	return;
    	    }

    	    if(row_!=r || column_!=c) output_.add_value(OUT_S_KEY,piw::makekey(c,r,1,cn,h,t));
    	    if(cur_roll_!=x) output_.add_value(OUT_S_ROLL,piw::makefloat_bounded_nb(1,-1,0,x,t));
    	    if(cur_yaw_!=y) output_.add_value(OUT_S_YAW,piw::makefloat_bounded_nb(1,-1,0,y,t));
      	    if(cur_pressure_!=z) output_.add_value(OUT_S_PRESSURE,piw::makefloat_bounded_nb(1,0,0,z,t));
			if(cur_freq_!=freq) output_.add_value(OUT_S_FREQ,piw::makefloat_bounded_units_nb(BCTUNIT_HZ,96000,0,0,freq,t));
    	}

        last_update_=t;
        cur_roll_=x;
 	    cur_yaw_=y;
 	    cur_pressure_=z;
 	    cur_freq_=freq;
 	    row_=r;
 	    column_=c;
 	    course_note_=cn;
    }
}



cwire_t::cwire_t(const piw::data_t &path, t3d_device_t::impl_t  *k):
		piw::event_data_source_real_t(path), device_(k), id_(piw::pathnull_nb(0)),
		active_(false),cur_val(0.0), release_(true), relative_(false), bipolar_(true)
{
    k->connect_wire(this,source());
}



void strip_t::update( unsigned long long t, float v)
{
    bool a = true;
    LOG_SINGLE(pic::logmsg() << "strip update " << t << " a " << a << " value " << v;)

	if(a != active_)
	{
		active_ = a;
		if(active_)
		{
			LOG_SINGLE(pic::logmsg() << "strip update :start " << t;)
			output_ = piw::xevent_data_buffer_t(OUT_STRIP_MASK,PIW_DATAQUEUE_SIZE_NORM);
			source_start(0,id_.restamp(t),output_);
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
	last_update_=t;

	LOG_SINGLE(pic::logmsg() << "strip update :update " << t << " val " << v;)
	output_.add_value(OUT_S_STRIP_REL,piw::makefloat_bounded_nb(1,-1,0,rel,t));
	output_.add_value(OUT_S_STRIP_ABS,piw::makefloat_bounded_nb(1,-1,0,v,t));
}

void breath_t::update(unsigned long long t,float v)
{
    bool a = true;

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

	LOG_SINGLE(pic::logmsg() << "breath update :update " << t << " val " << val;)
	output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,val,t));
}

void pedal_t::update(unsigned long long t, float v)
{
	// TODO, not quite sure if pedal is really -1 to 1, code looks like pedal can be configured to be -ve or +ve
	// im assume -1  to 1
    bool a = true;
 	LOG_SINGLE(pic::logmsg() << "pedal update " << t << " a " << a << " value " << v;)

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

	if(cur_val==v)
		return;

	if(t < (last_update_ + device_->updatePeriod()))
		return;

	cur_val=v;
	last_update_=t;
	output_.add_value(1,piw::makefloat_bounded_nb(1,-1,0,v,t));
}


t3d_server_t::t3d_server_t(t3d_listener_t* listener) : port_(0),socket_(NULL),listener_(listener)
{
	listener_=listener;
}

t3d_server_t::~t3d_server_t()
{
	if(socket_) delete socket_;
}

void t3d_server_t::thread_main()
{
	ZeroConfServiceListener listener;
	LOG_SINGLE(pic::logmsg() << "t3d_server_t::thread_main() " << port_;)
	socket_=new UdpListeningReceiveSocket (
            IpEndpointName( IpEndpointName::ANY_ADDRESS, port_),
            listener_);

	ZeroConf::NetService eigenService("local", "_osc._udp", "EigenD", port_);

	eigenService.setListener(&listener);
	eigenService.publish(true);
	socket_->Run();
	eigenService.stop();
}

void t3d_server_t::thread_init()
{
}

void t3d_server_t::thread_term()
{
	LOG_SINGLE(pic::logmsg() << "t3d_server_t::thread_term()";)
	if(socket_) delete socket_;
	socket_=NULL;
}


void t3d_server_t::port(unsigned p)
{
	port_=p;
}

void t3d_server_t::start()
{
	LOG_SINGLE(pic::logmsg() << "t3d_server_t::start()";)
	run();
}

void t3d_server_t::stop()
{
	LOG_SINGLE(pic::logmsg() << "t3d_server_t::stop()";)
	socket_->AsynchronousBreak();
	wait();
	LOG_SINGLE(pic::logmsg() << "t3d_server_t::stopped()";)
}

static const char *SP_FRM_MSG ="/t3d/frm";
static const char *SP_DR_MSG ="/t3d/dr";
static const char *SP_TCH_MSG ="/t3d/tch";
static const int SP_TCH_MSG_LEN=8;

// my own zones
static const char *SP_BREATH_MSG ="/t3d/breath";
static const char *SP_STRIP1_MSG ="/t3d/strip1";
static const char *SP_STRIP2_MSG ="/t3d/strip2";


// OSC Bundle (time)
//    /t3d/frm 
//    /t3d/tch
//    /t3d/tch 
//    /t3d/tch 
//    (...)
// End OSC Bundle

// /t3d/frm (int)frameID (int)deviceID
// /t3d/tch[n] (float)x, (float)y, (float)z, (float)note
// /t3d/matrix OSCBlob [data] 

// /t3d/conn int32
// /t3d/dr int32


// /t3d/dr i 200   data rate
// /t3d/frm ii 17149 291  frame deviceid 
// /t3d/frm ii 17151 291
// /t3d/tch1 ffff 0.021484 0.034180 0.041260 53.000000      tchN R Y Z note
// /t3d/tch1 ffff 0.000000 0.000000 0.000000 0.000000




// osc listner, here we will decode the message, check we dont have data loss, and that we are getting
// data at the required frequency
// we only need to tell the fast thread a couple of things
// a) new touch messages
// b) disconnection ... so that we can terminate any currently active notes. (optional)

// ------------------------------------------------------------------------------
// thing virtual functions
// ------------------------------------------------------------------------------
void t3d_listener_t::thing_dequeue_fast(const piw::data_nb_t &d)
{
    if(d.is_blob())
    {
    	device_->handle_message(d.time(),d);
    }
}


void t3d_listener_t::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint)
{
//	LOG_SINGLE(pic::logmsg() << "t3d_listener_t::ProcessMessage " << m.AddressPattern();)
	osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
	try
	{
		// in order of frequency and priority
		if (std::strcmp( m.AddressPattern(), SP_FRM_MSG ) == 0 )
		{
			osc::int32 frame,deviceId;
			args >> frame >> deviceId  >> osc::EndMessage;
//			LOG_SINGLE(pic::logmsg() << "t3d_listener_t::ProcessMessage frame message f " << frame << " device " << deviceId;)
		}
		else if (std::strncmp( m.AddressPattern(), SP_TCH_MSG,SP_TCH_MSG_LEN ) == 0 )
		{
			int touchId=0;
			float x,y,z,note;
			const char *str=m.AddressPattern() + SP_TCH_MSG_LEN;
			int len=strlen(str);
			for(int i=0;i<len;i++)
			{
				touchId*=10;
				touchId+= *str - '0';
//				LOG_SINGLE(pic::logmsg() <<  "str :" << *str << ":touchId:" << touchId;)
				str++;
			}

			args >> x >> y >> z >> note >> osc::EndMessage;
//			LOG_SINGLE(pic::logmsg() << "t3d_listener_t::ProcessMessage touch touchId " << touchId << " x " << x << " y " << y << " z " << z << " note " << note ;)

			t3d_message_t *blob;
		    unsigned long long t = piw::tsd_time();
		    piw::data_nb_t d = piw::makeblob_nb(t, sizeof(t3d_message_t), (unsigned char **)&blob);
		    blob->type_=T_TOUCH;
		    blob->touchId=touchId;
		    blob->note=note;
		    blob->x=x;
		    blob->y=y;
		    blob->z=z;
		    enqueue_fast(d);
		}
		else if (std::strcmp( m.AddressPattern(), SP_BREATH_MSG ) == 0 )
		{
			float x;
			args >> x >> osc::EndMessage;
//			LOG_SINGLE(pic::logmsg() << "t3d_listener_t::ProcessMessage breath  x " << x;)
			t3d_message_t *blob;
		    unsigned long long t = piw::tsd_time();
		    piw::data_nb_t d = piw::makeblob_nb(t, sizeof(t3d_message_t), (unsigned char **)&blob);
		    blob->type_=T_BREATH;
		    blob->x=x;
		    enqueue_fast(d);
		}
		else if (std::strcmp( m.AddressPattern(), SP_STRIP1_MSG ) == 0 )
		{
			float x;
			args >> x >> osc::EndMessage;
//			LOG_SINGLE(pic::logmsg() << "t3d_listener_t::ProcessMessage strip1  x " << x;)
			t3d_message_t *blob;
		    unsigned long long t = piw::tsd_time();
		    piw::data_nb_t d = piw::makeblob_nb(t, sizeof(t3d_message_t), (unsigned char **)&blob);
		    blob->type_=T_STRIP1;
		    blob->x=x;
		    enqueue_fast(d);
		}
		else if (std::strcmp( m.AddressPattern(), SP_STRIP2_MSG ) == 0 )
		{
			float x;
			args >> x >> osc::EndMessage;
//			LOG_SINGLE(pic::logmsg() << "t3d_listener_t::ProcessMessage strip2  x " << x;)
			t3d_message_t *blob;
		    unsigned long long t = piw::tsd_time();
		    piw::data_nb_t d = piw::makeblob_nb(t, sizeof(t3d_message_t), (unsigned char **)&blob);
		    blob->type_=T_STRIP2;
		    blob->x=x;
		    enqueue_fast(d);
		}

		else if( std::strcmp( m.AddressPattern(), SP_DR_MSG ) == 0 )
		{
			osc::int32 dr;
			args >> dr >> osc::EndMessage;
//			LOG_SINGLE(pic::logmsg() << "t3d_listener_t::ProcessMessage data rata " << dr;)
		}
	}
	catch( osc::Exception& e )
	{
		pic::logmsg() << "t3d_listener_t::ProcessMessage :: oscpack error while parsing message: "
			<< m.AddressPattern() << ": " << e.what() << "\n";
	}
}
/////////////////////////////////////////////////////////////


// light wire
light_wire_t::light_wire_t(t3d_device_t::impl_t  *p, const piw::event_data_source_t &es):
    piw::event_data_source_real_t(es.path()),
    root_(p),
    last_from_(0),
	light_t_(0)
{
    root_->children_.insert(std::make_pair(path(),this));

    root_->connect_wire(this, source());

    subscribe_and_ping(es);
}

static int __light_wire_invalidator(void *w_, void *_)
{
	light_wire_t *w = (light_wire_t *)w_;
    if(w->root_)
    {
        w->root_->del_ticker(w);
    }
    return 0;
}

void light_wire_t::invalidate()
{
    source_shutdown();

    unsubscribe();

    piw::tsd_fastcall(__light_wire_invalidator, this, 0);

    if(root_)
    {
        root_->children_.erase(path());
        root_ = 0;
    }
}

void light_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    output_ = piw::xevent_data_buffer_t(IN_MASK,PIW_DATAQUEUE_SIZE_TINY);

    input_ = b.iterator();

    unsigned long long t = id.time();
    last_from_ = t;

	piw::data_nb_t d;
	if(input_->latest(IN_LIGHT,d,t))
	{
		root_->enqueue_slow_nb(d);
	}

    source_start(seq,id,output_);

    root_->add_ticker(this);
}

void light_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    input_->set_signal(s,n);
    input_->reset(s,t);
    ticked(last_from_,t);
}

bool light_wire_t::event_end(unsigned long long t)
{
    ticked(last_from_,t);
    root_->del_ticker(this);
    return source_end(t);
}

void light_wire_t::ticked(unsigned long long f, unsigned long long t)
{
    last_from_ = t;

	piw::data_nb_t d;
	unsigned s;
	while(input_->next(SIG1(IN_LIGHT),s,d,t))
	{
		root_->enqueue_slow_nb(d);
	}
}

void light_wire_t::source_ended(unsigned seq)
{
    event_ended(seq);
}


}; //namespace t3d_device_plg


