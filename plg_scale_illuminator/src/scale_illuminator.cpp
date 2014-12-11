
#include "scale_illuminator.h"
#include <piw/piw_cfilter.h>
#include <piw/piw_data.h>
#include <piw/piw_status.h>

#include <picross/pic_ref.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_keys.h>
#include <picross/pic_ref.h>


#define OUT_LIGHT 1
#define OUT_MASK SIG1(OUT_LIGHT)

#define IN_CONTROL 1

// SET LOG_SINGLE to x and LOG_MULTI to 1 for logging
#define LOG_SINGLE(x) 
#define LOG_MULTI 0

namespace scale_illuminator_plg
{

//////////// scale_illuminator_t::impl_t  declaration
// illuminator wire used for input processing
struct control_wire_t:
    piw::wire_t,
    piw::wire_ctl_t,
    piw::event_data_sink_t,
    piw::event_data_source_real_t,
    virtual public pic::lckobject_t,
    pic::element_t<>
{
    control_wire_t(scale_illuminator_t::impl_t *p, const piw::event_data_source_t &);
    ~control_wire_t() { invalidate(); }

    void wire_closed() { delete this; }
    void invalidate();
    void event_start(unsigned,const piw::data_nb_t &, const piw::xevent_data_buffer_t &);
    bool event_end(unsigned long long);
    void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);
    void process(unsigned, const piw::data_nb_t &, unsigned long long);
    void ticked(unsigned long long f, unsigned long long t);
    void source_ended(unsigned);

    scale_illuminator_t::impl_t *root_;
    piw::xevent_data_buffer_t::iter_t input_;
    piw::xevent_data_buffer_t output_;

    unsigned long long last_from_;
};

// light wire used to power the lights ;o) 
struct light_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::counted_t
{
	light_wire_t(scale_illuminator_t::impl_t *impl);
	~light_wire_t();
	void source_ended(unsigned seq);
	void updateLights();

	scale_illuminator_t::impl_t *root_;
	piw::xevent_data_buffer_t output_;
};



//////////// scale_illuminator_t::impl_t  declaration
// this is based on latch, please look there for general comments on eigenD usage
struct scale_illuminator_t::impl_t :
	    piw::root_t,
	    piw::root_ctl_t,
	    piw::clocksink_t,
	    virtual pic::lckobject_t
{
    impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &oc);
    ~impl_t();
     void clocksink_ticked(unsigned long long f, unsigned long long t);
    void add_ticker(control_wire_t *w);
    void del_ticker(control_wire_t *w);
    void invalidate();
    piw::wire_t *root_wire(const piw::event_data_source_t &es);
    void root_closed();
    void root_opened();
    void root_clock();
    void root_latency();


    void control_change(const piw::data_nb_t &d);
    void reference_scale(const std::string &);
    void reference_tonic(float);
    void root_light(bool v);
    void inverted (bool v);


    pic::lckmap_t<piw::data_t, control_wire_t *>::lcktype children_;
    pic::ilist_t<control_wire_t> tickers_;
    bct_clocksink_t *up_;
    pic::ref_t<light_wire_t> light_wire_;


    pic::lckvector_t<unsigned>::nbtype courselen_;
    pic::lckvector_t<float>::nbtype scaleoffset_;
    pic::lckvector_t<float>::nbtype semitoneoffset_;
    pic::lckvector_t<float>::nbtype playing_scale_;
    float playing_max_note_; // last value in scale, derived from scale
    float playing_tonic_;
    float playing_base_note_;
    float playing_octave_;
    pic::lckvector_t<float>::nbtype reference_scale_;
    float reference_tonic_;

    bool root_light_;
    bool inverted_;
};



/// scale_illuminator_t::impl_t implementation
scale_illuminator_t::impl_t::impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c) :
	root_t(0), up_(0), playing_max_note_(0.0),
	playing_tonic_(0.0), playing_base_note_(0.0), playing_octave_(0.0),
	reference_tonic_(0.0),
	root_light_(true), inverted_(false)
{
	connect(c);
	cd->sink(this,"illuminatorinput");
	tick_enable(true);

	set_clock(this);
	light_wire_ = pic::ref(new light_wire_t(this));
	connect_wire(light_wire_.ptr(), light_wire_->source());
}

scale_illuminator_t::impl_t::~impl_t()
{
	LOG_SINGLE( pic::logmsg() << "scale_illuminator_t::impl_t::~impl_t";)
	tracked_invalidate();
	invalidate();
	delete light_wire_.ptr();
	light_wire_.assign(0);
}


void scale_illuminator_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
	control_wire_t *w;

	for(w=tickers_.head(); w!=0; w=tickers_.next(w))
	{
		w->ticked(f,t);
	}
}

void scale_illuminator_t::impl_t::add_ticker(control_wire_t *w)
{
	if(!tickers_.head())
	{
		tick_suppress(false);
	}

	tickers_.append(w);
}

void scale_illuminator_t::impl_t::del_ticker(control_wire_t *w)
{
	tickers_.remove(w);

	if(!tickers_.head())
	{
		tick_suppress(true);
	}

}

void scale_illuminator_t::impl_t::invalidate()
{
	tick_disable();

	pic::lckmap_t<piw::data_t,control_wire_t *>::lcktype::iterator ci;
	while((ci=children_.begin())!=children_.end())
	{
		delete ci->second;
	}
}

piw::wire_t *scale_illuminator_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
   pic::lckmap_t<piw::data_t,control_wire_t *>::lcktype::iterator ci;

	if((ci=children_.find(es.path()))!=children_.end())
	{
		delete ci->second;
	}

	return new control_wire_t(this, es);
}

void scale_illuminator_t::impl_t::root_closed() { invalidate(); }

void scale_illuminator_t::impl_t::root_opened() { root_clock(); root_latency(); }

void scale_illuminator_t::impl_t::root_clock()
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

void scale_illuminator_t::impl_t::root_latency()
{
	set_latency(get_latency());
}


void decode_courses(pic::lckvector_t<unsigned>::nbtype &courses, const piw::data_nb_t &courselen)
{
    courses.clear();

    LOG_SINGLE( pic::logmsg() << "decode_courses " << courselen;)

	unsigned dictlen = courselen.as_tuplelen();
    if(0 == dictlen)
        return;

    courses.reserve(5);

    for(unsigned i = 0; i < dictlen; ++i)
    {
    	unsigned l=courselen.as_tuple_value(i).as_long();
        courses.push_back(l);
    }

	#if LOG_MULTI > 0 
    unsigned column=0;
    pic::lckvector_t<unsigned>::nbtype::const_iterator ci,ce;
    ci = courses.begin();
    ce = courses.end();
    for(; ci != ce; ++ci)
	{
	 	pic::logmsg() << "decode_courses column " << column++ << "value= " << *ci;
	}
	#endif
}

void decode_courseoffset(pic::lckvector_t<float>::nbtype &scale,pic::lckvector_t<float>::nbtype &semis, const piw::data_nb_t &courseoffset)
{
	scale.clear();
	semis.clear();
    unsigned dictlen = courseoffset.as_tuplelen();
    if(0 == dictlen)
        return;

    scale.reserve(5);
    semis.reserve(5);

    for(unsigned i = 0; i < dictlen; ++i)
    {
    	float v=courseoffset.as_tuple_value(i).as_float();
    	float sc=0.0,sm=0.0;
    	if(v>9999)
    	{
    		sc=0;
    		sm=v-10000.0;
    	}
    	else
    	{
    		sm=0;
    		sc=v;
    	}

    	scale.push_back(sc);
    	semis.push_back(sm);
    }

    pic::lckvector_t<float>::nbtype::const_iterator ci,ce;
    ci = scale.begin();
    ce = scale.end();

	#if LOG_MULTI > 0 
	unsigned column=0;
	for(; ci != ce; ++ci)
	{
		pic::logmsg() << "decode_courseoffset column " << column++ << "courseoffset=" << *ci;
	}
	pic::lckvector_t<float>::nbtype::const_iterator si,se;
	si = semis.begin();
	se = semis.end();
	column=0;
	for(; si != se; ++si)
	{
		pic::logmsg() << "decode_courseoffset column " << column++ << "semioffset=" << *si;
	}
	#endif 
}

float decode_scale(pic::lckvector_t<float>::nbtype &scale, const std::string &s)
{
	LOG_SINGLE(    	pic::logmsg() << "decode_scale scale=" << s;)

	scale.clear();
    float max_note = 0;

    if(s.size()<=2)
        return 0;

    scale.reserve(12);

    std::istringstream iss(s.substr(1,s.size()-2));
    std::string part;
    while(std::getline(iss,part,','))
    {
        float f;
        std::istringstream(part) >> f;
        scale.push_back(f);
        if(f>max_note) max_note=f;
        LOG_SINGLE(    	pic::logmsg() << "decode_scale f=" << f;)
    }
    // we remove the last entry, as it is a repeat of the first, it only exists
    // for downstream scaler to determine the interval between last entry and first  (e.g B to C)
    scale.pop_back(); /// remove the last entry
    return max_note;
}


void scale_illuminator_t::impl_t::control_change(const piw::data_nb_t &d)
{
	#if LOG_MULTI > 0 
	if(d.is_null())
	{
		pic::logmsg() << "scale_illuminator_t::impl_t::control_change d=null!";
	}
	else
	{
		if(d.is_dict())
			pic::logmsg() << "scale_illuminator_t::impl_t::control_change " << d;
		else
			pic::logmsg() << "scale_illuminator_t::impl_t::control_change (not dict)" << d;
	}
	#endif 

    if(!d.is_null() && d.is_dict())
    {
		piw::data_nb_t t,b,o,s,co,cl;

		t = d.as_dict_lookup("tonic");
		if(!t.is_null())
		{
			LOG_SINGLE(	pic::logmsg() << "tonic " << t;)
			playing_tonic_ = t.as_renorm_float(0,12,0);
		}

		b = d.as_dict_lookup("base");
		if(!b.is_null())
		{
			LOG_SINGLE(	pic::logmsg() << "base " << b;)
			playing_base_note_ = b.as_renorm_float(-20,20,0);
		}

		o = d.as_dict_lookup("octave");
		if(!o.is_null())
		{
			LOG_SINGLE(	pic::logmsg() << "octave " << o;)
			playing_octave_ = o.as_renorm_float(-1,9,0);
		}

		s = d.as_dict_lookup("scale");
		if(s.is_string())
		{
			LOG_SINGLE(	pic::logmsg() << "scale "<< s ;)
			playing_max_note_=decode_scale(playing_scale_,s.as_stdstr());
		}
		else if (playing_scale_.size()==0)
		{
			LOG_SINGLE( pic::logmsg() << "defaulting to major scale";)
			//default input to major, just in case its not specified on KG
			playing_max_note_=decode_scale(playing_scale_,"[0,2,4,5,7,9,11,12]");
		}

		co = d.as_dict_lookup("courseoffset");
		if(!co.is_null())
		{
			LOG_SINGLE(	pic::logmsg() << "courseoffset " << co;)
			decode_courseoffset(scaleoffset_,semitoneoffset_, co);
		}
		cl = d.as_dict_lookup("courselen");
		if(!cl.is_null())
		{
			LOG_SINGLE( pic::logmsg() << "courselen " << cl;)
			decode_courses(courselen_,cl);
		}
		light_wire_->updateLights();
    }
}

static int __updateLights(void *f_, void * d_)
{
	light_wire_t *f = (light_wire_t *)f_;
	f->updateLights();
	return 0;
}

void scale_illuminator_t::impl_t::reference_scale(const std::string & s)
{
	decode_scale(reference_scale_,s);
	piw::tsd_fastcall(__updateLights,light_wire_.ptr(),NULL);
}

void scale_illuminator_t::impl_t::reference_tonic(float f)
{
	reference_tonic_ = f;
	piw::tsd_fastcall(__updateLights,light_wire_.ptr(),NULL);
}


void scale_illuminator_t::impl_t::root_light(bool v)
{
	root_light_=v;
	piw::tsd_fastcall(__updateLights,light_wire_.ptr(),NULL);
}

void scale_illuminator_t::impl_t::inverted(bool v)
{
	inverted_ = v;
	piw::tsd_fastcall(__updateLights,light_wire_.ptr(),NULL);
}

// MSH: this is NOT optimised, really we should precalc light status., but keep it simple to check 'logic'
// also I believe this will only get called on change anyway, so arguably, it doesnt really matter when/where we do it!
// this check is particularly ineffcient as we keep going thru vector
// one optimisation could be
// playing scale is repeated, so reocurences can be calculated.
// this means you can iterate thru playing scale, check if it exists (once) in ref scale, then set for all keys. i.e do one iteration of scale.
int note(float note, float tonic, float base_note, float semitone_offset, float max_note)
{
	return (int) (note+tonic+base_note+semitone_offset) % (int) max_note;
}

bool is_in_reference_scale(pic::lckvector_t<float>::nbtype& scale, float tonic, float base_note, float semitone_offset, float max_note, float value)
{
    pic::lckvector_t<float>::nbtype::const_iterator soi,soe;
    soi = scale.begin();
    soe = scale.end();
    for(;soi!=soe;soi++)
    {
    	if (note(*soi,tonic,base_note,semitone_offset,max_note) == value)
    	{
    		return true;
    	}
    }

	return false;
}


void updateLightBuffer(scale_illuminator_t::impl_t& impl,piw::xevent_data_buffer_t& outputbuffer)
{
    LOG_SINGLE( pic::logmsg() << "updateLightBuffer (start)";)
    piw::statusset_t status;

    pic::lckvector_t<unsigned>::nbtype::const_iterator cli,cle;
    pic::lckvector_t<float>::nbtype::const_iterator coi,coe;
    pic::lckvector_t<float>::nbtype::const_iterator smi,sme;
    pic::lckvector_t<float>::nbtype::const_iterator soi,soe;
    bool is_in_scale = false;

    if(impl.playing_scale_.size() == 0)
    {
    	LOG_SINGLE( pic::logmsg() << "updateLightBuffer (failed) - no playing scale";)
    	return;
    }

    cli = impl.courselen_.begin();
    cle = impl.courselen_.end();

    coi = impl.scaleoffset_.begin();
    coe = impl.scaleoffset_.end();

    smi = impl.semitoneoffset_.begin();
    sme = impl.semitoneoffset_.end();

    // go thru each course
    int course = 0;
	int course_offset = 0;
	int semitone_offset = 0;
    for(;cli!=cle;cli++)
    {
    	int course_len = *cli;
    	if(coi!=coe)
    	{
    		course_offset+=*coi;
    	}

    	if(smi!=sme)
    	{
    		semitone_offset+=*smi;
    		LOG_SINGLE( 		pic::logmsg() << "semitone_offset:" << *smi << " total " << semitone_offset;)
    	}

    	// go thru each row
        soi = impl.playing_scale_.begin();
        soe = impl.playing_scale_.end();
		for(int i=0;i<course_offset % (int) impl.playing_scale_.size();i++)
		{
			soi++;
			if(soi==soe)
			{
				soi=impl.playing_scale_.begin();
			}
		}

    	for(int row=0;row<course_len;row++)
    	{
    		unsigned char colour=BCTSTATUS_OFF;
    		int n = note(*soi,impl.playing_tonic_,impl.playing_base_note_,semitone_offset,impl.playing_max_note_);
    		bool is_tonic=false;
    		bool is_missing_ref_scale=impl.reference_scale_.size()==0;
    		if(impl.root_light_)
    		{
    			if (is_missing_ref_scale)
    			{
    				is_tonic = n == impl.playing_tonic_;
    			}
    			else
    			{
    				is_tonic = n== impl.reference_tonic_;
    			}
    		}
    		if(is_tonic)
    		{
    			colour= impl.inverted_ ? BCTSTATUS_ACTIVE : BCTSTATUS_INACTIVE;
    		}
    		else if(!is_missing_ref_scale)
    		{
    	 		is_in_scale = soi==soe ? false :
    	    				is_in_reference_scale(impl.reference_scale_,
    	    									  impl.reference_tonic_,
    	    									  0, // base_note
    	    									  0, // semi tone offset
    	    									  impl.playing_max_note_,
    	    									  n
    	    									);
				if (!impl.inverted_)
				{
					colour=is_in_scale?  BCTSTATUS_ACTIVE : BCTSTATUS_OFF;
				}
				else
				{
					colour=is_in_scale? BCTSTATUS_OFF : BCTSTATUS_INACTIVE;
				}
    		}
    		status.insert(piw::statusdata_t(true,piw::coordinate_t(course+1,row+1),colour));
        	soi++;
        	if(soi==soe)
        	{
        		soi=impl.playing_scale_.begin();
        	}
    	}

    	if(coi!=coe)
    	{
    		coi++;
    	}
    	if(smi!=sme)
    	{
    		smi++;
    	}
    	course++;
	}
    piw::data_nb_t buffer = piw::statusbuffer_t::make_statusbuffer(status);
    outputbuffer.add_value(OUT_LIGHT,buffer);
    LOG_SINGLE( pic::logmsg() << "updateLightBuffer d= "<< buffer;)
}



control_wire_t::control_wire_t(scale_illuminator_t::impl_t  *p, const piw::event_data_source_t &es):
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
	control_wire_t *w = (control_wire_t *)w_;
    if(w->root_)
    {
        w->root_->del_ticker(w);
    }
    return 0;
}

void control_wire_t::invalidate()
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

void control_wire_t::event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
{
    output_ = piw::xevent_data_buffer_t(OUT_MASK,PIW_DATAQUEUE_SIZE_NORM);

    input_ = b.iterator();

    unsigned long long t = id.time();
    last_from_ = t;

	piw::data_nb_t d;
	if(input_->latest(IN_CONTROL,d,t))
	{
		process(IN_CONTROL,d,t);
	}

    source_start(seq,id,output_);

    root_->add_ticker(this);
}

void control_wire_t::event_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
{
    input_->set_signal(s,n);
    input_->reset(s,t);
    ticked(last_from_,t);
}

bool control_wire_t::event_end(unsigned long long t)
{
    ticked(last_from_,t);
    root_->del_ticker(this);
    return source_end(t);
}

void control_wire_t::ticked(unsigned long long f, unsigned long long t)
{
    last_from_ = t;

	piw::data_nb_t d;
	unsigned s;
	while(input_->next(SIG1(IN_CONTROL),s,d,t))
	{
		process(s,d,t);
	}
}

void control_wire_t::source_ended(unsigned seq)
{
    event_ended(seq);
}


void control_wire_t::process(unsigned s, const piw::data_nb_t &d, unsigned long long t)
{
	LOG_SINGLE(	pic::logmsg() << "control_wire_t::process:" << s;)
    if(IN_CONTROL==s)
    {
    	root_->control_change(d);
    }
}


light_wire_t::light_wire_t(	scale_illuminator_t::impl_t *impl) : event_data_source_real_t(piw::pathone(1,0)), root_(impl)
{
	unsigned long long t = piw::tsd_time();
	output_ = piw::xevent_data_buffer_t(OUT_MASK,PIW_DATAQUEUE_SIZE_TINY);
	source_start(0,piw::pathone_nb(1,t),output_);
}

light_wire_t::~light_wire_t()
{
	LOG_SINGLE( pic::logmsg() << "light_wire_t::~light_wire_t";)
	source_shutdown();
}

void light_wire_t::source_ended(unsigned seq)
{
	LOG_SINGLE(	pic::logmsg() << "light_wire_t::source_ended";)
	source_end(piw::tsd_time());
}

void light_wire_t::updateLights()
{
	LOG_SINGLE(	pic::logmsg() << "light_wire_t::updateLights";)
	updateLightBuffer(*root_,output_);
}



//////////// scale_illuminator_t
scale_illuminator_t::scale_illuminator_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &output)
	: impl_(new impl_t(d,output))
{
}

scale_illuminator_t::~scale_illuminator_t()
{
	delete impl_;
}

piw::cookie_t scale_illuminator_t::cookie()
{
    return piw::cookie_t(impl_);
}



void scale_illuminator_t::reference_scale(const std::string &s)
{
	impl_->reference_scale(s);
}

void scale_illuminator_t::reference_tonic(float f)
{
	impl_->reference_tonic(f);
}

void scale_illuminator_t::root_light(bool v)
{
	impl_->root_light(v);
}

void scale_illuminator_t::inverted(bool v)
{
	impl_->inverted(v);
}

}; //namespace scale_illuminator_plg


