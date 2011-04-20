
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
#include <piw/piw_state.h>
#include <map>

struct piw::term_t::impl_t: public pic::atomic_counted_t
{
    impl_t(unsigned type): type_(type) { }
    virtual ~impl_t() { }

    virtual const char *pred() const { return 0; }
    virtual piw::data_t value() const { return piw::data_t(); }
    virtual unsigned arity() const { return 0; }
    virtual term_t arg(unsigned) const { return term_t(); }
    virtual void set_arg(unsigned,const term_t &) {}
    virtual void append_arg(const piw::term_t &v) {}
    virtual void render(std::ostringstream &) const = 0;

    unsigned type_;
};

namespace
{
    void render_string(std::ostringstream &s, const char *str)
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

    void render_generic(std::ostringstream &s, const piw::data_t &d)
    {
        s << '{';

        const unsigned char *wp = (const unsigned char *)d.wire_data();
        unsigned wl = d.wire_length();

        while(wl>0)
        {
            unsigned c = *wp;
            wp++; wl--;
            s << to_hex((c>>4)&0x0f) << to_hex(c&0x0f);
        }

        s << '}';
    }

    void render_data(std::ostringstream &s, const piw::data_t &d)
    {
        switch(d.type())
        {
            case BCTVTYPE_NULL: s << "None"; break;
            case BCTVTYPE_BOOL: s << (d.as_bool()?"True":"False"); break;
            case BCTVTYPE_INT: s << d.as_long(); break;
            case BCTVTYPE_FLOAT: s << d.as_denorm() << 'f'; break;
            case BCTVTYPE_STRING: render_string(s,d.as_string()); break;
            default: render_generic(s,d); break;
        }
    }

    struct term_atom_t: piw::term_t::impl_t
    {
        term_atom_t(const piw::data_t &value): impl_t(PIW_TERM_ATOM), value_(value) {}
        ~term_atom_t() {}
        piw::data_t value() const { return value_; }

        void render(std::ostringstream &stream) const { render_data(stream,value_); }

        piw::data_t value_;
    };

    struct term_list_t: piw::term_t::impl_t
    {
        term_list_t(unsigned len): impl_t(PIW_TERM_LIST), values_(len) {}
        term_list_t(): impl_t(PIW_TERM_LIST) {}
        ~term_list_t() { values_.resize(0); }

        unsigned arity() const { return values_.size(); }
        piw::term_t arg(unsigned n) const { return values_[n]; }
        void set_arg(unsigned n,const piw::term_t &v) { values_[n] = v; }

        void append_arg(const piw::term_t &v)
        {
            unsigned n = values_.size();
            values_.resize(n+1);
            values_[n].reset();
            values_[n]=v;
        }

        void render(std::ostringstream &stream) const
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

        pic::lckvector_t<piw::term_t>::nbtype values_;
    };

    struct term_pred_t: piw::term_t::impl_t
    {
        term_pred_t(const std::string &pred, unsigned len): impl_t(PIW_TERM_PRED), pred_(pred), values_(len) { }
        term_pred_t(const std::string &pred): impl_t(PIW_TERM_PRED), pred_(pred) { }
        ~term_pred_t() { values_.resize(0); }

        unsigned arity() const { return values_.size(); }
        piw::term_t arg(unsigned n) const { return values_[n]; }
        void set_arg(unsigned n,const piw::term_t &v) { values_[n] = v; }

        void append_arg(const piw::term_t &v)
        {
            unsigned n = values_.size();
            values_.resize(n+1);
            values_[n].reset();
            values_[n]=v;
        }

        const char *pred() const { return pred_.c_str(); }

        void render(std::ostringstream &stream) const
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

        std::string pred_;
        pic::lckvector_t<piw::term_t>::nbtype values_;
    };
}

piw::term_t::term_t()
{
}

piw::term_t::~term_t()
{
}

const piw::term_t::impl_t *piw::term_t::impl() const
{
    return impl_.ptr();
}

std::string piw::term_t::render() const
{
    if(!impl_.isvalid())
    {
        return "";
    }

    std::ostringstream stream;
    impl_->render(stream);
    return stream.str();
}

piw::term_t::term_t(const piw::data_t &v)
{
    impl_ = pic::ref(new term_atom_t(v));
}

piw::term_t::term_t(unsigned len)
{
    if(len)
        impl_ = pic::ref(new term_list_t(len));
    else
        impl_ = pic::ref(new term_list_t);
}

piw::term_t::term_t(const std::string &pred,unsigned len)
{
    if(len)
        impl_ = pic::ref(new term_pred_t(pred,len));
    else
        impl_ = pic::ref(new term_pred_t(pred));
}

piw::term_t::term_t(const term_t &t): impl_(t.impl_)
{
}

piw::term_t &piw::term_t::operator=(const term_t &t)
{
    impl_=t.impl_;
    return *this;
}

unsigned piw::term_t::type() const
{
    return impl_.isvalid()?impl_->type_:PIW_TERM_NULL;

}

unsigned piw::term_t::arity() const
{
    return impl_.isvalid()?impl_->arity():0;
}

const char *piw::term_t::pred() const
{
    return impl_.isvalid()?impl_->pred():0;
}

piw::data_t piw::term_t::value() const
{
    return impl_.isvalid()?impl_->value():piw::data_t();
}

piw::term_t piw::term_t::arg(unsigned n) const
{
    return impl_.isvalid()?impl_->arg(n):term_t();
}

void piw::term_t::set_arg(unsigned n, const term_t &v)
{
    if(impl_.isvalid())
    {
        impl_->set_arg(n,v);
    }
}

unsigned piw::term_t::count() const
{
    return impl_.isvalid()?impl_->count():0;
}

void piw::term_t::reset()
{
    impl_.reset();
}

void piw::term_t::append_arg(const term_t &v)
{

    if(impl_.isvalid())
    {
        impl_->append_arg(v);
    }
}

namespace
{
    struct parse_state_delegate: piw::parser_delegate_t
    {
        virtual void destroy_any(void *t)
        {
            delete (piw::term_t *)t;
        }

        virtual void *make_collection()
        {
            return new piw::term_t(0);
        }

        virtual void add_arg(void *t, void *a)
        {
            ((piw::term_t *)t)->append_arg(*(piw::term_t *)a);
        }

        virtual void *make_term(const char *p, unsigned pl, void *c_)
        {
            piw::term_t *c = (piw::term_t *)c_;
            piw::term_t *t = new piw::term_t(std::string(p,pl),c->arity());

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
            return new piw::term_t(piw::makelong(v,0));
        }

        virtual void *make_float(float v)
        {
            return new piw::term_t(piw::makefloat_bounded_units(BCTUNIT_GLOBAL,fabs(v),-fabs(v),0,v,0));
        }

        virtual void *make_bool(bool v)
        {
            return new piw::term_t(piw::makebool(v,0));
        }

        virtual void *make_none()
        {
            return new piw::term_t(piw::data_t());
        }

        virtual void *make_string(const char *s, unsigned sl)
        {
            return new piw::term_t(piw::makestring_len(s,sl));
        }

        virtual void *make_variable(const char *, unsigned)
        {
            return 0;
        }

        virtual void *make_subst(const char *, unsigned, const char *, unsigned)
        {
            return 0;
        }

        virtual void *make_generic(const piw::data_t &d)
        {
            return new piw::term_t(d);
        }

    };
};

piw::term_t piw::parse_state_term(const std::string &v)
{
    parse_state_delegate d;   
    piw::term_t *t = (piw::term_t *)piw::parse_term(&d,v.c_str());
    if(!t) return piw::term_t();
    piw::term_t t2(*t);
    delete t;
    return t2;
}
