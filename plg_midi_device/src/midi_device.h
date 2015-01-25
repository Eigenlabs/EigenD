
#ifndef __MIDI_DEVICE_INPUT__
#define __MIDI_DEVICE_INPUT__

#include "midi_device_plg_exports.h"

#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_clock.h>

namespace midi_device_plg
{

    class MIDI_DEVICE_PLG_DECLSPEC_CLASS midi_device_t : public pic::nocopy_t
    {
        public:
    		midi_device_t(piw::clockdomain_ctl_t *, const piw::cookie_t &outputs);
    		virtual ~midi_device_t();
            piw::change_nb_t control();
            piw::cookie_t cookie();
            void enable_hires_cc(bool);
            void enable_notes(bool);
            void enable_velocity(bool);
            void enable_poly_at(unsigned,bool);
            void enable_chan_at(unsigned,bool);
            void set_data_freq(int);
            void set_cc_map(unsigned,int);
            void set_pb_map(unsigned,int);
            void set_relative_map(unsigned,bool);
            void set_release_map(unsigned,bool);
            void set_bipolar_map(unsigned,bool);
            void set_note_cc(int);
            void set_note_cc_pressure(int);
            void channel(unsigned);
            void velocity_sample(unsigned);

            void set_hires_cc_map(unsigned,int);
            void set_hires_pb_map(unsigned,int);

            void enable_control_notes(bool);
            void set_colour(unsigned,unsigned);


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

