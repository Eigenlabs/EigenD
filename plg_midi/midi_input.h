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

#ifndef __CMID_BUNDLE__
#define __CMID_BUNDLE__

#include <piw/piw_bundle.h>
#include <pimidi_exports.h>

namespace pi_midi
{
    class PIMIDI_DECLSPEC_CLASS midi_input_port_t
    {
        public:
            class impl_t;

        public:
            midi_input_port_t(const piw::change_nb_t &sink);
            virtual ~midi_input_port_t();

            bool setport(long);
            long getport(void);

            void set_destination(const std::string &name);
            virtual void source_added(long uid, const std::string &name) {}
            virtual void source_removed(long uid) {}

            void run();
            void stop();

        private:
            impl_t *impl_;
    };

    class PIMIDI_DECLSPEC_CLASS midi_input_t
    {
        public:
            class impl_t;

        public:
            midi_input_t(const piw::cookie_t &keyboard_cookie,const piw::cookie_t &midi_cookie);
            virtual ~midi_input_t();
            bool setport(long);
            long getport(void);
            void set_trim(unsigned,float,float,bool);
            void clear_trim();
            unsigned current(unsigned);
            void set_destination(const std::string &name);
            virtual void source_added(long uid, const std::string &name) {}
            virtual void source_removed(long uid) {}
            void run();
            void stop();

            int gc_traverse(void *,void *) const;
            int gc_clear();

        private:
            impl_t *root_;
    };
};

#endif
