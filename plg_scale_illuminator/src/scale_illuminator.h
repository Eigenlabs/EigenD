
#ifndef __SCALE_ILLUMINATOR_INPUT__
#define __SCALE_ILLUMINATOR_INPUT__

#include "scale_illuminator_plg_exports.h"

#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_clock.h>

namespace scale_illuminator_plg
{

    class SCALE_ILLUMINATOR_PLG_DECLSPEC_CLASS scale_illuminator_t : public pic::nocopy_t
    {
        public:
    		scale_illuminator_t(piw::clockdomain_ctl_t *, const piw::cookie_t &output);
            ~scale_illuminator_t();
            piw::cookie_t cookie();
            void reference_scale(const std::string &);
            void reference_tonic(float);
            void inverted(bool v);
            void root_light(bool v);

            class impl_t;
         private:
            impl_t *impl_;
    };
}

#endif

