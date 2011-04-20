
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

#ifndef __ALPHA1_PASSIVE__
#define __ALPHA1_PASSIVE__

#include "alpha1_exports.h"

namespace alpha1
{
    class ALPHA1_DECLSPEC_CLASS passive_t
    {
        public:
            class impl_t;
        public:
            passive_t(const char *name,unsigned decimate=1);
            ~passive_t();

            unsigned short get_rawkey(unsigned key, unsigned sensor);

            void start_calibration_row(unsigned key, unsigned corner);
            void set_calibration_range(unsigned short min, unsigned short max);
            void set_calibration_point(unsigned point, unsigned short value);
            void write_calibration_row();
            void commit_calibration();
            void clear_calibration();
            unsigned get_temperature();

            void set_ledcolour(unsigned key, unsigned colour);
            void set_ledrgb(unsigned key, unsigned r, unsigned g, unsigned b);

            const char *get_name();
            unsigned long count();

            bool wait();
            void sync();

            void start_collecting();
            void stop_collecting();
            unsigned collect_count();
            unsigned short get_collected_key(unsigned scan, unsigned key, unsigned corner);

            void start();
            void stop();

            unsigned short get_strip1();
            unsigned short get_strip2();
            unsigned short get_breath();
        private:
            impl_t *impl_;
    };
};

#endif
