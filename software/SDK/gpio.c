#include "basesystem.h"
#include "gpio.h"
#include <stdlib.h>

volatile uint32_t *GPIO_DATA = (volatile uint32_t* ) (DEVICE_GPIO+0x0);
volatile uint32_t *GPIO_INPUTENABLE = (volatile uint32_t* ) (DEVICE_GPIO+0x4);
volatile uint32_t *GPIO_OUTPUTENABLE = (volatile uint32_t* ) (DEVICE_GPIO+0x8);
volatile uint32_t *GPIO_FIFOHASDATA = (volatile uint32_t* ) (DEVICE_GPIO+0xC);
