
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
#ifndef PI_WINDOWS
#include <unistd.h>
#else
#include <io.h>
#define sleep Sleep
#endif


#include <lib_alpha1/alpha1_usb.h>
#include <lib_alpha1/alpha1_active.h>
#include <picross/pic_usb.h>
#include <picross/pic_time.h>

struct printer_t: public  alpha1::active_t::delegate_t
{
    printer_t(int raw): raw(raw), count(0) { memset(map,0,22); }

    void kbd_dead()
    {
        printf("keyboard disconnected\n");
        exit(0);
    }

    void kbd_raw(unsigned long long t, unsigned key, unsigned c0, unsigned c1, unsigned c2, unsigned c3)
    {
        if((int)key==raw)
            printf("KR: %016llx: %04u %04u %04u %04u %04u\n", t, key, c0, c1, c2, c3);
    }

    void kbd_key(unsigned long long t, unsigned key, unsigned p, int r, int y)
    {
        printf("KK: %016llx: %u %u %d %d\n",t,key,p,r,y);
    }

    void kbd_keydown(unsigned long long t, const unsigned short *bitmap)
    {
        if(memcmp(map,bitmap,18)==0)
            return;
        
        memcpy(map,bitmap,18);

        printf("KD: %016llx: ",t);
        for(unsigned k=0; k<KBD_KEYS; ++k)
        {
            printf(alpha1::active_t::keydown(k,bitmap)?"X":"-");
        }
        printf("\n");
    }

    int raw;
    unsigned char count;
    unsigned short map[9];
};

int main(int ac, char **av)
{
    int raw = -1;

    if(ac==2)
    {
        raw = atoi(av[1]);
    }

    const char *usbdev;
    try
    {
        usbdev = pic::usbenumerator_t::find(BCTALPHA1_USBVENDOR,BCTALPHA1_USBPRODUCT).c_str();
    }
    catch(...)
    {
        fprintf(stderr,"can't find a keyboard\n");   
        exit(-1);
    }

    sleep(1);
    printer_t printer(raw);
    alpha1::active_t loop(usbdev, &printer);
    loop.set_raw(raw>0);

    sleep(1);
    loop.start();
    sleep(1);

    while(1)
    {
        pic_microsleep(2000);
        loop.poll(0);
    }

    return 0;
}
