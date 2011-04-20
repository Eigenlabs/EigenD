
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

#include <picross/pic_thread.h>
#include <picross/pic_time.h>
#include <picross/pic_log.h>
#include <lib_pico/pico_usb.h>
#include <lib_pico/pico_active.h>
#include <cmath>

#define WAIT 10000000ULL
#define TICK 5000UL

struct fast_t : pic::thread_t, pico::active_t::delegate_t
{
    fast_t(): thread_t(19), device_(0), poll_(true), quit_(false)
    {
    }

    void thread_main()
    {
        while(!quit_)
        {
            if(poll_)
                device_->poll(0ULL);
            pic_microsleep(TICK);
        }
    }

    pico::active_t *device_;
    volatile bool poll_;
    volatile bool quit_;
};

int main(int ac, char **av)
{
    const char *usbdev = pic::usbenumerator_t::find(0x2139,0x0101).c_str();
    fast_t fast;

    pico::active_t device(usbdev, &fast);
    fast.device_ = &device;

    for(unsigned k=0; k<18; ++k)
        device.set_led(k,1);

    device.start();
    fast.run();
    pic_microsleep(WAIT);

    for(unsigned k=0; k<18; ++k)
        device.set_led(k,2);

    fast.poll_ = false;
    pic_microsleep(WAIT);

    for(unsigned k=0; k<18; ++k)
        device.set_led(k,1);

    //device.restart();
    fast.poll_ = true;
    pic_microsleep(WAIT);

    fast.quit_ = true;
    fast.wait();
    device.stop();
    return 0;
}

