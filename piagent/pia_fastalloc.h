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

#ifndef __PIA_FASTALLOC__
#define __PIA_FASTALLOC__

#include "pia_exports.h"

#include <picross/pic_fastalloc.h>
#include <picross/pic_nocopy.h>

namespace pia
{
	class PIA_DECLSPEC_CLASS fastalloc_t: public pic::nballocator_t, public pic::nocopy_t
	{
		public:
			class nbimpl_t;
			class lckimpl_t;
		public:
			fastalloc_t();
			~fastalloc_t();
			virtual void *allocator_xmalloc(unsigned nb, size_t size,deallocator_t *dealloc, void **dealloc_arg);
		private:
			nbimpl_t *nbimpl_;
			lckimpl_t *lckimpl_;
	};

	class PIA_DECLSPEC_CLASS mallocator_t: public pic::nballocator_t, public pic::nocopy_t
	{
		public:
			class impl_t;
		public:
			mallocator_t();
			~mallocator_t();
			virtual void *allocator_xmalloc(unsigned nb, size_t size,deallocator_t *dealloc, void **dealloc_arg);
		private:
			impl_t *impl_;
	};
}

#endif
