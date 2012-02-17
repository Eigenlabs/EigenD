
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

#include <piagent/pia_scaffold.h>
#include <piw/piw_state.h>

namespace eigend
{
    struct p2c_t
    {
        virtual ~p2c_t() { }
        virtual void load_started(const char *) = 0;
        virtual void load_status(const char *,unsigned) = 0;
        virtual void load_ended() = 0;
        virtual void alert_dialog(const char *klass, const char *label, const char *text) = 0;
        virtual void info_dialog(const char *klass, const char *label, const char *text) = 0;
        virtual pic::f_string_t make_logger(const char *prefix) = 0;
        virtual void setups_changed(const char *) = 0;
        virtual void set_latest_release(const char *) = 0;
    };

    struct c2p_t
    {
        c2p_t() { }
        virtual ~c2p_t() { }
        virtual void initialise(p2c_t *p2c, pia::scaffold_gui_t *, const std::string &, const std::string &) = 0;
        virtual void set_args(const char *) = 0;
        virtual piw::term_t get_setups() = 0;
        virtual piw::term_t get_user_setups() = 0;
        virtual std::string get_setup_slot(const char *) = 0;
        virtual void load_setup(const char *setup,bool user,bool upgrade) = 0;
        virtual void delete_setup(const char *) = 0;
        virtual void set_current_setup(const char *setup,bool user) = 0;
        virtual bool save_current_setup() = 0;
        virtual std::string save_setup(const char *slot, const char *tag,const char *desc,const bool make_default) = 0;
        virtual std::string edit_setup(const char *orig, const char *slot, const char *tag,const char *desc) = 0;
        virtual std::string get_description(const char *file) = 0;
        virtual std::string get_logfile() = 0;
        virtual std::string get_email() = 0;
        virtual std::string get_username() = 0;
        virtual std::string get_subject() = 0;
        virtual std::string get_default_setup(bool force) = 0;
        virtual void set_default_setup(const char *) = 0;
        virtual std::string notes_to_words(const std::string &) = 0;
        virtual std::string words_to_notes(const std::string &) = 0;
        virtual void file_bug(const std::string &u, const std::string &e, const std::string &s, const std::string &d) = 0;
        virtual void upgrade_setups() = 0;
        virtual void quit() = 0;
        virtual void prepare_quit() = 0;
    };

}
