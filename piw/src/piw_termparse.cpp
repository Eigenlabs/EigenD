
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

#include <piw/piw_termparse.h>
#include <ctype.h>

namespace
{
    typedef piw::parser_delegate_t pd_t;

    void *parse_clause_(pd_t *pd, const char **in);

    void skip_space(const char **in)
    {
        while(**in && isspace(**in))
        {
            (*in)++;
        }
    }

    void skip_one(const char **in)
    {
        if(**in) 
        {
            (*in)++;
        }
    }

    bool is_body(char w)
    {
        if(!w || w==')' || w=='|' || w==',' || w==']') return false;
        return true;
    }

    bool is_string_start(char w)
    {
        if(w=='\"' || w=='\'') return true;
        return false;
    }

    bool is_number_start(char w)
    {
        if(w>='0' && w<='9') return true;
        if(w=='.') return true;
        if(w=='-') return true;
        if(w=='+') return true;
        return false;
    }

    bool is_variable_start(char w)
    {
        if(w>='A' && w<='Z') return true;
        if(w=='_') return true;
        return false;
    }

    bool is_variable_body(char w)
    {
        if(w>='a' && w<='z') return true;
        if(w>='A' && w<='Z') return true;
        if(w>='0' && w<='9') return true;
        if(w=='_') return true;
        return false;
    }

    bool is_word_start(char w)
    {
        if(w>='a' && w<='z') return true;
        if(w>='0' && w<='9') return true;
        if(w=='!' || w=='$' || w=='@' || w=='#' || w=='<' || w=='>') return true;
        return false;
    }

    bool is_word_body(char w)
    {
        if(is_word_start(w)) return true;
        if(w>='A' && w<='Z') return true;
        if(w=='_' || w=='.') return true;
        return false;
    }

    void *parse_list_(pd_t *pd, const char **in)
    {
        skip_space(in);

        if(**in != '[')
        {
            return 0;
        }

        skip_one(in);

        skip_space(in);

        if(**in == ']')
        {
            skip_one(in);
            return pd->make_list(pd->make_collection());
        }

        void *t1 = parse_clause_(pd,in);

        if(!t1)
        {
            return 0;
        }

        skip_space(in);

        if(**in == ']')
        {
            skip_one(in);
            void *c = pd->make_collection();
            pd->add_arg(c,t1);
            return pd->make_list(c);
        }

        if(**in == '|')
        {
            skip_one(in);

            void *t2 = parse_clause_(pd,in);

            if(!t2)
            {
                pd->destroy_any(t1);
                return 0;
            }

            skip_space(in);

            if(**in != ']')
            {
                pd->destroy_any(t1);
                pd->destroy_any(t2);
                return 0;
            }

            skip_one(in);
            return pd->make_split(t1,t2);
        }

        void *c = pd->make_collection();
        pd->add_arg(c,t1);

        for(;;)
        {
            skip_space(in);

            if(**in != ',')
            {
                pd->destroy_any(c);
                return 0;
            }

            skip_one(in);

            void *a = parse_clause_(pd,in);

            if(!a)
            {
                pd->destroy_any(c);
                return 0;
            }

            pd->add_arg(c,a);
            skip_space(in);

            if(**in == ']')
            {
                skip_one(in);
                return pd->make_list(c);
            }
        }
    }

#if defined(PI_WINDOWS) || defined(PI_LINUX)
	int digittoint(char a)
	{
		int ret = 0;
		if (a >= '0' && a<= '9')
		{
			ret = a - '0';
		}
		else if( a >= 'a' && a<= 'f' )
		{
			ret = a - 'a' + 10;
		}
		else if( a >= 'A' && a<= 'F' )
		{
			ret = a - 'A' + 10;
		}
		
		return ret;
	}
#endif
	
    void *parse_generic_(pd_t *pd, const char **in)
    {
        skip_space(in);

        if(**in != '{')
        {
            return 0;
        }

        (*in)++;

        unsigned char wp[BCTLIMIT_DATA];
        unsigned wl=0;

        while(**in && **in != '}')
        {
            unsigned c1,c2;
            if(!isxdigit(**in)) return 0;
            c1 = **in;
            (*in)++;
            if(!isxdigit(**in)) return 0;
            c2 = **in;
            (*in)++;
            wp[wl++] = 16*digittoint(c1)+digittoint(c2);
        }

        if(**in != '}') return 0;
        (*in)++;

        piw::data_t d = piw::makewire(wl,wp);
        return pd->make_generic(d);
    }

    void *parse_number_(pd_t *pd, const char **in)
    {
        bool neg = false;
        long n1 = 0;

        skip_space(in);

        if(**in == '-')
        {
            (*in)++;
            neg = true;
        }
        else if(**in == '+')
        {
            (*in)++;
        }

        if(**in != '.' && (**in <'0' && **in > '9'))
        {
            return 0;
        }

        while(**in >= '0' && **in<='9')
        {
            n1 = 10*n1+(**in)-'0';
            (*in)++;
        }

        if(**in != '.' && **in != 'e' && **in != 'E' && **in != 'f' && **in != 'F')
        {
            if(neg) n1=-n1;
            return pd->make_long(n1);
        }

        if(**in == 'f' || **in == 'F')
        {
            (*in)++;
            if(neg) n1=-n1;
            return pd->make_float(n1);
        }

        float f = float(n1);

        if(**in == '.')
        {
            (*in)++;

            float n2 = 0;
            unsigned digits = 0;

            while(**in >= '0' && **in<='9')
            {
                n2 = 10.0*n2+(**in)-'0';
                (*in)++;
                digits++;
            }

            if(digits) f += (float(n2)/pow((double)10.0,(double)digits));
        }

        if(neg) f=-f;

        if(**in != 'e' && **in != 'E')
        {
            if(**in == 'f' || **in == 'F') { (*in)++; }
            return pd->make_float(f);
        }

        (*in)++;

        long e = 0;
        neg = false;

        if(**in == '-')
        {
            (*in)++;
            neg = true;
        }
        else if(**in == '+')
        {
            (*in)++;
        }

        while(**in >= '0' && **in<='9')
        {
            e = 10*e+(**in)-'0';
            (*in)++;
        }

        if(**in == 'f' || **in == 'F') { (*in)++; }

        if(neg)
        {
            e = -e;
        }

        f = f*pow(10.0,e);
        return pd->make_float(f);
    }

    bool parse_string0_(const char **in, const char **wp, unsigned *wl, bool *esc)
    {
        skip_space(in);

        if(!**in)
        {
            return false;
        }

        char lead = **in;
        *esc = false;

        skip_one(in);

        const char *p = *in;

        while(**in && **in!=lead)
        {
            if(**in == '%') *esc = true;
            skip_one(in);
        }

        if(**in == lead)
        {
            skip_one(in);
            *wp = p;
            *wl = (*in)-p-1;
            return true;
        }

        return false;
    }

    void *parse_string_(pd_t *pd, const char **in)
    {
        const char *wp;
        unsigned wl;
        bool esc;

        if(!parse_string0_(in,&wp,&wl,&esc))
        {
            return 0;
        }

        if(!esc)
        {
            return pd->make_string(wp,wl);
        }

        char *es = (char *)malloc(wl);
        unsigned el=0;

        while(wl>0)
        {
            if(*wp!='%')
            {
                es[el++] = *wp++; wl--;
                continue;
            }

            wp++; wl--;

            if(*wp=='%')
            {
                es[el++] = '%'; wp++; wl--;
                continue;
            }

            if(wl<2 || !isxdigit(wp[0]) || !isxdigit(wp[1]))
            {
                free(es);
                return 0;
            }

            es[el++] = 16*digittoint(wp[0])+digittoint(wp[1]); wp+=2; wl-=2;
        }

        void *s = pd->make_string(es,el);
        free(es);
        return s;
    }

    bool parse_word_(const char **in, const char **w, unsigned *l)
    {
        skip_space(in);

        if(!is_word_start(**in))
        {
            return false;
        }

        *w = *in;
        (*in)++;

        while(is_word_body(**in))
        {
            (*in)++;
        }

        *l = *in-*w;
        return true;
    }

    void *parse_subst_(pd_t *pd, const char **in)
    {
        skip_space(in);

        if(**in != '~')
        {
            return 0;
        }

        skip_one(in);

        const char *wp;
        unsigned wl;

        if(**in != '(')
        {
            if(!parse_word_(in,&wp,&wl))
            {
                return 0;
            }

            return pd->make_subst(wp,wl,0,0);
        }

        skip_one(in);

        if(!parse_word_(in,&wp,&wl))
        {
            return 0;
        }

        skip_space(in);

        if(**in != ')')
        {
            return 0;
        }

        skip_one(in);
        skip_space(in);

        const char *w2p = *in;
        unsigned w2l;
        bool w2e;

        if(is_string_start(**in))
        {
            if(parse_string0_(in,&w2p,&w2l,&w2e))
            {
                return pd->make_subst(wp,wl,w2p,w2l);
            }

            return 0;
        }

        while(is_body(**in))
        {
            (*in)++;
        }

        w2l = (*in)-w2p;
        return pd->make_subst(wp,wl,w2l?w2p:0,w2l);
    }

    void *build_variable_(pd_t *pd, const char *wp, unsigned wl)
    {
        if(wl==4 && !strncmp(wp,"True",4)) return pd->make_bool(true);
        if(wl==5 && !strncmp(wp,"False",5)) return pd->make_bool(false);
        if(wl==4 && !strncmp(wp,"None",4)) return pd->make_none();
        return pd->make_variable(wp,wl);
    }

    void *build_atom_(pd_t *pd, const char *wp, unsigned wl)
    {
        if(wl==4 && !strncmp(wp,"True",4)) return pd->make_bool(true);
        if(wl==5 && !strncmp(wp,"False",5)) return pd->make_bool(false);
        if(wl==4 && !strncmp(wp,"None",4)) return pd->make_none();
        return pd->make_string(wp,wl);
    }

    void *parse_variable_(pd_t *pd, const char **in)
    {
        skip_space(in);

        if(!is_variable_start(**in))
        {
            return 0;
        }

        const char *p = *in;

        while(is_variable_body(**in))
        {
            (*in)++;
        }

        return build_variable_(pd,p,*in-p);
    }

    void *parse_clauselist_(pd_t *pd, const char **in)
    {
        void *t = pd->make_collection();

        for(;;)
        {
            const char *q = *in;

            void *a = parse_clause_(pd,in);

            if(!a)
            {
                *in = q;
                return t;
            }

            pd->add_arg(t,a);

            skip_space(in);

            if(**in != ',')
            {
                return t;
            }

            skip_one(in);
        }
    }

    void *parse_word_or_term_(pd_t *pd, const char **in, bool term)
    {
        skip_space(in);
        const char *wp;
        unsigned wl;

        if(!parse_word_(in,&wp,&wl))
        {
            return 0;
        }

        skip_space(in);

        if(**in != '(')
        {
            if(term)
            {
                return 0;
            }

            return build_atom_(pd,wp,wl);
        }

        skip_one(in);

        void *t = parse_clauselist_(pd,in);

        if(!t)
        {
            return 0;
        }

        skip_space(in);

        if(**in != ')')
        {
            pd->destroy_any(t);
            return 0;
        }

        skip_one(in);
        return pd->make_term(wp,wl,t);
    }

    void *parse_termlist_(pd_t *pd, const char **in)
    {
        void *t = pd->make_collection();

        for(;;)
        {
            const char *q = *in;

            void *a = parse_word_or_term_(pd,in,true);

            if(!a)
            {
                *in = q;
                return t;
            }

            pd->add_arg(t,a);

            skip_space(in);

            if(**in != ',')
            {
                return t;
            }

            skip_one(in);
        }
    }

    void *parse_clause_(pd_t *pd, const char **in)
    {
        skip_space(in);

        if(**in == '[') return parse_list_(pd,in);
        if(is_string_start(**in)) return parse_string_(pd,in);
        if(is_number_start(**in)) return parse_number_(pd,in);
        if(is_variable_start(**in)) return parse_variable_(pd,in);
        if(**in == '{') return parse_generic_(pd,in);
        if(**in == '~') return parse_subst_(pd,in);

        return parse_word_or_term_(pd,in,false);
    }

};

void *piw::parse_clause(pd_t *pd, const char *input)
{
    const char *input_copy = input;
    void *t = parse_clause_(pd,&input_copy);
    if(!t) return 0;
    skip_space(&input_copy);
    if(!*input_copy) return t;
    pd->destroy_any(t);
    return 0;
}

void *piw::parse_term(pd_t *pd, const char *input)
{
    const char *input_copy = input;
    void *t = parse_word_or_term_(pd,&input_copy,true);
    if(!t) return 0;
    skip_space(&input_copy);
    if(!*input_copy) return t;
    pd->destroy_any(t);
    return 0;
}

void *piw::parse_clauselist(pd_t *pd, const char *input)
{
    const char *input_copy = input;
    void *t = parse_clauselist_(pd,&input_copy);
    if(!t)
    {
        return 0;
    }
    skip_space(&input_copy);
    if(!*input_copy) return t;
    pd->destroy_any(t);
    return 0;
}

void *piw::parse_termlist(pd_t *pd, const char *input)
{
    const char *input_copy = input;
    void *t = parse_termlist_(pd,&input_copy);
    if(!t)
    {
        return 0;
    }
    skip_space(&input_copy);
    if(!*input_copy) return t;
    pd->destroy_any(t);
    return 0;
}

void piw::python_delegate_t::destroy_any(void *o)
{
    Py_DECREF((PyObject *)o);
}

void *piw::python_delegate_t::make_generic(const piw::data_t &d)
{
    return 0;
}

void *piw::python_delegate_t::make_collection()
{
    return py_make_collection();
}

void *piw::python_delegate_t::make_term(const char *p, unsigned l, void *c)
{
    PyObject *pred = PyString_FromStringAndSize(p,l);
    PyObject *cobj = (PyObject *)c;
    PyObject *term = py_make_term(pred,cobj);
    Py_DECREF(pred);
    Py_DECREF(cobj);
    return term;
}

void *piw::python_delegate_t::make_list(void *c)
{
    PyObject *cobj = (PyObject *)c;
    PyObject *list = py_make_list(cobj);
    Py_DECREF(cobj);
    return list;
}

void *piw::python_delegate_t::make_split(void *t1, void *t2)
{
    PyObject *pt1 = (PyObject *)t1;
    PyObject *pt2 = (PyObject *)t2;
    PyObject *split = py_make_split(pt1,pt2);
    Py_DECREF(pt1);
    Py_DECREF(pt2);
    return split;
}

void piw::python_delegate_t::add_arg(void *obj, void *arg)
{
    PyObject *pobj = (PyObject *)obj;
    PyObject *parg = (PyObject *)arg;
    py_add_arg(pobj,parg);
    Py_DECREF(parg);
}

void *piw::python_delegate_t::make_long(long v)
{
    return py_make_long(v);
}

void *piw::python_delegate_t::make_float(float v)
{
    return py_make_float(v);
}

void *piw::python_delegate_t::make_bool(bool v)
{
    return py_make_bool(v);
}

void *piw::python_delegate_t::make_none()
{
    return py_make_none();
}

void *piw::python_delegate_t::make_string(const char *p, unsigned l)
{
    PyObject *value = PyString_FromStringAndSize(p,l);
    PyObject *term = py_make_string(value);
    Py_DECREF(value);
    return term;
}

void *piw::python_delegate_t::make_variable(const char *p, unsigned l)
{
    PyObject *value = PyString_FromStringAndSize(p,l);
    PyObject *term = py_make_variable(value);
    Py_DECREF(value);
    return term;
}

void *piw::python_delegate_t::make_subst(const char *p1, unsigned l1, const char *p2, unsigned l2)
{

    if(!p2 || !l2)
    {
        PyObject *value1 = PyString_FromStringAndSize(p1,l1);
        PyObject *term = py_make_subst1(value1);
        Py_DECREF(value1);
        return term;
    }
    else
    {
        PyObject *value1 = PyString_FromStringAndSize(p1,l1);
        PyObject *value2 = PyString_FromStringAndSize(p2,l2);
        PyObject *term = py_make_subst2(value1,value2);
        Py_DECREF(value1);
        Py_DECREF(value2);
        return term;
    }
}

PyObject *piw::python_delegate_t::cvt_return(void *t)
{
    if(!t)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return (PyObject *)t;
}
