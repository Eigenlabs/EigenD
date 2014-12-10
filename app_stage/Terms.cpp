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

#include "Terms.h"
#include <ctype.h>
#include <map>

struct term_t::impl_t: public ReferenceCountedObject
{
    impl_t(unsigned type): type_(type) { }
    virtual ~impl_t() { }

    virtual String pred() const { return String::empty; }
    virtual String value() const { return String::empty; }
    virtual unsigned arity() const { return 0; }
    virtual term_t arg(unsigned) const { return term_t(); }
    virtual void set_arg(unsigned,const term_t &) {}
    virtual void append_arg(const term_t &v) {}
    virtual void render(String &) const = 0;

    unsigned type_;
};

namespace
{
    struct pd_t
    {
        virtual ~pd_t() {}
        virtual void destroy_any(void *) = 0;
        virtual void *make_collection() = 0;
        virtual void add_arg(void *, void *) = 0;
        virtual void *make_term(const char *, unsigned, void *) = 0;
        virtual void *make_list(void *) = 0;
        virtual void *make_split(void *, void *) = 0;
        virtual void *make_long(long) = 0;
        virtual void *make_float(float) = 0;
        virtual void *make_bool(bool) = 0;
        virtual void *make_none() = 0;
        virtual void *make_string(const char *, unsigned) = 0;
        virtual void *make_variable(const char *, unsigned) = 0;
        virtual void *make_subst(const char *, unsigned, const char *, unsigned) = 0;
    };

    void render_string(String &s, const char *str)
    {
        s << '\'';

        while(*str)
        {
            switch(*str)
            {
                case '\'': s << "%27"; break;
                case '%': s << "%25"; break;
                default: s << *str; break;
            }

            str++;
        }

        s << '\'';
    }

    char to_hex(unsigned d)
    {
        if(d<10) return '0'+d;
        return d-10+'a';
    }

    struct term_atom_t: term_t::impl_t
    {
        term_atom_t(const String &value): impl_t(PIW_TERM_ATOM), value_(value) {}
        ~term_atom_t() {}
        String value() const { return value_; }

        void render(String &stream) const { stream << value_; }

        String value_;
    };

    struct term_list_t: term_t::impl_t
    {
        term_list_t(unsigned len): impl_t(PIW_TERM_LIST), values_(len) {}
        term_list_t(): impl_t(PIW_TERM_LIST) {}
        ~term_list_t() { values_.resize(0); }

        unsigned arity() const { return values_.size(); }
        term_t arg(unsigned n) const { return values_[n]; }
        void set_arg(unsigned n,const term_t &v) { values_[n] = v; }

        void append_arg(const term_t &v)
        {
            unsigned n = values_.size();
            values_.resize(n+1);
            values_[n].reset();
            values_[n]=v;
        }

        void render(String &stream) const
        {
            stream << '[';
            bool comma = false;

            for(unsigned i=0;i<values_.size();i++)
            {
                const impl_t *impl = values_[i].impl();

                if(comma) stream << ',';
                comma = true;

                if(impl)
                {
                    impl->render(stream);
                }
                else
                {
                    stream << "None";
                }
            }

            stream << ']';
        }

        std::vector<term_t> values_;
    };

    struct term_pred_t: term_t::impl_t
    {
        term_pred_t(const String &pred, unsigned len): impl_t(PIW_TERM_PRED), pred_(pred), values_(len) { }
        term_pred_t(const String &pred): impl_t(PIW_TERM_PRED), pred_(pred) { }
        ~term_pred_t() { values_.resize(0); }

        unsigned arity() const { return values_.size(); }
        term_t arg(unsigned n) const { return values_[n]; }
        void set_arg(unsigned n,const term_t &v) { values_[n] = v; }

        void append_arg(const term_t &v)
        {
            unsigned n = values_.size();
            values_.resize(n+1);
            values_[n].reset();
            values_[n]=v;
        }

        String pred() const { return pred_; }

        void render(String &stream) const
        {
            stream << pred_ << '(';
            bool comma = false;

            for(unsigned i=0;i<values_.size();i++)
            {
                const impl_t *impl = values_[i].impl();

                if(comma) stream << ',';
                comma = true;

                if(impl)
                {
                    impl->render(stream);
                }
                else
                {
                    stream << "None";
                }
            }

            stream << ')';
        }

        String pred_;
        std::vector<term_t> values_;
    };

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

#if defined(WIN32) || defined(__linux__)
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
        if(**in == '~') return parse_subst_(pd,in);

        return parse_word_or_term_(pd,in,false);
    }

    struct parse_state_delegate: pd_t
    {
        virtual void destroy_any(void *t)
        {
            delete (term_t *)t;
        }

        virtual void *make_collection()
        {
            return new term_t(0);
        }

        virtual void add_arg(void *t, void *a)
        {
            ((term_t *)t)->append_arg(*(term_t *)a);
        }

        virtual void *make_term(const char *p, unsigned pl, void *c_)
        {
            term_t *c = (term_t *)c_;
            term_t *t = new term_t(String::fromUTF8(p,pl),c->arity());

            for(unsigned i=0;i<c->arity();i++)
            {
                t->set_arg(i,c->arg(i));
            }

            delete c;
            return t;
        }

        virtual void *make_list(void *c)
        {
            return c;
        }

        virtual void *make_split(void *, void *)
        {
            return 0;
        }

        virtual void *make_long(long v)
        {
            return new term_t(String((int64)v));
        }

        virtual void *make_float(float v)
        {
            return new term_t(String(v));
        }

        virtual void *make_bool(bool v)
        {
            return new term_t(String(v?1:0));
        }

        virtual void *make_none()
        {
            return new term_t();
        }

        virtual void *make_string(const char *s, unsigned sl)
        {
            return new term_t(String::fromUTF8(s,sl));
        }

        virtual void *make_variable(const char *, unsigned)
        {
            return 0;
        }

        virtual void *make_subst(const char *, unsigned, const char *, unsigned)
        {
            return 0;
        }
    };

    void *parse_clause(pd_t *pd, const char *input)
    {
        const char *input_copy = input;
        void *t = parse_clause_(pd,&input_copy);
        if(!t) return 0;
        skip_space(&input_copy);
        if(!*input_copy) return t;
        pd->destroy_any(t);
        return 0;
    }

    void *parse_term(pd_t *pd, const char *input)
    {
        const char *input_copy = input;
        void *t = parse_word_or_term_(pd,&input_copy,true);
        if(!t) return 0;
        skip_space(&input_copy);
        if(!*input_copy) return t;
        pd->destroy_any(t);
        return 0;
    }

    void *parse_clauselist(pd_t *pd, const char *input)
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

    void *parse_termlist(pd_t *pd, const char *input)
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
};

term_t::term_t()
{
}

term_t::~term_t()
{
}

const term_t::impl_t *term_t::impl() const
{
    return impl_.get();
}

String term_t::render() const
{
    if(!impl_.get())
    {
        return "";
    }

    String stream;
    impl_->render(stream);
    return stream;
}

term_t::term_t(const String &v)
{
    impl_ = ReferenceCountedObjectPtr<impl_t>(new term_atom_t(v));
}

term_t::term_t(unsigned len)
{
    if(len)
        impl_ = ReferenceCountedObjectPtr<impl_t>(new term_list_t(len));
    else
        impl_ = ReferenceCountedObjectPtr<impl_t>(new term_list_t);
}

term_t::term_t(const String &pred,unsigned len)
{
    if(len)
        impl_ = ReferenceCountedObjectPtr<impl_t>(new term_pred_t(pred,len));
    else
        impl_ = ReferenceCountedObjectPtr<impl_t>(new term_pred_t(pred));
}

term_t::term_t(const term_t &t): impl_(t.impl_)
{
}

term_t &term_t::operator=(const term_t &t)
{
    impl_=t.impl_;
    return *this;
}

unsigned term_t::type() const
{
    return impl_.get()?impl_->type_:PIW_TERM_NULL;

}

unsigned term_t::arity() const
{
    return impl_.get()?impl_->arity():0;
}

String term_t::pred() const
{
    return impl_.get()?impl_->pred():String::empty;
}

String term_t::value() const
{
    return impl_.get()?impl_->value():String();
}

term_t term_t::arg(unsigned n) const
{
    return impl_.get()?impl_->arg(n):term_t();
}

void term_t::set_arg(unsigned n, const term_t &v)
{
    if(impl_.get())
    {
        impl_->set_arg(n,v);
    }
}

unsigned term_t::count() const
{
    return impl_.get()?impl_->getReferenceCount():0;
}

void term_t::reset()
{
    impl_ = nullptr;
}

void term_t::append_arg(const term_t &v)
{

    if(impl_.get())
    {
        impl_->append_arg(v);
    }
}

term_t parse_state_term(const String &v)
{
    parse_state_delegate d;   
    term_t *t = (term_t *)parse_term(&d,v.toUTF8());
    if(!t) return term_t();
    term_t t2(*t);
    delete t;
    return t2;
}
