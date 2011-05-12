
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

#ifndef __REC_PLAYER__
#define __REC_PLAYER__
#include <pirecorder_exports.h>
#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_clock.h>

#include "rec_recording.h"

#define PLAYER_MODE_STRETCH 0
#define PLAYER_MODE_UNSTRETCH 1
#define PLAYER_MODE_CHOP 2

namespace recorder
{
    class PIRECORDER_DECLSPEC_CLASS player_t : public pic::nocopy_t
    {
        public:
            player_t(piw::clockdomain_ctl_t *, unsigned p, unsigned s,const piw::cookie_t &);
            ~player_t();

            piw::cookie_t cookie();

            unsigned long load(const std::string &name, const recording_t &, unsigned p);
            void unload(unsigned long r, bool);
            void abort(const std::string &);
            void abortcookie(unsigned const long);
            void abortall();
            piw::change_t player(unsigned long r, unsigned m);
            unsigned long getcookie(const std::string &name);
            unsigned long clonecookie(const std::string &name, unsigned poly);
            std::string cookiename(unsigned long r);

            class impl_t;
        private:
            impl_t *impl_;
    };
}

#endif

