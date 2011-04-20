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

#ifndef __PI_STATE_FILE__
#define __PI_STATE_FILE__

#include <picross/pic_config.h>

#ifndef PI_WINDOWS
#include <unistd.h>
#else
#include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include <picross/pic_nocopy.h>
#include <picross/pic_ref.h>


namespace pi
{
    namespace state
    {
        struct file_t: public pic::nocopy_t, virtual public pic::counted_t
        {
            virtual void flush() = 0;
            virtual unsigned char *write_payload(unsigned size, unsigned long *position, bool checkpoint) = 0;
            virtual const unsigned char *read_payload(unsigned long offset, unsigned *size) = 0;
            virtual std::string read_payload_string(unsigned long offset) = 0;
            virtual unsigned long write_payload_string(const std::string &data, bool checkpoint) = 0;
            virtual unsigned long checkpoint() = 0;
            virtual bool writeable() = 0;
            virtual bool isopen() = 0;
            virtual void close() = 0;
            virtual std::string name() = 0;
        };

        typedef pic::ref_t<file_t> fileref_t;

        fileref_t open_file(const char *filename, bool writeable);
    };
};

#endif
