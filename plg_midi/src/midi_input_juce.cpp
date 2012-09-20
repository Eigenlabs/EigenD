
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

#include <picross/pic_time.h>
#include <picross/pic_log.h>
#include <picross/pic_config.h>

#include <plg_midi/midi_input.h>

#include <lib_juce/juce.h>

#include <iostream>
#include <iomanip>

using namespace std;

#define MIDI_INPUT_DEBUG 0

// number of MIDI keys on keyboard
#define KEYS 128
#define KBD_LATENCY 5000
#define NULL_DEVICE "Null Device"

struct pi_midi::midi_input_port_t::impl_t: piw::thing_t, juce::MidiInputCallback
{
    impl_t(pi_midi::midi_input_port_t *d,const piw::change_nb_t &r): current_(-1), receiver_(r), delegate_(d), input_(0), virtual_input_(0), running_(false), null_device_name_(NULL_DEVICE)
    {
        snapshot_.save();
        piw::tsd_thing(this);
    }

    ~impl_t()
    {
        tracked_invalidate();
        stop();
    }

    void run()
    {
        if(!running_)
        {
            running_ = true;
            timer_slow(2000);
        }
    }

    void stop()
    {
        if(running_)
        {
            running_= false;
            cancel_timer_slow();

            if(input_)
            {
                input_->stop();
                delete input_;
                input_ = 0;
            }

            if(virtual_input_)
            {
                virtual_input_->stop();
                delete virtual_input_;
                virtual_input_=0;
            }
        }
    }

    void thing_dequeue_fast(const piw::data_nb_t &d)
    {
        receiver_(d);
    }

    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
    {
        snapshot_.install();

        try
        {            
            unsigned char *output_buffer;
            piw::data_nb_t midi_data = piw::makeblob_nb(0, message.getRawDataSize(), &output_buffer);
            memcpy(output_buffer, message.getRawData(), message.getRawDataSize());
            enqueue_fast(midi_data);
        }
        CATCHLOG()
    }

    bool setport(int uid)
    {
        if(input_)
        {
            input_->stop();
            delete input_;
            input_ = 0;
        }

        current_ = (uid==0)?(-1):uid;
        return true;
    }

    long getport()
    {
        return current_;
    }

    void thing_timer_slow()
    {
        scan();
    }

    void scan()
    {
        juce::StringArray old_devices(devices_);
        juce::StringArray new_devices = juce::MidiInput::getDevices();

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
                if(input_ && old_devices[0].hashCode()==current_)
                {
                    input_->stop();
                    delete input_;
                    input_=0;
                    current_=-1;
                }

                delegate_->source_removed(old_devices[0].hashCode());
            }

            old_devices.remove(0);
        }

        for(int i=0;i<new_devices.size();i++)
        {
            delegate_->source_added(new_devices[i].hashCode(),std::string(new_devices[i].getCharPointer()));
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

        if(!input_ && current_index<devices_.size())
        {
            if(current_index>0)
            {
                input_ = juce::MidiInput::openDevice(current_index-1,this);                    

                if(input_)
                {
                    pic::logmsg() << "opening " << devices_[current_index];
                    input_->start();
                }
                else
                {
                    pic::logmsg() << "could not open " << devices_[current_index];
                }
            }
        }

#if JUCE_MAC || JUCE_LINUX
        if(!virtual_input_ && virtual_name_.length()>0)
        {
            virtual_input_ = juce::MidiInput::createNewDevice(virtual_name_,this);

            if(virtual_input_)
            {
                pic::logmsg() << "created input " << virtual_name_;
                virtual_input_->start();
            }
            else
            {
                pic::logmsg() << "couldn't create input " << virtual_name_;
            }
        }
#endif
    }

    void set_destination(const std::string &name)
    {
#if JUCE_MAC || JUCE_LINUX
        if(virtual_input_)
        {
            virtual_input_->stop();
            delete virtual_input_;
            virtual_input_=0;
        }

        virtual_name_ = juce::String::fromUTF8(name.c_str());
#endif

    }

    int current_;
    juce::StringArray devices_;
    piw::change_nb_t receiver_;
    pi_midi::midi_input_port_t *delegate_;
    juce::MidiInput *input_;
    juce::MidiInput *virtual_input_;
    juce::String virtual_name_;
    piw::tsd_snapshot_t snapshot_;
    bool running_;
    juce::String null_device_name_;
};

pi_midi::midi_input_port_t::midi_input_port_t(const piw::change_nb_t &sink) { impl_ = new impl_t(this,sink); }
pi_midi::midi_input_port_t::~midi_input_port_t() { stop(); delete impl_; }
bool pi_midi::midi_input_port_t::setport(long port) { return impl_->setport(port); }
long pi_midi::midi_input_port_t::getport(void) { return impl_->getport(); }
void pi_midi::midi_input_port_t::set_destination(const std::string &name) { impl_->set_destination(name); }
void pi_midi::midi_input_port_t::run() { return impl_->run(); }
void pi_midi::midi_input_port_t::stop() { return impl_->stop(); }
