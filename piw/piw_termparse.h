
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

// On windows dont pass _DEBUG to python.h as it then tries to run the python with the python debug dll which is not part of the standard distribution
#ifdef _WIN32  
#ifdef _DEBUG
#define __REDEF_DEBUG__
#undef _DEBUG
#pragma message ("Undef _DEBUG") 
#endif
#endif

#include <Python.h>

#ifdef __REDEF_DEBUG__
#define _DEBUG
#undef __REDEF_DEBUG__
#pragma message ("redefine _DEBUG")
#endif

#include "piw_exports.h"
#include <piw/piw_data.h>

namespace piw
{
    struct PIW_DECLSPEC_CLASS parser_delegate_t
    {
        virtual ~parser_delegate_t() {}
        virtual void destroy_any(void *) = 0;
        virtual void *make_collection() = 0;
        virtual void add_arg(void *, void *) = 0;
        virtual void *make_term(const char *, unsigned, void *) = 0;
        virtual void *make_list(void *) = 0;
        virtual void *make_split(void *, void *) = 0;
        virtual void *make_long(long) = 0;
        virtual void *make_float(float) = 0;
        virtual void *make_generic(const piw::data_t &) = 0;
        virtual void *make_bool(bool) = 0;
        virtual void *make_none() = 0;
        virtual void *make_string(const char *, unsigned) = 0;
        virtual void *make_variable(const char *, unsigned) = 0;
        virtual void *make_subst(const char *, unsigned, const char *, unsigned) = 0;
    };

    struct PIW_DECLSPEC_CLASS python_delegate_t: parser_delegate_t
    {
        void destroy_any(void *o);
        void *make_generic(const piw::data_t &d);
        void *make_collection();
        void *make_term(const char *p, unsigned l, void *c);
        void *make_list(void *c);
        void *make_split(void *t1, void *t2);
        void add_arg(void *obj, void *arg);
        void *make_long(long v);
        void *make_float(float v);
        void *make_bool(bool v);
        void *make_none();
        void *make_string(const char *p, unsigned l);
        void *make_variable(const char *p, unsigned l);
        void *make_subst(const char *p1, unsigned l1, const char *p2, unsigned l2);

        static PyObject *cvt_return(void *t);

        virtual void py_destroy_any(PyObject *) = 0;
        virtual PyObject *py_make_term(PyObject *,PyObject *) = 0;
        virtual PyObject *py_make_list(PyObject *) = 0;
        virtual PyObject *py_make_split(PyObject *, PyObject *) = 0;
        virtual PyObject *py_make_collection() = 0;
        virtual void py_add_arg(PyObject *, PyObject *arg) = 0;
        virtual PyObject *py_make_long(long) = 0;
        virtual PyObject *py_make_float(float) = 0;
        virtual PyObject *py_make_bool(bool) = 0;
        virtual PyObject *py_make_string(PyObject *) = 0;
        virtual PyObject *py_make_variable(PyObject *) = 0;
        virtual PyObject *py_make_subst1(PyObject *) = 0;
        virtual PyObject *py_make_subst2(PyObject *, PyObject *) = 0;
        virtual PyObject *py_make_none() = 0;
    };

    PIW_DECLSPEC_FUNC(void) *parse_clause(parser_delegate_t *, const char *input);
    PIW_DECLSPEC_FUNC(void) *parse_clauselist(parser_delegate_t *, const char *input);
    PIW_DECLSPEC_FUNC(void) *parse_term(parser_delegate_t *, const char *input);
    PIW_DECLSPEC_FUNC(void) *parse_termlist(parser_delegate_t *, const char *input);

    inline PyObject *py_parse_clause(python_delegate_t *pd, const char *input) { return python_delegate_t::cvt_return(parse_clause(pd,input)); }
    inline PyObject *py_parse_clauselist(python_delegate_t *pd, const char *input) { return python_delegate_t::cvt_return(parse_clauselist(pd,input)); }
    inline PyObject *py_parse_term(python_delegate_t *pd, const char *input) { return python_delegate_t::cvt_return(parse_term(pd,input)); }
    inline PyObject *py_parse_termlist(python_delegate_t *pd, const char *input) { return python_delegate_t::cvt_return(parse_termlist(pd,input)); }
};
