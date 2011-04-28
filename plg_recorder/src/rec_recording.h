
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

#ifndef __RECORDING__
#define __RECORDING__
#include <plg_recorder/pirecorder_exports.h>
#include <picross/pic_ref.h>
#include <picross/pic_fastalloc.h>
#include <piw/piw_data.h>
#include <piw/piw_address.h>
#include <list>
#include <map>
#include <vector>
#include <set>

#define TAG_BAR_DURATION 1
#define TAG_START_BEAT 2
#define TAG_START_SONG_BEAT 3
#define TAG_BEAT_DURATION 4
#define TAG_BEAT_DELTA 5
#define TAG_SCHEMA 100

namespace recorder
{
    struct PIRECORDER_DECLSPEC_CLASS sample_t : virtual pic::lckobject_t
    {
        sample_t(unsigned long long t, float b, unsigned s, const piw::data_nb_t &v) : time(t), beat(b), signal(s), value(v) {}

        unsigned long long time;
        float beat;
        unsigned signal;
        piw::data_nb_t value;
    };

    typedef pic::lcklist_t<sample_t>::nbtype samplelist_t;

    struct PIRECORDER_DECLSPEC_CLASS event_t: virtual pic::atomic_counted_t, virtual pic::lckobject_t
    {
        event_t(unsigned long long t, float b, const piw::data_nb_t &v): time(t), beat(b), max_time(0), max_beat(0), value(v) , iscomplete_(false){}

        unsigned long long time;
        float beat;
        unsigned long long max_time;
        float max_beat;

        piw::data_nb_t value;

        bool iscomplete_;
        samplelist_t samples_;
    };

    typedef pic::lcklist_t<pic::ref_t<event_t> >::nbtype eventlist_t;
    typedef pic::lckmap_t<unsigned char, piw::data_nb_t>::nbtype taglist_t;

    struct PIRECORDER_DECLSPEC_CLASS recording_data_t: virtual pic::atomic_counted_t, virtual pic::lckobject_t
    {
        recording_data_t(unsigned signals, unsigned wires);

        unsigned signals_;
        unsigned wires_;
        eventlist_t events_;
        taglist_t tags_;
        float current_beat_;
        unsigned long long current_time_;
    };

    class PIRECORDER_DECLSPEC_CLASS readevent_t : virtual public pic::lckobject_t
    {
        public:
            readevent_t() {}
            readevent_t(const pic::ref_t<event_t> &e): event_(e) { reset(); }
            readevent_t(const readevent_t &e): event_(e.event_) { reset(); }
            readevent_t &operator=(const readevent_t &e) { event_=e.event_;  reset(); return *this; }

            piw::data_nb_t evt_id() const { return event_->value; }
            unsigned long long evt_time() const { return event_->time; }
            float evt_beat() const { return event_->beat; }
            unsigned long long evt_max_time() const { return event_->max_time; }
            float evt_max_beat() const { return event_->max_beat; }

            bool complete() const { return event_->iscomplete_; }

            unsigned long long cur_time() const { return current_sample_->time; }
            float cur_beat() const { return current_sample_->beat; }
            piw::data_nb_t cur_value() const { return current_sample_->value; }
            unsigned cur_signal() const { return current_sample_->signal; }

            bool isvalid() const;
            void reset();
            void next();
            void clear();

        private:
            pic::ref_t<event_t> event_;
            samplelist_t::const_iterator current_sample_;
    };

    class PIRECORDER_DECLSPEC_CLASS writeevent_t : virtual public pic::lckobject_t
    {
        public:
            writeevent_t() {}
            writeevent_t(const pic::ref_t<event_t> &e): event_(e) {}
            writeevent_t(const writeevent_t &e): event_(e.event_) {}
            writeevent_t &operator=(const writeevent_t &e) { event_=e.event_; return *this; }
            void add_value(unsigned long long time, float beat, unsigned signal, const piw::data_nb_t &d);
            void end_event(unsigned long long time, float beat);
        private:
            pic::ref_t<event_t> event_;
    };

    typedef pic::ref_t<recording_data_t> dataref_t;

    class PIRECORDER_DECLSPEC_CLASS recording_t: virtual public pic::lckobject_t
    {
        public:
            recording_t();
            recording_t(const recording_t &);
            recording_t &operator=(const recording_t &);
            recording_t(const dataref_t &);

            unsigned signals() const;
            unsigned wires() const;

            unsigned events() const { return isvalid() ? data_->events_.size() : 0; }

            bool isvalid() const;
            void reset();
            void next();

            bool operator==(const recording_t &r) const { return data_==r.data_; }

            piw::data_t get_tag(unsigned char tag) const;

            readevent_t cur_event() const;
            void write(const char *name) const;

        private:
            dataref_t data_;
            eventlist_t::const_iterator current_event_;
            pic::ref_t<event_t> event_;
    };

    PIRECORDER_DECLSPEC_FUNC(recording_t) read(const char *name);
    PIRECORDER_DECLSPEC_FUNC(recording_t) read_meta(const char *name);

    class PIRECORDER_DECLSPEC_CLASS recordmaker_t: virtual public pic::lckobject_t
    {
        public:
            recordmaker_t();

            void new_recording(unsigned signals, unsigned wires);
            void set_tag(unsigned char tag, const piw::data_nb_t &d);
            writeevent_t new_event(unsigned long long time, float beat, const piw::data_nb_t &d);
            recording_t get_recording();
            void set_time(float beat, unsigned long long time) { data_->current_beat_=beat; data_->current_time_=time; }

        private:
            dataref_t data_;
    };
}

#endif

