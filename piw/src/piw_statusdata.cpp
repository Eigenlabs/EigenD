
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

#include <piw/piw_status.h>

piw::statusdata_t::statusdata_t(bool musical, const piw::coordinate_t &coordinate, unsigned char status): musical_(musical), coordinate_(coordinate), status_(status)
{
}

piw::statusdata_t piw::statusdata_t::from_bytes(unsigned char *bytes)
{
    int x = piw::statusdata_t::c2int(&bytes[0]);
    int y = piw::statusdata_t::c2int(&bytes[2]);
    bool musical = bytes[4]&1;
    bool endrel_x = bytes[4]&(1<<2);
    bool endrel_y = bytes[4]&(1<<1);
    unsigned char status = bytes[5];
    return piw::statusdata_t(musical, piw::coordinate_t(x,y,endrel_x,endrel_y), status);
}

void piw::statusdata_t::to_bytes(unsigned char *bytes) const
{
    piw::statusdata_t::int2c(coordinate_.x_,bytes+0);
    piw::statusdata_t::int2c(coordinate_.y_,bytes+2);
    *(bytes+4) = musical_ | coordinate_.endrel_x_ << 2 | coordinate_.endrel_y_ << 1;
    *(bytes+5) = status_;
}

bool piw::statusdata_t::operator==(const statusdata_t &o) const
{
    return musical_ == o.musical_ && coordinate_ == o.coordinate_;
}

bool piw::statusdata_t::operator<(const statusdata_t &o) const
{
    if(musical_ < o.musical_) return true;
    if(musical_ > o.musical_) return false;
    if(coordinate_ < o.coordinate_) return true;
    if(coordinate_ > o.coordinate_) return false;
    return false;
}

bool piw::statusdata_t::operator>(const statusdata_t &o) const
{
    if(musical_ < o.musical_) return false;
    if(musical_ > o.musical_) return true;
    if(coordinate_ < o.coordinate_) return false;
    if(coordinate_ > o.coordinate_) return true;
    return false;
}

inline void piw::statusdata_t::int2c(int r, unsigned char *o)
{
    long k = r;

    if(r<0) 
    {
        k = (int)0x10000+r;
    }

    o[0] = ((k>>8)&0xff);
    o[1] = (k&0xff);
}

inline int piw::statusdata_t::c2int(unsigned char *c)
{
    unsigned long cx = (c[0]<<8) | (c[1]);

    if(cx>0x7fff)
    {
        return ((long)cx)-0x10000;
    }

    return cx;
}

std::ostream &operator<<(std::ostream &o, const piw::statusdata_t &d)
{
    o << "(" << d.musical_ << "," << d.coordinate_ << "," << d.status_ << ")";
    return o;
}
