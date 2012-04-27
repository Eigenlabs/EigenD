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

#ifndef __KBD_BUNDLE2__
#define __KBD_BUNDLE2__

#include <piw/piw_bundle.h>
#include <picross/pic_functor.h>
#include <picross/pic_usb.h>

#include "pikeyboardplg_exports.h"


namespace kbd
{
    class kbd_impl_t;

    class PIKEYBOARDPLG_DECLSPEC_CLASS alpha2_bundle_legacy_t: public pic::nocopy_t
    {
        public:
            alpha2_bundle_legacy_t(const char *usb,const piw::cookie_t &kc,const pic::notify_t &dead);
            ~alpha2_bundle_legacy_t();

            std::string name();
            piw::data_t get_columnlen();
            piw::data_t get_columnoffset();
            piw::data_t get_courselen();
            piw::data_t get_courseoffset();
            float get_threshold1();
            float get_threshold2();
            void set_threshold1(float);
            void set_threshold2(float);
            float get_roll_axis_window();
            float get_yaw_axis_window();
            void set_roll_axis_window(float);
            void set_yaw_axis_window(float);
            piw::change_nb_t led_functor();
            void close();

            void learn_pedal_min(unsigned pedal);
            void learn_pedal_max(unsigned pedal);
            unsigned get_pedal_min(unsigned pedal);
            unsigned get_pedal_max(unsigned pedal);
            void set_pedal_min(unsigned pedal,unsigned val);
            void set_pedal_max(unsigned pedal,unsigned val);

            void testmsg_write_lib(unsigned,unsigned,unsigned,unsigned);
            void testmsg_finish_lib();
            void testmsg_write_seq(unsigned,unsigned,unsigned);
            void testmsg_finish_seq();
            void start_test(unsigned duration);
            void arm_recording(unsigned duration);

            int gc_traverse(void *,void *) const;
            int gc_clear();

            void restart();

        private:
            kbd_impl_t *_root;
    };

    class PIKEYBOARDPLG_DECLSPEC_CLASS alpha2_bundle_t: public pic::nocopy_t
    {
        public:
            alpha2_bundle_t(pic::usbdevice_t *usb,const piw::cookie_t &kc,const piw::cookie_t &ac,const pic::notify_t &dead);
            ~alpha2_bundle_t();

            std::string name();
            piw::data_t get_columnlen();
            piw::data_t get_columnoffset();
            piw::data_t get_courselen();
            piw::data_t get_courseoffset();
            float get_threshold1();
            float get_threshold2();
            void set_threshold1(float);
            void set_threshold2(float);
            float get_roll_axis_window();
            float get_yaw_axis_window();
            void set_roll_axis_window(float);
            void set_yaw_axis_window(float);
            piw::change_nb_t led_functor();
            void close();

            void learn_pedal_min(unsigned pedal);
            void learn_pedal_max(unsigned pedal);
            unsigned get_pedal_min(unsigned pedal);
            unsigned get_pedal_max(unsigned pedal);
            void set_pedal_min(unsigned pedal,unsigned val);
            void set_pedal_max(unsigned pedal,unsigned val);

            void testmsg_write_lib(unsigned,unsigned,unsigned,unsigned);
            void testmsg_finish_lib();
            void testmsg_write_seq(unsigned,unsigned,unsigned);
            void testmsg_finish_seq();
            void start_test(unsigned duration);
            void arm_recording(unsigned duration);
            void set_hp_quality(unsigned q);
            void set_mic_quality(unsigned q);

            int gc_traverse(void *,void *) const;
            int gc_clear();

            piw::cookie_t audio_cookie();
            void mic_enable(bool);
            void mic_type(unsigned);
            void mic_gain(unsigned);
            void mic_pad(bool);
            void mic_automute(bool);
            void mic_disabled(const pic::notify_t &);
            void headphone_enable(bool);
            void loopback_enable(bool);
            void loopback_gain(float);
            void headphone_gain(unsigned);
            void headphone_limit(bool);
            void debounce_time(unsigned long us);
            void threshold_time(unsigned long long us);
            void key_threshold(unsigned);
            void key_noise(unsigned);

            void restart();

        private:
            kbd_impl_t *_root;
    };

    class PIKEYBOARDPLG_DECLSPEC_CLASS tau_bundle_t: public pic::nocopy_t
    {
        public:
            tau_bundle_t(pic::usbdevice_t *usb,const piw::cookie_t &kc,const pic::notify_t &dead);
            ~tau_bundle_t();
            std::string name();
            piw::data_t get_columnlen();
            piw::data_t get_columnoffset();
            piw::data_t get_courselen();
            piw::data_t get_courseoffset();
            float get_threshold1();
            float get_threshold2();
            void set_threshold1(float);
            void set_threshold2(float);
            float get_roll_axis_window();
            float get_yaw_axis_window();
            void set_roll_axis_window(float);
            void set_yaw_axis_window(float);
            void set_hp_quality(unsigned q);
            piw::change_nb_t led_functor();
            void close();

            void learn_pedal_min(unsigned pedal);
            void learn_pedal_max(unsigned pedal);
            unsigned get_pedal_min(unsigned pedal);
            unsigned get_pedal_max(unsigned pedal);
            void set_pedal_min(unsigned pedal,unsigned val);
            void set_pedal_max(unsigned pedal,unsigned val);

            void testmsg_write_lib(unsigned,unsigned,unsigned,unsigned);
            void testmsg_finish_lib();
            void testmsg_write_seq(unsigned,unsigned,unsigned);
            void testmsg_finish_seq();
            void start_test(unsigned duration);
            void arm_recording(unsigned duration);

            int gc_traverse(void *,void *) const;
            int gc_clear();

            piw::cookie_t audio_cookie();
            void headphone_enable(bool);
            void headphone_limit(bool);
            void headphone_gain(unsigned);
            void debounce_time(unsigned long us);
            void threshold_time(unsigned long long us);
            void key_threshold(unsigned);
            void key_noise(unsigned);

            void restart();

        private:
            kbd_impl_t *_root;
    };
};

#endif
