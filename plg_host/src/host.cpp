
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

#include "host.h"

#include <iomanip>
#include <sstream>

#include <picross/pic_time.h>
#include <picross/pic_power.h>
#include <piw/piw_tsd.h>
#include <piw/piw_window.h>
#include <piw/piw_clockclient.h>
#include <piw/piw_thing.h>
#include <piw/piw_sclone.h>
#include <piw/piw_state.h>
#include <piw/piw_aggregator.h>
#include <piw/piw_midi_from_belcanto.h>
#include <piw/piw_gui_mapper.h>
#include <piw/piw_control_params.h>

#include "juce.h"

struct host::plugin_list_t::impl_t: piw::thing_t
{
    impl_t(const std::string &plugins_cache, const pic::notify_t &complete): complete_(complete), plugins_cache_(plugins_cache.c_str())
    {
        juce::AudioPluginFormatManager::getInstance()->addDefaultFormats();
        load();
        piw::tsd_thing(this);
        timer_slow(5000);
    }

    void load()
    {
        if(plugins_cache_.existsAsFile() && plugins_cache_.getSize()>0)
        {
            juce::XmlDocument xml(plugins_cache_);
            list_.clear();
            list_.recreateFromXml(*xml.getDocumentElement());
            mod_time_ = plugins_cache_.getLastModificationTime();
        }
        else
        {
            mod_time_ = juce::Time(0);
        }
    }

    ~impl_t()
    {
        tracked_invalidate();
        close_thing();
    }

    int findType(const std::string &format, const std::string &manu, const std::string &name)
    {
        juce::String jformat = juce::String::fromUTF8(format.c_str());
        juce::String jmanu = juce::String::fromUTF8(manu.c_str());
        juce::String jname = juce::String::fromUTF8(name.c_str());

        for(int i=0;i<list_.getNumTypes();i++)
        {
            PluginDescription *d = list_.getType(i);

            if(jformat != d->pluginFormatName) continue;
            if(jmanu != d->manufacturerName) continue;
            if(jname != d->name) continue;

            pic::logmsg() << "match " << format << ":" << manu << ":" << name << " -> " 
                          << d->pluginFormatName << ":" << d->manufacturerName << ':' << d->name;

            return i;
        }

        pic::logmsg() << "couldn't find " << format << ":" << manu << ":" << name;
        return -1;
    }

    void thing_timer_slow()
    {
        if(plugins_cache_.exists())
        {
            if(plugins_cache_.getLastModificationTime() > mod_time_)
            {
                load();
                complete_();
            }
        }
    }

    juce::KnownPluginList list_;
    pic::notify_t complete_;
    juce::File plugins_cache_;
    juce::Time mod_time_;
};

host::plugin_list_t::plugin_list_t(const std::string &plugins_cache, const pic::notify_t &complete):
    impl_(new impl_t(plugins_cache,complete))
{
}

host::plugin_list_t::~plugin_list_t()
{
    delete impl_;
}

unsigned host::plugin_list_t::num_plugins()
{ 
    return impl_->list_.getNumTypes(); 
}

int host::plugin_list_t::find_plugin(const host::plugin_description_t &desc)
{
    return impl_->findType(desc.format(),desc.manufacturer(),desc.name());
}

host::plugin_description_t host::plugin_list_t::get_plugin(unsigned index)
{
    return plugin_description_t(*impl_->list_.getType(index));
}

namespace
{
    unsigned timestamp_to_sample_offset(unsigned bs,unsigned long long time, unsigned long long from, unsigned long long to)
    {
        unsigned long long et = std::max(time,from+1);
        et = std::min(et,to);
        unsigned long long offset = (bs*(et-from))/(to-from);
        return std::min((bs-1),(unsigned)offset);
    }

    struct midi_input_t: piw::input_root_t
    {
        midi_input_t(host::plugin_instance_t::impl_t *);
        bool schedule(unsigned long long,unsigned long long);
        host::plugin_instance_t::impl_t *controller_;
    };

    struct host_scalar_t: piw::root_t, piw::wire_t, piw::event_data_sink_t
    {
        host_scalar_t(host::plugin_instance_t::impl_t *);
        ~host_scalar_t() { invalidate(); };
        void invalidate();

        virtual void root_opened() { root_clock(); root_latency(); }
        virtual void root_closed() {}
        virtual void root_latency() {}
        virtual void root_clock();

        virtual piw::wire_t *root_wire(const piw::event_data_source_t &es) { subscribe_and_ping(es); return this; }
        void wire_closed() { invalidate();}

        void event_start(unsigned seq, const piw::data_nb_t &id, const piw::xevent_data_buffer_t &b)
        {
            iterator_ = b.iterator();
            iterator_->reset_all(id.time());
        }

        bool event_end(unsigned long long t)
        {
            iterator_.clear();
            return true;
        }

        void event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n)
        {
            iterator_->set_signal(sig,n);
            iterator_->reset(sig,t);
        }

        host::plugin_instance_t::impl_t *controller_;
        bct_clocksink_t *clk_;
        piw::xevent_data_buffer_t::iter_t iterator_;
    };

    struct host_output_scalar_t: piw::root_ctl_t, piw::wire_ctl_t, piw::event_data_source_real_t, virtual pic::lckobject_t
    {
        host_output_scalar_t(): event_data_source_real_t(piw::pathnull(0))
        {
        }

        void setup(unsigned num_channels, unsigned queue_size)
        {
            unsigned long long mask = (num_channels<64)?((1ULL<<num_channels)-1ULL):(-1ULL);
            buffer_ = piw::xevent_data_buffer_t(mask,queue_size);
        }

        void scalar_connect(const piw::cookie_t &c)
        {
            connect(c);
            connect_wire(this,source());
        }

        void scalar_disconnect()
        {
            wire_ctl_t::disconnect();
            root_ctl_t::disconnect();
        }

        piw::xevent_data_buffer_t buffer_;
    };

    struct host_param_tabs_t: juce::Component, virtual pic::tracked_t
    {
        host_param_tabs_t(host::plugin_instance_t::impl_t *);
        ~host_param_tabs_t() { tracked_invalidate(); deleteAllChildren(); }
        void resized() { tabs_->setBoundsInset(juce::BorderSize<int>(8)); }

        juce::TabbedComponent *tabs_;
        piw::mapping_functors_t au_mapper_;
        piw::mapping_functors_t midi_cc_mapper_;
        piw::mapping_functors_t midi_behaviour_mapper_;
    };

    struct host_param_table_t: piw::mapper_table_t
    {
        host_param_table_t(piw::settings_functors_t settings, piw::mapping_functors_t mapping, host::plugin_instance_t::impl_t *c): mapper_table_t(settings, mapping), controller_(c), parameter_count_(0), last_parameter_check_(0) {};
        int getNumRows();
        juce::String getRowName(int);
        void default_mapping(piw::mapper_cell_editor_t &);
        int get_parameter_count();

        host::plugin_instance_t::impl_t *controller_;
        int parameter_count_;
        unsigned long long last_parameter_check_;
    };

    struct host_dialog_t: juce::DocumentWindow
    {
        host_dialog_t(const juce::String &title, juce::Component *c):
            DocumentWindow(title,juce::Colours::black,juce::DocumentWindow::closeButton,true)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(c,true);
            centreAroundComponent(c,getWidth(),getHeight());
            setVisible(true);
            setTopLeftPosition(150,150);
        }

        void closeButtonPressed()
        {
            setVisible(false);
        }
    };


    struct mapping_dialog_t: host_dialog_t
    {
        mapping_dialog_t(host::plugin_instance_t::impl_t *controller);
    };

    // panel holding content and toolbar components
    struct host_panel_t: juce::Component, juce::ComponentListener
    {
        host_panel_t(Component *content, host::plugin_instance_t::impl_t *controller, piw::toolbar_delegate_t *delegate):
            toolbar_(new piw::toolbar_t(delegate)), controller_(controller), content_(content)
        {
            addAndMakeVisible(toolbar_);

            content_->addComponentListener(this);
            addAndMakeVisible(content_);

            componentMovedOrResized(*content_,true,true);
        }

        ~host_panel_t()
        {
            if(content_)
            {
                removeChildComponent(content_);
                content_->removeComponentListener(this);
                content_->setVisible(false);

                delete content_;
                content_ = 0;
            }

            if(toolbar_)
            {
                removeChildComponent(toolbar_);
                toolbar_->setVisible(false);

                delete toolbar_;
                toolbar_ = 0;
            }
        }

        void resized();

        void componentMovedOrResized(Component &c, bool moved, bool resized);

        juce::Toolbar *toolbar_;
        host::plugin_instance_t::impl_t *controller_;
        juce::Component *content_;
    };

    struct mainpanel_delegate_t: piw::toolbar_delegate_t
    {
        mainpanel_delegate_t(host::plugin_instance_t::impl_t *c): controller_(c), mapping_dialog_(0)
        {
        }

        void close()
        {
            if(mapping_dialog_)
            {
                delete mapping_dialog_;
                mapping_dialog_=0;
            }
        }

        enum item_id
        {
            id_automation = 4
        };

        void getAllToolbarItemIds(juce::Array<int> &ids)
        {
            ids.add(id_automation);
        }

        void getDefaultItemSet(juce::Array<int> &ids)
        {
            getAllToolbarItemIds(ids);
        }

        juce::ToolbarItemComponent *createItem(const int id)
        {
            switch(id)
            {
                case id_automation:
                    return button0_=new piw::toolbar_button_t(id,"Configure");
            }
            return 0;
        }

        void buttonClicked(juce::Button *b)
        {
            if(b==button0_) mapping();
        }

        void load()
        {
        }

        void save()
        {
        }

        void midi()
        {
        }

        void mapping();

        host::plugin_instance_t::impl_t *controller_;
        host_dialog_t *mapping_dialog_;
        juce::ToolbarButton *button0_;
    };

    struct host_view_t: juce::DocumentWindow
    {
        host_view_t(const juce::String &name, Component *content, host::plugin_instance_t::impl_t *controller,std::auto_ptr<juce::Rectangle<int> > &bounds);

        ~host_view_t()
        {
            clearContentComponent();
        }

        void closeButtonPressed();
        host::plugin_instance_t::impl_t *controller_;
        mainpanel_delegate_t delegate_;
    };

    struct metronome_input_t: host_scalar_t, juce::AudioPlayHead
    {
        metronome_input_t(host::plugin_instance_t::impl_t *c);

        bool getCurrentPosition(CurrentPositionInfo &result);

        unsigned buffer_size();

        void schedule(unsigned long long f, unsigned long long t)
        {
            if(!iterator_.isvalid())
            {
                return;
            }

            bool r = running_;
            piw::data_nb_t rd;
            while(iterator_->nextsig(2,rd,t))
            {
                r = rd.as_norm()!=0.f;
            }

            if(r!=running_)
            {
                running_ = r;
                if(r)
                {
                    sample_counter_ = -buffer_size();
                }
            }

            if(running_)
            {
                sample_counter_ += buffer_size();
            }

            piw::data_nb_t td;
            while(iterator_->nextsig(3,td,t))
            {
                tempo_ = td.as_renorm_float(BCTUNIT_BPM,0,100000,0);
            }

            piw::data_nb_t bd;
            while(iterator_->nextsig(1,bd,t))
            {
                interp_.recv_clock(0,bd);
            }

            piw::data_nb_t bb;
            while(iterator_->nextsig(4,bb,t))
            {
                beats_in_bar_ = lround(bb.as_array_ubound());
                interp_.recv_clock(1,bb);
            }

            if(interp_.clockvalid(0))
                beat_ = interp_.interpolate_clock(0,f+1);

            if(interp_.clockvalid(1))
                bar_beat_ = interp_.interpolate_clock(1,f+1);
        }

        piw::clockinterp_t interp_;
        double beat_;
        double tempo_;
        bool running_;
        double sample_counter_;
        long int beats_in_bar_;
        double bar_beat_;
    };

    struct plugin_guard_wrapper: pic::tracked_t
    {
        plugin_guard_wrapper(pic::flipflop_t<juce::AudioPluginInstance *>::guard_t &guard) : guard_(guard)
        {
        }

        float get_parameter(int param)
        {
            return guard_.value()->getParameter(param);
        }

        pic::flipflop_t<juce::AudioPluginInstance *>::guard_t &guard_;
    };
}

struct host::plugin_instance_t::impl_t: piw::params_delegate_t, piw::mapping_observer_t, piw::clocksink_t, piw::thing_t, virtual pic::tracked_t
{
    impl_t(plugin_observer_t *obs, piw::midi_channel_delegate_t *channel_delegate, piw::clockdomain_ctl_t *d,
        const piw::cookie_t &audio_out, const piw::cookie_t &midi_out, const pic::status_t &window_state): 
            audio_output_cookie_(audio_out), audio_input_(this), midi_input_(this), metronome_input_(this), 
            mapping_(*this), plugin_(0), window_(0), audio_buffer_(0,0), num_input_channels_(0), num_output_channels_(0),
            window_state_changed_(window_state), clockdomain_(d), sample_rate_(48000.0), buffer_size_(PLG_CLOCK_BUFFER_SIZE),
            observer_(obs), channel_delegate_(channel_delegate), active_(false),
            idle_count_(0), idle_time_ticks_(0), idle_time_sec_(10.f),
            main_delegate_(this),
            settings_functors_(piw::settings_functors_t::init(
                    piw::clearall_t::method(this,&host::plugin_instance_t::impl_t::clear_all),
                    piw::get_settings_t::method(this,&host::plugin_instance_t::impl_t::get_settings),
                    piw::change_settings_t::method(this,&host::plugin_instance_t::impl_t::change_settings),
                    piw::set_channel_t::method(channel_delegate_,&piw::midi_channel_delegate_t::set_midi_channel),
                    piw::set_channel_t::method(channel_delegate_,&piw::midi_channel_delegate_t::set_min_channel),
                    piw::set_channel_t::method(channel_delegate_,&piw::midi_channel_delegate_t::set_max_channel),
                    piw::get_channel_t::method(channel_delegate_,&piw::midi_channel_delegate_t::get_midi_channel),
                    piw::get_channel_t::method(channel_delegate_,&piw::midi_channel_delegate_t::get_min_channel),
                    piw::get_channel_t::method(channel_delegate_,&piw::midi_channel_delegate_t::get_max_channel)
                    )),
            mapping_delegate_(settings_functors_),
            midi_aggregator_(0), midi_from_belcanto_(0)
    {
        d->sink(this,"host");
        piw::tsd_thing(this);
        d->add_listener(pic::notify_t::method(this,&impl_t::clock_changed));

        midi_output_.scalar_connect(midi_out);

        for(unsigned i=0; i<15; i++)
        {
            param_input_[i] = std::auto_ptr<piw::param_input_t>(new piw::param_input_t(this,i+1));
        }
        param_input_[15] = std::auto_ptr<piw::param_input_t>(new piw::keynum_input_t(this,16));
        for(unsigned i=16; i<32; i++)
        {
            param_input_[i] = std::auto_ptr<piw::param_input_t>(new piw::param_input_t(this,i+1));
        }

        audio_output_.set_clock(this);
        midi_output_.set_clock(this);

        recalc_idle_time();

        host_window_.set_state_handler(window_state);

        midi_aggregator_ = new piw::aggregator_t(piw::cookie_t(&midi_input_), clockdomain_);

        midi_from_belcanto_ = new piw::midi_from_belcanto_t(midi_aggregator_->get_output(1), clockdomain_);
        midi_from_belcanto_->set_resend_current(piw::resend_current_t ::method(this, &host::plugin_instance_t::impl_t::resend_parameter_current));
        midi_from_belcanto_->set_midi_channel(0);
    }

    ~impl_t()
    {
        if(midi_from_belcanto_) delete midi_from_belcanto_;
        if(midi_aggregator_) delete midi_aggregator_;
    }

    void clock_changed()
    {
        pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(plugin_);
        juce::AudioPluginInstance *p(pg.value());
        if(!p)
        {
            return;
        }
        p->releaseResources();
        sample_rate_ = clockdomain_->get_sample_rate();
        buffer_size_ = clockdomain_->get_buffer_size();
        allocate_buffer(p);
        p->prepareToPlay(sample_rate_, buffer_size_);
        recalc_idle_time();
    }
    
    juce::AudioPluginInstance *find_plugin(juce::PluginDescription &desc,juce::String &err)
    {
        juce::AudioPluginInstance *plg = juce::AudioPluginFormatManager::getInstance()->createPluginInstance(desc,err);
        return plg;
    }

    bool open(const plugin_description_t &d)
    {
        plugin_description_t d2(d);
        juce::PluginDescription &desc = d2.desc_;

        close();

        juce::String err;
        juce::AudioPluginInstance *plg = find_plugin(desc,err);

        if(!plg)
        {
            return false;
        }

        num_input_channels_ = std::min(64,plg->getNumInputChannels());
        num_output_channels_ = std::min(64,plg->getNumOutputChannels());

        audio_output_.setup(num_output_channels_,PIW_DATAQUEUE_SIZE_ISO);
        midi_output_.setup(1,PIW_DATAQUEUE_SIZE_NORM);
        sample_rate_ = clockdomain_->get_sample_rate();
        buffer_size_ = clockdomain_->get_buffer_size();
        allocate_buffer(plg);
        midi_time_ = 0ULL;
        plg->setPlayHead(&metronome_input_);
        plg->prepareToPlay(sample_rate_, buffer_size_);
        plg->fillInPluginDescription(desc);
        plugin_.set(plg);
        observer_->description_changed(d2.to_xml());

        piw::tsd_window(&host_window_);
        pic::logmsg() << "opened " << d.id() << " in channels " << num_input_channels_ << " out channels " << num_output_channels_ << " buf channels " << audio_buffer_.getNumChannels();
        return true;
    }

    void allocate_buffer(juce::AudioPluginInstance *plg)
    {
        deallocate_buffer();

        int buffer_channels = std::max(plg->getNumInputChannels(),plg->getNumOutputChannels());
        float **chans = new float*[buffer_channels];
        try
        {
            for(int i=0; i<buffer_channels; ++i)
                chans[i] = (float *)pic_thread_lck_malloc(buffer_size_*sizeof(float));
            audio_buffer_.setDataToReferTo(chans,buffer_channels,buffer_size_);
        }
        catch(...)
        {
            delete[] chans;
            throw;
        }
        delete[] chans;
    }

    void deallocate_buffer()
    {
        for(int i=0; i<audio_buffer_.getNumChannels(); ++i)
        {
            pic_thread_lck_free(audio_buffer_.getSampleData(i,0),buffer_size_*sizeof(float));
        }
        audio_buffer_.setSize(0,0);
    }

    void show_gui()
    {
        if(!window_)
        {
            pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(plugin_);
            juce::AudioPluginInstance *p(pg.value());
            if(p)
            {
                juce::Component *editor = p->createEditorIfNeeded();
                if(0 == editor)
                {
                    editor = new juce::GenericAudioProcessorEditor(p);
                }
                if(editor != 0)
                {
                    window_ = new host_view_t(title_.c_str(),editor,this,bounds_);

                    //  move window a bit so we can move it back later to force redraw
                    //juce::Rectangle<int> r(window_->getBounds());
                    //window_->setBounds(r.getX()+1,r.getY(),window_->getWidth(),window_->getHeight());
                    //timer_slow(1000);
                    host_window_.set_window_state(true);
                    refresh_title();
                    observer_->showing_changed(true);
                }
            }
        }

        if(window_)
        {
            pic::to_front();
            window_->toFront(true);
        }
    }

    void update_gui()
    {
        pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(plugin_);
        juce::AudioPluginInstance *p(pg.value());
        if(p)
        {
            p->updateHostDisplay();
        }
    }

    void thing_timer_slow()
    {
        // horrible hack to move the window slightly to force it to redraw
        if(window_)
        {
            juce::Rectangle<int> r(window_->getBounds());
            window_->setBounds(r.getX()-1,r.getY(),window_->getWidth(),window_->getHeight());
        }
        cancel_timer_slow();
    }

    void hide_gui()
    {
        if(window_)
        {
            window_->setVisible(false);
            main_delegate_.close();
            mapping_delegate_.close();
            bounds_.reset(new juce::Rectangle<int>(window_->getBounds()));
            delete window_;
            window_ = 0;
            host_window_.set_window_state(false);
        }
        observer_->showing_changed(false);
    }

    bool is_showing()
    {
        return window_ && window_->isVisible();
    }

    juce::Rectangle<int> get_bounds()
    {
        if(window_ && is_showing())
        {
            return window_->getBounds();
        }
        else if(bounds_.get())
        {
            return *(bounds_.get());
        }
        else
        {
            return juce::Rectangle<int>(100,100,0,0);
        }
    }

    void set_bounds(const juce::Rectangle<int>& bounds)
    {
        bounds_.reset(new juce::Rectangle<int>(bounds));
        if(window_)
        {
            window_->setBounds(*(bounds_.get()));
        }
    }

    void close()
    {
        if(window_)
        {
            window_->setVisible(false);
            main_delegate_.close();
            mapping_delegate_.close();
            bounds_.reset(new juce::Rectangle<int>(window_->getBounds()));
            delete window_;
            window_ = 0;
        }

        host_window_.close_window();

        juce::AudioPluginInstance *p(plugin_.current());

        plugin_.set(0);

        deallocate_buffer();

        set_bypassed(true);

        if(p)
        {
            p->releaseResources();
            delete p;
        }

        observer_->description_changed("");
    }

    bool input_audio(unsigned long long from,unsigned long long to)
    {
        piw::xevent_data_buffer_t::iter_t &ei = audio_input_.iterator_;

        if(!ei.isvalid())
            return false;

        bool any = false;
        for(unsigned c=0; c<num_input_channels_; ++c)
        {
            piw::data_nb_t d;
            while(ei->nextsig(c+1,d,to))
            {
            }

            float *buffer = audio_buffer_.getSampleData(c);
            if(d.as_arraylen()==buffer_size_)
            {
                any = true;
                memcpy(buffer,d.as_array(),buffer_size_*sizeof(float));
            }
        }

        return any;
    }

    void input_parameters(unsigned long long from,unsigned long long to)
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

    void resend_parameter_current(const piw::data_nb_t &d)
    {
        for(unsigned i=0; i<32; ++i)
        {
            param_input_[i]->resend_current(d);
        }
    }

    void update_parameter_origins()
    {
        piw::tsd_fastcall(__update_parameter_origins,this,0);
    }

    static int __update_parameter_origins(void *i_, void *)
    {
        host::plugin_instance_t::impl_t *impl = (host::plugin_instance_t::impl_t *)i_;
        for(unsigned i=0; i<32; ++i)
        {
            impl->param_input_[i]->update_origins();
        }
        return 0;
    }

    bool input_midi(unsigned long long from,unsigned long long to)
    {
        return midi_input_.schedule(from,to);
    }

    void input_metronome(unsigned long long from,unsigned long long to)
    {
        metronome_input_.schedule(from,to);
    }

    bool output_audio(unsigned long long from,unsigned long long to)
    {
        bool any = false;

        for(unsigned c=0; c<num_output_channels_; ++c)
        {
            float *f;
            float *fs;
            piw::data_nb_t d = piw::makenorm_nb(to,buffer_size_,&f,&fs);
            memcpy(f,audio_buffer_.getSampleData(c),buffer_size_*sizeof(float));
            *fs = f[buffer_size_-1];
            audio_output_.buffer_.add_value(c+1,d);
            for(unsigned i=(buffer_size_/2); i<buffer_size_; ++i)
            {
                if(fabsf(f[i])>1.0e-6f)
                {
                    any = true;
                }
            }
        }

        return any;
    }

    bool output_midi(unsigned long long from,unsigned long long to)
    {
        juce::MidiBuffer::Iterator i(midi_buffer_);

        bool any = false;

        unsigned ticks_per_sample = (to-from)/buffer_size_;
        const unsigned char *data;
        int length;
        int position;
        while(i.getNextEvent(data,length,position))
        {
            any = true;
            unsigned char *blob;
            unsigned long long time = from+ticks_per_sample*position;
            time = std::max(midi_time_+1,time);
            time = std::min(to,time);
            piw::data_nb_t d = piw::makeblob_nb(time,length,&blob);
            memcpy(blob,data,length);
            midi_output_.buffer_.add_value(1,d);
            midi_time_ = time;
        }

        return any;
    }

    void start_output_events(unsigned long long t)
    {
        if(!active_)
        {
            active_ = true;
            audio_output_.source_start(0,piw::pathnull_nb(t),audio_output_.buffer_);
            midi_output_.source_start(0,piw::pathnull_nb(t),midi_output_.buffer_);
        }
    }

    void end_output_events(unsigned long long t)
    {
        if(active_)
        {
            active_ = false;
            audio_output_.source_end(t);
            midi_output_.source_end(t);
        }
    }

    void remove_upstream_clock(bct_clocksink_t *c)
    {
        remove_upstream(c);
    }

    void add_upstream_clock(bct_clocksink_t *c)
    {
        add_upstream(c);
    }

    void update_origins(piw::control_mapping_t &control_mapping)
    {
        pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(plugin_);
        if(!pg.value()) return;

        plugin_guard_wrapper wrapper(pg);
        mapping_.refresh_origins(control_mapping, pic::i2f_t::method(&wrapper,&plugin_guard_wrapper::get_parameter));
    }

    void update_mapping(piw::control_mapping_t &control_mapping)
    {
        mapping_.refresh_mappings(control_mapping);
    }

    void set_parameters(pic::lckvector_t<piw::param_data_t>::nbtype &params)
    {
        pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(plugin_);
        if(!pg.value()) return;

        pic::lckvector_t<piw::param_data_t>::nbtype::iterator i,e;

        i = params.begin();
        e = params.end();

        for(; i!=e; ++i)
        {
            if(PERNOTE_SCOPE==i->scope_)
            {
                unsigned channel = midi_from_belcanto_->get_active_midi_channel(i->id_);
                if(channel > 0) channel--;
                pg.value()->setParameter(i->param_+channel,i->value_);
            }
            else
            {
                pg.value()->setParameter(i->param_,i->value_);
            }
        }
    }

    void set_midi(pic::lckvector_t<piw::midi_data_t>::nbtype &midi)
    {
        midi_from_belcanto_->set_midi(midi);
    }

    void clocksink_ticked(unsigned long long from, unsigned long long to)
    {
        pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(plugin_);
        juce::AudioPluginInstance *p(pg.value());
        if(!p)
        {
            end_output_events(to);
            return;
        }

        for(unsigned c=0; c<num_input_channels_; ++c)
        {
            memset(audio_buffer_.getSampleData(c),0,buffer_size_*sizeof(float));
        }

        input_parameters(from,to);
        input_metronome(from,to);

        bool any_input = input_audio(from,to);
        if(p->acceptsMidi() && input_midi(from,to))
        {
            any_input = true;
        }

        bool any_output = false;

        if(any_input || idle_count_<idle_time_ticks_)
        {
            p->processBlock(audio_buffer_, midi_buffer_);
            any_output = output_audio(from,to);
            if(p->producesMidi() && output_midi(from,to))
            {
                any_output = true;
            }
        }

        if(any_output || any_input)
        {
            idle_count_ = 0;
            if(!active_)
            {
                start_output_events(to);
                trigger_slow();
            }
        }
        else
        {
            if(idle_count_<idle_time_ticks_)
            {
                ++idle_count_;
            }
            else
            {
                if(active_)
                {
                    end_output_events(to);
                    trigger_slow();
                }
            }
        }
    }

    void set_bypassed(bool b)
    {
        if(b)
        {
            tick_disable();
            audio_output_.scalar_disconnect();
            audio_input_clone_.clear_output(1);
            audio_input_clone_.set_output(1,audio_output_cookie_);
            active_=false;
            unsigned long long ts = piw::tsd_time();
            audio_output_.end_slow(ts);
            midi_output_.end_slow(ts);
        }
        else
        {
            audio_input_clone_.clear_output(1);
            audio_input_clone_.set_output(1,piw::cookie_t(&audio_input_));
            audio_output_.scalar_connect(audio_output_cookie_);
            tick_enable(false);
        }

        bypassed_ = b;
        refresh_title();
        observer_->bypassed_changed(b);
    }

    bool is_bypassed()
    {
        return bypassed_;
    }

    void mapping_changed(const std::string &mapping)
    {
        observer_->mapping_changed(mapping);

        update_parameter_origins();
    }

    void parameter_changed(unsigned param)
    {
        parameter_changed_params_(param);
        parameter_changed_midi_cc_(param);
        parameter_changed_midi_behaviour_(param);
    }

    void settings_changed()
    {
        perform_settings_updates(get_settings());
        mapping_delegate_.settings_changed();
    }

    std::string get_parameter_name(unsigned i)
    {
        return observer_->get_parameter_name(i);
    }

    void set_title(const std::string &s)
    {
        title_ = s;
        refresh_title();
    }

    void thing_trigger_slow()
    {
        pic::logmsg() << "going " << (active_?"active":"idle");
        refresh_title();
    }

    void refresh_title()
    {
        if(window_ && is_showing())
        {
            juce::String title(title_.c_str());
            if(bypassed_)
                title += " (bypassed)";
            else
                title += (active_?" (active)" :" (idle)");
            window_->setName(title);
        }

        if(host_window_.open())
        {
            host_window_.set_window_title(title_.c_str());
        }
    }

    void set_idle_time(float tt)
    {
        idle_time_sec_ = tt;
        recalc_idle_time();
    }

    void recalc_idle_time()
    {
        idle_time_ticks_ = (unsigned)(ceilf(idle_time_sec_*sample_rate_/buffer_size_));
    }

    void disable_idling()
    {
        idle_time_ticks_ = std::numeric_limits<unsigned>::max();
    }

    void clear_all()
    {
        mapping_.clear_params();
        mapping_.clear_midi_cc();
        mapping_.clear_midi_behaviour();
    }

    void change_settings(piw::global_settings_t settings)
    {
        mapping_.change_settings(settings);
    }

    void perform_settings_updates(piw::global_settings_t settings)
    {
        midi_from_belcanto_->set_send_notes(settings.send_notes_);
        midi_from_belcanto_->set_send_pitchbend(settings.send_pitchbend_);
        midi_from_belcanto_->set_control_interval(settings.minimum_decimation_);
    }

    piw::global_settings_t get_settings()
    {
        return mapping_.get_settings();
    }

    piw::sclone_t audio_input_clone_;
    piw::cookie_t audio_output_cookie_;

    host_scalar_t audio_input_;
    midi_input_t midi_input_;
    metronome_input_t metronome_input_;
    std::auto_ptr<piw::param_input_t> param_input_[32];

    piw::controllers_mapping_t mapping_;

    host_output_scalar_t audio_output_;
    host_output_scalar_t midi_output_;

    pic::flipflop_t<juce::AudioPluginInstance *> plugin_;
    host_view_t *window_;
    std::string title_;

    juce::AudioSampleBuffer audio_buffer_;
    unsigned num_input_channels_;
    unsigned num_output_channels_;
    juce::MidiBuffer midi_buffer_;

    pic::status_t window_state_changed_;
    piw::clockdomain_ctl_t *clockdomain_;
    double sample_rate_;
    unsigned buffer_size_;
    unsigned long long midi_time_;

    bool bypassed_;

    plugin_observer_t *observer_;
    piw::midi_channel_delegate_t *channel_delegate_;
    pic::f_int_t parameter_changed_params_;
    pic::f_int_t parameter_changed_midi_cc_;
    pic::f_int_t parameter_changed_midi_behaviour_;
    bool active_;

    unsigned idle_count_;
    unsigned idle_time_ticks_;
    float idle_time_sec_;

    piw::window_t host_window_;
    std::auto_ptr<juce::Rectangle<int> > bounds_;

    mainpanel_delegate_t main_delegate_;

    piw::settings_functors_t settings_functors_;
    piw::mapping_delegate_t mapping_delegate_;

    piw::aggregator_t *midi_aggregator_;
    piw::midi_from_belcanto_t *midi_from_belcanto_;
};


void host_view_t::closeButtonPressed()
{
    controller_->hide_gui();
}

void mainpanel_delegate_t::mapping()
{
    if(mapping_dialog_)
    {
        mapping_dialog_->setVisible(!mapping_dialog_->isVisible());
    }
    else
    {
        mapping_dialog_ = new mapping_dialog_t(controller_);
    }
}

mapping_dialog_t::mapping_dialog_t(host::plugin_instance_t::impl_t *controller):
    host_dialog_t(juce::String("Configure:")+controller->title_.c_str(),
            new host_panel_t(new host_param_tabs_t(controller), controller, &controller->mapping_delegate_))
{
    setResizable(true,true);
}

unsigned metronome_input_t::buffer_size()
{
    return controller_->buffer_size_;
}

host_view_t::host_view_t(const juce::String &name, Component *content, host::plugin_instance_t::impl_t *controller, std::auto_ptr<juce::Rectangle<int> > &bounds):
    DocumentWindow(name,juce::Colours::black,juce::DocumentWindow::closeButton,true), controller_(controller), delegate_(controller_)
{
    setResizable(true,true);
    setUsingNativeTitleBar(true);
    host_panel_t *c = new host_panel_t(content,controller,&controller_->main_delegate_);
    setContentOwned(c,true);
    if(bounds.get())
    {
        setTopLeftPosition(bounds->getX(),bounds->getY());
    }
    else
    {
        setTopLeftPosition(100,100);
    }

    setVisible(true);
    toFront(false);
}

midi_input_t::midi_input_t(host::plugin_instance_t::impl_t *c): piw::input_root_t(c), controller_(c)
{
}

bool midi_input_t::schedule(unsigned long long from, unsigned long long to)
{
    piw::host_wire_map_flipflop_t::guard_t guard(wires_);
    piw::host_wire_map_t::const_iterator wi = guard.value().begin(), we = guard.value().end();

    controller_->midi_buffer_.clear();

    bool any = false;
    for(; wi!=we; ++wi)
    {
        piw::xevent_data_buffer_t::iter_t &ei = wi->second->iterator_;
        if(!ei.isvalid())
        {
            continue;
        }

        piw::data_nb_t d;
        while(ei->nextsig(1,d,to))
        {
            any = true;
            const unsigned char *b = (const unsigned char *)d.as_blob();
            controller_->midi_buffer_.addEvent(b,d.as_bloblen(),timestamp_to_sample_offset(controller_->buffer_size_,d.time(),from,to));
        }
    }
    return any;
}

host_scalar_t::host_scalar_t(host::plugin_instance_t::impl_t *c): piw::root_t(0), controller_(c), clk_(0)
{
}

void host_scalar_t::invalidate()
{
    unsubscribe();
}

void host_scalar_t::root_clock()
{
    if(clk_) controller_->remove_upstream(clk_);

    clk_ = get_clock();

    if(clk_) controller_->add_upstream(clk_);
}

metronome_input_t::metronome_input_t(host::plugin_instance_t::impl_t *c):
    host_scalar_t(c), interp_(2), beat_(0.0), tempo_(120.0), running_(false),
    sample_counter_(-PLG_CLOCK_BUFFER_SIZE), beats_in_bar_(4), bar_beat_(0.0)
{
}

bool metronome_input_t::getCurrentPosition(CurrentPositionInfo &result)
{
    result.bpm = tempo_;
    result.timeSigNumerator = beats_in_bar_;
    result.timeSigDenominator = 4;
    result.timeInSeconds = sample_counter_/controller_->sample_rate_;
    result.ppqPosition = beat_;
    result.ppqPositionOfLastBarStart = beat_-bar_beat_;
    result.isPlaying = running_;
    return true;
}

host_param_tabs_t::host_param_tabs_t(host::plugin_instance_t::impl_t *c) : 
    au_mapper_(piw::mapping_functors_t::init(
                piw::is_mapped_t::method(&c->mapping_,&piw::controllers_mapping_t::is_mapped_param),
                piw::get_info_t::method(&c->mapping_,&piw::controllers_mapping_t::get_info_param),
                piw::map_t::method(&c->mapping_,&piw::controllers_mapping_t::map_param),
                piw::unmap_t::method(&c->mapping_,&piw::controllers_mapping_t::unmap_param),
                piw::get_name_t::method(c,&host::plugin_instance_t::impl_t::get_parameter_name),
                piw::clear_t::method(&c->mapping_,&piw::controllers_mapping_t::clear_params)
                )),
    midi_cc_mapper_(piw::mapping_functors_t::init(
                piw::is_mapped_t::method(&c->mapping_,&piw::controllers_mapping_t::is_mapped_midi),
                piw::get_info_t::method(&c->mapping_,&piw::controllers_mapping_t::get_info_midi),
                piw::map_t::method(&c->mapping_,&piw::controllers_mapping_t::map_midi),
                piw::unmap_t::method(&c->mapping_,&piw::controllers_mapping_t::unmap_midi),
                piw::get_name_t::method(c,&host::plugin_instance_t::impl_t::get_parameter_name),
                piw::clear_t::method(&c->mapping_,&piw::controllers_mapping_t::clear_midi_cc)
                )),
    midi_behaviour_mapper_(piw::mapping_functors_t::init(
                piw::is_mapped_t::method(&c->mapping_,&piw::controllers_mapping_t::is_mapped_midi),
                piw::get_info_t::method(&c->mapping_,&piw::controllers_mapping_t::get_info_midi),
                piw::map_t::method(&c->mapping_,&piw::controllers_mapping_t::map_midi),
                piw::unmap_t::method(&c->mapping_,&piw::controllers_mapping_t::unmap_midi),
                piw::get_name_t::method(c,&host::plugin_instance_t::impl_t::get_parameter_name),
                piw::clear_t::method(&c->mapping_,&piw::controllers_mapping_t::clear_midi_behaviour)
                ))
{
    host_param_table_t *pt = new host_param_table_t(c->settings_functors_, au_mapper_, c);
    piw::mapper_midi_cc_table_t *ct = new piw::mapper_midi_cc_table_t(c->settings_functors_, midi_cc_mapper_);
    piw::mapper_midi_behaviour_table_t *mt = new piw::mapper_midi_behaviour_table_t(c->settings_functors_, midi_behaviour_mapper_);

    pt->initialize();
    ct->initialize();
    mt->initialize();

    c->parameter_changed_params_ = pic::f_int_t::method(pt,&host_param_table_t::column_changed);
    c->parameter_changed_midi_cc_ = pic::f_int_t::method(ct,&piw::mapper_midi_cc_table_t::column_changed);
    c->parameter_changed_midi_behaviour_ = pic::f_int_t::method(mt,&piw::mapper_midi_behaviour_table_t::column_changed);

    addAndMakeVisible (tabs_ = new TabbedComponent(TabbedButtonBar::TabsAtTop));
    tabs_->setTabBarDepth(30);
    tabs_->addTab(T("Plugin Parameters"), Colours::lightgrey, pt, true);
    tabs_->addTab(T("MIDI CC Messages"), Colours::lightgrey, ct, true);
    tabs_->addTab(T("MIDI Behaviour"), Colours::lightgrey, mt, true);
    tabs_->setCurrentTabIndex(0);

    setBounds(0,0,800,500);
}

int host_param_table_t::get_parameter_count()
{
    pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(controller_->plugin_);
    juce::AudioPluginInstance *p(pg.value());
    if(!p) return 0;

    unsigned long long now = piw::tsd_time();
    if(parameter_count_ || last_parameter_check_ + 1000000 > now)
    {
        return parameter_count_;
    }

    p->refreshParameterListFromPlugin();
    int count = p->getNumParameters();
    parameter_count_ = count;
    last_parameter_check_ = now;
    return parameter_count_;
}

int host_param_table_t::getNumRows()
{
    int rows = get_parameter_count();
    return rows;
}

juce::String host_param_table_t::getRowName(int row)
{
    pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(controller_->plugin_);
    juce::AudioPluginInstance *p(pg.value());
    if(!p) return juce::String("");

    if(row < get_parameter_count())
    {
        std::stringstream oss;
        oss << row;
        oss << ": ";
        oss << p->getParameterName(row);
        return juce::String(oss.str().c_str());
    }
    else
    {
        return juce::String("");
    }
}

void host_param_table_t::default_mapping(piw::mapper_cell_editor_t &e)
{
    e.map(true,1.f,1.f,0.f,1.f,true,0.f,GLOBAL_SCOPE,0,BITS_7,-1,CURVE_LINEAR);
}

host::plugin_instance_t::plugin_instance_t(host::plugin_observer_t *obs, piw::midi_channel_delegate_t *channel_delegate,
    piw::clockdomain_ctl_t *d, const piw::cookie_t &audio_out, const piw::cookie_t &midi_out, 
    const pic::status_t &window_state_changed):
        impl_(new impl_t(obs,channel_delegate,d,audio_out,midi_out,window_state_changed))
{
}

host::plugin_instance_t::~plugin_instance_t()
{
    delete impl_;
}

void host_panel_t::resized()
{
    if(toolbar_) toolbar_->setBounds(0,0,getWidth(),32);
    if(content_) content_->setBounds(0,32,getWidth(),getHeight()-32);
    controller_->update_gui();
    if(content_) content_->repaint();
}

void host_panel_t::componentMovedOrResized(Component &c, bool moved, bool resized)
{
    if(resized)
    {
        setBounds(0,0,c.getWidth(),c.getHeight()+32);
        if(toolbar_) toolbar_->setBounds(0,0,c.getWidth(),32);
        if(content_) content_->setBounds(0,32,c.getWidth(),c.getHeight());
        controller_->update_gui();
        if(content_) content_->repaint();
    }
}

piw::clockdomain_ctl_t *host::plugin_instance_t::clock_domain()
{
    return impl_->clockdomain_;
}

piw::cookie_t host::plugin_instance_t::metronome_input()
{
    return piw::cookie_t(&impl_->metronome_input_);
}

piw::cookie_t host::plugin_instance_t::midi_from_belcanto()
{
    return impl_->midi_from_belcanto_->cookie();
}

piw::cookie_t host::plugin_instance_t::midi_aggregator()
{
    return impl_->midi_aggregator_->get_output(2);
}

piw::cookie_t host::plugin_instance_t::audio_input()
{
    return impl_->audio_input_clone_.cookie();
}

void host::plugin_instance_t::set_midi_channel(unsigned ch)
{
    impl_->midi_from_belcanto_->set_midi_channel(ch);
    impl_->settings_changed();
}

void host::plugin_instance_t::set_min_midi_channel(unsigned ch)
{
    impl_->midi_from_belcanto_->set_min_midi_channel(ch);
    impl_->settings_changed();
}

void host::plugin_instance_t::set_max_midi_channel(unsigned ch)
{
    impl_->midi_from_belcanto_->set_max_midi_channel(ch);
    impl_->settings_changed();
}

void host::plugin_instance_t::set_program_change(unsigned c)
{
    impl_->midi_from_belcanto_->set_program_change(c);
}

piw::change_nb_t host::plugin_instance_t::change_program()
{
    return impl_->midi_from_belcanto_->change_program();
}

void host::plugin_instance_t::set_bank_change(unsigned c)
{
    impl_->midi_from_belcanto_->set_bank_change(c);
}

piw::change_nb_t host::plugin_instance_t::change_bank()
{
    return impl_->midi_from_belcanto_->change_bank();
}

void host::plugin_instance_t::set_cc(unsigned c, unsigned v)
{
    impl_->midi_from_belcanto_->set_cc(c, v);
}

piw::change_nb_t host::plugin_instance_t::change_cc()
{
    return impl_->midi_from_belcanto_->change_cc();
}

piw::cookie_t host::plugin_instance_t::parameter_input(unsigned name)
{
    return piw::cookie_t(impl_->param_input_[name-1].get());
}

void host::plugin_instance_t::set_mapping(const std::string &mapping)
{
    if(mapping!="") impl_->mapping_.set_mapping(mapping);
}

std::string host::plugin_instance_t::get_mapping()
{
    return impl_->mapping_.get_mapping();
}

void host::plugin_instance_t::parameter_name_changed(unsigned iparam)
{
    impl_->parameter_changed(iparam);
}

void host::plugin_instance_t::map_param(unsigned iparam, piw::mapping_info_t info)
{
    impl_->mapping_.map_param(iparam,info);
}

void host::plugin_instance_t::map_midi(unsigned iparam, piw::mapping_info_t info)
{
    impl_->mapping_.map_midi(iparam,info);
}

void host::plugin_instance_t::unmap_param(unsigned iparam, unsigned short oparam)
{
    impl_->mapping_.unmap_param(iparam,oparam);
}

void host::plugin_instance_t::unmap_midi(unsigned iparam, unsigned short oparam)
{
    impl_->mapping_.unmap_midi(iparam,oparam);
}

bool host::plugin_instance_t::is_mapped_param(unsigned iparam, unsigned short oparam)
{
    return impl_->mapping_.is_mapped_param(iparam,oparam);
}

bool host::plugin_instance_t::is_mapped_midi(unsigned iparam, unsigned short oparam)
{
    return impl_->mapping_.is_mapped_midi(iparam,oparam);
}

piw::mapping_info_t host::plugin_instance_t::get_info_param(unsigned iparam, unsigned short oparam)
{
    return impl_->mapping_.get_info_param(iparam,oparam);
}

piw::mapping_info_t host::plugin_instance_t::get_info_midi(unsigned iparam, unsigned short oparam)
{
    return impl_->mapping_.get_info_midi(iparam,oparam);
}

void host::plugin_instance_t::clear_params()
{
    return impl_->mapping_.clear_params();
}

void host::plugin_instance_t::clear_midi_cc()
{
    return impl_->mapping_.clear_midi_cc();
}

void host::plugin_instance_t::clear_midi_behaviour()
{
    return impl_->mapping_.clear_midi_behaviour();
}

void host::plugin_instance_t::set_minimum_decimation(float decimation)
{
    piw::global_settings_t settings = impl_->mapping_.get_settings();
    settings.minimum_decimation_= decimation;
    impl_->change_settings(settings);
}

void host::plugin_instance_t::set_midi_notes(bool enabled)
{
    piw::global_settings_t settings = impl_->mapping_.get_settings();
    settings.send_notes_= enabled;
    impl_->change_settings(settings);
}

void host::plugin_instance_t::set_midi_pitchbend(bool enabled)
{
    piw::global_settings_t settings = impl_->mapping_.get_settings();
    settings.send_pitchbend_= enabled;
    impl_->change_settings(settings);
}

unsigned host::plugin_instance_t::input_channel_count()
{
    return impl_->num_input_channels_;
}

unsigned host::plugin_instance_t::output_channel_count()
{
    return impl_->num_output_channels_;
}

bool host::plugin_instance_t::open(const plugin_description_t &d)
{
    return impl_->open(d);
}

void host::plugin_instance_t::close()
{
    impl_->close();
}

void host::plugin_instance_t::set_showing(bool b)
{
    if(b)
    {
        impl_->show_gui();
    }
    else
    {
        impl_->hide_gui();
    }
}

bool host::plugin_instance_t::is_showing()
{
    return impl_->is_showing();
}

void host::plugin_instance_t::set_bypassed(bool b)
{
    impl_->set_bypassed(b);
}

bool host::plugin_instance_t::is_bypassed()
{
    return impl_->is_bypassed();
}

void host::plugin_instance_t::set_title(const std::string &s)
{
    impl_->set_title(s);
}

host::plugin_description_t host::plugin_instance_t::get_description()
{
    pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(impl_->plugin_);
    juce::AudioPluginInstance *p(pg.value());

    host::plugin_description_t d;
    if(p)
    {
        p->fillInPluginDescription(d.desc_);
    }
    return d;
}

piw::data_t host::plugin_instance_t::get_state()
{
    pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(impl_->plugin_);
    juce::AudioPluginInstance *p(pg.value());
    if(p)
    {
        juce::MemoryBlock mb;
        p->getStateInformation(mb);
        unsigned char *c;
        piw::data_t d = piw::makeblob(0,mb.getSize(),&c);
        memcpy(c,mb.getData(),mb.getSize());
        return d;
    }
    return piw::data_t();
}

piw::data_t host::plugin_instance_t::get_bounds()
{
    pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(impl_->plugin_);
    juce::AudioPluginInstance *p(pg.value());
    if(p)
    {
        juce::Rectangle<int> bounds(impl_->get_bounds());
        int bounds_array[4]={bounds.getX(),bounds.getY(),bounds.getWidth(),bounds.getHeight()};
        int size=sizeof(bounds_array);
        unsigned char *c;
        piw::data_t d = piw::makeblob(0,size,&c);
        memcpy(c,bounds_array,size);
        return d;
    }
    return piw::data_t();
}

void host::plugin_instance_t::set_state(const piw::data_t &blob)
{
    if(!blob.is_blob() || blob.as_bloblen()==0)
    {
        return;
    }

    pic::flipflop_t<juce::AudioPluginInstance *>::guard_t pg(impl_->plugin_);
    juce::AudioPluginInstance *p(pg.value());
    if(p)
    {
        p->setStateInformation(blob.as_blob(), blob.as_bloblen());
    }
}

void host::plugin_instance_t::set_bounds(const piw::data_t &blob)
{
    if(!blob.is_blob() || blob.as_bloblen()==0)
    {
        return;
    }

    int *bounds=(int *)blob.as_blob();
    impl_->set_bounds(juce::Rectangle<int>(bounds[0],bounds[1],bounds[2],bounds[3]));
}

void host::plugin_instance_t::set_idle_time(float tt)
{
    impl_->set_idle_time(tt);
}

void host::plugin_instance_t::disable_idling()
{
    impl_->disable_idling();
}

int host::plugin_instance_t::gc_clear()
{
    return impl_->host_window_.gc_clear();
}

int host::plugin_instance_t::gc_traverse(void *a,void *b)
{
    return impl_->host_window_.gc_traverse(a,b);
}

int host::plugin_list_t::gc_clear()
{
    return impl_->complete_.gc_clear();
}

int host::plugin_list_t::gc_traverse(void *a,void *b)
{
    return impl_->complete_.gc_traverse(a,b);
}
