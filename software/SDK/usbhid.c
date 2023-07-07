#include "usbhid.h"
#include "basesystem.h"
#include "uart.h"
#include <string.h>

volatile uint32_t *IO_USBATRX = (volatile uint32_t* )DEVICE_USBA; // Receive fifo
volatile uint32_t *IO_USBASTA = (volatile uint32_t* )(DEVICE_USBA+4); // Output FIFO state
