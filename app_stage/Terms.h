/*
 Copyright 2010-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#ifndef __STAGE__
#define __STAGE__

#include "juce.h"

#define PIW_TERM_NULL 0
#define PIW_TERM_ATOM 1
#define PIW_TERM_LIST 2
#define PIW_TERM_PRED 3

class term_t
{
    public:
        struct impl_t;

    public:
        term_t();
        term_t(const String &);
        term_t(unsigned);
        term_t(const String &,unsigned);
        ~term_t();

        term_t(const term_t &);
        term_t &operator=(const term_t &);

        bool is_pred() const { return type()==PIW_TERM_PRED; }
        bool is_atom() const { return type()==PIW_TERM_ATOM; }
        bool is_list() const { return type()==PIW_TERM_LIST; }
        bool is_null() const { return type()==PIW_TERM_NULL; }
        unsigned type() const;
        unsigned arity() const;
        String pred() const;

        String value() const;
        term_t arg(unsigned) const;
        void set_arg(unsigned, const term_t &);
        void append_arg(const term_t &);
        void add_arg(int,const term_t &t) { append_arg(t); }

        const impl_t *impl() const;
        String render() const;
        void reset();
        unsigned count() const;

    private:
        ReferenceCountedObjectPtr<impl_t> impl_;
};

term_t parse_state_term(const String &);

#endif
