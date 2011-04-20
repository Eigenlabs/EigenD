
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


//#include <pikeyboard/performotron_usb.h>
#include <lib_pico/pico_active.h>
#include <picross/pic_usb.h>
#include <picross/pic_time.h>

#define BCTPICO_USBVENDOR 0x2139
#define BCTPICO_USBPRODUCT 0x0101

struct printer_t: public  pico::active_t::delegate_t
{
    printer_t(): count(0) {}

    void kbd_dead()
    {
        printf("keyboard disconnected\n");
        exit(0);
    }

    void kbd_key(unsigned long long t, unsigned key, bool a, unsigned p, int r, int y)
    {
       printf("%llu %u %u %d %d\n",t,key,p,r,y);
    }

    void kbd_raw(bool resync,const pico::active_t::rawkbd_t &raw) 
    {
        
        const pico::active_t::rawkey_t *key = raw.keys;
        const unsigned short *color = NULL; 

        for( int i = 0; i < 20; i++)
        {
            color = key[i].c;
            printf( "key: %02d c1:%04d, c2:%04d, c3:%04d, c4:%04d\n",i,color[1],color[2],color[3],color[4] ); 
                
        }
    
    }
    unsigned char count;
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

    loop.load_calibration_from_device();
    pic_microsleep(1000000);
    loop.start();
    pic_microsleep(1000000);

    while(1)
    {       
        loop.poll(0);
		pic_microsleep(10000);
	 }

    return 0;
}
