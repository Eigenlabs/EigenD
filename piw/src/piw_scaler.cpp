
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

#define SCALER_DEBUG 0

#include <piw/piw_scaler.h>
#include <piw/piw_tsd.h>
#include <piw/piw_ufilter.h>
#include <piw/piw_status.h>
#include <piw/piw_aggregator.h>
#include <piw/piw_keys.h>
#include <picross/pic_float.h>

#include <vector>
#include <math.h>
#include <sstream>


namespace
{
    class sfunc_t;
    class ctlfunc_t;
}

static float __step(float x)
{
    if(x <= -0.5)
        return -1.0;
    if(x < 0.5)
        return 0.0;
    return 1.0;
}

pic::f2f_t piw::step_bucket() { return pic::f2f_t::callable(__step); }


/**
 * scaler_controller_t
 */

void piw::scaler_controller_t::parsevector(const std::string &s, pic::lckvector_t<float>::nbtype *v, unsigned sizehint)
{
    v->clear();

    if(s.size()<=2)
        return;

    v->reserve(sizehint);

    std::istringstream iss(s.substr(1,s.size()-2));
    std::string part;
    while(std::getline(iss,part,','))
    {
        float f;
        std::istringstream(part) >> f;
        v->push_back(f);
    }
}

void piw::scaler_controller_t::tuple2vector(const data_nb_t &t, pic::lckvector_t<float>::nbtype *v)
{
    PIC_ASSERT(t.is_tuple());

    v->clear();

    unsigned l = t.as_tuplelen();
    v->reserve(l);

    for(unsigned i = 0; i < l; i++)
    {
        data_nb_t e = t.as_tuple_value(i);
        if (e.is_float()) v->push_back(e.as_float());
        else if (e.is_long()) v->push_back(e.as_long());
        else PIC_THROW("unsupported tuple element type");
    }
}

struct piw::scaler_controller_t::impl_t: piw::ufilterctl_t, piw::ufilter_t, virtual pic::lckobject_t
{
    impl_t(const cookie_t &l): ufilter_t(this,l)
    {
        common_scales_.resize(8);
        common_scales_[0]=pic::ref(new scaler_controller_t::scale_t("[0,2,4,5,7,9,11,12]"));
        common_scales_[1]=pic::ref(new scaler_controller_t::scale_t("[0,1,2,3,4,5,6,7,8,9,10,11,12]"));
        common_scales_[2]=pic::ref(new scaler_controller_t::scale_t("[0,2,4,6,8,10,12]"));
        common_scales_[3]=pic::ref(new scaler_controller_t::scale_t("[0,2,3,5,7,8,10,12]"));
        common_scales_[4]=pic::ref(new scaler_controller_t::scale_t("[0,3,5,6,7,10,12]"));
        common_scales_[5]=pic::ref(new scaler_controller_t::scale_t("[0,2,3,6,7,8,11,12]"));
        common_scales_[6]=pic::ref(new scaler_controller_t::scale_t("[0,3,5,7,10,12]"));
        common_scales_[7]=pic::ref(new scaler_controller_t::scale_t("[0,2,4,7,9,12]"));
    }

    ufilterfunc_t *ufilterctl_create(const piw::data_t &);
    void ufilterctl_delete(ufilterfunc_t *f);

    unsigned long long ufilterctl_inputs() { return SIG1(1); }
    unsigned long long ufilterctl_outputs() { return SIG1(1); }
    unsigned long long ufilterctl_thru() { return 0ULL; }

    void control_changed(const piw::data_nb_t &id);
    void add_subscriber(scaler_subscriber_t *s) { subscribers_.push_back(s); }
    void del_subscriber(scaler_subscriber_t *s) { subscribers_.remove(s); }

    void update_lights(const piw::data_nb_t &id, piw::scaler_controller_t::sref_t scale, float mode, bool override);

    scaler_controller_t::bits_t bits(const piw::data_nb_t &id)
    {
        scaler_controller_t::bits_t b;

        const unsigned char *p = id.as_path();
        int l = id.as_pathlen();

        for(;l>=0;l--)
        {
            piw::data_nb_t d  = piw::makepath_nb(p,l);

            pic::lckmap_t<piw::data_nb_t,float,piw::path_less>::nbtype::iterator fi;
            pic::lckmap_t<piw::data_nb_t,sref_t,piw::path_less>::nbtype::iterator si;
            pic::lckmap_t<piw::data_nb_t,lref_t,piw::path_less>::nbtype::iterator li;

            if(!(b.bits&BTONIC))
            {
                fi=tonic_.find(d);
                if(fi!=tonic_.end())
                {
                    b.tonic=fi->second;
                    b.bits|=BTONIC;
                }
            }


            if(!(b.bits&BBASE))
            {
                fi=base_.find(d);
                if(fi!=base_.end())
                {
                    b.base=fi->second;
                    b.bits|=BBASE;
                }
            }


            if(!(b.bits&BOCT))
            {
                fi=oct_.find(d);
                if(fi!=oct_.end())
                {
                    b.oct=fi->second;
                    b.bits|=BOCT;
                }
            }


            if(!(b.bits&BSCALE))
            {
                si=scale_.find(d);
                if(si!=scale_.end())
                {
                    b.scale=si->second;
                    b.bits|=BSCALE;
                }
            }


            if(!(b.bits&BLAYOUT))
            {
                li=layout_.find(d);
                if(li!=layout_.end())
                {
                    b.layout=li->second;
                    b.bits|=BLAYOUT;
                }
            }

            if(b.bits!=0)
            {
                break;
            }
        }

        return b;
    }

    pic::lckmap_t<piw::data_nb_t,float,piw::path_less>::nbtype tonic_;
    pic::lckmap_t<piw::data_nb_t,float,piw::path_less>::nbtype base_;
    pic::lckmap_t<piw::data_nb_t,float,piw::path_less>::nbtype oct_;
    pic::lckmap_t<piw::data_nb_t,sref_t,piw::path_less>::nbtype scale_;
    pic::lckmap_t<piw::data_nb_t,lref_t,piw::path_less>::nbtype layout_;
    pic::lckmap_t<piw::data_nb_t,ctlfunc_t*,piw::path_less>::nbtype func_;

    pic::lckvector_t<sref_t>::nbtype common_scales_;
    pic::lcklist_t<scaler_subscriber_t *>::nbtype subscribers_;
};

namespace
{
    struct ctlfunc_t: piw::ufilterfunc_t
    {
        ctlfunc_t(piw::scaler_controller_t::impl_t *i): impl_(i), env_(0)
        {
        }

        void ufilterfunc_start(piw::ufilterenv_t *e,const piw::data_nb_t &id)
        {
            PIC_ASSERT(id.is_path());

            id_ = id;
            env_ = e;

            impl_->func_.insert(std::make_pair(id_,this));
            e->ufilterenv_start(id.time());
        }

        void ufilterfunc_data(piw::ufilterenv_t *e,unsigned sig,const piw::data_nb_t &d)
        {
            if(sig!=1) return;
            if(!d.is_dict()) return;

            PIC_ASSERT(env_ == e);

            pic::lckmap_t<piw::data_nb_t,float,piw::path_less>::nbtype::iterator ti;
            pic::lckmap_t<piw::data_nb_t,float,piw::path_less>::nbtype::iterator bi;
            pic::lckmap_t<piw::data_nb_t,float,piw::path_less>::nbtype::iterator oi;
            pic::lckmap_t<piw::data_nb_t,piw::scaler_controller_t::sref_t,piw::path_less>::nbtype::iterator si;
            pic::lckmap_t<piw::data_nb_t,piw::scaler_controller_t::lref_t,piw::path_less>::nbtype::iterator li;

            piw::data_nb_t t,b,o,s,co,cl;

            ti = impl_->tonic_.find(id_);
            if(ti!=impl_->tonic_.end()) impl_->tonic_.erase(ti);
            t = d.as_dict_lookup("tonic");
            if(!t.is_null()) impl_->tonic_.insert(std::make_pair(id_,t.as_renorm_float(0,12,0)));

            bi = impl_->base_.find(id_);
            if(bi!=impl_->base_.end()) impl_->base_.erase(bi);
            b = d.as_dict_lookup("base");
            if(!b.is_null()) impl_->base_.insert(std::make_pair(id_,b.as_renorm_float(-20,20,0)));

            oi = impl_->oct_.find(id_);
            if(oi!=impl_->oct_.end()) impl_->oct_.erase(oi);
            o = d.as_dict_lookup("octave");
            if(!o.is_null()) impl_->oct_.insert(std::make_pair(id_,o.as_renorm_float(-1,9,0)));

            si = impl_->scale_.find(id_);
            if(si!=impl_->scale_.end()) impl_->scale_.erase(si);
            s = d.as_dict_lookup("scale");
            if(s.is_string()) impl_->scale_.insert(std::make_pair(id_,pic::ref(new piw::scaler_controller_t::scale_t(s.as_string()))));

            li = impl_->layout_.find(id_);
            if(li!=impl_->layout_.end()) impl_->layout_.erase(li);
            co = d.as_dict_lookup("courseoffset");
            cl = d.as_dict_lookup("courselen");
            if(co.is_tuple() && cl.is_tuple()) impl_->layout_.insert(std::make_pair(id_,pic::ref(new piw::scaler_controller_t::layout_t(co,cl))));

            impl_->control_changed(id_);
        }

        void ufilterfunc_end(piw::ufilterenv_t *, unsigned long long)
        {
            env_ = 0;

            pic::lckmap_t<piw::data_nb_t,float,piw::path_less>::nbtype::iterator ti;
            pic::lckmap_t<piw::data_nb_t,float,piw::path_less>::nbtype::iterator bi;
            pic::lckmap_t<piw::data_nb_t,float,piw::path_less>::nbtype::iterator oi;
            pic::lckmap_t<piw::data_nb_t,piw::scaler_controller_t::sref_t,piw::path_less>::nbtype::iterator si;
            pic::lckmap_t<piw::data_nb_t,piw::scaler_controller_t::lref_t,piw::path_less>::nbtype::iterator li;
            pic::lckmap_t<piw::data_nb_t,ctlfunc_t*,piw::path_less>::nbtype::iterator fi;

            ti = impl_->tonic_.find(id_);
            if(ti!=impl_->tonic_.end()) impl_->tonic_.erase(ti);

            bi = impl_->base_.find(id_);
            if(bi!=impl_->base_.end()) impl_->base_.erase(bi);

            oi = impl_->oct_.find(id_);
            if(oi!=impl_->oct_.end()) impl_->oct_.erase(oi);

            si = impl_->scale_.find(id_);
            if(si!=impl_->scale_.end()) impl_->scale_.erase(si);

            li = impl_->layout_.find(id_);
            if(li!=impl_->layout_.end()) impl_->layout_.erase(li);

            fi = impl_->func_.find(id_);
            if(fi!=impl_->func_.end()) impl_->func_.erase(fi);

            id_ = piw::data_nb_t();
        }

        void update_lights(piw::scaler_controller_t::sref_t scale, float mode, bool override)
        {
            if(!env_) return;

            piw::scaler_controller_t::bits_t b(impl_->bits(id_));
            if(b.bits&BLAYOUT)
            {
                piw::scaler_controller_t::lref_t l=b.layout;
                piw::scaler_controller_t::sref_t s=scale;
                float m=mode;

                if(!override)
                {
                    if(b.bits&BSCALE) s=b.scale;
                    if(b.bits&BBASE)  m=b.base;
                }

                if(l.isvalid() && s.isvalid())
                {
                    unsigned scale_size = s->size()-1;

                    pic::lckmap_t<piw::statusdata_t,unsigned char>::nbtype status;

                    if(scale_size > 0)
                    {
                        float key_offset = 0.0;
                        float note_offset = 0.0;

                        for(unsigned c=0; c<b.layout->courses(); c++)
                        {
                            for(int k=0; k<int(b.layout->length(c)); k++)
                            {
                                l->offsets(c,key_offset,note_offset);
                                if(0 == abs(k+int(key_offset+m))%scale_size)
                                {
                                    piw::statusdata_t statusdata = piw::statusdata_t(true,c+1,k+1);
                                    status.insert(std::make_pair(statusdata,1));
                                }
                            }
                        }
                    }

                    env_->ufilterenv_output(1,piw::statusbuffer_t::make_statusbuffer(status));
                }
            }
        }

        piw::data_nb_t id_;
        piw::scaler_controller_t::impl_t *impl_;
        piw::ufilterenv_t *env_;
    };
}

piw::ufilterfunc_t *piw::scaler_controller_t::impl_t::ufilterctl_create(const piw::data_t &)
{
    return new ctlfunc_t(this);
}

void piw::scaler_controller_t::impl_t::ufilterctl_delete(ufilterfunc_t *f)
{
    if(f)
    {
        delete f;
    }
}

void piw::scaler_controller_t::impl_t::update_lights(const piw::data_nb_t &id, piw::scaler_controller_t::sref_t scale, float mode, bool override)
{
    const unsigned char *p = id.as_path();
    int l = id.as_pathlen();

    for(;l>=0;l--)
    {
        piw::data_nb_t d  = piw::makepath_nb(p,l);

        pic::lckmap_t<piw::data_nb_t,ctlfunc_t*,piw::path_less>::nbtype::iterator fi;
        fi=func_.find(d);
        if(fi!=func_.end())
        {
            fi->second->update_lights(scale,mode,override);
            return;
        }
    }
}

piw::scaler_controller_t::scaler_controller_t(const cookie_t &l): impl_(new impl_t(l))
{
}

piw::scaler_controller_t::~scaler_controller_t()
{
    delete impl_;
}

piw::cookie_t piw::scaler_controller_t::cookie()
{
    return impl_->cookie();
}

void piw::scaler_controller_t::impl_t::control_changed(const piw::data_nb_t &id)
{
    pic::lcklist_t<scaler_subscriber_t *>::nbtype::iterator i;

    for(i=subscribers_.begin(); i!=subscribers_.end(); i++)
    {
        (*i)->control_changed(id);
    }
}


/**
 * scaler_t
 */

struct piw::scaler_t::impl_t: piw::scaler_subscriber_t, piw::ufilterctl_t, piw::ufilter_t, pic::lckobject_t
{
    impl_t(piw::scaler_controller_t *sc,const piw::cookie_t &c,const pic::f2f_t &b);

    ufilterfunc_t *ufilterctl_create(const piw::data_t &);
    virtual void ufilterctl_delete(ufilterfunc_t *f);

    unsigned long long ufilterctl_inputs()
    {
        return SCALER_IN_MASK;
    }

    unsigned long long ufilterctl_outputs()
    {
        return SCALER_OUT_MASK;
    }

    unsigned long long ufilterctl_thru()
    {
        return ~ufilterctl_outputs();
    }

    piw::scaler_controller_t *controller_;
    pic::flipflop_functor_t<pic::f2f_t> bend_;
};

namespace
{
    struct sfunc_t : piw::scaler_subscriber_t, piw::ufilterfunc_t
    {
        sfunc_t(piw::scaler_t::impl_t *p) : parent_(p), time_(0)
        {
            krange_=1;
            grange_=12;
            tonic_=0;
            octave_=3;
            roctave_=0;
            mode_=0;
            scale_=parent_->controller_->common_scale_at(0);
            override_=false;
        }

        ~sfunc_t()
        {
            scale_data_.clear();
        }

        void setmodifier(const piw::data_nb_t &v)
        {
            modifier_=v.as_renorm_float(BCTUNIT_STEPS,-10,10,0);
            time_ = std::max(time_,v.time());
            //pic::logmsg() << "kbend = " << kbend_ << " time " << time_;
        }

        void setkbend(const piw::data_nb_t &v)
        {
            kbend_=v.as_renorm_float(-1,1,0);
            time_ = std::max(time_,v.time());
            //pic::logmsg() << "kbend = " << kbend_ << " time " << time_;
        }

        void setgbend(const piw::data_nb_t &v)
        {
            gbend_=v.as_renorm_float(-1,1,0);
            time_ = std::max(time_,v.time());
        }

        void setkrange(const piw::data_nb_t &v)
        {
            krange_=v.as_renorm_float(0,72,0);
            time_ = std::max(time_,v.time());
        }

        void setgrange(const piw::data_nb_t &v)
        {
            grange_=v.as_renorm_float(0,72,0);
            time_ = std::max(time_,v.time());
        }

        void setkey(const piw::data_nb_t &v)
        {
            float course,key;
            if(piw::decode_key(v,0,0,0,0,&course,&key))
            {
                keycourse_ = course-1.0;
                keynum_ = key-1.0;
                time_ = std::max(time_,v.time());
            }
        }

        void ufilterfunc_start(piw::ufilterenv_t *e,const piw::data_nb_t &id)
        {
            //e->ufilterenv_dump(false);
            id_=id;
            time_ = id.time();

#if SCALER_DEBUG>0
            pic::logmsg() << "------- scaler start " << time_;
#endif // SCALER_DEBUG>0

            keynum_=-1.0;
            keycourse_=0.0;

            piw::data_nb_t d;
            if(e->ufilterenv_latest(SCALER_KEY,d,time_))
            {
                float course,key;
                if(piw::decode_key(d,0,0,0,0,&course,&key))
                {
                    keycourse_=course-1.0;
                    keynum_=key-1.0;
                }
            }

            kbend_=0;
            gbend_=0;
            modifier_=0;
            override_=false;

            if(e->ufilterenv_latest(SCALER_OVERRIDE,d,time_)) setoverride(d);
            if(e->ufilterenv_latest(SCALER_TONIC,d,time_)) settonic(d);
            if(e->ufilterenv_latest(SCALER_BASE,d,time_)) setmode(d);
            if(e->ufilterenv_latest(SCALER_SCALE,d,time_)) setscale(d);
            if(e->ufilterenv_latest(SCALER_OCTAVE,d,time_)) setoctave(d);
            if(e->ufilterenv_latest(SCALER_ROCTAVE,d,time_)) setroctave(d);
            if(e->ufilterenv_latest(SCALER_KBEND,d,time_)) setkbend(d);
            if(e->ufilterenv_latest(SCALER_GBEND,d,time_)) setgbend(d);
            if(e->ufilterenv_latest(SCALER_KRANGE,d,time_)) setkrange(d);
            if(e->ufilterenv_latest(SCALER_GRANGE,d,time_)) setgrange(d);
            if(e->ufilterenv_latest(SCALER_MODIFIER,d,time_)) setmodifier(d);

#if SCALER_DEBUG>0
            pic::logmsg() << "scaler start key: " << keynum_ << ':' << keycourse_;
#endif // SCALER_DEBUG>0

            recalculate_note();
            recalculate_gbend();
            recalculate_kbend();
            recalculate_modifier();

            time_ = id.time();
            e->ufilterenv_start(time_);

            send_output(e);
        }

        void ufilterfunc_end(piw::ufilterenv_t *, unsigned long long t)
        {
#if SCALER_DEBUG>0
            pic::logmsg() << "scaler end " << time_;
#endif // SCALER_DEBUG>0
        }

        void settonic(const piw::data_nb_t &d)
        {
            tonic_ = d.as_renorm_float(0,12,0);
            time_ = std::max(time_,d.time());
        }

        void setmode(const piw::data_nb_t &d)
        {
            float old_mode = mode_;
            mode_ = d.as_renorm_float(-20,20,0);
            time_ = std::max(time_,d.time());
            if(old_mode != mode_)
            {
                update_lights(id_);
            }
        }

        void setscale(const piw::data_nb_t &d)
        {
            if(scale_data_.is_empty() || scale_data_.get().compare(d) != 0)
            {
                scale_data_.set_nb(d);

                if(d.is_string())
                {
                    scale_ = pic::ref(new piw::scaler_controller_t::scale_t(d.as_string()));
                }
                else if(!d.is_null())
                {
                    unsigned n = (unsigned)(fabsf(d.as_norm())*parent_->controller_->common_scale_count());
                    scale_ = parent_->controller_->common_scale_at(n);
                }

                time_ = std::max(time_,d.time());
                update_lights(id_);
            }
        }

        void setroctave(const piw::data_nb_t &d)
        {
            roctave_ = d.as_renorm_float(-10,10,0);
            time_ = std::max(time_,d.time());
        }

        void setoctave(const piw::data_nb_t &d)
        {
            octave_ = d.as_renorm_float(-1,9,0);
            time_ = std::max(time_,d.time());
        }

        void setoverride(const piw::data_nb_t &d)
        {
            bool old_override = override_;
            override_=(d.as_norm()!=0);
            time_ = std::max(time_,d.time());
            if(old_override != override_)
            {
                update_lights(id_);
            }
        }

        void ufilterfunc_data(piw::ufilterenv_t *env,unsigned sig,const piw::data_nb_t &d)
        {
            // pic::logmsg() << "sfunc_t::ufilterfunc_data sig=" << sig << " value=" << d << " time=" << d.time();

            bool nc=false;
            bool gc=false;
            bool kc=false;
            bool mc=false;

            switch(sig)
            {
                case SCALER_KEY: nc=true; setkey(d); break;
                case SCALER_OVERRIDE: nc=true;  setoverride(d); break;
                case SCALER_GBEND: gc=true; setgbend(d); break;
                case SCALER_GRANGE: gc=true; setgrange(d); break;
                case SCALER_KBEND: kc=true; setkbend(d); break;
                case SCALER_KRANGE: kc=true; setkrange(d); break;
                case SCALER_MODIFIER: mc=true; setmodifier(d); break;
            }

            if(nc)
            {
                recalculate_note();
            }

            if(mc)
            {
                recalculate_modifier();
            }

            if(gc)
            {
                recalculate_gbend();
            }

            if(kc)
            {
                recalculate_kbend();
            }

            send_output(env);
        }

        void control_changed(const piw::data_nb_t &id)
        {
            update_lights(id);
        }

        void update_lights(const piw::data_nb_t &id)
        {
            parent_->controller_->update_lights(id, scale_, mode_, override_);
        }

        void recalculate_note()
        {
            float t=tonic_,o=octave_,m=mode_;

            piw::scaler_controller_t::sref_t s=scale_;
            piw::scaler_controller_t::bits_t b(parent_->controller_->bits(id_));
            piw::scaler_controller_t::lref_t l;

            if(b.bits&BLAYOUT)
            {
                l=b.layout;
            }

            if(!override_)
            {
                if(b.bits&BTONIC) t=b.tonic;
                if(b.bits&BOCT)   o=b.oct;
                if(b.bits&BBASE)  m=b.base;
                if(b.bits&BSCALE) s=b.scale;
            }

            float key_offset = 0.0;
            float note_offset = 0.0;

            PIC_ASSERT(s.isvalid());

            if(l.isvalid())
            {
                l->offsets(keycourse_,key_offset,note_offset);
            }

            key_offset += (keynum_+m);


            int scale_size = s->size()-1;
            float ntave_size = s->at(scale_size);

            int ntave = 0;
            float offset = 0.0;

            if(key_offset>=0.0)
            {
                ntave = (int)(key_offset/scale_size);
                offset = key_offset - (ntave*scale_size);
            }
            else
            {
                ntave = (int)(key_offset/scale_size)-1;
                offset = key_offset - (ntave*scale_size);
            }

            //pic::logmsg() << "k=" << key_offset << " c=" << keycourse_ << " no=" << note_offset << " ss=" << scale_size << " sr=" << ntave_size << " tave=" << ntave << " offset=" << offset;

            note_ = note_offset+t+12.0*(roctave_+o+1.0)+ntave_size*ntave+s->interpolate(offset);

            if(note_<0.0)
            {
                note_=0.0;
            }

            note_hz_ = 440.0*powf(2.0,(note_-69.0)/12.0);
            dirty_ = true;
        }

        void send_output(piw::ufilterenv_t *env)
        {
            if(dirty_ && note_ >= 0 && note_hz_ > 0)
            {
                dirty_ = false;
                float bend = mbend_note_+kbend_note_+gbend_note_;
                float note = note_+bend;
                env->ufilterenv_output(SCALER_SCALENOTE,piw::makefloat_bounded_nb(136,16,16,note,time_));

                float hz = note_hz_;
                if(bend!=0.0)
                {
                    hz *= pic::approx::pow2(bend/12.0);
                }

                env->ufilterenv_output(SCALER_FREQUENCY,piw::makefloat_bounded_units_nb(BCTUNIT_HZ,96000,0,0,hz,time_));
            }
        }

        void recalculate_kbend()
        {
            float kbend_note = parent_->bend_(kbend_)*krange_;
            if (kbend_note_ == kbend_note) return;
            kbend_note_ = kbend_note;
            dirty_ = true;
        }

        void recalculate_gbend()
        {
            float gbend_note = grange_*gbend_;
            if (gbend_note_ == gbend_note) return;
            gbend_note_ = gbend_note;
            dirty_ = true;
        }

        void recalculate_modifier()
        {
            float mbend_note = modifier_;
            if (mbend_note_ == mbend_note) return;
            mbend_note_ = mbend_note;
            dirty_ = true;
        }

        float tonic_;
        float octave_;
        float roctave_;
        float mode_;
        piw::scaler_controller_t::sref_t scale_;
        piw::dataholder_nb_t scale_data_;
        bool override_;

        float krange_,grange_;

        float keynum_;
        float keycourse_;

        float modifier_;
        float kbend_;
        float gbend_;
        float note_;
        float note_hz_;
        float kbend_note_;
        float gbend_note_;
        float mbend_note_;

        bool dirty_;
        piw::data_nb_t id_;

        piw::scaler_t::impl_t *parent_;
        unsigned long long time_;
    };
}

piw::ufilterfunc_t *piw::scaler_t::impl_t::ufilterctl_create(const piw::data_t &)
{
    sfunc_t *s = new sfunc_t(this);
    controller_->add_subscriber(s);
    return s;
}

void piw::scaler_t::impl_t::ufilterctl_delete(ufilterfunc_t *f)
{
    if(f)
    {
        controller_->del_subscriber((scaler_subscriber_t *)f);
        delete f;
    }
}

piw::scaler_t::impl_t::impl_t(piw::scaler_controller_t *sc,const piw::cookie_t &c,const pic::f2f_t &b): ufilter_t(this,c), controller_(sc), bend_(b)
{
}

void piw::scaler_t::set_bend_curve(const pic::f2f_t &b)
{
    ctl_->bend_=b;
}

piw::scaler_t::scaler_t(piw::scaler_controller_t *sc,const piw::cookie_t &c,const pic::f2f_t &b): ctl_(new impl_t(sc,c,b))
{
}

piw::scaler_t::~scaler_t()
{
    delete ctl_;
}


int piw::scaler_t::gc_traverse(void *v,void *a) const { return ctl_->bend_.gc_traverse(v,a); }
int piw::scaler_t::gc_clear() { return ctl_->bend_.gc_clear(); }
piw::cookie_t piw::scaler_t::cookie() { return ctl_->cookie(); }

void piw::scaler_controller_t::add_subscriber(scaler_subscriber_t *s) { impl_->add_subscriber(s); }
void piw::scaler_controller_t::del_subscriber(scaler_subscriber_t *s) { impl_->del_subscriber(s); }

unsigned piw::scaler_controller_t::common_scale_count() { return impl_->common_scales_.size(); }
piw::scaler_controller_t::sref_t piw::scaler_controller_t::common_scale_at(unsigned i) { return impl_->common_scales_.at(i); }
piw::scaler_controller_t::bits_t piw::scaler_controller_t::bits(const piw::data_nb_t &id) { return impl_->bits(id); }
void piw::scaler_controller_t::update_lights(const piw::data_nb_t &id, piw::scaler_controller_t::sref_t scale, float mode, bool override) { impl_->update_lights(id, scale, mode, override); }
