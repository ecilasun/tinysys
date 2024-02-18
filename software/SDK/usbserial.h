#pragma once

#include "usbcommon.h"

// Descriptor types

#define USBDesc_Device						0x01
#define USBDesc_Configuration				0x02
#define USBDesc_String						0x03
#define USBDesc_Interface					0x04
#define USBDesc_Endpoint					0x05
#define USBDesc_DeviceQualifier				0x06
#define USBDesc_OtherSpeedConfiguration		0x07
#define USBDesc_InterfacePower				0x08

void USBSerialSetContext(struct SUSBSerialContext *ctx);
struct SUSBSerialContext *USBSerialGetContext();
int USBSerialInit(uint32_t enableInterrupts);

uint32_t USBSerialWriteRawBytes(void *outbytes, uint32_t count);
int USBSerialWriteN(const char *outstring, uint32_t count);
int USBSerialWrite(const char *outstring);
int USBSerialWriteHexByte(const uint8_t i);
int USBSerialWriteHex(const uint32_t i);
int USBSerialWriteDecimal(const int32_t i);

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
