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
#include "esp_log.h"

// EnCi: See schematic of tinysys board 2E for UART pin locations
#define PIN_TXD GPIO_NUM_22
#define PIN_RXD GPIO_NUM_23
#define PIN_RTS UART_PIN_NO_CHANGE
#define PIN_CTS UART_PIN_NO_CHANGE

// EnCi: bridge to UART port #1
#define UART_PORT_NUM 1

#define BUF_SIZE 1024

static QueueHandle_t uart_queue;

static uint8_t *jtag_buffer = NULL;
static uint8_t *uart_buffer = NULL;

static const char *TAG = "uart_events";

// From (PC to) ESP32 to FPGA
static void jtag_to_uart1_task(void *arg)
{
	do
	{
		int len = usb_serial_jtag_read_bytes(jtag_buffer, BUF_SIZE, portMAX_DELAY);
		if (len > 0)
		{
			uart_write_bytes(UART_PORT_NUM, (const char *)jtag_buffer, len);
			uart_flush(UART_PORT_NUM);
		}
	} while(1);
}

// From FPGA to ESP32 (to PC)
static void uart1_to_jtag_task(void *arg)
{
	do
	{
		uart_event_t event;
		if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY))
		{
			switch (event.type)
			{
				case UART_DATA:
				{
					uart_read_bytes(UART_PORT_NUM, uart_buffer, event.size, portMAX_DELAY);
					usb_serial_jtag_write_bytes(uart_buffer, event.size, portMAX_DELAY);
					usb_serial_jtag_ll_txfifo_flush();
				}
				break;

				case UART_FIFO_OVF:
				{
					ESP_LOGI(TAG, "hw fifo overflow");
					uart_flush_input(UART_PORT_NUM);
					xQueueReset(uart_queue);
				}
				break;

				case UART_BUFFER_FULL:
				{
					ESP_LOGI(TAG, "ring buffer full");
					// If buffer full happened, you should consider increasing your buffer size
					// As an example, we directly flush the rx buffer here in order to read more data.
					uart_flush_input(UART_PORT_NUM);
					xQueueReset(uart_queue);
				}
				break;

				case UART_BREAK:
				{
					ESP_LOGI(TAG, "uart rx break");
				}
				break;

				case UART_PARITY_ERR:
				{
					ESP_LOGI(TAG, "uart parity error");
				}
				break;

				case UART_FRAME_ERR:
				{
					ESP_LOGI(TAG, "uart frame error");
				}
				break;

				default:
				{
					ESP_LOGI(TAG, "uart event type: %d", event.type);
				}
				break;
			}
		}

	} while(1);
}

void app_main(void)
{
	usb_serial_jtag_driver_config_t usb_serial_config = {.tx_buffer_size = BUF_SIZE, .rx_buffer_size = BUF_SIZE};
	ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_config));

	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};

	ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0));
	ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, PIN_TXD, PIN_RXD, PIN_RTS, PIN_CTS));

	jtag_buffer = (uint8_t *)malloc(BUF_SIZE);
	uart_buffer = (uint8_t *)malloc(BUF_SIZE);

	esp_task_wdt_deinit();

	xTaskCreate(jtag_to_uart1_task, "jtag_to_uart1", 1024, NULL, 5, NULL);
	xTaskCreate(uart1_to_jtag_task, "uart1_to_jtag", 1024, NULL, 7, NULL);
}
