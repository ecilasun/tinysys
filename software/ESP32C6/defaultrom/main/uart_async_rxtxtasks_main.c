// Please see https://github.com/aliengreen/esp32_uart_bridge/tree/main for the original source code

/*
 *    esp32_uart_bridge.c
 *
 *     ________  ________  ___  ________  ________  _______
 *    |\   __  \|\   __  \|\  \|\   ___ \|\   ____\|\  ___ \
 *    \ \  \|\ /\ \  \|\  \ \  \ \  \_|\ \ \  \___|\ \   __/|
 *     \ \   __  \ \   _  _\ \  \ \  \ \\ \ \  \  __\ \  \_|/__
 *      \ \  \|\  \ \  \\  \\ \  \ \  \_\\ \ \  \|\  \ \  \_|\ \
 *       \ \_______\ \__\\ _\\ \__\ \_______\ \_______\ \_______\
 *        \|_______|\|__|\|__|\|__|\|_______|\|_______|\|_______|
 *
 *
 *    Copyright (c) 2023 Alien Green LLC
 *
 *    This file is part of UnitVM.
 *
 *    UnitVM is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    UnitVM is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with UnitVM.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    ASCII font see http://patorjk.com/software/taag/#p=display&f=3D-ASCII
 */

#include <fcntl.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/usb_serial_jtag.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/usb_serial_jtag_ll.h"
#include "sdkconfig.h"

// EnCi: See schematic of tinysys board 2E for UART pin locations
#define UBRIDGE_PIN_TXD GPIO_NUM_22
#define UBRIDGE_PIN_RXD GPIO_NUM_23
#define UBRIDGE_PIN_RTS UART_PIN_NO_CHANGE
#define UBRIDGE_PIN_CTS UART_PIN_NO_CHANGE

// EnCi: bridge to port #1
#define UBRIDGE_UART_PORT_NUM 1
#define UBRIDGE_UART_BAUD_RATE 115200
#define UBRIDGE_TASK_STACK_SIZE 2048

#define BUF_SIZE 1024

/* ----------------------------------------------------------- */

static void bridge_task(void *arg) {

  /* Configure USB-CDC */
  usb_serial_jtag_driver_config_t usb_serial_config = {.tx_buffer_size = 128,
                                                       .rx_buffer_size = 128};

  ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_config));

  /* Configure parameters of an UART driver,
   * communication pins and install the driver */
  uart_config_t uart_config = {
      .baud_rate = UBRIDGE_UART_BAUD_RATE,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
  intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

  // gpio_reset_pin(UBRIDGE_PIN_TXD);
  // gpio_reset_pin(UBRIDGE_PIN_RXD);
  // gpio_set_direction(UBRIDGE_PIN_TXD, GPIO_MODE_INPUT);
  // gpio_set_direction(UBRIDGE_PIN_RXD, GPIO_MODE_INPUT);

  ESP_ERROR_CHECK(uart_driver_install(UBRIDGE_UART_PORT_NUM, BUF_SIZE * 2, 0, 0,
                                      NULL, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(UBRIDGE_UART_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UBRIDGE_UART_PORT_NUM, UBRIDGE_PIN_TXD,
                               UBRIDGE_PIN_RXD, UBRIDGE_PIN_RTS,
                               UBRIDGE_PIN_CTS));

  /* Configure a bridge buffer for the incoming data */
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

  while (true) {

    int len = usb_serial_jtag_read_bytes(data, BUF_SIZE, 50 / portTICK_PERIOD_MS);
    if (len > 0) {
      uart_write_bytes(UBRIDGE_UART_PORT_NUM, (const char *)data, len);
      uart_flush(UBRIDGE_UART_PORT_NUM);
    }

    len = uart_read_bytes(UBRIDGE_UART_PORT_NUM, data, (BUF_SIZE), 50 / portTICK_PERIOD_MS);
    if (len > 0) {
      usb_serial_jtag_write_bytes(data, len, 500 / portTICK_PERIOD_MS);
      usb_serial_jtag_ll_txfifo_flush();
    }
  }

  /* Never reached */
  ESP_ERROR_CHECK(uart_driver_delete(UBRIDGE_UART_PORT_NUM));
}

/* ----------------------------------------------------------- */

void app_main(void) {

  xTaskCreate(bridge_task, "uart_bridge_task", UBRIDGE_TASK_STACK_SIZE, NULL,
              2 | portPRIVILEGE_BIT, NULL);

#ifdef CONFIG_ESP_TASK_WDT_EN
  /*
   * In case if 'Enable Task Watchdog Timer' is enabled in → Component
   * config → ESP System Settings. We must deinit a WDT on main task to avoid
   * an unexpected system restart.
   */
  esp_task_wdt_deinit();
#endif
}

/* ----------------------------------------------------------- */