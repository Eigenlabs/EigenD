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

#ifndef __PIA_UDPNET__
#define __PIA_UDPNET__
#include "pia_exports.h"
#include <picross/pic_fastalloc.h>
#include <piagent/pia_network.h>
#include <picross/pic_nocopy.h>

namespace pia
{
	class  PIA_DECLSPEC_CLASS udpnet_t: public pia::network_t, public pic::nocopy_t
	{
		public:
			class impl_t;
		public:
			udpnet_t(pic::nballocator_t *, bool clock);
			~udpnet_t();

			void *network_open(unsigned space,const char *name,bool promisc);
			void network_close(void *handle);
			int network_write(void *handle, const void *buffer, unsigned len);
			int network_time(void *handle, unsigned long long *time);
            int network_callback(void *handle, void (*cb)(void *ctx, const unsigned char *, unsigned), void *ctx);
		private:
			impl_t *impl_;
	};
}

#endif
