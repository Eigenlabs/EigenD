
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

#ifndef __LNG_CONTROLS__
#define __LNG_CONTROLS__

#include <piw/piw_controller.h>
#include <pilanguage_exports.h>

namespace language
{
    class PILANGUAGE_DECLSPEC_CLASS toggle_t : public piw::xcontrolled_t
    {
        public:
            toggle_t(const piw::data_t &v);
            virtual ~toggle_t(); 

            virtual void control_init();
            virtual void control_receive(unsigned index, const piw::data_nb_t &value);
            void reset(const piw::data_t &); // slow

            int gc_traverse(void *,void *) const;
            int gc_clear();

            class timpl_t;

        private:
            timpl_t *timpl_;
    };

    class PILANGUAGE_DECLSPEC_CLASS updown_t : public piw::xcontrolled_t
    {
        public:
            updown_t(const piw::data_t &v,float coarse,float fine);
            virtual ~updown_t(); 

            virtual void control_init();
            virtual void control_receive(unsigned index, const piw::data_nb_t &value);
            virtual void control_term(unsigned long long t);
            void reset(const piw::data_t &); // slow

            int gc_traverse(void *,void *) const;
            int gc_clear();

            class uimpl_t;

        private:
            uimpl_t *uimpl_;
    };

    class PILANGUAGE_DECLSPEC_CLASS xselector_t : public piw::xcontrolled_t
    {
        public:
            xselector_t(const piw::change_nb_t &g,const piw::data_t &v);
            ~xselector_t();
            piw::cookie_t cookie();

            virtual void control_init();
            virtual void control_receive(unsigned index, const piw::data_nb_t &value);
            virtual void control_term(unsigned long long t);
            void reset(const piw::data_t &); // slow

            int gc_traverse(void *,void *) const;
            int gc_clear();

            void set_choice(unsigned index,const piw::data_t &v);
            
            class simpl_t;

        private:
            simpl_t *simpl_;
    };
};

#endif
