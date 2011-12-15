
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


#ifndef __ARRANGER_MODEL__
#define __ARRANGER_MODEL__

#include <plg_arranger/piarranger_exports.h>

#include <piw/piw_clock.h>
#include <piw/piw_bundle.h>

namespace arranger
{
    typedef std::pair<unsigned,unsigned> colrow_t;

    class PIARRANGER_DECLSPEC_CLASS model_t : public pic::nocopy_t
    {
        public:
            model_t(piw::clockdomain_ctl_t *);
            ~model_t();

            piw::cookie_t cookie();
            bct_clocksink_t *get_clock();

            void set_target(unsigned r, const piw::change_t &target); // slow
            void clear_target(unsigned r); // slow

            piw::change_nb_t set_event();
            void event_set(const piw::change_nb_t &);
            bool get_event(const colrow_t &,float *f);
            void clear_events();
            void events_cleared(const piw::change_nb_t &);

            piw::change_nb_t set_loopstart();
            void loopstart_set(const piw::change_nb_t &);
            unsigned get_loopstart();

            piw::change_nb_t set_loopend();
            void loopend_set(const piw::change_nb_t &);
            unsigned get_loopend();

            piw::change_nb_t set_position();
            void position_set(const piw::change_nb_t &);
            unsigned get_position();

            piw::change_nb_t set_stepnumerator();
            void stepnumerator_set(const piw::change_nb_t &);

            piw::change_nb_t set_stepdenominator();
            void stepdenominator_set(const piw::change_nb_t &);

            piw::change_nb_t set_playstop();
            void playstop_set(const piw::change_nb_t &);
            bool is_playing();

            class impl_t;

            int gc_traverse(void *,void *) const;
            int gc_clear();

        private:
            impl_t *impl_;
    };
};

#endif
