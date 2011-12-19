
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

    static inline void int2c(int r, unsigned char *o)
    {
        long k = r;

        if(r<0) 
        {
            k = (int)0x10000+r;
        }

        o[0] = ((k>>8)&0xff);
        o[1] = (k&0xff);
    }

    static inline int c2int(unsigned char *c)
    {
        unsigned long cx = (c[0]<<8) | (c[1]);

        if(cx>0x7fff)
        {
            return ((long)cx)-0x10000;
        }

        return cx;
    }

};

struct piw::kgroup_mapper_t::impl_t: virtual pic::tracked_t, virtual pic::lckobject_t
{
    ~impl_t()
    {
        tracked_invalidate();
    }
    
    unsigned get_keynumber(const piw::data_nb_t &key, const piw::data_nb_t &geo)
    {
        return get_keynumber((unsigned)key.as_tuple_value(0).as_float(), (unsigned)key.as_tuple_value(1).as_float(), geo);
    }
    
    unsigned get_keynumber(unsigned row, unsigned col, const piw::data_nb_t &geo)
    {
        if(row>geo.as_tuplelen())
        {
            return 0;
        }

        unsigned key_number = 0;
        for(unsigned i=1; i<row; ++i)
        {
            key_number += geo.as_tuple_value(i-1).as_long();
        }

        key_number += col;

        return key_number;
    }

    const piw::data_nb_t get_physical_key(const piw::data_nb_t &key, unsigned long long t)
    {
        unsigned row = (unsigned)key.as_tuple_value(0).as_float();
        unsigned col = (unsigned)key.as_tuple_value(1).as_float();

        const piw::data_nb_t rowoffset = rowoffset_.get();

        unsigned col_offset = 0;
        if(row<=rowoffset.as_tuplelen())
        {
            piw::data_nb_t offset_val = rowoffset.as_tuple_value(row-1);
            if (!offset_val.is_null())
            {
                col_offset = offset_val.as_long();
            }
        }

        col = col - col_offset;

        unsigned row_offset = 0;
        for(unsigned i=1; i<=rowoffset.as_tuplelen() && i <= row; ++i)
        {
            if(rowoffset.as_tuple_value(i-1).is_null())
            {
                row_offset++;
            }
        }

        row = row - row_offset;

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
           !in.is_tuple() || in.as_tuplelen() != 4)
        {
            return in;
        }

        data_nb_t upstream_keynum = in.as_tuple_value(0);
        data_nb_t upstream_key = in.as_tuple_value(1);

        if(!upstream_key.is_tuple() || upstream_key.as_tuplelen() != 2 ||
           upstream_rowlen_.is_empty() || !upstream_rowlen_.get().is_tuple() ||
           !upstream_keynum.is_long())
        {
            return in;
        }

        pic::lckmap_t<unsigned,unsigned>::lcktype::const_iterator i;
        if((i=g.value().forward.find(upstream_keynum.as_long()))!=g.value().forward.end())
        {
            unsigned long long t = in.time();
            piw::data_nb_t physical_key = get_physical_key(upstream_key, t);
            piw::data_nb_t musical_key = get_musical_key(i->second, t);
            piw::data_nb_t result = piw::tuplenull_nb(t);
            result = piw::tupleadd_nb(result, piw::makelong_nb(get_keynumber(physical_key, rowlen_), t));
            result = piw::tupleadd_nb(result, physical_key);
            result = piw::tupleadd_nb(result, piw::makelong_nb(i->second,t));
            result = piw::tupleadd_nb(result, musical_key);
            return result;
        }

        return piw::makenull_nb(0);
    }

    void reverse_phys2phys(int dr, int dc, int *ur, int *uc)
    {
        pic::flipflop_t<mapping_t>::guard_t g(mapping_);

        if(0==dc || g.value().reverse.empty()) return;

        *ur = 0;
        *uc = 0;

        piw::data_nb_t rowlen = rowlen_.get();
        piw::data_nb_t rowoffset = rowoffset_.get();

        // sequential reverse
        if(0==dr && dc > 0)
        {
            piw::data_nb_t upstream_rowlen = upstream_rowlen_.get();
            unsigned ul = upstream_rowlen.as_tuplelen();
            unsigned rl = rowlen.as_tuplelen();
            unsigned ol = rowlen.as_tuplelen();
            unsigned l = std::min(std::min(ul,rl),ol);

            int upstream_keynum = 0;
            int local_keynum = 0;
            for(unsigned i=0; i<l; ++i)
            {
                piw::data_nb_t offset = rowoffset.as_tuple_value(i);
                int upstream_length = upstream_rowlen.as_tuple_value(i).as_long();
                int local_length = rowlen.as_tuple_value(i).as_long();
                int local_offset = 0;
                if(offset.is_long()) local_offset = offset.as_long();

                if(local_keynum+local_length>=dc)
                {
                    *uc = dc+(upstream_keynum+local_offset-local_keynum);
                    break;
                }
                upstream_keynum += upstream_length;
                local_keynum += local_length;
            }
        }
        // coordinate reverse
        else
        {
            piw::data_nb_t rowoffset_reverse = rowoffset_reverse_.get();

            int rrl = rowoffset_reverse.as_tuplelen();
            int rl = rowlen.as_tuplelen();

            if(dr<0) dr = dr+rl+1;
            if(dr<1 || dr>rl || dr>rrl) return;

            int mrl = rowlen.as_tuple_value(dr-1).as_long();

            if(dc<0) dc = dc+mrl+1;
            if(dc<1 || dc>mrl) return;

            *ur = rowoffset_reverse.as_tuple_value(dr-1).as_long();
            *uc = dc+rowoffset.as_tuple_value(*ur-1).as_long();
        }
    }

    void reverse_mus2phys(int dr, int dc, int *ur, int *uc)
    {
        pic::flipflop_t<mapping_t>::guard_t g(mapping_);

        if(0==dc || g.value().reverse.empty()) return;

        *ur = 0;
        *uc = 0;

        piw::data_nb_t rowlen = rowlen_.get();
        piw::data_nb_t rowoffset = rowoffset_.get();

        pic::lckmap_t<unsigned,unsigned>::lcktype::const_iterator i;

        // sequential reverse
        if(0==dr && dc > 0)
        {
            if((i=g.value().reverse.find(dc)) != g.value().reverse.end())
            {
                *uc = (int)i->second;
            }
        }
        // coordinate reverse
        else
        {
            unsigned musical_keynum = get_keynumber(dr, dc, courselen_);
            if((i=g.value().reverse.find(musical_keynum)) != g.value().reverse.end())
            {
                unsigned physical_keynum = i->second;
                piw::key_position(physical_keynum, upstream_rowlen_, ur, uc);
            }
        }
    }

    piw::data_nb_t reverse_mapping(const piw::data_nb_t &in)
    {
        if(!in.is_blob())
        {
            return in;
        }

        unsigned char* in_buffer=(unsigned char*)in.as_blob();
        unsigned in_size=in.host_length();

        unsigned char *out_buffer;
        piw::data_nb_t out = makeblob_nb(in.time(),in_size,&out_buffer);

        while(in_size>=5)
        {
            int ir = c2int(&in_buffer[0]);
            int ic = c2int(&in_buffer[2]);
            bool im = in_buffer[4]>>7;
            int xr = ir;
            int xc = ic;

            if(im)
            {
                reverse_mus2phys(ir,ic,&xr,&xc);
            }
            else
            {
                reverse_phys2phys(ir,ic,&xr,&xc);
            }

            int2c(xr,&out_buffer[0]);
            int2c(xc,&out_buffer[2]);
            out_buffer[4] = in_buffer[4]&0x7f; // kgroup converts everything to geometrical

            out_buffer+=5;
            in_buffer+=5;
            in_size-=5;
        }

        return out;
    }

    void set_upstream_rowlen(data_t rowlen)
    {
        upstream_rowlen_.set_normal(rowlen);
    }

    void set_rowlen(data_t rowlen)
    {
        rowlen_.set_normal(rowlen);
    }

    void set_rowoffset(data_t rowoffset)
    {
        rowoffset_.set_normal(rowoffset);

        unsigned l = rowoffset.as_tuplelen();
        piw::data_t r = piw::tuplenull(0);

        for(unsigned i=0; i<l; i++)
        {
            if(!rowoffset.as_tuple_value(i).is_null())
            {
                r = piw::tupleadd(r,piw::makelong(i+1,0));
            }
        }

        rowoffset_reverse_.set_normal(r);
    }

    void set_courselen(data_t courselen)
    {
        courselen_.set_normal(courselen);
    }

    pic::flipflop_t<mapping_t> mapping_;
    piw::dataholder_nb_t upstream_rowlen_;
    piw::dataholder_nb_t rowlen_;
    piw::dataholder_nb_t rowoffset_;
    piw::dataholder_nb_t rowoffset_reverse_;
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

void piw::kgroup_mapper_t::set_upstream_rowlen(data_t rowlen)
{
    impl_->set_upstream_rowlen(rowlen);
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
