
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

    piw::data_nb_t forward_mapping(const piw::data_nb_t &in)
    {
        pic::flipflop_t<mapping_t>::guard_t g(mapping_);

        if(g.value().forward.empty())
        {
            return in;
        }

        unsigned key = in.as_path()[in.as_pathlen()-1];

        pic::lckmap_t<unsigned,unsigned>::lcktype::const_iterator i;

        if((i=g.value().forward.find(key))!=g.value().forward.end())
        {
            piw::data_nb_t d = piw::pathreplacegrist_nb(in,i->second);
            return d.restamp(in.time());
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

    pic::flipflop_t<mapping_t> mapping_;
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

