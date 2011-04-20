
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


#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <picross/pic_config.h>
#ifdef PI_WINDOWS
#include <io.h>
#else
#include <unistd.h>
#endif


#include <lib_pico/pico_usb.h>
#include <lib_pico/pico_active.h>
#include <picross/pic_usb.h>
#include <picross/pic_time.h>

struct reader_t: public  pico::active_t::delegate_t
{
    reader_t() : thresh(0xffff), min(0xffff), max(0), count(0) {}

    void kbd_dead()
    {
        printf("keyboard disconnected\n");
        fflush(stdout);
        exit(0);
    }

    void kbd_raw(bool resync,const pico::active_t::rawkbd_t &kbd)
    {
        if(++count < 20)
            return;

        if(min == 0xffff)
            min = kbd.strip;

        thresh = std::min(thresh,kbd.strip);
        max = std::max(max,kbd.strip);
        printf("17 0 %d %d %d   \r",thresh,min,max);
        fflush(stdout);
    }

    unsigned short thresh;
    unsigned short min;
    unsigned short max;
    unsigned count;
};

int main(int ac, char **av)
{
    const char *usbdev = 0;

    if(ac==2)
    {
        usbdev=av[1];
    }
    else
    {
        try
        {
            usbdev = pic::usbenumerator_t::find(BCTPICO_USBVENDOR,BCTPICO_USBPRODUCT).c_str();
        }
        catch(...)
        {
            fprintf(stderr,"can't find a keyboard\n");   
            exit(-1);
        }
    }

    printf("hold down right end of strip controller (2 seconds)\n");
    fflush(stdout);
    pic_microsleep(2000000);

    printf("sweep strip controller then release (5 seconds)\n");
    fflush(stdout);

    reader_t reader;
    pico::active_t loop(usbdev, &reader);
    loop.start();

    unsigned count = 0;

    while(++count < 5000)
    {
        loop.poll(0);
        pic_microsleep(900);
    }

    printf("\n");
    fflush(stdout);

    exit(0);
}
