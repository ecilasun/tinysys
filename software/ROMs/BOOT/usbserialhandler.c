#include "usbserialhandler.h"
#include "usbserial.h"
#include "leds.h"
#include "ringbuffer.h"

// See:
// https://github.com/MicrochipTech/mla_usb/blob/master/src/usb_device_cdc.c
// https://github.com/tlh24/myopen/blob/master/firmware_stage7/usb.c

#define STALL_EP0 MAX3420WriteByte(rEPSTALLS, 0x23); 
#define SETBIT(reg,val) MAX3420WriteByte(reg, (MAX3420ReadByte(reg)|val));
#define CLRBIT(reg,val) MAX3420WriteByte(reg, (MAX3420ReadByte(reg)&~val));

// 115200 baud, 1 stop bit, no parity, 8bit data
static struct USBCDCLineCoding s_lineCoding = {115200, 0, 0, 8};

//static uint8_t s_outputbuffer[64];
//static uint32_t s_outputbufferlen = 0;

static uint32_t s_suspended = 0;

static uint8_t devconfig = 0;
static uint8_t devaddrs = 0;
static uint8_t encapsulatedcommand[0x20];
static uint32_t addrx0x81stalled = 0;

void set_configuration(uint8_t *SUD)
{
	devconfig = SUD[wValueL];	   // Store the config value

	if(devconfig != 0)			  // If we are configured, 
		SETBIT(rUSBIEN, bmSUSPIE);  // start looking for SUSPEND interrupts
	MAX3420ReadByte(rFNADDR | 0x1);	 // dummy read to set the ACKSTAT bit
}

void get_configuration(void)
{
	MAX3420WriteByte(rEP0FIFO, devconfig);	// Send the config value
	MAX3420WriteByte(rEP0BC | 0x1, 1);
}

void send_status(uint8_t *SUD)
{
	// Device: 0x0 -> bus powered, no remove wakeup
	// Interface: must be zero
	// Endpoint: must be zero
	uint8_t twozero[2] = {0, 0};
	MAX3420WriteBytes(rEP0FIFO, 2, (uint8_t*)&twozero);
	MAX3420WriteByte(rEP0BC | 0x1, 2);
}

void send_descriptor(uint8_t *SUD)
{
	uint16_t reqlen, sendlen, desclen;
	unsigned char *pDdata;			// pointer to ROM Descriptor data to send
	//
	// NOTE This function assumes all descriptors are 64 or fewer bytes and can be sent in a single packet
	//
	desclen = 0;					// check for zero as error condition (no case statements satisfied)
	reqlen = SUD[wLengthL] + 256*SUD[wLengthH];	// 16-bit

	// Access descriptor from usb utils
	struct SUSBContext *uctx = USBSerialGetContext();

	uint8_t desctype = SUD[wValueH];

	switch (desctype)
	{
		case  GD_DEVICE:
		{
			desclen = uctx->device.bLength;
			pDdata = (unsigned char*)&uctx->device;
			break;
		}
		case  GD_CONFIGURATION:
		{
			desclen = uctx->config.wTotalLength;
			pDdata = (unsigned char*)&uctx->config;
			break;
		}
		case  GD_STRING:
		{
			uint8_t idx = SUD[wValueL];  // String index
			desclen = uctx->strings[idx].bLength;
			pDdata = (unsigned char*)&uctx->strings[idx];
			break;
		}
	}	// end switch on descriptor type

	if (desclen != 0) // one of the case statements above filled in a value
	{
		sendlen = (reqlen <= desclen) ? reqlen : desclen; // send the smaller of requested and avaiable
		MAX3420WriteBytes(rEP0FIFO, sendlen, pDdata);
		MAX3420WriteByte(rEP0BC | 0x1, sendlen);   // load EP0BC to arm the EP0-IN transfer & ACKSTAT
	}
	else
		STALL_EP0  // none of the descriptor types match
}

void featurecontrol(int ctltype, uint16_t value, uint16_t index)
{
	// ctltype == 0 -> clear
	// ctltype == 1 -> set

	if (value == 0x00) // ENDPOINT_HALT
	{
		uint8_t stallmask = MAX3420ReadByte(rEPSTALLS);
		if (ctltype == 1) // Set
		{
			if (index==0x81) // Control
			{
				stallmask |= bmSTLEP1OUT;
				addrx0x81stalled = 1;
			}
			/*else -> can't halt */
		}
		else // Clear
		{
			if (index==0x81) // Control
			{
				stallmask &= ~bmSTLEP1OUT;
				addrx0x81stalled = 0;
				MAX3420WriteByte(rCLRTOGS, bmCTGEP1OUT);
			}
			/*else -> can't resume */
		}
		MAX3420WriteByte(rEPSTALLS, stallmask | bmACKSTAT);
	}
	else if (value == 0x01) // REMOTE_WAKEUP
	{
		/*uint8_t addrs =*/ MAX3420ReadByte(rFNADDR);
	}
	else // Unknown
	{
		STALL_EP0
	}
}

void std_request(uint8_t *SUD)
{
	switch(SUD[bRequest])
	{
		case	SR_GET_DESCRIPTOR:
		{
			send_descriptor(SUD);
			break;
		}
		case	SR_SET_FEATURE:
		{
			uint16_t value = (SUD[wValueH]<<8) | SUD[wValueL];
			uint16_t index = (SUD[wIndexH]<<8) | SUD[wIndexL];

			switch (SUD[bmRequestType])
			{
				// EP0
				case 0x00: STALL_EP0 break;
				// Interface
				case 0x01: MAX3420WriteByte(rEP0BC | 0x1, 0); break; // Zero byte response - ACK
				// Endpoint
				case 0x02: featurecontrol(1, value, index); break;
				// Unknown
				default: STALL_EP0 break;
			}
			break;
		}
		case	SR_CLEAR_FEATURE:
		{
			uint16_t value = (SUD[wValueH]<<8) | SUD[wValueL];
			uint16_t index = (SUD[wIndexH]<<8) | SUD[wIndexL];

			switch (SUD[bmRequestType])
			{
				// EP0
				case 0x00: STALL_EP0 break;
				// Interface
				case 0x01: MAX3420WriteByte(rEP0BC | 0x1, 0); break; // Zero byte response - ACK
				// Endpoint
				case 0x02: featurecontrol(0, value, index); break;
				// Unknown
				default: STALL_EP0 break;
			}
			break;
		}
		case	SR_GET_STATUS:			send_status(SUD);					break;
		case	SR_SET_INTERFACE:		STALL_EP0;							break;
		case	SR_GET_INTERFACE:		STALL_EP0;							break;
		case	SR_GET_CONFIGURATION:   get_configuration();				break;
		case	SR_SET_CONFIGURATION:   set_configuration(SUD);				break;
		case	SR_SET_ADDRESS:
		{
			devaddrs = MAX3420ReadByte(rFNADDR | 0x1);
			break;
		}
		default:
		{
			STALL_EP0
			break;
		}
	}
}

void class_request(uint8_t *SUD)
{
	// Microchip AN1247
	// https://www.microchip.com/content/dam/mchp/documents/OTH/ProductDocuments/LegacyCollaterals/01247a.pdf
	//
	// req  type value length	   data							name
	// 0	0x21 0	 numdatabytes control protocol based command  sendencapsulatedcommand
	// 1	0xA1 0	 numdatabytes protocol dependent data		 getencapsulatedresponse
	// 0x20 0x21 0	 7			line coding data				setlinecoding
	// 0x21 0xA1 0	 7			line coding data				getlinecoding
	// 0x22 0x21 2	 0			none							setcontrollinestate

	uint16_t reqlen = SUD[wLengthL] + 256*SUD[wLengthH];	// 16-bit

	switch(SUD[bRequest])
	{
		case CDC_SENDENCAPSULATEDRESPONSE:
		{
			// Command issued

			// RESPONSE_AVAILABLE(0x00000001) + 0x00000000
			// or
			// just two zeros for no response
			MAX3420ReadBytes(rEP0FIFO, reqlen, encapsulatedcommand);
			MAX3420WriteByte(rEP0BC | 0x1, 0); // Zero byte response - ACK

			break;
		}
		case CDC_GETENCAPSULATEDRESPONSE:
		{
			// Response requested

			// When unhandled, respond with a one-byte zero and do not stall the endpoint
			MAX3420WriteByte(rEP0FIFO, 0);
			MAX3420WriteByte(rEP0BC | 0x1, 1);

			break;
		}
		case CDC_SETLINECODING:
		{
			// Data rate/parity/number of stop bits etc

			struct USBCDCLineCoding newcoding;
			MAX3420ReadBytes(rEP0FIFO, sizeof(struct USBCDCLineCoding), (uint8_t*)&newcoding);

			s_lineCoding = newcoding;

			MAX3420WriteByte(rEP0BC | 0x1, 0); // Zero byte response - ACK

			break;
		}
		case CDC_GETLINECODING:
		{
			// Data rate/parity/number of stop bits etc
			// offset name		size description
			// 0	  dwDTERate   4	rate in bits per second
			// 4	  bCharFormat 1	stop bits: 0:1, 1:1.5, 2:2
			// 5	  bParityType 1	parity: 0:none,1:odd,2:even,3:mark,4:space
			// 6	  bDataBits   1	data bits: 5,6,7,8 or 16

			MAX3420WriteBytes(rEP0FIFO, sizeof(struct USBCDCLineCoding), (uint8_t*)&s_lineCoding);
			MAX3420WriteByte(rEP0BC | 0x1, sizeof(struct USBCDCLineCoding));

			break;
		}
		case CDC_SETCONTROLLINESTATE:
		{
			// bits  description
			// 15:2  reserved
			// 1	 carrier control signal: 0:inactive,1:active
			// 0	 DTR: 0:notpresent, 1:present

			MAX3420WriteByte(rEP0BC | 0x1, 0); // Zero byte response - ACK
			//STALL_EP0

			break;
		}
	}
}

void DoSetup()
{
	uint8_t SUD[8];
	MAX3420ReadBytes(rSUDFIFO, 8, SUD);

	switch(SUD[bmRequestType] & 0x60)
	{
		case 0x00: std_request(SUD); break;
		case 0x20: class_request(SUD); break;
		case 0x40:
		{
			STALL_EP0
			break;
		}
		default:
		{
			STALL_EP0
			break;
		}
	}
}

/*void EmitBufferedOutput()
{
	// If we have something pending in the output buffer, stream it out
	if (s_outputbufferlen != 0)
		USBWriteBytes(rEP2INFIFO, s_outputbufferlen, s_outputbuffer);
	USBWriteByte(rEP2INBC, s_outputbufferlen); // Zero or more bytes output
	// Done sending
	s_outputbufferlen = 0;
}*/

void BufferIncomingData()
{
	// Incoming EP1 data package
	uint8_t cnt = MAX3420ReadByte(rEP1OUTBC) & 63; // Cap size to 0..63
	if (cnt)
	{
		// Stash incoming data into the ringbuffer
		for (uint8_t i=0; i<cnt; ++i)
		{
			uint32_t incoming = MAX3420ReadByte(rEP1OUTFIFO);
			RingBufferWrite(&incoming, sizeof(uint32_t));
		}
	}
}

void HandleUSBSerial()
{
	uint32_t currLED = LEDGetState();

	// Initial value of rEPIRQ should be 0x19
	uint8_t epIrq = MAX3420ReadByte(rEPIRQ);
	uint8_t usbIrq = MAX3420ReadByte(rUSBIRQ);

	// Endpoint irq

	if (epIrq & bmSUDAVIRQ)
	{
		// Setup data available, 8 bytes data to follow
		LEDSetState(currLED | 0xC);
		DoSetup();
		MAX3420WriteByte(rEPIRQ, bmSUDAVIRQ); // Clear
	}
	else if (epIrq & bmOUT1DAVIRQ)
	{
		// Input
		LEDSetState(currLED | 0x8);
		BufferIncomingData();
		MAX3420WriteByte(rEPIRQ, bmOUT1DAVIRQ); // Clear
	}
	/*else if (epIrq & bmIN2BAVIRQ)
	{
		LEDSetState(currLED | 0x4);
		//USBWriteByte(rEPIRQ, bmIN2BAVIRQ); // Clear
		// Application note states we have to clear BAV IRQs by writing count for double-buffering to work
		MAX3420WriteByte(rEP2INBC, 0);
		// Output
		EmitBufferedOutput();
	}*/
	else if (epIrq & bmIN3BAVIRQ)
	{
		MAX3420WriteByte(rEPIRQ, bmIN3BAVIRQ); // Clear
	}
	else if (epIrq & bmIN0BAVIRQ)
	{
		MAX3420WriteByte(rEPIRQ, bmIN0BAVIRQ); // Clear
	}

	if (usbIrq & bmSUSPIRQ)
	{
		MAX3420WriteByte(rUSBIRQ, bmSUSPIRQ); // Clear
		s_suspended = 1;
	}
	else if (usbIrq & bmBUSACTIRQ)
	{
		MAX3420WriteByte(rUSBIRQ, bmBUSACTIRQ); // Clear
		s_suspended = 0;
	}
	else if (usbIrq & bmURESIRQ) // Bus reset
	{
		MAX3420WriteByte(rUSBIRQ, bmURESIRQ); // Clear
	}
	else if (usbIrq & bmURESDNIRQ) // Resume
	{
		MAX3420WriteByte(rUSBIRQ, bmURESDNIRQ); // Clear
		s_suspended = 0;
		MAX3420EnableIRQs();
	}

	LEDSetState(currLED);
}
