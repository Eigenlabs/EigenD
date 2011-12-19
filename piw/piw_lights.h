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

#ifndef __PIW_LIGHTS__
#define __PIW_LIGHTS__
#include "piw_exports.h"
#include "piw_bundle.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS lightsource_t
    {
        public:
            class impl_t;
        public:
            lightsource_t(const piw::change_nb_t &, unsigned, const cookie_t &);
            ~lightsource_t();
            void set_size(unsigned lights);
            unsigned get_size();
            void override(bool);
            void set_color(unsigned,unsigned);
            void set_status(unsigned,unsigned);
            piw::change_nb_t lightswitch();
            piw::change_nb_t blinker();
            int gc_traverse(void *, void *) const;
            int gc_clear();
            void enable(unsigned);
        private:
            impl_t *root_;
    };

	class PIW_DECLSPEC_CLASS lightconvertor_t
	{
        public:
            class impl_t;
        public:
            lightconvertor_t(bool, const cookie_t &);
            ~lightconvertor_t();
            void set_status_handler(unsigned, int, int, change_t);
            void remove_status_handler(unsigned);
            unsigned char get_status(int, int);
            cookie_t cookie();
            void set_default_color(unsigned index, unsigned color);
        private:
            impl_t *impl_;
	};
};

#endif
