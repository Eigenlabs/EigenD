
#ifndef __MIDI_MONITOR_INPUT__
#define __MIDI_MONITOR_INPUT__

#include "midi_monitor_plg_exports.h"

#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_clock.h>

namespace midi_monitor_plg
{

    class MIDI_MONITOR_PLG_DECLSPEC_CLASS midi_monitor_t : public pic::nocopy_t
    {
        public:
    		midi_monitor_t(piw::clockdomain_ctl_t *, const piw::cookie_t &output);
            ~midi_monitor_t();
            piw::change_nb_t control();
            piw::cookie_t cookie();
            void enable_notes(bool);
            void enable_poly_pressure(bool);
            void enable_cc_as_key(bool);
            void enable_cc_as_course(bool);
            void channel(unsigned);
            void nearest_match(bool);
            void first_match(bool);
            void use_velocity_as_state(bool);
            void use_channel_as_state(bool);
            void use_physical_mapping(bool);
            void control_offset(unsigned);

            class impl_t;
         private:
            impl_t *impl_;
    };
}

#endif

