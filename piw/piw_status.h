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
#include "piw_keys.h"

namespace piw
{
    struct PIW_DECLSPEC_CLASS statusdata_t
    {
        statusdata_t(bool, const piw::coordinate_t &, unsigned char);

        static statusdata_t from_bytes(unsigned char *);
        void to_bytes(unsigned char *) const;

        bool operator==(const statusdata_t &) const;
        bool operator<(const statusdata_t &) const;
        bool operator>(const statusdata_t &) const;

        const bool musical_;
        const piw::coordinate_t coordinate_;
        const unsigned char status_;

        private:

            static inline void int2c(int, unsigned char *);
            static inline int c2int(unsigned char *);
    };

    class PIW_DECLSPEC_CLASS statusbuffer_t
    {
        public:
            class impl_t;
        public:
            statusbuffer_t(const piw::change_nb_t &, unsigned, const piw::cookie_t &);
            statusbuffer_t(const piw::cookie_t &);
            ~statusbuffer_t();
            void send();
            void override(bool);
            void autosend(bool);
            void clear();
            void clear_status(bool, const piw::coordinate_t &);
            void set_status(bool, const piw::coordinate_t &, unsigned char);
            unsigned char get_status(bool, const piw::coordinate_t &);
            void set_blink_time(float);
            void set_blink_geometry(const piw::data_t &);
            piw::change_nb_t enabler();
            void enable(unsigned);
            piw::change_nb_t blinker();
            int gc_traverse(void *, void *) const;
            int gc_clear();
       public:
            static piw::data_nb_t make_statusbuffer(pic::lckset_t<piw::statusdata_t>::nbtype &);
       private:
            impl_t *root_;
    };

    class PIW_DECLSPEC_CLASS statusmixer_t
    {
        public:
            class impl_t;
        public:
            statusmixer_t(const piw::cookie_t &);
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
            statusledconvertor_t(unsigned, const unsigned *);
            ~statusledconvertor_t();
            void update_leds(piw::data_nb_t &status_data, void *kbd, void (*func_set_led)(void *self, unsigned key, unsigned color));
        private:
            impl_t *root_;
    };
};

PIW_DECLSPEC_FUNC(std::ostream) &operator<<(std::ostream &o, const piw::statusdata_t &d);

#endif
