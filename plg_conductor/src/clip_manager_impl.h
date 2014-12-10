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

#ifndef __CDTR_CLIP_MANAGER_IMPL_H__
#define __CDTR_CLIP_MANAGER_IMPL_H__

#include "conductor.h"
#include "conductor_library.h"
#include "sqlite_utils.h"
#include "clip_scanner.h"
#include "clip_manager_backend.h"
#include <lib_juce/juce.h>
#include <lib_sqlite/sqlite3.h>

namespace cdtr
{
    class cdtr::clip_manager_t::impl_t: public cdtr::conductor_library_t, public clip_manager_backend_t
    {
        public:
            impl_t(const std::string &library_path);
            ~impl_t();
    
            virtual bool rpc_change_tags_for_clip(void *context, const XmlElement *argument);
            virtual bool rpc_get_column_categories(void *context, const XmlElement *argument);
            virtual bool rpc_get_selected_clips(void *context, const XmlElement *argument);
            virtual bool rpc_get_all_tags(void *context, const XmlElement *argument);
            virtual bool rpc_add_to_clip_pool(void *context, const XmlElement *argument, int index);
            virtual bool rpc_add_tag(void *context, const XmlElement *argument);
            virtual bool rpc_remove_tag(void *context, const XmlElement *argument);
            virtual bool rpc_change_tag(void *context, const XmlElement *argument);
            virtual bool rpc_add_category(void *context, const XmlElement *argument);
        private:
            cdtr::clip_scanner_t scanner_;
            sqlite3 *open_clipdb();
    };
};

#endif
