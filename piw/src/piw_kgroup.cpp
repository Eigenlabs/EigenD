
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

#include <piw/piw_kgroup.h>
#include <piw/piw_keys.h>
#include <picross/pic_ref.h>

namespace
{
    struct mapping_t
    {
        pic::lckmap_t<unsigned,unsigned>::lcktype forward;
        pic::lckmap_t<unsigned,unsigned>::lcktype reverse;
        unsigned max_in;
    };
};

struct piw::kgroup_mapper_t::impl_t: virtual pic::tracked_t, virtual pic::lckobject_t
{
    ~impl_t()
    {
        tracked_invalidate();
    }
    
    unsigned get_key(const piw::data_nb_t &key, const piw::data_nb_t &geo)
    {
        unsigned row = (unsigned)key.as_tuple_value(0).as_float();
        unsigned col = (unsigned)key.as_tuple_value(1).as_float();
        
        if(row>geo.as_tuplelen())
        {
            return 0;
        }

        unsigned key_number = 0;
        for(unsigned i=1; i<row; ++i)
        {
            key_number += geo.as_tuple_value(i).as_long();
        }

        key_number += col;

        return key_number;
    }

    const piw::data_nb_t get_physical_key(const piw::data_nb_t &key, unsigned long long t)
    {
        unsigned row = (unsigned)key.as_tuple_value(0).as_float();
        unsigned col = (unsigned)key.as_tuple_value(1).as_float();

        unsigned offset = 0;
        if(row<=rowoffset_.get().as_tuplelen())
        {
            piw::data_nb_t offset_val = rowoffset_.get().as_tuple_value(row-1);
            if (!offset_val.is_null())
            {
                offset = offset_val.as_long();
            }
        }

        col = col - offset;

        piw::data_nb_t d = piw::tuplenull_nb(t);
        d = piw::tupleadd_nb(d, piw::makefloat_nb(row,t));
        d = piw::tupleadd_nb(d, piw::makefloat_nb(col,t));
        return d;
    }

    const piw::data_nb_t get_musical_key(unsigned key, unsigned long long t)
    {
        return piw::key_position(key, courselen_.get(), t);
    }

    piw::data_nb_t forward_mapping(const piw::data_nb_t &in)
    {
        pic::flipflop_t<mapping_t>::guard_t g(mapping_);

        if(g.value().forward.empty() ||
           rowlen_.is_empty() || rowoffset_.is_empty() || courselen_.is_empty() ||
           !in.is_tuple() || in.as_tuplelen() < 6)
        {
            return in;
        }

        data_nb_t upstream_keynum = in.as_tuple_value(0);
        data_nb_t upstream_key = in.as_tuple_value(1);
        data_nb_t upstream_rowlen = in.as_tuple_value(2);

        if(!upstream_key.is_tuple() || upstream_key.as_tuplelen() < 2 ||
           !upstream_rowlen.is_tuple())
        {
            return in;
        }

        unsigned key = get_key(upstream_key, upstream_rowlen);
        if(!key)
        {
            return piw::makenull_nb(0);
        }

        pic::lckmap_t<unsigned,unsigned>::lcktype::const_iterator i;

        if((i=g.value().forward.find(key))!=g.value().forward.end())
        {
            unsigned long long t = in.time();
            piw::data_nb_t physical_key = get_physical_key(upstream_key, t);
            piw::data_nb_t musical_key = get_musical_key(i->second, t);
            piw::data_nb_t result = piw::tuplenull_nb(t);
            result = piw::tupleadd_nb(result, piw::makelong_nb(get_key(physical_key, rowlen_), t));
            result = piw::tupleadd_nb(result, physical_key);
            result = piw::tupleadd_nb(result, rowlen_);
            result = piw::tupleadd_nb(result, piw::makelong_nb(i->second,t));
            result = piw::tupleadd_nb(result, musical_key);
            result = piw::tupleadd_nb(result, courselen_);
            return result;
        }

        return piw::makenull_nb(0);
    }

    piw::data_nb_t reverse_mapping(const piw::data_nb_t &in)
    {
        pic::flipflop_t<mapping_t>::guard_t g(mapping_);

        if(g.value().forward.empty())
        {
            return in;
        }

        unsigned char *dp;
        unsigned result_size=std::max(g.value().max_in,in.host_length());
        piw::data_nb_t result = makeblob_nb(in.time(),result_size,&dp);

        unsigned char* status=(unsigned char*)in.as_blob();
        unsigned status_size=in.host_length();
        memset(dp,BCTSTATUS_OFF,result_size);

        pic::lckmap_t<unsigned,unsigned>::lcktype::const_iterator i;
        for(i = g.value().forward.begin();i!=g.value().forward.end();++i)
        {
            if(i->second<=status_size)
            {
                dp[i->first-1]=status[i->second-1];
            }
        }

        return result;
    }

    void set_rowoffset(data_t rowoffset)
    {
        rowoffset_.set_normal(rowoffset);
    }

    void set_rowlen(data_t rowlen)
    {
        rowlen_.set_normal(rowlen);
    }

    void set_courselen(data_t courselen)
    {
        courselen_.set_normal(courselen);
    }

    pic::flipflop_t<mapping_t> mapping_;
    piw::dataholder_nb_t rowoffset_;
    piw::dataholder_nb_t rowlen_;
    piw::dataholder_nb_t courselen_;
};

piw::kgroup_mapper_t::kgroup_mapper_t(): impl_(new impl_t)
{
}

piw::kgroup_mapper_t::~kgroup_mapper_t()
{
    delete impl_;
}

piw::d2d_nb_t piw::kgroup_mapper_t::key_filter()
{
    return piw::d2d_nb_t::method(impl_,&impl_t::forward_mapping);
}

piw::d2d_nb_t piw::kgroup_mapper_t::light_filter()
{
    return piw::d2d_nb_t::method(impl_,&impl_t::reverse_mapping);
}

void piw::kgroup_mapper_t::clear_mapping()
{
    impl_->mapping_.alternate().forward.clear();
    impl_->mapping_.alternate().reverse.clear();
    impl_->mapping_.alternate().max_in = 0;
}

void piw::kgroup_mapper_t::set_mapping(unsigned in, unsigned out)
{
    impl_->mapping_.alternate().forward.insert(std::make_pair(in,out));
    impl_->mapping_.alternate().reverse.insert(std::make_pair(out,in));
    impl_->mapping_.alternate().max_in = std::max(impl_->mapping_.alternate().max_in,in);
}

void piw::kgroup_mapper_t::activate_mapping()
{
    impl_->mapping_.exchange();
}

void piw::kgroup_mapper_t::set_rowlen(data_t rowlen)
{
    impl_->set_rowlen(rowlen);
}

void piw::kgroup_mapper_t::set_rowoffset(data_t rowoffset)
{
    impl_->set_rowoffset(rowoffset);
}

void piw::kgroup_mapper_t::set_courselen(data_t courselen)
{
    impl_->set_courselen(courselen);
}
