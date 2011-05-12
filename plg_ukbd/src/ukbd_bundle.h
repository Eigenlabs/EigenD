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

#ifndef __UKBD_BUNDLE__
#define __UKBD_BUNDLE__

#include <piukbdplg_exports.h>
#include <piw/piw_bundle.h>
#include <picross/pic_functor.h>



namespace ukbd
{
    class PIUKBDPLG_DECLSPEC_CLASS bundle_t
    {
        public:
            class impl_t;

        public:
            bundle_t(const char *usb, const piw::cookie_t &c, const pic::notify_t &dead, const piw::change_t &ch);
            ~bundle_t();
            void close();
            std::string name();
            piw::change_nb_t led_functor();
            float get_threshold1();
            float get_threshold2();
            void set_threshold1(float);
            void set_threshold2(float);

            int gc_traverse(void *,void *) const;
            int gc_clear();

        private:
            impl_t *_root;
    };
};

#endif
