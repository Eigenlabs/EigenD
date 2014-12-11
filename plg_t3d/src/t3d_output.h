
#ifndef __T3D_OUTPUT__
#define __T3D_OUTPUT__

#include "t3d_plg_exports.h"

#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_clock.h>

namespace t3d_output_plg
{
    class T3D_PLG_DECLSPEC_CLASS t3d_output_t : public pic::nocopy_t // cant copy this
    {
        public:
            t3d_output_t(piw::clockdomain_ctl_t *, const std::string &a, unsigned p);
            ~t3d_output_t();

            void stop();
            piw::change_nb_t control();

            piw::cookie_t create_output(const std::string &prefix, bool fake_key, unsigned signals);
            void connect(const std::string &a, unsigned p);
            void set_max_voice_count(unsigned);
            void set_data_freq(unsigned);
            void set_kyma_mode(bool);
            void set_continuous_mode(bool);

            class impl_t;
            
        private:
            impl_t *impl_;

	};
}

#endif

