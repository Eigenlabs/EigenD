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

#ifndef __WBENCH__
#define __WBENCH__

#include <set>
#include <piw/piw_data.h>
#include <string>

namespace workbench
{
    struct p2c_t
    {
        virtual ~p2c_t() { }
        virtual void agentAdded(const std::string&) = 0;
        virtual void agentRemoved(const std::string&) = 0;
        virtual void agentChanged(const std::string&) = 0;

        virtual void instanceRemoved(const std::string&) = 0;
        virtual void portRemoved(const std::string&) = 0;
        virtual void report_error(const std::string&,const std::string&) = 0;

        virtual void nameChanged(const std::string&) = 0;
        virtual void connectionAdded(const std::string&) = 0;
        virtual void stateChanged(const char *) = 0;
        virtual void sourcekeys_updated(const std::string &, const std::string&) = 0;
        virtual void instanceName(const std::string &, const std::string &) = 0;
        virtual void agents_updated(const std::set<std::string> &)=0;
        virtual void instances_updated(const std::set<std::string> &)=0;
        virtual void loaded(float)=0;
        virtual void cinfo_updated(const std::string&, const std::string&, const std::set<std::string> &)=0;
        virtual void finfo_updated(const std::string&, const std::string&, const std::set<std::string> &)=0;
        virtual void enumerate_updated(const std::string&, const std::string&, int, int)=0;
        virtual void activated(const std::string&)=0;
        virtual void current(const std::string&, const std::string&)=0;
        virtual void value_changed(const std::string&)=0;
        virtual void test_string(const char *)=0;
        virtual void connectionPossible(const std::string &, const std::string&, bool) = 0;
    };

    struct c2p0_t
    {
        c2p0_t() { }
        virtual ~c2p0_t() { }
        virtual void set_args(const char *) = 0;
        virtual std::string get_logfile() = 0;
    };

    struct c2p_t
    {
        c2p_t() { }
        virtual ~c2p_t() { }
        virtual void initialise(p2c_t *p2c, const std::string &scope) = 0;
        virtual void quit() = 0;
        virtual std::string get_name(const std::string&)=0;
        virtual std::string get_scope(const std::string&)=0;
        virtual std::string get_absoluteID(const std::string&)=0;
        virtual std::string get_desc(const std::string&)=0;
        virtual std::string get_fulldesc(const std::string&)=0;
        virtual std::set<std::string> get_children(const std::string&)=0;
        virtual std::set<std::string> get_child_props(const std::string&, bool)=0;
        virtual std::set<std::string> get_descendants(const std::string&)=0;
        virtual std::set<std::string> get_parent(const std::string&)=0;
//        virtual std::string get_connections(const std::string&)=0;
        virtual std::set<std::string> get_connections(const std::string&)=0;
        virtual std::set<std::string> get_using_inputs(const std::string&)=0;
        virtual int get_numInputs(const std::string&)=0;
        virtual void connect(const std::string&, const std::string&)=0;
        virtual void connect_check(const std::string&, const std::string&)=0;
        virtual void connect_test(const std::string&, const std::string&, int, const std::string&, const std::string&)=0;
        virtual void disconnect(const std::string&, const std::string&, int,const std::string&, const std::string&)=0;

        virtual void set_state(const std::string &)=0;
        virtual void get_agents()=0;
        virtual void get_ordinals_used(const std::string &)=0;
        virtual void create_agent(const std::string&, int)=0;
        virtual void delete_agent(const std::string&)=0;
        virtual std::string get_value(const std::string&)=0;
        virtual bool has_property(const std::string&,const std::string&)=0; 
        virtual std::string get_property(const std::string&,const std::string&)=0; 
        virtual bool hasEditableValue(const std::string&)=0; 
        virtual bool has_master(const std::string&)=0; 
        virtual bool has_non_controller_master(const std::string&)=0; 
        virtual std::string get_domain(const std::string &)=0;
        virtual void set_boolvalue(const std::string &, bool)=0;
        virtual void set_stringvalue(const std::string &, const std::string &)=0;
        virtual void set_intvalue(const std::string &, int)=0;
        virtual void set_floatvalue(const std::string &, float)=0;
        virtual std::string get_inputs(const std::string &, const std::string &)=0;
        virtual int child_count(const std::string &)=0;
        virtual bool is_input(const std::string &)=0;
        virtual bool is_output(const std::string &)=0;
        virtual std::string get_master_filter(const std::string &, const std::string &)=0;
        virtual std::string get_master_control(const std::string &, const std::string &)=0;
        virtual bool has_protocol(const std::string &, const std::string &)=0;
        virtual void get_instances(const std::string&)=0;
        virtual void get_instanceName(const std::string&)=0;
        virtual void create_instance(const std::string&,int)=0;
        virtual void delete_instance(const std::string&,const std::string &)=0;
        virtual int get_ordinal(const std::string&)=0;
        virtual void setName(const std::string&, const std::string&)=0;

        virtual void enumerate(const std::string&,const std::string&)=0;
        virtual void cinfo(const std::string&,const std::string&,int,int)=0;
        virtual void finfo(const std::string&, const std::string&)=0;
        virtual void activate(const std::string&, const std::string&,const std::string&)=0;
        virtual void current(const std::string&)=0;
        virtual bool is_slave_of(const std::string&, const std::string&)=0;
        virtual void stop_progress()=0;
        virtual std::string get_tooltip(const std::string&)=0;
        virtual std::string get_tooltip_by_id(const std::string&)=0;
        virtual std::string get_helptext(const std::string&)=0;
        virtual std::string get_helptext_by_id(const std::string&)=0;
        virtual bool is_rig(const std::string&)=0; 
        virtual void invoke(const std::string&, const std::string&,int)=0;
        virtual void get_sourcekeys(const std::string&, const std::string&)=0;
        virtual std::string get_child(const std::string&, const std::string&)=0;
        virtual void monitor_on(const std::string&)=0;
        virtual void monitor_off(const std::string&)=0;
        virtual void get_test_string(int)=0;
     };

}
#endif
