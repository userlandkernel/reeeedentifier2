/*

  * Todos Systems AB SmartCard Reader
  * ABN AMRO e.dentifier2

  This code is proprietary property of Todos Systems AB.
  Reverse engineering is not a crime.
  This code does not represent any work of Todos Systems AB and serves academic purpose only.
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>
#include <mach/mach.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

mach_port_t masterPort = 0;

int UsbIo_GetDevice(io_iterator_t usbDeviceRef, void*, void*)
{

}

kern_return_t UsbIo_Open(uint64_t a1, uint64_t a2, const char *deviceName)
{
	kern_return_t err = KERN_SUCCESS;
	CFMutableDictionaryRef matchingDict = 0;
	CFNumberRef numberRef;
	io_iterator_t iterator = 0;
	io_service_t usbDeviceRef;
	mach_port_t masterPort;
	if(deviceName)
	{
		if(strncmp("ABN AMRO e.dentifier2", deviceName, 0x15))
		{
			fprintf(stderr, "%s:%d:%s() Unknown reader.\n", __FILE__, __LINE__, __func__);
			return 0;
		}
	}
	err = IOMasterPort(MACH_PORT_NULL, &masterPort);
	if(err)
	{
		printf("USBSimpleExample: could not create master port, err = %08x\n", err);
		return err;
	}
	matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
	if(!matchingDict)
	{
		printf("USBSimpleExample: could not create matching dictionary.\n");
		return -1;
	}
	numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ABNA_VID);
	if(!numberRef)
	{
		printf("USBSimpleExample: could not create CFNumberRef for vendor\n");
		return -1;
	}
	CFDictionaryAddValue(matchingDict, CFSTR(kUSBVendorID), numberRef);
	CFRelease(numberRef);
	numberRef = 0;
	numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ABNA_PID);
	if(!numberRef)
	{
		printf("USBSimpleExample: could not create CFNumberRef for product\n");
		return -1;
	}
	CFDictionaryAddValue(matchingDict, CFSTR(kUSBProductID), numberRef);
	CFRelease(numberRef);
	numberRef = 0;

	err = IOServiceGetMatchingServices(masterPort, matchingDict, &iterator);
	matchingDict = 0;

	while( (usbDeviceRef = IOIteratorNext(iterator)) )
	{	
		int openRet = UsbIo_GetDevice(usbDeviceRef, NULL, NULL);
		if(openRet != 2)
		{
			if (!openRet)
			{
				IOObjectRelease(iterator);
				iterator = NULL;
				mach_port_deallocate(mach_task_self(), masterPort);
				masterPort = MACH_PORT_NULL;
				return KERN_FAILURE;
			}
		}
		IOObjectRelease(usbDeviceRef);
		usbDeviceRef = NULL;
	}
	IOObjectRelease(iterator);
	iterator = 0;

	mach_port_deallocate(mach_task_self(), masterPort);
	return KERN_SUCCESS;
}

