
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

#ifndef __LATCH__
#define __LATCH__

#include "primitive_exports.h"

#include <piw/piw_bundle.h>
#include <piw/piw_data.h>
#include <piw/piw_clock.h>

namespace prim
{
    /*
     * The latch agent implementation.
     */
    class PRIMITIVE_PLG_DECLSPEC_CLASS latch_t : public pic::nocopy_t
    {
        public:

            /*
             * Construct a latch with a clock domain, and with an cookie towards
             * the output atoms.
             */
            latch_t(piw::clockdomain_ctl_t *, const piw::cookie_t &);

            /*
             * The destructor of the latch instance.
             */
            ~latch_t();

            /*
             * Gets the cookie that points to the latch's bundle endpoint so that
             * it can be connected to the agent's inputs to receive the event data.
             */
            piw::cookie_t cookie();

            /*
             * Set the minimum threshold for the latch to let the non controller 
             * signals through unchanged.
             */
            void set_minimum(float m);

            /*
             * Set the number of the latch controller signal, this will be used 
             * to compare against the minimum threshold value.
             */
            void set_controller(unsigned c);

            /*
             * Forward implementation class declaration with the private member 
             * variable is a pattern we use to clearly decouple the public agent 
             * interface from the class declaration of the actual implementation.
             */
            class impl_t;
            
        private:
            impl_t *impl_;
    };
}

#endif

