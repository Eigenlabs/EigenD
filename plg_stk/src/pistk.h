
#ifndef __STK__
#define __STK__

#include <piw/piw_clock.h>
#include <piw/piw_bundle.h>
#include "Cello.h"


namespace stk
{
    enum stk_run_state
    {
        stk_run = 0,
        stk_linger = 1,
        stk_stop = 2
    };


    class blownstring_t
    {
        public:
            blownstring_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~blownstring_t();
            piw::cookie_t cookie();

            class impl_t;

        private:
            impl_t *impl_;
    };

    class panpipe_t
    {
        public:
            panpipe_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~panpipe_t();
            piw::cookie_t cookie();

            class impl_t;

        private:
            impl_t *impl_;
    };

    class clarinet_t
    {
        public:
            clarinet_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~clarinet_t();
            piw::cookie_t cookie();

            class impl_t;

        private:
            impl_t *impl_;
    };

    class cello_t
    {
        public:
            cello_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d);
            ~cello_t();
            piw::cookie_t cookie();
            piw::cookie_t bow_cookie();

#if CELLO_TESTVALUE==1
            void set_param_num(unsigned num);
            void set_param_val(float val);
#endif // CELLO_TESTVALUE==1

            class impl_t;

        private:
            impl_t *impl_;
    };

}

#endif

