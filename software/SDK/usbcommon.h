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

struct USBHIDDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint32_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bDescrType;
	uint32_t wDescriptorLength;
};

struct SUSBSerialContext
{
	struct USBDeviceDescriptor device;
	struct USBConfigurationDescriptor config;
	struct USBInterfaceDescriptor data;
	struct USBEndpointDescriptor input;
	struct USBEndpointDescriptor output;
	struct USBStringDescriptor strings[6];
};

struct SUSBHostContext
{
	uint8_t m_TBD;
};

#define USBDESCTYPE_UNKNOWN						0x00
#define USBDESCTYPE_DEVICE						0x01
#define USBDESCTYPE_CONFIGURATION				0x02
#define USBDESCTYPE_STRING						0x03
#define USBDESCTYPE_INTERFACE					0x04
#define USBDESCTYPE_ENDPOINT					0x05
#define USBDESCTYPE_DEVICEQUALIFIER				0x06
#define USBDESCTYPE_OTHERSPEEDCFG				0x07
#define USBDESCTYPE_INTERFACEPOWER				0x08
#define USBDESCTYPE_OTG							0x09
#define USBDESCTYPE_DEBUG						0x0A
#define USBDESCTYPE_INTERFACEASSOC				0x0B
#define USBDESCTYPE_SECURITY					0x0C
#define USBDESCTYPE_KEY							0x0D
#define USBDESCTYPE_ENCRYPTTYPE					0x0E
#define USBDESCTYPE_BINARYDEVOBJSTORE			0x0F
#define USBDESCTYPE_DEVICECAPABILITY			0x10
#define USBDESCTYPE_WIRELESSEPCOMPANION			0x11
#define USBDESCTYPE_HIDSPECIFIC					0x21
#define USBDESCTYPE_SUPERSPEEDEPCOMPANION		0x30
#define USBDESCTYPE_SUPERSPEEDISOEPCOMPANION	0x31

// Device classes

#define USBClass_Device									0x00
#define USBClass_Audio									0x01
#define USBClass_CDCControl								0x02
#define USBClass_HID									0x03
#define USBClass_Physical								0x05
#define USBClass_Image									0x06
#define USBClass_Printer								0x07
#define USBClass_MassStorage							0x08
#define USBClass_Hub									0x09
#define USBClass_CDCData								0x0A
#define USBClass_SmartCard								0x0B
#define USBClass_ContentSecurity						0x0D
#define USBClass_Video									0x0E
#define USBClass_PersonalHealthcare						0x0F
#define USBClass_AudioVideoDevices						0x10
#define USBClass_BillboardDeviceClass					0x11
#define USBClass_USBTypeCBridgeClass					0x12
#define USBClass_USBBulkDisplayProtocolDeviceClass		0x13
#define USBClass_I3CDeviceClass							0x3C
#define USBClass_DiagnosticDevice						0xDC
#define USBClass_WirelessController						0xE0
#define USBClass_Miscellaneous							0xEF
#define USBClass_ApplicationSpecific					0xFE
#define USBClass_VendorSpecific							0xFF

#pragma pack(pop)
