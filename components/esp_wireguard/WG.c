/*
 * WG.c
 *
 *  Created on: Sep 15, 2024
 *      Author: DIIV
 */

#include <WG.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_event.h>
#include <esp_idf_version.h>
#include <esp_log.h>
#include <esp_system.h>
#include <lwip/netdb.h>
#include <ping/ping_sock.h>

#include <esp_wireguard.h>
#include "SyncTime.h"


static const char *TAG = "WireGuard";

TaskHandle_t taskWG = NULL;

static esp_err_t wireguard_setup(wireguard_ctx_t* ctx, wireguard_config_t *wgConnData)
{
    esp_err_t err = ESP_FAIL;

    ESP_LOGI(TAG, "Initializing WireGuard.");
    
    err = esp_wireguard_init(wgConnData, ctx);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wireguard_init: %s", esp_err_to_name(err));
        goto fail;
    }

    ESP_LOGI(TAG, "Connecting to the peer.");
    err = esp_wireguard_connect(ctx);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wireguard_connect: %s", esp_err_to_name(err));
        goto fail;
    }

    err = ESP_OK;
fail:
    return err;
}

esp_err_t WG_Connect(wireguard_config_t *wgConnData)
{
    esp_err_t err;
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    wireguard_ctx_t ctx = {0};
    
    esp_log_level_set("esp_wireguard", ESP_LOG_DEBUG);
    esp_log_level_set("wireguardif", ESP_LOG_DEBUG);
    esp_log_level_set("wireguard", ESP_LOG_DEBUG);
    
    obtain_time();
    time(&now);

    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Yekaterinburg is: %s", strftime_buf);


//	vTaskDelay(pdMS_TO_TICKS(5000));

    err = wireguard_setup(&ctx, wgConnData);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "wireguard_setup: %s", esp_err_to_name(err));
    }
	return ESP_OK;
}
