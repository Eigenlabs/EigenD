

#include <stdlib.h>
#include <errno.h>
#include <memory>
#include <mbstring.h>

#include <picross/pic_error.h>
#include <picross/pic_log.h>
#include <picross/pic_usb.h>
#include <picross/pic_time.h>
#include <picross/pic_strbase.h>
#include <picross/pic_ilist.h>
#include <picross/pic_windows.h>
#include <initguid.h>
#include <resources/OpenWinDev.h>


#define PACKETS_PER_PIPE	100
#define PACKET_POOL_SIZE	PACKETS_PER_PIPE * 50
#define NUMFRAMES			1
#define BUFFER_LEN			768 * NUMFRAMES

namespace
{
	struct usbbuffer_t: pic::element_t<>, virtual pic::lckobject_t
	{
		unsigned char Buffer[BUFFER_LEN];
		unsigned int Length;
		unsigned long Frame;
		unsigned long long Time;
		unsigned long BytesReceived;
	};

	struct usburb_t: virtual pic::lckobject_t
	{
		OVERLAPPED OverLapped;
		usbbuffer_t	*Data;
	};
	
	struct usbisopipe_t: virtual pic::lckobject_t
	{
		usbisopipe_t(pic::usbdevice_t::impl_t *dev,pic::usbdevice_t::in_pipe_t *pipe);
        ~usbisopipe_t() { CloseHandle(hPipe); }

		pic::usbdevice_t::in_pipe_t *pipe_;
		unsigned name_;
		unsigned size_;
		HANDLE hPipe;
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

struct pic::usbdevice_t::impl_t: virtual pic::lckobject_t
{
	impl_t(const char *, unsigned, pic::usbdevice_t *);
	~impl_t();

	bool poll_pipe(unsigned long long);
	bool add_inpipe(in_pipe_t *p);
    int thread_main();

	void start_pipes();
	void stop_pipes();
    void completed(usburb_t *);
    void submit(usburb_t *);
    void detach();
    void pipe_died();

	static int	thread_stub(int * InitData)	{ return (((pic::usbdevice_t::impl_t *)InitData)->thread_main());}

    HANDLE hDevice;
    HANDLE hIOCP1;
    HANDLE hThreadHandle1;

    std::string name_;
    CRITICAL_SECTION lock_;
    pic::usbdevice_t *device_;
    bool running_;
    bool stopping_;
    unsigned count_;
    unsigned long long frame_;
    pic::usbdevice_t::power_t *power_;
	usbisopipe_t *iso_pipe_;

    std::auto_ptr<usbbuffer_t> buffers_[PACKET_POOL_SIZE];
    std::auto_ptr<usburb_t> urbs_[PACKETS_PER_PIPE];

    char DevicePath[256];
            
    pic::ilist_t<usbbuffer_t> BufferPoolQueue;
    pic::ilist_t<usbbuffer_t> RxDataQueue;

    usbbuffer_t *pop_pool_queue()
    {
        EnterCriticalSection(&lock_);

        usbbuffer_t *buf = BufferPoolQueue.pop_front();

        if(buf)
        {
            LeaveCriticalSection(&lock_);
            return buf;
        }

        buf = RxDataQueue.pop_front();
        LeaveCriticalSection(&lock_);

        return buf;

    }

    usbbuffer_t *pop_data_queue(unsigned long long t)
    {
        EnterCriticalSection(&lock_);
        usbbuffer_t *buf = RxDataQueue.head();

        if(buf)
        {
            if(t && buf->Time>t)
            {
                buf = 0;
            }
            else
            {
                RxDataQueue.remove(buf);
            }
        }

        LeaveCriticalSection(&lock_);
        return buf;
    }

    void add_pool_queue(usbbuffer_t *buf)
    {
        EnterCriticalSection(&lock_);
        BufferPoolQueue.append(buf);
        LeaveCriticalSection(&lock_);
    }

    void add_data_queue(usbbuffer_t *buf)
    {
        EnterCriticalSection(&lock_);
        RxDataQueue.append(buf);
        LeaveCriticalSection(&lock_);
    }

};

usbisopipe_t::usbisopipe_t(pic::usbdevice_t::impl_t *dev,pic::usbdevice_t::in_pipe_t *pipe): pipe_(pipe), name_(pipe->in_pipe_name()), size_(pipe->in_pipe_size())
{
	char buffer[256];
	char PipeName[512];

	sprintf(buffer,"PIPE0%d",name_);
	strcpy(PipeName,dev->DevicePath);
	strcat(PipeName,"\\");
	strcat(PipeName,buffer);

	if (( hPipe = CreateFile(PipeName,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0)) == INVALID_HANDLE_VALUE)
	{
        pic::hurlmsg() << "CreateFile (iso pipe) " << SystemError();
        return;
	}

    if (CreateIoCompletionPort(hPipe,dev->hIOCP1,(ULONG_PTR)this,1) == INVALID_HANDLE_VALUE)
    {
        CloseHandle(hPipe);
        pic::hurlmsg() << "Create Completion Port (iso pipe) " << SystemError();
    }
}

void pic::usbdevice_t::impl_t::pipe_died()
{
    if(!stopping_)
    {
        if(iso_pipe_)
        {
            iso_pipe_->pipe_->pipe_died();
        }
        stopping_ = true;
    }
}

int pic::usbdevice_t::impl_t::thread_main()
{
	OVERLAPPED *OverLapped;
	usburb_t *Urb;
	unsigned long Spare;
	unsigned long IoSize;
	int Success;

    for(;;)
    {
        if(stopping_)
        {
            EnterCriticalSection(&lock_);

            if(count_==0)
            {
                LeaveCriticalSection(&lock_);
                pic::logmsg() << "shutting down; urb queue is empty";
                return 0;
            }

            LeaveCriticalSection(&lock_);
        }

		if ((Success = GetQueuedCompletionStatus(hIOCP1,&IoSize,&Spare,&OverLapped,5000)) != 0)
        {
            Urb = PIC_STRBASE(usburb_t,OverLapped,OverLapped);

            if(!Urb)
            {
                pic::logmsg() << "null overlapped from read";
                continue;
            }

            Urb->Data->Length = IoSize;

            EnterCriticalSection(&lock_);
            count_--;
            LeaveCriticalSection(&lock_);

            completed(Urb);
            submit(Urb);
        }
        else
        {
            SystemError e;

            if (e.error==0 || e.error==WAIT_TIMEOUT) // timed out
            {
                if(stopping_)
                {
                    pic::logmsg() << "timed out; count=" << count_;
                }

                continue;
            }

            Urb = PIC_STRBASE(usburb_t,OverLapped,OverLapped);

            if(!stopping_)
            {
                pic::logmsg() << "Usb read: " << e << ' ' << (void *)Urb;
            }

            if(Urb)
            {
                EnterCriticalSection(&lock_);
                count_--;
                LeaveCriticalSection(&lock_);
                add_pool_queue(Urb->Data);
            }

            pipe_died();
        }
    }

	return 0;
}

pic::usbdevice_t::impl_t::~impl_t()
{
    detach();
	CloseHandle(hDevice);
    DeleteCriticalSection(&lock_);
}

void pic::usbdevice_t::impl_t::completed(usburb_t *urb)
{
    if(!stopping_)
    {
        if(urb->Data->Length)
        {
            urb->Data->Frame = frame_++;
            urb->Data->Time = pic_microtime();
            add_data_queue(urb->Data);
        }
        else
        {
			add_pool_queue(urb->Data );
        }

        urb->Data = 0;
    }
}

void pic::usbdevice_t::impl_t::submit(usburb_t *urb)
{
    usbbuffer_t *buf;
    ULONG br;

    buf = pop_pool_queue();

    if(!buf)
    {
        pic::logmsg() << "out of buffers";
        pipe_died();
    }

    buf->Length = BUFFER_LEN;
    memset(buf->Buffer,0,buf->Length);
    memset(&urb->OverLapped,0,sizeof(urb->OverLapped));
    urb->Data = buf;

    if (ReadFile(iso_pipe_->hPipe,urb->Data->Buffer,urb->Data->Length,&br,&urb->OverLapped) == 0)
    {
        SystemError e;

        if(e.error==ERROR_IO_PENDING)
        {
            EnterCriticalSection(&lock_);
            count_++;
            LeaveCriticalSection(&lock_);
        }
        else
        {
            pic::logmsg() << "submitting urb: " << e;
            pipe_died();
        }
    }
    else
    {
        pic::logmsg() << "urb completed immediately " << br;
        urb->Data->Length = br;
        completed(urb);
        submit(urb);
    }
}

void pic::usbdevice_t::impl_t::start_pipes()
{
    if(!running_ && iso_pipe_)
    {
        unsigned long WorkerThreadId;
        stopping_ = false;

        if ((hThreadHandle1 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)thread_stub,this,CREATE_SUSPENDED,&WorkerThreadId)) != NULL)
        {
            SetThreadPriority(hThreadHandle1,THREAD_PRIORITY_TIME_CRITICAL);
            ResumeThread(hThreadHandle1); // Setup so start thread
            running_ = true;
        }			

        iso_pipe_->pipe_->pipe_started();

        for(unsigned i=0;i<PACKETS_PER_PIPE;i++)
        {
            submit(urbs_[i].get());
        }
    }
}

void pic::usbdevice_t::impl_t::stop_pipes()
{
    stopping_ = true;
    WaitForSingleObject(hThreadHandle1,INFINITE);
    running_ = false;
}

void pic::usbdevice_t::impl_t::detach()
{
    pic::logmsg() << "detaching client";
    stop_pipes();

    if(iso_pipe_)
    {
        delete iso_pipe_;
        iso_pipe_ = 0;
    }

	power_ = 0;
    pic::logmsg() << "done detaching client";
}

pic::usbdevice_t::impl_t::impl_t(const char *name, unsigned iface, pic::usbdevice_t *dev) : device_(dev), running_(false), stopping_(false), count_(0), frame_(0), iso_pipe_(0), power_(0)
{
    InitializeCriticalSectionAndSpinCount(&lock_,500);

    for(unsigned i=0;i<PACKETS_PER_PIPE;i++)
    {
        urbs_[i] = std::auto_ptr<usburb_t>(new usburb_t);
    }
	
    for(unsigned i=0;i<PACKET_POOL_SIZE;i++)
    {
        buffers_[i] = std::auto_ptr<usbbuffer_t>(new usbbuffer_t());
        add_pool_queue(buffers_[i].get());
    }

	if ((hDevice = CreateFile (name,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL)) == INVALID_HANDLE_VALUE)
	{
        pic::hurlmsg() << "Opening usb device " << name << ' ' << SystemError();
    }

    pic::logmsg() << "Opened Usb Device " << name;
    strcpy(DevicePath,name);

    if ((hIOCP1 = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1)) == NULL)
    {
        CloseHandle(hDevice);
        pic::hurlmsg() << "creating IO completion port" << SystemError();
    }
}

bool pic::usbdevice_t::impl_t::add_inpipe(in_pipe_t *p)
{
    if(iso_pipe_)
    {
		pic::logmsg() << "cannot add pipe: exists";
        return false;
    }

	try
	{
		iso_pipe_ = new usbisopipe_t(this,p);
		return true;
	}
    CATCHLOG()

    return false;
}

bool pic::usbdevice_t::add_inpipe(in_pipe_t *p)
{
	return impl_->add_inpipe(p);	
}

void pic::usbdevice_t::set_outpipe(out_pipe_t *p)
{
}

bool pic::usbdevice_t::impl_t::poll_pipe(unsigned long long t)
{
	usbbuffer_t *Data = NULL;

	while((Data=pop_data_queue(t)) != 0 )
	{	
		if( Data->Length == 384 )
		{
            iso_pipe_->pipe_->call_pipe_data(Data->Buffer,Data->Length,Data->Frame,Data->Time,Data->Time);
		}

		add_pool_queue( Data );
	}

	return false;
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
	EL_USB_VENDOR_MSG request;
	unsigned long Returnred = 0;

	memset( &request,0,sizeof( request ));

	request.Request_Type = type;
	request.Request = req;
	request.Value = val;
	request.Index = ind;
	request.Length = len;

	if (DeviceIoControl(impl_->hDevice,IOCTL_EIGENHARP_GET_BUFFER,&request,sizeof(EL_USB_VENDOR_MSG),buffer,len,&Returnred,0)==0)
	{
        pic::logmsg() << "control_in " << SystemError();
	}
}

void pic::usbdevice_t::control_out(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, const void *buffer, unsigned len, unsigned timeout)
{
    unsigned char request[128+sizeof(EL_USB_VENDOR_MSG)];
    EL_USB_VENDOR_MSG *reqhdr = (EL_USB_VENDOR_MSG *)request;
	unsigned long Returnred = 0;

	memset(request,0,sizeof(EL_USB_VENDOR_MSG));
	reqhdr->Request_Type = type;
	reqhdr->Request = req;
	reqhdr->Value = val; //address
	reqhdr->Index = ind; // index 0
	reqhdr->Length = len;
    memcpy(&request[sizeof(EL_USB_VENDOR_MSG)],buffer,len);
		
	if( DeviceIoControl(impl_->hDevice,IOCTL_EIGENHARP_SET_BUFFER,request,len+sizeof(EL_USB_VENDOR_MSG),NULL,0,&Returnred,0)==0)
	{
        pic::logmsg() << "control_out " << SystemError();
	}
}

void pic::usbdevice_t::control(unsigned char type, unsigned char req, unsigned short val, unsigned short ind, unsigned timeout)
{
	EL_USB_VENDOR_MSG request;
	unsigned long Returnred = 0;

	memset(&request,0,sizeof(request));
	request.Request_Type = type;
	request.Request = req;
	request.Value = val; //address
	request.Index = ind; // index 0
	request.Length = 0;
		
	if (DeviceIoControl(impl_->hDevice,IOCTL_EIGENHARP_SET_BUFFER,&request,sizeof(EL_USB_VENDOR_MSG),NULL,0,&Returnred,0)==0)
	{
        pic::logmsg() << "control " << SystemError();
	}
	
}

void pic::usbdevice_t::bulk_write(unsigned name, const void *data, unsigned len, unsigned timeout)
{
}

void pic::usbdevice_t::set_power_delegate(power_t *p)
{
    impl_->power_ = p;
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
