#pragma once

#include "usbcommon.h"

extern volatile uint32_t *IO_USBCTRX;
extern volatile uint32_t *IO_USBCSTA;

// Descriptor types

#define USBDesc_Device						0x01
#define USBDesc_Configuration				0x02
#define USBDesc_String						0x03
#define USBDesc_Interface					0x04
#define USBDesc_Endpoint					0x05
#define USBDesc_DeviceQualifier				0x06
#define USBDesc_OtherSpeedConfiguration		0x07
#define USBDesc_InterfacePower				0x08

void MAX3420FlushOutputFIFO();
uint8_t MAX3420GetGPX();
uint8_t MAX3420ReadByte(uint8_t command);
void MAX3420WriteByte(uint8_t command, uint8_t data);
int MAX3420ReadBytes(uint8_t command, uint8_t length, uint8_t *buffer);
void MAX3420WriteBytes(uint8_t command, uint8_t length, uint8_t *buffer);
void MAX3420EnableIRQs();

void USBSerialSetContext(struct SUSBContext *ctx);
struct SUSBContext *USBSerialGetContext();
int USBSerialInit(uint32_t enableInterrupts);

int USBSerialWriteN(const char *outstring, uint32_t count);
int USBSerialWrite(const char *outstring);
int USBSerialWriteHexByte(const uint8_t i);
int USBSerialWriteHex(const uint32_t i);
int USBSerialWriteDecimal(const int32_t i);

// MAX3420E Registers
#define rEP0FIFO    0<<3
#define rEP1OUTFIFO 1<<3
#define rEP2INFIFO  2<<3
#define rEP3INFIFO  3<<3
#define rSUDFIFO    4<<3
#define rEP0BC      5<<3
#define rEP1OUTBC   6<<3
#define rEP2INBC    7<<3
#define rEP3INBC    8<<3
#define rEPSTALLS   9<<3
#define rCLRTOGS    10<<3
#define rEPIRQ      11<<3
#define rEPIEN      12<<3
#define rUSBIRQ     13<<3
#define rUSBIEN     14<<3
#define rUSBCTL     15<<3
#define rCPUCTL     16<<3
#define rPINCTL     17<<3
#define rREVISION   18<<3
#define rFNADDR     19<<3
#define rGPIO       20<<3


// MAX3420E bit masks for register bits
// R9 EPSTALLS Register
#define bmACKSTAT   0x40
#define bmSTLSTAT   0x20
#define bmSTLEP3IN  0x10
#define bmSTLEP2IN  0x08
#define bmSTLEP1OUT 0x04
#define bmSTLEP0OUT 0x02
#define bmSTLEP0IN  0x01

// R10 CLRTOGS Register
#define bmEP3DISAB  0x80
#define bmEP2DISAB  0x40
#define bmEP1DISAB  0x20
#define bmCTGEP3IN  0x10
#define bmCTGEP2IN  0x08
#define bmCTGEP1OUT 0x04

// R11 EPIRQ register bits
#define bmSUDAVIRQ  0x20
#define bmIN3BAVIRQ 0x10
#define bmIN2BAVIRQ 0x08
#define bmOUT1DAVIRQ 0x04
#define bmOUT0DAVIRQ 0x02
#define bmIN0BAVIRQ 0x01

// R12 EPIEN register bits
#define bmSUDAVIE   0x20
#define bmIN3BAVIE  0x10
#define bmIN2BAVIE  0x08
#define bmOUT1DAVIE 0x04
#define bmOUT0DAVIE 0x02
#define bmIN0BAVIE  0x01

// R13 USBIRQ register bits
#define bmURESDNIRQ 0x80
#define bmVBUSIRQ   0x40
#define bmNOVBUSIRQ 0x20
#define bmSUSPIRQ   0x10
#define bmURESIRQ   0x08
#define bmBUSACTIRQ 0x04
#define bmRWUDNIRQ  0x02
#define bmOSCOKIRQ  0x01

// R14 USBIEN register bits
#define bmURESDNIE  0x80
#define bmVBUSIE    0x40
#define bmNOVBUSIE  0x20
#define bmSUSPIE    0x10
#define bmURESIE    0x08
#define bmBUSACTIE  0x04
#define bmRWUDNIE   0x02
#define bmOSCOKIE   0x01

// R15 USBCTL Register
#define bmHOSCSTEN  0x80
#define bmVBGATE    0x40
#define bmCHIPRES   0x20
#define bmPWRDOWN   0x10
#define bmCONNECT   0x08
#define bmSIGRWU    0x04

// R16 CPUCTL Register
#define bmIE        0x01

// R17 PINCTL Register
#define bmFDUPSPI   0x10
#define bmINTLEVEL  0x08
#define bmPOSINT    0x04
#define bmGPOB      0x02
#define bmGPOA      0x01

//
// GPX[B:A] settings (PINCTL register)
#define gpxOPERATE  0x00
#define gpxVBDETECT 0x01
#define gpxBUSACT   0x02
#define gpxSOF      0x03

// ************************
// Standard USB Requests
#define SR_GET_STATUS 0x00			// Get Status
#define SR_CLEAR_FEATURE 0x01		// Clear Feature
#define SR_RESERVED 0x02			// Reserved
#define SR_SET_FEATURE 0x03			// Set Feature
#define SR_SET_ADDRESS 0x05			// Set Address
#define SR_GET_DESCRIPTOR 0x06		// Get Descriptor
#define SR_SET_DESCRIPTOR 0x07		// Set Descriptor
#define SR_GET_CONFIGURATION 0x08	// Get Configuration
#define SR_SET_CONFIGURATION 0x09	// Set Configuration
#define SR_GET_INTERFACE 0x0a		// Get Interface
#define SR_SET_INTERFACE 0x0b		// Set Interface

// Get Descriptor codes
#define GD_DEVICE			0x01	// Get device descriptor: Device
#define GD_CONFIGURATION	0x02	// Get device descriptor: Configuration
#define GD_STRING			0x03	// Get device descriptor: String
#define GD_INTERFACE		0x04	// Get device descriptor: Interface
#define GD_ENDPOINT			0x05	// Get device descriptor: Endpoint
#define GD_DEVICEQUALIFIER	0x06
#define GD_OTHERSPEED		0x07
#define GD_INTERFACEPOWER	0x08
#define GD_ONTHEGO			0x09
#define GD_HID				0x21	// Get descriptor: HID
#define GD_REPORT			0x22	// Get descriptor: Report
#define CS_INTERFACE		0x24	// Get descriptor: Interface
#define CS_ENDPOINT			0x25	// Get descriptor: Endpoint

#define CDC_SENDENCAPSULATEDRESPONSE	0x00
#define CDC_GETENCAPSULATEDRESPONSE 	0x01
#define CDC_SETLINECODING				0x20
#define CDC_GETLINECODING				0x21
#define CDC_SETCONTROLLINESTATE			0x22

// SETUP packet offsets
#define bmRequestType 0
#define bRequest 1
#define wValueL 2
#define wValueH 3
#define wIndexL 4
#define wIndexH 5
#define wLengthL 6
#define wLengthH 7

// CDC bRequest values
#define SEND_ENCAPSULATED_COMMAND 0x00
#define GET_ENCAPSULATED_RESPONSE 0x01
#define SET_COMM_FEATURE 0x02
#define GET_COMM_FEATURE 0x03
#define CLEAR_COMM_FEATURE 0x04
#define SET_LINE_CODING 0x20
#define GET_LINE_CODING 0x21
#define SET_CONTROL_LINE_STATE 0x22
#define SEND_BREAK 0x23
