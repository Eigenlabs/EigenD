
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

#include <piw/piw_talker.h>
#include <piw/piw_keys.h>
#include <picross/pic_ref.h>

namespace
{
    struct d2d_const_functor
    {
        d2d_const_functor(const piw::data_t &d)
        {
            data_.set_normal(d);
        }

        d2d_const_functor()
        {
        }

        d2d_const_functor(const d2d_const_functor &o): data_(o.data_)
        {
        }

        d2d_const_functor &operator=(const d2d_const_functor &o)
        {
            data_ = o.data_;
            return *this;
        }

        bool operator==(const d2d_const_functor &o) const
        {
            return data_.get()==o.data_.get();
        }

        piw::data_nb_t operator()(const piw::data_nb_t &) const
        {
            return data_.get();
        }

        piw::dataholder_nb_t data_;
    };
};

struct piw::talker_mapper_t::impl_t: virtual pic::tracked_t, virtual pic::lckobject_t
{
    ~impl_t()
    {
        tracked_invalidate();
    }
    
    piw::data_nb_t mapping(const piw::data_nb_t &in)
    {
        piw::data_nb_t out;

        unsigned l = in.as_pathgristlen();

        if(l>0)
        {
            if(in.as_pathgrist()[0]==in_)
            {
                out = piw::pathprepend_grist_nb(piw::pathgristpretruncate_nb(in),out_);
                
            }
        }

        //pic::logmsg() << "talker mapper mapping " << in << "->" << out;
        return out;
    }

    unsigned in_;
    unsigned out_;
};

piw::talker_mapper_t::talker_mapper_t(): impl_(new impl_t)
{
}

piw::talker_mapper_t::~talker_mapper_t()
{
    delete impl_;
}

piw::d2d_nb_t piw::talker_mapper_t::key_filter()
{
    return piw::d2d_nb_t::method(impl_,&impl_t::mapping);
}

void piw::talker_mapper_t::set_mapping(unsigned in, unsigned out)
{
    impl_->in_ = in;
    impl_->out_ = out;
}

piw::d2d_nb_t piw::d2d_const(const piw::data_t &value)
{
    return d2d_nb_t::callable(d2d_const_functor(value));
}
