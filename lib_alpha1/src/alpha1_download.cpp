
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

#include <picross/pic_config.h>

#ifdef PI_WINDOWS
#include <io.h>
#endif

#include <picross/pic_usb.h>
#include <picross/pic_error.h>
#include <picross/pic_log.h>
#include <lib_alpha1/alpha1_download.h>
#include <fcntl.h>

namespace alpha1
{
    void tms_download(const char *name,const char *filename)
    {
        pic::usbdevice_t tms(name,0);
        pic::usbdevice_t::bulk_out_pipe_t bulk_pipe(6,64);

        tms.add_bulk_out(&bulk_pipe);

        unsigned char buffer[64];

        int fd;
        int l;

        pic::msg() << "downloading " << filename << " -> " << name << pic::log;

        if((fd=open(filename,O_RDONLY))<0)
        {
            pic::msg() << "Can't open " << filename << pic::hurl;
        }

        try
        {
            while((l=read(fd,buffer,sizeof(buffer)))>0)
            {
                bulk_pipe.bulk_write(buffer,l);
            }

            close(fd);
        }
        catch(...)
        {
            close(fd);
            throw;
        }
    }
}
