// Based on parts from https://github.com/nopnop2002/esp-idf-uart2bt

#include <fcntl.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/queue.h"
#include "cmd.h"

// EnCi: See schematic of tinysys board 2E for UART pin locations
#define PIN_TXD GPIO_NUM_22
#define PIN_RXD GPIO_NUM_23
#define PIN_RTS UART_PIN_NO_CHANGE
#define PIN_CTS UART_PIN_NO_CHANGE

// EnCi: bridge to UART port #1
#define UART_PORT_NUM 1

QueueHandle_t uart_queue;
QueueHandle_t ble_queue;

// From (PC to) ESP32 to FPGA
static void ble_to_uart1_task(void *arg)
{
	CMD_t cmdBuf;
	do
	{
		xQueueReceive(uart_queue, &cmdBuf, portMAX_DELAY);
		int txBytes = uart_write_bytes(UART_PORT_NUM, cmdBuf.payload, cmdBuf.length);
		if (txBytes != cmdBuf.length)
		{
			uart_flush(UART_PORT_NUM);
			ESP_LOGE(pcTaskGetName(NULL), "uart_write_bytes Fail. txBytes=%d cmdBuf.length=%d", txBytes, cmdBuf.length);
		}
	} while(1);
}

// From FPGA to ESP32 (to PC)
static void uart1_to_ble_task(void *arg)
{
	CMD_t cmdBuf;
	cmdBuf.BLE_event_id = BLE_UART_EVT;
	while (1)
	{
		cmdBuf.length = uart_read_bytes(UART_PORT_NUM, cmdBuf.payload, PAYLOAD_SIZE, portMAX_DELAY);
		if (cmdBuf.length > 0)
			xQueueSend(ble_queue, &cmdBuf, portMAX_DELAY);
	}
}

void ble_task(void * arg);

void app_main(void)
{
	// Initialize NVS.
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};

	ble_queue = xQueueCreate(10, sizeof(CMD_t));
	uart_queue = xQueueCreate(10, sizeof(CMD_t));

	ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, PAYLOAD_SIZE * 2, 0, 0, NULL, 0));
	ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, PIN_TXD, PIN_RXD, PIN_RTS, PIN_CTS));

	esp_task_wdt_deinit();

	xTaskCreate(ble_to_uart1_task, "ble_to_uart1", 1024, NULL, 2, NULL);
	xTaskCreate(uart1_to_ble_task, "uart1_to_ble", 1024, NULL, 2, NULL);
	xTaskCreate(ble_task, "BLE", 1024*4, NULL, 2, NULL);
}
