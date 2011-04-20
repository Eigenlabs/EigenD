
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
#else
#include <io.h>
#endif


#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <lib_alpha2/alpha2_usb.h>
#include <lib_alpha2/alpha2_active.h>
#include <picross/pic_usb.h>
#include <picross/pic_time.h>
#include <picross/pic_log.h>

struct printer_t: public  alpha2::active_t::delegate_t
{
    printer_t(int k): count(0), key_(k)
    {
        memset(curmap,0,18);
    }

    void kbd_dead()
    {
        printf("keyboard disconnected\n");
        exit(0);
    }

    void kbd_key(unsigned long long t, unsigned key, unsigned p, int r, int y)
    {
        if(count>0)
        {
            count--;
            printf("cooked: %04d %04d %04d %04d %llu\n", key, p, r, y, t);
        }

        if(key_>=0 && (int)key==key_)
            printf("cooked: %04d %04d %04d %04d %llu\n", key, p, r, y, t);

    }

    void kbd_raw(unsigned long long t, unsigned key, unsigned c1, unsigned c2, unsigned c3, unsigned c4)
    {
        printf("raw: %04d %04d %04d %04d %04d\n", key, c1, c2, c3, c4);
    }

    void kbd_keydown(unsigned long long t, const unsigned short *bitmap)
    {
        dump_keydown(bitmap);
    }

    void kbd_mic(unsigned char s,unsigned long long t, const float *data)
    {
        //for(unsigned i=0;i<16;i++) { printf("%f\n",data[i]); }
        if(s!=seq_) printf("**seq %d\n",s);
        else printf("seq %d\n",s);
        seq_=(s+1)&0xff;
    }

    void dump_keydown(const unsigned short *map)
    {
        if(memcmp(map,curmap,18)==0)
            return;

        memcpy(curmap,map,18);

        pic::msg_t m = pic::logmsg();
        for(unsigned k=0; k<KBD_KEYS+KBD_SENSORS+1; ++k)
        {
            m << (alpha2::active_t::keydown(k,map)?"X":"-");
        }
        count = 5;
    }

   
    void midi_data(unsigned long long t, const unsigned char *data, unsigned len)
    {
    }

    void pedal_down(unsigned long long t, unsigned pedal, unsigned p)
    {
        //printf( "pedal %u: %u\n",pedal,p );
    }


    unsigned short curmap[9];
    unsigned count;
    int key_;
    unsigned char seq_;
};

int main(int ac, char **av)
{
    const char *usbdev = 0;
    int key = -1;

    try
    {
        usbdev = pic::usbenumerator_t::find(0xbeca,0x0102).c_str();
        //usbdev = pic::usbenumerator_t::find(BCTKBD_USBVENDOR,BCTKBD_USBPRODUCT).c_str();
    }
    catch(...)
    {
        fprintf(stderr,"can't find a keyboard\n");   
        exit(-1);
    }

    if(ac==2)
    {
        key=atoi(av[1]);
        fprintf(stderr,"using key %d\n",key);
    }

    {
        printer_t printer(key);
        pic::usbdevice_t device(usbdev,0);
        alpha2::active_t loop(&device, &printer);
        loop.start();
        pic_microsleep(250000);
        loop.set_raw(true);

        for(unsigned long i=0; i<1000; i++)
        {
            loop.poll(0);
            pic_microsleep(5000);
        }
    }
    pic_microsleep(500000);

    return 0;
}

//int main( int ac, char **av )
//{
//	return 0;
//}
