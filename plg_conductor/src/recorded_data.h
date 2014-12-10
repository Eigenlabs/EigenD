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

#ifndef __CDTR_RECORDED_DATA_H__
#define __CDTR_RECORDED_DATA_H__

#include <picross/pic_fastalloc.h>
#include <picross/pic_stl.h>
#include <piw/piw_data.h>
#include <piw/piw_tsd.h>
#include <picross/pic_thread.h>
#include <picross/pic_time.h>

namespace cdtr
{
    class recorded_data_t;

    enum recorded_data_type_t {AUDIO, LABELS};

    class recorded_data_factory_t
    {
        public:
            recorded_data_factory_t(unsigned maximum);

            // this should be fast
            cdtr::recorded_data_t *get_instance(unsigned long long time, float songbeat, float barbeat, const cdtr::recorded_data_type_t type, const piw::data_t &data);
            // be careful to only delete recorded data with the factory it was created from
            void delete_instance(cdtr::recorded_data_t *data);
    
        private:
            const unsigned maximum_;
            pic_atomic_t allocation_count_;
    };

    class recorded_data_t: public pic::nocopy_t, virtual public pic::lckobject_t
    {
        public:
            unsigned long long get_time() const;
            float get_songbeat() const;
            float get_barbeat() const;
            const cdtr::recorded_data_type_t get_type() const;
            const piw::data_t get_data() const;
    
        private:
            friend class recorded_data_factory_t;

            recorded_data_t(unsigned long long time, float songbeat, float barbeat, const cdtr::recorded_data_type_t type, const piw::data_t &data);
            ~recorded_data_t();

            const unsigned long long time_;
            const float songbeat_;
            const float barbeat_;
            const cdtr::recorded_data_type_t type_;
            const piw::data_t data_;
    };

    
    typedef pic::lcklist_t<cdtr::recorded_data_t *>::nbtype recorded_data_list_t;
};

#endif
