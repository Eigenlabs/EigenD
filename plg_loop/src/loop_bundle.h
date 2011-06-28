
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

#ifndef __LOOP__
#define __LOOP__

#include <picross/pic_functor.h>
#include <piw/piw_bundle.h>
#include <plg_loop/src/loop_exports.h>

namespace piw
{
    class clockdomain_ctl_t;
}

namespace loop
{
    class PILOOP_DECLSPEC_CLASS player_t : public pic::nocopy_t
    {
        public:
            player_t(const piw::cookie_t &c, piw::clockdomain_ctl_t *,const pic::status_t &);
            ~player_t();
            void set_volume(float);
            float get_volume();
            void load(const char *);
            void unload();
            piw::change_t player(unsigned p);
            piw::cookie_t cookie();
            void set_chop(float c);

            class impl_t;
        private:
            impl_t *impl_;
    };
}

#endif

