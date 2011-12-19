
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

#include <piw/piw_function1.h>
#include <piw/piw_ufilter.h>
#include <picross/pic_log.h>

namespace
{
    class func_t: public piw::ufilterfunc_t
    {
        public:
            func_t(const piw::d2d_nb_t &f, const piw::data_t &initial,unsigned is, unsigned os, bool thru): functor_(f), initial_(initial), insig_(is), outsig_(os)
            {
            }

            void ufilterfunc_changed(piw::ufilterenv_t *e, void *f_)
            {
                functor_=*(piw::d2d_nb_t *)f_;
                recalc(e);
            }

            piw::data_nb_t getoutput()
            {
                if(functor_.iscallable())
                {
                    return functor_(input_);
                }

                return piw::data_nb_t();
            }


            void ufilterfunc_start(piw::ufilterenv_t *env,const piw::data_nb_t &id)
            {
                unsigned long long t=id.time();
                input_=initial_;
                output_.set_nb(piw::data_nb_t());

                piw::data_nb_t d;

                if(env->ufilterenv_latest(insig_,d,t))
                {
                    input_.set_nb(d);
                }

                recalc(env);
                env->ufilterenv_start(t);
            }

            void ufilterfunc_data(piw::ufilterenv_t *env,unsigned sig,const piw::data_nb_t &d)
            {
                PIC_ASSERT(sig==insig_);
                input_.set_nb(d);
                recalc(env);
            }

            void recalc(piw::ufilterenv_t *env)
            {
                piw::data_nb_t o(getoutput());
                if(o!=output_.get())
                {
                    output_.set_nb(o);
                    env->ufilterenv_output(outsig_,o);
                }
            }

        private:
            piw::d2d_nb_t functor_;
            piw::dataholder_nb_t input_;
            piw::dataholder_nb_t initial_;
            unsigned insig_;
            unsigned outsig_;
            piw::dataholder_nb_t output_;
    };
};

struct piw::function1_t::impl_t: public piw::ufilterctl_t
{
    impl_t(bool thru, unsigned input, unsigned output, const piw::data_t &initial, const cookie_t &c): output_(output), input_(input), thru_(thru), initial_(initial), filter_(this,c)
    {
    }

    virtual ~impl_t()
    {
    }

    ufilterfunc_t *ufilterctl_create(const piw::data_t &)
    {
        return new func_t(functor_,initial_,input_,output_,thru_);
    }

    void set_functor(const piw::d2d_nb_t &f)
    {
        functor_=f;
        filter_.changed(&functor_);
    }

    unsigned long long ufilterctl_inputs()
    {
        return SIG1(input_);
    }

    unsigned long long ufilterctl_outputs()
    {
        return SIG1(output_);
    }

    unsigned long long ufilterctl_thru()
    {
        return thru_?~ufilterctl_outputs():0;
    }

    void clear_functor()
    {
        functor_.clear();
        filter_.changed(&functor_);
    }

    unsigned output_;
    unsigned input_;
    bool thru_;
    d2d_nb_t functor_;
    piw::data_t initial_;
    piw::ufilter_t filter_;
};


piw::function1_t::function1_t(bool thru, unsigned input, unsigned output, const piw::data_t &initial, const cookie_t &o): impl_(new impl_t(thru,input,output,initial,o))
{
}

piw::cookie_t piw::function1_t::cookie()
{
    return impl_->filter_.cookie();
}

piw::function1_t::~function1_t()
{
    delete impl_;
}

void piw::function1_t::set_functor(const d2d_nb_t &f)
{
    impl_->set_functor(f);
}

int piw::function1_t::gc_traverse(void *v, void *a) const
{
    return impl_->functor_.gc_traverse(v,a);
}

int piw::function1_t::gc_clear()
{
    impl_->clear_functor();
    return 0;
}
