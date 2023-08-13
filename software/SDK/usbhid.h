#pragma once

#include "usbcommon.h"
#include "basesystem.h"

extern volatile uint32_t *IO_USBATRX;
extern volatile uint32_t *IO_USBASTA;

void MAX3421FlushOutputFIFO();
uint8_t MAX3421GetGPX();
uint8_t MAX3421ReadByte(uint8_t command);
void MAX3421WriteByte(uint8_t command, uint8_t data);
int MAX3421ReadBytes(uint8_t command, uint8_t length, uint8_t *buffer);
void MAX3421WriteBytes(uint8_t command, uint8_t length, uint8_t *buffer);
void MAX3421EnableIRQs();

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
    struct USBEndpointRecord *endpointInfo;
    uint8_t deviceClass;
};

void USBHostSetContext(struct SUSBContext *ctx);
struct SUSBContext *USBHostGetContext();
enum EBusState USBHostInit(uint32_t enableInterrupts);
enum EBusState USBBusProbe();
uint8_t USBGetDeviceDescriptor();
uint8_t USBGetHIDDescriptor(uint8_t _addr, uint8_t _ep);
uint8_t USBAttach(uint8_t *_paddr, uint8_t *_pep);
uint8_t USBDetach(uint8_t _addr);
uint8_t USBInTransfer(uint8_t _addr, uint8_t _ep, unsigned int _nbytes, char* _data, unsigned int _nak_limit);
uint8_t USBOutTransfer(uint8_t _addr, uint8_t _ep, unsigned int _nbytes, char* _data, unsigned int nak_limit);
uint8_t USBControlRequest(uint8_t _addr, uint8_t _ep, uint8_t _bmReqType, uint8_t _bRequest, uint8_t _wValLo, uint8_t _wValHi, unsigned int _wInd, unsigned int _nbytes, char* _dataptr, unsigned int _nak_limit);
uint8_t USBConfigHID(uint8_t _addr, uint8_t _ep);
void USBSetAddress(uint8_t _addr, uint8_t _ep);
uint8_t USBReadHIDData(uint8_t _addr, uint8_t _ep, uint8_t *_data);
uint8_t USBWriteHIDData(uint8_t _addr, uint8_t _ep, uint8_t *_data);
char HIDScanToASCII(const uint8_t _code, const uint8_t _uppercase);

// MAX3421E Registers - host mode
#define rRCVFIFO	1<<3
#define rSNDFIFO	2<<3
#define rSUDFIFO	4<<3
#define rRCVBC		6<<3
#define rSNDBC		7<<3
#define rUSBIRQ		13<<3
#define rUSBIEN		14<<3
#define rUSBCTL		15<<3
#define rCPUCTL		16<<3
#define rPINCTL		17<<3
#define rREVISION	18<<3
#define rIOPINS1	20<<3
#define rIOPINS2	21<<3
#define rGPINIRQ	22<<3
#define rGPINIEN	23<<3
#define rGPINPOL	24<<3
#define rHIRQ		25<<3
#define rHIEN		26<<3
#define rMODE		27<<3
#define rPERADDR	28<<3
#define rHCTL		29<<3
#define rHXFR		30<<3
#define rHRSL		31<<3

// USBIRQ
#define bmVBUSIRQ   0x40
#define bmNOVBUSIRQ 0x20
#define bmOSCOKIRQ  0x01

// USBIEN
#define bmVBUSIE    0x40
#define bmNOVBUSIE  0x20
#define bmOSCOKIE   0x01

// USBCTL
#define bmCHIPRES   0x20
#define bmPWRDOWN   0x10

// CPUCTL
#define bmPUSLEWID1 0x80
#define bmPULSEWID0 0x40
#define bmIE        0x01

// PINCTL
#define bmFDUPSPI   0x10
#define bmINTLEVEL  0x08
#define bmPOSINT    0x04
#define bmGPXB      0x02
#define bmGPXA      0x01

// GPX pin select
#define gpxOPERATE  0x00
#define gpxVBDETECT 0x01
#define gpxBUSACT   0x02
#define gpxSOF      0x03

// IOPINS1
#define bmGPOUT0    0x01
#define bmGPOUT1    0x02
#define bmGPOUT2    0x04
#define bmGPOUT3    0x08
#define bmGPIN0     0x10
#define bmGPIN1     0x20
#define bmGPIN2     0x40
#define bmGPIN3     0x80

// IOPINS2
#define bmGPOUT4    0x01
#define bmGPOUT5    0x02
#define bmGPOUT6    0x04
#define bmGPOUT7    0x08
#define bmGPIN4     0x10
#define bmGPIN5     0x20
#define bmGPIN6     0x40
#define bmGPIN7     0x80

// GPINIRQ
#define bmGPINIRQ0 0x01
#define bmGPINIRQ1 0x02
#define bmGPINIRQ2 0x04
#define bmGPINIRQ3 0x08
#define bmGPINIRQ4 0x10
#define bmGPINIRQ5 0x20
#define bmGPINIRQ6 0x40
#define bmGPINIRQ7 0x80

// GPINIEN
#define bmGPINIEN0 0x01
#define bmGPINIEN1 0x02
#define bmGPINIEN2 0x04
#define bmGPINIEN3 0x08
#define bmGPINIEN4 0x10
#define bmGPINIEN5 0x20
#define bmGPINIEN6 0x40
#define bmGPINIEN7 0x80

// GPINPOL
#define bmGPINPOL0 0x01
#define bmGPINPOL1 0x02
#define bmGPINPOL2 0x04
#define bmGPINPOL3 0x08
#define bmGPINPOL4 0x10
#define bmGPINPOL5 0x20
#define bmGPINPOL6 0x40
#define bmGPINPOL7 0x80

// HIRQ
#define bmBUSEVENTIRQ   0x01
#define bmRWUIRQ        0x02
#define bmRCVDAVIRQ     0x04
#define bmSNDBAVIRQ     0x08
#define bmSUSDNIRQ      0x10
#define bmCONDETIRQ     0x20
#define bmFRAMEIRQ      0x40
#define bmHXFRDNIRQ     0x80

// HIEN
#define bmBUSEVENTIE    0x01
#define bmRWUIE         0x02
#define bmRCVDAVIE      0x04
#define bmSNDBAVIE      0x08
#define bmSUSDNIE       0x10
#define bmCONDETIE      0x20
#define bmFRAMEIE       0x40
#define bmHXFRDNIE      0x80

// MODE
#define bmHOST          0x01
#define bmLOWSPEED      0x02
#define bmHUBPRE        0x04
#define bmSOFKAENAB     0x08
#define bmSEPIRQ        0x10
#define bmDELAYISO      0x20
#define bmDMPULLDN      0x40
#define bmDPPULLDN      0x80

// HCTL
#define bmBUSRST        0x01
#define bmFRMRST        0x02
#define bmSAMPLEBUS     0x04
#define bmSIGRSM        0x08
#define bmRCVTOG0       0x10
#define bmRCVTOG1       0x20
#define bmSNDTOG0       0x40
#define bmSNDTOG1       0x80

// HXFR
#define tokSETUP  0x10  // HS=0, ISO=0, OUTNIN=0, SETUP=1
#define tokIN     0x00  // HS=0, ISO=0, OUTNIN=0, SETUP=0
#define tokOUT    0x20  // HS=0, ISO=0, OUTNIN=1, SETUP=0
#define tokINHS   0x80  // HS=1, ISO=0, OUTNIN=0, SETUP=0
#define tokOUTHS  0xA0  // HS=1, ISO=0, OUTNIN=1, SETUP=0 
#define tokISOIN  0x40  // HS=0, ISO=1, OUTNIN=0, SETUP=0
#define tokISOOUT 0x60  // HS=0, ISO=1, OUTNIN=1, SETUP=0

// HRSL
#define bmRCVTOGRD  0x10
#define bmSNDTOGRD  0x20
#define bmKSTATUS   0x40
#define bmJSTATUS   0x80
#define bmSE0       0x00    // Disconnect
#define bmSE1       0xc0    // Illegal

// Host error codes
#define hrSUCCESS   0x00
#define hrBUSY      0x01
#define hrBADREQ    0x02
#define hrUNDEF     0x03
#define hrNAK       0x04
#define hrSTALL     0x05
#define hrTOGERR    0x06
#define hrWRONGPID  0x07
#define hrBADBC     0x08
#define hrPIDERR    0x09
#define hrPKTERR    0x0A
#define hrCRCERR    0x0B
#define hrKERR      0x0C
#define hrJERR      0x0D
#define hrTIMEOUT   0x0E
#define hrBABBLE    0x0F

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
#define USB_DESCRIPTOR_OTG              0x09    // bDescriptorType for an OTG Descriptor.

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
#define HID_RPT_PROTOCOL				0x01

// bInterfaceProtocol
#define HID_PROTOCOL_NONE				0x00
#define HID_PROTOCOL_KEYBOARD			0x01
#define HID_PROTOCOL_MOUSE				0x02

#define bmREQ_GET_DESCR     USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_DEVICE     //get descriptor request type
#define bmREQ_SET           USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_DEVICE     //set request type for all but 'set feature' and 'set interface'
#define bmREQ_CL_GET_INTF   USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE     //get interface request type
#define bmREQ_HIDOUT        USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE
#define bmREQ_HIDIN         USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE
#define bmREQ_HIDREPORT		USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_INTERFACE
