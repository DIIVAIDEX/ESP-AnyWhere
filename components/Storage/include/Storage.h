/*
 * ADC_TDS.h
 *
 *  Created on: Sep 19, 2024
 *      Author: DIIV
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdint.h>
#include <esp_wifi_types_generic.h>
#include <esp_wireguard.h>
#include <sys/cdefs.h>

typedef struct{
	wireguard_config_t wgConnData;
	wifi_ap_config_t wifi_ap_config;
	wifi_sta_config_t wifi_sta_config;
	char dns[16];
	uint32_t size;
}settings_t;


settings_t *StorageSettings_Get();
//storageCounters_t *StorageCounters_Get();
//settings_t *StorageSettings_Load();
void StorageSettings_Save(settings_t *_settingstoSave);
//storageCounters_t *StorageCounters_Load();
//void StorageCounters_Save(storageCounters_t *_countersToSave);
void StorageFlush();
void Storage_Init(void);
#endif /* STORAGE_H_ */
