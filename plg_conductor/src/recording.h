/*
 Copyright 2012 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#ifndef __CDTR_RECORDING_H__
#define __CDTR_RECORDING_H__

#include <picross/pic_atomic.h>
#include <picross/pic_fastalloc.h>

#include "recorded_data.h"

#define CONSUMER_IDLE 0
#define CONSUMER_RUNNING 1
#define CONSUMER_TEARDOWN 2

namespace cdtr
{
    class data_wire_t;

    class recording_t: public pic::element_t<2>, public pic::nocopy_t, virtual public pic::lckobject_t
    {
        public:
            recording_t(cdtr::data_wire_t *wire);
            virtual ~recording_t();
    
            void start();
            void stop();
            bool is_recording();
    
            void add(unsigned long long time, const cdtr::recorded_data_type_t type, const piw::data_nb_t &data); // fast thread
            void consume(bool *activity); // data consumption thread

            void clear_labels();
    
            virtual void prepare(cdtr::recorded_data_t &first) = 0;
            virtual void teardown() = 0;
            virtual bool consumed(cdtr::recorded_data_t &data) = 0;
    
        protected:
            cdtr::data_wire_t * const wire_;
    
        private:
            void __add(unsigned long long time, const cdtr::recorded_data_type_t type, const piw::data_t &data); // fast thread
            inline void __switch();
            inline void __stop();
    
            volatile bool is_recording_;
            cdtr::recorded_data_list_t * volatile recorded_data_[2];
            pic_atomic_t switch_;
            pic_atomic_t producer_;
            pic::mutex_t consumer_lock_;
            pic_atomic_t consumer_phase_;
            pic::flipflop_t<piw::data_t> last_labels_;
    };
};

#endif
