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

#ifndef __PIW_STATUS__
#define __PIW_STATUS__
#include "piw_exports.h"
#include "piw_bundle.h"

namespace piw
{
    struct PIW_DECLSPEC_CLASS statusdata_t
    {
        static inline void int2c(int r, unsigned char *o)
        {
            long k = r;

            if(r<0) 
            {
                k = (int)0x10000+r;
            }

            o[0] = ((k>>8)&0xff);
            o[1] = (k&0xff);
        }

        static inline int c2int(unsigned char *c)
        {
            unsigned long cx = (c[0]<<8) | (c[1]);

            if(cx>0x7fff)
            {
                return ((long)cx)-0x10000;
            }

            return cx;
        }

        static inline void status2c(bool m, unsigned char s, unsigned char *o)
        {
            unsigned char r = s&0x7f;
            if(m) r+=(1<<7);
            *o = r;
        }

        statusdata_t(const bool m, const int r, const int c): musical(m), row(r), col(c) {}

        bool operator==(const statusdata_t &o) const
        {
            return musical == o.musical && row == o.row && col == o.col;
        }

        bool operator<(const statusdata_t &o) const
        {
            if(musical < o.musical) return true;
            if(musical > o.musical) return false;
            if(row < o.row) return true;
            if(row > o.row) return false;
            if(col < o.col) return true;
            if(col > o.col) return false;
            return false;
        }

        const bool musical;
        const int row;
        const int col;
    };

    class PIW_DECLSPEC_CLASS statusbuffer_t
    {
        public:
            class impl_t;
        public:
            statusbuffer_t(const piw::change_nb_t &, unsigned, const cookie_t &);
            statusbuffer_t(const cookie_t &);
            ~statusbuffer_t();
            void send();
            void override(bool);
            void autosend(bool);
            void clear();
            void set_status(bool,int,int,unsigned char);
            unsigned char get_status(bool,int,int);
            void set_blink_time(float);
            void set_blink_size(unsigned);
            piw::change_nb_t enabler();
            void enable(unsigned);
            piw::change_nb_t blinker();
            int gc_traverse(void *, void *) const;
            int gc_clear();
       public:
            static piw::data_nb_t make_statusbuffer(pic::lckmap_t<piw::statusdata_t,unsigned char>::nbtype &);
       private:
            impl_t *root_;
    };

    class PIW_DECLSPEC_CLASS statusmixer_t
    {
        public:
            class impl_t;
        public:
            statusmixer_t(const cookie_t &);
            ~statusmixer_t();
            void autoupdate(bool);
            void update();
            cookie_t cookie();
        private:
            impl_t *root_;
    };

    class PIW_DECLSPEC_CLASS statusledconvertor_t
    {
        public:
            class impl_t;
        public:
            statusledconvertor_t(unsigned nc, const unsigned *cs);
            ~statusledconvertor_t();
            void update_leds(piw::data_nb_t &status_data, void *kbd, void (*func_set_led)(void *self, unsigned key, unsigned color));
        private:
            impl_t *root_;
    };
};

#endif
