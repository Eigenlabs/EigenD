#include <piw/piw_ufilter.h>
#include <piw/piw_data.h>
#include <piw/piw_status.h>
#include <piw/piw_keys.h>

#include <plg_midi/midi_pgm_chooser.h>

#define IN_KEY 1
#define IN_CONTROL 2
#define IN_MASK SIG2(IN_KEY,IN_CONTROL)

#define OUT_LIGHT 1
#define OUT_MIDI 2
#define OUT_MASK SIG2(OUT_LIGHT,OUT_MIDI)

//#define LOG_SINGLE(x) x
#define LOG_SINGLE(x)

static int REFRESH_DISPLAY=0x01;
static int SEND_PROGRAM=0x02;
static int SEND_BANK=0x04;

namespace pi_midi
{
    struct midi_pgm_chooser_t::impl_t: piw::ufilterctl_t, piw::ufilter_t, virtual public pic::tracked_t
    {
        impl_t(const piw::cookie_t &output, piw::clockdomain_ctl_t *domain)
        : ufilter_t(this, output), max_col_(5),max_row_(24),channel_(1),is_bank_mode(false),program_(-1),bank_(-1),window_(0)
        {
        }

        piw::ufilterfunc_t *ufilterctl_create(const piw::data_t &path);
        unsigned long long ufilterctl_thru() { return 0; }
        unsigned long long ufilterctl_inputs() { return IN_MASK; }
        unsigned long long ufilterctl_outputs() { return OUT_MASK; }
        
        void control_change(const piw::data_nb_t &d);

        void reset()
        {
            is_bank_mode=false;
            bank(0);
            program(0);
            window(0);
        }

        void up()
        {
            if(isBankMode())
            {
                if(bank_<127) bank(bank_+1);
            }
            else
            {
                if(program_<127) program(program_+1);
            }
        }
        
        void down()
        {
            if(isBankMode())
            {
                if(bank_>0) bank(bank_-1);
            }
            else
            {
                if(program_>0) program(program_-1);
            }
        }

        int maxCol() { return max_col_;}
        int maxRow() { return max_row_;}
        
        int channel() { return channel_;}
        void channel(int c) { channel_=c;}

        bool isBankMode() { return is_bank_mode;}
        
        int bank() { return bank_;}
        void bank(int c)
        {
            if(c!=bank_)
            {
                bank_=c;
                changed(&SEND_BANK);
            }
        }

        int program() { return program_;}
        void program(int c)
        {
            if(c!=program_)
            {
                program_=c;
                changed(&SEND_PROGRAM);
            }
        }

        int window() { return window_;}
        void window(int w)
        {
            if(w!=window_)
            {
                window_=w;
                changed(&REFRESH_DISPLAY);
            }
        }
        
        
        void bank_mode(bool b)
        {
            if(is_bank_mode!=b)
            {
                is_bank_mode=b;
                changed(&REFRESH_DISPLAY);
            }
        }
        
        int max_col_;
        int max_row_;
        int channel_;
        bool is_bank_mode;
        int program_;
        int bank_;
        int window_;
        pic::lckvector_t<unsigned>::nbtype columnlen_;
    };
    
}

namespace
{
    struct midi_pgm_chooser_func_t: piw::ufilterfunc_t
    {
        midi_pgm_chooser_func_t(pi_midi::midi_pgm_chooser_t::impl_t *root) : root_(root),control_wire_(false)
        {
        }
        
        void output_light(piw::ufilterenv_t *env, int pgm)
        {
            if(!control_wire_) return;
            
            LOG_SINGLE(pic::logmsg() << "midi_pgm_chooser_func_t::output_light " << pgm;)
 
            piw::statusset_t status;
            pgm = pgm - root_->window();
            int x = ( pgm / root_->maxRow()) + 1;
            int y = ( pgm % root_->maxRow()) + 1;
            unsigned led_on=root_->isBankMode()?BCTSTATUS_INACTIVE:BCTSTATUS_ACTIVE;
            
            LOG_SINGLE(pic::logmsg() << "midi_pgm_chooser_func_t::output_light pos" << pgm << " x " << x << " y " << y << " r " << root_->maxRow();)
            for(int c=1;c<root_->maxCol()+1;c++)
            {
                for(int r=1;r<root_->maxRow()+1;r++)
                {
                    int s=(c==x && r==y?led_on:BCTSTATUS_OFF);
                    
                    status.insert(piw::statusdata_t(false,piw::coordinate_t(c,r),s));
                }
            }
            piw::data_nb_t d = piw::statusbuffer_t::make_statusbuffer(status);
            if(d.compare(current_lights_,false) != 0)
            {
                current_lights_=d;
                env->ufilterenv_output(OUT_LIGHT, d);
            }
        }
        
        void output_program_change(piw::ufilterenv_t *env, int num, int channel)
        {
            unsigned t=piw::tsd_time();
            unsigned d0= 0xC0+(channel-1);
            unsigned d1=num;
            
            unsigned char *blob = 0;
            piw::data_nb_t d = piw::makeblob_nb(t,2,&blob);
            blob[0] = (unsigned char)d0;
            blob[1] = (unsigned char)d1;
            LOG_SINGLE(pic::logmsg() << "midi_pgm_chooser_func_t::output_program_change d0=" << std::hex << (unsigned)d0 << " d1=" << (unsigned)d1 << " len=2 time=" << std::dec << t;)
            
            env->ufilterenv_output(OUT_MIDI, d);
        }
        
        void output_bank_change(piw::ufilterenv_t *env, int num, int channel)
        {
            unsigned t=piw::tsd_time();
            unsigned d0= 0xB0+(channel-1);
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
        }


        void ufilterfunc_changed(piw::ufilterenv_t *env,void* pmode)
        {
            int mode =*((int*) pmode);
            if(control_wire_)
            {
                int bank=root_->bank();
                int program=root_->program();
                
                if(mode!=REFRESH_DISPLAY)
                {
                    if(root_->channel()>0)
                    {
                        if(mode==SEND_BANK)
                        {
                            output_bank_change(env,bank,root_->channel());
                        }
                        output_program_change(env,program,root_->channel());
                    }
                    else
                    {
                        for(int c=1;c<=16;c++)
                        {
                            if(mode==SEND_BANK)
                            {
                                output_bank_change(env,bank,root_->channel());
                            }
                            output_program_change(env,program,root_->channel());
                        }
                    }
                }
                int pgm=(root_->isBankMode()?bank:program);
                output_light(env,pgm);
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
                        int v=((int(column)-1) * (root_->maxRow())) + (int) row - 1;
                        if(root_->isBankMode())
                        {
                            root_->bank(v + root_->window());
                        }
                        else
                        {
                            root_->program(v + root_->window());
                        }
                    }
                    break;
                }
                case IN_CONTROL:
                {
                    control_wire_=true;
                    root_->control_change(d);
                    break;
                }
            }
        }

        void ufilterfunc_end(piw::ufilterenv_t *env, unsigned long long to)
        {
            ;
        }

        pi_midi::midi_pgm_chooser_t::impl_t *root_;
        piw::data_nb_t id_;
        bool control_wire_;
        piw::data_nb_t current_lights_;
    };
}

piw::ufilterfunc_t * pi_midi::midi_pgm_chooser_t::impl_t::ufilterctl_create(const piw::data_t &path) { return new midi_pgm_chooser_func_t(this); }

pi_midi::midi_pgm_chooser_t::midi_pgm_chooser_t(const piw::cookie_t &output, piw::clockdomain_ctl_t *domain) : impl_(new impl_t(output, domain)) {}
piw::cookie_t pi_midi::midi_pgm_chooser_t::cookie() { return impl_->cookie(); }
pi_midi::midi_pgm_chooser_t::~midi_pgm_chooser_t() { delete impl_; }



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
    changed(&REFRESH_DISPLAY);
}


static int __reset(void *i_, void *v_)
{
    pi_midi::midi_pgm_chooser_t::impl_t *i = (pi_midi::midi_pgm_chooser_t::impl_t *)i_;
    i->reset();
    return 0;
}

void pi_midi::midi_pgm_chooser_t::reset()
{
    piw::tsd_fastcall(__reset,impl_,(void *) 0L);
}

static int __up(void *i_, void *v_)
{
    pi_midi::midi_pgm_chooser_t::impl_t *i = (pi_midi::midi_pgm_chooser_t::impl_t *)i_;
    i->up();
    return 0;
}

void pi_midi::midi_pgm_chooser_t::up()
{
    piw::tsd_fastcall(__up,impl_,(void *) 0L);
}

static int __down(void *i_, void *v_)
{
    pi_midi::midi_pgm_chooser_t::impl_t *i = (pi_midi::midi_pgm_chooser_t::impl_t *)i_;
    i->down();
    return 0;
}

void pi_midi::midi_pgm_chooser_t::down()
{
    piw::tsd_fastcall(__down,impl_,(void *) 0L);
}

    
    
    
static int __program(void *i_, void *v_)
{
    pi_midi::midi_pgm_chooser_t::impl_t *i = (pi_midi::midi_pgm_chooser_t::impl_t *)i_;
    unsigned v = *(unsigned *)v_;
    i->program(v);
    return 0;
}

void pi_midi::midi_pgm_chooser_t::program(unsigned v)
{
    piw::tsd_fastcall(__program,impl_,(void *) &v);
}


static int __bank(void *i_, void *v_)
{
    pi_midi::midi_pgm_chooser_t::impl_t *i = (pi_midi::midi_pgm_chooser_t::impl_t *)i_;
    unsigned v = *(unsigned *)v_;
    i->bank(v);
    return 0;
}

void pi_midi::midi_pgm_chooser_t::bank(unsigned v)
{
    piw::tsd_fastcall(__bank,impl_,(void *) &v);
}


static int __bank_mode(void *i_, void *v_)
{
    pi_midi::midi_pgm_chooser_t::impl_t *i = (pi_midi::midi_pgm_chooser_t::impl_t *)i_;
    bool v = *(bool *)v_;
    i->bank_mode(v);
    return 0;
}

void pi_midi::midi_pgm_chooser_t::bank_mode(bool v)
{
    piw::tsd_fastcall(__bank_mode,impl_,(void *) &v);
}

static int __window(void *i_, void *v_)
{
    pi_midi::midi_pgm_chooser_t::impl_t *i = (pi_midi::midi_pgm_chooser_t::impl_t *)i_;
    unsigned v = *(unsigned *)v_;
    i->window(v);
    return 0;
}

void pi_midi::midi_pgm_chooser_t::window(unsigned v)
{
    piw::tsd_fastcall(__window,impl_,(void *) &v);
}

static int __channel(void *i_, void *v_)
{
    pi_midi::midi_pgm_chooser_t::impl_t *i = (pi_midi::midi_pgm_chooser_t::impl_t *)i_;
    unsigned v = *(unsigned *)v_;
    i->channel(v);
    return 0;
}

void pi_midi::midi_pgm_chooser_t::channel(unsigned v)
{
    piw::tsd_fastcall(__channel,impl_,(void *) &v);
}





