/*

  * Todos Systems AB SmartCard Reader
  * ABN AMRO e.dentifier2

  This code is proprietary property of Todos Systems AB.
  Reverse engineering is not a crime.
  This code does not represent any work of Todos Systems AB and serves academic purpose only.

*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <mach/mach.h>
#include <CoreFoundation/Corefoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

uint32_t ABNA_VID = 0x0b0c;
uint32_t ABNA_PID = 0x001e;

#define CMD_POWERON 		0x0000102
#define CMD_POWEROFF 		0x0000B02
#define CMD_FIRSTCHOICE 	0x1001280
#define CMD_GETMODE1		0x1021280
#define CMD_GETMODE2		0x1031280
#define CMD_STATUS			0x0000902

mach_port_t masterPort = MACH_PORT_NULL;
IONotificationPortRef gNotifyPort = NULL;
io_iterator_t gSendAsynReadIter = MACH_PORT_NULL;

typedef struct ED2USBIface {
	io_object_t notif;
	IOUSBDeviceInterface245 **deviceInterface;
	CFStringRef deviceName;
	UInt32 locationID;
} ED2USBIface;

void UsbIo_SetAsynRead(void* refcon, io_iterator_t iter) {
	kern_return_t kr = KERN_SUCCESS;
	io_service_t usbDevice = MACH_PORT_NULL;
	IOCFPlugInInterface **plugInInterface = NULL;
	SInt32 score;

	while( (usbDevice = IOIteratorNext(iter)) ) 
	{
		io_name_t deviceNameObj;
		CFStringRef deviceName;
		kr = IORegistryEntryGetName(usbDevice, deviceNameObj);
		if(KERN_SUCCESS != kr) {
			deviceNameObj[0] = '\0';
		}
		deviceName = CFStringCreateWithCString(kCFAllocatorDefault, deviceNameObj, kCFStringEncodingASCII);
		puts("GOT THE DEVICE w00t.!");
		CFShow(deviceName);
	}

}

void UsbIo_GetInterface(io_service_t iface, io_iterator_t iterator, IOUSBDeviceInterface** dev)
{
	kern_return_t err = KERN_SUCCESS;
	SInt32 score = 0;
	IOCFPlugInInterface **iodev = NULL;
	IOUSBInterfaceInterface300 **dev2 = NULL;
	CFUUIDRef pluginType = CFUUIDGetConstantUUIDWithBytes(
         0LL,
         45LL,
         151LL,
         134LL,
         198LL,
         158LL,
         243LL,
         17LL,
         212LL,
         173LL,
         81LL,
         0LL,
         10LL,
         39LL,
         5LL,
         40LL,
         97LL);
	CFUUIDRef interfaceType = CFUUIDGetConstantUUIDWithBytes(
         0LL,
         194LL,
         68LL,
         232LL,
         88LL,
         16LL,
         156LL,
         17LL,
         212LL,
         145LL,
         212LL,
         0LL,
         80LL,
         228LL,
         198LL,
         66LL,
         111LL);
	err = IOCreatePlugInInterfaceForService(iface, pluginType, interfaceType, &iodev, &score);
	if(err || !iodev) 
	{
		return;
	}
	CFUUIDRef uuid = CFUUIDGetConstantUUIDWithBytes(
         0LL,
         115LL,
         201LL,
         122LL,
         232LL,
         158LL,
         243LL,
         17LL,
         212LL,
         177LL,
         208LL,
         0LL,
         10LL,
         39LL,
         5LL,
         40LL,
         97LL);
	err = (*iodev)->QueryInterface(iodev, CFUUIDGetUUIDBytes(uuid), &dev2);
	if(err ||  !dev2) {
		printf("FAIL MAKE USB IFACE.\n");
	}
	err = (*dev2)->USBInterfaceOpen(dev2);
	if(err) {
		printf("FAIL OPEN\n");
		return;
	}
	UInt8 numEndpoints = 0;
	err = (*dev2)->GetNumEndpoints(dev2, &numEndpoints);
	if(err) {
		printf("FAIL ENDPOINS\n");
		return;
	}
	else {
		printf("EPS: %d\n", numEndpoints);
	}
	if(!numEndpoints) {
		err = (*dev2)->SetAlternateInterface(dev2, 1);
		if(err) {
			printf("FAIL ALTINTF\n");
			return;
		}
		numEndpoints = 0;
		err = (*dev2)->GetNumEndpoints(dev2, &numEndpoints);
		if(err) {
			printf("FAIL ENDPOINS\n");
			return;
		}
		numEndpoints = 13;
	}
	UInt8 numEndpointsBackup = numEndpoints;
	UsbIo_GetPipes(numEndpoints, &numEndpoints, dev2);

	printf("GET INTERFACE CALLED.\n");
}

int UsbIo_GetDevice(io_service_t usbDeviceRef, uint16_t *ioAddress, void *y)
{
	kern_return_t err = KERN_SUCCESS;
	IOCFPlugInInterface **iodev = NULL;
	IOUSBDeviceInterface **dev = NULL;
	SInt32 score = 0;
	CFUUIDRef pluginType = CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault, 157, 199, 183, 128, 158, 192, 17, 212, 165, 79, 0, 10, 39, 5, 40, 97);
	CFUUIDRef interfaceType = CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault, 194, 68, 232, 88, 16, 156, 17, 212, 145, 212, 0, 80, 228, 198, 66, 111);
	err = IOCreatePlugInInterfaceForService(usbDeviceRef, pluginType, interfaceType, &iodev, &score);
	if(err || !iodev) {
		fprintf(stderr, "%s:%d:%s() unable to create plugin. ret = %08x, iodev = %p", __FILE__, __LINE__, __func__, err, iodev);
		return KERN_FAILURE;
	}
	printf("At this time we need to create a CFPluginInterface for USB...\n");
	err = (*iodev)->QueryInterface(iodev, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID*)&dev);
	IODestroyPlugInInterface(iodev);
	if(err || !*(uint64_t*)dev) {
		fprintf(stderr, "%s:%d:%s() unable to create a device interface. ret = %08x, dev = %p\n", __FILE__, __LINE__, __func__, err, *(uint64_t*)&dev);
		return 1;
	}
	USBDeviceAddress devAddr;
	if((*dev)->GetDeviceAddress(dev, &devAddr)) {
		fprintf(stderr, "%s:%d:%s() Can not get the device address.\n", __FILE__, __LINE__, __func__);
		return 1;
	}
	fprintf(stderr, "%s:%d:%s() The address of the device %03i\n", __FILE__, __LINE__, __func__, devAddr);
	err = (*dev)->USBDeviceOpen(dev);
	if(err)
	{
		fprintf(stderr, "%s:%d:%s() Unable to open device. ret = %08x\n", __FILE__, __LINE__, __func__, err);
		return 1;
	}
	UInt8 numConfs = 0;
	err = (*dev)->GetNumberOfConfigurations(dev, &numConfs);
	if (err || !numConfs)
	{
		fprintf(stderr, "%s:%d:%s() Unable to obtain the number of configurations. ret = %08x\n", __FILE__, __LINE__, __func__, err);
		(*dev)->USBDeviceClose(dev);
		(*dev)->Release(dev);
		dev = NULL;
		return 1;
	}
	IOUSBConfigurationDescriptorPtr confDesc;
	if((*dev)->GetConfigurationDescriptorPtr(dev, 0, &confDesc)) {
		fprintf(stderr, "%s:%d:%s() Unable to get config descriptor for index 0.\n", __FILE__, __LINE__, __func__);
		(*dev)->USBDeviceClose(dev);
		(*dev)->Release(dev);
		dev = NULL;
		return 1;
	}
	if((*dev)->SetConfiguration(dev, confDesc->bConfigurationValue)) {
		fprintf(stderr, "%s:%d:%s() Unable to set the configuration.\n", __FILE__, __LINE__, __func__);
		(*dev)->USBDeviceClose(dev);
		(*dev)->Release(dev);
		dev = NULL;
		return 1;
	}
	IOUSBFindInterfaceRequest request = {
		.bInterfaceClass = kIOUSBFindInterfaceDontCare,
		.bInterfaceSubClass = kIOUSBFindInterfaceDontCare,
		.bInterfaceProtocol = kIOUSBFindInterfaceDontCare,
		.bAlternateSetting = kIOUSBFindInterfaceDontCare
	};
	*((uint64_t*)&request) = -1;
	io_iterator_t iterator = MACH_PORT_NULL;
	err = (*dev)->CreateInterfaceIterator(dev, &request, &iterator);
	if(err == KERN_SUCCESS) {
		io_service_t ifaceObj = IOIteratorNext(iterator);
		if(ifaceObj) 
		{
			while(ifaceObj) {
				UsbIo_GetInterface(ifaceObj, iterator, dev);
				IOObjectRelease(ifaceObj);
				ifaceObj = IOIteratorNext(iterator);
			}
		}
		else {
			for(int i = 0; i < 5; i++) {
				fprintf(stderr, "%s:%d:%s() Reader not found. Retry!\n", __FILE__, __LINE__, __func__);
				usleep(0x2710);
				ifaceObj = IOIteratorNext(iterator);
				if(ifaceObj) {
					UsbIo_GetInterface(ifaceObj, iterator, dev);
					IOObjectRelease(ifaceObj);
					ifaceObj = IOIteratorNext(iterator);
					return 0;
				}
			}
			fprintf(stderr, "%s:%d:%s() Reader not found. Give up!\n", __FILE__, __LINE__, __func__);
		}
		IOObjectRelease(iterator);
		fprintf(stderr, "%s:%d:%s() UsbIo_GetDevice OUT\n", __FILE__, __LINE__, __func__);
		(*dev)->USBDeviceClose(dev);
		(*dev)->Release(dev);
		return 0;
	}
	fprintf(stderr, "%s\n", mach_error_string(err));
	fprintf(stderr, "%s:%d:%s() Unable to create interface iterator\n", __FILE__, __LINE__, __func__);
	(*dev)->USBDeviceClose(dev);
	(*dev)->Release(dev);
	dev = NULL;
	return 1;
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
				iterator = 0;
				mach_port_deallocate(mach_task_self(), masterPort);
				masterPort = MACH_PORT_NULL;
				return KERN_FAILURE;
			}
		}
		IOObjectRelease(usbDeviceRef);
		usbDeviceRef = 0;
	}
	IOObjectRelease(iterator);
	iterator = 0;

	mach_port_deallocate(mach_task_self(), masterPort);
	return KERN_SUCCESS;
}


int main(int argc, char *argv[]) {
	mach_port_t masterPort;
	CFMutableDictionaryRef matchingDict;
	CFRunLoopSourceRef runLoopSource;
	CFNumberRef numberRef;
	kern_return_t err = KERN_SUCCESS;
	
	puts("No money? Suck a cock! - Fietsopa.");
	puts("Reverse Engineered by Sem Voigtländer.");
	
	UsbIo_Open(0, 0, NULL);
	if(IOMasterPort(0, &masterPort) || masterPort == MACH_PORT_NULL)
	{
		fprintf(stderr, "ERR: Couldn't create a master IOKit Port.\n");
	}
	else
	{
		CFMutableDictionaryRef matching = IOServiceMatching("IOUSBDevice");
		if(matching) {
			
			CFNumberRef vid = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ABNA_VID);
			CFNumberRef pid = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ABNA_PID);
			CFDictionarySetValue(matching, CFSTR("idVendor"), vid);
			CFRelease(vid);
			vid = NULL;
			CFDictionarySetValue(matching, CFSTR("idProduct"), pid);
			CFRelease(pid);
			pid = NULL;

			gNotifyPort = IONotificationPortCreate(masterPort);
			runLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
			CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
			
			err = IOServiceAddMatchingNotification(gNotifyPort, kIOMatchedNotification, matching, UsbIo_SetAsynRead, 0, &gSendAsynReadIter);
			
			UsbIo_SetAsynRead(NULL, gSendAsynReadIter);
			mach_port_deallocate(mach_task_self(), masterPort);
			masterPort = 0;

			CFRunLoopRun();
			printf("Together forever - Rick Astley. If we end up here our code has had too much beer.\n");

		}
		else {
			fprintf(stderr, "Can't create a USB matching dictionary\n");
			mach_port_deallocate(mach_task_self(), masterPort);
		}
	}
	return 0;
}
