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

#include "clip_scanner.h"

#include "clip_manager_impl.h"
#include "sqlite_utils.h"

#define CLIPDB_VERSION_KEY "clipdb_version"
#define CLIPDB_VERSION_VALUE "1.0"

cdtr::clip_scanner_t::clip_scanner_t(cdtr::clip_manager_t::impl_t *clip_manager): clip_manager_(clip_manager), shutdown_(false)
{
}

cdtr::clip_scanner_t::~clip_scanner_t()
{
    shutdown();
}

void cdtr::clip_scanner_t::shutdown()
{
    if(!isrunning()) return;

    shutdown_ = true;
    gate_.open();
    wait();
}

void cdtr::clip_scanner_t::thread_main()
{
    sqlite3 *db = prepare_clipdb();
    if(db)
    {
        while(!shutdown_)
        {
            scan(db);

            gate_.pass_and_shut_timed(10000000ULL);
        }

        sqlite3_close(db);
    }
}

void cdtr::clip_scanner_t::scan(sqlite3 *db)
{
    unsigned long long seen = piw::tsd_time();

    preparedstatement_t select_clip(db, "SELECT modified FROM clip WHERE uuid = ?");
    if(!select_clip.is_valid()) return;

    preparedstatement_t select_audio(db, "SELECT modified FROM audio WHERE uuid = ?");
    if(!select_audio.is_valid()) return;

    preparedstatement_t delete_clip(db, "DELETE FROM clip WHERE uuid = ?");
    if(!delete_clip.is_valid()) return;

    preparedstatement_t delete_audio(db, "DELETE FROM audio WHERE uuid = ?");
    if(!delete_audio.is_valid()) return;

    preparedstatement_t delete_meta(db, "DELETE FROM metadata WHERE uuid = ?");
    if(!delete_meta.is_valid()) return;

    preparedstatement_t delete_cliptags(db, "DELETE FROM cliptags WHERE uuid = ? AND type = 1");
    if(!delete_cliptags.is_valid()) return;

    preparedstatement_t insert_clip(db, "INSERT INTO clip (uuid, path, modified, seen, type) VALUES (?, ?, ?, ?, ?)");
    if(!insert_clip.is_valid()) return;

    preparedstatement_t insert_audio(db, "INSERT INTO audio (uuid, path, modified, seen, size, samplerate, bitdepth, channels) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    if(!insert_audio.is_valid()) return;

    preparedstatement_t insert_meta(db, "INSERT INTO metadata (uuid, key, value) VALUES (?, ?, ?)");
    if(!insert_meta.is_valid()) return;

    preparedstatement_t insert_cliptags(db, "INSERT INTO cliptags (uuid, key, value, type) VALUES (?, ?, ?, 1)");
    if(!insert_cliptags.is_valid()) return;

    preparedstatement_t update_seen_clip(db, "UPDATE clip SET seen = ? WHERE uuid = ?");
    if(!update_seen_clip.is_valid()) return;

    preparedstatement_t update_seen_audio(db, "UPDATE audio SET seen = ? WHERE uuid = ?");
    if(!update_seen_audio.is_valid()) return;

    preparedstatement_t purge_unseen_metadata(db, "DELETE FROM metadata WHERE uuid IN (SELECT uuid FROM clip WHERE seen != ?)");
    if(!purge_unseen_metadata.is_valid()) return;

    preparedstatement_t purge_unseen_audio(db, "DELETE FROM audio WHERE seen != ?");
    if(!purge_unseen_audio.is_valid()) return;

    preparedstatement_t purge_unseen_clip(db, "DELETE FROM clip WHERE seen != ?");
    if(!purge_unseen_clip.is_valid()) return;

    juce::File dir = clip_manager_->get_conductor_dir(CONDUCTOR_CLIPS);
    juce::DirectoryIterator iter_xml(dir, true, "*.xml", File::findFiles|File::ignoreHiddenFiles);
    while (iter_xml.next())
    {
        juce::File clip_xml(iter_xml.getFile());
        XmlElement *xml = XmlDocument::parse(clip_xml);
        if(xml && xml->hasTagName("clip"))
        {
            const char *uuid = xml->getStringAttribute("uuid").toUTF8();
            juce::String path_str = clip_xml.getFullPathName().substring(dir.getFullPathName().length()+1);
            const char *path = path_str.toUTF8();
            const char *type = xml->getStringAttribute("type").toUTF8();
            int64 modified = clip_xml.getLastModificationTime().toMilliseconds();

            bool store_clip = false;
            bool seen_clip = false;
            if(select_clip.bind_text(1, uuid))
            {
                if(select_clip.step_row())
                {
                    int64 old_modified = select_clip.column_int64(0);
                    store_clip = (old_modified != modified);
                    seen_clip = true;
                }
                else
                {
                    store_clip = true;
                }
            }
            select_clip.reset_and_clear();

            if(store_clip)
            {
                if(!delete_meta.bind_text(1, uuid) ||
                   !delete_meta.step_done())
                {
                    pic::logmsg() << "Error deleting metadata with UUID '" << uuid << "'";
                }
                delete_meta.reset_and_clear();

                if(!delete_cliptags.bind_text(1, uuid) ||
                   !delete_cliptags.step_done())
                {
                    pic::logmsg() << "Error deleting creation clip tags with UUID '" << uuid << "'";
                }
                delete_cliptags.reset_and_clear();

                if(!delete_clip.bind_text(1, uuid) ||
                   !delete_clip.step_done())
                {
                    pic::logmsg() << "Error deleting clip with UUID '" << uuid << "'";
                }
                delete_clip.reset_and_clear();

                if(!insert_clip.bind_text(1, uuid) || 
                   !insert_clip.bind_text(2, path) ||
                   !insert_clip.bind_int64(3, modified) || 
                   !insert_clip.bind_int64(4, seen) || 
                   !insert_clip.bind_text(5, type) || 
                   !insert_clip.step_done())
                {
                    pic::logmsg() << "Error inserting clip with UUID '" << uuid << "'";
                }
                insert_clip.reset_and_clear();

                XmlElement *meta = xml->getChildByName("meta");
                if(meta)
                {
                    forEachXmlChildElementWithTagName(*meta, child, "entry")
                    {
                        const char *key = child->getStringAttribute("key").toUTF8();
                        const char *value = child->getAllSubText().toUTF8();

                        if(!insert_meta.bind_text(1, uuid) || 
                           !insert_meta.bind_text(2, key) || 
                           !insert_meta.bind_text(3, value) || 
                           !insert_meta.step_done())
                        {
                            pic::logmsg() << "Error inserting meta for clip with UUID '" << uuid << "' with key '" << key << "' and value '" << value << "'";
                        }
                        insert_meta.reset_and_clear();
                    }
                }

                XmlElement *labels = xml->getChildByName("labels");
                if(labels)
                {
                    forEachXmlChildElementWithTagName(*labels, child, "label")
                    {
                        const char *key = child->getStringAttribute("category").toUTF8();
                        const char *value = child->getAllSubText().toUTF8();

                        if(!insert_cliptags.bind_text(1, uuid) || 
                           !insert_cliptags.bind_text(2, key) || 
                           !insert_cliptags.bind_text(3, value) || 
                           !insert_cliptags.step_done())
                        {
                            pic::logmsg() << "Error inserting creation tags for clip with UUID '" << uuid << "' with key '" << key << "' and value '" << value << "'";
                        }
                        insert_cliptags.reset_and_clear();
                    }
                }
            }
            else if(seen_clip)
            {
                if(!update_seen_clip.bind_int64(1, seen) || 
                   !update_seen_clip.bind_text(2, uuid) || 
                   !update_seen_clip.step_done())
                {
                    pic::logmsg() << "Error updating seen clip with UUID '" << uuid << "'";
                }
                update_seen_clip.reset_and_clear();
            }
        }
        else
        {
            pic::logmsg() << "Error parsing XML for clip '" << clip_xml.getFullPathName() << "'";
        }
    }

    juce::AiffAudioFormat aiff_format;
    juce::DirectoryIterator iter_audio(dir, true, "*.aiff", File::findFiles|File::ignoreHiddenFiles);
    while (iter_audio.next())
    {
        juce::File clip_audio(iter_audio.getFile());
        juce::FileInputStream *input = new juce::FileInputStream(clip_audio);
        juce::AudioFormatReader *reader = aiff_format.createReaderFor(input, true);
        if(reader)
        {
            juce::String uuid_str = reader->metadataValues.getValue("CueNote1Text", "");
            if(uuid_str.isEmpty())
            {
                pic::logmsg() << "Can't determine UUID for AIFF audio file '" << clip_audio.getFullPathName() << "'";
            }
            else
            {
                const char *uuid = uuid_str.toUTF8();
                juce::String path_str = clip_audio.getFullPathName().substring(dir.getFullPathName().length()+1);
                const char *path = path_str.toUTF8();
                int64 modified = clip_audio.getLastModificationTime().toMilliseconds();

                bool store_audio = false;
                bool seen_audio = false;
                if(select_audio.bind_text(1, uuid))
                {
                    if(select_audio.step_row())
                    {
                        int64 old_modified = select_audio.column_int64(0);
                        store_audio = (old_modified != modified);
                        seen_audio = true;
                    }
                    else
                    {
                        store_audio = true;
                    }
                }
                select_audio.reset_and_clear();

                if(store_audio)
                {
                    if(!delete_audio.bind_text(1, uuid) ||
                       !delete_audio.step_done())
                    {
                        pic::logmsg() << "Error deleting audio with UUID '" << uuid << "'";
                    }
                    delete_audio.reset_and_clear();

                    if(!insert_audio.bind_text(1, uuid) || 
                       !insert_audio.bind_text(2, path) ||
                       !insert_audio.bind_int64(3, modified) || 
                       !insert_audio.bind_int64(4, seen) || 
                       !insert_audio.bind_int64(5, reader->lengthInSamples) || 
                       !insert_audio.bind_int64(6, reader->sampleRate) || 
                       !insert_audio.bind_int64(7, reader->bitsPerSample) || 
                       !insert_audio.bind_int64(8, reader->numChannels) || 
                       !insert_audio.step_done())
                    {
                        pic::logmsg() << "Error inserting audio with UUID '" << uuid << "'";
                    }
                    insert_audio.reset_and_clear();
                }
                else if(seen_audio)
                {
                    if(!update_seen_audio.bind_int64(1, seen) || 
                       !update_seen_audio.bind_text(2, uuid) || 
                       !update_seen_audio.step_done())
                    {
                        pic::logmsg() << "Error updating seen audio with UUID '" << uuid << "'";
                    }
                    update_seen_audio.reset_and_clear();
                }
            }

            delete reader;
        }
        else
        {
            pic::logmsg() << "Error opening AIFF audio file for clip '" << clip_audio.getFullPathName() << "'";
        }
    }

    if(!purge_unseen_metadata.bind_int64(1, seen) ||
       !purge_unseen_metadata.step_done())
    {
        pic::logmsg() << "Error purging unseen metadata for seen '" << seen << "'";
    }

    if(!purge_unseen_audio.bind_int64(1, seen) ||
       !purge_unseen_audio.step_done())
    {
        pic::logmsg() << "Error purging unseen audio for seen '" << seen << "'";
    }

    if(!purge_unseen_clip.bind_int64(1, seen) ||
       !purge_unseen_clip.step_done())
    {
        pic::logmsg() << "Error purging unseen clip for seen '" << seen << "'";
    }
}

sqlite3 *cdtr::clip_scanner_t::prepare_clipdb()
{
    juce::File file = clip_manager_->get_conductor_file(CONDUCTOR_CLIPS_DB);
    sqlite3 *db;
    if(SQLITE_OK != sql_check(sqlite3_open(file.getFullPathName().toUTF8(), &db), db, (juce::String("Can't open SQLite database '")+juce::String(file.getFullPathName())).toUTF8()))
    {
        sqlite3_close(db);
        return 0;
    }

    // global properties table creation
    if(!sql_exec(db, "CREATE TABLE IF NOT EXISTS global (key TEXT PRIMARY KEY NOT NULL, value TEXT NOT NULL)"))
    {
        sqlite3_close(db);
        return 0;
    }

    // ensure that the clip database structure has the right version
    bool version_ok = false;
    preparedstatement_t select_global_value(db, "SELECT value FROM global WHERE key = ?");
    if(select_global_value.is_valid())
    {
        if(select_global_value.bind_text(1, CLIPDB_VERSION_KEY) &&
           select_global_value.step_row())
        {
            const unsigned char *version = select_global_value.column_text(0);
            if(0 == strcmp((const char *)version, CLIPDB_VERSION_VALUE))
            {
                pic::logmsg() << "Found clip manager database '" + file.getFullPathName() + "' with version " << version;
                version_ok = true;
            }
            else
            {
                pic::logmsg() << "Unsupported manager database '" + file.getFullPathName() + "' with version " << version;
                sqlite3_close(db);
                return 0;
            }
        }
    }

    if(!version_ok)
    {
        preparedstatement_t insert_global_value(db, "INSERT INTO global (key, value) VALUES (?, ?)");
        if(insert_global_value.is_valid())
        {
            if(insert_global_value.bind_text(1, CLIPDB_VERSION_KEY) &&
               insert_global_value.bind_text(2, CLIPDB_VERSION_VALUE) &&
               insert_global_value.step_done())
            {
                pic::logmsg() << "Initialized clip manager database '" + file.getFullPathName() + "' with version " << CLIPDB_VERSION_VALUE;
                version_ok = true;
            }
        }
    }
    
    if(!version_ok)
    {
        pic::logmsg() << "Couldn't ensure correct version of clip database: " << file.getFullPathName();
        sqlite3_close(db);
        return 0;
    }

    // clip manager tables creation
    std::vector<std::string> sqls;
    sqls.push_back("CREATE TABLE IF NOT EXISTS clip ("
        "uuid TEXT PRIMARY KEY,"
        "path TEXT NOT NULL,"
        "modified NUMERIC NOT NULL,"
        "seen NUMERIC NOT NULL,"
        "type TEXT NOT NULL)");
    sqls.push_back("CREATE TABLE IF NOT EXISTS audio ("
        "uuid TEXT PRIMARY KEY,"
        "path TEXT NOT NULL,"
        "modified NUMERIC NOT NULL,"
        "seen NUMERIC NOT NULL,"
        "size NUMERIC NOT NULL,"
        "samplerate INT NOT NULL,"
        "bitdepth INT NOT NULL,"
        "channels INT NOT NULL)");
    sqls.push_back("CREATE TABLE IF NOT EXISTS metadata ("
        "uuid TEXT NOT NULL,"
        "key TEXT NOT NULL,"
        "value TEXT NOT NULL,"
        "PRIMARY KEY(uuid, key))");
    sqls.push_back("CREATE INDEX IF NOT EXISTS clip_seen_idx ON clip (seen)");
    sqls.push_back("CREATE INDEX IF NOT EXISTS audio_seen_idx ON audio (seen)");
    sqls.push_back("CREATE INDEX IF NOT EXISTS metadata_uuid_idx ON metadata (uuid)");
    sqls.push_back("CREATE TABLE IF NOT EXISTS standardtags ("
        "key TEXT NOT NULL,"
        "value TEXT NOT NULL,"
        "PRIMARY KEY(key,value))");
    sqls.push_back("CREATE TABLE IF NOT EXISTS cliptags ("
        "uuid TEXT NOT NULL,"
        "key TEXT NOT NULL,"
        "value TEXT NOT NULL,"
        "type NUMERIC NOT NULL,"
        "PRIMARY KEY(uuid,key))");
    sqls.push_back("CREATE VIEW IF NOT EXISTS tags AS "
        "SELECT DISTINCT key, value FROM standardtags UNION "
        "SELECT DISTINCT key, value FROM cliptags WHERE type = 1;");

    // test data
    sqls.push_back("INSERT OR IGNORE INTO standardtags (key,value) VALUES ('Type','Audio')");
    sqls.push_back("INSERT OR IGNORE INTO standardtags (key,value) VALUES ('Type','Instrument')");
    sqls.push_back("INSERT OR IGNORE INTO standardtags (key,value) VALUES ('Type','Talker')");
    sqls.push_back("INSERT OR IGNORE INTO standardtags (key,value) VALUES ('Type','Scene')");
    sqls.push_back("INSERT OR IGNORE INTO standardtags (key,value) VALUES ('Type','Arrangement')");
    sqls.push_back("INSERT OR IGNORE INTO standardtags (key,value) VALUES ('Style','Ambient')");
    sqls.push_back("INSERT OR IGNORE INTO standardtags (key,value) VALUES ('Style','Blues')");
    sqls.push_back("INSERT OR IGNORE INTO standardtags (key,value) VALUES ('Instrument','Bass')");
    sqls.push_back("INSERT OR IGNORE INTO standardtags (key,value) VALUES ('Instrument','Guitar')");

    sqls.push_back("INSERT OR IGNORE INTO cliptags (uuid,key,value,type) VALUES ('1','Type','Audio',0)");
    sqls.push_back("INSERT OR IGNORE INTO cliptags (uuid,key,value,type) VALUES ('1','Style','Ambient',0)");
    sqls.push_back("INSERT OR IGNORE INTO cliptags (uuid,key,value,type) VALUES ('2','Type','Audio',0)");
    sqls.push_back("INSERT OR IGNORE INTO cliptags (uuid,key,value,type) VALUES ('2','Style','Hip Hop',0)");
    sqls.push_back("INSERT OR IGNORE INTO cliptags (uuid,key,value,type) VALUES ('3','Type','Talker',0)");
    sqls.push_back("INSERT OR IGNORE INTO cliptags (uuid,key,value,type) VALUES ('4','Type','Scene',0)");

    sqls.push_back("INSERT OR IGNORE INTO metadata(uuid,key,value) VALUES (1,'Display Name','Clip 1')");
    sqls.push_back("INSERT OR IGNORE INTO metadata(uuid,key,value) VALUES (2,'Display Name','Clip 2')");
    sqls.push_back("INSERT OR IGNORE INTO metadata(uuid,key,value) VALUES (3,'Display Name','Talker Clip 1')");
    sqls.push_back("INSERT OR IGNORE INTO metadata(uuid,key,value) VALUES (4,'Display Name','Scene Clip 1')");

    std::vector<std::string>::iterator it;
    for(it = sqls.begin(); it != sqls.end(); ++it)
    {
        if(!sql_exec(db, it->c_str()))
        {
            sqlite3_close(db);
            return 0;
        }
    }

    return db;
}
