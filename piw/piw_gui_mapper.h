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

#ifndef __PIW_GUI_MAPPER__
#define __PIW_GUI_MAPPER__

#include "piw_exports.h"

#include <picross/pic_functor.h>
#include <picross/pic_weak.h>
#include <piw/piw_control_mapping.h>

#include <lib_juce/juce.h>

namespace piw
{
    class PIW_DECLSPEC_CLASS toolbar_delegate_t: public juce::ToolbarItemFactory, public juce::Button::Listener
    {
    };

    class PIW_DECLSPEC_CLASS toolbar_t: public juce::Toolbar
    {
        public:
            toolbar_t(toolbar_delegate_t *);

        private:
            toolbar_delegate_t *delegate_;
    };

    class PIW_DECLSPEC_CLASS toolbar_button_t: public juce::ToolbarButton
    {
        public:
            toolbar_button_t(int, const juce::String &);
            bool getToolbarItemSizes(int, bool, int &, int &, int &);
    };

    typedef pic::functor_t<void()> clearall_t;
    typedef pic::functor_t<global_settings_t()> get_settings_t;
    typedef pic::functor_t<void(global_settings_t)> change_settings_t;
    typedef pic::functor_t<void(unsigned)> set_channel_t;
    typedef pic::functor_t<unsigned()> get_channel_t;

    struct PIW_DECLSPEC_CLASS settings_functors_t
    {
        clearall_t clearall_;
        get_settings_t get_settings_;
        change_settings_t change_settings_;
        set_channel_t set_midi_channel_;
        set_channel_t set_min_channel_;
        set_channel_t set_max_channel_;
        get_channel_t get_midi_channel_;
        get_channel_t get_min_channel_;
        get_channel_t get_max_channel_;

        static settings_functors_t init(clearall_t c, get_settings_t g, change_settings_t s,
            set_channel_t sch, set_channel_t smi, set_channel_t sma,
            get_channel_t gch, get_channel_t gmi, get_channel_t gma)
        {
            const settings_functors_t functors = {c,g,s,sch,smi,sma,gch,gmi,gma};
            return functors;
        }
    };

    class PIW_DECLSPEC_CLASS mapping_delegate_t: public toolbar_delegate_t
    {
        public:
            mapping_delegate_t(settings_functors_t functors) : settings_functors_(functors), button_clearall_(0), button_settings_(0), button_help_(0), help_(0), global_settings_(0) { }

            void close();
            void getAllToolbarItemIds(juce::Array<int> &);
            void getDefaultItemSet(juce::Array<int> &);
            juce::ToolbarItemComponent *createItem(const int);
            void buttonClicked(juce::Button *);
            void change_settings(global_settings_t);
            piw::global_settings_t get_settings();
            void settings_changed();
            unsigned get_midi_channel();
            void set_midi_channel(unsigned);
            unsigned get_min_channel();
            void set_min_channel(unsigned);
            unsigned get_max_channel();
            void set_max_channel(unsigned);

        private:
            void clearall();
            void settings();
            void help();

            settings_functors_t settings_functors_;
            juce::ToolbarButton *button_clearall_;
            juce::ToolbarButton *button_settings_;
            juce::ToolbarButton *button_help_;
            juce::DocumentWindow *help_;
            juce::Component *global_settings_;
    };

    typedef pic::functor_t<bool(unsigned,unsigned)> is_mapped_t;
    typedef pic::functor_t<mapping_info_t(unsigned,unsigned)> get_info_t;
    typedef pic::functor_t<void(unsigned,mapping_info_t)> map_t;
    typedef pic::functor_t<void(unsigned,unsigned)> unmap_t;
    typedef pic::functor_t<std::string(unsigned)> get_name_t;
    typedef pic::functor_t<void()> clear_t;

    struct PIW_DECLSPEC_CLASS mapping_functors_t
    {
        is_mapped_t is_mapped_;
        get_info_t get_info_;
        map_t map_;
        unmap_t unmap_;
        get_name_t get_name_;
        clear_t clear_;

        static mapping_functors_t init(is_mapped_t is, get_info_t i, map_t m, unmap_t u, get_name_t g, clear_t c)
        {
            const mapping_functors_t functors = {is,i,m,u,g,c};
            return functors;
        }
    };

    class mapper_table_t;

    class PIW_DECLSPEC_CLASS mapper_yaxis_editor_t: public juce::Component
    {
        public:
            mapper_yaxis_editor_t(mapper_table_t &);
            virtual ~mapper_yaxis_editor_t() { deleteAllChildren(); }
            void setup(int, int);
            void mouseUp(const juce::MouseEvent &);
            void resized();

            static int calculate_width(mapper_table_t &, int);

        protected:
            mapper_table_t &mapper_;
            juce::Label *label_;
            int row_;
    };

    class PIW_DECLSPEC_CLASS modal_dialog_listener_t
    {
        public:
            virtual ~modal_dialog_listener_t() {};
            virtual void dialog_closed() = 0;
    };

    class PopupDialogWindow;

    class PIW_DECLSPEC_CLASS mapper_cell_editor_t: public modal_dialog_listener_t, public juce::Component
    {
        public:
            mapper_cell_editor_t(mapper_table_t &);
            virtual ~mapper_cell_editor_t() { deleteAllChildren(); }
            virtual void setup(int, int);
            void mouseEnter(const juce::MouseEvent &);
            void mouseExit(const juce::MouseEvent &);
            void mouseUp(const juce::MouseEvent &);
            void resized() { label_->setBoundsInset(juce::BorderSize<int>(1)); }
            void close_popup();
            void dialog_closed();

            global_settings_t get_settings();
            bool is_mapped();
            mapping_info_t get_info();
            void map(bool, float, float, float, float, bool, float, unsigned, unsigned, unsigned, int, unsigned);
            void unmap();

            void edit_control_scope(bool edit) { edit_control_scope_ = edit; };
            bool edit_control_scope() { return edit_control_scope_; }

            void edit_resolution(bool edit) { edit_resolution_ = edit; };
            bool edit_resolution() { return edit_resolution_; }

        protected:
            friend class mapper_table_t;
            friend class mapper_midi_cc_table_t;
            friend class mapper_midi_behaviour_table_t;

            void draw_text();

            mapper_table_t &mapper_;
            bool edit_control_scope_;
            bool edit_resolution_;
            juce::Label *label_;
            int iparam_;
            int oparam_;
            PopupDialogWindow *active_popup_;
            Component *cell_popup_;
    };

    class PIW_DECLSPEC_CLASS mapper_midi_cell_editor_t: public mapper_cell_editor_t
    {
        public:
            mapper_midi_cell_editor_t(mapper_table_t &t) : mapper_cell_editor_t(t) {};
            void setup(int, int);
    };

    class PIW_DECLSPEC_CLASS mapper_table_model_t: public juce::TableListBoxModel
    {
        public:
            mapper_table_model_t(mapper_table_t &mapper): mapper_(mapper) { };

            void paintCell(juce::Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) { };

            int getNumRows();

        protected:
            mapper_table_t &mapper_;
    };

    class PIW_DECLSPEC_CLASS header_table_model_t: public mapper_table_model_t
    {
        public:
            header_table_model_t(mapper_table_t &mapper): mapper_table_model_t(mapper), mapper_(mapper) { };
            juce::Component *refreshComponentForCell(int, int, bool, juce::Component *);
            void paintRowBackground(juce::Graphics&, int, int, int, bool);
            void listWasScrolled();
            void selectedRowsChanged(int);

            int getNumRows();

        private:
            mapper_table_t &mapper_;
    };

    class PIW_DECLSPEC_CLASS mapping_table_model_t: public mapper_table_model_t
    {
        public:
            mapping_table_model_t(mapper_table_t &mapper): mapper_table_model_t(mapper), mapper_(mapper) { };
            juce::Component *refreshComponentForCell(int, int, bool, juce::Component *);
            void paintRowBackground(juce::Graphics&, int, int, int, bool);
            void listWasScrolled();
            void selectedRowsChanged(int);

        private:
            mapper_table_t &mapper_;
    };

    class PIW_DECLSPEC_CLASS mapper_table_t: public juce::Component, public juce::Button::Listener, public virtual pic::tracked_t
    {
        public:
            mapper_table_t(piw::settings_functors_t, piw::mapping_functors_t);
            virtual ~mapper_table_t() { tracked_invalidate(); deleteAllChildren(); }
            virtual void initialize();

            virtual void paint(juce::Graphics& g);
            void resized();
            void column_changed(int);
            int calculate_width(int);

            void mouseWheelMove(const juce::MouseEvent &, float, float);

            void buttonClicked(juce::Button* button);

            virtual int getNumRows() { return 0; };
            virtual juce::String getRowName(int row) { std::stringstream oss; oss << row; return juce::String(oss.str().c_str()); };
            virtual mapper_cell_editor_t *create_cell_editor(int, int);
            virtual mapper_yaxis_editor_t *create_yaxis_editor() { return new mapper_yaxis_editor_t(*this); }
            virtual void paintRowBackground(juce::Graphics&, int, int, int, bool);

        protected:
            friend class mapper_cell_editor_t;
            friend class mapper_yaxis_editor_t;
            friend class mapper_table_model_t;
            friend class header_table_model_t;
            friend class mapping_table_model_t;
            class ClearTabButton;

            virtual const juce::String get_cell_editor_name(mapper_cell_editor_t &);
            virtual void default_mapping(mapper_cell_editor_t &) = 0;

            piw::settings_functors_t settings_functors_;
            piw::mapping_functors_t mapping_functors_;
            header_table_model_t header_table_model_;
            mapping_table_model_t mapping_table_model_;
            juce::TableListBox *table_header_;
            juce::TableListBox *table_mapping_;
            juce::TextButton *help_button_;
            ClearTabButton *clear_tab_;
            juce::Font font_;
            unsigned long long last_modal_dismissal_;
            bool initialized_;
    };

    class PIW_DECLSPEC_CLASS mapper_midi_cc_table_t: public mapper_table_t
    {
        public:
            mapper_midi_cc_table_t(piw::settings_functors_t settings, piw::mapping_functors_t mapping): mapper_table_t(settings, mapping) {};

            int getNumRows();
            juce::String getRowName(int);
            mapper_cell_editor_t *create_cell_editor(int, int);

        protected:
            const juce::String get_cell_editor_name(mapper_cell_editor_t &);
            void default_mapping(mapper_cell_editor_t &);
    };

    class PIW_DECLSPEC_CLASS mapper_midi_behaviour_table_t: public mapper_table_t
    {
        public:
            mapper_midi_behaviour_table_t(piw::settings_functors_t settings, piw::mapping_functors_t mapping): mapper_table_t(settings, mapping) {};

            int getNumRows();
            juce::String getRowName(int);
            mapper_cell_editor_t *create_cell_editor(int, int);

        protected:
            const juce::String get_cell_editor_name(mapper_cell_editor_t &);
            void default_mapping(mapper_cell_editor_t &);
    };

}; // namespace piw


#endif /* __PIW_GUI_MAPPER__ */
