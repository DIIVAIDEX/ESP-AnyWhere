/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include "esp_err.h"
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C"
{
#endif

esp_netif_t *WiFi_Init(wifi_ap_config_t *_wifi_ap_config, wifi_sta_config_t *_wifi_sta_config, char *_dns);
esp_err_t WiFi_StartAP(void);
void WiFi_StopAP(void);
esp_err_t WiFi_EnableNAPT(bool enable);
//esp_err_t WiFi_SetDNS(esp_netif_t *netif, uint32_t addr);
void WiFi_StartSTA(void);
#ifdef __cplusplus
}
#endif
