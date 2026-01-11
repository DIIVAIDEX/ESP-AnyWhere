/*
 * main.c
 *
 *  Created on: Sep 14, 2024
 *      Author: DIIV
 */

#include "nvs_flash.h"
#include <WG.h>
#include <stdbool.h>
#include <stdint.h>
#include "SoftAP_STA.h"
#include "WebServer.h"
#include "Storage.h"
#include "iot_usbh_modem.h"
#include "LogsToWeb.h"
#include "esp_pm.h"


void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
	LogsToWeb_Init();
    
    /* Initialize default TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    Storage_Init();
	settings_t *settings = StorageSettings_Get();
	
	WiFi_Init(&settings->wifi_ap_config, &settings->wifi_sta_config, settings->dns);

	modemData_t *modemData = ModemUSB_Init();
    telemetry_t tlmtry = {.modemData = modemData};
    
	Telemetry_Start(&tlmtry);
	
	WG_Connect(&settings->wgConnData);
	
  	WebServer_Start();
  	
	esp_pm_config_t esp_pm_config = {	.max_freq_mhz = 240,
										.min_freq_mhz = 10,
										.light_sleep_enable = false
	};
	esp_pm_configure(&esp_pm_config);
}