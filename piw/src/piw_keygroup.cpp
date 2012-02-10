
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

#include <piw/piw_keygroup.h>
#include <piw/piw_keys.h>
#include <piw/piw_status.h>
#include <picross/pic_ref.h>

#define KEYGROUP_MAPPER_DEBUG 0

namespace
{
    typedef std::pair<int,int> coordinate_t;
    typedef pic::lckmap_t<coordinate_t,std::pair<coordinate_t,int> >::lcktype forward_mapping_t;
    typedef pic::lckmap_t<coordinate_t,coordinate_t>::lcktype reverse_mapping_t;

    struct mapping_t
    {
        forward_mapping_t forward;
        reverse_mapping_t reverse;
    };
};


/*
 * keygroup_mapper_t
 */

struct piw::keygroup_mapper_t::impl_t: virtual pic::tracked_t, virtual pic::lckobject_t
{
    ~impl_t()
    {
        tracked_invalidate();
    }

    piw::data_nb_t forward_mapping(const piw::data_nb_t &in)
    {
#if KEYGROUP_MAPPER_DEBUG>0
        pic::logmsg() << "forward_mapping in " << in;
#endif
        pic::flipflop_t<mapping_t>::guard_t gp(physical_mapping_);
        pic::flipflop_t<mapping_t>::guard_t gm(musical_mapping_);

        if(gp.value().forward.empty() ||
           gm.value().forward.empty())
        {
#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "forward_mapping out " << in;
#endif
            return in;
        }

        float row,col,course,key;
        piw::hardness_t hardness;
        if(!piw::decode_key(in,0,&row,&col,0,&course,&key,&hardness))
        {
#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "forward_mapping out " << in;
#endif
            return in;
        }

        const coordinate_t coord_phys = coordinate_t(row,col);
        const coordinate_t coord_mus = coordinate_t(course,key);
        forward_mapping_t::const_iterator ip = gp.value().forward.find(coord_phys);
        forward_mapping_t::const_iterator im = gm.value().forward.find(coord_mus);
        if(ip!=gp.value().forward.end() && im!=gm.value().forward.end())
        {
            piw::data_nb_t result = piw::makekey(ip->second.second,ip->second.first.first,ip->second.first.second,im->second.second,im->second.first.first,im->second.first.second,hardness,in.time());

#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "forward_mapping out " << result;
#endif
            return result;
        }

#if KEYGROUP_MAPPER_DEBUG>0
        pic::logmsg() << "forward_mapping out null";
#endif
        return piw::makenull_nb(0);
    }

    piw::data_nb_t reverse_mapping(const piw::data_nb_t &in)
    {
#if KEYGROUP_MAPPER_DEBUG>0
        pic::logmsg() << "reverse_mapping in " << in;
#endif
        pic::flipflop_t<mapping_t>::guard_t gm(musical_mapping_);
        pic::flipflop_t<mapping_t>::guard_t gp(physical_mapping_);

        if(gp.value().reverse.empty() ||
           gm.value().reverse.empty() ||
           !in.is_blob())
        {
#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "reverse_mapping out " << in;
#endif
            return in;
        }

        unsigned char* in_buffer = (unsigned char*)in.as_blob();
        unsigned in_size = in.host_length();

        unsigned char *out_buffer;
        piw::data_nb_t out = makeblob_nb(in.time(),in_size,&out_buffer);

        while(in_size >= 5)
        {
            int ir = piw::statusdata_t::c2int(&in_buffer[0]);
            int ic = piw::statusdata_t::c2int(&in_buffer[2]);
            bool im = in_buffer[4]>>7;
            unsigned xr = 0;
            unsigned xc = 0;

#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "reverse_mapping in " << ir << ", " << ic << ", " << im;
#endif
            const coordinate_t coord = coordinate_t(ir,ic);

            if(im)
            {
                reverse_mapping_t::const_iterator im = gm.value().reverse.find(coord);
                if(im!=gm.value().reverse.end())
                {
                    xr = im->second.first;
                    xc = im->second.second;
                }
            }
            else
            {
                reverse_mapping_t::const_iterator ip = gp.value().reverse.find(coord);
                if(ip!=gp.value().reverse.end())
                {
                    xr = ip->second.first;
                    xc = ip->second.second;
                }
            }

#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "reverse_mapping out " << xr << ", " << xc << ", " << im;
#endif

            piw::statusdata_t::int2c(xr,&out_buffer[0]);
            piw::statusdata_t::int2c(xc,&out_buffer[2]);
            out_buffer[4] = in_buffer[4];

            out_buffer+=5;
            in_buffer+=5;
            in_size-=5;
        }

#if KEYGROUP_MAPPER_DEBUG>0
        pic::logmsg() << "reverse_mapping out " << out;
#endif
        return out;
    }

    pic::flipflop_t<mapping_t> physical_mapping_;
    pic::flipflop_t<mapping_t> musical_mapping_;
};

piw::keygroup_mapper_t::keygroup_mapper_t(): impl_(new impl_t)
{
}

piw::keygroup_mapper_t::~keygroup_mapper_t()
{
    delete impl_;
}

piw::d2d_nb_t piw::keygroup_mapper_t::key_filter()
{
    return piw::d2d_nb_t::method(impl_,&impl_t::forward_mapping);
}

piw::d2d_nb_t piw::keygroup_mapper_t::light_filter()
{
    return piw::d2d_nb_t::method(impl_,&impl_t::reverse_mapping);
}

void piw::keygroup_mapper_t::clear_mapping()
{
    clear_physical_mapping();
    clear_musical_mapping();
}

void piw::keygroup_mapper_t::activate_mapping()
{
    activate_physical_mapping();
    activate_musical_mapping();
}

void piw::keygroup_mapper_t::clear_physical_mapping()
{
    impl_->physical_mapping_.alternate().forward.clear();
    impl_->physical_mapping_.alternate().reverse.clear();
}

void piw::keygroup_mapper_t::set_physical_mapping(int row_in, int column_in, int row_out, int column_out, int rel_row_out, int rel_column_out, unsigned sequential_out)
{
    coordinate_t in = coordinate_t(row_in,column_in);
    coordinate_t out = coordinate_t(row_out,column_out);
    coordinate_t out_rel = coordinate_t(rel_row_out,rel_column_out);
    coordinate_t out_rel_row = coordinate_t(rel_row_out,column_out);
    coordinate_t out_rel_column = coordinate_t(row_out,rel_column_out);
    coordinate_t out_seq = coordinate_t(0,sequential_out);

#if KEYGROUP_MAPPER_DEBUG>0
    pic::logmsg() << "set_physical_mapping out (" << row_in << ","  << column_in << ") (" << row_out << "," << column_out << ") (" << rel_row_out << "," << rel_column_out << ") " << sequential_out;
#endif

    impl_->physical_mapping_.alternate().forward.insert(std::make_pair(in,std::make_pair(out,sequential_out)));
    impl_->physical_mapping_.alternate().reverse.insert(std::make_pair(out,in));
    impl_->physical_mapping_.alternate().reverse.insert(std::make_pair(out_rel,in));
    impl_->physical_mapping_.alternate().reverse.insert(std::make_pair(out_rel_row,in));
    impl_->physical_mapping_.alternate().reverse.insert(std::make_pair(out_rel_column,in));
    impl_->physical_mapping_.alternate().reverse.insert(std::make_pair(out_seq,in));
}

void piw::keygroup_mapper_t::activate_physical_mapping()
{
    impl_->physical_mapping_.exchange();
}

void piw::keygroup_mapper_t::clear_musical_mapping()
{
    impl_->musical_mapping_.alternate().forward.clear();
    impl_->musical_mapping_.alternate().reverse.clear();
}

void piw::keygroup_mapper_t::set_musical_mapping(int course_in, int key_in, int course_out, int key_out, int rel_course_out, int rel_key_out, unsigned sequential_out)
{
    coordinate_t in = coordinate_t(course_in,key_in);
    coordinate_t out = coordinate_t(course_out,key_out);
    coordinate_t out_rel = coordinate_t(rel_course_out,rel_key_out);
    coordinate_t out_rel_course = coordinate_t(rel_course_out,key_out);
    coordinate_t out_rel_key = coordinate_t(course_out,rel_key_out);
    coordinate_t out_seq = coordinate_t(0,sequential_out);

#if KEYGROUP_MAPPER_DEBUG>0
    pic::logmsg() << "set_musical_mapping out (" << course_in << ","  << key_in << ") (" << course_out << "," << key_out << ") (" << rel_course_out << "," << rel_key_out << ") " << sequential_out;
#endif

    impl_->musical_mapping_.alternate().forward.insert(std::make_pair(in,std::make_pair(out,sequential_out)));
    impl_->musical_mapping_.alternate().reverse.insert(std::make_pair(out,in));
    impl_->musical_mapping_.alternate().reverse.insert(std::make_pair(out_rel,in));
    impl_->musical_mapping_.alternate().reverse.insert(std::make_pair(out_rel_course,in));
    impl_->musical_mapping_.alternate().reverse.insert(std::make_pair(out_rel_key,in));
    impl_->musical_mapping_.alternate().reverse.insert(std::make_pair(out_seq,in));
}

void piw::keygroup_mapper_t::activate_musical_mapping()
{
    impl_->musical_mapping_.exchange();
}


/**
 * modekey_handler_t
 **/

struct piw::modekey_handler_t::impl_t: virtual pic::tracked_t, virtual pic::lckobject_t
{
    impl_t() : row_(0), column_(0) {}

    ~impl_t()
    {
        tracked_invalidate();
    }

    bool key_filter(const piw::data_nb_t &d)
    {
        float row,col;
        piw::hardness_t hardness;
        if(row_ && column_ && !upstream_rowlen_.is_empty() && piw::decode_key(d,0,&row,&col,0,0,0,&hardness) && hardness > 0)
        {
            piw::data_nb_t geo = upstream_rowlen_.get();

            if(geo.is_tuple())
            {
                int mode_row = row_;
                int mode_col = column_;

#if KEYGROUP_MAPPER_DEBUG>0
                pic::logmsg() << "modekey key_filter " << d << " (" << mode_row << "," << mode_col << ")";
#endif

                if(mode_row<0)
                {
                    mode_row = geo.as_tuplelen() + mode_row + 1;
                }

                if(mode_col<0 && mode_row<=int(geo.as_tuplelen()))
                {
                    mode_col = geo.as_tuple_value(mode_row-1).as_long();
                }

                if(int(row)==mode_row && int(col)==mode_col)
                {
                    return true;
                }
            }
        }

        return false;
    }

    void set_modekey(int row, int column)
    {
        row_ = row;
        column_ = column;
    }

    int row_;
    int column_;
    piw::dataholder_nb_t upstream_rowlen_;
};

piw::modekey_handler_t::modekey_handler_t(): impl_(new impl_t)
{
    set_modekey(0, 0);
}

piw::modekey_handler_t::~modekey_handler_t()
{
    delete impl_;
}

piw::d2b_nb_t piw::modekey_handler_t::key_filter()
{
    return piw::d2b_nb_t::method(impl_,&impl_t::key_filter);
}

static int __set_modekey(void *i_, void *r_, void *c_)
{
    piw::modekey_handler_t::impl_t *i = (piw::modekey_handler_t::impl_t *)i_;
    int row = *(int *)r_;
    int column = *(int *)c_;
    i->set_modekey(row,column);
    return 0;
}

void piw::modekey_handler_t::set_modekey(int row, int column) { piw::tsd_fastcall3(__set_modekey,impl_,&row,&column); }
void piw::modekey_handler_t::set_upstream_rowlength(const piw::data_t &rowlen) { impl_->upstream_rowlen_.set_normal(rowlen); }
