#pragma once

#include "usbcommon.h"

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

void USBHostSetContext(struct SUSBContext *ctx);
struct SUSBContext *USBHostGetContext();
void USBHostInit(uint32_t enableInterrupts);
enum EBusState USBBusProbe();

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
