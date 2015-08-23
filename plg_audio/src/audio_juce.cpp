
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

#include "audio.h"

#include <piw/piw_clock.h>
#include <piw/piw_tsd.h>
#include <piw/piw_thing.h>
#include <piw/piw_ring.h>
#include <piw/piw_window.h>
#include <picross/pic_time.h>
#include <pibelcanto/plugin.h>
#include <lib_juce/juce.h>

#include <vector>
#include <memory>

#include "AudioDialogComponent.h"

#define MAX_CHANNEL 64

namespace
{
    enum MessageTypes 
    {
        messageRefreshAll,
        messageRefreshTypes,
        messageRefreshDevices,
        messageRefreshRates,
        messageRefreshSizes,
        messageRefreshControl,
        messageOpenDevice
    };
    
    struct AudioMessage: juce::Message
    {
        AudioMessage (const int type, void* const payload): type_ (type), payload_ (payload) {}
        ~AudioMessage() {}
        
        const int type_;
        void* const payload_;
    };

    struct devicespec_t
    {
        devicespec_t(const juce::String &uid, unsigned long sr, unsigned bs) : uid_(uid), samplerate_(sr), buffersize_(bs) {};
        const juce::String uid_;
        const unsigned long samplerate_;
        const unsigned buffersize_;
    };

    struct SettingsDialog;

    bool filter_sample_rate(unsigned long sr)
    {
        if(sr==44100) return true;
        if(sr==48000) return true;
        if(sr==96000) return true;
        return false;
    }

    bool filter_buffer_size(unsigned bs)
    {
        if(bs<=PLG_CLOCK_BUFFER_SIZE && bs>0) return true;
        return false;
    }

    struct AudioDialogComponent: JucerAudioDialogComponent, juce::MessageListener
    {
        AudioDialogComponent(SettingsDialog *settings);
        ~AudioDialogComponent();

        void refresh();
        void refresh_types();
        void refresh_devices();
        void refresh_rates();
        void refresh_sizes();
        void refresh_control();
        void open_device(const juce::String &uid, unsigned long sr, unsigned bs);

        void buttonClicked (Button* buttonThatWasClicked);
        void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
        void handleMessage(const juce::Message &message);

        private:
            void raw_refresh();
            void raw_refresh_types();
            void raw_refresh_devices();
            void raw_refresh_rates();
            void raw_refresh_sizes();
            void raw_refresh_control();
            void raw_open_device(const juce::String &uid, unsigned long sr, unsigned bs);

            SettingsDialog *settings_;
    };

    struct SettingsDialog: juce::DocumentWindow
    {
        SettingsDialog(pi_audio::audioctl_t::impl_t *root);
        ~SettingsDialog();
        void closeButtonPressed();
        void refresh() { component_.refresh(); }

        pi_audio::audioctl_t::impl_t *root_;
        AudioDialogComponent component_;
    };
};

namespace pi_audio
{
    class audio_output_t;

    struct audioctl_t::impl_t:
           piw::wire_t,
           piw::clocksource_t,
           piw::clocksink_t,
           piw::thing_t,
           piw::decode_ctl_t,
           piw::decoder_t,
           piw::event_data_sink_t,
           juce::AudioIODeviceCallback,
           virtual pic::tracked_t,
           virtual pic::lckobject_t
    {
        impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d, const std::string &dn, pi_audio::audioctl_t *del);
        ~impl_t();

        void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels, float** outputChannelData, int numOutputChannels, int numSamples);
        void audioDeviceAboutToStart (juce::AudioIODevice* device) {}
        void audioDeviceStopped() {}
        void enumerate();
        void enable_callbacks(bool e);
        unsigned numdevices();
        std::string device_name(unsigned index);
        std::string device_uid(unsigned index);
        juce::String default_uid();
        juce::String get_uid();
        void refresh_gui();
        void refresh_host();

        void set_clock(bct_clocksink_t *);
        void set_latency(unsigned l) {}
        void wire_closed();
        void wire_latency() {}
        piw::wire_t *wire_create(const piw::event_data_source_t &es);
        void event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &ei);
        bool event_end(unsigned long long t);
        void event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n);

        void notify(bool hcb, bool wcb, const juce::String &uid, unsigned long sr, unsigned bs);
        bool open_device(const juce::String &uid,unsigned long sr, unsigned bs);
        void close_device();
        void thing_dequeue_fast(const piw::data_nb_t &);
        void thing_trigger_slow();
        void thing_timer_fast();

        juce::String find_name(juce::AudioIODevice *t);
        juce::AudioIODeviceType *find_type(const juce::String &t);
        void add_type(const juce::String &,juce::AudioIODeviceType *);

        void window_state(bool);

        void set_output_channels(piw::data_t channels);

        static int __mute(void *, void *);

        bool running_, changing_;
        juce::String current_uid_;
        pi_audio::audioctl_t *delegate_;
        piw::clockdomain_ctl_t *clockdomain_;
        bct_clocksink_t *up_;
        piw::ringbuffer_t<5> audio_in_[MAX_CHANNEL];
        piw::ringbuffer_t<5> audio_out_[MAX_CHANNEL];
        bool mute_;
        piw::tsd_snapshot_t ctx_;
        piw::xevent_data_buffer_t::iter_t iterator_;

        long long last_time_;
        unsigned long long elapsed_;
        unsigned long long lastlog_;
        unsigned count_;
        pic_atomic_t alldropout_;
        pic_atomic_t resetdropout_;
        bool enabled_;
        std::vector<juce::String> devices_;
        std::map<juce::String,juce::AudioIODeviceType *> types_;
        juce::AudioIODevice *device_;
        piw::window_t settings_window_;
        SettingsDialog *settings_;
        std::auto_ptr<audio_output_t> audio_output_;
        pic::lckmap_t<unsigned,bool>::nbtype audio_outputs_state_;
        bool runaway_;
    };

    struct audio_output_t: piw::root_ctl_t, piw::wire_ctl_t, piw::event_data_source_real_t, piw::clockdomain_ctl_t, piw::clocksink_t
    {
        audio_output_t(pi_audio::audioctl_t::impl_t *root, const piw::cookie_t &c): event_data_source_real_t(piw::pathnull(0)), root_(root)
        {
            sink(this,"audio output");
            set_clock(this);
            connect(c);
            connect_wire(this,source());
            for(int i=1; i<=MAX_CHANNEL; ++i)
            {
                buffer_.set_signal(i,piw::tsd_dataqueue(PIW_DATAQUEUE_SIZE_ISO));
            }
            tick_enable(false);
            piw::tsd_fastcall(init__,this,0);
        }

        static int init__(void *self_, void *arg_)
        {
            audio_output_t *self = (audio_output_t *)self_;

            unsigned long long t = piw::tsd_time();
            self->source_start(0,piw::pathnull_nb(t),self->buffer_);

            return 1;
        }

        void invalidate()
        {
            tick_disable();
        }

        void clocksink_ticked(unsigned long long f, unsigned long long t)
        {
            for(unsigned i=0; i<MAX_CHANNEL; ++i)
            {
                if(!root_->audio_out_[i].is_empty())
                {
                    pic::lckmap_t<unsigned,bool>::nbtype::iterator output_state_it = root_->audio_outputs_state_.find(i);
                    if(output_state_it != root_->audio_outputs_state_.end() && output_state_it->second)
                    {
                        piw::data_nb_t d = root_->audio_out_[i].read_all(); 
                        buffer_.add_value(i+1,d.restamp(t));
                    }
                }
            }
        }

        ~audio_output_t()
        {
            invalidate();
        }

        piw::xevent_data_buffer_t buffer_;
        pi_audio::audioctl_t::impl_t *root_;
    };
}


AudioDialogComponent::~AudioDialogComponent()
{
}

AudioDialogComponent::AudioDialogComponent(SettingsDialog *settings): settings_(settings)
{
    refresh();
}

void AudioDialogComponent::handleMessage(const juce::Message &message)
{
    AudioMessage *msg = (AudioMessage *)&message;
    switch(msg->type_)
    {
        case messageRefreshAll:
            raw_refresh();
            break;
        case messageRefreshTypes:
            raw_refresh_types();
            break;
        case messageRefreshDevices:
            raw_refresh_devices();
            break;
        case messageRefreshRates:
            raw_refresh_rates();
            break;
        case messageRefreshSizes:
            raw_refresh_sizes();
            break;
        case messageRefreshControl:
            raw_refresh_control();
            break;
        case messageOpenDevice:
            devicespec_t *spec = (devicespec_t *)msg->payload_;
            if(spec)
            {
                raw_open_device(spec->uid_, spec->samplerate_, spec->buffersize_);
                delete spec;
            }
            break;
    }
}

void AudioDialogComponent::refresh()
{
    postMessage(new AudioMessage(messageRefreshAll,0));
}

void AudioDialogComponent::refresh_types()
{
    postMessage(new AudioMessage(messageRefreshTypes,0));
}

void AudioDialogComponent::refresh_devices()
{
    postMessage(new AudioMessage(messageRefreshDevices,0));
}

void AudioDialogComponent::refresh_rates()
{
    postMessage(new AudioMessage(messageRefreshRates,0));
}

void AudioDialogComponent::refresh_sizes()
{
    postMessage(new AudioMessage(messageRefreshSizes,0));
}

void AudioDialogComponent::refresh_control()
{
    postMessage(new AudioMessage(messageRefreshControl,0));
}

void AudioDialogComponent::open_device(const juce::String &uid, unsigned long sr, unsigned bs)
{
    devicespec_t *spec = new devicespec_t(uid,sr,bs);
    postMessage(new AudioMessage(messageOpenDevice,(void *)spec));
}

void AudioDialogComponent::raw_refresh()
{
    raw_refresh_types();
    raw_refresh_devices();
    raw_refresh_rates();
    raw_refresh_sizes();
    raw_refresh_control();
}

void AudioDialogComponent::raw_refresh_types()
{
    pi_audio::audioctl_t::impl_t *root = settings_->root_;
    std::map<juce::String,juce::AudioIODeviceType *>::iterator ti;
    juce::AudioIODevice *device = root->device_;
    int j;

    getTypeChooser()->clear(true);

    for(ti=root->types_.begin(),j=1; ti!=root->types_.end(); ti++,j++)
    {
        getTypeChooser()->addItem(ti->first,j);

        if(device && device->getTypeName()==ti->second->getTypeName())
        {
            getTypeChooser()->setSelectedId(j,true);
        }
    }
}

void AudioDialogComponent::raw_refresh_devices()
{
    pi_audio::audioctl_t::impl_t *root = settings_->root_;
    juce::AudioIODevice *device = root->device_;

    getDeviceChooser()->clear(true);
    getDeviceChooser()->addItem("None",1);
    getDeviceChooser()->setSelectedId(1,true);

    int dti = getTypeChooser()->getSelectedItemIndex();

    if(dti<0)
    {
        return;
    }

    juce::String dtn = getTypeChooser()->getItemText(dti);
    juce::AudioIODeviceType *dt = root->find_type(dtn);

    if(!dt)
    {
        return;
    }

    dt->scanForDevices();
    const juce::StringArray &devices = dt->getDeviceNames();

    for(int j=0; j<devices.size(); ++j)
    {
        juce::String dn(devices[j]);
        pic::logmsg() << "have device: " << dn;

        getDeviceChooser()->addItem(dn,j+2);

        if(device && device->getTypeName()==dt->getTypeName() && device->getName()==dn)
        {
            getDeviceChooser()->setSelectedId(j+2,true);
        }
    }

}

void AudioDialogComponent::raw_refresh_rates()
{
    pi_audio::audioctl_t::impl_t *root = settings_->root_;
    juce::AudioIODevice *device = root->device_;

    getRateChooser()->clear(true);

    if(!device)
    {
        return;
    }

    int cr = device->getCurrentSampleRate();

    Array<double> asr = device->getAvailableSampleRates();
    for(int j=0;j<asr.size();j++)
    {
        int r = asr[j];

        if(filter_sample_rate(r))
        {
            getRateChooser()->addItem(juce::String(r),r);

            if(r==cr)
            {
                getRateChooser()->setSelectedId(r,true);
            }
        }
    }
}

void AudioDialogComponent::raw_refresh_sizes()
{
    pi_audio::audioctl_t::impl_t *root = settings_->root_;
    juce::AudioIODevice *device = root->device_;

    getBufferChooser()->clear(true);

    if(!device)
    {
        return;
    }

    int cs = device->getCurrentBufferSizeSamples();
    pic::logmsg() << "current size is " << cs;
    bool cok = false;

    int minb = 0;

    if(device->getTypeName()=="Windows Audio")
    {
        minb = device->getDefaultBufferSize();
    }

    Array<int> abs = device->getAvailableBufferSizes();
    for(int j=0;j<abs.size();j++)
    {
        int b = abs[j];
        pic::logmsg() << "available size " << b;

        if(b>=minb && filter_buffer_size(b))
        {
            getBufferChooser()->addItem(juce::String(b),b);

            if(b==cs)
            {
                getBufferChooser()->setSelectedId(b,true);
                cok = true;
            }
        }
    }

    if(!cok)
    {
        getBufferChooser()->addItem(juce::String(cs),cs);
        getBufferChooser()->setSelectedId(cs,true);
    }
}

void AudioDialogComponent::raw_refresh_control()
{
    pi_audio::audioctl_t::impl_t *root = settings_->root_;
    juce::AudioIODevice *device = root->device_;

    if(device && device->hasControlPanel())
    {
        getControlButton()->setEnabled(true);
    }
    else
    {
        getControlButton()->setEnabled(false);
    }
}

void AudioDialogComponent::raw_open_device(const juce::String &uid,unsigned long sr, unsigned bs)
{
    if(settings_ && settings_->root_)
    {
        settings_->root_->open_device(uid, sr, bs);
    }
}

void AudioDialogComponent::buttonClicked(Button* buttonThatWasClicked)
{
    pi_audio::audioctl_t::impl_t *root = settings_->root_;

    if(buttonThatWasClicked == getControlButton())
    {
        juce::AudioIODevice *device = root->device_;

        if(device->hasControlPanel())
        {
            device->showControlPanel();
        }

        return;
    }

    if(buttonThatWasClicked == getOkButton())
    {
        unsigned long sr = getRateChooser()->getSelectedId();
        unsigned bs = getBufferChooser()->getSelectedId();
        int dti = getTypeChooser()->getSelectedItemIndex();
        int dni = getDeviceChooser()->getSelectedItemIndex();

        if(dti>=0 && dni>0)
        {
            juce::String uid = getTypeChooser()->getItemText(dti)+"/"+getDeviceChooser()->getItemText(dni);
            root->delegate_->device_changed(uid.toUTF8(),sr,bs);
        }

        settings_->closeButtonPressed();
        return;
    }
}

void AudioDialogComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    pi_audio::audioctl_t::impl_t *root = settings_->root_;
    juce::AudioIODevice *device = root->device_;

    if(comboBoxThatHasChanged == getTypeChooser())
    {
        refresh_devices();
        return;
    }

    if(comboBoxThatHasChanged == getDeviceChooser())
    {
        int dti = getTypeChooser()->getSelectedItemIndex();
        int dni = getDeviceChooser()->getSelectedItemIndex();

        if(dti>=0 && dni>0)
        {
            juce::String uid = getTypeChooser()->getItemText(dti)+"/"+getDeviceChooser()->getItemText(dni);
            open_device(uid,0,0);
        }

        refresh_rates();
        refresh_sizes();
        refresh_control();
        return;
    }

    if(comboBoxThatHasChanged == getRateChooser())
    {
        if(device)
        {
            unsigned long sr = getRateChooser()->getSelectedId();
            unsigned bs = getBufferChooser()->getSelectedId();
            open_device(root->get_uid(),sr,bs);
            refresh_sizes();
        }
        
        return;
    }

    if(comboBoxThatHasChanged == getBufferChooser())
    {
        if(device)
        {
            unsigned long sr = getRateChooser()->getSelectedId();
            unsigned bs = getBufferChooser()->getSelectedId();
            open_device(root->get_uid(),sr,bs);
        }
        
        return;
    }
}

SettingsDialog::SettingsDialog(pi_audio::audioctl_t::impl_t *root): juce::DocumentWindow("Audio Settings",juce::Colours::black,juce::DocumentWindow::closeButton,true), root_(root), component_(this)
{
    setUsingNativeTitleBar(true);
    setContentOwned(&component_,true);
    centreAroundComponent(&component_,getWidth(),getHeight());
    setVisible(true);
    setTopLeftPosition(150,150);
    setResizable(false,false);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::closeButtonPressed()
{
    root_->window_state(false);
}

void pi_audio::audioctl_t::impl_t::wire_closed()
{
    unsubscribe();
    piw::wire_t::disconnect();
}

void pi_audio::audioctl_t::impl_t::add_type(const juce::String &tn, juce::AudioIODeviceType *tp)
{
    if(tp)
    {
        types_.insert(std::make_pair(tn,tp));
    }
}

juce::String pi_audio::audioctl_t::impl_t::find_name(juce::AudioIODevice *device)
{
    std::map<juce::String,juce::AudioIODeviceType *>::iterator i;
    juce::String t = device->getTypeName();

    for(i=types_.begin(); i!=types_.end(); i++)
    {
        if(i->second->getTypeName()==t)
        {
            return i->first;
        }
    }

    return "";
}

juce::AudioIODeviceType *pi_audio::audioctl_t::impl_t::find_type(const juce::String &dt)
{
    std::map<juce::String,juce::AudioIODeviceType *>::iterator i;

    if((i=types_.find(dt))!=types_.end())
    {
        return i->second;
    }

    return 0;

}

void pi_audio::audioctl_t::impl_t::enumerate()
{
    std::map<juce::String,juce::AudioIODeviceType *>::iterator i;

    devices_.clear();

    for(i=types_.begin(); i!=types_.end(); i++)
    {
        i->second->scanForDevices();
        const juce::StringArray &devices = i->second->getDeviceNames();

        for(int d=0; d<devices.size(); ++d)
        {
            juce::String dn = devices[d];
            pic::logmsg() <<  "** type " << i->first << " dev " << dn;
            juce::String dc = i->first+"/"+dn;
            devices_.push_back(dc);
        }
    }
}

pi_audio::audioctl_t::impl_t::impl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d, const std::string &dn, pi_audio::audioctl_t *del):
    piw::decoder_t(this),
    running_(false),
    changing_(false),
    delegate_(del),
    up_(0), 
    mute_(false),
    last_time_(0),
    elapsed_(0),
    lastlog_(0),
    count_(0),
    alldropout_(0),
    resetdropout_(0),
    enabled_(0),
    device_(0),
    settings_(0),
    audio_output_(new audio_output_t(this, o))
{
    piw::tsd_thing(this);
    piw::tsd_clocksource(piw::makestring("juceaudio"),512,48000,this);

    add_type("Core Audio",juce::AudioIODeviceType::createAudioIODeviceType_CoreAudio());
    add_type("ALSA",juce::AudioIODeviceType::createAudioIODeviceType_ALSA());
    add_type("Jack",juce::AudioIODeviceType::createAudioIODeviceType_JACK());
    add_type("DirectX 10",juce::AudioIODeviceType::createAudioIODeviceType_WASAPI(false));
    add_type("ASIO",juce::AudioIODeviceType::createAudioIODeviceType_ASIO());
    //add_type("DirectX 9",juce::AudioIODeviceType::createAudioIODeviceType_DirectSound());

    d->set_source(piw::makestring("juceaudio"));
    d->sink(this,"coreaudio");

    enumerate();
    timer_fast(10U);

    piw::tsd_window(&settings_window_);
    settings_window_.set_state_handler(pic::status_t::method(this,&pi_audio::audioctl_t::impl_t::window_state));
    settings_window_.set_window_title("Audio Settings");
    settings_window_.set_window_state(false);
}

void pi_audio::audioctl_t::impl_t::window_state(bool state)
{
    pic::logmsg() << "window state: " << state << ' ' << (void *)settings_;

    if(state)
    {
        if(!settings_)
        {
            settings_window_.set_window_state(true);
            settings_ = new SettingsDialog(this);
        }
    }
    else
    {
        if(settings_)
        {
            settings_window_.set_window_state(false);
            delete settings_;
            settings_ = 0;
        }
    }
}

pi_audio::audioctl_t::impl_t::~impl_t()
{
    window_state(false);
    settings_window_.close_window();
    cancel_timer_fast();
    tracked_invalidate();
    close_device();
}

juce::String pi_audio::audioctl_t::impl_t::get_uid()
{
    return current_uid_.toUTF8();
}

void pi_audio::audioctl_t::impl_t::set_clock(bct_clocksink_t *s)
{
    if(up_)
    {
        remove_upstream(up_);
        up_ = 0;
    }
    if(s)
    {
        up_ = s;
        add_upstream(s);
    }
}

void pi_audio::audioctl_t::impl_t::refresh_gui()
{
    if(settings_)
    {
        settings_->refresh();
    }
}

void pi_audio::audioctl_t::impl_t::thing_trigger_slow()
{
    if(device_ && !running_ && !changing_)
    {
        float sr = device_->getCurrentSampleRate();
        unsigned bs = device_->getCurrentBufferSizeSamples();
        juce::String uid(current_uid_);
        pic::logmsg() << "restarting " << std::string(uid.getCharPointer()) << " sr=" << sr << " bs=" << bs;

        close_device();
        open_device(uid,sr,bs);
        refresh_gui();
    }

    enumerate();

    if(enabled_)
    {
        delegate_->device_list_changed();
    }
}

void pi_audio::audioctl_t::impl_t::thing_dequeue_fast(const piw::data_nb_t &d)
{
    unsigned long long t = d.time();
    if(d.as_long()<6666)
    {
        //pic::logmsg() << "short tick " << d.as_long();
    }
    if(d.as_long()>14666)
    {
        //pic::logmsg() << "long tick " << d.as_long();
    }

    clocksource_t::tick(t);

    if(!iterator_.isvalid())
    {
        return;
    }

    piw::data_nb_t d2;
    for(unsigned s = 1; s <= MAX_CHANNEL; ++s)
    {
        if(iterator_->nextsig(s,d2,t))
        {
            audio_in_[s-1].send(d2);
        }
    }
}

void pi_audio::audioctl_t::impl_t::thing_timer_fast()
{
    if(device_ && running_ && !changing_)
    {
        if(!device_->isOpen() || !device_->isPlaying() || runaway_)
        {
            pic::logmsg() << "output device stalled";
            running_ = false;
            trigger_slow();
        }
    }

    if(!device_ || !running_)
    {
        unsigned long long t = piw::tsd_time();
        thing_dequeue_fast(piw::makelong_nb(10000,t));
    }
}

int pi_audio::audioctl_t::impl_t::__mute(void *r_, void *m_)
{
    pi_audio::audioctl_t::impl_t *r = (pi_audio::audioctl_t::impl_t *)r_;
    r->mute_= *(bool *)m_;

    return 0;
}

bool pi_audio::audioctl_t::impl_t::open_device(const juce::String &requested_uid, unsigned long requested_sr, unsigned requested_bs)
{
    juce::String uid = requested_uid;

    if(uid.isEmpty())
    {
        uid = default_uid();
    }

    juce::String dt,odn,idn,e;
    unsigned bs = requested_bs;
    unsigned long sr = requested_sr;
    int i;

    if((i=uid.indexOf("/")) > 0)
    {
        dt = uid.substring(0,i);
        odn = uid.substring(i+1);
    }
    else
    {
        dt = uid;
    }

    pic::logmsg() << "opening device: " << requested_uid << " -> " << uid << ":" << odn;

    juce::AudioIODeviceType *t = find_type(dt);

    if(!t)
    {
        pic::logmsg() << "invalid device type: " << dt << " using default";

        dt = default_uid();
        odn = juce::String::empty;

        t = find_type(dt);

        if(!t)
        {
            pic::logmsg() << "invalid device type: " << dt;
            return false;
        }
    }

    t->scanForDevices();
    const juce::StringArray &d = t->getDeviceNames(false);

    if(d.size()==0)
    {
        pic::logmsg() << "no devices of type: " << dt;
        return false;
    }

    if(odn.isEmpty() || !d.contains(odn))
    {
        odn = d[t->getDefaultDeviceIndex(false)];
        pic::logmsg() << "using default device of type: " << dt << " = " << odn;
    }

    idn=odn;
    pic::logmsg() << "opening device " << odn << ':' << idn << " type " << dt;

    uid = dt+"/"+odn;

    if(!device_ || current_uid_ != uid)
    {
        close_device();
        device_ = t->createDevice(odn,idn);

        if(!device_)
        {
            pic::logmsg() << "Can't create device";
            return false;
        }

        current_uid_ = uid;
    }

    bool sok=false;

    Array<double> asr = device_->getAvailableSampleRates();
    for(i=0;i<asr.size();i++)
    {
        if(filter_sample_rate(asr[i]))
        {
            unsigned long dsr = asr[i];

            if(!sr)
            {
                sr=dsr;
            }

            if(dsr==sr)
            {
                sok=true;
                break;
            }
        }
    }

    if(!sok)
    {
        close_device();
        pic::logmsg() << "No Supported Sample rates";
        return false;
    }

    juce::BigInteger oc;
    juce::BigInteger ic;
    juce::StringArray ocn = device_->getOutputChannelNames();
    juce::StringArray icn = device_->getInputChannelNames();
    oc.setRange(0,ocn.size(),true);
    ic.setRange(0,icn.size(),true);

    delegate_->available_channels_changed(ocn.size(),icn.size());

    if(sr != device_->getCurrentSampleRate())
    {
        e = device_->open(ic,oc,sr,0);

        if(!e.isEmpty())
        {
            close_device();
            pic::logmsg() << "warning: cannot open output device " << uid << " sr=" << sr << " bs=" << 0 << ": "<< e;
            return false;
        }
    }


    if(!bs)
    {
        bs = device_->getDefaultBufferSize();
    }

    float req_sr = sr;
    unsigned req_bs = bs;

    while(true)
    {
        device_->close();

        e = device_->open(ic,oc,sr,bs);

        if(!e.isEmpty())
        {
            close_device();
            pic::logmsg() << "warning: cannot open output device " << uid << " sr=" << sr << " bs=" << bs << ": "<< e;
            return false;
        }


        float new_sr = device_->getCurrentSampleRate();
        unsigned new_bs = device_->getCurrentBufferSizeSamples();

        if(new_sr == sr && new_bs == bs)
        {
            break;
        }

        bs = new_bs;
        sr = new_sr;
    }

    set_details(bs,sr);
    runaway_ = false;
    device_->start(this);
    changing_ = false;
    running_ = true;

    pic::logmsg() << "opened " << std::string(current_uid_.getCharPointer()) << " sr=" << sr << ':' << req_sr << " bs=" << bs << ':' << req_bs << " out channels=" << ocn.size() << " in channels=" << icn.size();

    return true;
}

static int set_output_channels__(void *self_, void *channels_)
{
    pi_audio::audioctl_t::impl_t *impl = (pi_audio::audioctl_t::impl_t *)self_;
    piw::data_t channels = piw::data_t::from_given((bct_data_t)channels_);

    impl->audio_outputs_state_.clear();
    for(unsigned i=0; i<channels.as_tuplelen(); ++i)
    {
        unsigned channel = (unsigned)channels.as_tuple_value(i).as_long()-1;
        impl->audio_outputs_state_.insert(std::pair<unsigned,bool>(channel,true));
    }
    return 0;
}

void pi_audio::audioctl_t::impl_t::set_output_channels(piw::data_t channels)
{
    piw::tsd_fastcall(set_output_channels__,(void *)this,(void *)channels.give());
}

void pi_audio::audioctl_t::impl_t::enable_callbacks(bool e)
{
    enabled_ = e;

    if(enabled_)
    {
        trigger_slow();
    }
}

void pi_audio::audioctl_t::impl_t::audioDeviceIOCallback(const float** inputChannelData, int numInputChannels, float** outputChannelData, int numOutputChannels, int numSamples)
{
    int ii=0,ij=0;
    bool dropout=false;


    unsigned long long t = ctx_.time();

    while(ii<numOutputChannels && ij<MAX_CHANNEL)
    {
        if(!outputChannelData[ii])
        {
            ii++;
            continue;
        }

        if(!mute_ && !audio_in_[ij].is_empty())
        {
            piw::data_nb_t d = audio_in_[ij].read_all(); 
            bool dok = (d.is_array() && d.as_arraylen()==(unsigned)numSamples);

            if(dok)
            {
                memcpy(outputChannelData[ii],d.as_array(),numSamples*sizeof(float));
            }
            else if(iterator_.isvalid())
            {
                dropout=true;
            }
        }
        else
        {
            memset(outputChannelData[ii],0,numSamples*sizeof(float));
        }

        ii++;
        ij++;
    }

    int oi=0;
    while(oi<numInputChannels && oi<MAX_CHANNEL)
    {
        if(!inputChannelData[oi])
        {
            oi++;
            continue;
        }

        float *f,*fs;
        bct_data_t d = bct_entity_allocate_host(ctx_.entity(),PIC_ALLOC_NB,t,1,-1,0,BCTVTYPE_FLOAT,sizeof(float),(unsigned char **)&fs,numSamples,&f);
#ifdef DEBUG_DATA_ATOMICITY
        d->tid = 0;
#endif
        memcpy(f,inputChannelData[oi],numSamples*sizeof(float));

        audio_out_[oi].send_raw(&d);

        oi++;
    }

    long *dp;
    piw::data_nb_t d = ctx_.allocate_host(t,1,0,0,BCTVTYPE_INT,sizeof(long),(unsigned char **)&dp,0,0);
    long long us = pic_microtime();
    *dp=(long)(us-last_time_);//(lok?0:2)|(rok?0:1);
    last_time_ = us;

    /*
    if(count_%100==0)
    {
        pic::logmsg() << "audio io: in=" << numInputChannels << " out=" << numOutputChannels;
    }
    */

    if(resetdropout_)
    {
        resetdropout_ = 0;
        alldropout_ = 0;
    }

    if(count_%10==0)
    {
        unsigned long long avg = (t-elapsed_)/10;

        if(elapsed_>0)
        {
            if(t-lastlog_ > 10000000)
            {
                pic::logmsg() << "total audio dropouts " << alldropout_ <<  " average tick " << avg;
                lastlog_ = t;
            }

#if JUCE_WINDOWS
            if(!runaway_ && avg < 1000)
            {
                pic::logmsg() << "runaway device: shutting down audio";
                runaway_ = true;
            }
#endif
        }

        elapsed_=t;
    }

    if(!runaway_)
        enqueue_fast(d,1);

    if(dropout)
    {
        pic::logmsg() << "audio dropout at count " << count_;
        alldropout_++;
        count_ = 0;
    }
    else
    {
        count_++;
    }
}

void pi_audio::audioctl_t::impl_t::close_device()
{
    if(device_)
    {
        changing_=true;
        device_->stop();
        device_->close();
        delete device_;
        device_=0;
        current_uid_ = "";
    }
}

piw::wire_t *pi_audio::audioctl_t::impl_t::wire_create(const piw::event_data_source_t &es)
{
    subscribe(es);
    return this;
}

void pi_audio::audioctl_t::impl_t::event_start(unsigned seq,const piw::data_nb_t &id, const piw::xevent_data_buffer_t &ei)
{
    iterator_=ei.iterator();
    for(unsigned s = 1; s <= MAX_CHANNEL; ++s)
    {
        iterator_->reset(s,id.time());
    }
}

bool pi_audio::audioctl_t::impl_t::event_end(unsigned long long t)
{
    iterator_.clear();
    return true;
}

void pi_audio::audioctl_t::impl_t::event_buffer_reset(unsigned sig,unsigned long long t,const piw::dataqueue_t &o,const piw::dataqueue_t &n)
{
    iterator_->set_signal(sig,n);
    iterator_->reset(sig,t);
}

unsigned pi_audio::audioctl_t::impl_t::numdevices()
{
    return devices_.size();
}

std::string pi_audio::audioctl_t::impl_t::device_name(unsigned index)
{
    if(index<devices_.size())
    {
        juce::String d = devices_[index];
        int i = d.indexOf("/");

        if(i>0)
        {
            juce::String dt = d.substring(0,i);
            juce::String dn = d.substring(i+1);
            juce::String n = dn+" ("+dt+")";
            return std::string(n.getCharPointer());
        }
    }

    return "";
}

std::string pi_audio::audioctl_t::impl_t::device_uid(unsigned index)
{     
    if(index<devices_.size())
    {
        return std::string(devices_[index].getCharPointer());
    }

    return "";
}

juce::String pi_audio::audioctl_t::impl_t::default_uid()
{
#if JUCE_MAC
    return "Core Audio";
#endif
#if JUCE_WINDOWS
    return "DirectX 10";
#endif
#if JUCE_LINUX
    return "ALSA";
#endif
}

pi_audio::audioctl_t::audioctl_t(const piw::cookie_t &o, piw::clockdomain_ctl_t *d, const std::string &domain_name) : impl_(new impl_t(o,d,domain_name,this))
{
}

pi_audio::audioctl_t::~audioctl_t()
{
    impl_->enable_callbacks(false);
    delete impl_;
}

piw::cookie_t pi_audio::audioctl_t::cookie()
{
    PIC_ASSERT(impl_);
    return impl_->cookie();
}

bool pi_audio::audioctl_t::open_device(const std::string &uid, unsigned long sr, unsigned bs, bool cb)
{
    PIC_ASSERT(impl_);
    juce::String juid = juce::String::fromUTF8(uid.c_str());
    bool e = impl_->open_device(juid,sr,bs);
    impl_->refresh_gui();

    if(e && cb)
    {
        impl_->delegate_->device_changed(uid.c_str(),sr,bs);
    }

    return e;
}

std::string pi_audio::audioctl_t::device_name(unsigned i)
{
    PIC_ASSERT(impl_);
    return impl_->device_name(i);
}

std::string pi_audio::audioctl_t::device_uid(unsigned i)
{
    PIC_ASSERT(impl_);
    return impl_->device_uid(i);
}

unsigned pi_audio::audioctl_t::num_devices()
{
    PIC_ASSERT(impl_);
    return impl_->numdevices();
}

void pi_audio::audioctl_t::close_device()
{
    if(impl_)
    {
        impl_->close_device();
    }
}

void pi_audio::audioctl_t::mute()
{
    PIC_ASSERT(impl_);
    bool mute = true;
    piw::tsd_fastcall(pi_audio::audioctl_t::impl_t::__mute,impl_,&mute);
}

void pi_audio::audioctl_t::unmute()
{
    PIC_ASSERT(impl_);
    bool mute = false;
    piw::tsd_fastcall(pi_audio::audioctl_t::impl_t::__mute,impl_,&mute);
}

std::string pi_audio::audioctl_t::get_uid()
{
    return std::string(impl_->get_uid().getCharPointer());
}

unsigned long pi_audio::audioctl_t::get_dropout_count()
{
    return impl_->alldropout_;
}

void pi_audio::audioctl_t::enable_callbacks(bool e)
{
    impl_->enable_callbacks(e);
}

void pi_audio::audioctl_t::reset_dropout_count()
{
    pic_atomiccas(&impl_->resetdropout_,0,1);
}

void pi_audio::audioctl_t::show_gui(bool s)
{
    impl_->window_state(s);
}

void pi_audio::audioctl_t::set_output_channels(piw::data_t channels)
{
    impl_->set_output_channels(channels);
}
