#pragma once

#include <inttypes.h>

extern volatile uint32_t *IO_USBCTRX;
extern volatile uint32_t *IO_USBCSTA;

void MAX3420FlushOutputFIFO();
uint8_t MAX3420GetGPX();
uint8_t MAX3420OutFifoEmpty();
uint8_t MAX3420ReceiveFifoEmpty();
void MAX3420CtlReset();
uint8_t MAX3420ReadByte(uint8_t command);
void MAX3420WriteByte(uint8_t command, uint8_t data);
int MAX3420ReadBytes(uint8_t command, uint8_t length, uint8_t *buffer);
void MAX3420WriteBytes(uint8_t command, uint8_t length, uint8_t *buffer);
void MAX3420EnableIRQs();

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
