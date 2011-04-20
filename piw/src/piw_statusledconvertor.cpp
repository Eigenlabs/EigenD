
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

struct piw::statusledconvertor_t::impl_t: virtual pic::tracked_t
{
    impl_t(const unsigned num_keys): num_keys_(num_keys), initialized_(false), current_colors_(new unsigned[num_keys])
    {
        memset(current_colors_, CLR_OFF, num_keys);
    }

    ~impl_t()
    {
        if(current_colors_)
        {
            delete current_colors_;
            current_colors_=0;
        }
        tracked_invalidate();
    }

    void update_leds(piw::data_nb_t &data, void *kbd, void (*func_set_led)(void*, unsigned, unsigned))
    {
        if(!initialized_)
        {
            unsigned color=CLR_OFF;
            for(unsigned i=0;i<num_keys_;i++)
            {
                func_set_led(kbd, i, color);
            }
            initialized_=true;
        }

        unsigned char* status=(unsigned char*)data.as_blob();
        unsigned k=0;
        while(k<data.host_length() && k<num_keys_)
        {
            set_status(k,status[k],kbd,func_set_led);
            k++;
        }

        while(k<num_keys_)
        {
            set_status(k,BCTSTATUS_OFF,kbd,func_set_led);
            k++;
        }
    }

    void set_status(unsigned key, unsigned char status, void *kbd, void (*func_set_led)(void*, unsigned, unsigned))
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
        if(current_colors_[key]==color)
        {
            return;
        }

        func_set_led(kbd, key, color);

        current_colors_[key]=color;
    }

    const unsigned num_keys_;
    bool initialized_;
    unsigned *current_colors_;
};

piw::statusledconvertor_t::statusledconvertor_t(unsigned num_keys): root_(new impl_t(num_keys))
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
