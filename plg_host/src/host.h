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

#ifndef __HOST__
#define __HOST__

#include <picross/pic_functor.h>
#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <pihost_exports.h>
#include <piw/piw_control_mapping.h>

#include <string>
#include <lib_juce/juce.h>
#include <memory>

namespace host
{
    class PIHOST_DECLSPEC_CLASS plugin_description_t
    {
        public:
            explicit plugin_description_t(const juce::PluginDescription &desc): desc_(desc) {}
            plugin_description_t(const plugin_description_t &other): desc_(other.desc_)   {}
            plugin_description_t() {}
            plugin_description_t(const std::string &s,const std::string &n)
            {
                desc_.fileOrIdentifier=s.c_str();
                desc_.name=n.c_str();
            }


            std::string name() const         { return desc_.name.toCString(); }
            std::string format() const       { return desc_.pluginFormatName.toCString(); }
            std::string manufacturer() const { return desc_.manufacturerName.toCString(); }
            std::string version() const      { return desc_.version.toCString(); }
            std::string id() const           { return desc_.fileOrIdentifier.toCString(); }
            std::string description() const  { return name()+" "+format()+" from "+manufacturer(); }

            bool from_xml(const std::string &xml)
            {
                juce::String s(xml.c_str());
                juce::XmlDocument doc(s);
                return desc_.loadFromXml(*doc.getDocumentElement());
            }

            std::string to_xml() const
            {
                std::auto_ptr<juce::XmlElement> element(desc_.createXml());
                juce::String doc = element->createDocument(juce::String::empty,true);
                return doc.toCString();
            }

            juce::PluginDescription desc_;
    };

    plugin_description_t create_plugin_description(const std::string &s)
    {
        plugin_description_t pd;
        pd.from_xml(s);
        return pd;
    }

    class PIHOST_DECLSPEC_CLASS plugin_list_t: public pic::nocopy_t
    {
        public:
            plugin_list_t(const std::string &plugins_cache, const pic::notify_t &complete);
            ~plugin_list_t();

            unsigned num_plugins();
            plugin_description_t get_plugin(unsigned index);
            int find_plugin(const plugin_description_t &);
            void start_scan(bool force);

            int gc_clear();
            int gc_traverse(void *,void *);

            class impl_t;
        private:
            impl_t *impl_;
    };

    class PIHOST_DECLSPEC_CLASS plugin_observer_t
    {
        public:
            virtual ~plugin_observer_t() {}
            virtual void description_changed(const std::string &) = 0;
            virtual void showing_changed(bool) = 0;
            virtual void bypassed_changed(bool) = 0;
            virtual void mapping_changed(const std::string &) = 0;
            virtual std::string get_parameter_name(unsigned) = 0;
    };

    class PIHOST_DECLSPEC_CLASS plugin_instance_t: public pic::nocopy_t
    {
        public:
            plugin_instance_t(plugin_observer_t *, piw::midi_channel_delegate_t *,
                piw::clockdomain_ctl_t *, const piw::cookie_t &audio_out, const piw::cookie_t &midi_out,
                const pic::status_t &window_state_changed);
            ~plugin_instance_t();

            piw::clockdomain_ctl_t *clock_domain();

            piw::cookie_t metronome_input();
            piw::cookie_t midi_from_belcanto();
            piw::cookie_t midi_aggregator();
            piw::cookie_t audio_input();

            void set_midi_channel(unsigned);
            void set_min_midi_channel(unsigned);
            void set_max_midi_channel(unsigned);
            void set_program_change(unsigned);
            piw::change_nb_t change_program();
            void set_bank_change(unsigned);
            piw::change_nb_t change_bank();
            void set_cc(unsigned, unsigned);
            piw::change_nb_t change_cc();

            piw::cookie_t parameter_input(unsigned);
            void set_mapping(const std::string &);
            std::string get_mapping();
            void parameter_name_changed(unsigned);
            void map_param(unsigned, piw::mapping_info_t);
            void map_midi(unsigned, piw::mapping_info_t);
            void unmap_param(unsigned, unsigned short);
            void unmap_midi(unsigned, unsigned short);
            bool is_mapped_param(unsigned, unsigned short);
            bool is_mapped_midi(unsigned, unsigned short);
            piw::mapping_info_t get_info_param(unsigned, unsigned short);
            piw::mapping_info_t get_info_midi(unsigned, unsigned short);
            void clear_params();
            void clear_midi_cc();
            void clear_midi_behaviour();
            void set_minimum_decimation(float);
            void set_midi_notes(bool);
            void set_midi_pitchbend(bool);

            bool open(const plugin_description_t &);
            void close();

            unsigned input_channel_count();
            unsigned output_channel_count();

            void set_showing(bool);
            bool is_showing();
            void set_title(const std::string &);

            void set_bypassed(bool);
            bool is_bypassed();

            plugin_description_t get_description();
            piw::data_t get_state();
            void set_state(const piw::data_t &);
            piw::data_t get_bounds();
            void set_bounds(const piw::data_t &);

            void set_idle_time(float tt);
            void disable_idling();

            int gc_clear();
            int gc_traverse(void *,void *);

            class impl_t;
        private:
            impl_t *impl_;
    };
}

#endif
