
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

#ifndef __PIW_STATE__
#define __PIW_STATE__
#include "piw_exports.h"
#include <piw/piw_data.h>
#include <picross/pic_ref.h>

#define PIW_TERM_NULL 0
#define PIW_TERM_ATOM 1
#define PIW_TERM_LIST 2
#define PIW_TERM_PRED 3

namespace piw
{
    class PIW_DECLSPEC_CLASS term_t
    {
        public:
            class impl_t;

        public:
            term_t();
            term_t(const piw::data_t &);
            term_t(unsigned);
            term_t(const std::string &,unsigned);
            ~term_t();

            term_t(const term_t &);
            term_t &operator=(const term_t &);

            unsigned type() const;
            unsigned arity() const;
            const char *pred() const;

            piw::data_t value() const;
            term_t arg(unsigned) const;
            void set_arg(unsigned, const term_t &);
            void append_arg(const term_t &);
            void add_arg(int,const term_t &t) { append_arg(t); }

            const impl_t *impl() const;
            std::string render() const;
            void reset();
            unsigned count() const;

        private:
            pic::ref_t<impl_t> impl_;
    };

    PIW_DECLSPEC_FUNC(term_t) parse_state_term(const std::string &);
}

#endif
