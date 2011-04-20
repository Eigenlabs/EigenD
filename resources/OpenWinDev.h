/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com
*/

#include <initguid.h>
DEFINE_GUID(GUID_CLASS_EIGENLABS_ALPHA1_USB,0x873fdf, 0x61a8, 0x11d1, 0xaa, 0x5e, 0x0, 0xc0, 0x4f, 0xb1, 0x72, 0x8b);

#ifdef _EXPORTING
   #define DECLSPEC    __declspec(dllexport)
#else
   #define DECLSPEC    __declspec(dllimport)
#endif

#ifdef  __cplusplus
extern "C" {
#endif

DECLSPEC HANDLE OpenUsbDevice (LPGUID pGuid,PSTR outNameBuf);
extern DECLSPEC int NumberDevicesWin;

#pragma warning ( disable : 4200)
typedef struct EL_USB_VENDOR_MSG
{
	unsigned char	Ver;		// set to 0 
	unsigned char	Request_Type;
	unsigned char	Request;	// USB Vendor rquest 0 - 255
	unsigned short	Value;
	unsigned short	Index;
	unsigned short	Length;
} EL_USB_VENDOR_MSG, *PEL_USB_VENDOR_MSG;
#pragma warning ( default : 4200)

#define IOCTL_EIGENHARP_GET_CONFIG_DESCRIPTOR	CTL_CODE(FILE_DEVICE_UNKNOWN,0,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_RESET_DEVICE			CTL_CODE(FILE_DEVICE_UNKNOWN,1,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_RESET_PIPE				CTL_CODE(FILE_DEVICE_UNKNOWN,2,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_GET_BUFFER				CTL_CODE(FILE_DEVICE_UNKNOWN,3,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_EIGENHARP_SET_BUFFER				CTL_CODE(FILE_DEVICE_UNKNOWN,4,METHOD_BUFFERED,FILE_ANY_ACCESS)


#ifdef  __cplusplus
}
#endif
