#pragma once

#include "usbcommon.h"
#include "basesystem.h"

enum EBusState
{
	BUSUNKNOWN,
	SE0,
	SE1,
	FSHOST,
	LSHOST
};

enum EUSBDeviceState
{
	DEVS_UNKNOWN,
	DEVS_DETACHED,
	DEVS_ATTACHED,
	DEVS_ADDRESSING,
	DEVS_RUNNING,
	DEVS_ERROR,
	DEVS_HALT
};

// SETUP packet offsets
#define bmRequestType 0
#define bRequest 1
#define wValueL 2
#define wValueH 3
#define wIndexL 4
#define wIndexH 5
#define wLengthL 6
#define wLengthH 7

struct USBEndpointRecord
{
    uint8_t epAddr;
    uint8_t epTransferType;
    unsigned int maxPacketSize;
    uint8_t pollInterval;
    uint8_t sendToggle;
    uint8_t receiveToggle;
};

struct USBDeviceRecord
{
    struct USBEndpointRecord endpointInfo[3];
    uint8_t deviceClass;
	uint8_t connected;
};

void USBHostSetContext(struct SUSBHostContext *ctx);
struct SUSBHostContext *USBHostGetContext();
enum EBusState USBHostInit(uint32_t enableInterrupts);
enum EBusState USBBusProbe();
uint8_t USBGetDeviceDescriptor(uint8_t _addr, uint8_t _ep, uint8_t *_hidclass);
uint8_t USBGetHIDDescriptor(uint8_t _addr, uint8_t _ep, uint8_t *_protocol);
uint8_t USBAttach(uint8_t *_paddr, uint8_t *_pep);
uint8_t USBDetach(uint8_t _addr);
uint8_t USBControlData(uint8_t _addr, uint8_t _ep, unsigned int _nbytes, char* _dataptr, uint8_t _direction, unsigned int _nak_limit);
uint8_t USBControlRequest(uint8_t _addr, uint8_t _ep, uint8_t _bmReqType, uint8_t _bRequest, uint8_t _wValLo, uint8_t _wValHi, unsigned int _wInd, unsigned int _nbytes, char* _dataptr, unsigned int _nak_limit);
uint8_t USBConfigHID(uint8_t _hidClass, uint8_t _addr, uint8_t _ep);
void USBSetAddress(uint8_t _addr, uint8_t _ep);
uint8_t USBReadHIDData(uint8_t _addr, uint8_t _ep,uint8_t _dataLen, uint8_t *_data, uint8_t _reportIndex, uint8_t _reportType, uint8_t _hidClass);
uint8_t USBWriteHIDData(uint8_t _addr, uint8_t _ep, uint8_t *_data);
char HIDScanToASCII(const uint8_t _code, const uint8_t _uppercase);
void USBErrorString(uint8_t rcode);

// Full/low speed setup
#define MODE_FS_HOST    (bmDPPULLDN|bmDMPULLDN|bmHOST|bmSOFKAENAB)
#define MODE_LS_HOST    (bmDPPULLDN|bmDMPULLDN|bmHOST|bmLOWSPEED|bmSOFKAENAB)

#define USB_REQUEST_GET_STATUS                  0       // Standard Device Request - GET STATUS
#define USB_REQUEST_CLEAR_FEATURE               1       // Standard Device Request - CLEAR FEATURE
#define USB_REQUEST_SET_FEATURE                 3       // Standard Device Request - SET FEATURE
#define USB_REQUEST_SET_ADDRESS                 5       // Standard Device Request - SET ADDRESS
#define USB_REQUEST_GET_DESCRIPTOR              6       // Standard Device Request - GET DESCRIPTOR
#define USB_REQUEST_SET_DESCRIPTOR              7       // Standard Device Request - SET DESCRIPTOR
#define USB_REQUEST_GET_CONFIGURATION           8       // Standard Device Request - GET CONFIGURATION
#define USB_REQUEST_SET_CONFIGURATION           9       // Standard Device Request - SET CONFIGURATION
#define USB_REQUEST_GET_INTERFACE               10      // Standard Device Request - GET INTERFACE
#define USB_REQUEST_SET_INTERFACE               11      // Standard Device Request - SET INTERFACE
#define USB_REQUEST_SYNCH_FRAME                 12      // Standard Device Request - SYNCH FRAME

#define USB_FEATURE_ENDPOINT_HALT               0       // CLEAR/SET FEATURE - Endpoint Halt
#define USB_FEATURE_DEVICE_REMOTE_WAKEUP        1       // CLEAR/SET FEATURE - Device remote wake-up
#define USB_FEATURE_TEST_MODE                   2       // CLEAR/SET FEATURE - Test mode
#define USB_SETUP_HOST_TO_DEVICE                0x00    // Device Request bmRequestType transfer direction - host to device transfer
#define USB_SETUP_DEVICE_TO_HOST                0x80    // Device Request bmRequestType transfer direction - device to host transfer
#define USB_SETUP_TYPE_STANDARD                 0x00    // Device Request bmRequestType type - standard
#define USB_SETUP_TYPE_CLASS                    0x20    // Device Request bmRequestType type - class
#define USB_SETUP_TYPE_VENDOR                   0x40    // Device Request bmRequestType type - vendor
#define USB_SETUP_RECIPIENT_DEVICE              0x00    // Device Request bmRequestType recipient - device
#define USB_SETUP_RECIPIENT_INTERFACE           0x01    // Device Request bmRequestType recipient - interface
#define USB_SETUP_RECIPIENT_ENDPOINT            0x02    // Device Request bmRequestType recipient - endpoint
#define USB_SETUP_RECIPIENT_OTHER               0x03    // Device Request bmRequestType recipient - other

#define USB_DESCRIPTOR_DEVICE           0x01    // bDescriptorType for a Device Descriptor.
#define USB_DESCRIPTOR_CONFIGURATION    0x02    // bDescriptorType for a Configuration Descriptor.
#define USB_DESCRIPTOR_STRING           0x03    // bDescriptorType for a String Descriptor.
#define USB_DESCRIPTOR_INTERFACE        0x04    // bDescriptorType for an Interface Descriptor.
#define USB_DESCRIPTOR_ENDPOINT         0x05    // bDescriptorType for an Endpoint Descriptor.
#define USB_DESCRIPTOR_DEVICE_QUALIFIER 0x06    // bDescriptorType for a Device Qualifier.
#define USB_DESCRIPTOR_OTHER_SPEED      0x07    // bDescriptorType for a Other Speed Configuration.
#define USB_DESCRIPTOR_INTERFACE_POWER  0x08    // bDescriptorType for Interface Power.
#define USB_DESCRIPTOR_OTG              0x09    // bDescriptorType for an On-The-Go Descriptor.

#define HID_DESCRIPTOR_HID				0x21
#define HID_DESCRIPTOR_REPORT			0x22
#define HID_DESRIPTOR_PHY				0x23

#define HID_REQUEST_GET_REPORT			0x01
#define HID_REQUEST_GET_IDLE			0x02
#define HID_REQUEST_GET_PROTOCOL		0x03
#define HID_REQUEST_SET_REPORT			0x09
#define HID_REQUEST_SET_IDLE			0x0A
#define HID_REQUEST_SET_PROTOCOL		0x0B

#define USB_HID_BOOT_PROTOCOL			0x00
#define HID_REPORT_PROTOCOL				0x01

// bInterfaceProtocol
#define HID_PROTOCOL_NONE				0x00
#define HID_PROTOCOL_KEYBOARD			0x01
#define HID_PROTOCOL_MOUSE				0x02

// Report Types
#define HID_REPORTTYPE_INPUT			0x01
#define HID_REPORTTYPE_FEATURE			0x03

#define bmREQ_GET_DESCR     USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_DEVICE     //get descriptor request type
#define bmREQ_SET           USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_DEVICE     //set request type for all but 'set feature' and 'set interface'
#define bmREQ_CL_GET_INTF   USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE     //get interface request type
#define bmREQ_HIDOUT        USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE
#define bmREQ_HIDIN         USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE
#define bmREQ_HIDREPORT		USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_INTERFACE
#define bmREQ_CLEAR_FEATURE USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_ENDPOINT