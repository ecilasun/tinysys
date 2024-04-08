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
#define PIN_TXD GPIO_NUM_22
#define PIN_RXD GPIO_NUM_23
#define PIN_RTS UART_PIN_NO_CHANGE
#define PIN_CTS UART_PIN_NO_CHANGE

// EnCi: bridge to port #1
#define UART_PORT_NUM 1
#define UART_BAUD_RATE 115200
#define TASK_STACK_SIZE 2048

#define BUF_SIZE 1024

static void jtag_to_uart1_task(void *arg) {

  usb_serial_jtag_driver_config_t usb_serial_config = {.tx_buffer_size = BUF_SIZE, .rx_buffer_size = BUF_SIZE};
  ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_config));

  uart_config_t uart_config = {
      .baud_rate = UART_BAUD_RATE,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  int intr_alloc_flags = 0;

  ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, PIN_TXD, PIN_RXD, PIN_RTS, PIN_CTS));

  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

  do{

    int len = usb_serial_jtag_read_bytes(data, BUF_SIZE, 1 / portTICK_PERIOD_MS);
    if (len > 0)
	{
      uart_write_bytes(UART_PORT_NUM, (const char *)data, len);
      uart_flush(UART_PORT_NUM);
    }

    len = uart_read_bytes(UART_PORT_NUM, data, (BUF_SIZE), 1 / portTICK_PERIOD_MS);
    if (len > 0)
	{
      usb_serial_jtag_write_bytes(data, len, 1 / portTICK_PERIOD_MS);
      usb_serial_jtag_ll_txfifo_flush();
    }
  } while(1);

  ESP_ERROR_CHECK(uart_driver_delete(UART_PORT_NUM));
}

void app_main(void) {

  esp_task_wdt_deinit();
  xTaskCreate(jtag_to_uart1_task, "jtag_to_uart1", TASK_STACK_SIZE, NULL, 2 | portPRIVILEGE_BIT, NULL);
}
