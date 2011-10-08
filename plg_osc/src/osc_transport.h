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

#ifndef __OSC_TRANSPORT__
#define __OSC_TRANSPORT__

#include <piw/piw_tsd.h>
#include <vector>
#include <lib_lo/lo/lo.h>

/*
  Classes for dealing with our OSC abstraction
 */

namespace osc_plg
{
    //
    // Used to keep track of subscribers
    //
    struct osc_recipient_t: pic::nocopy_t
    {
            osc_recipient_t(const char *host, const char *port, unsigned long long t);
            ~osc_recipient_t();
            bool same_as(const char *host, const char *port);
            lo_address address_;
            unsigned long long time_;
    };

    //
    // The OSC server.  Uses a background thread to respond to messages
    //
    class osc_thread_t: pic::thread_t
    {
        public:

            // Construct a server containing a single agent.
            osc_thread_t(const std::string &agent);
            ~osc_thread_t();

            // Send a message to all fast subscribers
            void osc_send_fast(const char *name, lo_message msg);

            // thread functions
            void thread_main();
            void thread_init();
            void thread_term();

            // start and stop the server
            void osc_startup();
            void osc_shutdown();

            // build the OSC path for an output channel
            bool build_channel_url(char *buf, unsigned len, const std::string &port, unsigned index);

        private:

            void add_slow_recipient(const char *host, const char *port);
            void add_fast_recipient(const char *host, const char *port);
            void prune();

            static int slow_register0__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);
            static int slow_register1__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);
            static int slow_register2__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);
            static int fast_register0__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);
            static int fast_register1__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);
            static int fast_register2__(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data);

            bool stop_;
            unsigned counter_;
            piw::tsd_snapshot_t snapshot_;
            lo_server receiver_;
            std::string agent_;

            std::vector<osc_recipient_t *> slow_recipients_;
            std::vector<osc_recipient_t *> fast_recipients_;
    };
};

#endif
