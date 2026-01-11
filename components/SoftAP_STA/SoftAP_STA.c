/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <arpa/inet.h>
//#include "esp_system.h"
#include "esp_log.h"
//#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "lwip/lwip_napt.h"
#include "dhcpserver/dhcpserver.h"
#include "SoftAP_STA.h"

wifi_ap_config_t *wifi_ap_config;
wifi_sta_config_t *wifi_sta_config;
char *dns;

static const char *TAG = "WiFi_AP";
static int s_active_station_num = 0;
static esp_netif_t *s_wifi_netif[WIFI_MODE_MAX] = {NULL};
//static wifiAP_def_cfg_t wifiAP_def_cfg = WIFI_AP_DEF_CONFIG();

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD   WIFI_AUTH_WAPI_PSK
#endif

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/* FreeRTOS event group to signal when we are connected/disconnected */
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

/* Event handler for catching system events */
static void WiFi_EventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if(s_retry_num < CONFIG_MAXIMUM_STA_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        
        ESP_LOGI(TAG,"connect to the AP fail");
    }
    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_ASSIGNED_IP_TO_CLIENT) {
        ESP_LOGI(TAG, "Get IP addr");
    } else if (event_base == WIFI_EVENT) {
        switch (event_id) {
        case WIFI_EVENT_AP_STACONNECTED:
        wifi_event_ap_staconnected_t *eventConn = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG, "Station "MACSTR" joined, AID=%d",
                 MAC2STR(eventConn->mac), eventConn->aid);
            if (s_active_station_num == 0) {
                WiFi_EnableNAPT(true);
            }
            if (++s_active_station_num > 0) {
//                esp_event_post(MODEM_BOARD_EVENT, MODEM_EVENT_WIFI_STA_CONN, (void *)s_active_station_num, 0, 0);
            }
            break;
        case WIFI_EVENT_AP_STADISCONNECTED:
	        wifi_event_ap_stadisconnected_t *eventDisConn = (wifi_event_ap_stadisconnected_t *) event_data;
	        ESP_LOGI(TAG, "Station "MACSTR" left, AID=%d, reason:%d",
			MAC2STR(eventDisConn->mac), eventDisConn->aid, eventDisConn->reason);
        if (--s_active_station_num == 0) {
//          esp_event_post(MODEM_BOARD_EVENT, MODEM_EVENT_WIFI_STA_DISCONN, (void *)s_active_station_num, 0, 0);
            //TODO: esp-lwip NAPT now have a bug, disable then enable NAPT will cause random table index
            //modem_wifi_napt_enable(false);
        }
        	break;
        case WIFI_EVENT_STA_START:
            if (--s_active_station_num == 0) {
//                esp_event_post(MODEM_BOARD_EVENT, MODEM_EVENT_WIFI_STA_DISCONN, (void *)s_active_station_num, 0, 0);
                //TODO: esp-lwip NAPT now have a bug, disable then enable NAPT will cause random table index
                //modem_wifi_napt_enable(false);
            }
            break;
        case IP_EVENT_STA_GOT_IP:
            if (--s_active_station_num == 0) {
//                esp_event_post(MODEM_BOARD_EVENT, MODEM_EVENT_WIFI_STA_DISCONN, (void *)s_active_station_num, 0, 0);
                //TODO: esp-lwip NAPT now have a bug, disable then enable NAPT will cause random table index
                //modem_wifi_napt_enable(false);
            }
            break;
        default:
            break;
        }
    }
}

esp_err_t WiFi_SetDNS(esp_netif_t *netif, uint32_t addr)
{
    esp_netif_dns_info_t dns;
    dns.ip.u_addr.ip4.addr = addr;
    dns.ip.type = IPADDR_TYPE_V4;
    dhcps_offer_t dhcps_dns_value = OFFER_DNS;
    esp_netif_dhcps_stop(netif);
    ESP_ERROR_CHECK(esp_netif_dhcps_option(netif, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_dns_value, sizeof(dhcps_dns_value)));
    ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns));
    esp_netif_dhcps_start(netif);
    return ESP_OK;
}

esp_netif_t *WiFi_Init(wifi_ap_config_t *_wifi_ap_config, wifi_sta_config_t *_wifi_sta_config, char *_dns)
{
	wifi_ap_config = _wifi_ap_config;
	wifi_sta_config = _wifi_sta_config;
    esp_netif_t *wifi_netif = NULL;
	wifi_netif = esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Initialize event group */
    s_wifi_event_group = xEventGroupCreate();
    
    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFi_EventHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &WiFi_EventHandler, NULL));
//    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    s_wifi_netif[WIFI_MODE_AP] = wifi_netif;
    
    wifi_ap_config->authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
    wifi_ap_config->max_connection = 10;
    
    wifi_sta_config->scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifi_sta_config->failure_retry_cnt = CONFIG_MAXIMUM_STA_RETRY;
    wifi_sta_config->threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
    
    dns = _dns;
    
    return wifi_netif;
}

esp_err_t WiFi_StartAP(void)
{
    
    if (wifi_ap_config->ssid[0] == 0 || wifi_ap_config->password[0] == 0) {
        return ESP_ERR_INVALID_ARG;
    }

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, (wifi_config_t*)wifi_ap_config));
    
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi SoftAP started");
    ESP_LOGI(TAG, "softap ssid: %s password: %s", wifi_ap_config->ssid, wifi_ap_config->password);
    
    ESP_ERROR_CHECK(WiFi_SetDNS(s_wifi_netif[WIFI_MODE_AP], inet_addr(dns)));
    return ESP_OK;
}

/* Initialize wifi station */
void WiFi_StartSTA(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

//    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

//    esp_event_handler_instance_t instance_any_id;
//    esp_event_handler_instance_t instance_got_ip;
//    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
//                                                        ESP_EVENT_ANY_ID,
//                                                        &WiFi_EventHandler,
//                                                        NULL,
//                                                        &instance_any_id));
//    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
//                                                        IP_EVENT_STA_GOT_IP,
//                                                        &WiFi_EventHandler,
//                                                        NULL,
//                                                        &instance_got_ip));

    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = CONFIG_WIFI_REMOTE_AP_SSID,
            .password = CONFIG_WIFI_REMOTE_AP_PASSWORD,
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .failure_retry_cnt = CONFIG_MAXIMUM_STA_RETRY,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
            * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());


    /*
     * Wait until either the connection is established (WIFI_CONNECTED_BIT) or
     * connection failed for the maximum number of re-tries (WIFI_FAIL_BIT).
     * The bits are set by event_handler() (see above)
     */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* xEventGroupWaitBits() returns the bits before the call returned,
     * hence we can test which event actually happened. */
    if(bits & WIFI_CONNECTED_BIT){
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 CONFIG_WIFI_REMOTE_AP_SSID, CONFIG_WIFI_REMOTE_AP_PASSWORD);
    } 
    else if(bits & WIFI_FAIL_BIT){
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 CONFIG_WIFI_REMOTE_AP_SSID, CONFIG_WIFI_REMOTE_AP_PASSWORD);
    }
    else{
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

void WiFi_StopAP(void)
{
    ESP_ERROR_CHECK(esp_wifi_stop());
}

esp_err_t WiFi_EnableNAPT(bool enable)
{
    ip_napt_enable(_g_esp_netif_soft_ap_ip.ip.addr, enable);
    ESP_LOGI(TAG, "NAT is %s", enable ? "enabled" : "disabled");
    return ESP_OK;
}
