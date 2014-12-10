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

#ifndef __CDTR_SQLITE_UTILS_H__
#define __CDTR_SQLITE_UTILS_H__

#include <lib_sqlite/sqlite3.h>

namespace cdtr
{
    int sql_check(int rc, sqlite3 *db, const char *msg);
    bool sql_exec(sqlite3 *db, const char *sql);
    sqlite3_stmt *sql_prepare(sqlite3 *db, const char *sql);

    class preparedstatement_t
    {
        public:
            preparedstatement_t(sqlite3 *db, const char *sql);
            ~preparedstatement_t();
            bool is_valid();
            bool bind_text(int index, const char *value);
            bool bind_int64(int index, sqlite3_int64 value);
            bool step_done();
            bool step_row();
            const unsigned char *column_text(int index);
            sqlite3_int64 column_int64(int index);
            void reset_and_clear();

            class impl_t;
        private:
            impl_t * const impl_;
    };
};

#endif
