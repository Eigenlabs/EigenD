
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

#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_fastdata.h>
#include <piw/piw_bundle.h>
#include <piw/piw_window.h>
#include <picross/pic_time.h>
#include <picross/pic_log.h>
#include <picross/pic_safeq.h>
#include <lib_midi/midi_output_port.h>
#include <lib_juce/juce.h>

#define NULL_DEVICE "Null Device"

struct midi::midi_output_port_t::impl_t: piw::thing_t, pic::safe_worker_t
{
    impl_t(midi::midi_output_port_t *delegate_output): pic::safe_worker_t(10,PIC_THREAD_PRIORITY_HIGH), current_(-1), delegate_(delegate_output),output_(0),virtual_output_(0),running_(false), null_device_name_(NULL_DEVICE)
    {
        piw::tsd_thing(this);
    }

    void stop()
    {
        if(running_)
        {
            running_ = false;
            cancel_timer_slow();
            quit();

            if(output_)
            {
                delete output_;
                output_=0;
            }

            if(virtual_output_)
            {
                delete virtual_output_;
                virtual_output_=0;
            }
        }
    }

    void run()
    {
        if(!running_)
        {
            running_ = true;
            pic::thread_t::run();
            timer_slow(5000);
        }
    }

    ~impl_t()
    {
        tracked_invalidate();
        stop();
    }

    void thing_timer_slow()
    {            
        scan();
    }

    void scan()
    {
        juce::StringArray old_devices(devices_);
        juce::StringArray new_devices = juce::MidiOutput::getDevices();

        new_devices.insert(0,null_device_name_);
        devices_ = new_devices;

        int current_index= -1;

        while(old_devices.size()>0)
        {
            int si = new_devices.indexOf(old_devices[0]);

            if(si>=0)
            {
                new_devices.remove(si);
            }
            else
            {
                if(output_ && old_devices[0].hashCode()==current_)
                {
                    delete output_;
                    output_=0;
                    current_=-1;
                }

                delegate_->sink_removed(old_devices[0].hashCode());
            }

            old_devices.remove(0);
        }

        for(int i=0;i<new_devices.size();i++)
        {
            delegate_->sink_added(new_devices[i].hashCode(),std::string(new_devices[i].getCharPointer()));
        }

        for(int i=0;i<devices_.size();i++)
        {
            if(devices_[i].hashCode()==current_)
            {
                current_index = i;
            }
        }

        if(current_index<0)
        {
            current_index = 0;
            current_ = devices_[current_index].hashCode();
        }

        if(!output_ && current_index<devices_.size())
        {
            if(current_index>0)
            {
                output_ = juce::MidiOutput::openDevice(current_index-1);
            }
        }

#if JUCE_MAC || JUCE_LINUX
        if(!virtual_output_ && virtual_name_.length()>0)
        {
            virtual_output_ = juce::MidiOutput::createNewDevice(virtual_name_);

            if(!virtual_output_)
            {
                pic::logmsg() << "couldn't create output " << virtual_name_;
            }
            else
            {
                pic::logmsg() << "created output " << virtual_name_;
            }
        }
#endif
    }

    static void sender__(void *a_, void *b_, void *c_, void *d_)
    {
        impl_t *self = (impl_t *)a_;
        piw::data_nb_t d = piw::data_nb_t::from_given((bct_data_t)b_);
        self->output__(d);
    }

    void output(const piw::data_nb_t &d)
    {
        add(sender__,this,d.give_copy(),0,0);
    }

    void output__(const piw::data_nb_t &d)
    {
        if(output_ || virtual_output_)
        {
            juce::MidiMessage mm((const unsigned char *)d.as_blob(),d.as_bloblen());

            if(output_)
            {
                output_->sendMessageNow(mm);
            }

            if(virtual_output_)
            {
                virtual_output_->sendMessageNow(mm);
            }
        }
    }

    long get_port(void)
    {
        return current_;
    }

    bool set_port(long uid)
    {
        if(output_)
        {
            delete output_;
            output_ = 0;
        }

        current_ = uid;
        scan();
        return true;
    }

    void set_source(const std::string &name)
    {
        pic::logmsg() << "set source " << " name " << name;

#if JUCE_MAC || JUCE_LINUX

        if(virtual_output_)
        {
            delete virtual_output_;
            virtual_output_=0;
        }

        virtual_name_ = juce::String::fromUTF8(name.c_str());
#endif
    }

    int current_;
    juce::StringArray devices_;
    juce::String virtual_name_;
    midi::midi_output_port_t *delegate_;
    juce::MidiOutput *output_;
    juce::MidiOutput *virtual_output_;
    bool running_;
    juce::String null_device_name_;
};

namespace midi
{
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // midi output interface class
    //
    //
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    midi_output_port_t::midi_output_port_t(): impl_(new impl_t(this))
    {
    }

    long midi_output_port_t::get_port(void)
    {
        return impl_->get_port();
    }

    bool midi_output_port_t::set_port(long uid)
    {
        return impl_->set_port(uid);
    }

    void midi_output_port_t::run()
    {
        impl_->run();
    }

    void midi_output_port_t::stop()
    {
        impl_->stop();
    }

    midi_output_port_t::~midi_output_port_t()
    {
        stop();
        delete impl_;
    }

    piw::change_nb_t midi_output_port_t::get_midi_output_functor()
    {
        return piw::change_nb_t::method(impl_, &midi_output_port_t::impl_t::output);
    }

    void midi_output_port_t::set_source(const std::string &name)
    {
        impl_->set_source(name);
    }

} // namespace midi
