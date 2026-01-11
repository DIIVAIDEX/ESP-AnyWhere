/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
//#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
//#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "usbh_modem_board.h"
#include "iot_usbh_modem.h"
#include "driver/gpio.h"
#include <esp_modem_dce_common_commands.h> 

#ifdef CONFIG_EXAMPLE_PING_NETWORK
#include "ping/ping_sock.h"
#endif

#define MODEM_PWRKEY	GPIO_NUM_33
static const char *TAG = "4G USB";
extern esp_modem_dce_t *s_dce;

#define BSP_USB_MODE_SEL      (GPIO_NUM_18) // Select Host (high level) or Device (low level, default) mode

modemData_t modemData;

static void ModemPwrInit()
{
	gpio_config_t gpio_conf = {.mode = GPIO_MODE_OUTPUT,
							   .intr_type = GPIO_INTR_DISABLE,
							   .pull_down_en = GPIO_PULLDOWN_DISABLE,
							   .pull_up_en = GPIO_PULLUP_DISABLE,
							   .pin_bit_mask = 1ULL<<MODEM_PWRKEY};
	
	gpio_config(&gpio_conf);
	
	gpio_set_level(MODEM_PWRKEY, 0);
	ESP_LOGI(TAG, "MODEM_PWRKEY LOW");
	vTaskDelay(pdMS_TO_TICKS(1000));
	gpio_set_level(MODEM_PWRKEY,1);
	ESP_LOGI(TAG, "MODEM_PWRKEY HIGH");
//	vTaskDelay(pdMS_TO_TICKS(10000));
}

static void on_modem_event(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == MODEM_BOARD_EVENT) {
        if (event_id == MODEM_EVENT_SIMCARD_DISCONN) {
            ESP_LOGW(TAG, "Modem Board Event: SIM Card disconnected");
        } else if (event_id == MODEM_EVENT_SIMCARD_CONN) {
            ESP_LOGI(TAG, "Modem Board Event: SIM Card Connected");
        } else if (event_id == MODEM_EVENT_DTE_DISCONN) {
			ModemPwrInit();
            ESP_LOGW(TAG, "Modem Board Event: USB disconnected");
        } else if (event_id == MODEM_EVENT_DTE_CONN) {
            ESP_LOGI(TAG, "Modem Board Event: USB connected");
        } else if (event_id == MODEM_EVENT_DTE_RESTART) {
            ESP_LOGW(TAG, "Modem Board Event: Hardware restart");
        } else if (event_id == MODEM_EVENT_DTE_RESTART_DONE) {
            ESP_LOGI(TAG, "Modem Board Event: Hardware restart done");
        } else if (event_id == MODEM_EVENT_NET_CONN) {
            ESP_LOGI(TAG, "Modem Board Event: Network connected");
        } else if (event_id == MODEM_EVENT_NET_DISCONN) {
            ESP_LOGW(TAG, "Modem Board Event: Network disconnected");
        } else if (event_id == MODEM_EVENT_WIFI_STA_CONN) {
            ESP_LOGI(TAG, "Modem Board Event: Station connected");
        } else if (event_id == MODEM_EVENT_WIFI_STA_DISCONN) {
            ESP_LOGW(TAG, "Modem Board Event: All stations disconnected");
        }
    }
}

modemData_t *ModemUSB_Init(void)
{
	ModemPwrInit();

    /* Waiting for modem powerup */
    ESP_LOGI(TAG, "====================================");
    ESP_LOGI(TAG, "     ESP 4G Cat.1 Wi-Fi Router");
    ESP_LOGI(TAG, "====================================");

    /* Initialize modem board. Dial-up internet */
    modem_config_t modem_config = MODEM_DEFAULT_CONFIG();
    
    modem_config.handler = on_modem_event;
    modem_board_init(&modem_config);
    
    size_t operatorLen = OPERATOR_NAME_LEN;
    esp_modem_dce_get_operator_name(s_dce, &operatorLen, modemData.operatorName);
    
    return &modemData;
}

modemData_t *ModemUSB_GetSignalQuality(void)
{
	esp_modem_dce_csq_ctx_t signalQuality;
	esp_modem_dce_get_signal_quality(s_dce, NULL, &signalQuality);
	
	modemData.rssi = signalQuality.rssi;
	modemData.ber = signalQuality.ber;
	
    return &modemData;
}

void ModemUSB_SetNetMode(char netMode[2])
{
	if(esp_modem_dce_set_NetMode(s_dce, (void*)netMode, NULL) != ESP_OK)
		ESP_LOGE(TAG, "Failed to switch network mode!");
}