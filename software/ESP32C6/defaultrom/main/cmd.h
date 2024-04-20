#include "esp_gatts_api.h"

typedef enum {BLE_CONNECT_EVT, BLE_AUTH_EVT, BLE_WRITE_EVT, BLE_DISCONNECT_EVT, BLE_UART_EVT} COMMAND;

#define PAYLOAD_SIZE 128

typedef struct {
	uint16_t BLE_conn_id;
	esp_gatt_if_t BLE_gatts_if;
	uint16_t BLE_event_id;
	size_t length;
	uint8_t payload[PAYLOAD_SIZE];
	TaskHandle_t taskHandle;
} CMD_t;
