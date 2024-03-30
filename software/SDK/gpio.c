#include "basesystem.h"
#include "gpio.h"
#include <stdlib.h>

volatile uint32_t *GPIO_DATA = (volatile uint32_t* ) DEVICE_GPIO;
volatile uint32_t *GPIO_INPUTENABLE = (volatile uint32_t* ) (DEVICE_GPIO+4);
volatile uint32_t *GPIO_OUTPUTENABLE = (volatile uint32_t* ) (DEVICE_GPIO+8);
