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

#ifndef __PI_STATE_MAPPING__
#define __PI_STATE_MAPPING__

#include <map>
#include <piw/piw_data.h>
#include "sng_exports.h"

namespace pi
{
    namespace state
    {
        class SNG_DECLSPEC_CLASS mapping_t: public pic::counted_t
        {
            private:
                mapping_t(bool exclusive): exclusive_(exclusive) {}

            public:
                static pic::ref_t<mapping_t> create(bool exclusive) { return pic::ref(new mapping_t(exclusive)); }
                void add(const char *from, const char *to);
                std::string render() const;
                piw::data_t substitute(const piw::data_t &src) const;
                std::string substitute_string(const std::string &src) const;

            private:
                piw::data_t substitute_dict__(const piw::data_t &data) const;
                piw::data_t substitute_string__(const piw::data_t &data) const;
                bool substitute_stdstr__(std::string &) const;

            private:
                std::map<std::string,std::string> mapping_;
                bool exclusive_;
        };

        typedef pic::ref_t<mapping_t> mapref_t;
        inline mapref_t create_mapping(bool exclusive) { return mapping_t::create(exclusive); }
    };
};

#endif
