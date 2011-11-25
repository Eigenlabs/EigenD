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

#ifndef __PI_STATE_DATABASE__
#define __PI_STATE_DATABASE__

#include "sng_file.h"
#include "sng_mapping.h"
#include "sng_exports.h"

#include <piw/piw_data.h>
#include <picross/pic_ref.h>
#include <picross/pic_nocopy.h>

#include <string>

namespace pi
{
    namespace state
    {
        class node_t;
        class agent_t;
        class snapshot_t;
        class database_t;

        typedef pic::ref_t<database_t> dbref_t;
        typedef pic::ref_t<snapshot_t> snapref_t;
        typedef pic::ref_t<agent_t> agentref_t;
        typedef pic::ref_t<node_t> noderef_t;

        struct SNG_DECLSPEC_CLASS node_t: public pic::nocopy_t, virtual public pic::counted_t
        {
            virtual ~node_t() {}
            virtual piw::data_t get_data() const = 0;
            virtual bool set_data(const piw::data_t &v) = 0;
            virtual unsigned long save() = 0;
            virtual void erase() = 0;
            virtual std::string list_children() = 0;
            virtual unsigned char enum_children(unsigned char n) = 0;
            virtual noderef_t get_child(unsigned char n) = 0;
            virtual void erase_child(unsigned char n) = 0;
            virtual noderef_t snapshot() = 0;
            virtual void copy(const noderef_t &,const mapref_t &) = 0;
        };

        struct SNG_DECLSPEC_CLASS agent_t: public pic::nocopy_t, virtual public pic::counted_t
        {
            virtual ~agent_t() {}
            virtual std::string get_address() const = 0;
            virtual std::string get_plugin() = 0;
            virtual std::string get_name() = 0;
            virtual unsigned get_type() const = 0;
            virtual noderef_t get_root() = 0;
            virtual agentref_t checkpoint() = 0;
            virtual unsigned long set_checkpoint() = 0;
            virtual unsigned long get_checkpoint() const = 0;
            virtual bool isdirty() const = 0;
            virtual void set_type(unsigned) = 0;
        };

        struct SNG_DECLSPEC_CLASS snapshot_t: public pic::nocopy_t, virtual public pic::counted_t
        {
            virtual ~snapshot_t() {}
            virtual unsigned long previous() = 0;
            virtual unsigned long version() = 0;
            virtual unsigned long long timestamp() = 0;
            virtual std::string tag() = 0;
            virtual unsigned long save(unsigned long long ts, const char *tag) = 0;
            virtual unsigned agent_count() = 0;
            virtual agentref_t get_agent_index(unsigned n) = 0;
            virtual agentref_t get_agent_address(unsigned type,const std::string &name,bool create) = 0;
            virtual agentref_t set_agent(const agentref_t &) = 0;
            virtual bool erase_agent(const agentref_t &) = 0;
            virtual snapref_t clone(unsigned long long ts, const char *tag) = 0;
            virtual void copy(const snapref_t &, const mapref_t &, bool) = 0;
            virtual void erase() = 0;
        };

        struct SNG_DECLSPEC_CLASS database_t: public pic::nocopy_t, virtual public pic::counted_t
        {
            virtual ~database_t() {}
            virtual snapref_t get_trunk() = 0;
            virtual snapref_t get_version(unsigned long pos) = 0;
            virtual fileref_t get_file() = 0;
            virtual void close() = 0;
            virtual void flush() = 0;
            virtual bool writeable() = 0;
        };

        SNG_DECLSPEC_FUNC(dbref_t) open_database(const char *filename, bool writeable);
    };
}

#endif
