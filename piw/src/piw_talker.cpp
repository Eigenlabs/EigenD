
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

        piw::data_nb_t operator()(const piw::data_nb_t &x) const
        {
            return data_.get();
        }

        piw::dataholder_nb_t data_;
    };
};

piw::d2d_nb_t piw::d2d_const(const piw::data_t &value)
{
    return d2d_nb_t::callable(d2d_const_functor(value));
}
