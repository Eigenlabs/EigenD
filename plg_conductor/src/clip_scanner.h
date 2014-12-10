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

#ifndef __CDTR_CLIP_SCANNER_H__
#define __CDTR_CLIP_SCANNER_H__

#include <picross/pic_thread.h>
#include <lib_sqlite/sqlite3.h>

#include "conductor.h"

namespace cdtr
{
    class clip_scanner_t: public pic::thread_t
    {
        public:
            clip_scanner_t(cdtr::clip_manager_t::impl_t *clip_manager);
            ~clip_scanner_t();
    
            void shutdown();
            void thread_main();
            void scan(sqlite3 *db);
    
        private:
            sqlite3 *prepare_clipdb();

            cdtr::clip_manager_t::impl_t * const clip_manager_;
            pic::xgate_t gate_;
            volatile bool shutdown_;
    };
};

#endif
