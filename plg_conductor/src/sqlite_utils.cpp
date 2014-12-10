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

#include "sqlite_utils.h"

#include <lib_juce/juce.h>
#include <picross/pic_log.h>

class cdtr::preparedstatement_t::impl_t
{
    public:
        impl_t(sqlite3 *db, const char *sql): db_(db), sql_(sql), stmt_(0)
        {
            stmt_ = sql_prepare(db_, sql_.toUTF8());
        }

        ~impl_t()
        {
            if(stmt_)
            {
                sqlite3_finalize(stmt_);
            }
        }

        bool is_valid()
        {
            return stmt_ != 0;
        }

        bool bind_text(int index, const char *value)
        {
            if(!stmt_) return false;
            return SQLITE_OK == sql_check(sqlite3_bind_text(stmt_, index, value, -1, SQLITE_STATIC), db_, sql_.toUTF8());
        }

        bool bind_int64(int index, sqlite3_int64 value)
        {
            if(!stmt_) return false;
            return SQLITE_OK == sql_check(sqlite3_bind_int64(stmt_, index, value), db_, sql_.toUTF8());
        }

        bool step_done()
        {
            if(!stmt_) return false;
            return SQLITE_DONE == sql_check(sqlite3_step(stmt_), db_, sql_.toUTF8());
        }

        bool step_row()
        {
            if(!stmt_) return false;
            return SQLITE_ROW == sql_check(sqlite3_step(stmt_), db_, sql_.toUTF8());
        }

        const unsigned char *column_text(int index)
        {
            if(!stmt_) return 0;
            return sqlite3_column_text(stmt_, index);
        }

        sqlite3_int64 column_int64(int index)
        {
            if(!stmt_) return 0;
            return sqlite3_column_int64(stmt_, index);
        }

        void reset_and_clear()
        {
            if(stmt_)
            {
                sqlite3_reset(stmt_);
                sqlite3_clear_bindings(stmt_);
            }
        }

    private:
        sqlite3 * const db_;
        const juce::String sql_;
        sqlite3_stmt *stmt_;
};

int cdtr::sql_check(int rc, sqlite3 *db, const char *msg)
{
    if(rc != SQLITE_OK && rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        const char *errMsg = sqlite3_errmsg(db);
        if(msg)
        {
            pic::logmsg() << "SQLite error " << rc << " '" << msg << ": " << errMsg;
        }
        else
        {
            pic::logmsg() << "SQLite error " << rc << ": " << errMsg;
        }
    }

    return rc;
}

bool cdtr::sql_exec(sqlite3 *db, const char *sql)
{
    return SQLITE_OK == sql_check(sqlite3_exec(db, sql, 0, 0, 0), db, sql);
}

sqlite3_stmt *cdtr::sql_prepare(sqlite3 *db, const char *sql)
{
    sqlite3_stmt *stmt = 0;
    if(SQLITE_OK == sql_check(sqlite3_prepare_v2(db, sql, -1, &stmt, 0), db, sql))
    {
        return stmt;
    }
    pic::logmsg() << "Failed to prepare SQL statement '" << sql << "'";
    return 0;
}

cdtr::preparedstatement_t::preparedstatement_t(sqlite3 *db, const char *sql) : impl_(new impl_t(db, sql)) {}
cdtr::preparedstatement_t::~preparedstatement_t() { delete impl_; }
bool cdtr::preparedstatement_t::is_valid() { return impl_->is_valid(); }
bool cdtr::preparedstatement_t::bind_text(int index, const char *value) { return impl_->bind_text(index, value); }
bool cdtr::preparedstatement_t::bind_int64(int index, sqlite3_int64 value) { return impl_->bind_int64(index, value); }
bool cdtr::preparedstatement_t::step_done() { return impl_->step_done(); }
bool cdtr::preparedstatement_t::step_row() { return impl_->step_row(); }
const unsigned char *cdtr::preparedstatement_t::column_text(int index) { return impl_->column_text(index); }
sqlite3_int64 cdtr::preparedstatement_t::column_int64(int index) { return impl_->column_int64(index); }
void cdtr::preparedstatement_t::reset_and_clear() { impl_->reset_and_clear(); }