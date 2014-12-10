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


#ifndef __CONDUCTOR_MANAGER_H__
#define __CONDUCTOR_MANAGER_H__

#include "widget.h"
#include <lib_juce/juce.h>
#include <piw/piw_thing.h>
#include <picross/pic_safeq.h>

namespace cdtr
{
    class clip_manager_backend_t: public widget_provider_t, public pic::safe_worker_t, public piw::thing_t
    {
        public:
            clip_manager_backend_t();
            ~clip_manager_backend_t();

            void bump_sequence();
            void value_changed(const piw::data_t &value);

            virtual bool rpc_change_tags_for_clip(void *context, const XmlElement *argument) = 0;
            virtual bool rpc_get_column_categories(void *context, const XmlElement *argument) = 0;
            virtual bool rpc_get_selected_clips(void *context, const XmlElement *argument) = 0;
            virtual bool rpc_get_all_tags(void *context, const XmlElement *argument) = 0;
            virtual bool rpc_add_to_clip_pool(void *context, const XmlElement *argument, int index) = 0;
            virtual bool rpc_add_tag(void *context, const XmlElement *argument) = 0;
            virtual bool rpc_remove_tag(void *context, const XmlElement *argument) = 0;
            virtual bool rpc_change_tag(void *context, const XmlElement *argument) = 0;
            virtual bool rpc_add_category(void *context, const XmlElement *argument) = 0;

            void complete_rpc(void *context, const XmlElement *value = 0);

        protected:
            static void dispatch__(void *,void *,void *,void *);
            bool rpc_invoked(void *context, int method, const piw::data_t &arguments);
            void thing_dequeue_slow(const piw::data_t &d);
            void thing_trigger_slow();

        private:
            unsigned sequence_;
    };
};

#endif
