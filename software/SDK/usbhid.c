#include "usbhid.h"
#include "usbserial.h"
#include "basesystem.h"
#include <string.h>

static struct USBEndpointRecord dev0ep;
static struct USBDeviceRecord s_deviceTable[8];

volatile uint32_t *IO_USBATRX = (volatile uint32_t* )DEVICE_USBA; // Receive fifo
volatile uint32_t *IO_USBASTA = (volatile uint32_t* )(DEVICE_USBA+4); // Output FIFO state

static uint32_t statusF = 0;
static uint32_t sparebyte = 0;

#define ASSERT_MAX3421_CS *IO_USBATRX = 0x100;
#define RESET_MAX3421_CS *IO_USBATRX = 0x101;

static struct SUSBContext *s_usbhost = NULL;

void USBHostSetContext(struct SUSBContext *ctx)
{
	s_usbhost = ctx;
}

struct SUSBContext *USBHostGetContext()
{
	return s_usbhost;
}

void MAX3421FlushOutputFIFO()
{
	while ((*IO_USBASTA)&0x1) {}
}

uint8_t MAX3421ReadByte(uint8_t command)
{
	ASSERT_MAX3421_CS
	*IO_USBATRX = command;   // -> status in read FIFO
	statusF = *IO_USBATRX;   // unused
	*IO_USBATRX = 0;		 // -> read value in read FIFO
	sparebyte = *IO_USBATRX; // output value
	RESET_MAX3421_CS

	return sparebyte;
}

void MAX3421WriteByte(uint8_t command, uint8_t data)
{
	ASSERT_MAX3421_CS
	*IO_USBATRX = command | 0x02; // -> zero in read FIFO
	statusF = *IO_USBATRX;		// unused
	*IO_USBATRX = data;		   // -> zero in read FIFO
	sparebyte = *IO_USBATRX;	  // unused
	RESET_MAX3421_CS
}

int MAX3421ReadBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3421_CS
	*IO_USBATRX = command;   // -> status in read FIFO
	statusF = *IO_USBATRX;   // unused
	//*IO_USBCTRX = 0;		 // -> read value in read FIFO
	//sparebyte = *IO_USBCTRX; // output value

	for (int i=0; i<length; i++)
	{
		*IO_USBATRX = 0;		  // send one dummy byte per input desired
		buffer[i] = *IO_USBATRX;  // store data byte
	}
	RESET_MAX3421_CS

	return 0;
}

void MAX3421WriteBytes(uint8_t command, uint8_t length, uint8_t *buffer)
{
	ASSERT_MAX3421_CS
	*IO_USBATRX = command | 0x02;   // -> status in read FIFO
	statusF = *IO_USBATRX;		  // unused
	//*IO_USBATRX = 0;		 // -> read value in read FIFO
	//sparebyte = *IO_USBATRX; // output value

	for (int i=0; i<length; i++)
	{
		*IO_USBATRX = buffer[i];  // send one dummy byte per input desired
		sparebyte = *IO_USBATRX;  // unused
	}
	RESET_MAX3421_CS
}

void MAX3421CtlReset()
{
	// Reset MAX3421E by setting res high then low
	MAX3421WriteByte(rUSBCTL, bmCHIPRES);
	MAX3421WriteByte(rUSBCTL, 0);

	// Wait for all output to be sent to the device
	MAX3421FlushOutputFIFO();
	E32Sleep(3*ONE_MILLISECOND_IN_TICKS);

	// Wait for oscillator OK interrupt for the 12MHz external clock
	uint8_t rd = 0;
	while ((rd & bmOSCOKIRQ) == 0)
	{
		rd = MAX3421ReadByte(rUSBIRQ);
		E32Sleep(3*ONE_MILLISECOND_IN_TICKS);
	}
	MAX3421WriteByte(rUSBIRQ, bmOSCOKIRQ); // Clear IRQ
}

enum EBusState USBBusProbe()
{
	uint8_t bus_sample;

	bus_sample = MAX3421ReadByte(rHRSL); // Get J,K status
	bus_sample &= (bmJSTATUS|bmKSTATUS); // zero the rest of the byte

	switch( bus_sample )
	{
		case bmJSTATUS:
			if((MAX3421ReadByte(rMODE) & bmLOWSPEED) == 0 ) {
				//USBSerialWrite("full speed host - jstatus\n");
				MAX3421WriteByte(rMODE, MODE_FS_HOST);       //start full-speed host
				return FSHOST;
			}
			else {
				//USBSerialWrite("low speed host - jstatus\n");
				MAX3421WriteByte(rMODE, MODE_LS_HOST);        //start low-speed host
				return LSHOST;
			}
			break;
		case bmKSTATUS:
			if(( MAX3421ReadByte(rMODE) & bmLOWSPEED) == 0 )
			{
				//USBSerialWrite("low speed host - kstatus\n");
				MAX3421WriteByte(rMODE, MODE_LS_HOST);       //start low-speed host
				return LSHOST;
			}
			else
			{
				//USBSerialWrite("full speed host - kstatus\n");
				MAX3421WriteByte(rMODE, MODE_FS_HOST);       //start full-speed host
				return FSHOST;
			}
			break;
		case bmSE1:              //illegal state
			return SE1;
			break;
		case bmSE0:              //disconnected state
			MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST | bmSEPIRQ);
			return SE0;
			break;
	}

	return BUSUNKNOWN;
}

void USBHostInit(uint32_t enableInterrupts)
{
	// Must set context first
	if (s_usbhost==NULL)
		return;

	for(uint8_t i = 0; i<8; i++ )
	{
		s_deviceTable[i].endpointInfo = NULL;
		s_deviceTable[i].deviceClass = 0;
	}
	s_deviceTable[0].endpointInfo = &dev0ep;
	dev0ep.sendToggle = bmSNDTOG0;
	dev0ep.receiveToggle = bmRCVTOG0;

	MAX3421WriteByte(rPINCTL, bmFDUPSPI | bmINTLEVEL | gpxSOF);
	MAX3421CtlReset();
	MAX3421WriteByte(rIOPINS1, 0x0);
	MAX3421WriteByte(rIOPINS2, 0x0);

	MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST);
	MAX3421WriteByte(rHIEN, bmCONDETIE | bmFRAMEIE);

	MAX3421WriteByte(rHCTL, bmSAMPLEBUS);
	while(!(MAX3421ReadByte(rHCTL) & bmSAMPLEBUS)) {}; //wait for sample operation to finish
	/*struct EBusState busState =*/ USBBusProbe();

	MAX3421WriteByte(rHIRQ, bmCONDETIRQ);
	if (enableInterrupts)
	{
		MAX3421WriteByte(rCPUCTL, bmIE);
	}
}

uint8_t USBDispatchPacket(uint8_t _token, uint8_t _ep, unsigned int _nak_limit)
{
	unsigned long timeout = E32ReadTime() + 200*ONE_MILLISECOND_IN_TICKS;
	uint8_t tmpdata;
	uint8_t rcode = 0xFF;
	unsigned int nak_count = 0;
	char retry_count = 0;

	while(timeout > E32ReadTime())
	{
		MAX3421WriteByte(rHXFR, (_token|_ep));
		rcode = 0xFF;
		while( E32ReadTime() < timeout )
		{
			tmpdata = MAX3421ReadByte(rHIRQ);
			if(tmpdata & bmHXFRDNIRQ )
			{
				MAX3421WriteByte(rHIRQ, bmHXFRDNIRQ);
				rcode = 0x00;
				break;
			}
		}

		if( rcode != 0x00 )
			return rcode;

		rcode = MAX3421ReadByte(rHRSL) & 0x0f;

		switch(rcode)
		{
			case hrNAK:
			{
				nak_count++;
				if( _nak_limit && ( nak_count == _nak_limit ))
					return rcode;
			}
			break;

			case hrTIMEOUT:
			{
				retry_count++;
				if(retry_count == 64 )
					return rcode;
			}
			break;

			default:
				return rcode;
		}
	}

	return rcode;
}

uint8_t USBControlStatus(uint8_t _ep, uint8_t _direction, unsigned int _nak_limit)
{
	uint8_t rcode;
	if(_direction)
		rcode = USBDispatchPacket(tokOUTHS, _ep, _nak_limit);
	else
		rcode = USBDispatchPacket(tokINHS, _ep, _nak_limit);

	return rcode;
}

uint8_t USBInTransfer(uint8_t _addr, uint8_t _ep, unsigned int _nbytes, char* _data, unsigned int _nak_limit)
{
	uint8_t rcode;
	uint8_t pktsize;
	uint8_t maxpktsize = s_deviceTable[_addr].endpointInfo[_ep].maxPacketSize;

	unsigned int xfrlen = 0;
	MAX3421WriteByte(rHCTL, s_deviceTable[_addr].endpointInfo[_ep].receiveToggle);
	while(1)
	{
		rcode = USBDispatchPacket(tokIN, _ep, _nak_limit);
		if(rcode)
			return rcode;

		if((MAX3421ReadByte(rHIRQ) & bmRCVDAVIRQ) == 0 )
			return 0xf0;

		pktsize = MAX3421ReadByte(rRCVBC);
		MAX3421ReadBytes(rRCVFIFO, pktsize, (uint8_t*)_data);
		_data += pktsize;
		MAX3421WriteByte(rHIRQ, bmRCVDAVIRQ);
		xfrlen += pktsize;

		if ((pktsize < maxpktsize ) || (xfrlen >= _nbytes))
		{
			if(MAX3421ReadByte(rHRSL) & bmRCVTOGRD )
				s_deviceTable[_addr].endpointInfo[_ep].receiveToggle = bmRCVTOG1;
			else
				s_deviceTable[_addr].endpointInfo[_ep].receiveToggle = bmRCVTOG0;
			return 0;
		}
	}
}

uint8_t USBOutTransfer(uint8_t _addr, uint8_t _ep, unsigned int _nbytes, char* _data, unsigned int nak_limit)
{
	uint8_t rcode = 0xFF, retry_count;
	char* data_p = _data;
	unsigned int bytes_tosend, nak_count;
	unsigned int bytes_left = _nbytes;
	uint8_t maxpktsize = s_deviceTable[_addr].endpointInfo[_ep].maxPacketSize;
	unsigned long timeout = E32ReadTime() + 200*ONE_MILLISECOND_IN_TICKS;

	if (!maxpktsize)
		return 0xFE;

	MAX3421WriteByte(rHCTL, s_deviceTable[_addr].endpointInfo[_ep].sendToggle);
	while(bytes_left)
	{
		retry_count = 0;
		nak_count = 0;
		bytes_tosend = (bytes_left >= maxpktsize) ? maxpktsize : bytes_left;

		MAX3421WriteBytes(rSNDFIFO, bytes_tosend, (uint8_t*)data_p);
		MAX3421WriteByte(rSNDBC, bytes_tosend);
		MAX3421WriteByte(rHXFR, (tokOUT|_ep));
		while(!(MAX3421ReadByte(rHIRQ) & bmHXFRDNIRQ));
		MAX3421WriteByte(rHIRQ, bmHXFRDNIRQ);

		rcode = MAX3421ReadByte(rHRSL) & 0x0f;
		while( rcode && ( timeout > E32ReadTime()))
		{
			switch( rcode )
			{
				case hrNAK:
				{
					nak_count++;
					if(nak_limit && (nak_count == 64))
						return rcode;
				}
				break;

				case hrTIMEOUT:
				{
					retry_count++;
					if(retry_count == 64)
						return rcode;
				}
				break;

				default:  
					return rcode;
			}

			MAX3421WriteByte(rSNDBC, 0);
			MAX3421WriteByte(rSNDFIFO, *data_p);
			MAX3421WriteByte(rSNDBC, bytes_tosend);
			MAX3421WriteByte(rHXFR, (tokOUT | _ep));
			while(!(MAX3421ReadByte(rHIRQ) & bmHXFRDNIRQ));
			MAX3421WriteByte(rHIRQ, bmHXFRDNIRQ);
			rcode = MAX3421ReadByte(rHRSL) & 0x0f;
		}

		bytes_left -= bytes_tosend;
		data_p += bytes_tosend;
	}
	s_deviceTable[_addr].endpointInfo[_ep].sendToggle = (MAX3421ReadByte(rHRSL) & bmSNDTOGRD) ? bmSNDTOG1 : bmSNDTOG0;
	return rcode;
}

uint8_t USBControlData(uint8_t _addr, uint8_t _ep, unsigned int _nbytes, char* _dataptr, uint8_t _direction, unsigned int _nak_limit)
{
	uint8_t rcode;
	if( _direction )
	{
		s_deviceTable[_addr].endpointInfo[_ep].receiveToggle = bmRCVTOG1;
		rcode = USBInTransfer(_addr, _ep, _nbytes, _dataptr, _nak_limit);
		return rcode;
	}
	else
	{
		s_deviceTable[_addr].endpointInfo[_ep].sendToggle = bmSNDTOG1;
		rcode = USBOutTransfer(_addr, _ep, _nbytes, _dataptr, _nak_limit );
		return rcode;
	}
}

uint8_t USBControlRequest(uint8_t _addr, uint8_t _ep, uint8_t _bmReqType, uint8_t _bRequest, uint8_t _wValLo, uint8_t _wValHi, unsigned int _wInd, unsigned int _nbytes, char* _dataptr, unsigned int _nak_limit)
{
	uint8_t direction = 0;
	uint8_t rcode;
	uint8_t setup_pkt[8];

	MAX3421WriteByte(rPERADDR, _addr);
	if( _bmReqType & 0x80 )
		direction = 1;

	setup_pkt[bmRequestType] = _bmReqType;
	setup_pkt[bRequest] = _bRequest;
	setup_pkt[wValueL] = _wValLo;
	setup_pkt[wValueH] = _wValHi;
	setup_pkt[wIndexL] = _wInd&0xFF;
	setup_pkt[wIndexH] = (_wInd>>8)&0xFF;
	setup_pkt[wLengthL] = _nbytes&0xFF;
	setup_pkt[wLengthH] = (_nbytes>>8)&0xFF;

	MAX3421WriteBytes(rSUDFIFO, 8, setup_pkt);
	rcode = USBDispatchPacket(tokSETUP, _ep, _nak_limit);

	if(rcode)
	{
		USBSerialWrite("Setup packet error 0x");
		USBSerialWriteHex(rcode);
		USBSerialWrite("\n");
		return(rcode);
	}

	if(_dataptr != NULL )
	{
		// data stage, if present
		rcode = USBControlData(_addr, _ep, _nbytes, _dataptr, direction, _nak_limit);
	}

	if(rcode)
	{
		//return error
		USBSerialWrite("Data packet error 0x");
		USBSerialWriteHex(rcode);
		USBSerialWrite("\n");
		return(rcode);
	}

	rcode = USBControlStatus(_ep, direction, _nak_limit);

	return rcode;
}

uint8_t USBGetDeviceDescriptor()
{
	struct USBDeviceDescriptor ddesc;

	s_deviceTable[0].endpointInfo[0].maxPacketSize = 8;
    uint8_t rcode = USBControlRequest(0, 0, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0x00, USB_DESCRIPTOR_DEVICE, 0x0000, 8, (char*)&ddesc, 64);

	if (rcode != 0)
		return rcode;

	s_deviceTable[0].endpointInfo[0].maxPacketSize = ddesc.bMaxPacketSizeEP0;

	// Retry with actual descriptor size
	// 18 == USB_DEVICE_DESCRIPTOR_SIZE
	rcode = USBControlRequest(0, 0, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0x00, USB_DESCRIPTOR_DEVICE, 0x0000, 18, (char*)&ddesc, 64);

	if (rcode != 0)
		return rcode;

	USBSerialWrite("\ndesctype:");
	USBSerialWriteHex(ddesc.bDescriptorType);
	USBSerialWrite("\nusbver:");
	USBSerialWriteHex(ddesc.bcdUSB);
	USBSerialWrite("\ndevclass:");
	USBSerialWriteHex(ddesc.bDeviceClass);
	USBSerialWrite("\nsubclass:");
	USBSerialWriteHex(ddesc.bDeviceSubClass);
	USBSerialWrite("\nprotocol:");
	USBSerialWriteHex(ddesc.bDeviceProtocol);
	USBSerialWrite("\nep0maxsize:");
	USBSerialWriteHex(ddesc.bMaxPacketSizeEP0);
	USBSerialWrite("\nvid:");
	USBSerialWriteHex(ddesc.idVendor);
	USBSerialWrite("\npid:");
	USBSerialWriteHex(ddesc.idProduct);
	USBSerialWrite("\ndev:");
	USBSerialWriteHex(ddesc.bcdDevice);
	USBSerialWrite("\nman$:");
	USBSerialWriteHex(ddesc.iManufacturer);
	USBSerialWrite("\nprod$:");
	USBSerialWriteHex(ddesc.iProduct);
	USBSerialWrite("\nser$:");
	USBSerialWriteHex(ddesc.iSerialNumber);
	USBSerialWrite("\nconfigs:");
	USBSerialWriteHex(ddesc.bNumConfigurations);
	USBSerialWrite("\n");

	struct USBConfigurationDescriptor cdef;
	for (uint8_t c=0; c<ddesc.bNumConfigurations; ++c)
	{
		// 9 == USB_CONFIGURATION_DESCRIPTOR_SIZE
		rcode = USBControlRequest(0, 0, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, c, USB_DESCRIPTOR_CONFIGURATION, 0x0000, 9, (char*)&cdef, 64);

		if (rcode != 0)
			return rcode;

		USBSerialWrite(" device config #");
		USBSerialWriteDecimal(c);

		USBSerialWrite("\n type:");
		USBSerialWriteHex(cdef.bDescriptorType);
		USBSerialWrite("\n tlen:");
		USBSerialWriteHex(cdef.wTotalLength);
		USBSerialWrite("\n interfaces:");
		USBSerialWriteHex(cdef.bNumInterfaces);
		USBSerialWrite("\n cval:");
		USBSerialWriteHex(cdef.bConfigurationValue);
		USBSerialWrite("\n conf$:");
		USBSerialWriteHex(cdef.iConfiguration);
		USBSerialWrite("\n attrib:");
		USBSerialWriteHex(cdef.bmAttributes);
		USBSerialWrite("\n maxpower:");
		USBSerialWriteHex(cdef.MaxPower);
		USBSerialWrite("\n");

		// re-request config descriptor with actual data size (cdef.wTotalLength)
		char rawdata[256];
		rcode = USBControlRequest(0, 0, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, c, USB_DESCRIPTOR_CONFIGURATION, 0x0000, cdef.wTotalLength, rawdata, 64);

		if (rcode != 0)
			return rcode;

		// Skip to the first interface descriptor and list all interfaces
		struct USBInterfaceDescriptor *idef = (struct USBInterfaceDescriptor *)&rawdata[9];
		uint8_t eptotal = 0;
		for (uint8_t i=0; i<cdef.bNumInterfaces; ++i)
		{
			USBSerialWrite("  interface #");
			USBSerialWriteDecimal(i);
			USBSerialWrite("\n  type:");
			USBSerialWriteHex(idef->bDescriptorType);
			USBSerialWrite("\n  inum:");
			USBSerialWriteHex(idef->bInterfaceNumber);
			USBSerialWrite("\n  altset:");
			USBSerialWriteHex(idef->bAlternateSetting);
			USBSerialWrite("\n  numep:");
			USBSerialWriteHex(idef->bNumEndpoints);
			USBSerialWrite("\n  class:");
			USBSerialWriteHex(idef->bInterfaceClass);
			USBSerialWrite("\n  subclass:");
			USBSerialWriteHex(idef->bInterfaceSubClass);
			USBSerialWrite("\n  protocol:");
			USBSerialWriteHex(idef->bInterfaceProtocol);
			USBSerialWrite("\n  iface$:");
			USBSerialWriteHex(idef->iInterface);
			USBSerialWrite("\n");
			eptotal += idef->bNumEndpoints;
			++idef;
		}

		struct USBEndpointDescriptor *edef = (struct USBEndpointDescriptor *)idef;
		for (uint8_t e=0; e<eptotal; ++e)
		{
			USBSerialWrite("   endpoint #");
			USBSerialWriteDecimal(e);
			USBSerialWrite("\n   type:");
			USBSerialWriteHex(edef->bDescriptorType);
			USBSerialWrite("\n   address:");
			USBSerialWriteHex(edef->bEndpointAddress);
			USBSerialWrite("\n   interval:");
			USBSerialWriteHex(edef->bInterval);
			USBSerialWrite("\n   length:");
			USBSerialWriteHex(edef->bLength);
			USBSerialWrite("\n   attrib:");
			USBSerialWriteHex(edef->bmAttributes);
			USBSerialWrite("\n   maxpacketsize:");
			USBSerialWriteHex(edef->wMaxPacketSize);
			USBSerialWrite("\n");
			++edef;
		}

		// Get language descriptor
		struct USBStringLanguageDescriptor lang;
		rcode = USBControlRequest(0, 0, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0, USB_DESCRIPTOR_STRING, 0x0000, 4, (char*)&lang, 64);

		if (rcode != 0)
			return rcode;

		USBSerialWrite("  Language descriptor (string #0):");
		USBSerialWrite("\n  length:");
		USBSerialWriteHex(lang.bLength);
		USBSerialWrite("\n  language:");
		USBSerialWriteHex(lang.wLanguage);
		USBSerialWrite("\n");

		// TODO: Rest of the strings will be:
		// loop for all: short request to get size; long request to get actual string
	}

	USBSerialWrite("\n");
	return 0;
}

uint8_t USBAttach(uint8_t *_paddr)
{
	for (int i=1; i<8; ++i)
	{
		if (s_deviceTable[i].endpointInfo == NULL)
		{
			// Placeholder endpoint
			s_deviceTable[i].endpointInfo = s_deviceTable[0].endpointInfo;

			// Set new address
    		uint8_t rcode = USBControlRequest(0, 0, bmREQ_SET, USB_REQUEST_SET_ADDRESS, i, 0x00, 0x0000, 0x0000, NULL, 64);
			if(rcode == 0)
			{
				USBSerialWrite("Device attached: \\dev\\usb\\");
				USBSerialWriteDecimal(i);
				USBSerialWrite("\n");
				*_paddr = i;
				return 0;
			}
			else
				return rcode;
		}
	}

	return 0xFF; // Can't assign address
}

uint8_t USBDetach(uint8_t _addr)
{
	USBSerialWrite("Device detached: \\dev\\usb\\");
	USBSerialWriteDecimal(_addr);
	USBSerialWrite("\n");

	s_deviceTable[_addr].endpointInfo = NULL;
	return 0;
}

uint8_t USBConfigHID()
{
	// TODO: HID device address defaults to 1
	uint8_t addr = 1;
	uint8_t ep = 0;
	uint8_t conf = 1; // Is this correct?

	USBSerialWrite("setting HID configuration\n");
	uint8_t rcode = USBControlRequest(addr, ep, bmREQ_SET, USB_REQUEST_SET_CONFIGURATION, conf, 0x00, 0x0000, 0x0000, NULL, 64);

	if (rcode == 0)
	{
		USBSerialWrite("switching to boot protocol\n");
		rcode = USBControlRequest(addr, ep, bmREQ_HIDOUT, HID_REQUEST_SET_PROTOCOL, HID_PROTOCOL_KEYBOARD, USB_HID_BOOT_PROTOCOL, 0x0000, 0x0000, NULL, 64);
	}

	return rcode;
}

uint8_t USBGetHIDDescriptor()
{
	// TODO: HID device address defaults to 1
	uint8_t addr = 1;
	uint8_t ep = 0;

	// NOTE: You can parse this data using
	// http://eleccelerator.com/usbdescreqparser/
	// Remember to click USB HID Report Descriptor to parse!

	// For a keyboard we expect to start with:
	// 0501 (generic keyboard)
	// 0906 (usage keyboard)

	USBSerialWrite("getting HID descriptor\n");
	char tmpdata[64];
    uint8_t rcode = USBControlRequest(addr, ep, bmREQ_HIDREPORT, USB_REQUEST_GET_DESCRIPTOR, 0x00, HID_DESCRIPTOR_REPORT, 0x0000, 64, tmpdata, 64);

	if (rcode != 0)
		return rcode;

	for(int i=0;i<64;++i)
		USBSerialWriteHexByte(tmpdata[i]);
	USBSerialWrite("\n");

	return 0;
}

void USBSetAddress(uint8_t _addr, uint8_t _ep)
{
	MAX3421WriteByte(rPERADDR, _addr);
	uint8_t mode = MAX3421ReadByte(rMODE);
	MAX3421WriteByte(rMODE, mode | bmHUBPRE);
}

uint8_t USBReadHIDData(uint8_t *_data)
{
	uint8_t addr = 1;
	uint8_t ep = 0;

	uint8_t reportID = 1;
    uint8_t rcode = USBControlRequest(addr, ep, bmREQ_HIDIN, HID_REQUEST_GET_REPORT, reportID, HID_DESCRIPTOR_HID, HID_PROTOCOL_KEYBOARD, 8, (char*)_data, 64);

	return rcode;
}