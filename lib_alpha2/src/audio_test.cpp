
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

#define FREQ 1000.f
#define SAMPLE_RATE 48000.f
#define BUFFER_SIZE 512
#define PI 3.14159f

#include <cmath>
#include <iostream>

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
#include <picross/pic_time.h>

#define INTERVAL 667

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
            usbdev = pic::usbenumerator_t::find(BCTKBD_USBVENDOR,BCTKBD_USBPRODUCT).c_str();
	}
        catch(...)
        {
            fprintf(stderr,"can't find a keyboard\n");   
            exit(-1);
        }
    }
    
    alpha2::active_t::delegate_t d;
    pic::usbdevice_t device(usbdev,0);
    alpha2::active_t loop(&device, &d);
    loop.headphone_enable(true);
    loop.headphone_gain(120);
    loop.headphone_limit(true);
    loop.start();
	
    float phase = 0.f;
    float inc = FREQ/SAMPLE_RATE;
    float sine[BUFFER_SIZE*2];
	
    while(1)
    {
        for(unsigned i=0; i<BUFFER_SIZE; i++)
        {
            sine[i*2+0] = std::sin(phase*2.f*PI);
            sine[i*2+1] = std::sin(phase*2.f*PI);
            phase += inc;
            if(phase>1.f)
                phase -= 1.f;
        }
        
	    loop.audio_write(sine,BUFFER_SIZE,48000);
        pic_microsleep((unsigned long)(1000000.0*BUFFER_SIZE/SAMPLE_RATE));
    	loop.poll( 0 );
     }
	
    return 0;
}
