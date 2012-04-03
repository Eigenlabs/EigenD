
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

#ifndef __PIW_RING__
#define __PIW_RING__

#include <piw/piw_data.h>

namespace piw
{
    template <unsigned SIZE> class ringbuffer_t
    {
        public:
            ringbuffer_t(): in_(0), out_(0)
            {
                for(unsigned i=0;i<SIZE;i++)
                {
                    buffer_[i]=0;
                }
            }

            ~ringbuffer_t()
            {
                for(unsigned i=0;i<SIZE;i++)
                {
                    if(buffer_[i])
                    {
                        piw_data_decref_atomic(buffer_[i]);
                        buffer_[i]=0;
                    }
                }
            }

            unsigned used()
            {
                int x = in_-out_;
                if(x>=0)
                {
                    return x;
                }
                return SIZE+x;
            }

            unsigned space()
            {
                return SIZE-used()-1;
            }

            unsigned advance_in()
            {
                in_=(in_+1)%SIZE;
                return in_;
            }

            unsigned advance_out()
            {
                out_=(out_+1)%SIZE;
                return out_;
            }

            bool send(const piw::data_nb_t &d)
            {
                PIC_ASSERT(!d.is_null());
                if(space() > 0)
                {
                    buffer_[in_]=d.give_copy();
                    advance_in();
                    return true;
                }

                return false;
            }

            // be careful when using this method, you shouldn't ever use the data after it's
            // been passed into it
            bool send_raw(bct_data_t *d)
            {
                if(0 == (*d)) return false;

                if(space() > 0)
                {
                    buffer_[in_]=*d;
                    advance_in();
                    d = 0;
                    return true;
                }

                piw_data_decref_atomic(*d);
                d = 0;
                return false;
            }

            void write(const piw::data_nb_t &d)
            {
                if(!send(d))
                {
                    PIC_THROW("buffer full");
                }
            }

            bool is_empty()
            {
                return used() == 0;
            }

            piw::data_nb_t read_all()
            {
                piw::data_nb_t d;
                while(true)
                {
                    piw::data_nb_t n(read());
                    if(n.is_null())
                    {
                        return d;
                    }
                    d = n;
                }
            }

            piw::data_nb_t read()
            {
                if(used()==0)
                {
                    return null_;
                }

                bct_data_t data = buffer_[out_];
                buffer_[out_]=0;
                piw::data_nb_t d = piw::data_nb_t::from_given(data);
                advance_out();
                return d;
            }

            piw::data_nb_t peek()
            {
                if(used()==0)
                {
                    return null_;
                }

                return piw::data_nb_t::from_lent(buffer_[out_]);
            }

        private:
            piw::data_nb_t null_;
            volatile bct_data_t buffer_[SIZE];
            volatile unsigned in_,out_;
    };
}

#endif
