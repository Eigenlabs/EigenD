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

#ifndef __PIW_CONTROLLERDICT__
#define __PIW_CONTROLLERDICT__

#include "piw_exports.h"
#include "piw_bundle.h"

namespace piw
{
    class PIW_DECLSPEC_CLASS controllerdict_t
    {
        public:
            class impl_t;
        public:
            controllerdict_t(const cookie_t &);
            ~controllerdict_t();
            piw::data_t get_ctl_value(const std::string &k);
            piw::data_t get_ctl_dict();
            void put_ctl(const std::string &k,const piw::data_t &v);
            void put(const std::string &k,const piw::data_t &v);
            piw::change_t changetonic(unsigned t);
            piw::change_t changescale(const std::string &s);
       private:
            impl_t *root_;
    };
};

#endif
