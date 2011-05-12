
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

#include <picross/pic_usb.h>
#include <picross/pic_log.h>
#include <picross/pic_thread.h>
#include <picross/pic_error.h>

#include <stdio.h>
#include <io.h>
#include <string>
#include <set>


#include <picross/pic_windows.h>
#include <initguid.h>
#include <resources/OpenWinDev.h>

using namespace std;

struct pic::usbenumerator_t::impl_t: public pic::thread_t, virtual public pic::tracked_t
{
    impl_t(unsigned short, unsigned short, const f_string_t &);
    ~impl_t();

    void thread_main();
    void thread_init();
    void thread_pass();
    void stop() { stop_=true; wait(); }
    void attached(const char *);

    unsigned short vendor_;
    unsigned short product_;
    f_string_t delegate_;
    pic::mutex_t lock_;
    bool stop_;

    std::set<std::string > devices_;
    std::set<std::string > working_;
};

static unsigned enumerate__(unsigned short vendor, unsigned short product, const pic::f_string_t &callback)
{
	HANDLE hDevice;

    char devname[512];
    devname[0] = 0;

	unsigned short ush_vid = 0;
	unsigned short ush_pid = 0;
 
	if ((hDevice = OpenUsbDevice((LPGUID)&GUID_CLASS_EIGENLABS_ALPHA1_USB,devname)) != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDevice);
		
		basic_string <char>::size_type vid_pos, pid_pos;
		string device( devname );
		
		vid_pos = device.find( "vid",0);
		pid_pos = device.find( "pid",0);

        if(vid_pos!=string::npos && pid_pos!=string::npos)
        {
            std::istringstream(device.substr( vid_pos + 4,4)) >> std::hex >> ush_vid;
            std::istringstream(device.substr( pid_pos + 4,4)) >> std::hex >> ush_pid;
            if(vendor==ush_vid && product==ush_pid)
            {
                try
                {
                    callback(devname);
                    return 1;
                }
                CATCHLOG();
            }
        }
        else
        {
            pic::logmsg() << "could not find vid and/or pid in " << device;
        }
	}
	
    return 0;
}

pic::usbenumerator_t::impl_t::impl_t(unsigned short v, unsigned short p, const f_string_t &d): pic::thread_t(PIC_THREAD_PRIORITY_LOW), vendor_(v), product_(p), delegate_(d)
{
}

pic::usbenumerator_t::impl_t::~impl_t()
{
    tracked_invalidate();
    stop();
}

void pic::usbenumerator_t::impl_t::attached(const char *name)
{
    working_.insert(name);
    if(devices_.find(name) == devices_.end())
    {        
        devices_.insert(name);
        printf( "device attached %s\n",name);
        try
        {
            pic::mutex_t::guard_t g(lock_);
            delegate_(name);
        }
        CATCHLOG()
    }
}

void pic::usbenumerator_t::impl_t::thread_pass()
{
    working_.clear();
    enumerate__(vendor_,product_,pic::f_string_t::method(this,&impl_t::attached));
    devices_ = working_;
 }

void pic::usbenumerator_t::impl_t::thread_init()
{
    stop_=false;
}

void pic::usbenumerator_t::impl_t::thread_main()
{
#ifdef DEBUG_DATA_ATOMICITY
    std::cout << "Started USB enumerator thread with ID " << pic_current_threadid() << std::endl;
#endif
 
    while(!stop_)
    {
        thread_pass();
        Sleep(1000);
    }
}

pic::usbenumerator_t::usbenumerator_t(unsigned short v, unsigned short p, const f_string_t &d)
{
    impl_ = new impl_t(v,p,d);
}

pic::usbenumerator_t::usbenumerator_t(unsigned short v, unsigned short p, const pic::f_string_t &d, const pic::f_string_t &g)
{
    //impl_ = new impl_t(v,p,d,g);
}

pic::usbenumerator_t::~usbenumerator_t()
{
    delete impl_;
}

void pic::usbenumerator_t::start()
{
    impl_->run();
}

void pic::usbenumerator_t::stop()
{
    impl_->stop();
}

unsigned pic::usbenumerator_t::enumerate(unsigned short vendor, unsigned short product, const f_string_t &del)
{
    try
    {
        return enumerate__(vendor,product,del);
    }
    CATCHLOG();
    return 0;
}

int pic::usbenumerator_t::gc_traverse(void *v,void *a) const
{
    return impl_->delegate_.gc_traverse(v,a);
}

int pic::usbenumerator_t::gc_clear()
{
    pic::mutex_t::guard_t g(impl_->lock_);
    return impl_->delegate_.gc_clear();
}
