#include "usbserial.h"
#include "basesystem.h"
#include "max3420e.h"
#include "leds.h"
#include "encoding.h"
#include <string.h>

// Helper routines to set up the onboard MAX3420 in USB serial mode

static struct SUSBSerialContext *s_usbser = NULL;

void USBSerialSetContext(struct SUSBSerialContext *ctx)
{
	s_usbser = ctx;
}

struct SUSBSerialContext *USBSerialGetContext()
{
	return s_usbser;
}

#ifdef __cplusplus
char16_t vendorname[] = u"ENGIN"; // 10
char16_t devicename[] = u"tinysys usb serial"; // 36
char16_t deviceserial[] = u"EC000000"; // 16
char16_t devicedata[] = u"data"; // 8
#else
char vendorname[] = {'E',0,'N',0,'G',0,'I',0,'N',0};
char devicename[] = {'t',0,'i',0,'n',0,'y',0,'s',0,'y',0,'s',0,' ',0,'u',0,'s',0,'b',0,' ',0,'s',0,'e',0,'r',0,'i',0,'a',0,'l',0};
char deviceserial[] = {'E',0,'C',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0};
char devicedata[] = {'d',0,'a',0,'t',0,'a',0};
#endif

void MakeCDCDescriptors(struct SUSBSerialContext *ctx)
{
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/network/device-descriptor
	// https://www.usb.org/sites/default/files/CDC_EEM10.pdf
	// https://beyondlogic.org/usbnutshell/usb6.shtml

	// Device
	ctx->device.bLength = sizeof(struct USBDeviceDescriptor); // 18
	ctx->device.bDescriptorType = USBDesc_Device;
	ctx->device.bcdUSB = 0x0101;
	ctx->device.bDeviceClass = USBClass_CDCControl;
	ctx->device.bDeviceSubClass = 0x0;
	ctx->device.bDeviceProtocol = 0x0;
	ctx->device.bMaxPacketSizeEP0 = 64;
	ctx->device.idVendor = 0xFFFF;
	ctx->device.idProduct = 0x0001;
	ctx->device.bcdDevice = 0x0001;
	ctx->device.iManufacturer = 1;
	ctx->device.iProduct = 2;
	ctx->device.iSerialNumber = 3;
	ctx->device.bNumConfigurations =1;

	// Configuration
	ctx->config.bLength = sizeof(struct USBConfigurationDescriptor); // 9
	ctx->config.bDescriptorType = USBDesc_Configuration;
	ctx->config.wTotalLength = 0x0020; // 32 bytes; includes config, interface and endpoints (not the strings)
	ctx->config.bNumInterfaces = 1;
	ctx->config.bConfigurationValue = 1;
	ctx->config.iConfiguration = 0;
	ctx->config.bmAttributes = 0x80; // Bus powered
	ctx->config.MaxPower = 0xFA; // 500 mA (for the entire device)

	// Data Interface
	ctx->data.bLength = sizeof(struct USBInterfaceDescriptor); // 9
	ctx->data.bDescriptorType = USBDesc_Interface;
	ctx->data.bInterfaceNumber = 0;   // Interface #1
	ctx->data.bAlternateSetting = 0;
	ctx->data.bNumEndpoints = 2;	  // 2 endpoints (data in / data out)
	ctx->data.bInterfaceClass = USBClass_CDCData;
	ctx->data.bInterfaceSubClass = 0x00;
	ctx->data.bInterfaceProtocol = 0x00;
	ctx->data.iInterface = 4;

	// Data in
	ctx->input.bLength = sizeof(struct USBEndpointDescriptor); // 7
	ctx->input.bDescriptorType = USBDesc_Endpoint;
	ctx->input.bEndpointAddress = 0x82;		// EP2 in
	ctx->input.bmAttributes = 0x02;			// Bulk endpoint
	ctx->input.wMaxPacketSize = 64;
	ctx->input.bInterval = 0;

	// Data out
	ctx->output.bLength = sizeof(struct USBEndpointDescriptor); // 7
	ctx->output.bDescriptorType = USBDesc_Endpoint;
	ctx->output.bEndpointAddress = 0x01;	// EP1 out
	ctx->output.bmAttributes = 0x02;		// Bulk endpoint
	ctx->output.wMaxPacketSize = 64;
	ctx->output.bInterval = 0;

	// Strings
	ctx->strings[0].bLength = sizeof(struct USBStringLanguageDescriptor); // 4
	ctx->strings[0].bDescriptorType = USBDesc_String;
#ifdef __cplusplus
	ctx->strings[0].bString[0] = 0x0409; // English-United Sates
#else
	ctx->strings[0].bString[0] = 0x09; // English-United Sates
	ctx->strings[0].bString[1] = 0x04;
#endif
	ctx->strings[1].bLength = sizeof(struct USBCommonDescriptor) + 5*2; // 10
	ctx->strings[1].bDescriptorType = USBDesc_String;
	__builtin_memcpy(ctx->strings[1].bString, vendorname, 10);
	ctx->strings[2].bLength = sizeof(struct USBCommonDescriptor) + 18*2; // 36
	ctx->strings[2].bDescriptorType = USBDesc_String;
	__builtin_memcpy(ctx->strings[2].bString, devicename, 36);
	ctx->strings[3].bLength = sizeof(struct USBCommonDescriptor) + 8*2; // 16
	ctx->strings[3].bDescriptorType = USBDesc_String;
	__builtin_memcpy(ctx->strings[3].bString, deviceserial, 16);
	ctx->strings[4].bLength = sizeof(struct USBCommonDescriptor) + 4*2; // 8
	ctx->strings[4].bDescriptorType = USBDesc_String;
	__builtin_memcpy(ctx->strings[4].bString, devicedata, 8);
}

void MAX3420EnableIRQs()
{
	// Enable IRQs
	// NOTE: bmIN2BAVIE is disabled since having it on spams the INT pin beyond practical serviceability
	// Try to push data by polling from main thread instead
	MAX3420WriteByte(rEPIEN, bmSUDAVIE | /*bmIN2BAVIE |*/ bmOUT1DAVIE);
	MAX3420WriteByte(rUSBIEN, bmURESIE | bmURESDNIE);
}

int USBSerialInit(uint32_t enableInterrupts)
{
	// Must set context first
	if (s_usbser==NULL)
		return 0;

	// Generate descriptor table for a USB serial device
	MakeCDCDescriptors(s_usbser);

	// Enable full-duplex, int low, gpxA/B passthrough
	MAX3420WriteByte(rPINCTL, bmFDUPSPI | bmINTLEVEL | gpxSOF);

	// Reset the chip and wait for clock to stabilize
	MAX3420CtlReset();

	// Turn off GPIO
	MAX3420WriteByte(rGPIO, 0x0);

	// Connect
	MAX3420WriteByte(rUSBCTL, bmCONNECT | bmVBGATE);

	if (enableInterrupts)
	{
		MAX3420EnableIRQs();
		// Enable interrupt generation via INT pin
		MAX3420WriteByte(rCPUCTL, bmIE);
	}

	return 1;
}

int USBSerialWriteN(const char *outstring, uint32_t count)
{
	uint8_t *cptr = (uint8_t*)outstring;
	uint32_t blockCount = count/64;
	uint32_t leftoverCount = count%64;

	uint32_t currLED = LEDGetState();
	LEDSetState(currLED | 0x8);

	for(uint32_t i=0; i<blockCount; ++i)
	{
		// Wait for buffer available for EP2 fifo
		while((MAX3420ReadByte(rEPIRQ) & bmIN2BAVIRQ) == 0) { }
		MAX3420WriteByte(rCPUCTL, 0); // Disable MAX3420 interrupts so we don't fall into ISR for USB
		MAX3420WriteBytes(rEP2INFIFO, 64, (uint8_t*)cptr);
		MAX3420WriteByte(rEP2INBC, 64);
		MAX3420FlushOutputFIFO();
		MAX3420WriteByte(rCPUCTL, bmIE); // Enable MAX3420 interrupts
		cptr += 64;
	}

	if (leftoverCount)
	{
		// Wait for buffer available for EP2 fifo
		while((MAX3420ReadByte(rEPIRQ) & bmIN2BAVIRQ) == 0) { }
		MAX3420WriteByte(rCPUCTL, 0); // Disable MAX3420 interrupts so we don't fall into ISR for USB
		MAX3420WriteBytes(rEP2INFIFO, leftoverCount, (uint8_t*)cptr);
		MAX3420WriteByte(rEP2INBC, leftoverCount);
		MAX3420FlushOutputFIFO();
		MAX3420WriteByte(rCPUCTL, bmIE); // Enable MAX3420 interrupts
	}

	LEDSetState(currLED);

	return count;
}

int USBSerialWrite(const char *outstring)
{
	uint32_t count = 0;
	while(outstring[count]!=0) { count++; }
	return USBSerialWriteN(outstring, count);
}

int USBSerialWriteHexByte(const uint8_t i)
{
	const char hexdigits[] = "0123456789ABCDEF";
	char msg[] = "  ";

	msg[0] = hexdigits[((i>>4)%16)];
	msg[1] = hexdigits[(i%16)];

	return USBSerialWriteN(msg, 2);
}

int USBSerialWriteHex(const uint32_t i)
{
	const char hexdigits[] = "0123456789ABCDEF";
	char msg[] = "        ";

	msg[0] = hexdigits[((i>>28)%16)];
	msg[1] = hexdigits[((i>>24)%16)];
	msg[2] = hexdigits[((i>>20)%16)];
	msg[3] = hexdigits[((i>>16)%16)];
	msg[4] = hexdigits[((i>>12)%16)];
	msg[5] = hexdigits[((i>>8)%16)];
	msg[6] = hexdigits[((i>>4)%16)];
	msg[7] = hexdigits[(i%16)];

	return USBSerialWriteN(msg, 8);
}

int USBSerialWriteDecimal(const int32_t i)
{
	const char digits[] = "0123456789";
	char msg[] = "                                ";

	int d = 1000000000;
	uint32_t enableappend = 0;
	uint32_t m = 0;

	if (i<0)
		msg[m++] = '-';

	for (int c=0;c<10;++c)
	{
		uint32_t r = ((i/d)%10)&0x7FFFFFFF;
		// Ignore preceeding zeros
		if ((r!=0) || enableappend || d==1)
		{
			enableappend = 1; // Rest of the digits can be appended
			msg[m++] = digits[r];
		}
		d = d/10;
	}
	msg[m] = 0;

	return USBSerialWrite(msg);
}
