#ifndef __T3D_DEVICE__
#define __T3D_DEVICE__

#include "t3d_plg_exports.h"

#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_clock.h>

namespace t3d_device_plg
{

    class T3D_PLG_DECLSPEC_CLASS t3d_device_t : public pic::nocopy_t
    {
        public:
    		t3d_device_t(piw::clockdomain_ctl_t *, const piw::cookie_t &outputs);
            ~t3d_device_t();

            void stop();

            void connect(unsigned,unsigned);

            piw::change_nb_t control();
            piw::cookie_t cookie();
            void col_size(int);
            void row_size(int);
            void whole_roll(bool);
            void whole_yaw(bool);
            void continuous_key(bool);
            void touch_mode(bool);

            piw::data_t get_columnlen();
            piw::data_t get_columnoffset();
            piw::data_t get_courselen();
            piw::data_t get_courseoffset();


            class impl_t;
         private:
            impl_t *impl_;
    };
}

#endif

