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

#ifndef __WSPACE__
#define __WSPACE__

#include <piw/piw_tsd.h>
#include "workbench.h"
#include "sizes.h"
#include "utils.h"
#include "juce.h"
#include "epython.h"
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include "ProgressLayer.h"

class Workspace;

class WorkspaceListener
{
public:
    WorkspaceListener(){};
    virtual ~WorkspaceListener(){};

    virtual void agentAdded(String id)=0;
    virtual void agentPropertyChanged(String id)=0;

    virtual void agentRemoved(String id)=0;
    virtual void instanceRemoved(String id)=0;
    virtual void portRemoved(String id)=0;
    virtual void connection_created(String id)=0;
    virtual void nameChanged(String id)=0;
    virtual void instanceName(String id, String name)=0;
    virtual void checkConnections(String id, std::set<String>)=0;

    virtual void hookAdded(String id)=0;
    virtual void horizontalTrunkAdded(String id)=0;
    virtual void verticalTrunkAdded(String id)=0;
    virtual void trunkCornerAdded(String id)=0;
    virtual void routingAdded(String id)=0;
    virtual void sourcePinAdded(String id)=0;
    virtual void destinationPinAdded(String id)=0;
    virtual void looseWireAdded(String id)=0;
    virtual void propertiesAdded(String id)=0;

    virtual void hookChanged(String id)=0;
    virtual void horizontalTrunkChanged(String id)=0;
    virtual void verticalTrunkChanged(String id)=0;
    virtual void trunkCornerChanged(String id)=0;
    virtual void routingChanged(String id)=0;
    virtual void sourcePinChanged(String id)=0;
    virtual void destinationPinChanged(String id)=0;
    virtual void looseWireChanged(String id)=0;
    virtual void propertiesChanged(String id)=0;

    virtual void hookRemoved(String id)=0;
    virtual void horizontalTrunkRemoved(String id)=0;
    virtual void verticalTrunkRemoved(String id)=0;
    virtual void trunkCornerRemoved(String id)=0;
    virtual void routingRemoved(String id)=0;
    virtual void propertiesRemoved(String id)=0;

    virtual void agentsUpdated(const std::set<std::string> & agents)=0;
    virtual void instancesUpdated(const std::set<std::string> & ords)=0;
    virtual void cinfo_updated(String s,String path,const std::set<std::string> & cinfo)=0;
    virtual void finfo_updated(String s,String path,const std::set<std::string> & finfo)=0;
    virtual void enumerate_updated(String s,String path,int nf, int nc)=0;
    virtual void activated(String s)=0;
    virtual void current(String id, String cookie)=0;
    virtual void sourcekeys_updated(String id,String keys)=0;
    virtual void report_error(String err1, String err2)=0;
    virtual void getTemporaryPosition(int& x, int& y)=0;
    virtual void connectionPossible(String sid, String did, bool)=0;
};

class PropertyStore
{

public:
    PropertyStore(String type, String id);

    void set_number(const String &key,int value);
    void set_number(const String &key, double value);
    int get_number(const String &key);
    double get_double(const String &key);

    void set_list(const String &key, unsigned index, const String &value);
    String get_list(const String &key, unsigned index);
    unsigned list_getsize(const String &key);
    void remove_list(const String &key);
    bool has_list(const String &key);
    int list_count();
    String get_listkey(int index);
    void remove_list_item(const String &listkey, const String &value);
    void add_list_item(const String &listkey, const String &value);
    bool has_list_item(const String &listkey, const String &value);

    String get_string(const String &key);
    void set_string(const String &key, String value);
    bool has_key(const String key);

    XmlElement* get_element();

private:
    XmlElement* p_;
    
};


class Connection
{
public:
    Connection(String srcId, String dstId,PropertyStore* props, workbench::c2p_t* backend);
    String get_id();
// XXX
//    Atom input();
//    Atom output();
    String input();
    String output();
    PropertyStore* get_props();
    String get_using();
    String get_master_filter();
    String get_master_control();
    String getDescription();
    bool is_hidden();

private:
    String id_;
    String inputId_;
    String outputId_;
    PropertyStore* props_;
    bool hidden_;
    workbench::c2p_t* backend_;
};


class Atom
{
public:
    Atom(String id, bool root, workbench::c2p_t* backend, Workspace* w);
    ~Atom();
    bool is_root();
    bool is_input();
    bool is_output();
    Atom* get_parent();
    unsigned child_count();
    Atom* get_child(unsigned index);
    Atom* get_child(String name);
    int get_ordinal();
    String get_name();
    String get_scope();
    String get_absoluteID();
    String get_tooltip();
    String get_helptext();
    String get_desc();
    String get_fulldesc();
    String get_id();
    bool hasEditableValue();
    bool has_master();
    bool has_non_controller_master();
    String get_value();
    void set_value(bool val);
    void set_value(int val);
    void set_value(float val);
    void set_value(String val);
    bool is_slave_of(Atom* other);

    String get_domain();
    bool has_protocol(String prot);
    bool hasProperty(String p);
    String getProperty(String p);
    void get_instances();
    void create_instance(int ord);
    void delete_instance(const String &cid);
    void get_instanceName();
    void getDescendants(std::set<String>& ids);
    void getChildren(std::set<String>& ids);
    void getChildProps(std::vector<NamedId>& ids, bool showNames);
    void get_using_inputs(std::set<String>& ids);
    int get_numInputs();
    Atom* getDescendant(String id);
    String* getScope();

// XXX  not using these yet - should be I think
//    void add_input_connection(Connection* c);
//    void add_output_connection(Connection* c);
//    unsigned input_count();
//    unsigned output_count();
    Connection* input_connection(unsigned);
    Connection* output_conection(unsigned);

    void setName(const String&);

    void get_current();
    void activate(String path,String cookie);
    void enumerate(String path);
    void finfo(String path);
    void cinfo(String path);

    void invoke(String,int);
    void getSourceKeys(const String&);

    // void set_name(const std::string &)
    void monitor_on();
    void monitor_off();

private:
    bool root_;
    String id_;
//    String name_;
    std::vector<NamedId> children_;
    workbench::c2p_t* backend_;
    void setup_child_ids(const std::set<std::string> &ids);
    Workspace* w_;
};

class Agent
{
    public:
        Agent(String id,bool isRig, PropertyStore* props, workbench::c2p_t* backend, Workspace* w);
        ~Agent();
        bool is_workspace();
        Atom* get_root();
        int getX();
        int getY();
        PropertyStore* get_store();

    private:
        bool rig_;
        Atom* atom_;
        PropertyStore* props_;
        workbench::c2p_t* backend_;
        Workspace* w_;
};

class ValueMonitorListener
{
    public:
        ValueMonitorListener(){};
        virtual ~ValueMonitorListener(){};
        virtual void value_changed(String id)=0;
};

class ValueMonitor
{
    public:
        ValueMonitor();
        void add(Atom*);
        void remove(Atom*);
        void clear();
        void addListener(ValueMonitorListener*);
        void removeListener();
        void value_changed(String id);
        void printInfo();

    private:
       // XXX listeners
       ValueMonitorListener* listener_;
       std::map<String, Atom*>atoms_;
};


class Workspace:public workbench::p2c_t
{
public:
    Workspace(epython::PythonBackend *backend);
    ~Workspace();
    //void stateChanged(const std::string &state);
    void stateChanged(const char *);
    void sourcekeys_updated(const std::string &id, const std::string &sourcekeys);
    void instanceName(const std::string &id, const std::string &name);
    void agents_updated(const std::set<std::string> & agents);
    void instances_updated(const std::set<std::string> & ords);
    void cinfo_updated(const std::string& s,const std::string& path,const std::set<std::string> & cinfo);
    void finfo_updated(const std::string& s,const std::string& path,const std::set<std::string> & finfo);
    void enumerate_updated(const std::string& s,const std::string& path,int nf, int nc);
    void activated(const std::string& s);
    void report_error(const std::string& s1,const std::string& s2);
    void current(const std::string& id,const std::string& cookie);
    void createAgent(int x, int y, String agentType, int ordinal);
    void deleteAgent(String id);
    
    void createHook(int x, int y);
    void createTrunk(int x, int y, int width, int height, int orientation, int direction,String assemblyId, int assemblySize, int index);
    void createVerticalTrunk(int x, int y, int width, int height,int direction,String assemblyId, int assemblySize, int index);
    void createHorizontalTrunk(int x, int y, int width, int height,int direction, String assemblyId, int assemblySize, int index);

    void createTrunkCorner(int x, int y, String assemblyId, int assemblyIndex, int cornerType);
    void create_connection(Atom* src, Atom*  dst);
    void check_connection(String srcId, String  dstId);
    void create_connection(String srcId, String  dstId);
    void create_connection(String srcId, String  dstId, String u, String f, String c);
    void connectionPossible(const std::string &sid, const std::string &did, bool possible);

    void createLooseWire(String id);
    void createSourcePin(int x, int y);
    void createDestinationPin(int x, int y);

    Connection* get_connection(String id);
    void delete_connection(String id, String u, String f, String c);
    void get_connections(String id);
    String getParentId(String id);
    String get_tooltip(String agent);
    String get_helptext(String agent);

    void getAgentNames();
    void addListener(WorkspaceListener* listener);

    Agent* getAgent(String id);
    PropertyStore* create_store(String type,String id);
    void init_store(PropertyStore* p);
    bool has_store(String id);
    PropertyStore* get_store(String id);
    void remove_store(PropertyStore* p);
    void save_store(String id);
    void save_store(PropertyStore* p);
    String generateID();
    String getFullName(String id);
    String get_absoluteID(String id);

    void agentRemoved(const std::string& id);
    void agentAdded(const std::string& id);
    void agentChanged(const std::string& id);
    void instanceRemoved(const std::string& id);
    void portRemoved(const std::string& id);
    void nameChanged(const std::string& id);
    void connectionAdded(const std::string& id);
    void loaded(float p);

    void getDescendants(String id,std::set<String>& ids);
//    String get_inputs(String sid, String mid);
    bool has_protocol(String id,String prot);
    void initialise_backend(String scope);
    void quit();
    ValueMonitor* getValueMonitor();
    void value_changed(const std::string&);
    void get_test_string(int);
    void test_string(const char*);

private:
    epython::PythonBackend *python_backend_;
    workbench::c2p_t* backend_;
    WorkspaceListener* listener_;
    int idCount_;
    bool agentsChanged_;

    std::map<String,PropertyStore*> props_;
    std::set<String> tmpProps_;
    XmlElement* properties_;
    void readState(String p);
    void readAgent(XmlElement* e);
    void readHook(XmlElement* e);
    void readHorizontalTrunk(XmlElement* e);
    void readVerticalTrunk(XmlElement* e);
    void readTrunkCorner(XmlElement* e);
    void readSourcePin(XmlElement* e);
    void readDestinationPin(XmlElement* e);
    void readRouting(XmlElement* e);
    void readLooseWire(XmlElement* e);
    void readWorkspaceProps(XmlElement* e);


    void set_state();
    void storeRemoved(String id);
    void setPosition(PropertyStore* p, String aid);
    void setTemporaryPosition(PropertyStore* p, String aid);
    std::map<String,int> agentPosX_;
    std::map<String,int> agentPosY_;
    int agentCount_;

    void start_loading();
    void deleteProgressWindow();

    double progress_;
    double oldProgress_;
    ProgressBar* progressBar_;
    bool progressBarDisplayed_;
    ResizableWindow* progressWindow_;
    ValueMonitor* valueMonitor_;
};

#endif
