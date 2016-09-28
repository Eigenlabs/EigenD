#include "t3d_output.h"
#include <piw/piw_keys.h>
#include <picross/pic_float.h>
#include <picross/pic_stdint.h>

#include <lib_lo/lo/lo.h>
#include <lib_op/osc/OscReceivedElements.h>
#include <lib_op/osc/OscPacketListener.h>
#include <lib_op/ip/UdpSocket.h>

#include <map>

#include "Madrona/SoundplaneOSCOutput.h"

// turn logging by setting LOG_SINGLE(x) x
//#define  LOG_SINGLE(x) x
#define  LOG_SINGLE(x)


#ifndef UINT64_MAX
#define UINT64_MAX  (18446744073709551615ULL)
#endif

#define IN_KEY 1
#define IN_PRESSURE 2
#define IN_ROLL 3
#define IN_YAW 4
#define IN_FREQ 5

#define IN_CONTROL 1

#define IN_MASK SIG5(IN_KEY,IN_PRESSURE,IN_ROLL,IN_YAW,IN_FREQ)


// hardcode in t3d to be once a second
#define INFREQUENT_TIME 1000*1000

namespace
{

    struct t3d_wire_t;

    struct t3d_connection_t: piw::root_t
    {
        t3d_connection_t(t3d_output_plg::t3d_output_t::impl_t *server, const std::string &prefix, bool key_, unsigned ident);
        ~t3d_connection_t();
        piw::wire_t *root_wire(const piw::event_data_source_t &);

        void root_opened();
        void root_closed();
        void root_clock();

        void root_latency();

        piw::cookie_t cookie() { return piw::cookie_t(this); }

        t3d_output_plg::t3d_output_t::impl_t *server_;

        std::string prefix_;
        bool key_;
        unsigned ident_;
        bct_clocksink_t *upstream_;

        std::vector<t3d_wire_t *> wires_;
    };

    struct t3d_wire_t:
        piw::wire_t,
        piw::event_data_sink_t,
        pic::element_t<>,
        virtual pic::lckobject_t
    {
        t3d_wire_t(t3d_connection_t *output, unsigned index, const piw::event_data_source_t &es);
        ~t3d_wire_t();

        int voiceId() { return voiceId_;}
        void voiceId(int v) { voiceId_=v;}
        unsigned long long voiceTime() { return voiceTime_;}
        void voiceTime(unsigned long long t) { voiceTime_=t;}
        bool valid() { return voiceId_ >=0;}

        void wire_closed();

        void ticked(unsigned long long from, unsigned long long to);
        void event_start(unsigned seq, const piw::data_nb_t &id,const piw::xevent_data_buffer_t &b);
        bool event_end(unsigned long long);
        void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);
        void send_latest(unsigned long long t,bool state);

        void send(unsigned long long time, unsigned signal,const piw::data_nb_t& d, bool isStart);
        void set_x(const piw::data_nb_t& d);
        void set_y(const piw::data_nb_t& d);
        void set_z(const piw::data_nb_t& d);
        void set_note(const piw::data_nb_t& d);
        void set_key(const piw::data_nb_t& d);

        SoundplaneOSCOutput * t3d();

        t3d_connection_t *output_;

        unsigned index_;

        piw::xevent_data_buffer_t::iter_t iterator_;

        piw::dataholder_nb_t id_string_;

        unsigned long long last_processed_;
        int voiceId_;
        unsigned long long voiceTime_;
        float absx_, absy_,x_,y_,z_,note_,column_,row_;
    };
};




class kyma_listener_t: public osc::OscPacketListener
{
	virtual void ProcessMessage( const osc::ReceivedMessage& m,  const IpEndpointName& remoteEndpoint );
};

class kyma_server_t: public pic::thread_t
{
public:
	kyma_server_t();
	virtual ~kyma_server_t();

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
	kyma_listener_t listener_;
};


struct t3d_output_plg::t3d_output_t::impl_t:
    piw::clocksink_t
{
    impl_t(piw::clockdomain_ctl_t *d, const std::string &a, unsigned p);
    ~impl_t();

    void stop();

    void clocksink_ticked(unsigned long long f, unsigned long long t);

    piw::cookie_t create_output(const std::string &prefix, bool key, unsigned signals);
    void control_change(const piw::data_nb_t &d);

    void deactivate_wire_slow(t3d_wire_t *w);
    void activate_wire_fast(t3d_wire_t *w);
    void deactivate_wire_fast(t3d_wire_t *w);
    bool connect(const std::string &a, unsigned p,unsigned long long t);
    void kyma(bool);

    kyma_server_t kyma_server_;
    std::map<std::string,t3d_connection_t *> outputs_;
    SoundplaneOSCOutput * t3d_;
    pic::ilist_t<t3d_wire_t> active_wires_;
    std::vector<t3d_wire_t *> voices_;
	std::string host_;
	int port_;
	unsigned long long int last_connect_time_;
	unsigned long long int last_tick_;
	unsigned long long int last_infreq_;
    pic::lckvector_t<unsigned>::nbtype columnlen_;
    unsigned num_columns_;
    unsigned num_rows_;
    bool continuous_;
};


kyma_server_t::kyma_server_t() : port_(0),socket_(NULL)
{
}

kyma_server_t::~kyma_server_t()
{
	if(socket_) delete socket_;
}

void kyma_server_t::thread_main()
{
//	pic::logmsg() << "kyma server main()";
	socket_=new UdpListeningReceiveSocket (
            IpEndpointName( IpEndpointName::ANY_ADDRESS, port_),
            &listener_);
	socket_->Run();
}
void kyma_server_t::thread_init()
{
}

void kyma_server_t::thread_term()
{
//	pic::logmsg() << "kyma server terminated";
	if(socket_) delete socket_;
//	socket_=NULL;
}


void kyma_server_t::port(unsigned p)
{
	port_=p;
}

void kyma_server_t::start()
{
//	pic::logmsg() << "kyma server start()";
	run();
}

void kyma_server_t::stop()
{
//	pic::logmsg() << "kyma server stop()";
	socket_->AsynchronousBreak();
	wait();
//	pic::logmsg() << "kyma server stopped";
}


void kyma_listener_t::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint)
{
	pic::logmsg() << "received kyma msg: " << m.AddressPattern();
	osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
	osc::int32 a1;
	try
	{
		if( std::strcmp( m.AddressPattern(), "/osc/response_from" ) == 0 )
		{
			args >> a1 >> osc::EndMessage;
			// set Kyma mode
			pic::logmsg() << "kyma response " << a1;
//			if (mOSCOutput.getKymaMode())
//			{
//				mKymaIsConnected = true;
//			}
		}
		else if (std::strcmp( m.AddressPattern(), "/osc/notify/midi/Soundplane" ) == 0 )
		{
			args >> a1 >> osc::EndMessage;
			// set voice count to a1
			pic::logmsg() << "max touches to (ignoring!) " << a1;
//			int newTouches = clamp((int)a1, 0, kSoundplaneMaxTouches);
//			if(mKymaIsConnected)
//			{
//				// Kyma is sending 0 sometimes, which there is probably
//				// no reason to respond to
//				if(newTouches > 0)
//				{
//					setProperty("max_touches", newTouches);
//				}
//			}
		}
	}
	catch( osc::Exception& e )
	{
		pic::logmsg() << "oscpack error while parsing message: "
			<< m.AddressPattern() << ": " << e.what() << "\n";
	}
}





#define SDM_T_VOICE 0
#define SDM_T_X 1
#define SDM_T_Y 2
#define SDM_T_Z 3
#define SDM_T_DZ 4
#define SDM_T_NOTE 5

#define SDM_C_ZONEID 0
#define SDM_C_X 5
#define SDM_C_Y 6
#define SDM_C_Z 7



bool sendT3DMessage( SoundplaneOSCOutput *pOut,MLSymbol type)
{
	if(! pOut->isActive()) return false;

	SoundplaneDataMessage msg;
	msg.mType = type;
	msg.mZoneName=nullZone.c_str();

	pOut->processMessage(&msg);
	return true;
}



bool sendT3DTouchMessage( SoundplaneOSCOutput *pOut,
							MLSymbol type, MLSymbol subtype,
							int voice,
							float note, float x, float y, float z)
{
	if(! pOut->isActive()) return false;

	SoundplaneDataMessage msg;
	msg.mType = type;
	msg.mSubtype = subtype;
	msg.mData[SDM_T_VOICE] = voice;
	msg.mData[SDM_T_NOTE] = note;
	msg.mData[SDM_T_X] = x;
	msg.mData[SDM_T_Y] = y;
	msg.mData[SDM_T_Z] = z;
	msg.mData[SDM_T_DZ] = z;
	msg.mZoneName=nullZone.c_str();

//	pic::logmsg() << "sendT3DTouchMessage"
//					<< " type:" << type
//					<< " subtype:" << subtype
//					<< " voice:" << voice
//					<< " x:" << x
//					<< " y:" << y
//					<< " z:" << z
//					<< " note:" << note;

	pOut->processMessage(&msg);
	return true;
}

bool sendT3DControlMessage( SoundplaneOSCOutput *pOut,
							MLSymbol type, MLSymbol subtype,int zoneId, float x, float y, float z, const char* zone)
{
	if(! pOut->isActive()) return false;

//	pic::logmsg() << "sendT3DControlMessage"
//					<< " type:" << type
//					<< " zoneId:" << zoneId
//					<< " subtype:" << subtype
//					<< " x:" << x
//					<< " y:" << y
//					<< " z:" << z
//					<< " zone:" << zone;


	SoundplaneDataMessage msg;
	msg.mType = type;
	msg.mSubtype = subtype;
	msg.mData[SDM_C_ZONEID]= zoneId;
	msg.mData[SDM_C_X] = x;
	msg.mData[SDM_C_Y] = y;
	msg.mData[SDM_C_Z] = z;
	msg.mZoneName=zone;


	pOut->processMessage(&msg);
	return true;
}



t3d_wire_t::t3d_wire_t(t3d_connection_t *output, unsigned index, const piw::event_data_source_t &es):
		output_(output), index_(index), last_processed_(0), voiceId_(-1), voiceTime_(0),
		absx_(0.0), absy_(0.0),x_(0.0),y_(0.0),z_(0.0),note_(0.0)
{
    output_->wires_[index_] = this;

    subscribe_and_ping(es);
}

t3d_wire_t::~t3d_wire_t()
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

void t3d_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    output_->server_->activate_wire_fast(this);
    iterator_ = b.iterator();

    pic::msg_t msg;
    msg << id;
    id_string_.set_nb(piw::makestring_nb(msg.str().c_str(),id.time()));

    last_processed_=id.time();
    absx_=absy_=x_=y_=z_=note_=0.0;
    if(voiceId() > 0)
    {
        ::sendT3DMessage(t3d(),MLS_startFrameSym);
        send_latest(last_processed_,true);
        ::sendT3DMessage(t3d(),MLS_endFrameSym);
    }
    iterator_->reset_all(last_processed_+1);
}

bool t3d_wire_t::event_end(unsigned long long t)
{
	// turn off, if the voice has not already been deallocated
    if(valid())
    {
        ::sendT3DMessage(t3d(),MLS_startFrameSym);
        send_latest(t,false);
        if(output_->key_)
        {
            ::sendT3DTouchMessage(t3d(),MLS_touchSym,MLS_offSym,voiceId_,0,0.0f,0.0f,0.0f);
        }
        ::sendT3DMessage(t3d(),MLS_endFrameSym);
    }

    id_string_.clear_nb();
    iterator_.clear();
    output_->server_->deactivate_wire_fast(this);

    return true;
}


void t3d_wire_t::event_buffer_reset(unsigned s,unsigned long long t, const piw::dataqueue_t &oq,const piw::dataqueue_t &nq)
{
    iterator_->set_signal(s,nq);
    iterator_->reset(s,t);
    if(valid())
    {
        ::sendT3DMessage(t3d(),MLS_startFrameSym);
        send_latest(t,false);
        ::sendT3DMessage(t3d(),MLS_endFrameSym);
    }
}


void t3d_wire_t::ticked(unsigned long long from, unsigned long long to)
{
    piw::data_nb_t d;
    unsigned s=1;

    if(!valid())
	{
		// voice has been deallocated clear all incoming events
		iterator_->reset_all(to);
	    absx_=absy_=x_=y_=z_=note_=0.0;
	}
    else
    {
        while(iterator_->next(IN_MASK,s,d,to))
        {
            send(to,s,d,false);
        }
    }
    last_processed_ = to;
}

SoundplaneOSCOutput * t3d_wire_t::t3d()
{
	return output_->server_->t3d_;
}

void t3d_wire_t::set_key(const piw::data_nb_t& d)
{
	if(output_->server_->continuous_)
	{
		float course, key;
		piw::hardness_t hardness;
		if(piw::decode_key(d,&column_,&row_,&course,&key,&hardness))
		{
			absx_= (row_ - 0.5 + (x_ * 0.5)) / output_->server_->num_rows_ ;
			absy_= (column_ - 0.5 + (y_ * 0.5)) / output_->server_->num_columns_ ;
		}
	}
}

void t3d_wire_t::set_y(const piw::data_nb_t& d)
{
	y_ =  d.as_denorm_float();
	if(output_->server_->continuous_)
	{
		absy_= (column_ - 0.5 + (y_ * 0.5)) / output_->server_->num_columns_ ;
	}
	else
	{
		absy_= (y_+1.0)/2.0;
	}
}

void t3d_wire_t::set_x(const piw::data_nb_t& d)
{
	x_ = d.as_denorm_float();
	if(output_->server_->continuous_)
	{
		absx_= (row_ - 0.5 + (x_ * 0.5)) / output_->server_->num_rows_ ;
	}
	else
	{
		absx_= (x_+1.0)/2.0;
	}
}
void t3d_wire_t::set_z(const piw::data_nb_t& d)
{
	z_ = d.as_denorm_float();
}

void t3d_wire_t::set_note(const piw::data_nb_t& d)
{
	float freq = d.as_denorm_float();
	note_ = 12.0f * pic_log2(freq/440.0f) + 69.0f;
  	LOG_SINGLE(pic::logmsg() << "t3d_wire_t::set_note freq " << freq << " note" << note_;)
}

void t3d_wire_t::send_latest(unsigned long long t,bool state)
{
	if(output_->key_)
	{
		if(valid())
		{
			piw::data_nb_t d;
			if(iterator_->latest(IN_KEY,d,t)) set_key(d);
			if(iterator_->latest(IN_ROLL,d,t)) set_x(d);
			if(iterator_->latest(IN_YAW,d,t)) set_y(d);
			if(iterator_->latest(IN_PRESSURE,d,t)) set_z(d);
			if(iterator_->latest(IN_FREQ,d,t))
			{
				send(t,IN_FREQ,d,state);
			}
		}
	}
	else
	{
		MLSymbol type =MLS_nullSym, subtype = MLS_nullSym;
		type = MLS_controllerSym;
		subtype = MLS_ySym;
		float x=0.0f,y=0.0f,z=0.0f;
		int zoneId=output_->ident_;
		const char* zone =  output_->prefix_.c_str();
		::sendT3DControlMessage(t3d(),type,subtype,zoneId,x,y,z,zone);
	}
}


void t3d_wire_t::send(unsigned long long t,unsigned s, const piw::data_nb_t& d, bool isStart)
{
	MLSymbol type =MLS_nullSym, subtype = MLS_nullSym;
    if(output_->key_)
    {
    	switch(s){
    		case IN_KEY:
			{
				set_key(d);
			}
			break;
    		case IN_PRESSURE:
			{
	//	    	pic::logmsg() << "z" << d;
				set_z(d);
			}
			break;
    		case IN_ROLL:
			{
	//	    	pic::logmsg() << "x" << d;
				set_x(d);
			}
			break;
    		case IN_YAW:
			{
	//	    	pic::logmsg() << "y" << d;
				set_y(d);
			}
			break;
    		case IN_FREQ:
			{
				set_note(d);
			}
			break;
    	}

    	LOG_SINGLE(pic::logmsg() << "t3d_wire_t::send nr:" << output_->server_->num_rows_ << " nc " << output_->server_->num_columns_ << " r " << row_ << " c " << column_ <<  " x:" << x_ << " y:" << y_ << " ax " << absx_ << " ay " << absy_;)

    	int voice=voiceId_;
		type=MLS_touchSym;
		if(isStart)
		{
			subtype = MLS_onSym;
		}
		else
		{
			subtype = MLS_continueSym;
		}
		::sendT3DTouchMessage(t3d(),type,subtype,voice,note_,absx_,absy_,z_);
    }
	else
	{
		if(s==1)
		{
			type = MLS_controllerSym;
			subtype = MLS_ySym;
			float x=0.0f,y=0.0f,z=0.0f;
			int zoneId=output_->ident_;
			const char* zone =  output_->prefix_.c_str();
			y = d.as_denorm_float();
			::sendT3DControlMessage(t3d(),type,subtype,zoneId,x,y,z,zone);
		}
	}
}



void t3d_wire_t::wire_closed()
{
    delete this;
}

t3d_connection_t::t3d_connection_t(t3d_output_plg::t3d_output_t::impl_t *server, const std::string &prefix, bool key, unsigned ident):
  piw::root_t(0), server_(server), prefix_(prefix), key_(key), ident_(ident), upstream_(0)
{
    // add ourself to the list of outputs in the server.
    server_->outputs_.insert(std::make_pair(prefix_,this));
}

t3d_connection_t::~t3d_connection_t()
{
    tracked_invalidate();
    disconnect();

    server_->outputs_.erase(prefix_);

    for(unsigned i=0;i<wires_.size();i++)
    {
        if(wires_[i])
        {
            delete wires_[i];
        }
    }
}

piw::wire_t *t3d_connection_t::root_wire(const piw::event_data_source_t &es)
{
    unsigned i = 0;
    for(i=0;i<wires_.size();i++)
    {
        if(!wires_[i])
        {
            goto found_slot;
        }
    }
    wires_.resize(i+1);
    wires_[i] = 0;

found_slot:
    return new t3d_wire_t(this,i,es);
}

void t3d_connection_t::root_closed()
{
    delete this;
}


void t3d_connection_t::root_clock()
{
    bct_clocksink_t *c = get_clock();

    if(c!=upstream_)
    {
        if(upstream_)
        {
            server_->remove_upstream(upstream_);
        }

        upstream_=c;

        if(upstream_)
        {
            server_->add_upstream(upstream_);
        }
    }
}


void t3d_connection_t::root_opened()
{
    root_clock();
}

void t3d_connection_t::root_latency()
{
}

t3d_output_plg::t3d_output_t::impl_t::impl_t(piw::clockdomain_ctl_t *d, const std::string &a, unsigned p) :
		num_columns_(0),num_rows_(0),continuous_(false)
{
	t3d_ = new SoundplaneOSCOutput();

	t3d_->setDataFreq(250.0f);
	t3d_->setKymaMode(false);
	t3d_->setSerialNumber(0x123);
	t3d_->setMaxTouches(kSoundplaneMaxTouches);

	t3d_->initialize();
	t3d_->setActive(false);
	last_connect_time_=0;
//	connect(a,p,piw::tsd_time());
	last_tick_=piw::tsd_time();
	last_infreq_=piw::tsd_time();

    d->sink(this,"t3d output");

    tick_enable(false); // dont suppress ticks
}


t3d_output_plg::t3d_output_t::impl_t::~impl_t()
{
	LOG_SINGLE(pic::logmsg() << "t3d_output_plg::t3d_output_t::impl_t::~impl_t start";)

	delete t3d_;

    tick_disable();
    close_sink();
    std::map<std::string,t3d_connection_t *>::iterator i;

    while((i=outputs_.begin()) != outputs_.end())
    {
        delete i->second;
    }
	LOG_SINGLE(pic::logmsg() << "t3d_output_plg::t3d_output_t::impl_t::~impl_t done";)
}

void t3d_output_plg::t3d_output_t::impl_t::stop()
{
	LOG_SINGLE(pic::logmsg() << "t3d_output_plg::t3d_output_t::impl_t::stop";)
	kyma(false);
	if(t3d_->isActive())
	{
		t3d_->notify(false);
		t3d_->setActive(false);
	}
	delete t3d_;  // no close, so we have to delete it

	// add a dummy until desctuctor time
	t3d_ = new SoundplaneOSCOutput();
	t3d_->initialize();
	t3d_->setActive(false);
	port_=0; // put port to zero, to say dont try to connect it!
}



bool t3d_output_plg::t3d_output_t::impl_t::connect(const std::string &a, unsigned p,unsigned long long t)
{
	LOG_SINGLE(pic::logmsg() << "t3d connecting :" << host_ << ":" << port_;)
	if(p==0) return false;

	host_=a;
	port_=p;
	last_connect_time_=t;

	if(t3d_->connect(host_.c_str(),port_))
	{
		t3d_->setActive(true);
		LOG_SINGLE(pic::logmsg() << "t3d connected :" << host_ << ":" << port_;)
		t3d_->notify(true);
		return true;
	}
	t3d_->setActive(false);
	LOG_SINGLE(pic::logmsg() << "t3d unable to connect :" << host_ << ":" << port_;)
	return false;
}


void t3d_output_plg::t3d_output_t::impl_t::kyma(bool a)
{
	bool wasActive=t3d_->getKymaMode();
	if(a==wasActive) return;
	t3d_->setKymaMode(a);
	if(wasActive)
	{
		LOG_SINGLE(pic::logmsg() << "stopping kyma reciever";)
		kyma_server_.stop();
	}
	else
	{
		LOG_SINGLE(pic::logmsg() << "starting kyma reciever";)
		kyma_server_.port(3124);
		kyma_server_.start();
	}
}




piw::cookie_t t3d_output_plg::t3d_output_t::impl_t::create_output(const std::string &prefix, bool key, unsigned ident)
{
    std::map<std::string,t3d_connection_t *>::iterator i;

    // check for a duplicate output.
    // return a null cookie in that case (an event bucket)
    if((i=outputs_.find(prefix))!=outputs_.end())
    {
        return piw::cookie_t();
    }

    t3d_connection_t *o = new t3d_connection_t(this,prefix,key,ident);
    return o->cookie();
}


static int remover__(void *impl__, void *wire__)
{
    t3d_output_plg::t3d_output_t::impl_t *impl = (t3d_output_plg::t3d_output_t::impl_t *)impl__;
    t3d_wire_t *wire = (t3d_wire_t *)wire__;
    impl->deactivate_wire_fast(wire);
//    impl->active_wires_.remove(wire);
    return 0;
}

void t3d_output_plg::t3d_output_t::impl_t::deactivate_wire_slow(t3d_wire_t *w)
{
    piw::tsd_fastcall(remover__,this,w);
}

void t3d_output_plg::t3d_output_t::impl_t::activate_wire_fast(t3d_wire_t *w)
{
    unsigned i = 0;
    unsigned oldestVoice = 0;
    unsigned long long oldestTime = UINT64_MAX;

    // dont need voice allocation on non-keys
    if(w->output_->key_ == false)
    {
    	w->voiceId(1000);
    	w->voiceTime(0);
        active_wires_.append(w);
        return;
    }


    //find an empty slot, up to maxium... increase vector if needed, or reallocate voices if necessary
    for(i=0;i<voices_.size() && i < (unsigned) t3d_->getMaxTouches() ;i++)
    {
        if(!voices_[i])
        {
            goto found_slot;
        }
        if (voices_[i]->voiceTime() < oldestTime)
        {
        	oldestVoice=i;
        	oldestTime = voices_[i]->voiceTime();
        }
    }
    if(i < (unsigned) t3d_->getMaxTouches() )
    {
    	voices_.resize(i+1);
    	voices_[i] = 0;
    }
    else
    {
    	// hit max voice so reallocate
    	i=oldestVoice;
	    sendT3DMessage(t3d_,MLS_startFrameSym);
        sendT3DTouchMessage(t3d_,MLS_touchSym,MLS_offSym,i,0.0f,0.0f,0.0f,0.0f);
	    sendT3DMessage(t3d_,MLS_endFrameSym);
		voices_[i]->voiceId(-1);
    }

found_slot:
    voices_[i] = w;
    unsigned long long t=pic_microtime();
    w->voiceTime(t);
	w->voiceId(i);
//	pic::logmsg() << "allocate voice:" << i;
    active_wires_.append(w);
}

void t3d_output_plg::t3d_output_t::impl_t::deactivate_wire_fast(t3d_wire_t *w)
{
    if(w->output_->key_ == false)
    {
    	w->voiceId(-1);
    	w->voiceTime(0);
        active_wires_.remove(w);
    	return;
    }


	int v=w->voiceId();
	voices_[v]=0;
	w->voiceId(-1);
	w->voiceTime(0);
//	pic::logmsg() << "deallocate voice:" << v;
    active_wires_.remove(w);
}


void t3d_output_plg::t3d_output_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
	static const long retryTime = 15*1000*1000; // try to reconnect every 15 seconds
	if (!t3d_->isActive())
	{
		if(t > last_connect_time_+retryTime)
		{
			last_connect_time_=t;
			if(!connect(host_.c_str(),port_,t))
			{
				return;
			}
		}
		else
		{
			return;
		}
	}


	sendT3DMessage(t3d_, MLS_startFrameSym);

	t3d_wire_t *w;
	for(w=active_wires_.head(); w!=0; w=active_wires_.next(w))
	{
		w->ticked(f,t);
	}
	sendT3DMessage(t3d_, MLS_endFrameSym);

	if (t > (last_infreq_ + (INFREQUENT_TIME) ))
	{
		t3d_->doInfrequentTasks();
		last_infreq_ = t;
	}

	last_tick_=t;
}

unsigned decode_unsigned(pic::lckvector_t<unsigned>::nbtype &vec, const piw::data_nb_t &inp)
{
    vec.clear();

	LOG_SINGLE(pic::logmsg() << "t3d decode_unsigned " << inp;)
	unsigned max = 0;

	unsigned dictlen = inp.as_tuplelen();
    if(0 == dictlen)
        return max;

    vec.reserve(5);

    for(unsigned i = 0; i < dictlen; ++i)
    {
    	unsigned l=inp.as_tuple_value(i).as_long();
        vec.push_back(l);
        if(l>max) max=l;
    }

//    unsigned c=0;
//    pic::lckvector_t<unsigned>::nbtype::const_iterator ci,ce;
//    ci = vec.begin();
//    ce = vec.end();
//
//    for(; ci != ce; ++ci)
//    {
//    	pic::logmsg() << "decode_unsigned c " << c++ << "v= " << *ci;
//    }
    return max;
}


void t3d_output_plg::t3d_output_t::impl_t::control_change(const piw::data_nb_t &d)
{
	if(d.is_null())
	{
		pic::logmsg() << "t3d_output_t::impl_t::control_change d=null!";
	}
	else
	{
		if(d.is_dict())
		{
			LOG_SINGLE(pic::logmsg() << "t3d_output_t::impl_t::control_change " << d;)
		}
		else
			pic::logmsg() << "t3d_output_t::impl_t::control_change (not dict)" << d;
	}

    if(!d.is_null() && d.is_dict())
    {
		piw::data_nb_t columns;

		columns = d.as_dict_lookup("columnlen");
		if(!columns.is_null())
		{
			LOG_SINGLE(pic::logmsg() << "t3d_output_t::impl_t::control_change columnlen " << columns;)
			num_rows_=decode_unsigned(columnlen_,columns);
			num_columns_=columnlen_.size();

			LOG_SINGLE(pic::logmsg() << "t3d_output_t::impl_t::control_change nr " << num_rows_ << " nc " << num_columns_;)
		}
//		columnsoffset = d.as_dict_lookup("columnsoffset");
//		if(!columns.is_null())
//		{
//			pic::logmsg() << "columnsoffset " << columns;
//			decode_unsigned(columnsoffset_,columnsoffset);
//		}
    }
}



////////////////t3d output  outer class methods




/*
 * Static methods that can be called from the fast thread.
 */

static int __set_data_freq(void *i_, void *d_)
{
    t3d_output_plg::t3d_output_t::impl_t *i = (t3d_output_plg::t3d_output_t::impl_t *)i_;
    unsigned d = *(unsigned *)d_;
    i->t3d_->setDataFreq(d);
    return 0;
}

static int __set_max_voice_count(void *i_, void *d_)
{
    t3d_output_plg::t3d_output_t::impl_t *i = (t3d_output_plg::t3d_output_t::impl_t *)i_;
    unsigned d = *(unsigned *)d_;
    i->t3d_->setMaxTouches(d);
    return 0;
}

static int __set_kyma_mode(void *i_, void *d_)
{
    t3d_output_plg::t3d_output_t::impl_t *i = (t3d_output_plg::t3d_output_t::impl_t *)i_;
    bool d = *(bool *)d_;
    i->kyma(d);
    return 0;
}


static int __set_continuous_mode(void *i_, void *d_)
{
    t3d_output_plg::t3d_output_t::impl_t *i = (t3d_output_plg::t3d_output_t::impl_t *)i_;
    bool d = *(bool *)d_;
    i->continuous_=d;
    return 0;
}

t3d_output_plg::t3d_output_t::t3d_output_t(piw::clockdomain_ctl_t *d, const std::string &a, unsigned p): impl_(new impl_t(d,a,p))
{
}

t3d_output_plg::t3d_output_t::~t3d_output_t()
{
    delete impl_;
}

piw::cookie_t t3d_output_plg::t3d_output_t::create_output(const std::string &prefix,bool key,unsigned ident)
{
    return impl_->create_output(prefix,key,ident);
}

void t3d_output_plg::t3d_output_t::set_max_voice_count(unsigned max_voice_count)
{
    piw::tsd_fastcall(__set_max_voice_count,impl_,&max_voice_count);
}

void t3d_output_plg::t3d_output_t::set_data_freq(unsigned dataFreq)
{
    piw::tsd_fastcall(__set_data_freq,impl_,&dataFreq);
}


void t3d_output_plg::t3d_output_t::set_kyma_mode(bool kyma_mode)
{
    piw::tsd_fastcall(__set_kyma_mode,impl_,&kyma_mode);
}

void t3d_output_plg::t3d_output_t::set_continuous_mode(bool mode)
{
    LOG_SINGLE(pic::logmsg() << "t3d_output_plg::t3d_output_t::set_continuous_mode( " << mode; )
    piw::tsd_fastcall(__set_continuous_mode,impl_,&mode);
}

struct connect_t
{
     std::string host;
     unsigned port;
};

static int __connect(void *i_, void *d_)
{
    t3d_output_plg::t3d_output_t::impl_t *i = (t3d_output_plg::t3d_output_t::impl_t *)i_;
    connect_t *d=(connect_t*) d_;
    LOG_SINGLE(pic::logmsg() << "__connect " << d->host << " port " << d->port; )
    i->connect(d->host,d->port,piw::tsd_time());
    return 0;
}

void t3d_output_plg::t3d_output_t::connect(const std::string &a, unsigned p)
{
     LOG_SINGLE(pic::logmsg() << "t3d_output_plg::t3d_output_t::connect " << a << " port " << p; )
     connect_t d;
     d.host=a;
     d.port=p;
     piw::tsd_fastcall(__connect,impl_,&d);
}

piw::change_nb_t t3d_output_plg::t3d_output_t::control()
{
    return piw::change_nb_t::method(impl_,&t3d_output_plg::t3d_output_t::impl_t::control_change);
}

static int stopper__(void *i_, void *)
{
	t3d_output_plg::t3d_output_t::impl_t *i = (t3d_output_plg::t3d_output_t::impl_t *)i_;
	i->stop();
    return 0;
}

void t3d_output_plg::t3d_output_t::stop()
{
	piw::tsd_fastcall(stopper__,impl_,0);
}

