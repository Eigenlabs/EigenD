

#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <memory>
#include <mbstring.h>
#include <stdio.h>
#include <io.h>
#include <string>
#include <set>

#include <picross/pic_thread.h>
#include <picross/pic_error.h>
#include <picross/pic_log.h>
#include <picross/pic_usb.h>
#include <picross/pic_time.h>
#include <picross/pic_fastalloc.h>
#include <picross/pic_strbase.h>
#include <picross/pic_ilist.h>
#include <picross/pic_windows.h>
#include <initguid.h>
#include <resources/openwindev.h>

#define FRAMES_PER_URB_ADVANCED      16
#define FRAMES_PER_URB_LEGACY        1

#define URBS_PER_PIPE_ADVANCED	    (64/FRAMES_PER_URB_ADVANCED)
#define BUFFERS_PER_PIPE_ADVANCED	(4*URBS_PER_PIPE_ADVANCED)
#define URBS_PER_PIPE_LEGACY        64
#define BUFFERS_PER_PIPE_LEGACY	    (8*URBS_PER_PIPE_LEGACY)

namespace
{
	struct usbpipe_out_t;
	struct usbpipe_in_t;

    struct usbbuf_t: pic::element_t<0>, pic::element_t<1>, virtual pic::lckobject_t
    {
        usbbuf_t(usbpipe_in_t *pipe);
        ~usbbuf_t();

        EIGENHARP_USB_PACKET_HDR *headers() { return (EIGENHARP_USB_PACKET_HDR *)buffer_; }
        unsigned char *payload() { return buffer_+packets_*sizeof(EIGENHARP_USB_PACKET_HDR); }

        usbpipe_in_t *pipe_;
        unsigned char *buffer_;
        unsigned size_;
        unsigned psize_;
        unsigned packets_;
        bool completed_;
        unsigned consumed_;
    };

	struct usburb_t: pic::element_t<0>, virtual pic::lckobject_t
	{
		OVERLAPPED overlapped_;
		usbbuf_t *buffer_;
	};

    template <class X,int N> struct list_t: pic::ilist_t<X,N>
    {
        ~list_t()
        {
            X *i;

            while((i=pop_front())!=0)
            {
                delete i;
            }
        }
    };
	
	struct usbpipe_in_t: virtual pic::lckobject_t
	{
		usbpipe_in_t(pic::usbdevice_t::impl_t *dev, pic::usbdevice_t::iso_in_pipe_t *pipe);
        ~usbpipe_in_t();

		void pipe_died();
		void completed(usbbuf_t *, unsigned long);
        void submit(usburb_t *);
        bool poll_pipe(unsigned long long);

		void start();
        usbbuf_t *pop_free_queue();
        usbbuf_t *pop_receive_queue();
        void append_free_queue(usbbuf_t *buf);
        void append_receive_queue(usbbuf_t *buf);
        void prepend_receive_queue(usbbuf_t *buf);

				
        CRITICAL_SECTION lock_;
		pic::usbdevice_t::iso_in_pipe_t *pipe_;
		unsigned long long frame_;
        list_t<usbbuf_t,0> buffer_list_;
        list_t<usburb_t,0> urb_list_;
        pic::ilist_t<usbbuf_t,1> free_queue_;
        pic::ilist_t<usbbuf_t,1> receive_queue_;
		pic::usbdevice_t::impl_t *device_;
		unsigned piperef_;
		unsigned size_;
		HANDLE phandle_;
        bool died_;
        bool stolen_;
        bool advanced_;
        unsigned short packets_;
	};

	struct usbpipe_out_t: virtual pic::lckobject_t
	{
		usbpipe_out_t(pic::usbdevice_t::impl_t *dev, pic::usbdevice_t::iso_out_pipe_t *pipe);
        ~usbpipe_out_t();

		pic::usbdevice_t::impl_t *device_;
		unsigned piperef_;
		unsigned size_;
		HANDLE phandle_;
	};

    class SystemError
    {
        public:
            SystemError() { error = GetLastError(); }
            SystemError(unsigned long e) { error = e; }
            unsigned long error;
    };

    std::ostream &operator<<(std::ostream &o, const SystemError &e)
    {
        char buffer[512];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,e.error,LANG_NEUTRAL,buffer,256,NULL);
        unsigned l = strlen(buffer);
        while(l && isspace(buffer[l-1])) l--;
        buffer[l] = 0;
        o << e.error << ": " << buffer;
        return o;
    }

};

struct pic::usbdevice_t::bulk_out_pipe_t::impl_t: virtual pic::lckobject_t
{
    impl_t(pic::usbdevice_t::impl_t *dev,pic::usbdevice_t::bulk_out_pipe_t *pipe);
    ~impl_t();

    void bulk_write(const void *data, unsigned len, unsigned timeout);

    pic::usbdevice_t::impl_t *device_;
    pic::usbdevice_t::bulk_out_pipe_t *pipe_;
    unsigned piperef_;
    unsigned size_;
    HANDLE phandle_;
};

struct pic::usbdevice_t::impl_t: pic::thread_t, virtual pic::lckobject_t
{
	impl_t(const char *, unsigned, pic::usbdevice_t *);
	~impl_t();

	bool poll_pipe(unsigned long long);
	bool add_iso_in(iso_in_pipe_t *p);
	void set_iso_out(iso_out_pipe_t *p);
    bool add_bulk_out(bulk_out_pipe_t *p);
	void start_pipes();
	void stop_pipes();
    void detach();
    void thread_main();

    pic::usbdevice_t::power_t *power;
    std::string name_;

	pic::flipflop_t<pic::lcklist_t<usbpipe_in_t *>::lcktype> inpipes_;
    pic::flipflop_t<usbpipe_out_t *> pipe_out_;

    HANDLE	dhandle_;
    HANDLE	iohandle_;
    char    pathname_[256];
            
    CRITICAL_SECTION lock_;

    pic::usbdevice_t *device_;
    bool stopping_;
    bool hispeed_;
    unsigned count_;

};

struct pic::usbenumerator_t::impl_t: pic::thread_t, virtual pic::tracked_t, virtual pic::lckobject_t
{
    impl_t(unsigned short, unsigned short, const f_string_t &, const f_string_t &);
    ~impl_t();

    void thread_main();
    void thread_init();
    void thread_pass();
    void stop() { stop_=true; wait(); }

    unsigned short vendor_;
    unsigned short product_;
    f_string_t added_,removed_;
    pic::mutex_t lock_;
    bool stop_;

    std::set<std::string > devices_;
};

pic::usbdevice_t::bulk_out_pipe_t::impl_t::impl_t(pic::usbdevice_t::impl_t *dev,pic::usbdevice_t::bulk_out_pipe_t *pipe): device_(dev), pipe_(pipe), piperef_(pipe->out_pipe_name()), size_(pipe->out_pipe_size())
{
	char buffer[256];
	char PipeName[512];

    pipe_->impl_ = this;

	sprintf(buffer,"PIPE0%d",piperef_);
	strcpy(PipeName,device_->pathname_);
	strcat(PipeName,"\\");
	strcat(PipeName,buffer);

	if (( phandle_ = CreateFile(PipeName,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,0,0)) == INVALID_HANDLE_VALUE)
	{
		pic::logmsg() << "CreateFile() for bulk out " << SystemError();
	}
}

pic::usbdevice_t::bulk_out_pipe_t::impl_t::~impl_t()
{
    CloseHandle(phandle_);
}

usbpipe_out_t::usbpipe_out_t(pic::usbdevice_t::impl_t *dev, pic::usbdevice_t::iso_out_pipe_t *pipe): device_(dev), piperef_(pipe->out_pipe_name()), size_(pipe->out_pipe_size())
{
}

usbpipe_out_t::~usbpipe_out_t()
{
}

usbbuf_t *usbpipe_in_t::pop_free_queue()
{
    EnterCriticalSection(&lock_);
    usbbuf_t *buf = free_queue_.pop_front();

    if(!buf)
    {
        if(receive_queue_.head() && receive_queue_.head()->completed_)
        {
            buf = receive_queue_.pop_front();
            stolen_ = true;
            pic::logmsg() << "stole buffer";
        }
    }

    LeaveCriticalSection(&lock_);
    return buf;
}

void usbpipe_in_t::append_free_queue(usbbuf_t *buf)
{
    EnterCriticalSection(&lock_);
    free_queue_.append(buf);
    LeaveCriticalSection(&lock_);
}

usbbuf_t *usbpipe_in_t::pop_receive_queue()
{
    EnterCriticalSection(&lock_);
    usbbuf_t *buf = receive_queue_.pop_front();
    LeaveCriticalSection(&lock_);
    return buf;
}

void usbpipe_in_t::prepend_receive_queue(usbbuf_t *buf)
{
    EnterCriticalSection(&lock_);
    receive_queue_.prepend(buf);
    LeaveCriticalSection(&lock_);
}

void usbpipe_in_t::append_receive_queue(usbbuf_t *buf)
{
    EnterCriticalSection(&lock_);
    receive_queue_.append(buf);
    LeaveCriticalSection(&lock_);
}

void usbpipe_in_t::submit(usburb_t *urb)
{
    if(device_->stopping_)
    {
        return;
    }

    if(!(urb->buffer_ = pop_free_queue()))
    {
        pic::logmsg() << "no buffers";
        pipe_died();
        return;
    }

    memset(urb->buffer_->buffer_,0,urb->buffer_->size_);
    memset(&urb->overlapped_,0,sizeof(urb->overlapped_));
    ULONG br;

    for(unsigned i=0;i<packets_;i++)
    {
        urb->buffer_->headers()[i].Length = 0xffffffff;
    }

    unsigned char *dp = urb->buffer_->buffer_;
    unsigned dl = urb->buffer_->size_;

    if(!advanced_)
    {
        dp += packets_*sizeof(EIGENHARP_USB_PACKET_HDR);
        dl -= packets_*sizeof(EIGENHARP_USB_PACKET_HDR);
    }

    urb->buffer_->completed_ = false;
    urb->buffer_->consumed_ = 0;

    if(advanced_)
    {
        append_receive_queue(urb->buffer_);
    }

    if (ReadFile(phandle_,dp,dl,&br,&urb->overlapped_) != 0)
    {
        pic::logmsg() << "* Completed in ReadPacket() NOT EXPECTED *"; // Expecting it to complete in IOCP thread
        completed(urb->buffer_,br);

        submit(urb);
        return;
    }

    if (GetLastError() == ERROR_IO_PENDING)
    {
        EnterCriticalSection(&device_->lock_);
        device_->count_++;
        LeaveCriticalSection(&device_->lock_);
        return;
    }

    pic::logmsg() << "ReadFile() " << phandle_ << ' ' << (void *)dp << ' ' << dl << ' ' << SystemError();
    pipe_died();
}

usbbuf_t::usbbuf_t(usbpipe_in_t *pipe): pipe_(pipe)
{
    packets_ = pipe_->packets_;
    psize_ = pipe_->size_;
    size_ = packets_*(psize_+sizeof(EIGENHARP_USB_PACKET_HDR));
    buffer_ = (unsigned char *)pic::nb_malloc(PIC_ALLOC_LCK,size_);
}

usbbuf_t::~usbbuf_t()
{
    pic::nb_free(buffer_);
}

usbpipe_in_t::~usbpipe_in_t()
{
    CloseHandle(phandle_);
}

void usbpipe_in_t::pipe_died()
{
    if(!died_)
    {
        died_ = true;
        pipe_->pipe_died();
    }
}

usbpipe_in_t::usbpipe_in_t(pic::usbdevice_t::impl_t *dev, pic::usbdevice_t::iso_in_pipe_t *pipe): pipe_(pipe), frame_(0ULL), device_(dev), piperef_(pipe->in_pipe_name()), size_(pipe->in_pipe_size()), died_(false), stolen_(false)
{
    InitializeCriticalSectionAndSpinCount(&lock_,0x400);

	char buffer[256];
	char PipeName[512];

	sprintf(buffer,"PIPE0%d",piperef_);
	strcpy(PipeName,device_->pathname_);
	strcat(PipeName,"\\");
	strcat(PipeName,buffer);

	if (( phandle_ = CreateFile(PipeName,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0)) == INVALID_HANDLE_VALUE)
	{
		pic::hurlmsg() << "CreateFile() for iso in " << SystemError();
    }

    static ULONG_PTR t = piperef_;

    if (CreateIoCompletionPort(phandle_,device_->iohandle_,t,1) == NULL)
    {
        pic::hurlmsg() << "CreateIoCompletionPort() " << SystemError();
    }

    pic::logmsg() << "Pipe Created " << phandle_;

	unsigned long retval = 0;
    advanced_ = false;

	if (DeviceIoControl(phandle_,IOCTL_EIGENHARP_SET_ADVANCED,NULL,0,0,0,&retval,0)!=0)
	{
		pic::logmsg() << "set advanced timestamp mode";
        advanced_ = true;
	}

    packets_ = (device_->hispeed_?8:1)*(advanced_?FRAMES_PER_URB_ADVANCED:FRAMES_PER_URB_LEGACY);

    unsigned buf_count = advanced_?BUFFERS_PER_PIPE_ADVANCED:BUFFERS_PER_PIPE_LEGACY;

    for(unsigned i=0;i<buf_count;i++)
    {
        usbbuf_t *buf = new usbbuf_t(this);
        free_queue_.append(buf);
        buffer_list_.append(buf);
    }

    unsigned urb_count = advanced_?URBS_PER_PIPE_ADVANCED:URBS_PER_PIPE_LEGACY;

    for(unsigned i=0;i<urb_count;i++)
    {
        usburb_t *urb = new usburb_t;
        urb_list_.append(urb);
    }

}

void pic::usbdevice_t::impl_t::thread_main()
{
	OVERLAPPED *overlapped;
	unsigned long key;
	unsigned long br;
	int ok;
    usburb_t *urb;
    usbpipe_in_t *pipe;
    usbbuf_t *buf;

    for(;;)
	{
        if(stopping_)
        {
            unsigned c;

            EnterCriticalSection(&lock_);
            c = count_;
            LeaveCriticalSection(&lock_);

            if(c == 0)
            {
                pic::logmsg() << "queue has drained";
                break;
            }
        }

        ok = GetQueuedCompletionStatus(iohandle_,&br,&key,&overlapped,5000);

        urb = 0;
        pipe = 0;
        buf = 0;

        if(overlapped)
        {
            urb = PIC_STRBASE(usburb_t,overlapped,overlapped_);
            buf = urb->buffer_;
            pipe = buf->pipe_;
        }

        unsigned long error = 0;
        
        if(!ok)
        {
            error = GetLastError();
        }

        if(urb)
        {
            EnterCriticalSection(&lock_);
            count_--;
            LeaveCriticalSection(&lock_);

            if(!stopping_)
            {
                pipe->submit(urb);
            }

            if(ok)
            {
                pipe->completed(buf,br);
                continue;
            }
        }

        if (ok || error==0 || error==WAIT_TIMEOUT) // timed out
        {
            continue;
        }

        if(!stopping_)
        {
            pic::logmsg() << "GetQueuedCompletionStatus " << SystemError(error);
        }

        if(pipe)
        {
            pipe->pipe_died();
        }

        stopping_ = true;
	}

	return;
}

void usbpipe_in_t::completed(usbbuf_t *buf, unsigned long size)
{
    EIGENHARP_USB_PACKET_HDR *hdr = (EIGENHARP_USB_PACKET_HDR *)(buf->buffer_);
    unsigned frame_inc = (advanced_?FRAMES_PER_URB_ADVANCED:FRAMES_PER_URB_LEGACY)*((device_->hispeed_)?8:1);

    if(!advanced_)
    {
        if(packets_==1)
        {
            hdr[0].Length = size;
            hdr[0].Time = pic_microtime();
            hdr[0].Frame = frame_;
        }
        else
        {
            unsigned long long t = pic_microtime();
            unsigned long long inc = device_->hispeed_?125:1000;

            t -= (packets_-1)*inc;

            hdr[0].Length = size_;
            hdr[0].Time = t;
            hdr[0].Frame = frame_;

            for(unsigned i=1;i<packets_;i++)
            {
                hdr[i].Length = size_;
                hdr[i].Time = hdr[i-1].Time+inc;
                hdr[i].Frame = hdr[i-1].Frame+1;
            }
        }

        frame_ += frame_inc;
    }
    else
    {
        unsigned long long frame = hdr[0].Frame;

        if(frame_ && (frame != frame_+frame_inc))
        {
            if(device_->hispeed_)
            {
                pic::logmsg() << "pipe " << piperef_ << " dropped frame: got " << frame/8 << ':' << frame%8 << " expected " << (frame_+frame_inc)/8 << ':' << (frame_+frame_inc)%8;
            }
            else
            {
                pic::logmsg() << "pipe " << piperef_ << " dropped frame: got " << frame << " expected " << (frame_+frame_inc);
            }
        }

        frame_ = frame;
    }

    buf->completed_ = true;

    if(!advanced_)
    {
        if(size)
        {
            append_receive_queue(buf);
        }
        else
        {
            append_free_queue(buf);
        }
    }
}


void usbpipe_in_t::start()
{
    usburb_t *urb;

    for(urb=urb_list_.head(); urb; urb = urb_list_.next(urb))
    {
        submit(urb);
    }
}

void pic::usbdevice_t::bulk_out_pipe_t::impl_t::bulk_write(const void *data, unsigned len, unsigned timeout)
{
	unsigned long BytesSent =  0;

	if (WriteFile(phandle_,data,len,&BytesSent,0) == 0)
	{
        pic::logmsg() << "bulk write failed " << SystemError();
	}
}

pic::usbdevice_t::impl_t::impl_t(const char *name, unsigned iface, pic::usbdevice_t *dev): thread_t(PIC_THREAD_PRIORITY_REALTIME), device_(dev), pipe_out_(0), stopping_(false), count_(0)
{
    InitializeCriticalSectionAndSpinCount(&lock_,0x400);
	
	if ((dhandle_ = CreateFile (name,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL)) == INVALID_HANDLE_VALUE)
	{
		pic::hurlmsg() << "CreateFile() " << SystemError();
    }

    ULONG speed;
	unsigned long retval = 0;
    char version[32];

	if (DeviceIoControl(dhandle_,IOCTL_EIGENHARP_GET_VERSION,NULL,0,version,32,&retval,0)==0)
	{
		pic::logmsg() << "can't get driver version";
	}
    else
    {
        pic::logmsg() << "USB driver is version " << version;
    }

	if (DeviceIoControl(dhandle_,IOCTL_EIGENHARP_GET_SPEED,NULL,0,&speed,sizeof(ULONG),&retval,0)==0)
	{
		pic::logmsg() << "can't get speed; assuming full speed";
        speed = 0;
	}

    hispeed_ = (speed!=0)?true:false;

    pic::logmsg() << "Opened " << (hispeed_?"High":"Full") << " Speed Usb Device " << name;

    strcpy(pathname_,name);

    if ((iohandle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1)) == NULL)
    {
        CloseHandle(dhandle_);
        pic::hurlmsg() << "CreateIoCompletionPort() " << SystemError();
    }
}

bool pic::usbdevice_t::add_bulk_out(bulk_out_pipe_t *p)
{
    return impl_->add_bulk_out(p);
}

bool pic::usbdevice_t::add_iso_in(iso_in_pipe_t *p)
{
	return impl_->add_iso_in(p);	
}

void pic::usbdevice_t::set_iso_out(iso_out_pipe_t *p)
{
	impl_->set_iso_out(p);
}

bool pic::usbdevice_t::impl_t::add_bulk_out(bulk_out_pipe_t *p)
{
    new pic::usbdevice_t::bulk_out_pipe_t::impl_t(this,p);
    return true;
}

bool pic::usbdevice_t::impl_t::add_iso_in(iso_in_pipe_t *p)
{
	try
	{
		usbpipe_in_t *pipe  = new usbpipe_in_t(this,p);
		inpipes_.alternate().push_back(pipe);
		inpipes_.exchange();
		return true;
	}
	catch(std::exception &e)
	{
		pic::logmsg() << "cannot add pipe: " << e.what();
		return false;
	}
	catch(...)
	{
		pic::logmsg() << "cannot add pipe: unknown error";
		return false;
	}
}

void pic::usbdevice_t::impl_t::set_iso_out(iso_out_pipe_t *p)
{
	try
	{
		pipe_out_.set(new usbpipe_out_t(this,p));
	}
	catch(std::exception &e)
	{
		pic::logmsg() << "cannot add pipe: " << e.what();
	}
	catch(...)
	{
		pic::logmsg() << "cannot add pipe: unknown error";
	}
}

pic::usbdevice_t::impl_t::~impl_t()
{
    detach();
	CloseHandle(dhandle_);
    DeleteCriticalSection(&lock_);

}

bool pic::usbdevice_t::impl_t::poll_pipe(unsigned long long t)
{
    pic::flipflop_t<pic::lcklist_t<usbpipe_in_t *>::lcktype>::guard_t g(inpipes_);
    pic::lcklist_t<usbpipe_in_t *>::lcktype::const_iterator i;
    bool stolen = false;

    for(i=g.value().begin(); i!=g.value().end(); i++)
    {
        if((*i)->poll_pipe(t))
        {
            stolen = true;
        }
    }

    return stolen;
}

bool usbpipe_in_t::poll_pipe(unsigned long long t)
{
	usbbuf_t *buf = NULL;

    bool stolen = stolen_;
    stolen_ = false;

	while((buf=pop_receive_queue()) != 0)
	{	
        while(buf->consumed_ < buf->packets_)
        {
            EIGENHARP_USB_PACKET_HDR *h = &buf->headers()[buf->consumed_];
            const unsigned char *p = buf->payload()+buf->consumed_*buf->psize_;

            if(!h->Length)
            {
                buf->consumed_++;
                continue;
            }

            if(h->Length >= 0xffff)
            {
                break;
            }

            if(t && h->Time>t)
            {
                break;
            }

            pipe_->call_pipe_data(p,h->Length,h->Frame,h->Time,t);
            buf->consumed_++;
        }

        if(buf->consumed_ < buf->packets_)
        {
            prepend_receive_queue(buf);
            return stolen;
        }

        append_free_queue(buf);
	}

	return stolen;
}

pic::usbdevice_t::usbdevice_t(const char *name, unsigned iface)
{
	pic::logmsg() << "usb device created";
	impl_ = new usbdevice_t::impl_t(name,iface,this);
}

void pic::usbdevice_t::start_pipes()
{
	impl_->start_pipes();
}

void pic::usbdevice_t::impl_t::stop_pipes()
{
    if(!isrunning())
    {
        return;
    }

	stopping_ = true;
    wait();
}

void pic::usbdevice_t::impl_t::start_pipes()
{
    if(isrunning())
    {
        return;
    }

	stopping_ = false;
	pic::lcklist_t<usbpipe_in_t *>::lcktype::const_iterator i;
	
	for(i=inpipes_.alternate().begin(); i!=inpipes_.alternate().end(); i++)
	{
		(*i)->pipe_->pipe_started();	// send start pipe deviocts
		(*i)->start();				
	}
	
    run();
	pic::logmsg() << "pipes started!" ; 
}

void pic::usbdevice_t::impl_t::detach()
{
    pic::logmsg() << "detaching client";

    stop_pipes();

	while( inpipes_.alternate().size() > 0 )
	{
		usbpipe_in_t *p = inpipes_.alternate().front();
		inpipes_.alternate().pop_front();
        inpipes_.exchange();
        delete p;		
	}

	power = 0;
	//device_ = NULL;
    pic::logmsg() << "done detaching client";
}

void pic::usbdevice_t::stop_pipes()
{
    impl_->stop_pipes();
}

pic::usbdevice_t::~usbdevice_t()
{
	delete impl_;
}

bool pic::usbdevice_t::poll_pipe(unsigned long long t)
{
	return impl_->poll_pipe(t);
}

void pic::usbdevice_t::control_in(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, void *buffer, unsigned len, unsigned timeout)
{
	EIGENHARP_USB_VENDOR_MSG request;
	unsigned long retval = 0;

	memset( &request,0,sizeof( request ));

	request.Request_Type = type;
	request.Request = req;
	request.Value = val;
	request.Index = ind;
	request.Length = len;

	if (DeviceIoControl(impl_->dhandle_,IOCTL_EIGENHARP_GET_BUFFER,&request,sizeof(EIGENHARP_USB_VENDOR_MSG),buffer,len,&retval,0)==0)
	{
		pic::logmsg() << "can't do control_in request " << SystemError() << ' ' << std::hex << (int)type << ':' << (int)req;
	}
}

void pic::usbdevice_t::control_out(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, const void *buffer, unsigned len, unsigned timeout)
{
    unsigned char request[128+sizeof(EIGENHARP_USB_VENDOR_MSG)];
    EIGENHARP_USB_VENDOR_MSG *reqhdr = (EIGENHARP_USB_VENDOR_MSG *)request;
	unsigned long retval = 0;

	memset(request,0,sizeof(EIGENHARP_USB_VENDOR_MSG));
	reqhdr->Request_Type = type;
	reqhdr->Request = req;
	reqhdr->Value = val; //address
	reqhdr->Index = ind; // index 0
	reqhdr->Length = len;
    memcpy(&request[sizeof(EIGENHARP_USB_VENDOR_MSG)],buffer,len);
		
	if( DeviceIoControl(impl_->dhandle_,IOCTL_EIGENHARP_SET_BUFFER,request,len+sizeof(EIGENHARP_USB_VENDOR_MSG),NULL,0,&retval,0)==0)
	{
		pic::logmsg() << "can't do control_out request " << SystemError() << ' ' << std::hex << (int)type << ':' << (int)req;
	}
}

void pic::usbdevice_t::control(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, unsigned timeout)
{
	EIGENHARP_USB_VENDOR_MSG request;
	unsigned long retval = 0;

	memset(&request,0,sizeof(request));
	request.Request_Type = type;
	request.Request = req;
	request.Value = val; //address
	request.Index = ind; // index 0
	request.Length = 0;
		
	if (DeviceIoControl(impl_->dhandle_,IOCTL_EIGENHARP_SET_BUFFER,&request,sizeof(EIGENHARP_USB_VENDOR_MSG),NULL,0,&retval,0)==0)
	{
		pic::logmsg() << "can't do control request " << SystemError() << ' ' << std::hex << (int)type << ':' << (int)req;
	}
	
}

void pic::usbdevice_t::bulk_out_pipe_t::bulk_write(const void *data, unsigned len, unsigned timeout)
{
    impl_->bulk_write(data,len,timeout);
}

void pic::usbdevice_t::set_power_delegate(power_t *p)
{
    impl_->power = p;
}

void pic::usbdevice_t::detach()
{
    impl_->detach();
}

const char *pic::usbdevice_t::name()
{
    return impl_->name_.c_str();
}

pic::usbdevice_t::iso_out_guard_t::iso_out_guard_t(usbdevice_t *d): impl_(d->impl()), current_(0)
{
}

pic::usbdevice_t::iso_out_guard_t::~iso_out_guard_t()
{
}

unsigned char *pic::usbdevice_t::iso_out_guard_t::advance()
{
    return current_;
}

pic::usbenumerator_t::impl_t::impl_t(unsigned short v, unsigned short p, const f_string_t &a, const f_string_t &r): pic::thread_t(0), vendor_(v), product_(p), added_(a), removed_(r)
{
}

pic::usbenumerator_t::impl_t::~impl_t()
{
    tracked_invalidate();
    stop();
}

static void attached__(void *ss_, unsigned short vid, unsigned short pid, const char *name)
{
    std::set<std::string> *ss = (std::set<std::string> *)ss_;
    ss->insert(name);
}

void pic::usbenumerator_t::impl_t::thread_pass()
{
    std::set<std::string> working;

    working.clear();
    EigenHarp_Enumerate(vendor_,product_,attached__,&working);

    std::set<std::string>::const_iterator i;

    pic::mutex_t::guard_t g(lock_);

    for(i=devices_.begin(); i!=devices_.end(); i++)
    {
        if(working.find(*i)==working.end())
        {
            printf("device detached %s\n",i->c_str());

            try
            {
                removed_(i->c_str());
            }
            CATCHLOG()
        }
    }

    for(i=working.begin(); i!=working.end(); i++)
    {
        if(devices_.find(*i)==devices_.end())
        {
            printf("device attached %s\n",i->c_str());

            try
            {
                added_(i->c_str());
            }
            CATCHLOG()
        }
    }

    devices_ = working;
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

pic::usbenumerator_t::usbenumerator_t(unsigned short v, unsigned short p, const f_string_t &a)
{
    impl_ = new impl_t(v,p,a,f_string_t());
}

pic::usbenumerator_t::usbenumerator_t(unsigned short v, unsigned short p, const f_string_t &a, const f_string_t &r)
{
    impl_ = new impl_t(v,p,a,r);
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

static void eh_callback__(void *ctx, unsigned short, unsigned short, const char *dev)
{
    (*(pic::f_string_t *)ctx)(dev);
}

unsigned pic::usbenumerator_t::enumerate(unsigned short vendor, unsigned short product, const f_string_t &callback)
{
    try
    {
        f_string_t f2(callback);
        return EigenHarp_Enumerate(vendor,product,eh_callback__,(void *)&f2);
    }
    CATCHLOG();
    return 0;
}

int pic::usbenumerator_t::gc_traverse(void *v,void *a) const
{
    int r;
    pic::mutex_t::guard_t g(impl_->lock_);
    if((r=impl_->added_.gc_traverse(v,a))!=0) return r;
    if((r=impl_->removed_.gc_traverse(v,a))!=0) return r;
    return 0;
}

int pic::usbenumerator_t::gc_clear()
{
    pic::mutex_t::guard_t g(impl_->lock_);
    impl_->added_.gc_clear();
    impl_->removed_.gc_clear();
    return 0;
}
