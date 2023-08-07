#pragma once

#include <inttypes.h>

// Descriptors (always byte packed)

#pragma pack(push, 1)

struct USBCDCLineCoding
{
	uint32_t dwDTERate;
	uint8_t bCharFormat;
	uint8_t bParityType;
	uint8_t bDataBits;
};

struct USBCommonDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
};

struct USBDeviceDescriptor
{
	uint8_t bLength;			// Length of total structure -> sizeof(USBDeviceDescriptor)
	uint8_t bDescriptorType;	// Type of descriptor
	uint16_t bcdUSB;			// Version in BCD
	uint8_t bDeviceClass;		// Device class
	uint8_t bDeviceSubClass;	// Subclass in device group
	uint8_t bDeviceProtocol;	// Protocol code
	uint8_t bMaxPacketSizeEP0;	// Max packet size of endpoint0; 8,16,32 or 64
	uint16_t idVendor;			// Vendor ID
	uint16_t idProduct;			// Product code
	uint16_t bcdDevice;			// Device code
	uint8_t iManufacturer;		// Index of manufacturer string
	uint8_t iProduct;			// Index of product string
	uint8_t iSerialNumber;		// Index of serial number string
	uint8_t bNumConfigurations;	// Number of configuration descriptors
};

struct USBConfigurationDescriptor
{
	uint8_t bLength;			// Length of total structure -> sizeof(USBConfigurationDescriptor)
	uint8_t bDescriptorType;	// Type of descriptor
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t MaxPower;
};

struct USBEndpointDescriptor
{
	uint8_t bLength;			// Length of total structure -> sizeof(USBEndpointDescriptor)
	uint8_t bDescriptorType;	// Type of descriptor
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;			// Interval in ms if this is an interrupt endpoint
};

struct USBInterfaceDescriptor
{
	uint8_t bLength;			// Length of total structure -> sizeof(USBInterfaceDescriptor)
	uint8_t bDescriptorType;	// Type of descriptor
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
};

struct USBStringLanguageDescriptor
{
	uint8_t bLength;			// Length of total structure -> sizeof(USBStringLanguageDescriptor)
	uint8_t bDescriptorType;	// Type of descriptor
	uint16_t wLanguage;
};

struct USBStringDescriptor
{
	uint8_t bLength;			// Length of total structure -> sizeof(USBCommonDescriptor) + length of string
	uint8_t bDescriptorType;	// Type of descriptor
#ifdef __cplusplus
	char16_t bString[64];		// UTF16 characters
#else
	char bString[128];
#endif
};

struct SUSBContext
{
	struct USBDeviceDescriptor device;
	struct USBConfigurationDescriptor config;
	struct USBInterfaceDescriptor data;
	struct USBEndpointDescriptor input;
	struct USBEndpointDescriptor output;
	struct USBStringDescriptor strings[6];
};

#pragma pack(pop)
