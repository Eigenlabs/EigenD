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

#include "Workspace.h"

Connection::Connection(String srcId, String dstId,PropertyStore* props, workbench::c2p_t *backend)
{
    // XXX input and output need reversing ?
    // currently refer to the input and output of the connection itself
    inputId_=srcId;
    outputId_=dstId;
    id_=srcId+":"+dstId;
    props_=props;
    backend_=backend;
    hidden_=false;
    bool inhidden=backend_->has_protocol(juceToStdString(inputId_), juceToStdString("hidden-connection"));
    bool outhidden=backend_->has_protocol(juceToStdString(outputId_), juceToStdString("hidden-connection"));
    hidden_=inhidden||outhidden;
}

bool Connection::is_hidden()
{
    bool inhidden=backend_->has_protocol(juceToStdString(inputId_), juceToStdString("hidden-connection"));
    bool outhidden=backend_->has_protocol(juceToStdString(outputId_), juceToStdString("hidden-connection"));
    hidden_=inhidden||outhidden;

    return hidden_;
}

String Connection::get_using()
{
     return stdToJuceString(backend_->get_inputs(juceToStdString(outputId_), juceToStdString(inputId_)));
}

String Connection::get_master_filter()
{
    return stdToJuceString(backend_->get_master_filter(juceToStdString(outputId_),juceToStdString(inputId_)));
}

String Connection::get_master_control()
{
    return stdToJuceString(backend_->get_master_control(juceToStdString(outputId_),juceToStdString(inputId_)));
}


String Connection::get_id()
{
    return id_;
}

String Connection::input()
{
    return inputId_;
}

String Connection::output()
{
    return outputId_;
}

String Connection::getDescription()
{
    String src= stdToJuceString(backend_->get_fulldesc(juceToStdString(inputId_)));
    String dst= stdToJuceString(backend_->get_fulldesc(juceToStdString(outputId_)));
    return src +" to " + dst;
}

PropertyStore* Connection::get_props()
{
    return props_;
}

NamedId::NamedId(String sid, String sname)
{
    id=sid;
    name=sname;
}

NamedId::~NamedId()
{

}

Atom::Atom(String id, bool root, workbench::c2p_t *backend, Workspace* w)
{
    id_=id;
    root_=root;
    backend_=backend;
    w_=w;
    //name_=stdToJuceString(backend_->get_desc(juceToStdString(id_)));
}

Atom::~Atom()
{
    w_->getValueMonitor()->remove(this);
//    std::cout<<"atom destructor"<<id_.toUTF8()<<" "<<this<<std::endl;
}

Atom* Atom::get_parent()
{
    String pid=String();
    std::set<std::string> ids=backend_->get_parent(juceToStdString(id_));
    std::set<std::string>::const_iterator i = ids.begin();
    if (i!=ids.end())
    {
        pid = String::fromUTF8(i->c_str());
    }
    return new Atom(pid,false,backend_, w_);
}

void Atom::setName(const String& name)
{
    backend_->setName(juceToStdString(id_),juceToStdString(name));
}

void Atom::getSourceKeys(const String& name)
{
    //std::cout<<"Atom::getSourceKeys"<<name.toUTF8()<<std::endl;
    backend_->get_sourcekeys(juceToStdString(id_),juceToStdString(name));
}

bool Atom::is_slave_of(Atom* other)
{
    return backend_->is_slave_of(juceToStdString(id_),juceToStdString(other->get_id()));
}

unsigned Atom::child_count()
{
    return backend_->child_count(juceToStdString(id_));
}

void Atom::setup_child_ids(const std::set<std::string> &ids)
{
    children_.clear();
    std::set<std::string>::const_iterator i = ids.begin();

    while(i!=ids.end())
    {
        String s = String::fromUTF8(i->c_str());
          children_.push_back(NamedId(s,stdToJuceString(backend_->get_desc(juceToStdString(s)))));
        i++; 
    }

    std::sort(children_.begin(),children_.end(), sortChildren);

}



Atom* Atom::get_child(unsigned index)
{
    if(index==0)
    {
        setup_child_ids(backend_->get_children(juceToStdString(id_)));
    }

    //pic::logmsg()<<"get_child "<<index;
    if(children_.size()>index)
    {
        return new Atom((children_[index]).id,false,backend_,w_);
    }
    return 0;
}

Atom* Atom::get_child(String name)
{
    String cid= stdToJuceString(  backend_->get_child(juceToStdString(id_),juceToStdString(name))); 
    if(cid.isNotEmpty())
    {
        return new Atom(cid,false,backend_,w_);
    }
    return 0;
}

bool Atom::hasEditableValue()
{
    return backend_->hasEditableValue(juceToStdString(id_));
}

bool Atom::hasProperty(String prop)
{
    return backend_->has_property(juceToStdString(id_),juceToStdString(prop));
}

String Atom::getProperty(String prop)
{
    return stdToJuceString(backend_->get_property(juceToStdString(id_),juceToStdString(prop)));
}


bool Atom::has_master()
{
    return backend_->has_master(juceToStdString(id_));
}

bool Atom::has_non_controller_master()
{
    return backend_->has_non_controller_master(juceToStdString(id_));
}


String Atom::get_scope()
{
    return stdToJuceString(backend_->get_scope(juceToStdString(id_)));
}

String Atom::get_absoluteID()
{
    return stdToJuceString(backend_->get_absoluteID(juceToStdString(id_)));
}

String Atom::get_value()
{
    return stdToJuceString(backend_->get_value(juceToStdString(id_)));
}

int Atom::get_ordinal()
{
    return backend_->get_ordinal(juceToStdString(id_));
}

void Atom::set_value(bool val)
{
    pic::logmsg()<< "setting value of "<<std::string(id_.toUTF8())<<" to "<<val;
    backend_->set_boolvalue(juceToStdString(id_),val);
}

void Atom::set_value(int val)
{
    pic::logmsg()<< "setting value of "<<std::string(id_.toUTF8())<<" to "<<val;
    backend_->set_intvalue(juceToStdString(id_),val);

}
void Atom::set_value(float val)
{
    pic::logmsg()<< "setting value of "<<std::string(id_.toUTF8())<<" to "<<val;
    backend_->set_floatvalue(juceToStdString(id_),val);
}

void Atom::set_value(String val)
{
    pic::logmsg()<< "setting value of "<<std::string(id_.toUTF8())<<" to "<<juceToStdString(val);
    backend_->set_stringvalue(juceToStdString(id_),juceToStdString(val));
}

void Atom::invoke(String val,int i)
{
    backend_->invoke(juceToStdString(id_),juceToStdString(val),i);
}


String Atom::get_domain()
{
    return stdToJuceString(backend_->get_domain(juceToStdString(id_)));
}

bool Atom::has_protocol(String prot)
{
    return backend_->has_protocol(juceToStdString(id_), juceToStdString(prot));
}

//// XXX
void Atom::get_current()
{
    if(has_protocol("browse"))
    {
        backend_->current(juceToStdString(id_));
    }
}

void Atom::activate(String path,String cookie)
{
    backend_->activate(juceToStdString(id_),juceToStdString(path),juceToStdString(cookie));
}


void Atom::enumerate(String path)
{
    if(has_protocol("browse"))
    {
       backend_->enumerate(juceToStdString(id_),juceToStdString(path));
    }
}

void Atom::finfo(String path)
{
    if(has_protocol("browse"))
    {
        backend_->finfo(juceToStdString(id_), juceToStdString(path));
    }
}

void Atom::cinfo(String path)
{
    if(has_protocol("browse"))
    {
        backend_->cinfo(juceToStdString(id_),juceToStdString(path),0,10000);
    }
}


void Atom::get_instances()
{
   if(has_protocol("create"))
   {
       backend_->get_instances(juceToStdString(id_));
   }
}

void Atom::get_instanceName()
{
    if(has_protocol("create"))
    {
        backend_->get_instanceName(juceToStdString(id_));
    }
}

void Atom::create_instance(int ordinal)
{
    backend_->create_instance(juceToStdString(id_),ordinal);
}

void Atom::delete_instance(const String &cid)
{
    backend_->delete_instance(juceToStdString(id_),juceToStdString(cid));
}

Atom* Atom::getDescendant(String id)
{
    return new Atom(id, false, backend_,w_);
}

void Atom::getDescendants(std::set<String>& ids)
{
    // XXX duplicated from workspace
    std::set<std::string>dids=backend_->get_descendants(juceToStdString(id_));
    std::set<std::string>::const_iterator i = dids.begin();
    unsigned count = 0;
    
    while(i!=dids.end())
    {
        String s = String::fromUTF8(i->c_str());
        ids.insert(s);
        i++; count++;
    }
}

void Atom::getChildren(std::set<String>& ids)
{
    std::set<std::string>cids=backend_->get_children(juceToStdString(id_));
    std::set<std::string>::const_iterator i = cids.begin();
    unsigned count = 0;
    
    while(i!=cids.end())
    {
        String s = String::fromUTF8(i->c_str());
        ids.insert(s);
        i++; count++;
    }
}
void Atom::getChildProps(std::vector<NamedId>& props, bool showNames)
{
    std::set<std::string>cids=backend_->get_child_props(juceToStdString(id_),showNames);
    std::set<std::string>::const_iterator i = cids.begin();
    //unsigned count = 0;
    
//    std::vector<NamedId> props;
    while(i!=cids.end())
    {
        String s = String::fromUTF8(i->c_str());
        pic::logmsg()<<"getChildProps "<<std::string(s.toUTF8());
        props.push_back(NamedId(s,stdToJuceString(backend_->get_desc(juceToStdString(s)))));
       // ids.insert(s);
        i++; //count++;
    }

    std::sort(props.begin(),props.end(), sortChildren);

}


void Atom::get_using_inputs(std::set<String>& ids)
{
    std::set<std::string>dids=backend_->get_using_inputs(juceToStdString(id_));
    std::set<std::string>::const_iterator i = dids.begin();
    
    while(i!=dids.end())
    {
        String s = String::fromUTF8(i->c_str());
        ids.insert(s);
        i++; 
    }
}

int Atom::get_numInputs()
{
    return backend_->get_numInputs(juceToStdString(id_));
}

bool Atom::is_input()
{
    return backend_->is_input(juceToStdString(id_));
}

bool Atom::is_output()
{
    return backend_->is_output(juceToStdString(id_));
}

String Atom::get_name()
{
    return stdToJuceString(backend_->get_name(juceToStdString(id_)));
}

String Atom::get_tooltip()
{
    return stdToJuceString(backend_->get_tooltip_by_id(juceToStdString(id_)));
}

String Atom::get_helptext()
{
    return stdToJuceString(backend_->get_helptext_by_id(juceToStdString(id_)));
}

String Atom::get_desc()
{
    return stdToJuceString(backend_->get_desc(juceToStdString(id_)));
}

String Atom::get_fulldesc()
{
    std::string s=backend_->get_fulldesc(juceToStdString(id_));
//    pic::logmsg()<<"Atom::get_fulldesc"<<s;
    return stdToJuceString(s);
}

void Atom::monitor_on()
{
    backend_->monitor_on(juceToStdString(id_));
}

void Atom::monitor_off()
{
    backend_->monitor_off(juceToStdString(id_));
}

String Atom:: get_id()
{
    return id_;
}

//void Atom::add_input_connection(Connection* c)
//{
//    //input_conns_.insert(std::pair<String, Connection*>(c->get_id(),c));
//}
//
//void Atom::add_output_connection(Connection* c)
//{
//    //output_conns_.insert(std::pair<String, Connection*>(c->get_id(),c));
//}

//unsigned Atom::input_count()
//{
//
//    std::set<std::string>inputs=backend_->get_using_inputs(juceToStdString(id_));
//    return inputs.size();
////    return input_conns_.size();
//}
//
//unsigned Atom::output_count()
//{
//return 0;
////    return output_conns_.size();
//}

Agent::Agent(String id, bool isRig, PropertyStore* props, workbench::c2p_t* backend, Workspace* w)
{
    rig_=isRig;
    props_=props;
    backend_=backend;
    w_=w;
    atom_=new Atom(id,true,backend_,w_);
}

bool Agent:: is_workspace()
{
    return rig_;
}

Agent::~Agent()
{
}

Atom* Agent::get_root()
{
   return atom_; 
}



PropertyStore* Agent:: get_store()
{
    return props_;
}

PropertyStore::PropertyStore(String type, String id)
{
    p_=new XmlElement(type);
    p_->setAttribute("id",id);
}

int PropertyStore::get_number(const String &key)
{
   return p_->getIntAttribute(key); 
}

double PropertyStore::get_double(const String &key)
{
   return p_->getDoubleAttribute(key); 
}

void PropertyStore::set_number(const String &key, int value)
{
    if (p_!=0)
    {
//        pic::logmsg()<<"PropertyStore::set_number (int) "<<key.toUTF8()<<" "<<value;
        p_->setAttribute(key,value);
    }
}

void PropertyStore::set_number(const String &key, double value)
{
    if (p_!=0)
    {
//        pic::logmsg()<<"PropertyStore::set_number (double)"<<key.toUTF8()<<" "<<value;
        p_->setAttribute(key,value);
    }
}

String PropertyStore::get_string(const String &key)
{
    if(p_!=0)
    {
        if (key=="type")
        {
            return p_->getTagName();
        }
        else
        {
            return p_->getStringAttribute(key);
        }
    }
    return String();
}

void PropertyStore::set_string(const String &key, String value)
{
    if (p_!=0)
    {
        p_->setAttribute(key,value);
    }

}

bool PropertyStore::has_key(const String key)
{
    return p_->hasAttribute(key); 
}

unsigned PropertyStore::list_getsize(const String &key)
{
    unsigned size=0;
    XmlElement* l =p_->getChildByName(key.toUTF8());
    if (l!=0)
    {
        return l->getNumAttributes();
    }
    
    return size;
}

bool PropertyStore::has_list(const String &key)
{
    bool val;
    XmlElement* e= p_->getChildByName(key.toUTF8());
    val=(e!=0);
//    pic::logmsg()<<"has_list "<<key.toUTF8()<<"="<<val;
    return val; 
}

void PropertyStore::set_list(const String &key,unsigned index,const String &value )
{
//    pic::logmsg()<<"set_list: key="<<key.toUTF8()<<" index="<< index;
    XmlElement* l;
    l=p_->getChildByName(key.toUTF8());
    if (l==0)
    {
        l=new XmlElement(key);
        p_->addChildElement(l);
//        pic::logmsg()<<"childElement added";
    }
    
    l->setAttribute(String(index), value);
}

void PropertyStore::add_list_item(const String &listkey, const String &value)
{
    XmlElement* l;
    l=p_->getChildByName(listkey.toUTF8());
    if (l==0)
    {
        l=new XmlElement(listkey);
        p_->addChildElement(l);
//        pic::logmsg()<<"childElement added";
    }

    bool alreadyInList=false;
    for(int index=0; index<l->getNumAttributes();index++)
    {
        if(l->getAttributeValue(index)==value)
        {
            alreadyInList=true;
            break;
        }
    }   

    if(!alreadyInList)
    {
        int index=1;
        while (l->hasAttribute(String(index)))
        {
            index++;
        }
//        pic::logmsg()<<"adding "<<value.toUTF8()<<"to list "<<listkey.toUTF8()<<" with index "<<index;
        l->setAttribute(String(index), value);
    }
}

void PropertyStore::remove_list_item(const String &listkey,const String &value)
{
//    pic::logmsg()<<"remove list item"<<value.toUTF8();
    XmlElement* l=p_->getChildByName(listkey.toUTF8());
    
    if (l!=0)
    {
        for(int index=0; index<l->getNumAttributes();index++)
        {
            if(l->getAttributeValue(index)==value)
            {
                l->removeAttribute(l->getAttributeName(index));
//                pic::logmsg()<<"removing "<< value.toUTF8()<<" from list "<<listkey.toUTF8();
                break;
            }
        }
    }
}

bool PropertyStore::has_list_item(const String &listkey,const String &value)
{
    XmlElement* l=p_->getChildByName(listkey.toUTF8());
    
    if (l!=0)
    {
        for(int index=0; index<l->getNumAttributes();index++)
        {
            if(l->getAttributeValue(index)==value)
            {
                return true;
            }
        }
    }
    return false;
}



String PropertyStore::get_list(const String &key, unsigned index)
{
    String value=String();
    XmlElement* l;
    l=p_->getChildByName(key.toUTF8());
    if (l!=0)
    {
        value= l->getStringAttribute(String(index));
    }
    return value;
}

int PropertyStore::list_count()
{
    return p_->getNumChildElements();
}

String PropertyStore::get_listkey(int index)
{
    XmlElement* l = p_->getChildElement(index);
    return l->getTagName();
}

void PropertyStore::remove_list(const String &key)
{
    XmlElement* l=p_->getChildByName(key.toUTF8());
    if (l!=0)
    {
        p_->removeChildElement(l,true);
        pic::logmsg()<<"PropertyStore: removelist "<<std::string(key.toUTF8());
    }
}


XmlElement* PropertyStore::get_element()
{
    return p_;
}

ValueMonitor::ValueMonitor()
{
    listener_=0;
}

void ValueMonitor::add(Atom* atom)
{
    String id=atom->get_id();
    atoms_.insert(std::pair<String,Atom*>(id,atom));
//    pic::logmsg()<<id.toUTF8()<<" atom added to valueMonitor map: size= "<<atoms_.size();
    atom->monitor_on();

//    pic::logmsg()<<"valueMonitorMap";
//    for(std::map<String, Atom*>::iterator iter=atoms_.begin();iter!=atoms_.end();iter++)
//    {
//        pic::logmsg()<<"id in valuemonitor map "<<(iter->first).toUTF8()<<" "<<iter->second;
//    }
}

void ValueMonitor::printInfo()
{
    std::cout<<"valueMonitorMap size="<<atoms_.size()<<std::endl;
    for(std::map<String, Atom*>::iterator iter=atoms_.begin();iter!=atoms_.end();iter++)
    {
        std::cout<<"id in valuemonitor map "<<std::string((iter->first).toUTF8())<<" "<<iter->second<<std::endl;
    }

}

void ValueMonitor::remove(Atom* atom)
{
    atom->monitor_off();
    String id=atom->get_id();
    atoms_.erase(id);
    //int n =atoms_.erase(id);
    //pic::logmsg()<<n<<" atoms erased from valueMonitor map: size="<<atoms_.size();
    //pic::logmsg()<<"valueMonitorMap";
//    for(std::map<String, Atom*>::iterator iter=atoms_.begin();iter!=atoms_.end();iter++)
//    {
//        pic::logmsg()<<"id in valuemonitor map "<<(iter->first).toUTF8()<<" "<<iter->second;
//        //std::cout<<"id in valuemonitor map "<<(iter->first).toUTF8()<<" "<<iter->second<<std::endl;
//    }


}

void ValueMonitor::clear()
{
//    pic::logmsg()<<"valueMonitor clear: size="<<atoms_.size();
//    pic::logmsg()<<"valueMonitorMap";
//    for(std::map<String, Atom*>::iterator iter=atoms_.begin();iter!=atoms_.end();iter++)
//    {
//        pic::logmsg()<<"id in valuemonitor map "<<(iter->first).toUTF8()<<" "<<iter->second;
//    }

    for(std::map<String, Atom*>::iterator iter=atoms_.begin();iter!=atoms_.end();iter++)
    {
        (iter->second)->monitor_off();
    }


    atoms_.clear();
//    pic::logmsg()<<"valueMonitor map cleared";
}

void ValueMonitor::addListener(ValueMonitorListener* listener)
{
    listener_=listener;
    // just one listener initially
}

void ValueMonitor::removeListener()
{
    listener_=0;
}

void ValueMonitor::value_changed(String id)
{
   std::map<String,Atom*>::iterator i=atoms_.find(id);
   if(i!=atoms_.end())
   {
        if(listener_!=0)
        {
            listener_->value_changed(id);
        } 
   }
}

Workspace::Workspace(epython::PythonBackend *python_backend)
{
    python_backend_ = python_backend;
    backend_ = (workbench::c2p_t *)python_backend_->mediator();

    progressWindow_=0;
    progress_=0;
    progressBar_=0;
    progressBarDisplayed_=false;

    agentsChanged_=false;
    idCount_=0;
    listener_=0;
    agentCount_=0;
    properties_=new XmlElement("PROPERTIES");
    valueMonitor_=new ValueMonitor();
}

Workspace::~Workspace()
{
    delete python_backend_;
}

void Workspace::get_test_string(int s)
{
    backend_->get_test_string(s);
}

void Workspace::test_string(const char* s)
{
    pic::logmsg()<<"workspace test_string "<<s;
}

void Workspace::initialise_backend(String scope)
{
    backend_->initialise(this,juceToStdString(scope));
}

void Workspace::quit()
{
    backend_->quit();
}

ValueMonitor* Workspace::getValueMonitor()
{
    return valueMonitor_;
}

void Workspace::value_changed(const std::string& id)
{
    pic::logmsg()<<"workspace value_changed";
    valueMonitor_->value_changed(stdToJuceString(id));
}

//void Workspace::stateChanged(const std::string& setup)
void Workspace::stateChanged(const char * setup)
{
   start_loading();
   pic::logmsg()<<"workspace: stateChanged ";
   std::cout<<"workspace: stateChanged "<<std::endl;
   agentsChanged_=false;
   readState(String(setup));
   if(!agentsChanged_)
   {
        pic::logmsg()<<"agents not changed - call stop_progress ";
        backend_->stop_progress();
   }
}

void Workspace::instanceName(const std::string& id, const std::string& name)
{
   listener_->instanceName(stdToJuceString(id), stdToJuceString(name));
}

String Workspace::get_tooltip(String agent)
{
    return stdToJuceString(backend_->get_tooltip(juceToStdString(agent))); 
}

String Workspace::get_helptext(String agent)
{
    return stdToJuceString(backend_->get_helptext(juceToStdString(agent))); 
}


void Workspace::readState(String p)
{
    //pic::logmsg()<<"readState"<<p.toUTF8();
    pic::logmsg()<<"readState";
    tmpProps_.clear();
    XmlDocument doc(p);
    properties_=doc.getDocumentElement();

    XmlElement* e=properties_->getFirstChildElement();
    while(e!=0)
    {
        if(e->hasTagName("Agent"))
        {
            readAgent(e);
        }
        else if(e->hasTagName("Hook"))
        {
            readHook(e);
        }

        else if(e->hasTagName("HorizontalTrunk"))
        {
            readHorizontalTrunk(e);
        }

        else if(e->hasTagName("VerticalTrunk"))
        {
            readVerticalTrunk(e);
        }

        else if(e->hasTagName("TrunkCorner"))
        {
            readTrunkCorner(e);
        }

        else if(e->hasTagName("SourcePin"))
        {
            readSourcePin(e);    
        }

        else if(e->hasTagName("DestinationPin"))
        {
            readDestinationPin(e);    
        }

        else if(e->hasTagName("Workspace"))
        {
           readWorkspaceProps(e);
        }

        e=e->getNextElement();
    }

    e=properties_->getFirstChildElement();
    while(e!=0)
    {
        if(e->hasTagName("Connection"))
        {
            readRouting(e);
        }
        else if(e->hasTagName("LooseWire"))
        {
            readLooseWire(e);
        }

        e=e->getNextElement();
    }
    

    std::set<String> removedStores;
    for (std::map<String,PropertyStore*>::iterator iter=props_.begin();iter!=props_.end();iter++)
    {
        String id=iter->first;
        std::set<String>::iterator p=tmpProps_.find(id);
        if(p==tmpProps_.end())
        {
            removedStores.insert(iter->first);
        }
    }
    
    for (std::set<String>::iterator iter=removedStores.begin();iter!=removedStores.end();iter++)
    {
        storeRemoved(*iter);
    }
}

void Workspace::storeRemoved(String id)
{
    pic::logmsg()<<"Workspace::storeRemoved "<<std::string(id.toUTF8());
    PropertyStore* p=get_store(id);
    if(p!=0)
    {
        String t=p->get_string("type");        
        if (t=="Hook")
        {
            listener_->hookRemoved(id);
        }
        else if (t=="HorizontalTrunk")
        {
            listener_->horizontalTrunkRemoved(id);
        }
        else if (t=="VerticalTrunk")
        {
            listener_->verticalTrunkRemoved(id);
        }
        else if (t=="TrunkCorner")
        {
            listener_->trunkCornerRemoved(id);
        }
        else if (t=="Connection")
        {
            listener_->routingRemoved(id);
        }
        // XXX
        // loosewires and pins?
        else if(t=="Workspace")
        {
            listener_->propertiesRemoved(id);
        }
        else if(t=="Agent")
        {
            agentsChanged_=true;
        }

    }
}

void Workspace::readAgent(XmlElement* e)
{
    String id =e->getStringAttribute("id");
    PropertyStore* p;
    if (has_store(id))
    {
        p=get_store(id);
        bool changed=false;
        int newx=e->getIntAttribute("x");
        if(newx!=p->get_number("x"))
        {
            p->set_number("x",newx);
            changed=true;
        }
        
        int newy=e->getIntAttribute("y");
        if(newy!=p->get_number("y"))
        {
            p->set_number("y",newy);
            changed=true;
        }

        int newEx=e->getIntAttribute("expanded");
        if(newEx!=p->get_number("expanded"))
        {
            p->set_number("expanded",newEx);
            changed=true;
        }
// XXX expandedlist changed?

        XmlElement* elist=e->getChildByName("expandedlist");
        if(elist!=0)
        {
            for(int i=0; i<elist->getNumAttributes();i++)
            {
                unsigned index=(elist->getAttributeName(i)).getIntValue();
                String val=elist->getAttributeValue(i);
                p->set_list("expandedlist",index,val);
            }
        }

        if (changed)
        {
            listener_->agentPropertyChanged(id);
        }
    }

    else
    {
        p= create_store("Agent",id);
        agentsChanged_=true;
        p->set_number("x",e->getIntAttribute("x"));
        p->set_number("y",e->getIntAttribute("y"));
        p->set_number("expanded",e->getIntAttribute("expanded"));

        XmlElement* elist=e->getChildByName("expandedlist");
        if(elist!=0)
        {
            for(int i=0; i<elist->getNumAttributes();i++)
            {
                unsigned index=(elist->getAttributeName(i)).getIntValue();
                String val=elist->getAttributeValue(i);
                p->set_list("expandedlist",index,val);
            }
        }
    }
    tmpProps_.insert(id);

}

void Workspace::readHook(XmlElement* e)
{
    String id =e->getStringAttribute("id");
    PropertyStore* p;
    if(has_store(id))
    {
        p=get_store(id);
        // XXX only call hookChanged if x or y have changed
        p->set_number("x",e->getIntAttribute("x"));
        p->set_number("y",e->getIntAttribute("y"));
        p->set_number("sticky",e->getIntAttribute("sticky"));
        forEachXmlChildElement(*e,l)
        {
            if ((l->getTagName().startsWith("input")))
            {
                String listname=l->getTagName();
                unsigned index=0;
                while(l->hasAttribute(String(index)))
                {
                    p->set_list(listname,index,l->getStringAttribute(String(index)));
//                    std::cout<<"readHook (store exists) "<<l->getStringAttribute(String(index)).toUTF8()<<std::endl;
                    index++;
                }
            }
        }


        listener_->hookChanged(id);

    }
    else
    {
        p= create_store("Hook",id);
        p->set_number("x",e->getIntAttribute("x"));
        p->set_number("y",e->getIntAttribute("y"));
        p->set_number("sticky",e->getIntAttribute("sticky"));
        forEachXmlChildElement(*e,l)
        {
            if ((l->getTagName().startsWith("input")))
            {
                String listname=l->getTagName();
                unsigned index=0;
                while(l->hasAttribute(String(index)))
                {
                    p->set_list(listname,index,l->getStringAttribute(String(index)));
//                    std::cout<<"readHook (new store) "<<l->getStringAttribute(String(index)).toUTF8()<<std::endl;
                    index++;
                }
            }
        }

        listener_->hookAdded(id);
    }

    tmpProps_.insert(id);
}

void Workspace::readWorkspaceProps(XmlElement* e)
{
//  XXX
    String id =e->getStringAttribute("id");
    PropertyStore* p;
    if(has_store(id))
    {
        p=get_store(id);
        p->set_number("viewoffsetx",e->getIntAttribute("viewoffsetx"));
        p->set_number("viewoffsety",e->getIntAttribute("viewoffsety"));
        p->set_number("zoomfactor",e->getDoubleAttribute("zoomfactor"));
        pic::logmsg()<<"readWorkspaceProps: zoomfactor="<<e->getDoubleAttribute("zoomfactor");
        listener_->propertiesChanged(id);

    }
    else
    {
        p= create_store("Workspace",id);
        p->set_number("viewoffsetx",e->getIntAttribute("viewoffsetx"));
        p->set_number("viewoffsety",e->getIntAttribute("viewoffsety"));
        p->set_number("zoomfactor",e->getDoubleAttribute("zoomfactor"));
        listener_->propertiesAdded(id);
    }

    tmpProps_.insert(id);

}

void Workspace::readSourcePin(XmlElement* e)
{
    String id =e->getStringAttribute("id");
    PropertyStore* p;
    if(has_store(id))
    {
        p=get_store(id);
        // XXX only call sourcePinChanged if x or y have changed
        p->set_number("x",e->getIntAttribute("x"));
        p->set_number("y",e->getIntAttribute("y"));
        listener_->sourcePinChanged(id);

    }
    else
    {
        p= create_store("SourcePin",id);
        p->set_number("x",e->getIntAttribute("x"));
        p->set_number("y",e->getIntAttribute("y"));
        listener_->sourcePinAdded(id);
    }

    tmpProps_.insert(id);
}

void Workspace::readDestinationPin(XmlElement* e)
{
    String id =e->getStringAttribute("id");
    PropertyStore* p;
    if(has_store(id))
    {
        p=get_store(id);
        // XXX only call destinationPinChanged if x or y have changed
        p->set_number("x",e->getIntAttribute("x"));
        p->set_number("y",e->getIntAttribute("y"));
        listener_->destinationPinChanged(id);

    }
    else
    {
        p= create_store("DestinationPin",id);
        p->set_number("x",e->getIntAttribute("x"));
        p->set_number("y",e->getIntAttribute("y"));
        listener_->destinationPinAdded(id);
    }

    tmpProps_.insert(id);
}

void Workspace::readHorizontalTrunk(XmlElement* e)
{
    pic::logmsg()<<"horizontal trunk 1";
    String id =e->getStringAttribute("id");
    bool newStore;
    PropertyStore* p;
    if(has_store(id))
    {
        p=get_store(id);
        pic::logmsg()<<"store exists p="<<p;
        newStore=false;
    }
    else
    {
        p= create_store("HorizontalTrunk",id);
        newStore=true;
    }
    p->set_number("x",e->getIntAttribute("x"));
    p->set_number("y",e->getIntAttribute("y"));
    p->set_number("width",e->getIntAttribute("width"));
    p->set_number("height",e->getIntAttribute("height"));
    p->set_number("direction",e->getIntAttribute("direction"));
    p->set_string("assemblyId",e->getStringAttribute("assemblyId"));
    p->set_number("assemblySize",e->getIntAttribute("assemblySize"));
    p->set_number("assemblyIndex",e->getIntAttribute("assemblyIndex"));

    pic::logmsg()<<"horizontal trunk 2";
    forEachXmlChildElement(*e,l)
    {
        if ((l->getTagName().startsWith("input")))
        {
            String listname=l->getTagName();
            unsigned index=0;
            while(l->hasAttribute(String(index)))
            {
                p->set_list(listname,index,l->getStringAttribute(String(index)));
                index++;
            }
        }
    }

    if (newStore)
    {
        listener_->horizontalTrunkAdded(id); 
    }
    else
    {
        listener_->horizontalTrunkChanged(id);
    }

    tmpProps_.insert(id);
}

void Workspace::readVerticalTrunk(XmlElement* e)
{
    String id =e->getStringAttribute("id");
    PropertyStore* p;
    bool newStore;
    if(has_store(id))
    {
        newStore=false;
        p=get_store(id);
    }
    else
    {
        newStore=true;
        p= create_store("VerticalTrunk",id);
    }
    p->set_number("x",e->getIntAttribute("x"));
    p->set_number("y",e->getIntAttribute("y"));
    p->set_number("width",e->getIntAttribute("width"));
    p->set_number("height",e->getIntAttribute("height"));
    p->set_number("direction",e->getIntAttribute("direction"));
    p->set_string("assemblyId",e->getStringAttribute("assemblyId"));
    p->set_number("assemblySize",e->getIntAttribute("assemblySize"));
    p->set_number("assemblyIndex",e->getIntAttribute("assemblyIndex"));

    forEachXmlChildElement(*e,l)
    {
        if ((l->getTagName().startsWith("input")))
        {
            String listname=l->getTagName();
            unsigned index=0;
            while(l->hasAttribute(String(index)))
            {
                p->set_list(listname,index,l->getStringAttribute(String(index)));
                index++;
            }
        }
    }

    if(newStore)
    {
        listener_->verticalTrunkAdded(id); 
    }
    else
    {
        listener_->verticalTrunkChanged(id);
    }

    tmpProps_.insert(id);
}

void Workspace::readTrunkCorner(XmlElement* e)
{
    String id =e->getStringAttribute("id");
    bool newStore;
    PropertyStore* p;
    if (has_store(id))
    {
        p=get_store(id);
        newStore=false;
    }
    else
    {
        p= create_store("TrunkCorner",id);
        newStore=true;
    }
    p->set_number("x",e->getIntAttribute("x"));
    p->set_number("y",e->getIntAttribute("y"));
    p->set_number("width",e->getIntAttribute("width"));
    p->set_number("height",e->getIntAttribute("height"));
    p->set_string("assemblyId",e->getStringAttribute("assemblyId"));
    p->set_number("assemblyIndex",e->getIntAttribute("assemblyIndex"));
    p->set_number("cornerType",e->getIntAttribute("cornerType"));

    if(newStore)
    {
        listener_->trunkCornerAdded(id); 
    }
    else
    {
        listener_->trunkCornerChanged(id);
    }

    tmpProps_.insert(id);
}

void Workspace::readRouting(XmlElement* e)
{
    String id =e->getStringAttribute("id");
    bool newStore;
    PropertyStore* p;
    if(has_store(id))
    {
        p=get_store(id);
        newStore=false;
    }
    else
    {
        p= create_store("Connection",id);
        newStore=true;
    }
    XmlElement* l =e->getChildByName("routing");
    if (l!=0)
    {
        unsigned index=0;
        while(l->hasAttribute(String(index)))
        {
            p->set_list("routing",index,l->getStringAttribute(String(index)));
            index++;
        }
    }
    if (newStore)
    {
        listener_->routingAdded(id);
    }
    else
    {
        listener_->routingChanged(id);
    }

    tmpProps_.insert(id);
}

void Workspace::readLooseWire(XmlElement* e)
{
    String id =e->getStringAttribute("id");
    bool newStore;
    PropertyStore* p;
    if(has_store(id))
    {
        p=get_store(id);
        newStore=false;
    }
    else
    {
        p= create_store("LooseWire",id);
        newStore=true;
    }
    XmlElement* l =e->getChildByName("routing");
    if (l!=0)
    {
        unsigned index=0;
        while(l->hasAttribute(String(index)))
        {
            p->set_list("routing",index,l->getStringAttribute(String(index)));
            index++;
        }
    }
    if (newStore)
    {
        listener_->looseWireAdded(id);
    }
    else
    {
        listener_->looseWireChanged(id);
    }

    tmpProps_.insert(id);
}

void Workspace::getDescendants(String id,std::set<String>& ids)
{
    // XXX get rid of all the copying
    std::set<std::string>dids=backend_->get_descendants(juceToStdString(id));
    std::set<std::string>::const_iterator i = dids.begin();
    unsigned count = 0;
    
    while(i!=dids.end())
    {
        String s = String::fromUTF8(i->c_str());
        ids.insert(s);
        i++; count++;
    }
}

String Workspace::getParentId(String id)
{
    String s=String();
    std::set<std::string> ids=backend_->get_parent(juceToStdString(id));
    std::set<std::string>::const_iterator i = ids.begin();
    if (i!=ids.end())
    {
        s = String::fromUTF8(i->c_str());
    }
    return s;
}

void Workspace::get_connections(String id)
{
    std::set<std::string> cons=backend_->get_connections(juceToStdString(id));
    std::set<std::string>::const_iterator i=cons.begin();
    std::set<String> conset;
    while(i!=cons.end())
    {
        String s = String::fromUTF8(i->c_str());
        conset.insert(s);
        connectionAdded(juceToStdString(s)); 
        i++;
    }
   
    listener_->checkConnections(id,conset); 
}

void Workspace::addListener(WorkspaceListener* listener)
{
    listener_=listener;
    pic::logmsg()<<"Workspace:listener added"<<listener;
}

void Workspace::create_connection(Atom* src, Atom* dst)
{
    create_connection(src->get_id(), dst->get_id());

}

void Workspace::check_connection(String srcId, String dstId)
{
    pic::logmsg()<<"Workspace checkConnection:srcId="<<std::string(srcId.toUTF8())<<" dstId="<<std::string(dstId.toUTF8());
    backend_->connect_check(juceToStdString(srcId), juceToStdString(dstId));
}

void Workspace::create_connection(String srcId, String dstId)
{
    pic::logmsg()<<"Workspace createConnection:srcId="<<std::string(srcId.toUTF8())<<" dstId="<<std::string(dstId.toUTF8());
    backend_->connect(juceToStdString(srcId), juceToStdString(dstId));
}

void Workspace::create_connection(String srcId, String dstId,String u, String f,String c)
{
    int ui;
    if (u.isEmpty())
    {
        ui=0;
    }
    else
    {
        ui=u.getIntValue();
    }

    backend_->connect_test(juceToStdString(srcId), juceToStdString(dstId), ui,juceToStdString(f),juceToStdString(c));

}
void Workspace::connectionAdded(const std::string& cid)
{
    String id=stdToJuceString(cid);
    listener_->connection_created(id);
}

Connection* Workspace::get_connection(String cid)
{
    PropertyStore* props=get_store(cid);
//    if (props==0)
//    {
//        pic::logmsg()<<"property_store not found for "<<cid.toUTF8();
//    }
    String sourceId=cid.upToFirstOccurrenceOf(":",false,true);
    String dstId=cid.fromFirstOccurrenceOf(":",false,true);
    Connection* con=new Connection(sourceId, dstId,props,backend_);
    return con;
}

void Workspace::delete_connection(String id, String u, String f, String c)
{
    String sid=id.upToFirstOccurrenceOf(":",false,true);
    String did=id.fromFirstOccurrenceOf(":",false,true);
    int ui;
    if (u.isEmpty())
    {
        ui=0;
    }
    else
    {
        ui=u.getIntValue();
    }
    backend_->disconnect(juceToStdString(sid),juceToStdString(did),ui,juceToStdString(f),juceToStdString(c));
}

void Workspace::createAgent(int x, int y, String agentType, int ordinal)
{
    backend_->create_agent(juceToStdString(agentType),ordinal);
    String name=agentType.toLowerCase() + " "+String(ordinal);
    agentPosX_.insert(std::pair<String, int>(name,x));
    agentPosY_.insert(std::pair<String, int>(name,y));
}

void Workspace:: agentAdded(const std::string& id)
{
//    pic::logmsg()<<"Workspace:agentAdded "<<id;
    String aid=stdToJuceString(id);
    if (get_store(aid)==0)
    {
//        pic::logmsg()<<"creating store";
        PropertyStore* p= create_store("Agent",aid);
        setPosition(p,aid);
        init_store(p);
    }
    agentCount_++;
    listener_-> agentAdded(aid);
}

void Workspace::setPosition(PropertyStore* p,String aid)
{
    String name=(stdToJuceString(backend_->get_desc(juceToStdString(aid)))).toLowerCase();
    pic::logmsg()<<"Workspace::setPosition for new agent "<<std::string(aid.toUTF8())<<" name="<<std::string(name.toUTF8());
    std::map<String,int>::iterator i=agentPosX_.find(name);
    std::map<String,int>::iterator j=agentPosY_.find(name);
    if ((i!=agentPosX_.end()) && (j!=agentPosY_.end()))
    {
        p->set_number("x",i->second);
        p->set_number("y",j->second);
        pic::logmsg()<<"using stored position";
        agentPosX_.erase(name);
        agentPosY_.erase(name);
    }

    else
    {
        setTemporaryPosition(p,aid);
    }
}

void Workspace::setTemporaryPosition(PropertyStore* p,String aid)
{
        int x=0;
        int y=0;
        listener_->getTemporaryPosition(x,y);
        p->set_number("x",x);
        p->set_number("y",y);
}

String Workspace::generateID()
{
    return String(Time::currentTimeMillis())+ String(idCount_++);
}


void Workspace::init_store(PropertyStore* p)
{
    properties_->addChildElement(p->get_element());
    set_state();
}

void  Workspace::createHook(int x, int y)
{
    String id =generateID();
    PropertyStore* p= create_store("Hook",id);

    p->set_number("x",x);
    p->set_number("y",y);
    p->set_number("sticky",0);
    init_store(p);
    listener_-> hookAdded(id);
}

void Workspace::createLooseWire(String id)
{
    PropertyStore* p= create_store("LooseWire",id);
    init_store(p);
    listener_->looseWireAdded(id);
}

void Workspace::createSourcePin(int x, int y)
{
    String id ="SourcePin"+generateID();
    PropertyStore* p= create_store("SourcePin",id);

    p->set_number("x",x);
    p->set_number("y",y);
    init_store(p);
    listener_-> sourcePinAdded(id);
}

void Workspace::createDestinationPin(int x, int y)
{
    String id ="DestinationPin"+generateID();
    PropertyStore* p= create_store("DestinationPin",id);

    p->set_number("x",x);
    p->set_number("y",y);
    init_store(p);
    listener_-> destinationPinAdded(id);
}

void Workspace::createTrunk(int x, int y, int width, int height, int orientation,int direction, String assemblyId,int assemblySize, int index)
{
    pic::logmsg()<<"creating trunk: assemblyId="<<assemblyId<<" assemblySize="<<assemblySize<<" index="<<index;
    if (orientation==0)
    {
        createHorizontalTrunk(x,y,width,height,direction, assemblyId,assemblySize, index);
    }
    else
    {
        createVerticalTrunk(x,y,width,height,direction, assemblyId,assemblySize, index);
    }
}

void  Workspace::createHorizontalTrunk(int x, int y, int width, int height,int direction, String assemblyId,int assemblySize, int index)
{
    String id =generateID();;
    PropertyStore* p= create_store("HorizontalTrunk",id);
    p->set_number("x",x);
    p->set_number("y",y);
    p->set_number("width",width);
//    p->set_number("height",height);
    p->set_number("height",50);
    p->set_number("direction",direction);
    p->set_string("assemblyId", assemblyId);
    p->set_number("assemblySize",assemblySize);
    p->set_number("assemblyIndex",index);
    
    init_store(p);
    listener_-> horizontalTrunkAdded(id);
}

void  Workspace::createVerticalTrunk(int x, int y, int width, int height,int direction, String assemblyId,int assemblySize, int index)
{
    String id =generateID();;
    PropertyStore* p= create_store("VerticalTrunk",id);
    p->set_number("x",x);
    p->set_number("y",y);
    //p->set_number("width",width);
    p->set_number("width",50);
    p->set_number("height", height);
    p->set_number("direction",direction);
    p->set_string("assemblyId", assemblyId);
    p->set_number("assemblySize",assemblySize);
    p->set_number("assemblyIndex",index);
 
    init_store(p);
    listener_-> verticalTrunkAdded(id);
}

void Workspace::createTrunkCorner(int x, int y,String assemblyId, int assemblyIndex, int cornerType)
{
    String id =generateID();;
    PropertyStore* p= create_store("TrunkCorner",id);

    p->set_number("x",x-25);
    p->set_number("y",y-25);
    p->set_number("width",50);
    p->set_number("height", 50);
    p->set_string("assemblyId", assemblyId);
    p->set_number("assemblyIndex",assemblyIndex);
    p->set_number("cornerType",cornerType);
    pic::logmsg()<<"creating corner: assemblyId="<<assemblyId<<" assemblyIndex="<<assemblyIndex<<" cornerType="<<cornerType;

    init_store(p);
    listener_->trunkCornerAdded(id);
}

void Workspace::remove_store(PropertyStore* p)
{
    String type=p->get_string("type");
    String id =p->get_string("id");;
    bool stateChanged=false;

    pic::logmsg()<<"workspace::remove_store "<<std::string(type.toUTF8())<<" " <<std::string(id.toUTF8());
    forEachXmlChildElement(*properties_,e)
    {
        if(e->hasTagName(type)&& id==e->getStringAttribute("id"))
        {
            properties_->removeChildElement(e,false);
            stateChanged=true;
            break;
        }
    }
    props_.erase(id);
    delete p;
    if (stateChanged)
    {
        set_state();
    }
}

PropertyStore* Workspace::create_store(String type,String value)
{
//    pic::logmsg()<<"create_store"<<type.toUTF8()<<" "<<value.toUTF8();
    PropertyStore* p=new PropertyStore(type,value); 
    props_.insert(std::pair<String, PropertyStore*>(value,p));
    return p;
}

PropertyStore* Workspace::get_store(String pid)
{
    std::map<String,PropertyStore*>::iterator pos=props_.find(pid);
    if (pos!=props_.end())
    {
        return pos->second;
    }
    else
    {
        return 0;
    }
}

bool Workspace::has_store(String pid)
{
    std::map<String,PropertyStore*>::iterator pos=props_.find(pid);
    return pos!=props_.end();
}

Agent* Workspace::getAgent(String agentId)
{
    PropertyStore* props=get_store(agentId);
    bool isRig=backend_->is_rig(juceToStdString(agentId));
    Agent* agent=new Agent(agentId, isRig,props,backend_,this);
    return agent;
}

void Workspace::deleteAgent(String id)
{
    backend_->delete_agent(juceToStdString(id));
}

void Workspace::agentRemoved(const std::string& id)
{
   agentCount_--;
   listener_->agentRemoved( stdToJuceString(id));
}

void Workspace::sourcekeys_updated(const std::string& id, const std::string& sourcekeys)
{
    listener_->sourcekeys_updated(stdToJuceString(id), stdToJuceString(sourcekeys));
}

void Workspace::connectionPossible(const std::string& sid, const std::string& did, bool possible)
{
    listener_->connectionPossible(stdToJuceString(sid),stdToJuceString(did),possible);
}

void Workspace::agents_updated(const std::set<std::string> & agents)
{
    listener_->agentsUpdated(agents);
}

void Workspace::report_error(const std::string &s1, const std::string &s2)
{
    listener_->report_error(stdToJuceString(s1), stdToJuceString(s2));
}

void Workspace::instances_updated(const std::set<std::string> & ords)
{
    pic::logmsg()<<"workspace: instancesUpdated";
    listener_->instancesUpdated(ords);
}

void Workspace::cinfo_updated(const std::string &s,const std::string &path,const std::set<std::string> & cinfo)
{
    pic::logmsg()<<"workspace: cinfo_updated";
    listener_->cinfo_updated(stdToJuceString(s),stdToJuceString(path),cinfo);
}
void Workspace::finfo_updated(const std::string &s,const std::string &path,const std::set<std::string> & finfo)
{
    pic::logmsg()<<"workspace: finfo_updated";
    listener_->finfo_updated(stdToJuceString(s),stdToJuceString(path),finfo);
}

void Workspace::enumerate_updated(const std::string &s,const std::string &path,int nf, int nc)
{
    pic::logmsg()<<"workspace: enumerate_updated";
    listener_->enumerate_updated(stdToJuceString(s),stdToJuceString(path),nf,nc);
}

void Workspace::activated(const std::string &s)
{
    pic::logmsg()<<"workspace: activated";
    listener_->activated(stdToJuceString(s));
}

void Workspace::current(const std::string& id, const std::string& cookie)
{
    pic::logmsg()<<"workspace: current";
    listener_->current(stdToJuceString(id), stdToJuceString(cookie));
}


void Workspace::instanceRemoved(const std::string& id)
{
   pic::logmsg()<<"workspace: instanceRemoved "<<id;
   // XXX test for crash on delete connected keygroup output
   get_connections(stdToJuceString(id));

   listener_->instanceRemoved( stdToJuceString(id));
}

void Workspace::portRemoved(const std::string& id)
{
   pic::logmsg()<<"workspace: portRemoved "<<id;
   listener_->portRemoved( stdToJuceString(id));
}
void Workspace::agentChanged(const std::string& id)
{
   pic::logmsg()<<"workspace: agentChanged "<<id;
   get_connections(stdToJuceString(id));
}

void Workspace::set_state()
{
    backend_->set_state(std::string(properties_->createDocument("").toUTF8()));
}

void Workspace::nameChanged(const std::string& id)
{
   pic::logmsg()<<"workspace: nameChanged "<<id;
   listener_->nameChanged(stdToJuceString(id));
}

void Workspace::save_store(PropertyStore* p)
{
    //std::cout<<"Workspace::save_store"<<std::endl;
    if (p!=0)
    {
        String id =p->get_string("id");;
        XmlElement* e=p->get_element();

        String requiredTagName=e->getTagName();

        //pic::logmsg()<<"save_store: requiredTagName="<<requiredTagName.toUTF8();
        //std::cout<<"save_store: requiredTagName="<<requiredTagName.toUTF8()<<std::endl;
        forEachXmlChildElementWithTagName(*properties_,oldElement,requiredTagName)
        {
            if(oldElement->getStringAttribute("id")==id)         
            {
                properties_->replaceChildElement(oldElement,e);
                break;
            }
        }

        set_state();
        //pic::logmsg()<<"save_store: set_state returned";
        //std::cout<<"save_store: set_state returned"<<std::endl;
    }
}

void Workspace::save_store(String id)
{
    pic::logmsg()<<"save_store "<<std::string(id.toUTF8());
    PropertyStore* p=get_store(id);
    save_store(p);
}

bool Workspace::has_protocol(String id, String prot)
{
    return backend_->has_protocol(juceToStdString(id), juceToStdString(prot));
}

void Workspace::getAgentNames()
{
    backend_->get_agents();
}

String Workspace::get_absoluteID(String id)
{
    return stdToJuceString(backend_->get_absoluteID(juceToStdString(id)));
}

String Workspace::getFullName(String id)
{
    return stdToJuceString(backend_->get_fulldesc(juceToStdString(id)));
}

void Workspace::loaded(float p)
{
    if(p>1)
    {
        p=1.0f;
    }
    if((double)p>=oldProgress_)
    {
        progress_=(double)p;
    }
    else
    {
        return;
    }

    pic::logmsg()<<"frontend: "<< p*100 << " percent of agents loaded";
    if(progressWindow_==0)
    {
        if(p<0.5)
        {
            progressBarDisplayed_=false;
            start_loading();
            progressBarDisplayed_=true;
        }
    }
    
    if(progressWindow_!=0)
    {
        if((progress_-oldProgress_)>0.02)
        {
            oldProgress_=progress_;
            MessageManager::getInstance()->runDispatchLoopUntil(40);
        }

        if (p>=0.99)
        {
            deleteProgressWindow();
        }
    }
}

void Workspace::start_loading()
{
    if (!progressBarDisplayed_)
    {
        if(progressWindow_!=0)
        {
            deleteProgressWindow();
        }

        pic::logmsg()<<"create Progress window";
        progress_=0;
        progressBar_=new ProgressBar(progress_);
        progressWindow_=new ResizableWindow("Loading",false);
        Component* tt=dynamic_cast<Component*>(listener_);

        if(tt!=0)
        {
            ProgressLayer* pl=tt->findParentComponentOfClass<ProgressLayer>();
            //ResizableWindow* tlw=dynamic_cast<ResizableWindow*>(tt->getTopLevelComponent());
            if(pl!=0)
            {
                //tlw->getContentComponent()->addAndMakeVisible(progressWindow_);
                pl->addAndMakeVisible(progressWindow_);
                progressWindow_->centreAroundComponent(pl, 300,22);
            }
        }

        progressWindow_->setResizable(false,false);
        progressWindow_->setContentComponent(progressBar_); 
        progressWindow_->setVisible(true);
        oldProgress_=0;
    }
}

void Workspace::deleteProgressWindow()
{
    pic::logmsg()<<"delete Progress window";
    delete progressWindow_;
    progressWindow_=0;

}


