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

#ifndef __KBD_BUNDLE__
#define __KBD_BUNDLE__

#include <piw/piw_bundle.h>
#include <picross/pic_functor.h>
#include <plg_keyboard/src/kbd_exports.h>
namespace kbd
{
    class  PIKEYBOARDPLG_DECLSPEC_CLASS bundle_t
    {
        public:
            class impl_t;

        public:
            bundle_t(const char *usb, const piw::cookie_t &c, const pic::notify_t &dead);
            ~bundle_t();
            std::string name();
            float get_roll_axis_window();
            float get_yaw_axis_window();
            void set_roll_axis_window(float);
            void set_yaw_axis_window(float);
            float get_threshold1();
            float get_threshold2();
            void set_threshold1(float);
            void set_threshold2(float);
            piw::change_nb_t led_functor();
            void close();

            void learn_pedal_min(unsigned pedal) {}
            void learn_pedal_max(unsigned pedal) {}
            unsigned get_pedal_min(unsigned pedal) { return 0; }
            unsigned get_pedal_max(unsigned pedal) { return 4095; }
            void set_pedal_min(unsigned pedal,unsigned val) {}
            void set_pedal_max(unsigned pedal,unsigned val) {}

            int gc_traverse(void *,void *) const;
            int gc_clear();

        private:
            impl_t *_root;
    };
};

#endif
