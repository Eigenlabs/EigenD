#include <piw/piw_ufilter.h>
#include <piw/piw_data.h>
#include <piw/piw_status.h>
#include <piw/piw_keys.h>

#include <plg_midi/midi_pgm_chooser.h>

#define IN_KEY 1
#define IN_MASK SIG1(IN_KEY)

#define OUT_LIGHT 1
#define OUT_MIDI 2
#define OUT_MASK SIG2(OUT_LIGHT,OUT_MIDI)

#define LOG_SINGLE(x) x
//#define LOG_SINGLE(x)

namespace pi_midi
{
    struct midi_pgm_chooser_t::impl_t: piw::ufilterctl_t, piw::ufilter_t, virtual public pic::tracked_t
    {
        impl_t(const piw::cookie_t &output, piw::clockdomain_ctl_t *domain)
        : ufilter_t(this, output), max_col_(5),max_row_(24),channel_(1),is_bank_change(false)
        {
        }

        piw::ufilterfunc_t *ufilterctl_create(const piw::data_t &path);
        unsigned long long ufilterctl_thru() { return 0; }
        unsigned long long ufilterctl_inputs() { return IN_MASK; }
        unsigned long long ufilterctl_outputs() { return OUT_MASK; }
        
        void control_change(const piw::data_nb_t &d);

        void reset() { ; }

        int maxCol() { return max_col_;}
        int maxRow() { return max_row_;}
        int channel() { return channel_;}
        bool isBankChange() { return is_bank_change;}
        
        int max_col_;
        int max_row_;
        int channel_;
        bool is_bank_change;
        pic::lckvector_t<unsigned>::nbtype columnlen_;
    };
    
}

namespace
{
    struct midi_pgm_chooser_func_t: piw::ufilterfunc_t
    {
        midi_pgm_chooser_func_t(pi_midi::midi_pgm_chooser_t::impl_t *root) : root_(root),current_(0)
        {
        }
        
        void output_light(piw::ufilterenv_t *env, int x, int y,unsigned long long t)
        {
            piw::statusset_t status;
            for(int c=1;c<root_->maxCol()+1;c++)
            {
                for(int r=1;r<root_->maxRow()+1;r++)
                {
                    int s=(c==x && r==y?BCTSTATUS_ACTIVE:BCTSTATUS_OFF);
                    
                    status.insert(piw::statusdata_t(false,piw::coordinate_t(c,r),s));
                }
            }
            piw::data_nb_t d = piw::statusbuffer_t::make_statusbuffer(status);
            env->ufilterenv_output(OUT_LIGHT, d);
        }
        
        void output_program_change(piw::ufilterenv_t *env, int num, unsigned long long t)
        {
            unsigned d0= 0xC0+(root_->channel()-1);
            unsigned d1=num;
            
            unsigned char *blob = 0;
            piw::data_nb_t d = piw::makeblob_nb(t,2,&blob);
            blob[0] = (unsigned char)d0;
            blob[1] = (unsigned char)d1;
            LOG_SINGLE(pic::logmsg() << "midi_pgm_chooser_func_t::output_program_change d0=" << std::hex << (unsigned)d0 << " d1=" << (unsigned)d1 << " len=2 time=" << std::dec << t;)
            
            env->ufilterenv_output(OUT_MIDI, d);
        }
        
        void output_bank_change(piw::ufilterenv_t *env, int num, unsigned long long t)
        {
            unsigned d0= 0xB0+(root_->channel()-1);
            unsigned d1=0; //Bank CC
            unsigned d2=num;
            
            unsigned char *blob = 0;
            piw::data_nb_t d = piw::makeblob_nb(t,3,&blob);
            
            blob[0] = (unsigned char)d0;
            blob[1] = (unsigned char)d1;
            blob[2] = (unsigned char)d2;
            LOG_SINGLE(pic::logmsg() << "midi_pgm_chooser_func_t::output_bank_change d0=" << std::hex << (unsigned)d0 << " d1=" << (unsigned)d1 << " d2=" << (unsigned)d2 << " len=3 time=" << std::dec << t;)
            
            env->ufilterenv_output(OUT_MIDI, d);
        }

        // ufilter functions
        void ufilterfunc_start(piw::ufilterenv_t *env, const piw::data_nb_t &id)
        {
            id_ = id;

            env->ufilterenv_start(id.time());

            output_light(env, 1, 1, id.time());
            if(root_->isBankChange())
            {
                output_bank_change(env,current_,id.time());
            }
            else
            {
                output_program_change(env,current_,id.time());
            }
        }


        void ufilterfunc_data(piw::ufilterenv_t *env,unsigned sig,const piw::data_nb_t & d)
        {
            switch(sig)
            {
                case IN_KEY:
                {
                    float column, row, course, key;
                    piw::hardness_t hardness;
                    if(piw::decode_key(d,&column,&row,&course,&key,&hardness))
                    {
                        int v=((int(column)-1) * (root_->maxCol()-1)) + (int) row;
                        if(v!=current_)
                        {
                            output_light(env, (int) column, (int) row, d.time());
                            if(root_->isBankChange())
                            {
                                output_bank_change(env,v,d.time());
                            }
                            else
                            {
                                output_program_change(env,v,d.time());
                            }
                            current_=v;
                        }
                    }
                }
            }
        }

        void ufilterfunc_end(piw::ufilterenv_t *env, unsigned long long to)
        {
            ;
        }

        pi_midi::midi_pgm_chooser_t::impl_t *root_;
        piw::data_nb_t id_;
        int current_;
    };
}

piw::ufilterfunc_t * pi_midi::midi_pgm_chooser_t::impl_t::ufilterctl_create(const piw::data_t &path) { return new midi_pgm_chooser_func_t(this); }

pi_midi::midi_pgm_chooser_t::midi_pgm_chooser_t(const piw::cookie_t &output, piw::clockdomain_ctl_t *domain) : impl_(new impl_t(output, domain)) {}
piw::cookie_t pi_midi::midi_pgm_chooser_t::cookie() { return impl_->cookie(); }
pi_midi::midi_pgm_chooser_t::~midi_pgm_chooser_t() { delete impl_; }

piw::change_nb_t pi_midi::midi_pgm_chooser_t::control()
{
    LOG_SINGLE(pic::logmsg() << "piw::change_nb_t pi_midi::midi_pgm_chooser_t::control()";)
    return piw::change_nb_t::method(impl_,&pi_midi::midi_pgm_chooser_t::impl_t::control_change);
}


unsigned decode_unsigned(pic::lckvector_t<unsigned>::nbtype &vec, const piw::data_nb_t &inp)
{
    vec.clear();
    
	LOG_SINGLE(pic::logmsg() << "midi_pgm_chooser_t decode_unsigned " << inp;)
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
    return max;
}


void pi_midi::midi_pgm_chooser_t::impl_t::control_change(const piw::data_nb_t &d)
{
    LOG_SINGLE(pic::logmsg() << "pi_midi::midi_pgm_chooser_t::impl_t::control_change";)
	if(d.is_null())
	{
		pic::logmsg() << "midi_pgm_chooser_t::impl_t::control_change d=null!";
	}
	else
	{
		if(d.is_dict())
		{
			LOG_SINGLE(pic::logmsg() << "midi_pgm_chooser_t::impl_t::control_change " << d;)
		}
		else
			pic::logmsg() << "midi_pgm_chooser_t::impl_t::control_change (not dict)" << d;
	}
    
    if(!d.is_null() && d.is_dict())
    {
		piw::data_nb_t columns;
        
		columns = d.as_dict_lookup("columnlen");
		if(!columns.is_null())
		{
			LOG_SINGLE(pic::logmsg() << "t3d_output_t::impl_t::control_change columnlen " << columns;)
			max_row_=decode_unsigned(columnlen_,columns);
			max_col_=columnlen_.size();
            
			LOG_SINGLE(pic::logmsg() << "t3d_output_t::impl_t::control_change nr " << max_row_ << " nc " << max_col_;)
		}
        //		columnsoffset = d.as_dict_lookup("columnsoffset");
        //		if(!columns.is_null())
        //		{
        //			pic::logmsg() << "columnsoffset " << columns;
        //			decode_unsigned(columnsoffset_,columnsoffset);
        //		}
    }
}


static int __reset(void *i_, void *v_)
{
    pi_midi::midi_pgm_chooser_t::impl_t *i = (pi_midi::midi_pgm_chooser_t::impl_t *)i_;
//    bool v = *(bool *)v_;
    i->reset();
    return 0;
}

void pi_midi::midi_pgm_chooser_t::reset()
{
    piw::tsd_fastcall(__reset,impl_,(void *) 0L);
}


