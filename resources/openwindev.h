
#ifdef  __cplusplus
extern "C" {
#endif

typedef struct EIGENHARP_USB_PACKET_HDR
{
    ULONG64 Time;
	ULONG Frame;
	ULONG Length;
} EIGENHARP_USB_PACKET_HDR, *PEIGENHARP_USB_PACKET_HDR;

typedef struct EIGENHARP_USB_VENDOR_MSG
{
	unsigned char	Ver;		// set to 0 
	unsigned char	Request_Type;
	unsigned char	Request;	// USB Vendor rquest 0 - 255
	unsigned short	Value;
	unsigned short	Index;
	unsigned short	Length;
} EIGENHARP_USB_VENDOR_MSG, *PEIGENHARP_USB_VENDOR_MSG;

#define IOCTL_EIGENHARP_GET_CONFIG_DESCRIPTOR	CTL_CODE(FILE_DEVICE_UNKNOWN,0,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_RESET_DEVICE			CTL_CODE(FILE_DEVICE_UNKNOWN,1,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_RESET_PIPE				CTL_CODE(FILE_DEVICE_UNKNOWN,2,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_GET_BUFFER				CTL_CODE(FILE_DEVICE_UNKNOWN,3,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_SET_BUFFER				CTL_CODE(FILE_DEVICE_UNKNOWN,4,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_GET_SPEED     			CTL_CODE(FILE_DEVICE_UNKNOWN,5,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_SET_ADVANCED  			CTL_CODE(FILE_DEVICE_UNKNOWN,6,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_GET_VERSION   			CTL_CODE(FILE_DEVICE_UNKNOWN,7,METHOD_BUFFERED,FILE_ANY_ACCESS)

#ifndef OPENWINDEV_DRIVER

#ifdef OPENWINDEV_EXPORTING
   #define OPENWINDEV_DECLSPEC    __declspec(dllexport)
#else
   #define OPENWINDEV_DECLSPEC    __declspec(dllimport)
#endif

OPENWINDEV_DECLSPEC int __cdecl EigenHarp_Enumerate(unsigned short vid, unsigned short pid, void (__cdecl *visitor)(void *,unsigned short,unsigned short,const char *), void *);
OPENWINDEV_DECLSPEC const char * __cdecl EigenHarp_CurrentVersion(void);

#endif


#ifdef  __cplusplus
}
#endif
