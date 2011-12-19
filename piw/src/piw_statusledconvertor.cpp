
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

#define MAX_COURSES 10

namespace
{
    static inline int c2int(unsigned char *c)
    {
        unsigned long cx = (c[0]<<8) | (c[1]);

        if(cx>0x7fff)
        {
            return ((long)cx)-0x10000;
        }

        return cx;
    }

    static inline int rc2k(int r, int c, int nc, const unsigned *co, const unsigned *cl)
    {
        int tc = co[nc-1]+cl[nc-1];

        if(r==0 && c==0) return -1;
        if(r==0 && c<=tc) return c-1;
        if(r<0) r=nc+r+1;
        if(r<1 || r>nc) return -1;
        if(c<0) c=cl[r-1]+c+1;
        if(c<1 || c>(int)cl[r-1]) return -1;
        return co[r-1]+c-1;
    }

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
    impl_t(unsigned num_courses, const unsigned *course_len): num_courses_(num_courses), course_len_(course_len)
    {
        num_keys_ = 0;

        for(unsigned i=0;i<num_courses_;i++)
        {
            course_offset_[i] = num_keys_;
            num_keys_ += course_len_[i];
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

    void update_leds(piw::data_nb_t &data, void *kbd, void (*func_set_led)(void*, unsigned, unsigned))
    {
        if(!data.is_blob()) return;

        memset(next_status_,0,num_keys_);

        unsigned char* rs = ((unsigned char*)data.as_blob());
        unsigned rl = data.as_bloblen();

        while(rl>=5)
        {
            int kr = c2int(&rs[0]);
            int kc = c2int(&rs[2]);
            unsigned kv = rs[4]&0x7f;

            if(kv!=BCTSTATUS_OFF)
            {
                int kn = rc2k(kr,kc,num_courses_,course_offset_,course_len_);

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
    unsigned num_courses_;
    unsigned course_offset_[MAX_COURSES];
    const unsigned *course_len_;
    bool initialized_;
    unsigned char *current_colors_;
    unsigned char *next_status_;
};

piw::statusledconvertor_t::statusledconvertor_t(unsigned num_courses, const unsigned *course_len): root_(new impl_t(num_courses,course_len))
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
