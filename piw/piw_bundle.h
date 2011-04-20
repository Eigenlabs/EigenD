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

#ifndef __PIW_BUNDLE__
#define __PIW_BUNDLE__

#include "piw_exports.h"

#include "piw_data.h"
#include "piw_dataqueue.h"
#include "piw_tsd.h"
#include <picross/pic_nocopy.h>
#include <picross/pic_weak.h>
#include <picross/pic_ref.h>
#include <picross/pic_stl.h>

#define MAX_SIGNALS 64

namespace piw
{
    class fastdata_t;
    class clocksink_t;

    class wire_t;
    class wire_ctl_t;
    class root_t;
    class root_ctl_t;
    class cookie_t;

    class event_data_sink_t;

    class xevent_data_buffer_t;

    class PIW_DECLSPEC_CLASS event_data_t: public pic::atomic_counted_t, public pic::lckobject_t, public pic::nocopy_t
    {
        private:
            friend class evtiterator_t;
            friend class xevent_data_buffer_t;
            dataqueue_t signals_[MAX_SIGNALS];
    };

    class PIW_DECLSPEC_CLASS evtiterator_t: public pic::atomic_counted_t, public pic::lckobject_t, public pic::nocopy_t
    {
        public:
            evtiterator_t(const pic::ref_t<event_data_t> &e);
            bool next(unsigned long long mask,unsigned &sig,piw::data_nb_t &data,unsigned long long t);
            bool latest(unsigned sig,piw::data_nb_t &data,unsigned long long t);
            bool nextsig(unsigned sig,piw::data_nb_t &data,unsigned long long t);
            void reset(unsigned sig,unsigned long long t);
            void reset_all(unsigned long long t);
            void set_signal(unsigned s,const dataqueue_t &q) { event_->signals_[s-1]=q; }
            void clear_signal(unsigned s) { event_->signals_[s-1].clear(); }
            void dump(unsigned sig,bool full);
            void dump_index();
            bool isvalid(unsigned s) const { return event_->signals_[s-1].isvalid(); }

        private:
            unsigned long long index_[MAX_SIGNALS];
            pic::ref_t<event_data_t> event_;
    };

    class PIW_DECLSPEC_CLASS xevent_data_buffer_t: public pic::lckobject_t
    {
        public:
            xevent_data_buffer_t(): event_(pic::ref(new event_data_t)) {}
            xevent_data_buffer_t(unsigned long long mask,unsigned size);

            inline void add_value(unsigned sig,const piw::data_nb_t &d) { event_->signals_[sig-1].write_fast(d); }
            inline void add_value(unsigned sig,const piw::data_t &d) { event_->signals_[sig-1].write_slow(d); }
            inline dataqueue_t signal(unsigned sig) const { return event_->signals_[sig-1]; }

            void merge(const xevent_data_buffer_t &,unsigned long long);
            void set_signal(unsigned s,const dataqueue_t &q) { event_->signals_[s-1]=q; }
            bool isvalid(unsigned s) const { return signal(s).isvalid(); }

            typedef pic::ref_t<evtiterator_t> iter_t;
            inline iter_t iterator() const { return pic::ref(new evtiterator_t(event_)); }

            void dump(bool full) const;

            inline bool latest(unsigned sig,piw::data_nb_t &d,unsigned long long t) { return event_->signals_[sig-1].latest(d,0,t); }

        private:
            pic::ref_t<event_data_t> event_;
    };

    class event_data_source_real_t;

    class PIW_DECLSPEC_CLASS evtsource_data_t: public pic::atomic_counted_t, virtual public pic::lckobject_t
    {
        public:
            evtsource_data_t(event_data_source_real_t *src, const piw::data_t &path);
            ~evtsource_data_t();

            void event_start(unsigned seq,const piw::data_nb_t &id,const xevent_data_buffer_t &init); // fast
            bool event_end(unsigned long long t); // fast
            void event_buffer_reset(unsigned sig, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n); // fast

        private:
            friend class event_data_source_t;
            friend class event_data_source_real_t;
            friend class event_data_sink_t;

            void event_ended(unsigned seq); // fast

            pic::lcklist_t<event_data_sink_t *>::nbtype clients_; // fast
            pic::flipflop_t<event_data_source_real_t *> source_;
            xevent_data_buffer_t buffer_;
            piw::dataholder_nb_t event_;
            piw::data_t path_;
            unsigned seq_internal_;
            unsigned seq_current_;
            unsigned lingering_count_;
    };

    class PIW_DECLSPEC_CLASS event_data_source_t
    {
        private:
            inline event_data_source_t(const pic::ref_t<evtsource_data_t> &list): list_(list) {}

        public:
            inline event_data_source_t(const event_data_source_t &ds) { *this=ds; }
            inline event_data_source_t &operator=(const event_data_source_t &ds) { list_=ds.list_;  return *this; }

            inline piw::data_t path() const { return list_->path_; }
            inline piw::data_nb_t event() const { return list_->event_; }

        private:
            friend class event_data_sink_t;
            friend class event_data_source_real_t;
            pic::ref_t<evtsource_data_t> list_;
    };

    class PIW_DECLSPEC_CLASS event_data_source_real_t: public pic::nocopy_t
    {
        public:
            event_data_source_real_t(const piw::data_t &path);
            inline virtual ~event_data_source_real_t() { source_end(piw::tsd_time()); source_shutdown(); };

            inline event_data_source_t source() { return event_data_source_t(list_); }

            inline piw::data_t path() const { return list_->path_; }
            inline piw::data_nb_t event() const { return list_->event_; }

            inline void source_start(unsigned seq, const piw::data_nb_t &id,const xevent_data_buffer_t &b) { list_->event_start(seq,id,b); } // fast
            inline bool source_end(unsigned long long t) { return list_->event_end(t); } // fast
            inline void source_buffer_reset(unsigned s, unsigned long long t, const piw::dataqueue_t &o, const piw::dataqueue_t &n) { list_->event_buffer_reset(s,t,o,n); } // fast
            void source_shutdown();
            void start_slow(const piw::data_t &id,const xevent_data_buffer_t &init); // slow
            void end_slow(unsigned long long t); // slow

            virtual void source_ended(unsigned seq) {}

        private:
            friend class event_data_sink_t;
            pic::ref_t<evtsource_data_t> list_;
    };

    class PIW_DECLSPEC_CLASS event_data_sink_t: virtual public pic::lckobject_t
    {
        public:
            inline event_data_sink_t(): seq_outstanding_(0), started_(false), lingering_(false) {}
            inline virtual ~event_data_sink_t() {}

            void subscribe(const event_data_source_t &s);
            void subscribe_and_ping(const event_data_source_t &s);
            void unsubscribe(); //slow

            void subscribe_fast(const event_data_source_t &s);
            void subscribe_and_ping_fast(const event_data_source_t &s);
            void unsubscribe_fast();
            void event_ended(unsigned seq);

            virtual void event_start(unsigned seq, const piw::data_nb_t &id,const xevent_data_buffer_t &b) {};
            virtual bool event_end(unsigned long long) { return true; };
            virtual void event_buffer_reset(unsigned,unsigned long long, const piw::dataqueue_t &,const piw::dataqueue_t &) {};

            piw::data_nb_t current_id(); // fast
            xevent_data_buffer_t current_data(); // fast

        private:
            friend class evtsource_data_t;

            static int subscribe__(void *,void *);
            static int subscribe_and_ping__(void *,void *);
            static int unsubscribe__(void *,void *);

            pic::ref_t<evtsource_data_t> subject_;
            unsigned seq_outstanding_;
            bool started_;
            bool lingering_;
    };

    class PIW_DECLSPEC_CLASS wire_t: public pic::nocopy_t
    {
        private:
            friend class wire_ctl_t;
        public:
            wire_t();
            void disconnect();
            virtual ~wire_t();
        protected:
            virtual void wire_closed() = 0;
        private:
            wire_ctl_t *_ctl;
    };

    class PIW_DECLSPEC_CLASS wire_ctl_t: public pic::nocopy_t
    {
        private:
            friend class wire_t;
            friend class root_ctl_t;
        public:
            wire_ctl_t();
            virtual ~wire_ctl_t();
            void disconnect();
            void set_wire(wire_t *);
            inline wire_t *get_wire() { return _wire; }
        private:
            wire_t *_wire;
    };

    class PIW_DECLSPEC_CLASS decode_ctl_t: public pic::counted_t
    {
        public:
            inline decode_ctl_t() {}
            inline virtual ~decode_ctl_t() {}
            virtual wire_t *wire_create(const piw::event_data_source_t &) = 0;
            virtual void set_clock(bct_clocksink_t *) = 0;
            virtual void set_latency(unsigned l) = 0;
    };

    class PIW_DECLSPEC_CLASS root_t: public pic::nocopy_t, virtual public pic::tracked_t
    {
        private:
            friend class root_ctl_t;
        public:
            root_t(unsigned latency);
            bct_clocksink_t *get_clock();
            void disconnect();
            unsigned get_latency();
            virtual ~root_t();
        protected:
            virtual wire_t *root_wire(const piw::event_data_source_t &) = 0;
            virtual void root_closed() = 0;
            virtual void root_clock() = 0;
            virtual void root_opened() = 0;
            virtual void root_latency() = 0;
        private:
            root_ctl_t *_ctl;
            unsigned latency_;
    };

    class PIW_DECLSPEC_CLASS root_ctl_t: public pic::nocopy_t
    {
        private:
            friend class root_t;
        public:
            root_ctl_t();
            ~root_ctl_t();
            void connect(const cookie_t &);
            void connect_wire(wire_ctl_t *, const piw::event_data_source_t &);
            void set_clock(bct_clocksink_t *);
            void disconnect();
            void set_latency(unsigned latency);
            inline unsigned ctl_latency() { return latency_; }
        private:
            root_t *_root;
            bct_clocksink_t *_clock;
            unsigned latency_;
    };

    class PIW_DECLSPEC_CLASS cookie_t
    {
        private:
            friend class root_t;
            friend class root_ctl_t;
        public:
            cookie_t(root_t *b = 0);
        protected:
            pic::weak_t<root_t> _root;
    };

    class PIW_DECLSPEC_CLASS decoder_t: public root_t
    {
        public:
            decoder_t(decode_ctl_t *ctl);
            ~decoder_t();
            inline cookie_t cookie() { return cookie_t(this); }
            void shutdown();
            unsigned wire_count();

            class impl_t;
        private:
            void root_closed();
            void root_clock();
            void root_opened();
            void root_latency();
            wire_t *root_wire(const piw::event_data_source_t &);
            impl_t *impl_;
    };
};

#define SIG1(s1) (1ULL<<(s1-1))
#define SIG2(s1,s2) (SIG1((s1))|SIG1((s2)))
#define SIG3(s1,s2,s3) (SIG2((s1),(s2))|SIG1((s3)))
#define SIG4(s1,s2,s3,s4) (SIG3((s1),(s2),(s3))|SIG1((s4)))
#define SIG5(s1,s2,s3,s4,s5) (SIG4((s1),(s2),(s3),(s4))|SIG1((s5)))
#define SIG6(s1,s2,s3,s4,s5,s6) (SIG5((s1),(s2),(s3),(s4),(s5))|SIG1((s6)))
#define SIG7(s1,s2,s3,s4,s5,s6,s7) (SIG6((s1),(s2),(s3),(s4),(s5),(s6))|SIG1((s7)))
#define SIG8(s1,s2,s3,s4,s5,s6,s7,s8) (SIG7((s1),(s2),(s3),(s4),(s5),(s6),(s7))|SIG1((s8)))
#define SIG9(s1,s2,s3,s4,s5,s6,s7,s8,s9) (SIG8((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8))|SIG1((s9)))
#define SIG10(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10) (SIG9((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9))|SIG1((s10)))
#define SIG11(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11) (SIG10((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10))|SIG1((s11)))
#define SIG12(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12) (SIG11((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11))|SIG1((s12)))
#define SIG13(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13) (SIG12((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12))|SIG1((s13)))
#define SIG14(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14) (SIG13((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13))|SIG1((s14)))
#define SIG15(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15) (SIG14((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13),(s14))|SIG1((s15)))
#define SIG16(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16) (SIG15((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13),(s14),(s15))|SIG1((s16)))
#define SIG17(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17) (SIG16((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13),(s14),(s15),(s16))|SIG1((s17)))
#define SIG18(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17,s18) (SIG17((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13),(s14),(s15),(s16),(s17))|SIG1((s18)))
#define SIG19(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17,s18,s19) (SIG18((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13),(s14),(s15),(s16),(s17),(s18))|SIG1((s19)))
#define SIG20(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17,s18,s19,s20) (SIG19((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13),(s14),(s15),(s16),(s17),(s18),(s19))|SIG1((s20)))
#define SIG21(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17,s18,s19,s20,s21) (SIG20((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13),(s14),(s15),(s16),(s17),(s18),(s19),(s20))|SIG1((s21)))
#define SIG22(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17,s18,s19,s20,s21,s22) (SIG21((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13),(s14),(s15),(s16),(s17),(s18),(s19),(s20),(s21))|SIG1((s22)))
#define SIG23(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17,s18,s19,s20,s21,s22,s23) (SIG22((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13),(s14),(s15),(s16),(s17),(s18),(s19),(s20),(s21),(s22))|SIG1((s23)))
#define SIG24(s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17,s18,s19,s20,s21,s22,s23,s24) (SIG23((s1),(s2),(s3),(s4),(s5),(s6),(s7),(s8),(s9),(s10),(s11),(s12),(s13),(s14),(s15),(s16),(s17),(s18),(s19),(s20),(s21),(s22),(s23))|SIG1((s24)))

#endif
