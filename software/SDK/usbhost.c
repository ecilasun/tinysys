#include "usbhost.h"
#include "usbserial.h"
#include "max3421e.h"
#include <string.h>

// Please see:
// https://www.analog.com/media/en/technical-documentation/user-guides/max3421erevisions-1-and-2-host-out-transfers.pdf

static struct USBEndpointRecord dev0controlEP;
static struct USBDeviceRecord s_deviceTable[8];

static uint32_t s_protosubclass = 0;

static struct SUSBHostContext *s_usbhost = NULL;
static uint8_t s_HIDDescriptorLen = 64;

// EBusState
static uint32_t* s_probe_result = (uint32_t*)USB_HOST_STATE;

void USBHostSetContext(struct SUSBHostContext *ctx)
{
	s_usbhost = ctx;
}

struct SUSBHostContext *USBHostGetContext()
{
	return s_usbhost;
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

enum EBusState USBHostInit(uint32_t enableInterrupts)
{
	// Must set context first
	if (s_usbhost==NULL)
		return BUSUNKNOWN;

	for(uint8_t i = 0; i<8; i++ )
	{
		s_deviceTable[i].endpointInfo = NULL;
		s_deviceTable[i].deviceClass = 0;
	}

	// Set up control endpoint for default device
	s_deviceTable[0].endpointInfo = &dev0controlEP;
	dev0controlEP.sendToggle = bmSNDTOG0;
	dev0controlEP.receiveToggle = bmRCVTOG0;

	MAX3421WriteByte(rPINCTL, bmFDUPSPI | bmINTLEVEL | gpxSOF);
	MAX3421CtlReset();
	MAX3421WriteByte(rIOPINS1, 0x0);
	MAX3421WriteByte(rIOPINS2, 0x0);

	MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST);
	MAX3421WriteByte(rHIEN, bmCONDETIE | bmFRAMEIE);

	MAX3421WriteByte(rHCTL, bmSAMPLEBUS);

	while(!(MAX3421ReadByte(rHCTL) & bmSAMPLEBUS)) {}; //wait for sample operation to finish

	*s_probe_result = (uint32_t)USBBusProbe();

	MAX3421WriteByte(rHIRQ, bmCONDETIRQ);
	if (enableInterrupts)
	{
		MAX3421WriteByte(rCPUCTL, bmIE);
	}

	// Reset keymap
	uint16_t* keystate = (uint16_t*)KEYBOARD_KEYSTATE_BASE;
	__builtin_memset(keystate, 0x00, 256*sizeof(uint16_t));

	return (enum EBusState)(*s_probe_result);
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
		MAX3421WriteByte(rHXFR, (_token | _ep));
		rcode = 0xFF;
		while( timeout > E32ReadTime() )
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

		while( timeout > E32ReadTime() )
		{
			rcode = MAX3421ReadByte(rHRSL) & 0x0f;
			if (rcode != hrBUSY)
				break;
			// else
			// BUSY!
		}

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
			return 0xf0; // No data available

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
	return 0;
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
		MAX3421WriteByte(rHXFR, (tokOUT | _ep));
		while(!(MAX3421ReadByte(rHIRQ) & bmHXFRDNIRQ));
		MAX3421WriteByte(rHIRQ, bmHXFRDNIRQ);

		rcode = MAX3421ReadByte(rHRSL) & 0x0f;

		// Not ACK, check condition and repeat transfer
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

			// Write only first byte to FIFO to regain control and re-send
			MAX3421WriteByte(rSNDBC, 0);
			MAX3421WriteByte(rSNDFIFO, *data_p);
			MAX3421WriteByte(rSNDBC, bytes_tosend);

			// Relaunch transfer
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

void USBErrorString(uint8_t rcode)
{
	if (rcode == hrSUCCESS)
		USBSerialWrite("SUCCESS\n");
	else if (rcode == hrBUSY)
		USBSerialWrite("BUSY\n");
	else if (rcode == hrBADREQ)
		USBSerialWrite("BADREQ\n");
	else if (rcode == hrUNDEF)
		USBSerialWrite("UNDEF\n");
	else if (rcode == hrNAK)
		USBSerialWrite("NAK\n");
	else if (rcode == hrSTALL)
		USBSerialWrite("STALL\n");
	else if (rcode == hrTOGERR)
		USBSerialWrite("TOGERR\n");
	else if (rcode == hrWRONGPID)
		USBSerialWrite("WRONGPID\n");
	else if (rcode == hrBADBC)
		USBSerialWrite("BADBC\n");
	else if (rcode == hrPIDERR)
		USBSerialWrite("PIDERR\n");
	else if (rcode == hrPKTERR)
		USBSerialWrite("PKTERR\n");
	else if (rcode == hrCRCERR)
		USBSerialWrite("CRCERR\n");
	else if (rcode == hrKERR)
		USBSerialWrite("KERR\n");
	else if (rcode == hrJERR)
		USBSerialWrite("JERR\n");
	else if (rcode == hrTIMEOUT)
		USBSerialWrite("TIMEOUT\n");
	else if (rcode == hrBABBLE)
		USBSerialWrite("BABBLE\n");
	else
	{
		USBSerialWrite("0x");
		USBSerialWriteHex(rcode);
		USBSerialWrite("\n");
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
		USBSerialWrite("Setup packet error: ");
		USBErrorString(rcode);
		return(rcode);
	}

	if(_dataptr != NULL )
	{
		// data stage, if present
		rcode = USBControlData(_addr, _ep, _nbytes, _dataptr, direction, _nak_limit);
	}

	if(rcode)
	{
		USBSerialWrite("Data packet error: ");
		USBErrorString(rcode);
		return(rcode);
	}

	rcode = USBControlStatus(_ep, direction, _nak_limit);

	if(rcode)
	{
		USBSerialWrite("Control status error: ");
		USBErrorString(rcode);
	}

	return rcode;
}

uint8_t USBParseDescriptor(uint8_t *_desc, uint8_t* _dtype, uint8_t* _offset)
{
	uint8_t stringCount = 0;
	struct USBCommonDescriptor* dcom = (struct USBCommonDescriptor*)_desc;
	switch (dcom->bDescriptorType)
	{
		case USBDESCTYPE_DEVICE:
		{
			*_dtype = USBDESCTYPE_DEVICE;
			*_offset = sizeof(struct USBDeviceDescriptor);
			struct USBDeviceDescriptor* desc = (struct USBDeviceDescriptor*)_desc;
			USBSerialWrite("\n   dtype: device");
			USBSerialWrite("\n  usbver: ");
			USBSerialWriteDecimal(desc->bcdUSB>>8);
			USBSerialWrite(".");
			USBSerialWriteDecimal(desc->bcdUSB & 0xFF);
			USBSerialWrite("\ndevclass: 0x");
			USBSerialWriteHexByte(desc->bDeviceClass);
			USBSerialWrite("\nsubclass: 0x");
			USBSerialWriteHexByte(desc->bDeviceSubClass);
			USBSerialWrite("\nprotocol: 0x");
			USBSerialWriteHexByte(desc->bDeviceProtocol);
			USBSerialWrite("\nep0maxsz: 0x");
			USBSerialWriteHexByte(desc->bMaxPacketSizeEP0);
			USBSerialWrite("\n     VID: 0x");
			USBSerialWriteHex(desc->idVendor);
			USBSerialWrite("\n     PID: 0x");
			USBSerialWriteHex(desc->idProduct);
			USBSerialWrite("\n     dev: 0x");
			USBSerialWriteHex(desc->bcdDevice);
			USBSerialWrite("\n    man$: 0x");
			USBSerialWriteHexByte(desc->iManufacturer);
			stringCount += desc->iManufacturer!=0 ? 1:0;
			USBSerialWrite("\n   prod$: 0x");
			USBSerialWriteHexByte(desc->iProduct);
			stringCount += desc->iProduct!=0 ? 1:0;
			USBSerialWrite("\n   ser#$: 0x");
			USBSerialWriteHexByte(desc->iSerialNumber);
			stringCount += desc->iSerialNumber!=0 ? 1:0;
			USBSerialWrite("\n configs: 0x");
			USBSerialWriteHexByte(desc->bNumConfigurations);
			USBSerialWrite("\n");
		}
		break;

		case USBDESCTYPE_CONFIGURATION:
		{
			*_dtype = USBDESCTYPE_CONFIGURATION;
			*_offset = sizeof(struct USBConfigurationDescriptor);
			struct USBConfigurationDescriptor* desc = (struct USBConfigurationDescriptor*)_desc;
			USBSerialWrite("\n     dtype: configuration");
			USBSerialWrite("\n      tlen: 0x");
			USBSerialWriteHex(desc->wTotalLength);
			USBSerialWrite("\ninterfaces: 0x");
			USBSerialWriteHexByte(desc->bNumInterfaces);
			USBSerialWrite("\n      cval: 0x");
			USBSerialWriteHexByte(desc->bConfigurationValue);
			USBSerialWrite("\n     conf$: 0x");
			USBSerialWriteHexByte(desc->iConfiguration);
			stringCount += desc->iConfiguration!=0 ? 1:0;
			USBSerialWrite("\n    attrib: 0x");
			USBSerialWriteHexByte(desc->bmAttributes);
			USBSerialWrite("\n  maxpower: 0x");
			USBSerialWriteHexByte(desc->MaxPower);
			USBSerialWrite("\n");
		}
		break;

		case USBDESCTYPE_STRING:
		{
			*_dtype = USBDESCTYPE_STRING;
			*_offset = sizeof(struct USBStringLanguageDescriptor);
		}
		break;

		case USBDESCTYPE_INTERFACE:
		{
			*_dtype = USBDESCTYPE_INTERFACE;
			*_offset = sizeof(struct USBInterfaceDescriptor);
			struct USBInterfaceDescriptor *desc = (struct USBInterfaceDescriptor *)_desc;
			USBSerialWrite("\n       dtype: interface");
			USBSerialWrite("\n        inum: 0x");
			USBSerialWriteHexByte(desc->bInterfaceNumber);
			USBSerialWrite("\n      altset: 0x");
			USBSerialWriteHexByte(desc->bAlternateSetting);
			USBSerialWrite("\n       numep: 0x");
			USBSerialWriteHexByte(desc->bNumEndpoints);
			USBSerialWrite("\n       class: ");
			switch(desc->bInterfaceClass)
			{
				case USBClass_MassStorage:
					USBSerialWrite("mass storage");
				break;
				case USBClass_HID:
					USBSerialWrite("HID");
				break;
				default:
					USBSerialWriteHexByte(desc->bInterfaceClass);
				break;
			}
			if (desc->bInterfaceClass == USBClass_HID)
			{
				USBSerialWrite("\n    subclass: ");
				switch (desc->bInterfaceSubClass)
				{
					case 0:
						USBSerialWrite("none");
						break;
					case 1:
						USBSerialWrite("boot");
						//s_protosubclass = 1; // Boot protocol used
						break;
					default:
						USBSerialWrite("reserved");
						break;
				}
			}
			else
			{
				USBSerialWrite("\n    subclass: 0x");
				USBSerialWriteHexByte(desc->bInterfaceSubClass);
			}
			if (desc->bInterfaceClass == USBClass_HID)
			{
				USBSerialWrite("\n    protocol: ");
				switch (desc->bInterfaceProtocol)
				{
					case 0:
						USBSerialWrite("none");
						break;
					case 1:
						USBSerialWrite("keyboard");
						break;
					case 2:
						USBSerialWrite("mouse");
						break;
					default:
						USBSerialWrite("reserved");
						break;
				}
			}
			else
			{
				USBSerialWrite("\n    protocol: 0x");
				USBSerialWriteHexByte(desc->bInterfaceProtocol);
			}
			USBSerialWrite("\n      iface$: 0x");
			USBSerialWriteHexByte(desc->iInterface);
			stringCount += desc->iInterface!=0 ? 1:0;
			USBSerialWrite("\n");
		}
		break;

		case USBDESCTYPE_ENDPOINT:
		{
			*_dtype = USBDESCTYPE_ENDPOINT;
			*_offset = sizeof(struct USBEndpointDescriptor);
			struct USBEndpointDescriptor *desc = (struct USBEndpointDescriptor*)_desc;
			USBSerialWrite("\n         dtype: endpoint");
			USBSerialWrite("\n       address: 0x");
			USBSerialWriteHexByte(desc->bEndpointAddress);
			USBSerialWrite("\n      interval: 0x");
			USBSerialWriteHexByte(desc->bInterval);
			USBSerialWrite("\n        length: 0x");
			USBSerialWriteHexByte(desc->bLength);
			USBSerialWrite("\n        attrib: 0x");
			USBSerialWriteHexByte(desc->bmAttributes);
			USBSerialWrite("\n maxpacketsize: 0x");
			USBSerialWriteHex(desc->wMaxPacketSize);
			USBSerialWrite("\n");
		}
		break;

		case USBDESCTYPE_HIDSPECIFIC:
		{
			*_dtype = USBDESCTYPE_HIDSPECIFIC;
			*_offset = sizeof(struct USBHIDDescriptor);
			struct USBHIDDescriptor *desc = (struct USBHIDDescriptor*)_desc;
			USBSerialWrite("\n         dtype: HID");
			USBSerialWrite("\n           HID: 0x");
			USBSerialWriteHexByte(desc->bcdHID);
			USBSerialWrite("\n  country code: 0x");
			USBSerialWriteHexByte(desc->bCountryCode);
			USBSerialWrite("\n     descBytes: 0x");
			USBSerialWriteHexByte(desc->bNumDescriptors);
			s_HIDDescriptorLen = desc->bNumDescriptors;
			USBSerialWrite("\n          type: 0x");
			USBSerialWriteHexByte(desc->bDescrType);
			USBSerialWrite("\n");
			*_offset = dcom->bLength;
		}
		break;

		case USBDESCTYPE_UNKNOWN:
		case USBDESCTYPE_DEVICEQUALIFIER:
		case USBDESCTYPE_OTHERSPEEDCFG:
		case USBDESCTYPE_INTERFACEPOWER:
		case USBDESCTYPE_OTG:
		case USBDESCTYPE_DEBUG:
		case USBDESCTYPE_INTERFACEASSOC:
		case USBDESCTYPE_SECURITY:
		case USBDESCTYPE_KEY:
		case USBDESCTYPE_ENCRYPTTYPE:
		case USBDESCTYPE_BINARYDEVOBJSTORE:
		case USBDESCTYPE_DEVICECAPABILITY:
		case USBDESCTYPE_WIRELESSEPCOMPANION:
		case USBDESCTYPE_SUPERSPEEDEPCOMPANION:
		case USBDESCTYPE_SUPERSPEEDISOEPCOMPANION:
		{
			*_dtype = USBDESCTYPE_UNKNOWN;
			*_offset = dcom->bLength;
			USBSerialWrite("\n   dtype: 0x");
			USBSerialWriteHexByte(dcom->bDescriptorType);
			USBSerialWrite("\n");
		}
		break;

		default:
		{
			*_dtype = USBDESCTYPE_UNKNOWN;
			*_offset = 0xFF;
			USBSerialWrite("\nError: invalid descriptor 0x\n");
			USBSerialWriteHexByte(dcom->bDescriptorType);
			USBSerialWrite("\n");
		}
		break;
	}

	return stringCount;
}

uint8_t USBGetDeviceDescriptor(uint8_t _addr, uint8_t _ep)
{
	s_protosubclass = 0;
	struct USBDeviceDescriptor ddesc;

	s_deviceTable[_addr].endpointInfo[_ep].maxPacketSize = 8;
    uint8_t rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0x00, USB_DESCRIPTOR_DEVICE, 0x0000, 8, (char*)&ddesc, 64);

	if (rcode != 0)
		return rcode;

	s_deviceTable[_addr].endpointInfo[_ep].maxPacketSize = ddesc.bMaxPacketSizeEP0;

	// Retry with actual descriptor size
	// 18 == USB_DEVICE_DESCRIPTOR_SIZE
	rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0x00, USB_DESCRIPTOR_DEVICE, 0x0000, 18, (char*)&ddesc, 64);

	if (rcode != 0)
		return rcode;
	
	int stringCount = 0;

	uint8_t dtype = USBDESCTYPE_UNKNOWN;
	uint8_t offset = 0;
	stringCount += USBParseDescriptor((uint8_t*)&ddesc, &dtype, &offset);

	struct USBConfigurationDescriptor cdef;
	for (uint8_t c=0; c<ddesc.bNumConfigurations; ++c)
	{
		// 9 == USB_CONFIGURATION_DESCRIPTOR_SIZE
		rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, c, USB_DESCRIPTOR_CONFIGURATION, 0x0000, 9, (char*)&cdef, 64);

		if (rcode != 0)
			return rcode;

		// re-request config descriptor with actual data size (cdef.wTotalLength)
		char rawdata[256];
		rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, c, USB_DESCRIPTOR_CONFIGURATION, 0x0000, cdef.wTotalLength, rawdata, 64);

		if (rcode != 0)
			return rcode;

		dtype = USBDESCTYPE_UNKNOWN;
		offset = 0;
		stringCount += USBParseDescriptor((uint8_t*)&cdef, &dtype, &offset);

		while (offset < cdef.wTotalLength)
		{
			uint8_t descsize = 0;
			stringCount += USBParseDescriptor((uint8_t*)&rawdata[offset], &dtype, &descsize);
			if (descsize == 0xFF)
				break;
			offset += descsize;
		}

		// Get language descriptor and strings
		struct USBStringLanguageDescriptor lang;
		rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0, USB_DESCRIPTOR_STRING, 0x0000, 4, (char*)&lang, 64);

		if (rcode != 0)
			return rcode;

		USBSerialWrite("Language descriptor (string #0):");
		USBSerialWrite("\n      length: 0x");
		USBSerialWriteHexByte(lang.bLength);
		USBSerialWrite("\n    language: 0x");
		USBSerialWriteHex(lang.wLanguage);
		USBSerialWrite("\n");

		for (int s=0; s<stringCount; ++s)
		{
			struct USBStringDescriptor str;
			// Only length, though I thought we'd have to read the full USBStringDescriptor
			rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, s+1, USB_DESCRIPTOR_STRING, 0x0000, 1, (char*)&str, 64);
			if (rcode == 0 && str.bLength != 0)
			{
				rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, s+1, USB_DESCRIPTOR_STRING, 0x0000, str.bLength, (char*)&str, 64);
				if (rcode == 0)
				{
					USBSerialWrite("str#");
					USBSerialWriteDecimal(s+1);
					USBSerialWrite(": \"");
					USBSerialWriteN((char*)str.bString, str.bLength-2);
					USBSerialWrite("\"\n");
				}
			}

			if (rcode)
			{
				// Clear stall condition if this is not supported
				uint16_t epAddress = 0x81;	// TODO: get it from device->endpoints[_ep]->epAddress
				rcode = USBControlRequest(_addr, _ep, bmREQ_CLEAR_FEATURE, USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, 0, epAddress, 0, NULL, 64);
				if (rcode == 0)
				{
					USBSerialWrite("stall cleared, won't query strings\n");
					// Stop requesting strings
					break;
				}
			}
		}
	}

	USBSerialWrite("\n");
	return 0;
}

uint8_t USBAttach(uint8_t *_paddr, uint8_t *_pep)
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
				*_pep = 0; // TODO: User the real endpoint here
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
	// Can't detach device at default address (0)
	if (_addr != 0)
	{
		USBSerialWrite("Device detached: \\dev\\usb\\");
		USBSerialWriteDecimal(_addr);
		USBSerialWrite("\n");
		s_deviceTable[_addr].endpointInfo = NULL;
	}

	return 0;
}

uint8_t USBConfigHID(uint8_t _addr, uint8_t _ep)
{
	uint8_t config = 1;
	uint8_t interface = 0;

	USBSerialWrite("setting configuration\n");
	uint8_t rcode = USBControlRequest(_addr, _ep, bmREQ_SET, USB_REQUEST_SET_CONFIGURATION, config, 0x00, 0x0000, 0x0000, NULL, 64);

	if (rcode == 0)
	{
		USBSerialWrite("setting interface\n");
		rcode = USBControlRequest(_addr, _ep, bmREQ_SET, USB_REQUEST_SET_INTERFACE, interface, 0x00, 0x0000, 0x0000, NULL, 64);

		// Check if interface changes are supported (maybe we only have one default)
		if (rcode == hrSTALL)
		{
			uint16_t epAddress = 0x81;	// TODO: get it from device->endpoints[_ep]->epAddress
			rcode = USBControlRequest(_addr, _ep, bmREQ_CLEAR_FEATURE, USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, 0, epAddress, 0, NULL, 64);
			if (rcode == 0)
				USBSerialWrite("stall cleared\n");
		}
	}

	if (s_protosubclass == 1) // Using boot protocol
	{
		USBSerialWrite("switching to boot protocol\n");
		// iface is interface number
		uint8_t iface = 0;
		// Could also use HID_REPORT_PROTOCOL for report protocol
		rcode = USBControlRequest(_addr, _ep, bmREQ_HIDOUT, HID_REQUEST_SET_PROTOCOL, USB_HID_BOOT_PROTOCOL, 0x00, iface, 0x0000, NULL, 64);
	}

	return rcode;
}

uint8_t USBGetHIDDescriptor(uint8_t _addr, uint8_t _ep, uint8_t *_protocol)
{
	// NOTE: You can parse this data using
	// http://eleccelerator.com/usbdescreqparser/
	// Remember to click USB HID Report Descriptor to parse!

	// For a keyboard we expect to start with:
	// 0501 (generic keyboard)
	// 0906 (usage keyboard)

	USBSerialWrite("getting HID descriptor\n");
	char tmpdata[96];
    uint8_t rcode = USBControlRequest(_addr, _ep, bmREQ_HIDREPORT, USB_REQUEST_GET_DESCRIPTOR, 0x00, HID_DESCRIPTOR_REPORT, 0x0000, s_HIDDescriptorLen, tmpdata, 64);

	if (rcode != 0)
		return rcode;

	for(int i=0;i<s_HIDDescriptorLen;++i)
		USBSerialWriteHexByte(tmpdata[i]);
	USBSerialWrite("\n");

	if (tmpdata[3] == 0x02) 
	{
		*_protocol = HID_PROTOCOL_MOUSE;
		USBSerialWrite("Mouse\n");
	}
	else if (tmpdata[3] == 0x06)
	{
		*_protocol = HID_PROTOCOL_KEYBOARD;
		USBSerialWrite("Keyboard\n");
	}
	else
	{
		*_protocol = HID_PROTOCOL_NONE;
		USBSerialWrite("Non-HID\n");
	}

	return 0;
}

void USBSetAddress(uint8_t _addr, uint8_t _ep)
{
	MAX3421WriteByte(rPERADDR, _addr);
	// To cross to a low speed peripheral over a USB HUB
	// Only applicable to low speed peripheral
	//uint8_t mode = MAX3421ReadByte(rMODE);
	//MAX3421WriteByte(rMODE, mode | bmHUBPRE);
}

uint8_t USBReadHIDData(uint8_t _addr, uint8_t _ep, uint8_t _dataLen, uint8_t *_data, uint8_t _reportIndex, uint8_t _reportType)
{
	uint8_t rcode;

	if (s_protosubclass == 1) // Boot protocol
	{
		// Using interrupt endpoint
		MAX3421WriteByte(rPERADDR, _addr);
		rcode = USBControlData(_addr, _ep, _dataLen, (char*)_data, 1, 64);
	}
	else
	{
		// Using control endpoint
		uint8_t iface = 0;
    	rcode = USBControlRequest(_addr, _ep, bmREQ_HIDIN, HID_REQUEST_GET_REPORT, _reportIndex, _reportType, iface, _dataLen, (char*)_data, 64);
	}

	return rcode;
}

uint8_t USBWriteHIDData(uint8_t _addr, uint8_t _ep, uint8_t *_data)
{
	uint8_t reportID = 0;
	uint8_t reportType = 2; // LED control
	uint8_t iface = 0;
    uint8_t rcode = USBControlRequest(_addr, _ep, bmREQ_HIDOUT, HID_REQUEST_SET_REPORT, reportID, reportType, iface, 1, (char*)_data, 64);

	return rcode;
}

char scantoasciitable_lowercase[] = {
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	' ',  ' ',  ' ',  ' ',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l', // 0
	'm',  'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '1',  '2', // 1
	'3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',   10,   27,    8,    9,  ' ',  '-',  '=',  '[', // 2
	']', '\\',  '#',  ';', '\'',  '^',  ',',  '.',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 3
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  '.',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 4
	' ',  ' ',  ' ',  ' ',  '/',  '*',  '-',  '+',   13,  '1',  '2',  '3',  '4',  '5',  '6',  '7', // 5
	'8',  '9',  '0',  '.', '\\',  ' ',  ' ',  '=',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 6
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 7
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 8
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 9
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // A
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  '(',  ')',  '{',  '}',    8,    9,  ' ',  ' ',  ' ',  ' ', // B
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // C
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // D
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // E
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' '  // F
};

char scantoasciitable_uppercase[] = {
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	' ',  ' ',  ' ',  ' ',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L', // 0
	'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '!',  '@', // 1
	'#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',   10,   27,    8,    9,  ' ',  '_',  '+',  '{', // 2
	'}',  '|',  '~',  ':',  '"',  '~',  '<',  '>',  '?',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 3
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  '.',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 4
	' ',  ' ',  ' ',  ' ',  '/',  '*',  '-',  '+',   13,  '1',  '2',  '3',  '4',  '5',  '6',  '7', // 5
	'8',  '9',  '0',  '.', '\\',  ' ',  ' ',  '=',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 6
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 7
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 8
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // 9
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // A
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  '(',  ')',  '{',  '}',    8,    9,  ' ',  ' ',  ' ',  ' ', // B
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // C
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // D
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ', // E
	' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' '  // F
};

char HIDScanToASCII(const uint8_t _code, const uint8_t _uppercase)
{
    return _uppercase ? scantoasciitable_uppercase[_code] : scantoasciitable_lowercase[_code];
}
