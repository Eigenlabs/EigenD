
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
    typedef std::pair<int,int> coord_t;
    typedef pic::lckmap_t<coord_t,coord_t>::lcktype forward_mapping_t;
    typedef pic::lckmap_t<coord_t,coord_t>::lcktype reverse_mapping_t;

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
        pic::flipflop_t<mapping_t>::guard_t guard_phys(physical_mapping_);
        pic::flipflop_t<mapping_t>::guard_t guard_mus(musical_mapping_);

        if(guard_phys.value().forward.empty() ||
           guard_mus.value().forward.empty())
        {
#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "forward_mapping empty out " << in;
#endif
            return in;
        }

        float column,row,course,key;
        piw::hardness_t hardness;
        if(!piw::decode_key(in,&column,&row,&course,&key,&hardness))
        {
#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "forward_mapping not key out " << in;
#endif
            return in;
        }

        const coord_t coord_phys = coord_t(column,row);
        const coord_t coord_mus = coord_t(course,key);
        forward_mapping_t::const_iterator in_phys = guard_phys.value().forward.find(coord_phys);
        forward_mapping_t::const_iterator in_mus = guard_mus.value().forward.find(coord_mus);
        if(in_phys!=guard_phys.value().forward.end() && in_mus!=guard_mus.value().forward.end())
        {
            piw::data_nb_t result = piw::makekey(in_phys->second.first,in_phys->second.second,in_mus->second.first,in_mus->second.second,hardness,in.time());

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
        pic::flipflop_t<mapping_t>::guard_t guard_mus(musical_mapping_);
        pic::flipflop_t<mapping_t>::guard_t guard_phys(physical_mapping_);

        if(guard_phys.value().reverse.empty() ||
           guard_mus.value().reverse.empty() ||
           !in.is_blob())
        {
#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "reverse_mapping empty out " << in;
#endif
            return in;
        }

        unsigned char* in_buffer = (unsigned char*)in.as_blob();
        unsigned in_size = in.host_length();

        unsigned char *out_buffer;
        piw::data_nb_t out = makeblob_nb(in.time(),in_size,&out_buffer);

        while(in_size >= 6)
        {
            piw::statusdata_t in_status = piw::statusdata_t::from_bytes(in_buffer);
            int in_x = in_status.coordinate_.x_;
            int in_y = in_status.coordinate_.y_;
            if(in_status.coordinate_.endrel_x_)
            {
                if(in_x < 0) in_x -= piw::MAX_KEY+1;
                else in_x += piw::MAX_KEY+1;
            }
            if(in_status.coordinate_.endrel_y_)
            {
                if(in_y < 0) in_y -= piw::MAX_KEY+1;
                else in_y += piw::MAX_KEY+1;
            }
            int out_x = 0;
            int out_y = 0;

#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "reverse_mapping values in " << in_x << ", " << in_y << ", " << in_status.musical_;
#endif
            const coord_t coord = coord_t(in_x,in_y);

            if(in_status.musical_)
            {
                reverse_mapping_t::const_iterator in_mus = guard_mus.value().reverse.find(coord);
                if(in_mus!=guard_mus.value().reverse.end())
                {
                    out_x = in_mus->second.first;
                    out_y = in_mus->second.second;
                }
            }
            else
            {
                reverse_mapping_t::const_iterator in_phys = guard_phys.value().reverse.find(coord);
                if(in_phys!=guard_phys.value().reverse.end())
                {
                    out_x = in_phys->second.first;
                    out_y = in_phys->second.second;
                }
            }

#if KEYGROUP_MAPPER_DEBUG>0
            pic::logmsg() << "reverse_mapping values out " << out_x << ", " << out_y << ", " << in_status.musical_;
#endif

            piw::statusdata_t(in_status.musical_,piw::coordinate_t(out_x,out_y),in_status.status_).to_bytes(out_buffer);

            out_buffer+=6;
            in_buffer+=6;
            in_size-=6;
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

void piw::keygroup_mapper_t::set_physical_mapping(int column_in, int row_in, int rel_column_in, int rel_row_in, int column_out, int row_out, int rel_column_out, int rel_row_out)
{
#if KEYGROUP_MAPPER_DEBUG>0
    pic::logmsg() << "set_physical_mapping in (" << column_in << ","  << row_in << "), rel in (" << rel_column_in << "," << rel_row_in << "), out (" << column_out << "," << row_out << "), rel out (" << rel_column_out << "," << rel_row_out << ")";
#endif

    if(abs(column_in) > piw::MAX_KEY || abs(row_in) > piw::MAX_KEY || abs(column_out) > piw::MAX_KEY || abs(row_out) > piw::MAX_KEY)
    {
        return;
    }

    coord_t in = coord_t(column_in,row_in);
    coord_t out = coord_t(column_out,row_out);
    impl_->physical_mapping_.alternate().forward.insert(std::make_pair(in,out));
    impl_->physical_mapping_.alternate().reverse.insert(std::make_pair(out,in));

    if(rel_column_in < 0) rel_column_in -= piw::MAX_KEY+1;
    else rel_column_in += piw::MAX_KEY+1;
    if(rel_row_in < 0) rel_row_in -= piw::MAX_KEY+1;
    else rel_row_in += piw::MAX_KEY+1;

    coord_t in_rel = coord_t(rel_column_in,rel_row_in);
    coord_t in_rel_column = coord_t(rel_column_in,row_in);
    coord_t in_rel_row = coord_t(column_in,rel_row_in);
    impl_->physical_mapping_.alternate().forward.insert(std::make_pair(in_rel,out));
    impl_->physical_mapping_.alternate().forward.insert(std::make_pair(in_rel_column,out));
    impl_->physical_mapping_.alternate().forward.insert(std::make_pair(in_rel_row,out));

    if(rel_column_out < 0) rel_column_out -= piw::MAX_KEY+1;
    else rel_column_out += piw::MAX_KEY+1;
    if(rel_row_out < 0) rel_row_out -= piw::MAX_KEY+1;
    else rel_row_out += piw::MAX_KEY+1;

    coord_t out_rel = coord_t(rel_column_out,rel_row_out);
    coord_t out_rel_column = coord_t(rel_column_out,row_out);
    coord_t out_rel_row = coord_t(column_out,rel_row_out);
    impl_->physical_mapping_.alternate().reverse.insert(std::make_pair(out_rel,in));
    impl_->physical_mapping_.alternate().reverse.insert(std::make_pair(out_rel_column,in));
    impl_->physical_mapping_.alternate().reverse.insert(std::make_pair(out_rel_row,in));
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

void piw::keygroup_mapper_t::set_musical_mapping(int course_in, int key_in, int rel_course_in, int rel_key_in, int course_out, int key_out, int rel_course_out, int rel_key_out)
{
#if KEYGROUP_MAPPER_DEBUG>0
    pic::logmsg() << "set_musical_mapping in (" << course_in << ","  << key_in << "), rel in (" << rel_course_in << "," << rel_key_in << "), out (" << course_out << "," << key_out << "), rel out (" << rel_course_out << "," << rel_key_out << ")";
#endif

    if(abs(course_in) > piw::MAX_KEY || abs(key_in) > piw::MAX_KEY || abs(course_out) > piw::MAX_KEY || abs(key_out) > piw::MAX_KEY)
    {
        return;
    }

    coord_t in = coord_t(course_in,key_in);
    coord_t out = coord_t(course_out,key_out);
    impl_->musical_mapping_.alternate().forward.insert(std::make_pair(in,out));
    impl_->musical_mapping_.alternate().reverse.insert(std::make_pair(out,in));

    if(rel_course_in < 0) rel_course_in -= piw::MAX_KEY+1;
    else rel_course_in += piw::MAX_KEY+1;
    if(rel_key_in < 0) rel_key_in -= piw::MAX_KEY+1;
    else rel_key_in += piw::MAX_KEY+1;

    coord_t in_rel = coord_t(rel_course_in,rel_key_in);
    coord_t in_rel_course = coord_t(rel_course_in,key_in);
    coord_t in_rel_key = coord_t(course_in,rel_key_in);
    impl_->musical_mapping_.alternate().forward.insert(std::make_pair(in_rel,out));
    impl_->musical_mapping_.alternate().forward.insert(std::make_pair(in_rel_course,out));
    impl_->musical_mapping_.alternate().forward.insert(std::make_pair(in_rel_key,out));

    if(rel_course_out < 0) rel_course_out -= piw::MAX_KEY+1;
    else rel_course_out += piw::MAX_KEY+1;
    if(rel_key_out < 0) rel_key_out -= piw::MAX_KEY+1;
    else rel_key_out += piw::MAX_KEY+1;

    coord_t out_rel = coord_t(rel_course_out,rel_key_out);
    coord_t out_rel_course = coord_t(rel_course_out,key_out);
    coord_t out_rel_key = coord_t(course_out,rel_key_out);
    impl_->musical_mapping_.alternate().reverse.insert(std::make_pair(out_rel,in));
    impl_->musical_mapping_.alternate().reverse.insert(std::make_pair(out_rel_course,in));
    impl_->musical_mapping_.alternate().reverse.insert(std::make_pair(out_rel_key,in));
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
    impl_t() {}

    ~impl_t()
    {
        tracked_invalidate();
    }

    bool key_filter(const piw::data_nb_t &d)
    {
        float column, row;
        piw::hardness_t hardness;
        if(key_.is_valid() && !upstream_columnlen_.is_empty() && piw::decode_key(d,&column,&row,0,0,&hardness) && hardness > 0)
        {
            piw::data_nb_t geo = upstream_columnlen_.get();

            if(geo.is_tuple())
            {
                int mode_column = key_.x_;
                int mode_row = key_.y_;

#if KEYGROUP_MAPPER_DEBUG>0
                pic::logmsg() << "modekey key_filter " << d << " (" << mode_column << "," << mode_row << ")";
#endif

                if(key_.endrel_x_)
                {
                    mode_column = geo.as_tuplelen() - mode_column + 1;
                }

                if(key_.endrel_y_ && mode_column<=int(geo.as_tuplelen()))
                {
                    mode_row = geo.as_tuple_value(mode_column-1).as_long() - mode_row + 1;
                }

                if(int(column)==mode_column && int(row)==mode_row)
                {
                    return true;
                }
            }
        }

        return false;
    }

    void set_modekey(piw::coordinate_t key)
    {
        key_ = key;
    }

    piw::coordinate_t key_;
    piw::dataholder_nb_t upstream_columnlen_;
};

piw::modekey_handler_t::modekey_handler_t(): impl_(new impl_t)
{
    set_modekey(piw::coordinate_t());
}

piw::modekey_handler_t::~modekey_handler_t()
{
    delete impl_;
}

piw::d2b_nb_t piw::modekey_handler_t::key_filter()
{
    return piw::d2b_nb_t::method(impl_,&impl_t::key_filter);
}

static int __set_modekey(void *i_, void *k_)
{
    piw::modekey_handler_t::impl_t *i = (piw::modekey_handler_t::impl_t *)i_;
    piw::coordinate_t k = *(piw::coordinate_t *)k_;
    i->set_modekey(k);
    return 0;
}

void piw::modekey_handler_t::set_modekey(const piw::coordinate_t &key) { piw::tsd_fastcall(__set_modekey,impl_,(void *)&key); }
void piw::modekey_handler_t::set_upstream_columnlength(const piw::data_t &columnlen) { impl_->upstream_columnlen_.set_normal(columnlen); }
