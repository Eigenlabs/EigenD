
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
#include <picross/pic_config.h>

#define KEY 0

struct printer_t: public  pico::active_t::delegate_t
{
    void kbd_dead()
    {
        printf("keyboard disconnected\n");
        fflush(stdout);
        exit(0);
    }

    void kbd_raw(bool resync,const pico::active_t::rawkbd_t &kbd)
    {
        //printf("got %u %u %u %u at %llu\n",kbd.keys[KEY].c[0],kbd.keys[KEY].c[1],kbd.keys[KEY].c[2],kbd.keys[KEY].c[3],pic_microtime());
        printf("%u %llu\n",kbd.strip,kbd.keys[0].time);
        fflush(stdout);
    }
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

    printer_t printer;
    pico::active_t loop(usbdev, &printer);
    loop.start();

    while(1)
    {
        loop.poll(0);
        pic_microsleep(900);
    }

    return 0;
}
