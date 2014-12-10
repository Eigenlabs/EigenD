/*
 Copyright 2012-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include <picross/pic_ref.h>

class PropertyStore
{
    public:
        class Listener
        {
            virtual void property_changed(const std::string &key) = 0;
        };

    public:
        bool has_list(const std::string &key);
        unsigned list_getsize(const std::string &key);
        void list_setsize(const std::string &key);
        std::string list_get(const std::string &key, unsigned index);
        void list_set(const std::string &key, unsigned index, const std::string &value);

        bool has_string(const std::string &key);
        void set_string(const std::string &key, const std::string &value);
        int get_string(const std::string &key);

        bool has_number(const std::string &key);
        void set_number(const std::string &key, int value);
        std::string get_number(const std::string &key);

        class Impl;

    private:
        pic::ref_t<Impl> impl_;


};

class Connection
{
    public:
        class impl_t;

    public:
        std::string get_id();
        Atom input();
        Atom output();
};

class Atom
{
    public:
        bool is_root();
        bool is_input();
        bool is_output();

        Atom get_parent();
        unsigned child_count();
        Atom get_child(unsigned index);

        std::string get_name();
        std::string get_id();

        unsigned input_count();
        Connection input_connection(unsigned);

        unsigned output_count();
        Connection output_connection(unsigned);

        void set_name(const std::string &);
};

class Agent
{
    public:
        class Listener
        {
        };

    public:
        bool is_workspace();
        Workspace get_workspace();
        Atom get_root();
};

class Workspace
{
    public:
        class Listener
        {
            virtual void agent_created() = 0;
            virtual void agent_deleted() = 0;
            virtual void connection_created() = 0;
            virtual void connection_deleted() = 0;
        };

    public:
        Workspace();

        unsigned agent_count();
        Atom get_agent(unsigned index);

        Connection create_connection();
        Connection get_connection(const std::string &id);

        unsigned store_count(const std::string &type);
        PropertyStore get_store_by_index(const std::string &type, unsigned index);
        void delete_store_by_index(const std::string &type,unsigned index);
        PropertyStore get_store(const std::string &type, const std::string &id);
        void delete_store(const std::string &type, const std::string &id);
        PropertyStore create_store(const std::string &type, const std::string &id);

        // create agent
        // delete agent
        // agent types

};

Workspace RootWorkspace();
