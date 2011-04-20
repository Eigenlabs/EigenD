
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

#ifndef PI_WINDOWS
#include <unistd.h>
#include <sys/param.h>
#else
#include <io.h>
#endif

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <lib_alpha1/alpha1_usb.h>
#include <lib_alpha1/alpha1_active.h>
#include <picross/pic_usb.h>
#include <picross/pic_time.h>

struct printer_t: public  alpha1::active_t::delegate_t
{
    printer_t(): count(0),min(4095),max(0),mink(0),maxk(0),minc(0),maxc(0) {}

    void kbd_dead()
    {
        printf("keyboard disconnected\n");
        exit(0);
    }

    void kbd_raw(unsigned long long t, unsigned key, unsigned c1, unsigned c2, unsigned c3, unsigned c4)
    {
        if(c1<min) { min=c1; mink=key; minc=0; }
        if(c2<min) { min=c2; mink=key; minc=1; }
        if(c3<min) { min=c3; mink=key; minc=2; }
        if(c4<min) { min=c4; mink=key; minc=3; }

        if(c1>max) { max=c1; maxk=key; maxc=0; }
        if(c2>max) { max=c2; maxk=key; maxc=1; }
        if(c3>max) { max=c3; maxk=key; maxc=2; }
        if(c4>max) { max=c4; maxk=key; maxc=3; }

        if(key==119)
        {
            printf("min=%04d (%03d/%d) max=%04d (%03d/%d)\n",min,mink,minc,max,maxk,maxc);
            min=4095;max=0;
        }
    }

    unsigned char count;
    unsigned min,max,mink,maxk,minc,maxc;
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
            usbdev = pic::usbenumerator_t::find(BCTALPHA1_USBVENDOR,BCTALPHA1_USBPRODUCT).c_str();
        }
        catch(...)
        {
            fprintf(stderr,"can't find a keyboard\n");   
            exit(-1);
        }
    }

    printer_t printer;
    alpha1::active_t loop(usbdev, &printer);
    loop.set_raw(true);

    pic_microsleep(1000000);
	loop.start();
    pic_microsleep(1000000);

    while(1)
    {
        pic_microsleep(2000);
        loop.poll(0);
    }

    return 0;
}
