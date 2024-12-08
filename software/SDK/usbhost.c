/** @file
 * 
 *  @brief USB Host driver
 *
 *  This file contains the USB Host driver for the MAX3421E USB Host controller.
 *  It is used to communicate with USB devices.
 *
 *  The driver is based on the MAX3421E datasheet and the USB 2.0 specification
 *  The MAX3421E specification can be found at https://www.analog.com/media/en/technical-documentation/user-guides/max3421erevisions-1-and-2-host-out-transfers.pdf
*/

#include "usbhost.h"
#include "max3421e.h"
#include <string.h>

//#define DEBUG_USB_HOST
#if defined(DEBUG_USB_HOST)
#include "uart.h"
#endif

#define MAXDEVICES 2
static struct USBDeviceRecord s_deviceTable[MAXDEVICES];

static uint32_t s_protosubclass = 0;
static uint32_t s_cval = 0;

static struct SUSBHostContext *s_usbhost = NULL;
static uint8_t s_HIDDescriptorLen = 64;

// EBusState
static uint32_t *s_probe_result = (uint32_t*)USB_HOST_STATE;

/**
 * @brief Set the USB Host context
 * 
 * Set the USB Host context to be used by the USB Host driver.
 * 
 * @param ctx USB Host context
 */
void USBHostSetContext(struct SUSBHostContext *ctx)
{
	s_usbhost = ctx;
}

/**
 * @brief Get the USB Host context
 * 
 * Get the USB Host context used by the USB Host driver.
 * 
 * @return USB Host context
 */
struct SUSBHostContext *USBHostGetContext()
{
	return s_usbhost;
}

/**
 * @brief Probe the USB bus
 * 
 * Probe the USB bus to determine the state of the bus.
 * This function is primarily used to determine if a device is connected to the bus.
 * If a device is connected, the function will determine if the device is a full-speed or low-speed device.
 * After determining the device speed, the function will set the MAX3421E to the appropriate mode for access.
 * 
 * @return Bus state
 */
enum EBusState USBBusProbe()
{
	uint8_t bus_sample;

	bus_sample = MAX3421ReadByte(rHRSL); // Get J,K status
	bus_sample &= (bmJSTATUS|bmKSTATUS); // zero the rest of the byte

	switch( bus_sample )
	{
		case bmJSTATUS:
			if((MAX3421ReadByte(rMODE) & bmLOWSPEED) == 0 ) {
				MAX3421WriteByte(rMODE, MODE_FS_HOST);       //start full-speed host
#if defined(DEBUG_USB_HOST)
				UARTPrintf("J:FSHOST\n");
#endif
				return FSHOST;
			}
			else {
				MAX3421WriteByte(rMODE, MODE_LS_HOST);        //start low-speed host
#if defined(DEBUG_USB_HOST)
				UARTPrintf("J:LSHOST\n");
#endif
				return LSHOST;
			}
			break;
		case bmKSTATUS:
			if(( MAX3421ReadByte(rMODE) & bmLOWSPEED) == 0 )
			{
				MAX3421WriteByte(rMODE, MODE_LS_HOST);       //start low-speed host
#if defined(DEBUG_USB_HOST)
				UARTPrintf("K:LSHOST\n");
#endif
				return LSHOST;
			}
			else
			{
				MAX3421WriteByte(rMODE, MODE_FS_HOST);       //start full-speed host
#if defined(DEBUG_USB_HOST)
				UARTPrintf("K:FSHOST\n");
#endif
				return FSHOST;
			}
			break;
		case bmSE1:              //illegal state
#if defined(DEBUG_USB_HOST)
			UARTPrintf("SE1\n");
#endif
			return SE1;
			break;
		case bmSE0:              //disconnected state
			MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST | bmSEPIRQ);
#if defined(DEBUG_USB_HOST)
			UARTPrintf("SE0\n");
#endif
			return SE0;
			break;
	}

	return BUSUNKNOWN;
}

/** @brief Initialize the MAX3421E USB Host controller
 * 
 * Initialize the MAX3421E USB Host controller and the internal structures.
 * 
 * @param enableInterrupts Enable interrupts
 * @return Initial bus state
 */
enum EBusState USBHostInit(uint32_t enableInterrupts)
{
	// Must set context first
	if (s_usbhost==NULL)
		return BUSUNKNOWN;

	for(uint8_t i = 0; i<MAXDEVICES; i++ )
	{
		// Control endpoint
		s_deviceTable[i].endpointInfo[0].epAddr = 0;
		s_deviceTable[i].endpointInfo[0].epTransferType = 0;
		s_deviceTable[i].endpointInfo[0].maxPacketSize = 8;
		s_deviceTable[i].endpointInfo[0].pollInterval = 0;
		s_deviceTable[i].endpointInfo[0].receiveToggle = bmRCVTOG0;
		s_deviceTable[i].endpointInfo[0].sendToggle = bmSNDTOG0;

		// Other endpoints
		s_deviceTable[i].endpointInfo[1].epAddr = 0x81;
		s_deviceTable[i].endpointInfo[1].epTransferType = 0;
		s_deviceTable[i].endpointInfo[1].maxPacketSize = 8;
		s_deviceTable[i].endpointInfo[1].pollInterval = 0;
		s_deviceTable[i].endpointInfo[1].receiveToggle = bmRCVTOG0;
		s_deviceTable[i].endpointInfo[1].sendToggle = bmSNDTOG0;

		s_deviceTable[i].endpointInfo[2].epAddr = 0x82;
		s_deviceTable[i].endpointInfo[2].epTransferType = 0;
		s_deviceTable[i].endpointInfo[2].maxPacketSize = 8;
		s_deviceTable[i].endpointInfo[2].pollInterval = 0;
		s_deviceTable[i].endpointInfo[2].receiveToggle = bmRCVTOG0;
		s_deviceTable[i].endpointInfo[2].sendToggle = bmSNDTOG0;

		s_deviceTable[i].endpointInfo[3].epAddr = 0x83;
		s_deviceTable[i].endpointInfo[3].epTransferType = 0;
		s_deviceTable[i].endpointInfo[3].maxPacketSize = 8;
		s_deviceTable[i].endpointInfo[3].pollInterval = 0;
		s_deviceTable[i].endpointInfo[3].receiveToggle = bmRCVTOG0;
		s_deviceTable[i].endpointInfo[3].sendToggle = bmSNDTOG0;

		s_deviceTable[i].endpointInfo[4].epAddr = 0x84;
		s_deviceTable[i].endpointInfo[4].epTransferType = 0;
		s_deviceTable[i].endpointInfo[4].maxPacketSize = 8;
		s_deviceTable[i].endpointInfo[4].pollInterval = 0;
		s_deviceTable[i].endpointInfo[4].receiveToggle = bmRCVTOG0;
		s_deviceTable[i].endpointInfo[4].sendToggle = bmSNDTOG0;

		s_deviceTable[i].deviceClass = 0;
		s_deviceTable[i].connected = 0;
	}

	MAX3421WriteByte(rPINCTL, bmFDUPSPI | bmINTLEVEL | gpxSOF);

	int reset = MAX3421CtlReset();
	if (reset != 1)
	{
		*s_probe_result = CHIPFAILURE;
		return CHIPFAILURE; // Failed
	}

	MAX3421WriteByte(rIOPINS1, 0x0);
	MAX3421WriteByte(rIOPINS2, 0x0);

	MAX3421WriteByte(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST);
	MAX3421WriteByte(rHIEN, bmCONDETIE | bmFRAMEIE);

	MAX3421WriteByte(rHCTL, bmSAMPLEBUS);

	while(!(MAX3421ReadByte(rHCTL) & bmSAMPLEBUS)) {}; //wait for sample operation to finish

	enum EBusState probe_result = USBBusProbe();

	MAX3421WriteByte(rHIRQ, bmCONDETIRQ);
	if (enableInterrupts)
	{
		MAX3421WriteByte(rCPUCTL, bmIE);
	}

	// Reset keymap
	uint16_t* keystate = (uint16_t*)KEYBOARD_KEYSTATE_BASE;
	__builtin_memset(keystate, 0x00, 256*sizeof(uint16_t));

	*s_probe_result = probe_result;
	return probe_result;
}

/**
 * @brief Send a packet to the USB Host controller
 * 
 * Send a packet to the USB Host controller and wait for the response.
 * 
 * @param _token Token
 * @param _ep Endpoint
 * @param _nak_limit Maximum number of NAKs
 */
uint8_t USBDispatchPacket(uint8_t _token, uint8_t _ep, unsigned int _nak_limit)
{
	uint64_t timeout = E32ReadTime() + 200*ONE_MILLISECOND_IN_TICKS;
	uint8_t tmpdata;
	uint8_t rcode = 0xFE;
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

/**
 * @brief Send a control request to the USB Host controller
 * 
 * Send a control request to the USB Host controller and return the response.
 * 
 * @param _ep Endpoint
 * @param _direction Direction
 * @param _nak_limit Maximum number of NAKs
 */
uint8_t USBControlStatus(uint8_t _ep, uint8_t _direction, unsigned int _nak_limit)
{
	uint8_t rcode;
	if(_direction)
		rcode = USBDispatchPacket(tokOUTHS, _ep, _nak_limit);
	else
		rcode = USBDispatchPacket(tokINHS, _ep, _nak_limit);

	return rcode;
}

/**
 * @brief Transfer data from a USB host controller
 * 
 * Read data from a USB host controller and return the status.
 * 
 * @param _addr Address
 * @param _ep Endpoint
 * @param _nbytes Number of bytes to read
 * @param _data Data pointer to write to
 * @param _nak_limit Maximum number of NAKs
 * @return Status
 */
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

/**
 * @brief Transfer data to a USB host controller
 * 
 * Write data to a USB host controller and return the status.
 * 
 * @param _addr Address
 * @param _ep Endpoint
 * @param _nbytes Number of bytes to write
 * @param _data Data pointer to write from
 * @param _nak_limit Maximum number of NAKs
 * @return Status
 */
uint8_t USBOutTransfer(uint8_t _addr, uint8_t _ep, unsigned int _nbytes, char* _data, unsigned int nak_limit)
{
	uint8_t rcode = 0xFD, retry_count;
	char* data_p = _data;
	unsigned int bytes_tosend, nak_count;
	unsigned int bytes_left = _nbytes;
	uint8_t maxpktsize = s_deviceTable[_addr].endpointInfo[_ep].maxPacketSize;
	uint64_t timeout = E32ReadTime() + 200*ONE_MILLISECOND_IN_TICKS;

	if (!maxpktsize)
		return 0xFC;

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

/**
 * @brief Read a report from a HID device
 * 
 * Read a report from a HID device and return the status.
 * 
 * @param _addr Address
 * @param _ep Endpoint
 * @param _dataLen Data length
 * @param _data Data pointer to write to
 * @param _reportIndex Report index
 * @param _reportType Report type
 * @param _hidClass HID class
 * @return Status
 */
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

/**
 * @brief Output an error string in debug builds
 * 
 * Output an error string in debug builds matching the error code.
 * 
 * @param rcode Error code
 */
void USBErrorString(uint8_t rcode)
{
#ifdef DEBUG_USB_HOST
	if (rcode == hrSUCCESS)
		UARTPrintf("USB:SUCCESS\n");
	else if (rcode == hrBUSY)
		UARTPrintf("USB:BUSY\n");
	else if (rcode == hrBADREQ)
		UARTPrintf("USB:BADREQ\n");
	else if (rcode == hrUNDEF)
		UARTPrintf("USB:UNDEF\n");
	else if (rcode == hrNAK)
		UARTPrintf("USB:NAK\n");
	else if (rcode == hrSTALL)
		UARTPrintf("USB:STALL\n");
	else if (rcode == hrTOGERR)
		UARTPrintf("USB:TOGERR\n");
	else if (rcode == hrWRONGPID)
		UARTPrintf("USB:WRONGPID\n");
	else if (rcode == hrBADBC)
		UARTPrintf("USB:BADBC\n");
	else if (rcode == hrPIDERR)
		UARTPrintf("USB:PIDERR\n");
	else if (rcode == hrPKTERR)
		UARTPrintf("USB:PKTERR\n");
	else if (rcode == hrCRCERR)
		UARTPrintf("USB:CRCERR\n");
	else if (rcode == hrKERR)
		UARTPrintf("USB:KERR\n");
	else if (rcode == hrJERR)
		UARTPrintf("USB:JERR\n");
	else if (rcode == hrTIMEOUT)
		UARTPrintf("USB:TIMEOUT\n");
	else if (rcode == hrBABBLE)
		UARTPrintf("USB:BABBLE\n");
	else
	{
		UARTPrintf("USB:%02X\n", rcode);
	}
#endif
}

/**
 * @brief Send a control request to a USB host controller
 * 
 * Send a control request to a USB host controller and return the status.
 * 
 * @param _addr Address
 * @param _ep Endpoint
 * @param _bmReqType Request type
 * @param _bRequest Request
 * @param _wValLo Value low
 * @param _wValHi Value high
 * @param _wInd Index
 * @param _nbytes Number of bytes
 * @param _dataptr Data pointer
 * @param _nak_limit Maximum number of NAKs
 * @return Status
 */
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
		return(rcode);

	if(_dataptr != NULL )
	{
		// data stage, if present
		rcode = USBControlData(_addr, _ep, _nbytes, _dataptr, direction, _nak_limit);
	}

	if(rcode)
		return(rcode);

	rcode = USBControlStatus(_ep, direction, _nak_limit);
	return rcode;
}

/**
 * @brief Get a device descriptor from a USB host controller
 * 
 * Get a device descriptor from a USB host controller and return the status.
 * 
 * @param _addr Address
 * @param _ep Endpoint
 * @param _hidclass HID class
 * @param _output Output pointer
 * @param _outlen Output length
 * @return Status
 */
uint8_t USBParseDescriptor(uint8_t _addr, uint8_t *_desc, uint8_t* _dtype, uint8_t* _offset, uint8_t* _hidclass)
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
			stringCount += desc->iManufacturer!=0 ? 1:0;
			stringCount += desc->iProduct!=0 ? 1:0;
			stringCount += desc->iSerialNumber!=0 ? 1:0;
#ifdef DEBUG_USB_HOST
			UARTPrintf("imanu,iprod,iseri: %x %x %x\n", desc->iManufacturer, desc->iProduct, desc->iSerialNumber);

#endif
		}
		break;

		case USBDESCTYPE_CONFIGURATION:
		{
			*_dtype = USBDESCTYPE_CONFIGURATION;
			*_offset = sizeof(struct USBConfigurationDescriptor);
			struct USBConfigurationDescriptor* desc = (struct USBConfigurationDescriptor*)_desc;
			s_cval = desc->bConfigurationValue;
			stringCount += desc->iConfiguration!=0 ? 1:0;
#ifdef DEBUG_USB_HOST
			UARTPrintf("config: %x\n", desc->bConfigurationValue);
#endif
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

			stringCount += desc->iInterface!=0 ? 1:0;
			if ((desc->bInterfaceClass == USBClass_HID) && (desc->bInterfaceSubClass == 1))
			{
				//s_protosubclass = 1; // Boot protocol supported
#ifdef DEBUG_USB_HOST
				UARTPrintf("hasboot\n");
#endif
			}
			if (desc->bInterfaceClass == USBClass_HID)
			{
				//desc->bInterfaceNumber?
				switch (desc->bInterfaceProtocol)
				{
					case 0:
						*_hidclass = 0;
						break;
					case 1:
						*_hidclass = 1;
						break;
					case 2:
						*_hidclass = 2;
						break;
					default:
						*_hidclass = 3;
						break;
				}
#ifdef DEBUG_USB_HOST
				UARTPrintf("HIDclass: %x\n", *_hidclass);
#endif
			}
		}
		break;

		case USBDESCTYPE_ENDPOINT:
		{
			*_dtype = USBDESCTYPE_ENDPOINT;
			*_offset = sizeof(struct USBEndpointDescriptor);
			struct USBEndpointDescriptor *desc = (struct USBEndpointDescriptor*)_desc;

			uint8_t eaddr = desc->bEndpointAddress&0x0F;
			s_deviceTable[_addr].endpointInfo[eaddr].epAddr = desc->bEndpointAddress;
			s_deviceTable[_addr].endpointInfo[eaddr].maxPacketSize = desc->wMaxPacketSize;
			s_deviceTable[_addr].endpointInfo[eaddr].pollInterval = desc->bInterval;
			s_deviceTable[_addr].endpointInfo[eaddr].epTransferType = desc->bEndpointAddress&0x80 ? 0 : 1;
			s_deviceTable[_addr].endpointInfo[eaddr].attribs = desc->bmAttributes;

#ifdef DEBUG_USB_HOST
			UARTPrintf("EP addr, maxsize, interval: %x %x %x\n", desc->bEndpointAddress, desc->wMaxPacketSize, desc->bInterval);
#endif
		}
		break;

		case USBDESCTYPE_HIDSPECIFIC:
		{
			*_dtype = USBDESCTYPE_HIDSPECIFIC;
			*_offset = sizeof(struct USBHIDDescriptor);
			struct USBHIDDescriptor *desc = (struct USBHIDDescriptor*)_desc;

			s_HIDDescriptorLen = desc->bNumDescriptors;
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
		}
		break;

		default:
		{
			*_dtype = USBDESCTYPE_UNKNOWN;
			*_offset = 0xFF;
		}
		break;
	}

	return stringCount;
}

/**
 * @brief Get a device descriptor from a USB host controller
 * 
 * Get a device descriptor from a USB host controller and return the status.
 * The HID class is also returned in addition to the device descriptor.
 * 
 * @param _addr Address
 * @param _ep Endpoint
 * @param _hidclass HID class
 * @param _output Output pointer
 * @param _outlen Output length
 * @return Status
 */
uint8_t USBGetDeviceDescriptor(uint8_t _addr, uint8_t _ep, uint8_t *_hidclass, void* _output, uint32_t *_outlen)
{
	s_protosubclass = 0;
	struct USBDeviceDescriptor ddesc;
	uint8_t *target = (uint8_t*)_output;

	s_deviceTable[_addr].endpointInfo[_ep].maxPacketSize = 8;
    uint8_t rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0x00, USB_DESCRIPTOR_DEVICE, 0x0000, 8, (char*)&ddesc, 64);

	if (rcode != 0)
	{
#ifdef DEBUG_USB_HOST
		UARTPrintf("Control request(1) failed\n");
#endif
		USBErrorString(rcode);
		return rcode;
	}

	s_deviceTable[_addr].endpointInfo[_ep].maxPacketSize = ddesc.bMaxPacketSizeEP0;

	// Retry with actual descriptor size
	// 18 == USB_DEVICE_DESCRIPTOR_SIZE
	rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0x00, USB_DESCRIPTOR_DEVICE, 0x0000, 18, (char*)&ddesc, 64);

	if (rcode != 0)
	{
#ifdef DEBUG_USB_HOST
		UARTPrintf("Control request(2) failed\n");
#endif
		USBErrorString(rcode);
		return rcode;
	}
	
	int stringCount = 0;

	uint8_t dtype = USBDESCTYPE_UNKNOWN;
	uint8_t offset = 0;
	stringCount += USBParseDescriptor(_addr, (uint8_t*)&ddesc, &dtype, &offset, _hidclass);
	if (target)
	{
		uint32_t L = sizeof(struct USBDeviceDescriptor);
		__builtin_memcpy(target, &ddesc, L);
		target += L;
		*_outlen += L;
	}

	struct USBConfigurationDescriptor cdef;
	for (uint8_t c=0; c<ddesc.bNumConfigurations; ++c)
	{
		// 9 == USB_CONFIGURATION_DESCRIPTOR_SIZE
		rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, c, USB_DESCRIPTOR_CONFIGURATION, 0x0000, 9, (char*)&cdef, 64);
		if (target)
		{
			uint32_t L = sizeof(struct USBConfigurationDescriptor);
			__builtin_memcpy(target, &cdef, L);
			target += L;
			*_outlen += L;
		}

		if (rcode != 0)
		{
#ifdef DEBUG_USB_HOST
		UARTPrintf("Control request(3) failed\n");
#endif
			USBErrorString(rcode);
			return rcode;
		}

		// re-request config descriptor with actual data size (cdef.wTotalLength)
		char *tmpdata = (char*)KERNEL_TEMP_MEMORY;
		rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, c, USB_DESCRIPTOR_CONFIGURATION, 0x0000, cdef.wTotalLength, tmpdata, 64);
		if (target)
		{
			uint32_t L = cdef.wTotalLength;
			__builtin_memcpy(target, tmpdata, L);
			target += L;
			*_outlen += L;
		}

		if (rcode != 0)
		{
#ifdef DEBUG_USB_HOST
		UARTPrintf("Control request(4) failed\n");
#endif
			USBErrorString(rcode);
			return rcode;
		}

		dtype = USBDESCTYPE_UNKNOWN;
		offset = 0;
		stringCount += USBParseDescriptor(_addr, (uint8_t*)&cdef, &dtype, &offset, _hidclass);

		while (offset < cdef.wTotalLength)
		{
			uint8_t descsize = 0;
			stringCount += USBParseDescriptor(_addr, (uint8_t*)&tmpdata[offset], &dtype, &descsize, _hidclass);
			if (descsize == 0xFF)
				break;
			offset += descsize;
		}

		// Get language descriptor and strings
		struct USBStringLanguageDescriptor lang;
		rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0, USB_DESCRIPTOR_STRING, 0x0000, 4, (char*)&lang, 64);
		if (target)
		{
			uint32_t L = sizeof(struct USBStringLanguageDescriptor);
			__builtin_memcpy(target, &lang, L);
			target += L;
			*_outlen += L;
		}

		if (rcode != 0)
		{
#ifdef DEBUG_USB_HOST
		UARTPrintf("Control request(5) failed\n");
#endif
			USBErrorString(rcode);
			return rcode;
		}

		// Some devices can't read strings somehow
		for (int s=0; s<stringCount; ++s)
		{
			struct USBStringDescriptor str;

			// Only length, though I thought we'd have to read the full USBStringDescriptor
			rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, s+1, USB_DESCRIPTOR_STRING, 0x0000, 1, (char*)&str, 64);
			if (rcode == 0 && str.bLength != 0)
				rcode = USBControlRequest(_addr, _ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, s+1, USB_DESCRIPTOR_STRING, 0x0000, str.bLength, (char*)&str, 64);

			if (target && rcode == 0)
			{
				uint32_t L = 2 + str.bLength; // including the blength+bdescriptortype
				__builtin_memcpy(target, &str, L);
				target += L;
				*_outlen += L;
				target[L] = 0;
			}

			if (rcode)
			{
				// Clear stall condition if this is not supported
				uint16_t epAddress = 0x81;	// TODO: get it from device->endpoints[_ep]->epAddress
				rcode = USBControlRequest(_addr, _ep, bmREQ_CLEAR_FEATURE, USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, 0, epAddress, 0, NULL, 64);
				if (rcode == 0)
				{
#ifdef DEBUG_USB_HOST
					UARTPrintf("Clear stall failed\n");
#endif
					USBErrorString(rcode);
					break;
				}
			}
		}
	}

	return 0;
}

/**
 * @brief Attach a USB device to the USB host controller
 * 
 * Attach a USB device to the USB host controller by assigning an address to it.
 * 
 * @param _paddr Address pointer
 * @param _pep Endpoint pointer
 * @return Status
 */
uint8_t USBAttach(uint8_t *_paddr, uint8_t *_pep)
{
	for (int i=1; i<8; ++i)
	{
		if (s_deviceTable[i].connected == 0)
		{
			s_deviceTable[i].connected = 1;

			// Set new address
    		uint8_t rcode = USBControlRequest(0, 0, bmREQ_SET, USB_REQUEST_SET_ADDRESS, i, 0x00, 0x0000, 0x0000, NULL, 64);
			if(rcode == 0)
			{
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

/**
 * @brief Detach a USB device from the USB host controller
 * 
 * Detach a USB device from the USB host controller by removing it from the device table.
 * 
 * @param _addr Address
 * @return Status
 */
uint8_t USBDetach(uint8_t _addr)
{
	// Can't detach device at default address (0)
	if (_addr != 0)
	{
		s_deviceTable[_addr].connected = 0;
		s_deviceTable[_addr].endpointInfo[0].sendToggle = bmSNDTOG0;
		s_deviceTable[_addr].endpointInfo[0].receiveToggle = bmRCVTOG0;
		s_deviceTable[_addr].endpointInfo[1].sendToggle = bmSNDTOG0;
		s_deviceTable[_addr].endpointInfo[1].receiveToggle = bmRCVTOG0;
		s_deviceTable[_addr].endpointInfo[2].sendToggle = bmSNDTOG0;
		s_deviceTable[_addr].endpointInfo[2].receiveToggle = bmRCVTOG0;
		s_deviceTable[_addr].endpointInfo[3].sendToggle = bmSNDTOG0;
		s_deviceTable[_addr].endpointInfo[3].receiveToggle = bmRCVTOG0;
		s_deviceTable[_addr].endpointInfo[4].sendToggle = bmSNDTOG0;
		s_deviceTable[_addr].endpointInfo[4].receiveToggle = bmRCVTOG0;
	}

	return 0;
}

/**
 * @brief Configure a HID device
 * 
 * Configure a HID device using control requests.
 * It will also try to set the protocol to boot protocol if the HID class is a mouse.
 * 
 * @param _hidClass HID class
 * @param _addr Address
 * @param _ep Endpoint
 * @return Status
 */
uint8_t USBConfigHID(uint8_t _hidClass, uint8_t _addr, uint8_t _ep)
{
	uint8_t config = s_cval; // Usually 1
	uint8_t interface = 0;

	uint8_t rcode = USBControlRequest(_addr, _ep, bmREQ_SET, USB_REQUEST_SET_CONFIGURATION, config, 0x00, 0x0000, 0x0000, NULL, 64);

	if (rcode == 0)
	{
		rcode = USBControlRequest(_addr, _ep, bmREQ_SET, USB_REQUEST_SET_INTERFACE, interface, 0x00, 0x0000, 0x0000, NULL, 64);

		// Check if interface changes are supported (maybe we only have one default)
		if (rcode == hrSTALL)
		{
			uint16_t epAddress = 0x81;	// TODO: get it from device->endpoints[_ep]->epAddress
			rcode = USBControlRequest(_addr, _ep, bmREQ_CLEAR_FEATURE, USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, 0, epAddress, 0, NULL, 64);
		}
	}

	if (s_protosubclass == 1 && _hidClass == 2) // Using boot protocol, allowed only for mouse (keyboard and other use report protocol)
	{
		// iface is interface number
		uint8_t iface = 0;
		// Could also use HID_REPORT_PROTOCOL for report protocol
		rcode = USBControlRequest(_addr, _ep, bmREQ_HIDOUT, HID_REQUEST_SET_PROTOCOL, USB_HID_BOOT_PROTOCOL, 0x00, iface, 0x0000, NULL, 64);
	}

	return rcode;
}

/**
 * @brief Get a HID descriptor from a USB host controller
 * 
 * Get a HID descriptor from a USB host controller and return the status.
 * This data can be parsed using http://eleccelerator.com/usbdescreqparser/
 * (Just remember to click USB HID Report Descriptor to parse)
 * 
 * For a keyboard we expect to start with:
 * 0501 (generic keyboard)
 * 0906 (usage keyboard)
 * 
 * @param _addr Address
 * @param _ep Endpoint
 * @param _protocol Protocol
 * @return Status
 */
uint8_t USBGetHIDDescriptor(uint8_t _addr, uint8_t _ep, uint8_t *_protocol)
{
	// TODO: We need kalloc() for this kind of unknown data pool
	char *tmpdata = (char*)KERNEL_TEMP_MEMORY;
    uint8_t rcode = USBControlRequest(_addr, _ep, bmREQ_HIDREPORT, USB_REQUEST_GET_DESCRIPTOR, 0x00, HID_DESCRIPTOR_REPORT, 0x0000, s_HIDDescriptorLen, tmpdata, 64);

	if (rcode != 0)
		return rcode;

	if (tmpdata[3] == 0x02) 
	{
		*_protocol = HID_PROTOCOL_MOUSE;
#ifdef DEBUG_USB_HOST
		UARTPrintf("Mouse\n");
#endif
	}
	else if (tmpdata[3] == 0x04) 
	{
		*_protocol = HID_PROTOCOL_JOYSTICK;
#ifdef DEBUG_USB_HOST
		UARTPrintf("Joystick\n");
#endif
	}
	else if (tmpdata[3] == 0x05) 
	{
		*_protocol = HID_PROTOCOL_GAMEPAD;
#ifdef DEBUG_USB_HOST
		UARTPrintf("Gamepad\n");
#endif
	}
	else if (tmpdata[3] == 0x06)
	{
		*_protocol = HID_PROTOCOL_KEYBOARD;
#ifdef DEBUG_USB_HOST
		UARTPrintf("Keyboard\n");
#endif
	}
	else
	{
		*_protocol = HID_PROTOCOL_NONE;
#ifdef DEBUG_USB_HOST
		UARTPrintf("Non-HID\n");
#endif
	}

	return 0;
}

/**
 * @brief Set the address of a USB device
 * 
 * Set the address of a USB device using a direct write to the MAX3421E.
 * 
 * @param _addr Address
 * @param _ep Endpoint
 */
void USBSetAddress(uint8_t _addr, uint8_t _ep)
{
	MAX3421WriteByte(rPERADDR, _addr);
	// To cross to a low speed peripheral over a USB HUB
	// Only applicable to low speed peripheral
	//uint8_t mode = MAX3421ReadByte(rMODE);
	//MAX3421WriteByte(rMODE, mode | bmHUBPRE);
}

/**
 * @brief Read HID data from a USB host controller
 * 
 * Read HID data from a USB host controller and return the status.
 * The HID data for a given report type contains bytes that represent the state of the device.
 * For a keyboard, this could be the state of the keys.
 * For a mouse, this could be the state of the buttons and the movement of the mouse.
 * 
 * @param _addr Address
 * @param _ep Endpoint
 * @param _dataLen Data length
 * @param _data Data pointer
 * @param _reportIndex Report index
 * @param _reportType Report type
 * @param _hidClass HID class
 * @return Status
 */
uint8_t USBReadHIDData(uint8_t _addr, uint8_t _ep, uint8_t _dataLen, uint8_t *_data, uint8_t _reportIndex, uint8_t _reportType, uint8_t _hidClass)
{
	uint8_t rcode;

	if (s_protosubclass == 1 && _hidClass == 2) // Use boot protocol for mouse
	{
		// Using interrupt endpoint
		MAX3421WriteByte(rPERADDR, _addr);
		s_deviceTable[_addr].endpointInfo[_ep].receiveToggle = bmRCVTOG1;
		rcode = USBInTransfer(_addr, _ep, _dataLen, (char*)_data, 64);
	}
	else // Use report protocol of keyboard and others
	{
		// Using control endpoint
		uint8_t iface = 0; // TODO: gather correct read interface index
    	rcode = USBControlRequest(_addr, _ep, bmREQ_HIDIN, HID_REQUEST_GET_REPORT, _reportIndex, _reportType, iface, _dataLen, (char*)_data, 64);
	}

	return rcode;
}

/**
 * @brief Write HID data to a USB host controller
 * 
 * Write HID data to a USB host controller and return the status.
 * For a keyboard, this could be the state of the LEDs, such as caps lock.
 * 
 * @param _addr Address
 * @param _ep Endpoint
 * @param _data Data pointer
 * @return Status
 */
uint8_t USBWriteHIDData(uint8_t _addr, uint8_t _ep, uint8_t *_data)
{
	uint8_t reportID = 0;
	uint8_t reportType = 2; // LED control
	uint8_t iface = 0; // TODO: gather correct write interface index
    uint8_t rcode = USBControlRequest(_addr, _ep, bmREQ_HIDOUT, HID_REQUEST_SET_REPORT, reportID, reportType, iface, 1, (char*)_data, 64);

	return rcode;
}
