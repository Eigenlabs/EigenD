
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

#ifndef __OSC_OUTPUT__
#define __OSC_OUTPUT__

#include "osc_plg_exports.h"

#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_clock.h>

namespace osc_plg
{
    /*
     An OSC server for fast output.

     Each server may potentially contain 1 or more Agents.  This server
     only allows 1 agent.  Each agent contains a number of EigenD ports.
     Each port can transmit multiple concurrent events.

     See plg_osc/OSC

     */
    class OSC_PLG_DECLSPEC_CLASS osc_server_t : public pic::nocopy_t // cant copy this
    {
        public:
            // Construct a server with a clock domain, and with an agent name.
            osc_server_t(piw::clockdomain_ctl_t *, const std::string &agent);
            ~osc_server_t();

            // Create an output. Each output transmits a number of signals
            // together in each packet.  (signals 1..signals)
            // fake_key causes the output to fake a key number signal (this
            // won't be necessary in EigenD 2.0+)
            piw::cookie_t create_output(const std::string &prefix, bool fake_key, unsigned signals);

            // Remove an output.  Any bundle component driving this output
            // will be silently disconnected.
            void remove_output(const std::string &prefix);

			// Forward implementation class declaration with the private member variable
			// is a pattern we use to clearly decouple the public agent interface from the
			// class declaration of the actual implementation.
            class impl_t;
            
        private:
            impl_t *impl_;
    };
}

#endif

