/* Finding Partitions Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <assert.h>
#include "esp_log.h"
#include "Storage.h"
#include "freertos/FreeRTOS.h"
#include "esp_partition.h"

#define FLASH_ERASED_VALUE 0xFFFFFFFFFFFFFFFF

static const char *TAG = "Storage";

const esp_partition_t *storagePart;
const esp_partition_t *settingsPart;
//uint32_t currFreeSegmentOffset = 0;
uint64_t lastWriteTime;

settings_t settings;

bool IsDataDiffirent(uint8_t *src, uint8_t *dst, uint32_t len)
{
	//TODO: It will be great to compare in uint32_t! Optimize it!
	for(uint32_t i = 0; i < len; i++){
		if(*(src + i) != *(dst + i))
			return true;
	}
	return false;
}

settings_t *StorageSettings_Get()
{
	return &settings;
}

static void StorageSettings_Load(settings_t *settings)
{	
	ESP_ERROR_CHECK(esp_partition_read(settingsPart, 0, settings, sizeof(settings_t)));
}

void StorageSettings_Save(settings_t *_settingstoSave)
{
	settings_t settingsInStorage, *settingstoSave = &settings;
	
	StorageSettings_Load(&settingsInStorage);
	
	if(_settingstoSave != NULL) settingstoSave = _settingstoSave;
	
	if(IsDataDiffirent((uint8_t*)settingstoSave, (uint8_t*)&settingsInStorage, sizeof(settings_t))){
		ESP_ERROR_CHECK(esp_partition_erase_range(settingsPart, 0, settingsPart->size));
		
		ESP_LOGI(TAG, "OLD Settings:");
	    ESP_LOG_BUFFER_HEXDUMP(TAG, &settingsInStorage, sizeof(settings_t), ESP_LOG_INFO);
		ESP_LOGI(TAG, "NEW Settings:");
	    ESP_LOG_BUFFER_HEXDUMP(TAG, settingstoSave, sizeof(settings_t), ESP_LOG_INFO);
	
		esp_partition_write(settingsPart, 0, settingstoSave, sizeof(settings_t));
		
    	ESP_LOGI(TAG, "Settings saved, bytes written: %d", sizeof(settings_t));
	}
}

static void SetDefaultSettings(void)
{
	settings_t settings = {	.wgConnData.private_key =  CONFIG_WG_PRIVATE_KEY, 							
							.wgConnData.listen_port = CONFIG_WG_LOCAL_PORT, 							
							.wgConnData.fw_mark = 0,
							.wgConnData.public_key = CONFIG_WG_PEER_PUBLIC_KEY,
							.wgConnData.allowed_ip = CONFIG_WG_LOCAL_IP_ADDRESS, 							
							.wgConnData.allowed_ip_mask = CONFIG_WG_LOCAL_IP_NETMASK, 							
							.wgConnData.endpoint = CONFIG_WG_PEER_ADDRESS, 							
							.wgConnData.port = CONFIG_WG_PEER_PORT, 							
							.wgConnData.persistent_keepalive = CONFIG_WG_PERSISTENT_KEEP_ALIVE, 							

							.wifi_ap_config.ssid = CONFIG_WIFI_SSID,
							.wifi_ap_config.password = CONFIG_WIFI_PASSWORD,
							.wifi_ap_config.channel = CONFIG_WIFI_CHANNEL,
							.wifi_sta_config.ssid = CONFIG_WIFI_REMOTE_AP_SSID,
							.wifi_sta_config.password = CONFIG_WIFI_REMOTE_AP_PASSWORD,
							.dns = CONFIG_WIFI_DEFAULT_DNS,
							.size = sizeof(settings_t)};
	
	StorageSettings_Save(&settings);
}

void StorageFlush()
{
	esp_partition_erase_range(settingsPart, 0, settingsPart->size);
//	esp_partition_erase_range(storagePart, 0, storagePart->size);
    esp_restart();
}

void Storage_Init(void)
{
//    storagePart = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    settingsPart = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "settings");
//    assert(storagePart != NULL);
    assert(settingsPart != NULL);

	StorageSettings_Load(&settings);
	if(settings.size != sizeof(settings_t))SetDefaultSettings();

//	FindFirstFreeSegmentOffset();
	
	StorageSettings_Load(&settings);
//	StorageCounters_Load(&counters);
}
