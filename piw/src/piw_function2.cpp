
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

#include <piw/piw_ufilter.h>
#include <piw/piw_function2.h>
#include <picross/pic_log.h>

namespace
{
    struct func_t: public piw::ufilterfunc_t
    {
        func_t(const piw::dd2d_nb_t &f, const piw::data_t &i1, const piw::data_t &i2, unsigned is1, unsigned is2, unsigned os,bool thru):
            _functor(f), initial1_(i1), initial2_(i2), insig1_(is1), insig2_(is2), outsig_(os)
        {
        }

        void ufilterfunc_changed(piw::ufilterenv_t *e, void *f_)
        {
            _functor=*(piw::dd2d_nb_t *)f_;
            recalc(e);
        }

        piw::data_nb_t getoutput()
        {
            if(_functor.iscallable())
            {
                piw::data_nb_t d = _functor(input1_,input2_);
                //pic::logmsg() << input1_ << " : " << input2_ << " -> " << d << '@' << d.time();
                return d;
            }

            return piw::data_nb_t();
        }

        void ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id)
        {
            input1_=initial1_;
            input2_=initial2_;
            output_=piw::data_nb_t();

            unsigned long long t=id.time();
            piw::data_nb_t d;

            if(env->ufilterenv_latest(insig1_,d,t))
                input1_=d;

            if(env->ufilterenv_latest(insig2_,d,t))
                input2_=d;

            recalc(env);
            env->ufilterenv_start(t);
        }

        void ufilterfunc_data(piw::ufilterenv_t *env,unsigned sig,const piw::data_nb_t &d)
        {
            //pic::logmsg() << "ufilter data " << sig << ":" << d;
            if(sig==insig1_)
                input1_=d;

            if(sig==insig2_)
                input2_=d;

            recalc(env);
        }

        void recalc(piw::ufilterenv_t *env)
        {
            piw::data_nb_t o = getoutput();
            if(o!=output_)
            {
                output_=o;
                env->ufilterenv_output(outsig_,o);
            }
        }

        piw::dd2d_nb_t _functor;
        piw::dataholder_nb_t initial1_;
        piw::dataholder_nb_t initial2_;
        piw::data_nb_t input1_;
        piw::data_nb_t input2_;
        unsigned insig1_;
        unsigned insig2_;
        unsigned outsig_;
        piw::data_nb_t output_;
    };

    struct add_t
    {
        add_t(float f): fact_(f) {}
        add_t(const add_t &o): fact_(o.fact_) {}
        bool operator==(const add_t &o) const { return fact_==o.fact_; }
        piw::data_nb_t operator()(const piw::data_nb_t &a1, const piw::data_nb_t &a2) const
        {
            float r1 = a1.as_denorm_float();
            float r2 = fact_*a2.as_denorm_float();
            unsigned long long t = std::max(a1.time(),a2.time());
            return piw::makefloat_bounded_units_nb(a1.units(),a1.as_array_ubound(),a1.as_array_lbound(),a1.as_array_rest(),r1+r2,t);
        }

        float fact_;
    };
};

struct piw::function2_t::impl_t: public piw::ufilterctl_t
{
    impl_t(bool thru, unsigned input1, unsigned input2, unsigned output,const piw::data_t &i1, const piw::data_t &i2, const piw::cookie_t &c):
        _output(output), _input1(input1), _input2(input2), _thru(thru), initial1_(i1), initial2_(i2), filter_(this,c)
    {
    }

    virtual ~impl_t()
    {
    }

    ufilterfunc_t *ufilterctl_create(const piw::data_t &)
    {
        return new func_t(_functor,initial1_,initial2_,_input1,_input2,_output,_thru);
    }

    void set_functor(const piw::dd2d_nb_t &f)
    {
        _functor=f;
        filter_.changed(&_functor);
    }

    unsigned long long ufilterctl_inputs()
    {
        return SIG2(_input1,_input2);
    }

    unsigned long long ufilterctl_outputs()
    {
        return SIG1(_output);
    }

    unsigned long long ufilterctl_thru()
    {
        return _thru?~ufilterctl_outputs():0;
    }

    void clear_functor()
    {
        _functor.clear();
        filter_.changed(&_functor);
    }

    unsigned _output;
    unsigned _input1;
    unsigned _input2;
    bool _thru;
    piw::dd2d_nb_t _functor;
    piw::data_t initial1_;
    piw::data_t initial2_;

    piw::ufilter_t filter_;
};

piw::function2_t::function2_t(bool thru, unsigned input1, unsigned input2, unsigned output, const piw::data_t &i1, const piw::data_t &i2,const cookie_t &o): impl_(new impl_t(thru,input1,input2,output,i1,i2,o))
{
}

piw::cookie_t piw::function2_t::cookie()
{
    return impl_->filter_.cookie();
}

piw::function2_t::~function2_t()
{
    delete impl_;
}

void piw::function2_t::set_functor(const piw::dd2d_nb_t &f)
{
    impl_->set_functor(f);
}

piw::dd2d_nb_t piw::add(float fact)
{
    return piw::dd2d_nb_t::callable(add_t(fact));
}

int piw::function2_t::gc_traverse(void *v, void *a) const
{
    return impl_->_functor.gc_traverse(v,a);
}

int piw::function2_t::gc_clear()
{
    impl_->clear_functor();
    return 0;
}
