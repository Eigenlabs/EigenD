
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

/*
 * midi_converter.cpp
 *
 */

#include <sstream>
#include <memory>

#include <picross/pic_weak.h>
#include <piw/piw_bundle.h>
#include <piw/piw_clock.h>
#include <piw/piw_thing.h>
#include <piw/piw_tsd.h>
#include <piw/piw_window.h>

#include <lib_midi/midi_converter.h>

#include <lib_juce/juce.h>

using namespace std;

namespace midi
{
    struct ConverterDialog;

    struct midi_converter_t::impl_t: params_delegate_t, mapping_observer_t, piw::clocksink_t, piw::thing_t, virtual pic::tracked_t
    {
        impl_t(mapping_observer_t &, midi_channel_delegate_t &, piw::clockdomain_ctl_t *, midi_from_belcanto_t &, const std::string &);
        ~impl_t() { invalidate(); }

        void resend_parameter_current(const piw::data_nb_t &d);

        void close();
        void invalidate();
        void window_state(bool state);
        const std::string get_title() { return title_; };

        void clocksink_ticked(unsigned long long, unsigned long long);

        void mapping_changed(const std::string &);
        void parameter_changed(unsigned);
        void settings_changed();
        std::string get_parameter_name(unsigned i);

        void set_mapping(const std::string &mapping);
        std::string get_mapping();
        void parameter_name_changed(unsigned);
        void map_param(unsigned, mapping_info_t);
        void map_midi(unsigned, mapping_info_t);
        void unmap_param(unsigned, unsigned short);
        void unmap_midi(unsigned, unsigned short);
        bool is_mapped_param(unsigned, unsigned short);
        bool is_mapped_midi(unsigned, unsigned short);
        mapping_info_t get_info_param(unsigned, unsigned short);
        mapping_info_t get_info_midi(unsigned, unsigned short);
        void clear_all();
        void change_settings(global_settings_t);
        void perform_settings_updates(global_settings_t);
        global_settings_t get_settings();
        void clear_params();
        void clear_midi_cc();
        void clear_midi_behaviour();
        void set_minimum_decimation(float);
        void set_midi_notes(bool);
        void set_midi_pitchbend(bool);
        void set_midi_hires_velocity(bool);

        void remove_upstream_clock(bct_clocksink_t *);
        void add_upstream_clock(bct_clocksink_t *);
        void update_origins(control_mapping_t &);
        void update_mapping(control_mapping_t &);
        void set_parameters(pic::lckvector_t<param_data_t>::nbtype &);
        void set_midi(pic::lckvector_t<midi_data_t>::nbtype &);

        std::auto_ptr<param_input_t> param_input_[32];

        midi_channel_delegate_t &channel_delegate_;
        settings_functors_t settings_functors_;

        piw::clockdomain_ctl_t *clockdomain_;

        mapping_observer_t &observer_;
        pic::f_int_t parameter_changed_midi_cc_;
        pic::f_int_t parameter_changed_midi_behaviour_;

        midi_from_belcanto_t &midi_from_belcanto_;

        const std::string title_;
        piw::window_t params_window_;
        ConverterDialog *params_;
        controllers_mapping_t mapping_;
        mapping_delegate_t mapping_delegate_;
    };

    /*
     * ConverterDialogComponent
     */

    struct ConverterDialogComponent: juce::Component, juce::ComponentListener
    {
        ConverterDialogComponent(midi_converter_t::impl_t *root, ConverterDialog *dialog):
            root_(root), dialog_(dialog),
            midi_cc_mapper_(midi::mapping_functors_t::init(
                    is_mapped_t::method(&root->mapping_,&controllers_mapping_t::is_mapped_midi),
                    get_info_t::method(&root->mapping_,&controllers_mapping_t::get_info_midi),
                    map_t::method(&root->mapping_,&controllers_mapping_t::map_midi),
                    unmap_t::method(&root->mapping_,&controllers_mapping_t::unmap_midi),
                    get_name_t::method(root,&midi_converter_t::impl_t::get_parameter_name),
                    clear_t::method(root,&midi_converter_t::impl_t::clear_midi_cc)
                    )),
            midi_behaviour_mapper_(midi::mapping_functors_t::init(
                    is_mapped_t::method(&root->mapping_,&controllers_mapping_t::is_mapped_midi),
                    get_info_t::method(&root->mapping_,&controllers_mapping_t::get_info_midi),
                    map_t::method(&root->mapping_,&controllers_mapping_t::map_midi),
                    unmap_t::method(&root->mapping_,&controllers_mapping_t::unmap_midi),
                    get_name_t::method(root,&midi_converter_t::impl_t::get_parameter_name),
                    clear_t::method(root,&midi_converter_t::impl_t::clear_midi_behaviour)
                    ))
        {
            mapper_midi_cc_table_t *ct = new mapper_midi_cc_table_t(root_->settings_functors_, midi_cc_mapper_);
            mapper_midi_behaviour_table_t *mt = new mapper_midi_behaviour_table_t(root_->settings_functors_, midi_behaviour_mapper_);

            ct->initialize();
            mt->initialize();

            root->parameter_changed_midi_cc_ = pic::f_int_t::method(ct,&mapper_midi_cc_table_t::column_changed);
            root->parameter_changed_midi_behaviour_ = pic::f_int_t::method(mt,&mapper_midi_behaviour_table_t::column_changed);

            addAndMakeVisible(toolbar_ = new toolbar_t(&root->mapping_delegate_));

            addAndMakeVisible(content_ = new juce::TabbedComponent(juce::TabbedButtonBar::TabsAtTop));
            content_->setTabBarDepth(30);
            content_->addTab(T("MIDI CC Messages"), juce::Colours::lightgrey, ct, true);
            content_->addTab(T("MIDI Behaviour"), juce::Colours::lightgrey, mt, true);
            content_->setCurrentTabIndex(0);
            content_->addComponentListener(this);

            componentMovedOrResized(*content_,true,true);

            setBounds(0,0,800,500);
        }

        ~ConverterDialogComponent()
        {
            deleteAllChildren();
        }

        void resized()
        {
            toolbar_->setBounds(0,0,getWidth(),32);
            content_->setBounds(0,32,getWidth(),getHeight()-32);
            content_->repaint();
        }

        void componentMovedOrResized(Component &c, bool moved, bool is_resized)
        {
            if(is_resized)
            {
                setBounds(0,0,c.getWidth(),c.getHeight()+32);
                resized();
            }
        }

        midi_converter_t::impl_t *root_;
        ConverterDialog *dialog_;
        midi::mapping_functors_t midi_cc_mapper_;
        midi::mapping_functors_t midi_behaviour_mapper_;
        juce::Toolbar *toolbar_;
        juce::TabbedComponent *content_;
    };

    /*
     * ConverterDialog
     */

    struct ConverterDialog: juce::DocumentWindow
    {
        ConverterDialog(midi_converter_t::impl_t *root):
            juce::DocumentWindow(root->get_title().c_str(),juce::Colours::black,juce::DocumentWindow::closeButton,true), root_(root), component_(0)
        {
            component_ = new ConverterDialogComponent(root_,this);

            setUsingNativeTitleBar(true);
            setContentOwned(component_,true);
            centreAroundComponent(component_,getWidth(),getHeight());
            setTopLeftPosition(150,150);
            setResizable(true,true);
        }

        ~ConverterDialog()
        {
            deleteAllChildren();
            clearContentComponent();
        }

        void closeButtonPressed()
        {
            root_->window_state(false);
            setVisible(false);
        }

        midi_converter_t::impl_t *root_;
        ConverterDialogComponent *component_;
    };

    /*
     * midi_converter_t::impl_t
     */

    midi_converter_t::impl_t::impl_t(mapping_observer_t &observer, midi_channel_delegate_t &channel_delegate,
        piw::clockdomain_ctl_t *d, midi_from_belcanto_t &midi_from_belcanto, const std::string &title):
        channel_delegate_(channel_delegate),
        settings_functors_(settings_functors_t::init(
                clearall_t::method(this,&midi_converter_t::impl_t::clear_all),
                get_settings_t::method(this,&midi_converter_t::impl_t::get_settings),
                change_settings_t::method(this,&midi_converter_t::impl_t::change_settings),
                set_channel_t::method(&channel_delegate_,&midi_channel_delegate_t::set_midi_channel),
                set_channel_t::method(&channel_delegate_,&midi_channel_delegate_t::set_min_channel),
                set_channel_t::method(&channel_delegate_,&midi_channel_delegate_t::set_max_channel),
                get_channel_t::method(&channel_delegate_,&midi_channel_delegate_t::get_midi_channel),
                get_channel_t::method(&channel_delegate_,&midi_channel_delegate_t::get_min_channel),
                get_channel_t::method(&channel_delegate_,&midi_channel_delegate_t::get_max_channel)
                )),
        clockdomain_(d), observer_(observer), midi_from_belcanto_(midi_from_belcanto),
        title_(title), params_(0), mapping_(*this),
        mapping_delegate_(settings_functors_)
    {
        d->sink(this,"midi converter");
        piw::tsd_thing(this);
        piw::tsd_window(&params_window_);

        for(unsigned i=0; i<15; i++)
        {
            param_input_[i] = std::auto_ptr<param_input_t>(new param_input_t(this,i+1));
        }
        param_input_[15] = std::auto_ptr<param_input_t>(new keynum_input_t(this,16));
        for(unsigned i=16; i<32; i++)
        {
            param_input_[i] = std::auto_ptr<param_input_t>(new param_input_t(this,i+1));
        }

        midi_from_belcanto_.set_resend_current(resend_current_t::method(this, &midi_converter_t::impl_t::resend_parameter_current));

        params_window_.set_state_handler(pic::status_t::method(this,&midi_converter_t::impl_t::window_state));
        params_window_.set_window_title(title.c_str());
        params_window_.set_window_state(false);

        tick_enable(false);
    }

    void midi_converter_t::impl_t::close()
    {
        if(params_)
        {
            params_window_.set_window_state(false);
            params_->setVisible(false);
            params_window_.close_window();
        }
    }

    void midi_converter_t::impl_t::invalidate()
    {
        tick_disable();
        close();
        tracked_invalidate();

        if(params_)
        {
            delete params_;
            params_=0;
        }
    }

    void midi_converter_t::impl_t::resend_parameter_current(const piw::data_nb_t &d)
    {
        for(unsigned i=0; i<32; ++i)
        {
            param_input_[i]->resend_current(d);
        }
    }

    void midi_converter_t::impl_t::window_state(bool state)
    {
        pic::logmsg() << "window state: " << state;

        if(state)
        {
            if(0==params_)
            {
                params_=new ConverterDialog(this);
            }

            params_window_.set_window_state(true);
            params_->setVisible(true);
        }
        else
        {
            params_window_.set_window_state(false);
            if(params_)
            {
                params_->setVisible(false);
            }
        }
    }
    
    void midi_converter_t::impl_t::clocksink_ticked(unsigned long long from, unsigned long long to)
    {
        // schedule the key input parameter before anything else
        param_input_[15]->schedule(from,to);
        for(unsigned i=0; i<15; ++i)
        {
            param_input_[i]->schedule(from,to);
        }
        for(unsigned i=16; i<32; ++i)
        {
            param_input_[i]->schedule(from,to);
        }
    }

    void midi_converter_t::impl_t::mapping_changed(const std::string &mapping)
    {
        observer_.mapping_changed(mapping);
    }

    void midi_converter_t::impl_t::parameter_changed(unsigned param)
    {
        observer_.parameter_changed(param);
        parameter_changed_midi_cc_(param);
        parameter_changed_midi_behaviour_(param);
    }

    void midi_converter_t::impl_t::settings_changed()
    {
        perform_settings_updates(get_settings());
        observer_.settings_changed();
        mapping_delegate_.settings_changed();
    }

    std::string midi_converter_t::impl_t::get_parameter_name(unsigned i)
    {
        return observer_.get_parameter_name(i);
    }

    void midi_converter_t::impl_t::set_mapping(const std::string &mapping)
    {
        mapping_.set_mapping(mapping);
    }

    std::string midi_converter_t::impl_t::get_mapping()
    {
        return mapping_.get_mapping();
    }

    void midi_converter_t::impl_t::map_param(unsigned iparam, mapping_info_t info)
    {
    }

    void midi_converter_t::impl_t::map_midi(unsigned iparam, mapping_info_t info)
    {
        mapping_.map_midi(iparam,info);
    }

    void midi_converter_t::impl_t::unmap_param(unsigned iparam, unsigned short oparam)
    {
    }

    void midi_converter_t::impl_t::unmap_midi(unsigned iparam, unsigned short oparam)
    {
        mapping_.unmap_midi(iparam,oparam);
    }

    bool midi_converter_t::impl_t::is_mapped_param(unsigned iparam, unsigned short oparam)
    {
        return false;
    }

    bool midi_converter_t::impl_t::is_mapped_midi(unsigned iparam, unsigned short oparam)
    {
        return mapping_.is_mapped_midi(iparam,oparam);
    }

    mapping_info_t midi_converter_t::impl_t::get_info_param(unsigned iparam, unsigned short oparam)
    {
        return mapping_info_t();
    }

    mapping_info_t midi_converter_t::impl_t::get_info_midi(unsigned iparam, unsigned short oparam)
    {
        return mapping_.get_info_midi(iparam,oparam);
    }

    void midi_converter_t::impl_t::clear_all()
    {
        clear_params();
        clear_midi_cc();
        clear_midi_behaviour();
    }

    void midi_converter_t::impl_t::change_settings(global_settings_t settings)
    {
        perform_settings_updates(settings);
        mapping_.change_settings(settings);
    }

    void midi_converter_t::impl_t::perform_settings_updates(global_settings_t settings)
    {
        midi_from_belcanto_.set_send_notes(settings.send_notes_);
        midi_from_belcanto_.set_send_pitchbend(settings.send_pitchbend_);
        midi_from_belcanto_.set_send_hires_velocity(settings.send_hires_velocity_);
        midi_from_belcanto_.set_control_interval(settings.minimum_decimation_);
    }

    global_settings_t midi_converter_t::impl_t::get_settings()
    {
        return mapping_.get_settings();
    }

    void midi_converter_t::impl_t::clear_params()
    {
    }

    void midi_converter_t::impl_t::clear_midi_cc()
    {
        mapping_.clear_midi_cc();
    }

    void midi_converter_t::impl_t::clear_midi_behaviour()
    {
        mapping_.clear_midi_behaviour();
    }

    void midi_converter_t::impl_t::set_minimum_decimation(float decimation)
    {
        global_settings_t settings = mapping_.get_settings();
        settings.minimum_decimation_= decimation;
        mapping_.change_settings(settings);
    }

    void midi_converter_t::impl_t::set_midi_notes(bool enabled)
    {
        global_settings_t settings = mapping_.get_settings();
        settings.send_notes_= enabled;
        mapping_.change_settings(settings);
    }

    void midi_converter_t::impl_t::set_midi_pitchbend(bool enabled)
    {
        global_settings_t settings = mapping_.get_settings();
        settings.send_pitchbend_= enabled;
        mapping_.change_settings(settings);
    }

    void midi_converter_t::impl_t::set_midi_hires_velocity(bool enabled)
    {
        global_settings_t settings = mapping_.get_settings();
        settings.send_hires_velocity_= enabled;
        mapping_.change_settings(settings);
    }

    void midi_converter_t::impl_t::remove_upstream_clock(bct_clocksink_t *c)
    {
        remove_upstream(c);
    }

    void midi_converter_t::impl_t::add_upstream_clock(bct_clocksink_t *c)
    {
        add_upstream(c);
    }

    void midi_converter_t::impl_t::update_origins(control_mapping_t &control_mapping)
    {
    }

    void midi_converter_t::impl_t::update_mapping(control_mapping_t &control_mapping)
    {
        mapping_.refresh_mappings(control_mapping);
    }

    void midi_converter_t::impl_t::set_parameters(pic::lckvector_t<param_data_t>::nbtype &params)
    {
    }

    void midi_converter_t::impl_t::set_midi(pic::lckvector_t<midi_data_t>::nbtype &midi)
    {
        midi_from_belcanto_.set_midi(midi);
    }

    /*
     * midi_converter_t
     */

    midi_converter_t::midi_converter_t(mapping_observer_t &observer, midi_channel_delegate_t &channel_delegate,
        piw::clockdomain_ctl_t *d, midi_from_belcanto_t &midi_from_belcanto, const std::string &title): impl_(new impl_t(observer, channel_delegate, d, midi_from_belcanto, title)) {}
    midi_converter_t::~midi_converter_t() { delete impl_; }

    piw::clockdomain_ctl_t* midi_converter_t::clock_domain()
    {
        return impl_->clockdomain_;
    }

    piw::cookie_t midi_converter_t::parameter_input(unsigned name)
    {
        return piw::cookie_t(impl_->param_input_[name-1].get());
    }

    void midi_converter_t::set_mapping(const std::string &mapping)
    {
        if(mapping!="") impl_->set_mapping(mapping);
    }

    std::string midi_converter_t::get_mapping()
    {
        return impl_->get_mapping();
    }

    void midi_converter_t::parameter_name_changed(unsigned iparam)
    {
        impl_->parameter_changed_midi_cc_(iparam);
        impl_->parameter_changed_midi_behaviour_(iparam);
    }

    void midi_converter_t::map_param(unsigned iparam, mapping_info_t info)
    {
        impl_->map_param(iparam,info);
    }

    void midi_converter_t::map_midi(unsigned iparam, mapping_info_t info)
    {
        impl_->map_midi(iparam,info);
    }

    void midi_converter_t::unmap_param(unsigned iparam, unsigned oparam)
    {
        impl_->unmap_param(iparam,oparam);
    }

    void midi_converter_t::unmap_midi(unsigned iparam, unsigned oparam)
    {
        impl_->unmap_midi(iparam,oparam);
    }

    bool midi_converter_t::is_mapped_param(unsigned iparam, unsigned oparam)
    {
        return impl_->is_mapped_param(iparam,oparam);
    }

    bool midi_converter_t::is_mapped_midi(unsigned iparam, unsigned oparam)
    {
        return impl_->is_mapped_midi(iparam,oparam);
    }

    mapping_info_t midi_converter_t::get_info_param(unsigned iparam, unsigned oparam)
    {
        return impl_->get_info_param(iparam,oparam);
    }

    mapping_info_t midi_converter_t::get_info_midi(unsigned iparam, unsigned oparam)
    {
        return impl_->get_info_midi(iparam,oparam);
    }

    void midi_converter_t::set_minimum_decimation(float decimation)
    {
        impl_->set_minimum_decimation(decimation);
    }

    void midi_converter_t::set_midi_notes(bool enabled)
    {
        impl_->set_midi_notes(enabled);
    }

    void midi_converter_t::set_midi_pitchbend(bool enabled)
    {
        impl_->set_midi_pitchbend(enabled);
    }

    void midi_converter_t::set_midi_hires_velocity(bool enabled)
    {
        impl_->set_midi_hires_velocity(enabled);
    }

    void midi_converter_t::set_midi_channel(unsigned ch)
    {
        impl_->midi_from_belcanto_.set_midi_channel(ch);
        impl_->settings_changed();
    }

    void midi_converter_t::set_min_midi_channel(unsigned ch)
    {
        impl_->midi_from_belcanto_.set_min_midi_channel(ch);
        impl_->settings_changed();
    }

    void midi_converter_t::set_max_midi_channel(unsigned ch)
    {
        impl_->midi_from_belcanto_.set_max_midi_channel(ch);
        impl_->settings_changed();
    }

    void midi_converter_t::set_program_change(unsigned c)
    {
        impl_->midi_from_belcanto_.set_program_change(c);
    }

    piw::change_nb_t midi_converter_t::change_program()
    {
        return impl_->midi_from_belcanto_.change_program();
    }

    void midi_converter_t::set_bank_change(unsigned c)
    {
        impl_->midi_from_belcanto_.set_bank_change(c);
    }

    piw::change_nb_t midi_converter_t::change_bank()
    {
        return impl_->midi_from_belcanto_.change_bank();
    }

    void midi_converter_t::set_cc(unsigned c, unsigned v)
    {
        impl_->midi_from_belcanto_.set_cc(c, v);
    }

    piw::change_nb_t midi_converter_t::change_cc()
    {
        return impl_->midi_from_belcanto_.change_cc();
    }

    void midi_converter_t::close()
    {
        impl_->close();
    }

    int midi_converter_t::gc_clear()
    {
        return impl_->params_window_.gc_clear();
    }

    int midi_converter_t::gc_traverse(void *a,void *b)
    {
        return impl_->params_window_.gc_traverse(a,b);
    }

} // namespace midi
