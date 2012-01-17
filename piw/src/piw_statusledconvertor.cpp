
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

#include <piw/piw_tsd.h>
#include <piw/piw_status.h>
#include <piw/piw_thing.h>
#include <picross/pic_fastalloc.h>
#include <picross/pic_ref.h>

#define MAX_ROWS 10

namespace
{
    static inline int status2color(unsigned status)
    {
        unsigned color=0;

        switch(status)
        {
            case BCTSTATUS_OFF:
                color=CLR_OFF;
                break;
            case BCTSTATUS_ACTIVE:
                color=CLR_GREEN;
                break;
            case BCTSTATUS_INACTIVE:
                color=CLR_RED;
                break;
            case BCTSTATUS_UNKNOWN:
                color=CLR_ORANGE;
                break;
            case BCTSTATUS_MIXED:
                color=CLR_ORANGE;
                break;
            case BCTSTATUS_BLINK:
                color=CLR_ORANGE;
                break;
            case BCTSTATUS_SELECTOR_ON:
                color=CLR_GREEN;
                break;
            case BCTSTATUS_SELECTOR_OFF:
                color=CLR_RED;
                break;
            case BCTSTATUS_CHOOSE_AVAILABLE:
                color=CLR_RED;
                break;
            case BCTSTATUS_CHOOSE_USED:
                color=CLR_ORANGE;
                break;
            case BCTSTATUS_CHOOSE_ACTIVE:
                color=CLR_GREEN;
                break;
            default:
                color=CLR_OFF;
                break;
        }

        return color;
    }

};

struct piw::statusledconvertor_t::impl_t: virtual pic::tracked_t, virtual pic::lckobject_t
{
    impl_t(unsigned num_rows, const unsigned *row_len): num_rows_(num_rows), row_len_(row_len)
    {
        num_keys_ = 0;

        for(unsigned i=0;i<num_rows_;i++)
        {
            row_offset_[i] = num_keys_;
            num_keys_ += row_len_[i];
        }

        current_colors_ = (unsigned char *)pic::nb_malloc(PIC_ALLOC_LCK,num_keys_);
        next_status_ = (unsigned char *)pic::nb_malloc(PIC_ALLOC_LCK,num_keys_);

        memset(current_colors_, 0xff, num_keys_);
    }

    ~impl_t()
    {
        tracked_invalidate();

        if(current_colors_)
        {
            pic::nb_free(current_colors_);
            current_colors_=0;
        }

        if(next_status_)
        {
            pic::nb_free(next_status_);
            next_status_=0;
        }
    }

    inline int xy2k(bool musical, int x, int y)
    {
        if(musical)
        {
            if(x==0 && y==0) return -1;
            if(y>(int)num_keys_) return -1;
            
            return y-1;
        }
        else
        {
            int tc = row_offset_[num_rows_-1]+row_len_[num_rows_+1];

            if(x==0 && y==0) return -1;
            if(x==0 && y<=tc) return y-1;
            if(x<1 || x>(int)num_rows_) return -1;
            if(x<0) x=num_rows_+x+1;
            if(y<0) y=row_len_[x-1]+y+1;
            if(y<1 || y>(int)row_len_[x-1]) return -1;

            return row_offset_[x-1]+y-1;
        }
    }

    void update_leds(piw::data_nb_t &data, void *kbd, void (*func_set_led)(void*, unsigned, unsigned))
    {
        if(!data.is_blob()) return;

        memset(next_status_,0,num_keys_);

        unsigned char* rs = ((unsigned char*)data.as_blob());
        unsigned rl = data.as_bloblen();

        while(rl>=5)
        {
            int kx = piw::statusdata_t::c2int(&rs[0]);
            int ky = piw::statusdata_t::c2int(&rs[2]);
            unsigned kv = rs[4]&0x7f;
            bool musical = rs[4]&(1<<7);

            if(kv!=BCTSTATUS_OFF)
            {
                int kn = xy2k(musical,kx,ky);

                if(kn>=0)
                {
                    unsigned os = next_status_[kn];

                    if(os!=kv)
                    {
                        if(os && os!=BCTSTATUS_OFF)
                        {
                            kv = BCTSTATUS_MIXED;
                        }

                        next_status_[kn] = kv;
                    }
                }
            }

            rs+=5;
            rl-=5;
        }

        for(unsigned i=0;i<num_keys_;i++)
        {
            unsigned c = status2color(next_status_[i]);

            if(c != current_colors_[i])
            {
                current_colors_[i] = c;
                func_set_led(kbd,i,c);
            }
        }
    }

    unsigned num_keys_;

    unsigned num_rows_;
    unsigned row_offset_[MAX_ROWS];
    const unsigned *row_len_;

    bool initialized_;
    unsigned char *current_colors_;
    unsigned char *next_status_;
};

piw::statusledconvertor_t::statusledconvertor_t(unsigned num_rows, const unsigned *row_len): root_(new impl_t(num_rows,row_len))
{
}

piw::statusledconvertor_t::~statusledconvertor_t()
{
    delete root_;
}

void piw::statusledconvertor_t::update_leds(piw::data_nb_t &status_data, void *kbd, void (*func_set_led)(void*, unsigned, unsigned))
{
    root_->update_leds(status_data, kbd, func_set_led);
}
