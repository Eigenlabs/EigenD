
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


#include <stdlib.h>
#include <errno.h>
#include <memory>
#include <mbstring.h>

#include <picross/pic_error.h>
#include <picross/pic_log.h>
#include <picross/pic_usb.h>
#include <picross/pic_time.h>
#include <picross/pic_stack.h>
#include <picross/pic_ilist.h>
#include <picross/pic_windows.h>
#include <initguid.h>
#include <resources/OpenWinDev.h>


//#define PACKETS_PER_PIPE	32

#define PACKETS_PER_PIPE	100
#define PACKET_POOL_SIZE	PACKETS_PER_PIPE * 50

//#define NUMFRAMES			8
//#define BUFFER_LEN			512 * NUMFRAMES //for alpha

#define NUMFRAMES			1
#define BUFFER_LEN			768 * NUMFRAMES

#define OUTPIPE_IDLE 0
#define OUTPIPE_BUSY 1
#define OUTPIPE_START 2
#define OUTPIPE_SHUTDOWN 3

namespace
{
	struct usbpipe_t;
	struct usburb_t;
	struct usbbulkpipe_t;
	struct usburb_t;

	typedef struct UsbBuffer_t : pic::element_t<>
	{
		unsigned char			Buffer[BUFFER_LEN];
		unsigned int			Length;
		unsigned long			Frame;
		unsigned long long			Time;
		pic::usbdevice_t::in_pipe_t *   Pipe;
	}UsbBuffer_t, *pUsbBuffer_t;

	struct usbpacket_t
	{	
		OVERLAPPED				OverLapped;	// MUST BE FIRST
		struct UsbBuffer_t	*	Data;
		unsigned int			Piperef;
		usburb_t			*	Urb;
	};

	struct win32_usbdevice_t
	{
		win32_usbdevice_t(const char *, unsigned, pic::usbdevice_t *);
		~win32_usbdevice_t();

		HANDLE fd() { return hDevice; }
		HANDLE	hDevice;
		HANDLE	hIOCP1;
		HANDLE	hIOCP2;
		HANDLE	hThreadHandle1;
		HANDLE	hThreadHandle2;
		char DevicePath[256];
				
		pic::ilist_t<UsbBuffer_t> BufferPoolQueue;
		pic::ilist_t<UsbBuffer_t> RxDataQueue;

        UsbBuffer_t *pop_pool_queue()
        {
            EnterCriticalSection(&lock_);
            UsbBuffer_t *buf = BufferPoolQueue.pop_front();
            LeaveCriticalSection(&lock_);
            return buf;
        }

        UsbBuffer_t *pop_data_queue(unsigned long long t)
        {
            EnterCriticalSection(&lock_);
            UsbBuffer_t *buf = RxDataQueue.head();
            if(buf)
            {
                if(buf->Time>t)
                    buf = 0;
                else
                    RxDataQueue.remove(buf);
            }

            LeaveCriticalSection(&lock_);
            return buf;
        }

        void add_pool_queue(UsbBuffer_t *buf)
        {
            EnterCriticalSection(&lock_);
            BufferPoolQueue.append(buf);
            LeaveCriticalSection(&lock_);
        }

        void add_data_queue(UsbBuffer_t *buf)
        {
            EnterCriticalSection(&lock_);
            RxDataQueue.append(buf);
            LeaveCriticalSection(&lock_);
        }

        CRITICAL_SECTION lock_;

		pic::usbdevice_t *device_;
		bool stopping_;

		int RunThread1();
		int RunThread2();
		void start_thread();

		struct usbbulkpipe_t * BulkPipe;

		std::auto_ptr<UsbBuffer_t> buffers_[PACKET_POOL_SIZE];
	};

	int	WorkThread1(int * InitData)	{return(((win32_usbdevice_t*)InitData)->RunThread1());}
	int	WorkThread2(int * InitData)	{return(((win32_usbdevice_t*)InitData)->RunThread2());}

	struct usbpipe_t
	{
		usbpipe_t(win32_usbdevice_t *dev,unsigned name,unsigned size);
		virtual ~usbpipe_t();

		void submit(usburb_t *);
		unsigned count() { return count_; }
		unsigned name() { return piperef_; }
		unsigned size() { return size_;} 
		virtual void completed(usburb_t *) = 0;
		virtual void pipe_died() {}
		unsigned get() { return piperef_; }
		win32_usbdevice_t *device_;
		unsigned piperef_;
		unsigned size_;
		volatile int count_;
        pic_atomic_t killed_;
		HANDLE hPipe;
	};

	struct usbbulkpipe_t: usbpipe_t
	{
		usbbulkpipe_t(win32_usbdevice_t *dev,unsigned name,unsigned size):usbpipe_t(dev,name,size)
		{
			;
		}

		virtual void completed(usburb_t *u)
		{
		}

		void bulk_write(unsigned name, const void *data, unsigned len, unsigned timeout);
		void OpenPipe();
		void start();

	};
	
	struct usburb_t: pic::stacknode_t
	{
		usburb_t(usbpipe_t *pipe, unsigned pktsize);
		virtual ~usburb_t();

		void completed() { pipe->completed(this); }
		void submit() { pipe->submit(this); }

		usbpipe_t *pipe;
		usbpacket_t Packet;

		unsigned long BytesReceived;
	};
	
	struct usbpipe_in_t: usbpipe_t
	{
		usbpipe_in_t(win32_usbdevice_t *dev, pic::usbdevice_t::in_pipe_t *pipe): usbpipe_t(dev,pipe->in_pipe_name(),pipe->in_pipe_size()), pipe_(pipe), frame_(0ULL)
		{
			for(unsigned i=0;i<PACKETS_PER_PIPE;i++)
			{
				urbs_[i] = std::auto_ptr<usburb_t>(new usburb_t(this,size_));
			}

		}

		void pipe_died() { pipe_->pipe_died(); }
		virtual void completed(usburb_t *);
		void start();
		void OpenPipe();
				
		pic::usbdevice_t::in_pipe_t *pipe_;
		unsigned long long frame_;
		std::auto_ptr<usburb_t> urbs_[PACKETS_PER_PIPE];
	};

	struct usbpipe_out_t: usbpipe_t, pic::stack_t
	{
		usbpipe_out_t(win32_usbdevice_t *dev, pic::usbdevice_t::out_pipe_t *pipe): usbpipe_t(dev,pipe->out_pipe_name(),pipe->out_pipe_size()),shutdown_latch(OUTPIPE_START)
		{
			for(unsigned i=0; i<PACKETS_PER_PIPE; i++)
			{
				urbs_[i] = std::auto_ptr<usburb_t>(new usburb_t(this,size_));
				free_.push(urbs_[i].get());
			}
		}

		virtual void completed(usburb_t *u)
		{
			free_.push(u);
		}

		pic::stack_t free_;
		std::auto_ptr<usburb_t> urbs_[PACKETS_PER_PIPE];
        pic_atomic_t shutdown_latch;
	};

	typedef pic::lcklist_t<usbpipe_in_t *>::lcktype pipe_list_t;
	typedef pic::flipflop_t<pipe_list_t> pipe_flipflop_t;
   
};

struct pic::usbdevice_t::impl_t
{
	impl_t(const char *, unsigned, pic::usbdevice_t *);
	~impl_t();

	bool poll_pipe(unsigned long long);
	bool add_inpipe(in_pipe_t *p);
	void set_outpipe(out_pipe_t *p);
	void start_pipes();
    void detach(bool call_died);

	win32_usbdevice_t device_;
    pic::usbdevice_t::power_t *power;
    std::string name_;

	pipe_flipflop_t inpipes_;
	std::auto_ptr<usbpipe_out_t> pipe_out_;
	in_pipe_t *key_data_pipe_;
};

usburb_t::usburb_t(usbpipe_t *p, unsigned s): pipe(p)
{
	memset(&Packet,0,sizeof(usbpacket_t));
	BytesReceived=0;
}

usburb_t::~usburb_t()
{
}

int  OutputError(const char * aErrString)
{
	unsigned long	error;
	char Buffer[512];

	error = GetLastError();

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,error,LANG_NEUTRAL,Buffer,256,NULL);

    pic::logmsg() << aErrString << ": " << error << " -> " << Buffer;
	return error;
}

int win32_usbdevice_t::RunThread1()
{
	OVERLAPPED	*	OverLapped;
	usbpacket_t	*	PacketCompletion;
	unsigned long	Spare;
	unsigned long	IoSize;
	int 			Success;

	while(stopping_ == false)
	{
		if ((Success = GetQueuedCompletionStatus(hIOCP1,&IoSize,&Spare,&OverLapped,20000/*INFINITE*/)) != 0)
		{
			if ((PacketCompletion = (usbpacket_t *)OverLapped) != NULL)
			{
				PacketCompletion->Data->Length = IoSize;
				PacketCompletion->Urb->pipe->completed(PacketCompletion->Urb);
			}
			else
			{
				pic::logmsg() << "!!!!!!!!!! data NULL !!!!!!!!!!\n";
				//_ASSERT(0);
			}
		}
		else
		{			
            unsigned long error;
			if ((error=OutputError("GetQueuedCompletionStatus() ")) != 258) // timed out
			{
				if( error==ERROR_GEN_FAILURE || error==ERROR_OPERATION_ABORTED) //attached device not functioning
				{
					device_->impl()->detach(true);
					stopping_ = true;					
				}
				else
				{
					pic::logmsg() << "OverLapped (" << OverLapped << ")";
					//_ASSERT(IoSize==0);
				}
			}
		}
	}

	return(0);
}

void usbpipe_in_t::completed(usburb_t *urb)
{
	count_--;
	
	if(device_ && !device_->stopping_)
	{
		if(urb->Packet.Data->Length)
		{
			urb->Packet.Data->Frame = frame_++;
			urb->Packet.Data->Time = pic_microtime();
			urb->Packet.Data->Pipe = pipe_;

			//device_->RxDataQueue.append( (UsbBuffer_t*)urb->Packet.Data );
			device_->add_data_queue( (UsbBuffer_t*)urb->Packet.Data );
		}	
		else
		{
			//device_->BufferPoolQueue.append( (UsbBuffer_t*)urb->Packet.Data );
			device_->add_pool_queue( (UsbBuffer_t*)urb->Packet.Data );
		}

		urb->submit();
	}
}

void usbpipe_t::submit(usburb_t *urb)
{
	if(device_ && urb->pipe->device_)
	{
		//while((urb->Packet.Data=(UsbBuffer_t*)urb->pipe->device_->BufferPoolQueue.head()) != 0 )
		while((urb->Packet.Data=(UsbBuffer_t*)urb->pipe->device_->pop_pool_queue()) != 0 )
		{
			urb->Packet.Urb = urb;
			//urb->pipe->device_->BufferPoolQueue.remove( urb->Packet.Data );

			if (urb->Packet.Data)
			{
				urb->Packet.Data->Length = BUFFER_LEN;

				if (ReadFile(hPipe,urb->Packet.Data->Buffer,urb->Packet.Data->Length,&urb->BytesReceived,(OVERLAPPED*)&urb->Packet) == 0)
				{
					if (GetLastError() == ERROR_IO_PENDING)
					{
						count_++;
						return;
					}
					else
					{
						OutputError("ReadFile() ");
						//_ASSERT(0); // catch if debug build
					}
				}
				else
				{
					pic::logmsg() << "* Completed in ReadPacket() NOT EXPECTED *"; // Expecting it to complete in IOCP thread
					//_ASSERT(0);
				}
			}
			else
			{
				pic::logmsg() << "Bad data buffer";
				//_ASSERT(0);
			}

			pic::logmsg() << "can't submit frame " << errno;
			pipe_died();
			device_=0;
		}
	}
}

void usbpipe_in_t::OpenPipe()
{
	char buffer[256];

	sprintf(buffer,"PIPE0%d",piperef_);

	char PipeName[512];

	strcpy(PipeName,device_->DevicePath);
	strcat(PipeName,"\\");
	strcat(PipeName,buffer);

	if (( hPipe = CreateFile(PipeName,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0))!= INVALID_HANDLE_VALUE)
	{
		static ULONG_PTR t = piperef_;

		if ((device_->hIOCP1 = CreateIoCompletionPort(hPipe,device_->hIOCP1,t,1)) != INVALID_HANDLE_VALUE)
		{
		}
		else
		{
			OutputError("CreateIoCompletionPort() ");
		}
	}
	else
	{
		OutputError("CreateFile() ");
	}
}

void usbpipe_in_t::start()
{
	count_=0;

	OpenPipe();
	
	for(unsigned i=0;i<PACKETS_PER_PIPE;i++)
	{
		submit(urbs_[i].get());
	}
}

usbpipe_t::usbpipe_t(win32_usbdevice_t *dev, unsigned name, unsigned size): device_(dev),killed_(0)
{
	pic::logmsg() << "Name and size:" << name << " " << size;
	piperef_=name;
	size_=size;
}

usbpipe_t::~usbpipe_t()
{
}

void usbbulkpipe_t::OpenPipe()
{
	char buffer[256];

	sprintf(buffer,"PIPE0%d",piperef_);

	char PipeName[512];

	strcpy(PipeName,device_->DevicePath);
	strcat(PipeName,"\\");
	strcat(PipeName,buffer);

	if (( hPipe = CreateFile(PipeName,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0))!= INVALID_HANDLE_VALUE)
	{
		static ULONG_PTR t = piperef_;

		if ((device_->hIOCP2 = CreateIoCompletionPort(hPipe,device_->hIOCP2,t,1)) != INVALID_HANDLE_VALUE)
		{
		}
		else
		{
			OutputError("CreateIoCompletionPort() ");
		}
	}
	else
	{
		OutputError("CreateFile() ");
	}
}

void usbbulkpipe_t::bulk_write(unsigned name, const void *data, unsigned len, unsigned timeout)
{
	unsigned long BytesSent =  0;
	OVERLAPPED overlapped;

	memset(&overlapped,0,sizeof(OVERLAPPED));

	name;

	if (WriteFile(hPipe,data,len,&BytesSent,(OVERLAPPED*)&overlapped) == 0)
	{
		;
	}

}

void usbbulkpipe_t::start()
{
	count_=0;
	OpenPipe();
}

void win32_usbdevice_t::start_thread()
{
	unsigned long WorkerThreadId;

	if ((hThreadHandle1 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)WorkThread1,this,CREATE_SUSPENDED,&WorkerThreadId)) != NULL)
	{
		//SetThreadPriority(hThreadHandle1,THREAD_PRIORITY_HIGHEST);
        SetThreadPriority(hThreadHandle1,THREAD_PRIORITY_TIME_CRITICAL);

		for(unsigned i=0;i<PACKET_POOL_SIZE;i++)
		{
			struct UsbBuffer_t * buff = new UsbBuffer_t();
			buffers_[i] = std::auto_ptr<UsbBuffer_t>(buff);
			//BufferPoolQueue.append( buff );
            add_pool_queue(buff);
		}
	}			
	//moving this code where we add in pipe for reading key data 
	ResumeThread(hThreadHandle1); // Setup so start thread

}

int win32_usbdevice_t::RunThread2()
{
	OVERLAPPED	*	OverLapped;
	unsigned long	Spare;
	unsigned long	IoSize;
	int 			Success;

	while(stopping_ == false)
	{
		if ((Success = GetQueuedCompletionStatus(hIOCP2,&IoSize,&Spare,&OverLapped,INFINITE)) != 0)
		{
		}
		else
		{
			//printf("Read Error (%x)\n",OverLapped);//
			OutputError("GetQueuedCompletionStatus() ");
			//_ASSERT(IoSize==0);
		}
	}

	return 0;
}

win32_usbdevice_t::win32_usbdevice_t(const char *name, unsigned iface, pic::usbdevice_t *dev) : device_(dev), stopping_(false)
{
    InitializeCriticalSection(&lock_);
	
	if ((hDevice = CreateFile (name,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL)) != INVALID_HANDLE_VALUE)
	{
		pic::logmsg() << "Opened Usb Device " << name;

		strcpy(DevicePath,name);

		if ((hIOCP1 = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1)) != NULL)
		{
		}
		else
		{
			OutputError("CreateIoCompletionPort() ");
		}
	}
	else
	{
		hDevice = NULL;
		OutputError("CreateFile() ");
	}
}


win32_usbdevice_t::~win32_usbdevice_t()
{
	stopping_ = true;
	CloseHandle(hDevice);
	
	Sleep(1000); // let the pipes close before we release the memory
    DeleteCriticalSection(&lock_);
}

pic::usbdevice_t::impl_t::impl_t(const char *name, unsigned iface, pic::usbdevice_t *dev):  device_(name,iface,dev), pipe_out_(0)
{
}

bool pic::usbdevice_t::add_inpipe(in_pipe_t *p)
{
	//thread one here handling key data 
	impl_->device_.start_thread();
	return impl_->add_inpipe(p);	
}

void pic::usbdevice_t::set_outpipe(out_pipe_t *p)
{
	impl_->set_outpipe(p);
}

bool pic::usbdevice_t::impl_t::add_inpipe(in_pipe_t *p)
{
	try
	{
		usbpipe_in_t *pipe  = new usbpipe_in_t(&device_,p);
		inpipes_.alternate().push_back(pipe);
		inpipes_.exchange();
		key_data_pipe_ = p;
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

void pic::usbdevice_t::impl_t::set_outpipe(out_pipe_t *p)
{
	try
	{
		pipe_out_  = std::auto_ptr<usbpipe_out_t>(new usbpipe_out_t(&device_,p));
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
}

bool pic::usbdevice_t::impl_t::poll_pipe(unsigned long long t)
{
	// add poll pipe code here
	UsbBuffer_t *Data = NULL;
	/*if( device_ == 0 )
		return;*/

	//while((Data=(UsbBuffer_t*)device_.RxDataQueue.head()) != 0 )
	while((Data=(UsbBuffer_t*)device_.pop_data_queue(t)) != 0 )
	{	
		//device_.RxDataQueue.remove( Data );
		if( Data->Length == 384 )
		{
            Data->Pipe->call_pipe_data(Data->Buffer,Data->Length,Data->Frame,Data->Time,Data->Time);
		}
		else
		{
		}

		//device_.BufferPoolQueue.append( Data );
		device_.add_pool_queue( Data );
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

void pic::usbdevice_t::impl_t::start_pipes()
{
	device_.stopping_ = false;
	pipe_list_t::const_iterator i;
	
	for(i=inpipes_.alternate().begin(); i!=inpipes_.alternate().end(); i++)
	{
		(*i)->pipe_->pipe_started();	// send start pipe deviocts
		(*i)->start();				
	}
	
	pic::logmsg() << "pipes started!" ; 
}

void pic::usbdevice_t::impl_t::detach(bool call_died)
{
    pic::logmsg() << "detaching client";
	while( inpipes_.alternate().size() > 0 )
	{
		usbpipe_in_t *p = inpipes_.alternate().front();
		inpipes_.alternate().pop_front();
        inpipes_.exchange();
        if(call_died)
        {
            p->pipe_died();
        }
        delete p;		
	}

	power = 0;
	//device_ = NULL;
    pic::logmsg() << "done detaching client";
}

void pic::usbdevice_t::stop_pipes()
{
	impl_->device_.stopping_ = true;
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

	if (DeviceIoControl(impl_->device_.fd(),IOCTL_EIGENHARP_GET_BUFFER,&request,sizeof(EL_USB_VENDOR_MSG),buffer,len,&Returnred,0)==0)
	{
		OutputError("can't do control_in request");
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
		
	if( DeviceIoControl(impl_->device_.fd(),IOCTL_EIGENHARP_SET_BUFFER,request,len+sizeof(EL_USB_VENDOR_MSG),NULL,0,&Returnred,0)==0)
	{
		OutputError("can't do control_out request");
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
		
	if (DeviceIoControl(impl_->device_.fd(),IOCTL_EIGENHARP_SET_BUFFER,&request,sizeof(EL_USB_VENDOR_MSG),NULL,0,&Returnred,0)==0)
	{
		OutputError("can't do control request");
	}
	
}

void pic::usbdevice_t::bulk_write(unsigned name, const void *data, unsigned len, unsigned timeout)
{
	if (name == 1)//BCTKBD_BULKOUT_EP)
	{
		impl_->device_.BulkPipe->bulk_write(name,data,len,timeout);
	}
	else
		PIC_THROW("bad bulk write address");

}

void pic::usbdevice_t::set_power_delegate(power_t *p)
{
    impl_->power = p;
}

void pic::usbdevice_t::detach()
{
    impl_->detach(false);
}

const char *pic::usbdevice_t::name()
{
    return impl_->name_.c_str();
}

pic::usbdevice_t::iso_out_guard_t::iso_out_guard_t(usbdevice_t *d): impl_(d->impl()), current_(0)
{
    if(!impl_->pipe_out_.get() )
    {
        return;
    }

    if(impl_->pipe_out_->killed_)
    {
        return;
    }

    if(!pic_atomiccas(&impl_->pipe_out_->shutdown_latch,OUTPIPE_IDLE,OUTPIPE_BUSY))
    {
        return;
    }
    //todo: make it available like in macosx_usb_device implementation , just commented out for time being to bypass compilation errors.    
    //current_ = impl_->pipe_out_->iso_write_start();
}

pic::usbdevice_t::iso_out_guard_t::~iso_out_guard_t()
{
    /*if(impl_->outpipe)
        pic_atomiccas(&impl_->outpipe->shutdown_latch,OUTPIPE_BUSY,OUTPIPE_IDLE);
    */
}

unsigned char *pic::usbdevice_t::iso_out_guard_t::advance()
{
    /*PIC_ASSERT(impl_->outpipe);
    current_=impl_->outpipe->iso_write_advance(ISO_OUT_STRIDE);
    */
    return current_;
}
