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

#include "clip_manager_impl.h"

namespace
{
    std::ostream &operator<<(std::ostream &o, const XmlElement &e)
    {
        String xml = e.createDocument("");
        o << xml;
        return o;
    }

    XmlElement *make_tag(const String &tag, const String &category)
    {
        XmlElement *e = new XmlElement("TAG");
        e->setAttribute("name",tag);
        e->setAttribute("category",category);
        return e;
    }

};

cdtr::clip_manager_t::impl_t::impl_t(const std::string &library_path): cdtr::conductor_library_t(library_path), scanner_(this)
{
    scanner_.run();
}

cdtr::clip_manager_t::impl_t::~impl_t()
{
    scanner_.shutdown();
}

bool cdtr::clip_manager_t::impl_t::rpc_change_tags_for_clip(void *context, const XmlElement *clipElement)
{
    pic::logmsg() << "change tags for clip";
    sqlite3* db=open_clipdb();
    if(db!=0)
    {
        if(clipElement->hasTagName("CLIP"))
        {
            const char* uuid=clipElement->getStringAttribute("uuid").toUTF8();

            XmlElement* e=clipElement->getChildByName("TAGS");
            pic::logmsg()<<"     database open";

            //   delete rows for this clip in cliptags
            preparedstatement_t delete_clip_tags(db, "DELETE FROM cliptags WHERE uuid = ? AND type = 0");
            if(delete_clip_tags.is_valid())
            {
                if(!delete_clip_tags.bind_text(1,uuid) ||
                !delete_clip_tags.step_done())
                {
                    pic::logmsg()<<"Error removing tags for clip "<<uuid;
                }
                pic::logmsg()<<"   clip tags deleted";
                delete_clip_tags.reset_and_clear();
            }

            XmlElement* t=e->getChildByName("TAG");

            //   insert new values

            preparedstatement_t insert_clip_tag(db, "INSERT INTO cliptags (uuid, key, value, type) VALUES (?, ?, ?, 0)");
            while(t!=0)
            {
                const char* key=t->getStringAttribute("category").toUTF8();
                const char* value=t->getStringAttribute("name").toUTF8(); 

                //preparedstatement_t insert_clip_tag(db, "INSERT INTO cliptags (uuid, key, value, type) VALUES (?, ?, ?, 0)");
                if(insert_clip_tag.is_valid())
                {

                    if( !insert_clip_tag.bind_text(1, uuid) || 
                       !insert_clip_tag.bind_text(2, key) || 
                       !insert_clip_tag.bind_text(3, value) || 
                       !insert_clip_tag.step_done())
                    {
                        pic::logmsg() << "Error inserting clip tag with key '" << key << "' and value '" << value << "'";
                    }
                    pic::logmsg()<<"   new clip tag added "<<uuid<<" "<<key<<" "<<value;
                    insert_clip_tag.reset_and_clear();
                }

                t=t->getNextElement();
            }
        }
    }

    complete_rpc(context);
    bump_sequence();
    return true;
}

bool cdtr::clip_manager_t::impl_t::rpc_get_column_categories(void *context, const XmlElement *e)
{
    pic::logmsg() << "get column categories";
    XmlElement columnCategories("CATEGORIES");

    sqlite3* db=open_clipdb();
    if(db!=0)
    {
        preparedstatement_t select_tag_categories(db, "SELECT DISTINCT key FROM tags");
        if(select_tag_categories.is_valid())
        {
            pic::logmsg()<<"clip_manager_impl:select_tag_categories";
            while(select_tag_categories.step_row())
            {
                XmlElement* c1=new XmlElement(String("CATEGORY"));
                const char* category=(const char*)select_tag_categories.column_text(0);
                pic::logmsg()<<"   category "<<category;
                c1->setAttribute("Name",String(category));
                columnCategories.addChildElement(c1);
            }

            select_tag_categories.reset_and_clear();
        }

        sqlite3_close(db);
    }

    complete_rpc(context,&columnCategories);
    return true;
}

bool cdtr::clip_manager_t::impl_t::rpc_remove_tag(void *context, const XmlElement *argument)
{
    std::cout<<"clip_manager_impl: rpc_remove_tag"<<argument->createDocument("")<<std::endl;
    if(argument->hasTagName("tag"))
    {
        const char* key=argument->getStringAttribute("category").toUTF8();
        const char* value=argument->getStringAttribute("name").toUTF8();

        sqlite3* db=open_clipdb();
        if(db!=0)
        {
            preparedstatement_t delete_tag(db, "DELETE FROM standardtags WHERE key=? AND value= ? ");
            if(delete_tag.is_valid())
            {

                  if( !delete_tag.bind_text(1, key) || 
                   !delete_tag.bind_text(2, value) || 
                   !delete_tag.step_done())
                {
                    pic::logmsg() << "Error deleting tag with key '" << key << "' and value '" << value << "'";
                }
                delete_tag.reset_and_clear();
            }

            sqlite3_close(db);
        }
    }
    complete_rpc(context);
    bump_sequence();
    return true;
}

bool cdtr::clip_manager_t::impl_t::rpc_add_category(void *context, const XmlElement *argument)
{
    std::cout<<"clip_manager_impl: add category"<<std::endl;

    const char* key="New Category";
    const char* value="New Tag";

    sqlite3* db=open_clipdb();
    if(db!=0)
    {
        preparedstatement_t insert_tag(db, "INSERT INTO standardtags (key, value) VALUES (?, ?)");
        if(insert_tag.is_valid())
        {

              if( !insert_tag.bind_text(1, key) || 
               !insert_tag.bind_text(2, value) || 
               !insert_tag.step_done())
            {
                pic::logmsg() << "Error inserting tag with key '" << key << "' and value '" << value << "'";
            }
            insert_tag.reset_and_clear();
        }

        sqlite3_close(db);
    }

    complete_rpc(context);
    bump_sequence();
    return true;
}

bool cdtr::clip_manager_t::impl_t::rpc_add_tag(void *context, const XmlElement *argument)
{
    std::cout<<"clip_manager_impl: rpc_add_tag"<<argument->createDocument("")<<std::endl;
    if(argument->hasTagName("tag"))
    {
        const char* key=argument->getStringAttribute("category").toUTF8();
        const char* value=argument->getStringAttribute("name").toUTF8();

        sqlite3* db=open_clipdb();
        if(db!=0)
        {
            preparedstatement_t insert_tag(db, "INSERT INTO standardtags (key, value) VALUES (?, ?)");
            if(insert_tag.is_valid())
            {

                  if( !insert_tag.bind_text(1, key) || 
                   !insert_tag.bind_text(2, value) || 
                   !insert_tag.step_done())
                {
                    pic::logmsg() << "Error inserting tag with key '" << key << "' and value '" << value << "'";
                }
                insert_tag.reset_and_clear();
            }

            sqlite3_close(db);
        }
    }
    complete_rpc(context);
    bump_sequence();
    return true;
}

bool cdtr::clip_manager_t::impl_t::rpc_change_tag(void *context, const XmlElement *argument)
{
    std::cout<<"clip_manager_impl: rpc_change_tag"<<argument->createDocument("")<<std::endl;
    if(argument->hasTagName("tag"))
    {
//        const char* uuid=argument->getStringAttribute("uuid").toUTF8();
        const char* key=argument->getStringAttribute("category").toUTF8();
        const char* value=argument->getStringAttribute("oldname").toUTF8();
        const char* newValue=argument->getStringAttribute("name").toUTF8();

        sqlite3* db=open_clipdb();
        if(db!=0)
        {
            preparedstatement_t insert_tag(db, "UPDATE standardtags SET value = ? WHERE key = ? AND value= ?" );
            if(insert_tag.is_valid())
            {

                  if( !insert_tag.bind_text(1, newValue) || 
                   !insert_tag.bind_text(2, key) || 
                   !insert_tag.bind_text(3, value) || 
                   !insert_tag.step_done())
                {
                    pic::logmsg() << "Error changing tag with key '" << key << "' and value '" << value << "'";
                }
                insert_tag.reset_and_clear();
            }

            sqlite3_close(db);
        }
    }

    complete_rpc(context);
    bump_sequence();
    return true;
}

//bool cdtr::clip_manager_t::impl_t::rpc_get_selected_clips(void *context, const XmlElement *argument)
//{
// XXX
// save the selected tags as a set or vector

// if no tags selected return all clips

// else

// for each selected tag pair
//    select distinct uuid from cliptags where key=selected_tag_key and value= selected tag value
//      add the uuid to a list of uuids if not already there

// for each uuid in the list
//     build the clip xml

//}

bool cdtr::clip_manager_t::impl_t::rpc_get_selected_clips(void *context, const XmlElement *argument)
{
    // XXX This is a bit of a horror - produces right result but needs rewriting 

    pic::logmsg() << "clip_manager_t:get selected clips";

    std::set<std::pair<String,String> >selected;
    selected.clear();

    if(argument->hasTagName("TAGS"))
    {
        XmlElement* t=argument->getChildByName("TAG");
        while(t!=0)
        {
            pic::logmsg()<<"   name of selected tag "<<t->getStringAttribute("Name");
            pic::logmsg()<<"   category of selected tag "<<t->getStringAttribute("Category");
            selected.insert(std::pair<String,String>(t->getStringAttribute("Category"),t->getStringAttribute("Name")));
            t=t->getNextElement();
        }
    }


    XmlElement clips=XmlElement("CLIPS");
    XmlElement selected_clips("CLIPS");
    sqlite3* db=open_clipdb();
    if(db!=0)
    {
        preparedstatement_t select_displayname(db, "SELECT value FROM metadata WHERE key='Display Name' AND uuid=?");
        preparedstatement_t get_clip(db, "SELECT uuid,key,value FROM cliptags ORDER BY uuid" );
        //preparedstatement_t get_clip(db, "SELECT metadata.uuid,metadata.value,cliptags.key,cliptags.value FROM cliptags,metadata WHERE (cliptags.uuid=metadata.uuid AND metadata.key='Display Name') ORDER BY cliptags.uuid" );

        if(get_clip.is_valid())
        {
            String currentUuid=String::empty;
            XmlElement* c=0;
            while(get_clip.step_row())
            {
                const char* uuid=(const char*)get_clip.column_text(0);
                const char* tagKey=(const char*)get_clip.column_text(1);
                const char* tagValue=(const char*)get_clip.column_text(2);
                std::cout<<"clip uuid="<<uuid<<" Tag (key,value)"<<tagKey<<","<<tagValue<<std::endl;

                if(!(String(uuid).equalsIgnoreCase(currentUuid))||currentUuid.isEmpty())
                {
                    c=new XmlElement("CLIP");
                    c->setAttribute("name",String(uuid));//default
                    c->setAttribute("uuid",String(uuid));

                    clips.addChildElement(c);
                    currentUuid=String(uuid);
                }

                c->addChildElement(make_tag(String(tagValue),String(tagKey)));

            }
            get_clip.reset_and_clear();
        }

        if(selected.empty())
        {
            std::cout<<"selected is empty"<<std::endl;
//      get the uuids that are in clip but not in cliptags 
//      to ensure clip does not disappear if user removes all tags
//      using metadata rather than clip to simulate
            preparedstatement_t get_untagged_clips(db,"SELECT DISTINCT uuid FROM metadata WHERE uuid NOT IN (SELECT uuid FROM cliptags)");
            if(get_untagged_clips.is_valid())
            {
                XmlElement* c=0;
                while(get_untagged_clips.step_row())
                {
                    const char* uuid=(const char*)get_untagged_clips.column_text(0);
                    std::cout<<"untagged clip uuid="<<uuid<<std::endl;

                    c=new XmlElement("CLIP");
                    c->setAttribute("name",String(uuid));//default
                    c->setAttribute("uuid",String(uuid));
                    clips.addChildElement(c);
                }
                get_untagged_clips.reset_and_clear();

            }
            else
            {
                std::cout<<"get_untagged_clips not valid"<<std::endl;
            }
        }

        XmlElement* e=clips.getFirstChildElement();

        while (e!=0)
        {
            if(e->hasTagName("CLIP"))
            {
                if(select_displayname.is_valid())
                {
                    if(!select_displayname.bind_text(1,e->getStringAttribute("uuid").toUTF8()))
                    {
                       pic::logmsg()<<"Error getting displayname from metadata"; 
                    }
                    else
                    {
                        while(select_displayname.step_row())
                        {
                            const char* clipName=(const char*)select_displayname.column_text(0);
                            e->setAttribute("name",String(clipName));
                        }
                    }

                    select_displayname.reset_and_clear();
                }

                std::set<std::pair<String,String> >clip_tags;
                XmlElement* t=e->getChildByName("TAG");

                while(t!=0)
                {
                    clip_tags.insert(std::pair<String,String>(t->getStringAttribute("category"),t->getStringAttribute("name")));
                    t=t->getNextElement();
                }

                bool missing=false;

                for(std::set<std::pair<String,String> >::iterator i=selected.begin();i!=selected.end();i++)
                {
                    std::set<std::pair<String,String> >::iterator j=clip_tags.find((*i));
                    if(j==clip_tags.end())
                    {
                        missing=true;
                    }
                }

                if (!missing)
                {
                    XmlElement* c = new XmlElement(*e);
                    selected_clips.addChildElement(c);
                }
            }

            e=e->getNextElement();
        }

        sqlite3_close(db);
    }

    complete_rpc(context,&selected_clips);
    return true;
}

bool cdtr::clip_manager_t::impl_t::rpc_get_all_tags(void *context, const XmlElement *e)
{
    pic::logmsg() << "get all tags";
    XmlElement tags=XmlElement("TAGS");
    // select all tags from db tags table 
    sqlite3* db=open_clipdb();
    if(db!=0)
    {

        preparedstatement_t select_all_tags(db, "SELECT key,value FROM tags");
        if(select_all_tags.is_valid())
        {
            pic::logmsg()<<"clip_manager_impl:select_all_tags";
            while(select_all_tags.step_row())
            {
                const char* category=(const char*)select_all_tags.column_text(0);
                const char* tagName=(const char*)select_all_tags.column_text(1);
                tags.addChildElement(make_tag(String(tagName),String(category)));
            }

            select_all_tags.reset_and_clear();
        }

        sqlite3_close(db);
    }


    complete_rpc(context,&tags);
    return true;
}

bool cdtr::clip_manager_t::impl_t::rpc_add_to_clip_pool(void *context, const XmlElement *e, int index)
{
    // XXX
    pic::logmsg() << "add to clip pool";
    complete_rpc(context);
    return true;
}

sqlite3 *cdtr::clip_manager_t::impl_t::open_clipdb()
{
    juce::File file = get_conductor_file(CONDUCTOR_CLIPS_DB);
    sqlite3 *db;
    if(SQLITE_OK != sql_check(sqlite3_open(file.getFullPathName().toUTF8(), &db), db, (juce::String("Can't open SQLite database '")+juce::String(file.getFullPathName())).toUTF8()))
    {
        sqlite3_close(db);
        return 0;
    }
    return db;
}

