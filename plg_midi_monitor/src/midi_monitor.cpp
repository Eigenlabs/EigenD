
#include "midi_monitor.h"
#include <piw/piw_cfilter.h>
#include <piw/piw_data.h>
#include <piw/piw_status.h>

#include <picross/pic_ref.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_keys.h>
#include <picross/pic_ref.h>

#include "libmidi/midi_decoder.h"

#define OUT_LIGHT 1
#define OUT_MASK SIG1(OUT_LIGHT)

#define IN_MIDI 2


namespace midi_monitor_plg
{

//////////// midi_monitor_t::impl_t  declaration

struct midi_wire_t:
    piw::wire_t,
    piw::wire_ctl_t,
    piw::event_data_sink_t,
    piw::event_data_source_real_t,
    virtual public pic::lckobject_t,
    pic::element_t<>
{
    midi_wire_t(midi_monitor_t::impl_t *p, const piw::event_data_source_t &);
    ~midi_wire_t() { invalidate(); }

    void wire_closed() { delete this; }
    void invalidate();
    void event_start(unsigned,const piw::data_nb_t &, const piw::xevent_data_buffer_t &);
    bool event_end(unsigned long long);
    void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &);
    void process(unsigned, const piw::data_nb_t &, unsigned long long);
    void ticked(unsigned long long f, unsigned long long t);
    void source_ended(unsigned);

    midi_monitor_t::impl_t *root_;
    piw::xevent_data_buffer_t::iter_t input_;
    piw::xevent_data_buffer_t output_;

    unsigned long long last_from_;
};

// light wire used to power the lights ;o) 
struct light_wire_t: piw::wire_ctl_t, piw::event_data_source_real_t, virtual public pic::counted_t
{
	light_wire_t(midi_monitor_t::impl_t *impl);
	~light_wire_t();
	void source_ended(unsigned seq);
	void updateLights();
	void midiNote(int channel, int note, int velocity);
	void midiCC(int channel, int cc, int value);
	void midiPolyPressure(int channel, int note, int velocity);

	midi_monitor_t::impl_t *root_;
	piw::xevent_data_buffer_t output_;
	piw::statusset_t status_;
};



//////////// midi_monitor_t::impl_t  declaration
// this is based on latch, please look there for general comments on eigenD usage
struct midi_monitor_t::impl_t :
	    piw::root_t,
	    piw::root_ctl_t,
	    piw::clocksink_t,
		midi::mididecoder_t,
	    virtual pic::lckobject_t
{
    impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &oc);
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

	// callbacks for midi_decoder
    void decoder_noteoff(unsigned channel, unsigned number, unsigned velocity);
    void decoder_noteon(unsigned channel, unsigned number, unsigned velocity);
    void decoder_polypressure(unsigned channel, unsigned number, unsigned value);
    void decoder_cc(unsigned channel, unsigned number, unsigned value);

    // not interested in these at this time
    void decoder_programchange(unsigned channel, unsigned value) {}
    void decoder_channelpressure(unsigned channel, unsigned value) {}
    void decoder_pitchbend(unsigned channel, unsigned value) {}

	// from midi wire
    void decode_midi(const piw::data_nb_t &d, unsigned long long t);
	// from control callback
    void control_change(const piw::data_nb_t &d);

    pic::lckmap_t<piw::data_t, midi_wire_t *>::lcktype children_;
    pic::ilist_t<midi_wire_t> tickers_;
    bct_clocksink_t *up_;
    pic::ref_t<light_wire_t> light_wire_;

    pic::lckvector_t<unsigned>::nbtype courselen_;
    pic::lckvector_t<float>::nbtype scaleoffset_;  // couse offset set in scale intervals 
    pic::lckvector_t<float>::nbtype semitoneoffset_; // course offset set in semitones (normal)
    pic::lckvector_t<float>::nbtype playing_scale_;
    pic::lckvector_t<unsigned>::nbtype columnlen_;
    pic::lckvector_t<unsigned>::nbtype columnsoffset_;

    float playing_max_note_; // last value in scale, derived from scale
    float playing_tonic_;
    float playing_base_note_;
    float playing_octave_;

    bool enable_notes_;
    bool enable_cc_per_key_;
    bool enable_cc_as_course_;
    bool enable_poly_pressure_;

    int  channel_;
    bool nearestMatch_;
    bool firstMatchOnly_;
    bool use_velocity_as_state_;
    bool use_channel_as_state_;
    bool use_physical_mapping_;
    int control_offset_;
};



/// midi_monitor_t::impl_t implementation
midi_monitor_t::impl_t::impl_t(piw::clockdomain_ctl_t *cd, const piw::cookie_t &c) :
	root_t(0), up_(0),
	playing_max_note_(0.0),
	playing_tonic_(0.0), playing_base_note_(0.0), playing_octave_(0.0),
	enable_notes_(true), enable_cc_per_key_(false), enable_cc_as_course_(false), enable_poly_pressure_(false),
	channel_(0), nearestMatch_(false), firstMatchOnly_(false),
	use_velocity_as_state_(true), use_channel_as_state_(false),
	use_physical_mapping_(false),
	control_offset_(0)

{
	connect(c);
	cd->sink(this,"monitorinput");
	tick_enable(true);

	set_clock(this);
	light_wire_ = pic::ref(new light_wire_t(this));
	connect_wire(light_wire_.ptr(), light_wire_->source());
}

midi_monitor_t::impl_t::~impl_t()
{
	// pic::logmsg() << "midi_monitor_t::impl_t::~impl_t";
	tracked_invalidate();
	invalidate();
	delete light_wire_.ptr();
	light_wire_.assign(0);
}


void midi_monitor_t::impl_t::clocksink_ticked(unsigned long long f, unsigned long long t)
{
	midi_wire_t *w;

	for(w=tickers_.head(); w!=0; w=tickers_.next(w))
	{
		w->ticked(f,t);
	}
}

void midi_monitor_t::impl_t::add_ticker(midi_wire_t *w)
{
	if(!tickers_.head())
	{
		tick_suppress(false);
	}

	tickers_.append(w);
}

void midi_monitor_t::impl_t::del_ticker(midi_wire_t *w)
{
	tickers_.remove(w);

	if(!tickers_.head())
	{
		tick_suppress(true);
	}

}

void midi_monitor_t::impl_t::invalidate()
{
	tick_disable();

	pic::lckmap_t<piw::data_t,midi_wire_t *>::lcktype::iterator ci;
	while((ci=children_.begin())!=children_.end())
	{
		delete ci->second;
	}
}

piw::wire_t *midi_monitor_t::impl_t::root_wire(const piw::event_data_source_t &es)
{
   pic::lckmap_t<piw::data_t,midi_wire_t *>::lcktype::iterator ci;

	if((ci=children_.find(es.path()))!=children_.end())
	{
		delete ci->second;
	}

	return new midi_wire_t(this, es);
}

void midi_monitor_t::impl_t::root_closed() { invalidate(); }

void midi_monitor_t::impl_t::root_opened() { root_clock(); root_latency(); }

void midi_monitor_t::impl_t::root_clock()
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

void midi_monitor_t::impl_t::root_latency()
{
	set_latency(get_latency());
}


void decode_unsigned(pic::lckvector_t<unsigned>::nbtype &vec, const piw::data_nb_t &inp)
{
    vec.clear();

	pic::logmsg() << "decode_unsigned " << inp;

	unsigned dictlen = inp.as_tuplelen();
    if(0 == dictlen)
        return;

    vec.reserve(5);

    for(unsigned i = 0; i < dictlen; ++i)
    {
    	unsigned l=inp.as_tuple_value(i).as_long();
        vec.push_back(l);
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
}

float decode_scale(pic::lckvector_t<float>::nbtype &scale, const std::string &s)
{
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
//    	pic::logmsg() << "decode_scale f=" << f;
    }
    // we remove the last entry, as it is a repeat of the first, it only exists
    // for downstream midir to determine the interval between last entry and first  (e.g B to C)
    scale.pop_back(); /// remove the last entry
    return max_note;
}

// helpers
int midiVelocityToStatus(int velocity)
{
	if (velocity==0) return BCTSTATUS_OFF;
	if (velocity<43) return BCTSTATUS_ACTIVE;
	if (velocity<86) return BCTSTATUS_INACTIVE;
	return BCTSTATUS_UNKNOWN;
}

int midiCCToStatus(int ccvalue)
{
	return midiVelocityToStatus(ccvalue);
}

int midiPressureToStatus(int pressure)
{
	return midiVelocityToStatus(pressure);
}

int midiChannelToStatus(int channel,int velocity)
{
	if (velocity==0) return BCTSTATUS_OFF;
	return ((channel-1) % (BCTSTATUS_MIXED - 1)) + 1;
}


// callbacks from decoder
void midi_monitor_t::impl_t::decoder_noteoff(unsigned channel, unsigned number, unsigned velocity)
{
	if (!enable_notes_) return;
	//pic::logmsg() << "note off " << channel << " n " << number << " v " << velocity ;
	light_wire_->midiNote(channel+1,number,0); // for the moment, treat noteoff, as midi note on, with velocity zero (pretty common practice)
}

void midi_monitor_t::impl_t::decoder_noteon(unsigned channel, unsigned number, unsigned velocity)
{
	if (!enable_notes_) return;
	//pic::logmsg() << "noteon " << channel << " n " << number << " v " << velocity ;
	light_wire_->midiNote(channel+1,number,velocity);
}


void midi_monitor_t::impl_t::decoder_polypressure(unsigned channel, unsigned number, unsigned value)
{
	if (!enable_poly_pressure_) return;
	light_wire_->midiPolyPressure(channel+1,number,value);
}

void midi_monitor_t::impl_t::decoder_cc(unsigned channel, unsigned number, unsigned value)
{
	if (! (enable_cc_as_course_ || enable_cc_per_key_) ) return;
	light_wire_->midiCC(channel+1,number,value);
}

void midi_monitor_t::impl_t::decode_midi(const piw::data_nb_t &d, unsigned long long t)
{
	unsigned length = d.as_bloblen();
	const unsigned char *buffer = (const unsigned char *)(d.as_blob());
	decoder_input(buffer, length);
}

void midi_monitor_t::impl_t::control_change(const piw::data_nb_t &d)
{
	if(d.is_null())
	{
		pic::logmsg() << "midi_monitor_t::impl_t::control_change d=null!";
	}
	else
	{
		if(d.is_dict())
			pic::logmsg() << "midi_monitor_t::impl_t::control_change " << d;
		else
			pic::logmsg() << "midi_monitor_t::impl_t::control_change (not dict)" << d;
	}
    if(!d.is_null() && d.is_dict())
    {
		piw::data_nb_t t,b,o,s,co,cl,columns,columnsoffset;

		t = d.as_dict_lookup("tonic");
		if(!t.is_null())
		{
			pic::logmsg() << "tonic " << t;
			playing_tonic_ = t.as_renorm_float(0,12,0);
		}

		b = d.as_dict_lookup("base");
		if(!b.is_null())
		{
			pic::logmsg() << "base " << b;
			playing_base_note_ = b.as_renorm_float(-20,20,0);
		}

		o = d.as_dict_lookup("octave");
		if(!o.is_null())
		{
			pic::logmsg() << "octave " << o;
			playing_octave_ = o.as_renorm_float(-1,9,0);
		}

		s = d.as_dict_lookup("scale");
		if(s.is_string())
		{
			pic::logmsg() << "scale "<< s ;
			playing_max_note_=decode_scale(playing_scale_,s.as_stdstr());
		}
		else if (playing_scale_.size()==0)
		{
			pic::logmsg() << "defaulting to major scale";
			//default input to major, just in case its not specified on KG
			playing_max_note_=decode_scale(playing_scale_,"[0,2,4,5,7,9,11,12]");
		}

		co = d.as_dict_lookup("courseoffset");
		if(!co.is_null())
		{
			pic::logmsg() << "courseoffset " << co;
			decode_courseoffset(scaleoffset_,semitoneoffset_, co);
		}
		cl = d.as_dict_lookup("courselen");
		if(!cl.is_null())
		{
			pic::logmsg() << "courselen " << cl;
			decode_unsigned(courselen_,cl);
		}
		columns = d.as_dict_lookup("columnlen");
		if(!columns.is_null())
		{
			pic::logmsg() << "columnlen " << columns;
			decode_unsigned(columnlen_,columns);
		}
		columnsoffset = d.as_dict_lookup("columnsoffset");
		if(!columns.is_null())
		{
			pic::logmsg() << "columnsoffset " << columns;
			decode_unsigned(columnsoffset_,columnsoffset);
		}
    }
}

void lightColumn(midi_monitor_t::impl_t& impl,piw::xevent_data_buffer_t& outputbuffer,piw::statusset_t& status, int col, int value)
{
//	pic::logmsg() <<  "E:lightPhysicalColumn n=" << col << "v=" << value;

	if (col<0 || col> (int) impl.columnlen_.size()) return;

	pic::lckvector_t<unsigned>::nbtype::const_iterator cli,cle;

    cli = impl.columnlen_.begin();  // lengh of a given course
    cle = impl.columnlen_.end();

    // first find the length of the appropriate column.
	for(int c=0;cli!=cle && c<col ;cli++,c++) { ; }

	if(cli==cle) return;

   	int column_len = *cli;  // max length of course

//   	pic::logmsg() <<  "1:lightPhysicalColumn column_len=" << column_len ;

   	float step=127/column_len;

   	if (column_len<1) return;
   	if (column_len==1)
   	{
   		// if only one key, then use the normal 'cc per key' behaviour
   		int state = midiCCToStatus(value);
		piw::statusdata_t s(!impl.use_physical_mapping_,piw::coordinate_t(col+1,1),state);
		pic::lckset_t<piw::statusdata_t>::nbtype::iterator si=status.find(s);
		if(si!=status.end())
		{
			status.erase(si);
		}
		status.insert(s);
   	}
   	else
   	{
   		// real column behaviour of a bar
		for(int i=0; i<column_len;i++)
		{
			int state = BCTSTATUS_OFF;
			if(value>i*step)
			{
				state= i / (column_len/3) + 1;
			}

			piw::statusdata_t s(!impl.use_physical_mapping_,piw::coordinate_t(col+1,i+1),state);
			pic::lckset_t<piw::statusdata_t>::nbtype::iterator si=status.find(s);
			if(si!=status.end())
			{
				status.erase(si);
			}
			status.insert(s);
		}
   	}

    piw::data_nb_t buffer = piw::statusbuffer_t::make_statusbuffer(status);
    outputbuffer.add_value(OUT_LIGHT,buffer);
}

// given a midi note, update all LEDs representing that value using the musical layout
void lightKey(midi_monitor_t::impl_t& impl,piw::xevent_data_buffer_t& outputbuffer,piw::statusset_t& status, int note, int state)
{
	//pic::logmsg() << "lightkey " << note << " s " << state;
    pic::lckvector_t<unsigned>::nbtype::const_iterator cli,cle;
    pic::lckvector_t<float>::nbtype::const_iterator coi,coe;
    pic::lckvector_t<float>::nbtype::const_iterator smi,sme;
    pic::lckvector_t<float>::nbtype::const_iterator soi,soe;

    if(note < 0 || impl.playing_scale_.size() == 0) return;

    // what is the midi note of the top of keygroup?
    int base = (impl.playing_octave_ + 1) * 12; // midi octaves go -2 to 8, whereas eigenD goes -1 to 9 !!
    base += impl.playing_tonic_;
    base += impl.playing_base_note_;

    // note: courses can have -ve offsets, so base may be greater than midi note,
    // and still be valid for keygroup


    // a bunch of iterators, that are information about the courses....
    cli = impl.courselen_.begin();  // lengh of a given course
    cle = impl.courselen_.end();
    coi = impl.scaleoffset_.begin(); // offset in scale intervals for a given course
    coe = impl.scaleoffset_.end();
    smi = impl.semitoneoffset_.begin();  // offset in semitones for a given course
    sme = impl.semitoneoffset_.end();

    // ASSUMPTIONS
    // there is a course length for every course
    // courselen(i) equates to semioffset(i) equates to scaleoffset(i)
    // exceptions
    // there can be less offsets than courselen, in which case offset = 0
    // there can be both a semitoneoffset and scaleoffset (though in practice eigenD doesnt support this)
    // courses are continous i.e. [1,1] [1,2] [1,3] NOT [1,1] [1,3]

    int course = 0;
	int semitone_offset = 0;

   	bool quitCourseSearch=false;
 	// for each course
	for(;cli!=cle && quitCourseSearch==false;cli++)
    {
    	int course_len = *cli;  // max length of course
    	if(coi!=coe)
    	{
    		int scale_offset=*coi;
    		int scale_semi= ( scale_offset /(int) impl.playing_scale_.size() ) * impl.playing_max_note_;

    		// calculate semitone offset by going thru scale by the scale offset
            soi = impl.playing_scale_.begin();
            soe = impl.playing_scale_.end();
    		for(int i=0;i<scale_offset % (int) impl.playing_scale_.size();i++)
    		{
    			soi++;
    		}
    		semitone_offset+=*soi+scale_semi;
    	}

    	if(smi!=sme)
    	{
    		semitone_offset+=*smi;
    	}

    	int coursebase=base+semitone_offset;

    	if(note >= coursebase)
    	{
			//now we want to determine position within course
			int midioffset = note - coursebase;  //  num of semitones within course

			// now we need to determine position within course, the note within the course
			// is in scale intervals

			// first how many entire scales are there
			// (comments will assume 12/octave for ease of explanation, but code does not make that assumption!)

			int scalemuliple = midioffset / impl.playing_max_note_ ; // number of full octaves (12)

			if (midioffset < ( (scalemuliple + 1) * impl.playing_max_note_ ))
			{
				// determine the number of semitones over the scales
				int partialsemitones = midioffset % (int) impl.playing_max_note_;

				int pos = scalemuliple * impl.playing_scale_.size();

				soi = impl.playing_scale_.begin();
				soe = impl.playing_scale_.end();

				bool found=false;

				for(;soi!=soe && pos < course_len && !found;soi++)
				{
					if(partialsemitones - *soi == 0 )
					{
						piw::statusdata_t s(!impl.use_physical_mapping_,piw::coordinate_t(course+1,pos+1),state);
						pic::lckset_t<piw::statusdata_t>::nbtype::iterator i=status.find(s);
						if(i!=status.end())
						{
							status.erase(i);
						}
						status.insert(s);
						found=true;
					}
					else if(partialsemitones - *soi < 0 )
					{
						// over shoot, e.g. C# in C Maj, we hit here at D
						if(impl.nearestMatch_)
						{
							piw::statusdata_t s(!impl.use_physical_mapping_,piw::coordinate_t(course+1,pos+1),state);
							pic::lckset_t<piw::statusdata_t>::nbtype::iterator i=status.find(s);
							if(i!=status.end())
							{
								status.erase(i);
							}
							status.insert(s);
						}
						else
						{
							// else we are only using exact matches
						}
						found=true;
					}
					pos++;
				}
				if(found)
				{
					if(impl.firstMatchOnly_)
					{
						quitCourseSearch=true;
					}
				}
				else
				{
					//else ps > 0, meaning we ran out of notes... due to not a full 'octave'
				}
			}
			else
			{
				// else note is above range of course
			}
    	}
    	else
    	{
        	// else note is below the base of the course, i.e. out of range
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
}
void lightMidiNote(midi_monitor_t::impl_t& impl,piw::xevent_data_buffer_t& outputbuffer,piw::statusset_t& status, int channel, int note, int velocity)
{
	if (impl.channel_ == 0 || impl.channel_==channel)
	{
		int state = 0;
		if (! (impl.use_channel_as_state_ || impl.use_velocity_as_state_) && velocity > 0) state=BCTSTATUS_UNKNOWN;
		if (impl.use_channel_as_state_) state+=midiChannelToStatus(channel,velocity);
		if (impl.use_velocity_as_state_) state+=midiVelocityToStatus(velocity);
		state = state % BCTSTATUS_MIXED;
		lightKey(impl,outputbuffer,status, note,state);
		return;
	}
}

void lightMidiPolyPressure(midi_monitor_t::impl_t& impl,piw::xevent_data_buffer_t& outputbuffer,piw::statusset_t& status, int channel, int note, int pressure)
{
	if (impl.channel_ == 0 || impl.channel_==channel)
	{
		lightKey(impl,outputbuffer,status,note,midiPressureToStatus(pressure));
		return;
	}
}


void lightMidiCC(midi_monitor_t::impl_t& impl,piw::xevent_data_buffer_t& outputbuffer,piw::statusset_t& status, int channel, int cc, int value)
{
	if (impl.channel_ == 0 || impl.channel_==channel)
	{
		if(impl.enable_cc_per_key_)
		{
			// light individuals keys based on CC value... currently using CC num as note number for layout
			lightKey(impl,outputbuffer,status,cc-impl.control_offset_,midiCCToStatus(value));
			return;
		}

		if(impl.enable_cc_as_course_)
		{
			// this will use a course, and put the CC value as a 'bar' on this course.
			lightColumn(impl,outputbuffer,status,cc-impl.control_offset_,value);
		}
	}
}



// midi wire
midi_wire_t::midi_wire_t(midi_monitor_t::impl_t  *p, const piw::event_data_source_t &es):
    piw::event_data_source_real_t(es.path()),
    root_(p),
    last_from_(0)
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
    output_ = piw::xevent_data_buffer_t(OUT_MASK,PIW_DATAQUEUE_SIZE_NORM);

    input_ = b.iterator();

    unsigned long long t = id.time();
    last_from_ = t;

	piw::data_nb_t d;
	if(input_->latest(IN_MIDI,d,t))
	{
		process(IN_MIDI,d,t);
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
	while(input_->next(SIG1(IN_MIDI),s,d,t))
	{
		process(s,d,t);
	}
}

void midi_wire_t::source_ended(unsigned seq)
{
    event_ended(seq);
}


void midi_wire_t::process(unsigned s, const piw::data_nb_t &d, unsigned long long t)
{
    if(IN_MIDI==s)
    {
		root_->decode_midi(d,t);
    }
}

// light wire 
light_wire_t::light_wire_t(	midi_monitor_t::impl_t *impl) : event_data_source_real_t(piw::pathone(1,0)), root_(impl)
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

void  light_wire_t::midiNote(int channel, int note, int velocity)
{
	lightMidiNote(*root_,output_, status_, channel,note,velocity);
}

void  light_wire_t::midiPolyPressure(int channel, int note, int pressure)
{
	lightMidiPolyPressure(*root_,output_, status_, channel,note,pressure);
}

void  light_wire_t::midiCC(int channel, int cc, int value)
{
//	pic::logmsg() <<  "light_wire_t::lightMidiCC c=" << channel << " cc=" << cc << " v=" << value ;
	lightMidiCC(*root_,output_,status_, channel,cc,value);
}


//////////// midi_monitor_t
midi_monitor_t::midi_monitor_t(piw::clockdomain_ctl_t *d, const piw::cookie_t &output)
	: impl_(new impl_t(d,output))
{
}

midi_monitor_t::~midi_monitor_t()
{
	delete impl_;
}

piw::cookie_t midi_monitor_t::cookie()
{
    return piw::cookie_t(impl_);
}


piw::change_nb_t midi_monitor_t::control()
{
    return piw::change_nb_t::method(impl_,&midi_monitor_t::impl_t::control_change);
}

void midi_monitor_t::enable_notes(bool v)
{
	if(impl_) impl_->enable_notes_=v;
}

void midi_monitor_t::enable_poly_pressure(bool v)
{
	if(impl_) impl_->enable_poly_pressure_=v;
}

void midi_monitor_t::enable_cc_as_key(bool v)
{
	if(impl_) impl_->enable_cc_per_key_=v;
}

void midi_monitor_t::enable_cc_as_course(bool v)
{
	if(impl_) impl_->enable_cc_as_course_=v;
}

void midi_monitor_t::channel(unsigned v)
{
	if(impl_) impl_->channel_=v;
}

void midi_monitor_t::nearest_match(bool v)
{
	if(impl_) impl_->nearestMatch_=v;
}

void midi_monitor_t::first_match(bool v)
{
	if(impl_) impl_->firstMatchOnly_=v;
}

void midi_monitor_t::use_velocity_as_state(bool v)
{
	if(impl_) impl_->use_velocity_as_state_=v;
}

void midi_monitor_t::use_channel_as_state(bool v)
{
	if(impl_) impl_->use_channel_as_state_=v;
}

void  midi_monitor_t::use_physical_mapping(bool v)
{
	if(impl_) impl_->use_physical_mapping_=v;
}

void  midi_monitor_t::control_offset(unsigned v)
{
	if(impl_) impl_->control_offset_=v;
}



}; //namespace midi_monitor_plg


