
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

#ifndef __PIK_ACTIVE__
#define __PIK_ACTIVE__

#include "alpha1_exports.h"
#include <picross/pic_usb.h>
#include <picross/pic_fastalloc.h>

#define KBD_KEYS 132

#define KBD_DESENSE (KBD_KEYS+0)
#define KBD_BREATH1 (KBD_KEYS+1)
#define KBD_BREATH2 (KBD_KEYS+2)
#define KBD_STRIP1  (KBD_KEYS+3)
#define KBD_STRIP2  (KBD_KEYS+4)
#define KBD_SENSORS 5

namespace alpha1
{
    class ALPHA1_DECLSPEC_CLASS active_t: virtual public pic::lckobject_t, public pic::pollable_t
    {
        public:
            class impl_t;

        public:
            struct delegate_t
            {
                virtual ~delegate_t() {}
                virtual void kbd_dead() {}
                virtual void kbd_raw(unsigned long long t, unsigned key, unsigned c1, unsigned c2, unsigned c3, unsigned c4) {}
                virtual void kbd_key(unsigned long long t, unsigned key, unsigned p, int r, int y) {}
                virtual void kbd_keydown(unsigned long long t, const unsigned short *bitmap) {}
            };

            inline static unsigned word2key(unsigned word) { return word*16; }
            inline static unsigned short key2word(unsigned key) { return key/16; }
            inline static unsigned short key2mask(unsigned key) { return (1<<(key%16)); }
            inline static bool keydown(unsigned key, const unsigned short *bitmap) { return (bitmap[key2word(key)]&key2mask(key))!=0; }

        public:
            active_t(const char *name, delegate_t *);
            ~active_t();

            void start();
            void stop();
            bool poll(unsigned long long t);

            void set_led(unsigned key, unsigned colour);
            void set_raw(bool raw);
            void set_calibration(unsigned key, unsigned corner, unsigned short min, unsigned short max, const unsigned short *table);
            void write_calibration();
            void clear_calibration();
            unsigned get_temperature();

            const char *get_name();

            pic::usbdevice_t *device();

        private:
            impl_t *_impl;
    };
};

#endif
