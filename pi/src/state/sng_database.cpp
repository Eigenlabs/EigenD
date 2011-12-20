
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

#include "sng_database.h"
#include <piembedded/pie_message.h>
#include <piembedded/pie_wire.h>
#include <set>
#include <map>
#include <picross/pic_log.h>

namespace
{
    class xnode_t;
    class xagent_t;
    class xsnapshot_t;
    class xdatabase_t;

    typedef pic::weak_t<xnode_t> backref_t;

    static xnode_t *promote(const pi::state::noderef_t &n)
    {
        return (xnode_t *)(n.checked_ptr());
    }

    static xagent_t *promote(const pi::state::agentref_t &n)
    {
        return (xagent_t *)(n.checked_ptr());
    }

    static xsnapshot_t *promote(const pi::state::snapref_t &n)
    {
        return (xsnapshot_t *)(n.checked_ptr());
    }

    class xnode_t: virtual public pic::tracked_t, public pi::state::node_t
    {
        private:
            xnode_t(const backref_t &parent, const pi::state::dbref_t &db): parent_(parent), db_(db), pos_(0), loaded_(true) {}
            xnode_t(const backref_t &parent, const pi::state::dbref_t &db, unsigned long pos): parent_(parent), db_(db), pos_(pos), loaded_(false) { load1__(pos); }
            ~xnode_t() { tracked_invalidate(); }

        public:
            static pi::state::noderef_t create(const backref_t &parent, const pi::state::dbref_t &db) { return pic::ref(new xnode_t(parent,db)); }
            static pi::state::noderef_t create(const backref_t &parent, const pi::state::dbref_t &db, unsigned long pos) { return pic::ref(new xnode_t(parent,db,pos)); }

            piw::data_t get_data() const { return value_; }
            bool set_data(const piw::data_t &v) { PIC_ASSERT(db_->writeable()); if(v!=value_) { value_=v; dirty(); return true; } return false; }
            void dirty() { xnode_t *n = this; while(n) { PIC_ASSERT(db_->writeable()); n->load__(); n->pos_=0; n=n->parent_.ptr(); } }
            bool isdirty() const { return pos_==0; }
            unsigned long save() { load__(); if(isdirty()) { pos_=save__(); } return pos_; }
            void erase() { PIC_ASSERT(db_->writeable()); if(parent_.isvalid()) parent_->erase_node__(this); }
            pi::state::noderef_t snapshot() { return create(backref_t(), db_, save()); }

            void copy(const pi::state::noderef_t &src, const pi::state::mapref_t &mapping)
            {
                PIC_ASSERT(db_->writeable());
                load__();
                dirty();

                if(mapping.isvalid())
                {
                    set_data(mapping->substitute(src->get_data()));
                }
                else
                {
                    set_data(src->get_data());
                }

                std::map<unsigned,pi::state::noderef_t>::iterator i;
                
                while((i=children_.begin())!=children_.end())
                {
                    promote(i->second)->parent_.clear();
                    children_.erase(i);
                }

                xnode_t *s = promote(src);
                s->load__();

                for(i=s->children_.begin(); i!=s->children_.end(); i++)
                {
                    get_child(i->first)->copy(i->second,mapping);
                }
            }

            std::string list_children()
            {
                load__();

                std::map<unsigned,pi::state::noderef_t>::iterator i;
                std::string s;

                for(i=children_.begin();i!=children_.end();i++)
                {
                    s+=(char)(i->first);
                }

                return s;
            }

            unsigned char enum_children(unsigned char n)
            {
                load__();

                for(;;)
                {
                    if(n==255)
                    {
                        return 0;
                    }

                    n++;

                    if(children_.find(n)!=children_.end())
                    {
                        return n;
                    }
                }
            }

            pi::state::noderef_t get_child0(unsigned char n)
            {
                load__();

                std::map<unsigned,pi::state::noderef_t>::iterator i=children_.find(n);

                if(i!=children_.end())
                {
                    return i->second;
                }

                return pi::state::noderef_t();
            }

            pi::state::noderef_t get_child(unsigned char n)
            {
                load__();

                std::map<unsigned,pi::state::noderef_t>::iterator i=children_.find(n);

                if(i!=children_.end())
                {
                    return i->second;
                }

                PIC_ASSERT(db_->writeable());
                pi::state::noderef_t x = create(backref_t(this), db_);
                children_.insert(std::make_pair(n,x));
                dirty();
                return x;
            }

            void erase_child(unsigned char n)
            {
                PIC_ASSERT(db_->writeable());
                load__();

                std::map<unsigned,pi::state::noderef_t>::iterator i=children_.find(n);

                if(i==children_.end())
                {
                    return;
                }

                dirty();
                promote(i->second)->parent_.clear();
                children_.erase(n);
            }

            void load__() { if(!loaded_) load2__(pos_); loaded_=true; }
            void load1__(unsigned long pos)
            {
                const unsigned char *buffer;
                unsigned size;
                const unsigned char *dp;
                unsigned df,x;
                unsigned short dl;
                
                buffer = db_->get_file()->read_payload(pos,&size);

                x=pie_getdata(buffer,size,&df,&dl,&dp);
                PIC_ASSERT(x>0);
                PIC_ASSERT((size-x)%5==0);
                value_ = piw::makewire(dl,dp);
            }

            void load2__(unsigned long pos)
            {
                const unsigned char *buffer;
                unsigned size;
                unsigned x,y;
                std::map<unsigned,unsigned long> c;
                std::map<unsigned,unsigned long>::iterator i;

                uint32_t p;
                unsigned n;
                pi::state::noderef_t node;

                buffer = db_->get_file()->read_payload(pos,&size);

                x=pie_skipdata(buffer,size);
                PIC_ASSERT(x>0);
                PIC_ASSERT((size-x)%5==0);
                y=(size-x)/5;

                buffer+=x;

                // collect nodes before creating them as node
                // creation will invalidate our buffer

                while(y>0)
                {
                    n=buffer[0];
                    pie_getu32(&buffer[1],4,&p);
                    c.insert(std::make_pair(n,p));
                    y--; buffer+=5;
                }

                for(i=c.begin(); i!=c.end(); i++)
                {
                    node=create(backref_t(this),db_,i->second);
                    children_.insert(std::make_pair(i->first,node));
                }
            }

            unsigned long save__()
            {
                PIC_ASSERT(db_->writeable());

                std::map<unsigned,pi::state::noderef_t>::iterator i;
                std::map<unsigned,unsigned long> c;
                std::map<unsigned,unsigned long>::iterator ic;

                // flush out children first

                for(i=children_.begin(); i!=children_.end(); i++)
                {
                    c.insert(std::make_pair(i->first,i->second->save()));
                }

                unsigned x=pie_datalen(value_.wire_length());
                unsigned y = c.size();
                unsigned size = y*5+x;
                unsigned long pos;
                unsigned char *buffer = db_->get_file()->write_payload(size,&pos,false);

                pie_setdata(buffer,x,0,value_.wire_length(),value_.wire_data());

                buffer+=x;

                for(ic=c.begin();ic!=c.end();ic++)
                {
                    buffer[0]=ic->first;
                    pie_setu32(&buffer[1],4,ic->second);
                    buffer+=5;
                }

                return pos;
            }

            void erase_node__(xnode_t *n)
            {
                load__();

                std::map<unsigned,pi::state::noderef_t>::iterator i=children_.begin();

                while(i!=children_.end())
                {
                    if(i->second==n)
                    {
                        children_.erase(i);
                        n->parent_.clear();
                        dirty();
                        return;
                    }

                    i++;
                }
            }

        private:
            backref_t parent_;
            pi::state::dbref_t db_;
            unsigned long pos_;
            bool loaded_;

            std::map<unsigned,pi::state::noderef_t> children_;
            piw::data_t value_;
    };

    class xagent_t: public pi::state::agent_t
    {
        private:
            xagent_t(const pi::state::dbref_t &db, unsigned type, const std::string &name): db_(db), pos_(0), checkpoint_(0), name_(name), type_(type) { root_=xnode_t::create(backref_t(),db_); }
            xagent_t(const pi::state::dbref_t &db, unsigned long pos): db_(db), pos_(pos), checkpoint_(pos) { load1__(pos); }

        public:
            static pi::state::agentref_t create(const pi::state::dbref_t &db, unsigned type, const std::string &name) { return pic::ref(new xagent_t(db,type,name)); }
            static pi::state::agentref_t create(const pi::state::dbref_t &db, unsigned long pos) { return pic::ref(new xagent_t(db,pos)); }

            std::string get_address() const { return name_; }
            pi::state::noderef_t get_root() { load__(); return root_; }
            unsigned get_type() const { return type_; }
            unsigned long save() { PIC_ASSERT(db_->writeable()); load__(); if(!pos_ || isdirty()) { pos_=save__(); } return pos_; }
            unsigned long set_checkpoint() { save(); checkpoint_ = pos_; return pos_; }
            unsigned long get_checkpoint() const { return checkpoint_; }
            bool isdirty() const { if(pos_==0) return true; if(!root_.isvalid()) return false; return promote(root_)->isdirty(); }
            void set_type(unsigned type) { load__(); type_=type; pos_=0; }

            std::string get_plugin()
            {
                load__();
                piw::data_t meta_data = promote(root_)->get_data();

                if(!meta_data.is_dict()) return "";

                piw::data_t n = meta_data.as_dict_lookup("plugin");
                if(n.is_string()) return n.as_string();
                return "";
            }

            std::string get_name()
            {
                load__();
                piw::data_t meta_data = promote(root_)->get_data();

                if(!meta_data.is_dict()) return "";

                piw::data_t n = meta_data.as_dict_lookup("name");
                piw::data_t o = meta_data.as_dict_lookup("ordinal");
                piw::data_t cn = meta_data.as_dict_lookup("cname");
                piw::data_t co = meta_data.as_dict_lookup("cordinal");

                std::ostringstream s;

                if(n.is_string() && n.as_string())
                {  
                    s<<n.as_string();
                }
                else
                {
                    if(cn.is_string() && cn.as_string())
                    {
                        s<<cn.as_string();
                    }
                }

                if(o.is_long())
                {
                    if(o.as_long())
                    {
                        s<<' '<<o.as_long();
                    }
                }
                else
                {
                    if(co.is_long() && co.as_long())
                    {
                        s<<' '<<co.as_long();
                    }
                }
                

                return s.str();
            }

            pi::state::agentref_t checkpoint()
            {
                if(checkpoint_)
                {
                    return create(db_,checkpoint_);
                }
                else
                {
                    return create(db_,type_,name_);
                }
            }

            void load__() { if(!root_.isvalid()) load2__(pos_); }
            void load1__(unsigned long pos)
            {
                const unsigned char *buffer;
                unsigned size;
                unsigned x;

                buffer = db_->get_file()->read_payload(pos,&size);

                PIC_ASSERT(size>=6);
                x=buffer[0];
                type_ = buffer[1];
                PIC_ASSERT(size==6+x);
                name_=std::string((const char *)&buffer[6],x);
            }
            void load2__(unsigned long pos)
            {
                const unsigned char *buffer;
                unsigned size;
                unsigned x;
                uint32_t p;

                buffer = db_->get_file()->read_payload(pos,&size);

                PIC_ASSERT(size>=6);
                x=buffer[0];
                PIC_ASSERT(size==6+x);
                pie_getu32(&buffer[2],4,&p);

                root_=xnode_t::create(backref_t(),db_,p);
            }
            unsigned long save__()
            {
                unsigned x = name_.length();
                unsigned size = x+6;
                unsigned long p = root_->save();
                unsigned long pos;

                unsigned char *buffer = db_->get_file()->write_payload(size,&pos,false);

                buffer[0]=x;
                buffer[1]=type_;
                pie_setu32(&buffer[2],4,p);
                memcpy(&buffer[6],name_.c_str(),x);

                return pos;
            }

        private:
            pi::state::noderef_t root_;

            pi::state::dbref_t db_;
            unsigned long pos_;
            unsigned long checkpoint_;

            std::string name_;
            unsigned type_;
    };

    class xsnapshot_t: public pi::state::snapshot_t
    {
        private:
            xsnapshot_t(const pi::state::dbref_t &db): db_(db), pos_(0), loaded_(true), timestamp_(0), previous_(0) {}
            xsnapshot_t(const pi::state::dbref_t &db, unsigned long pos): db_(db), pos_(pos), loaded_(false) { load1__(pos); }

        public:
            static pi::state::snapref_t create(const pi::state::dbref_t &db) { return pic::ref(new xsnapshot_t(db)); }
            static pi::state::snapref_t create(const pi::state::dbref_t &db, unsigned long pos) { return pic::ref(new xsnapshot_t(db,pos)); }

            unsigned long previous() { return previous_; }
            unsigned long version() { return pos_; }
            unsigned long long timestamp() { return timestamp_; }
            std::string tag() { return tag_; }

            pi::state::snapref_t clone(unsigned long long ts, const char *tag)
            {
                unsigned long pos = save(ts,tag);
                return create(db_,pos);
            }

            void erase()
            {
                PIC_ASSERT(db_->writeable());

                load__();
                std::set<pi::state::agentref_t>::iterator i;

                while((i=agents_.begin())!=agents_.end())
                {
                    agents_.erase(i);
                }
            }

            void copy(const pi::state::snapref_t &src, const pi::state::mapref_t &mapping, bool save)
            {
                PIC_ASSERT(db_->writeable());

                load__();
                std::set<pi::state::agentref_t>::iterator i;

                while((i=agents_.begin())!=agents_.end())
                {
                    agents_.erase(i);
                }

                xsnapshot_t *s = promote(src);

                for(i=s->agents_.begin();i!=s->agents_.end();i++)
                {
                    std::string an((*i)->get_address());

                    if(mapping.isvalid())
                    {
                        an = mapping->substitute_string(an);
                    }

                    pi::state::agentref_t a = xagent_t::create(db_,(*i)->get_type(),an);
                    agents_.insert(a);
                    a->get_root()->copy(promote(*i)->get_root(),mapping);   

                    if(save)
                    {
                        a->set_checkpoint();
                    }
                }
            }

            unsigned long save(unsigned long long ts, const char *tag)
            {
                PIC_ASSERT(db_->writeable());

                timestamp_=ts;
                previous_=db_->get_file()->checkpoint();
                tag_=tag;
                pos_=save__();
                return pos_;
            }

            unsigned agent_count()
            {
                load__();
                return agents_.size();
            }

            bool erase_agent(const pi::state::agentref_t &a)
            {
                PIC_ASSERT(db_->writeable());

                load__();
                std::set<pi::state::agentref_t>::iterator i;

                for(i=agents_.begin(); i!=agents_.end(); i++)
                {
                    if((*i)->get_address()==a->get_address())
                    {
                        agents_.erase(i);
                        return true;
                    }
                }

                return false;
            }

            pi::state::agentref_t set_agent(const pi::state::agentref_t &a)
            {
                PIC_ASSERT(db_->writeable());

                load__();
                std::set<pi::state::agentref_t>::iterator i;

                for(i=agents_.begin(); i!=agents_.end(); i++)
                {
                    if((*i)->get_address()==a->get_address())
                    {
                        agents_.erase(i);
                        break;
                    }
                }

                agents_.insert(a);
                return a;
            }

            pi::state::agentref_t get_agent_index(unsigned n)
            {
                load__();
                PIC_ASSERT(n<=agents_.size());
                std::set<pi::state::agentref_t>::iterator i = agents_.begin();
                while(n>0) { i++; n--; }
                return (*i);
            }

            pi::state::agentref_t get_agent_address(unsigned type,const std::string &name,bool create)
            {
                load__();
                std::set<pi::state::agentref_t>::iterator i;

                for(i=agents_.begin(); i!=agents_.end(); i++)
                {
                    if((*i)->get_address()==name && (*i)->get_type()==type)
                    {
                        return (*i);
                    }
                }

                if(!create)
                    return pi::state::agentref_t();

                PIC_ASSERT(db_->writeable());
                pi::state::agentref_t a = xagent_t::create(db_,type,name);
                agents_.insert(a);
                return a;
            }

            void load__() { if(!loaded_) load2__(pos_); loaded_=true; }

            void load1__(unsigned long pos)
            {
                const unsigned char *buffer;
                unsigned size;
                unsigned short y;

                buffer = db_->get_file()->read_payload(pos,&size);

                PIC_ASSERT(size>=14);
                pie_getu64(&buffer[0],8,(uint64_t *)&timestamp_);
                pie_getu32(&buffer[8],4,&previous_);
                pie_getu16(&buffer[12],2,&y);
                PIC_ASSERT((size-14-y)%4==0);

                tag_=std::string((const char *)&buffer[14],y);
            }

            void load2__(unsigned long pos)
            {
                const unsigned char *buffer;
                unsigned size;
                unsigned x;
                uint32_t p;
                uint16_t y;

                buffer = db_->get_file()->read_payload(pos,&size);

                PIC_ASSERT(size>=14);
                pie_getu16(&buffer[12],2,&y);
                PIC_ASSERT((size-14-y)%4==0);

                buffer+=(14+y);
                x=(size-14-y)/4;

                std::set<unsigned long> a;
                std::set<unsigned long>::iterator ai;

                while(x>0)
                {
                    pie_getu32(buffer,4,&p);
                    a.insert(p);
                    buffer+=4;
                    x--;
                }

                pi::state::agentref_t agent;

                for(ai=a.begin();ai!=a.end();ai++)
                {
                    agent=xagent_t::create(db_,*ai);
                    agents_.insert(agent);
                }
            }

            unsigned long save__()
            {
                PIC_ASSERT(db_->writeable());

                std::set<unsigned long> a;
                std::set<unsigned long>::iterator ai;
                std::set<pi::state::agentref_t>::iterator i;

                for(i=agents_.begin(); i!=agents_.end(); i++)
                {
                    unsigned long cp = promote(*i)->save();

                    if(cp != 0)
                    {
                        a.insert(cp);
                    }
                }

                unsigned x = a.size();
                unsigned y = tag_.length();
                unsigned size = x*4+14+y;
                unsigned long pos;
                unsigned char *buffer = db_->get_file()->write_payload(size,&pos,true);

                pie_setu64(&buffer[0],8,timestamp_);
                pie_setu32(&buffer[8],4,previous_);
                pie_setu16(&buffer[12],2,y);

                memcpy(&buffer[14],tag_.c_str(),y);

                buffer+=(14+y);

                for(ai=a.begin();ai!=a.end();ai++)
                {
                    pie_setu32(buffer,4,*ai);
                    buffer+=4;
                }

                return pos;
            }

        private:
            pi::state::dbref_t db_;
            unsigned long pos_;
            bool loaded_;

            std::set<pi::state::agentref_t> agents_;
            unsigned long long timestamp_;
            uint32_t previous_;
            std::string tag_;
    };

    class xdatabase_t: public pi::state::database_t
    {
        private:
            xdatabase_t(const char *filename,bool writeable): file_(pi::state::open_file(filename,writeable))
            {
            }

        public:
            static pi::state::dbref_t create(const char *filename, bool writeable) { return pic::ref(new xdatabase_t(filename,writeable)); }

            pi::state::snapref_t get_trunk()
            {
                unsigned long trunk = file_->checkpoint();

                if(trunk==0)
                {
                    return xsnapshot_t::create(pi::state::dbref_t::from_lent(this));
                }
                else
                {
                    return xsnapshot_t::create(pi::state::dbref_t::from_lent(this),trunk);
                }
            }


            pi::state::snapref_t get_version(unsigned long pos)
            {
                return xsnapshot_t::create(pi::state::dbref_t::from_lent(this),pos);
            }

            bool writeable() { return file_->writeable(); }
            pi::state::fileref_t get_file() { return file_; }
            void close() { file_->close(); }
            void flush() { file_->flush(); }

        private:
            pi::state::fileref_t file_;
    };
};

pi::state::dbref_t pi::state::open_database(const char *filename, bool writeable)
{
    return xdatabase_t::create(filename,writeable);
}
