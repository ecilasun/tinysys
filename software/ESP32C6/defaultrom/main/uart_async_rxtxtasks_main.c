// Based on ESR32 UART event example
// Part of tinysys SoC

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "hal/usb_serial_jtag_ll.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/usb_serial_jtag.h"
#include "esp_log.h"
#include "esp_vfs_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"

// SDCard:
// https://github.com/espressif/esp-idf/tree/v5.2.3/examples/storage/sd_card/sdspi
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/sdspi_host.html

#define PIN_TXD GPIO_NUM_22
#define PIN_RXD GPIO_NUM_23
#define PIN_RTS UART_PIN_NO_CHANGE
#define PIN_CTS UART_PIN_NO_CHANGE

// Send reset request to tinysys CPUs
#define PIN_REBOOT GPIO_NUM_18

static const char *TAG = "tinysys";

// external port going to tinysys
#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM	(3)

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
static QueueHandle_t uart0_queue;

static void jtag_task(void *arg)
{
	uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
	do
	{
		int datasize = usb_serial_jtag_read_bytes(dtmp, RD_BUF_SIZE, portMAX_DELAY);
		if (datasize > 0)
		{
			if (dtmp[0] == '~')
			{
				ESP_LOGI(TAG, "restarting tinysys CPUs");
				gpio_set_level(PIN_REBOOT, 0);
				vTaskDelay(100 / portTICK_PERIOD_MS);
				gpio_set_level(PIN_REBOOT, 1);
			}
			else // Pass through
				uart_write_bytes(EX_UART_NUM, (const char*) dtmp, datasize);
		}
	} while(1);
}

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    //size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for (;;) {
        //Waiting for UART event.
        if (xQueueReceive(uart0_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            //ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
            switch (event.type) {
            //Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
            other types of events. If we take too much time on data event, the queue might
            be full.*/
            case UART_DATA:
                //ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
				if (event.size > 0)
				{
					// From tinysys to console
					/*int txBytes =*/ usb_serial_jtag_write_bytes(dtmp, event.size, portMAX_DELAY);
				}
				//for(int i = 0; i < event.size; i++)
				//	esp_rom_printf("%c", dtmp[i]);
                //uart_write_bytes(EX_UART_NUM, (const char*) dtmp, event.size);
                break;
            //Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                //ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart0_queue);
                break;
            //Event of UART ring buffer full
            case UART_BUFFER_FULL:
                //ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider increasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart0_queue);
                break;
            //Event of UART RX break detected
            case UART_BREAK:
                //ESP_LOGI(TAG, "uart rx break");
                break;
            //Event of UART parity check error
            case UART_PARITY_ERR:
                //ESP_LOGI(TAG, "uart parity error");
                break;
            //Event of UART frame error
            case UART_FRAME_ERR:
                //ESP_LOGI(TAG, "uart frame error");
                break;
            //UART_PATTERN_DET
            case UART_PATTERN_DET:
                /*uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
                int pos = uart_pattern_pop_pos(EX_UART_NUM);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1) {
                    // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                    // record the position. We should set a larger queue size.
                    // As an example, we directly flush the rx buffer here.
                    uart_flush_input(EX_UART_NUM);
                } else {
                    uart_read_bytes(EX_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", dtmp);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }*/
                break;
            //Others
            default:
                //ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_NONE);

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

	gpio_config(& (gpio_config_t) {
		.pin_bit_mask = 1ULL << PIN_REBOOT,
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	});

	usb_serial_jtag_driver_config_t usb_serial_config =  {
			.tx_buffer_size = 1024,
			.rx_buffer_size = 1024 };
	ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_config));

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    //Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0);
    uart_param_config(EX_UART_NUM, &uart_config);

    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(EX_UART_NUM, PIN_TXD, PIN_RXD, PIN_RTS, PIN_CTS);

    //Set uart pattern detect function.
    //uart_enable_pattern_det_baud_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 9, 0, 0);
    //Reset the pattern queue length to record at most 20 pattern positions.
    //uart_pattern_queue_reset(EX_UART_NUM, 20);

	esp_task_wdt_deinit();

	esp_log_level_set(TAG, ESP_LOG_INFO);
	ESP_LOGI(TAG, "starting up TinySys terminal\n");

    //Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "tinysys_uart_task", 2048, NULL, 12, NULL);
	xTaskCreate(jtag_task, "tinysys_jtag_task", 2048, NULL, 12, NULL);

	// Let the tinysys CPUs know we are ready
	gpio_set_level(PIN_REBOOT, 1);
}