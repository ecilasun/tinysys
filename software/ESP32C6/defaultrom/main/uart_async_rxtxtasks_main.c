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
#include "esp_vfs_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/queue.h"
#include "cmd.h"

// EnCi: See schematic of tinysys board 2E for UART pin locations
#define PIN_TXD GPIO_NUM_22
#define PIN_RXD GPIO_NUM_23
#define PIN_RTS UART_PIN_NO_CHANGE
#define PIN_CTS UART_PIN_NO_CHANGE

// EnCi: bridge to UART port
#define UART_PORT_NUM 1

QueueHandle_t uart_send_queue;
QueueHandle_t jtag_send_queue;

static void jtag_rx_task(void *arg)
{
	CMD_t cmdBuf;
	do
	{
		cmdBuf.length = usb_serial_jtag_read_bytes(cmdBuf.payload, PAYLOAD_SIZE, portMAX_DELAY);
		if (cmdBuf.length > 0)
			/*BaseType_t err =*/ xQueueSendFromISR(uart_send_queue, &cmdBuf, NULL);
	} while(1);
}

static void jtag_tx_task(void *arg)
{
	CMD_t cmdBuf;
	do
	{
		xQueueReceive(jtag_send_queue, &cmdBuf, portMAX_DELAY);
		int txBytes = usb_serial_jtag_write_bytes(cmdBuf.payload, cmdBuf.length, portMAX_DELAY);
		if (txBytes)
			usb_serial_jtag_ll_txfifo_flush();
	} while(1);
}

static void uart_tx_task(void *arg)
{
	CMD_t cmdBuf;
	do
	{
		xQueueReceive(uart_send_queue, &cmdBuf, portMAX_DELAY);
		int txBytes = uart_write_bytes(UART_PORT_NUM, cmdBuf.payload, cmdBuf.length);
		if (txBytes != cmdBuf.length)
			uart_flush(UART_PORT_NUM);
	} while(1);
}

static void uart_rx_task(void *arg)
{
	CMD_t cmdBuf;
	while (1)
	{
		cmdBuf.length = uart_read_bytes(UART_PORT_NUM, cmdBuf.payload, PAYLOAD_SIZE, portMAX_DELAY);
		if (cmdBuf.length > 0)
		{
			// Queue to JTAG/USB
			xQueueSendFromISR(jtag_send_queue, &cmdBuf, NULL);
		}
	}
}

void app_main(void)
{
	// Initialize NVS.
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

	setvbuf(stdin, NULL, _IONBF, 0);

	// Enable blocking mode on stdin and stdout
	fcntl(fileno(stdout), F_SETFL, 0);
	fcntl(fileno(stdin), F_SETFL, 0);

	usb_serial_jtag_driver_config_t usb_serial_config =  USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_config));

	// Comm setup between tinysys Soc and the ESP32
	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};

	uart_send_queue = xQueueCreate(256, sizeof(CMD_t));
	jtag_send_queue = xQueueCreate(256, sizeof(CMD_t));

	ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, 2*UART_HW_FIFO_LEN(UART_PORT_NUM), 2*UART_HW_FIFO_LEN(UART_PORT_NUM), 0, NULL, ESP_INTR_FLAG_IRAM));
	ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, PIN_TXD, PIN_RXD, PIN_RTS, PIN_CTS));
	ESP_ERROR_CHECK(uart_disable_pattern_det_intr(UART_PORT_NUM));

	esp_task_wdt_deinit();

	xTaskCreate(jtag_rx_task, "jtag_rx", 1024, NULL, 2, NULL);
	xTaskCreate(jtag_tx_task, "jtag_tx", 1024, NULL, 3, NULL);
	xTaskCreate(uart_rx_task, "uart_rx", 1024, NULL, 2, NULL);
	xTaskCreate(uart_tx_task, "uart_tx", 1024, NULL, 3, NULL);
	ESP_LOGI(pcTaskGetName(0), "TinySys ready");
}
