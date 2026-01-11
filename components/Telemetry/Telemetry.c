/*
 * telemetry.c
 *
 *  Created on: Jun 23, 2025
 *      Author: DIIV
 */
 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "Telemetry.h"
#include "ComWS.h"
#include <stdint.h>
#include "driver/temperature_sensor.h"

#define TELEMETRY_SEND_INTERVAL	pdMS_TO_TICKS(3 * 1000)

static const char *TAG = "Telemetry";
temperature_sensor_handle_t temp_sensor = NULL;
temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-20, 50);
TaskHandle_t telemetryTaskHandler = NULL;

telemetry_t tlmtry;

static void Telemetry_Routine(void *arg)
{
	uint64_t webStatsSendTimer = 0;
	
    ESP_LOGI(TAG, "Install temperature sensor, expected temp ranger range: 10~50 ℃");
    temperature_sensor_install(&temp_sensor_config, &temp_sensor);
    ESP_LOGI(TAG, "Enable temperature sensor");
    temperature_sensor_enable(temp_sensor);
	for(;;)
	{
		if(xTaskGetTickCount() > webStatsSendTimer + TELEMETRY_SEND_INTERVAL){
			temperature_sensor_get_celsius(temp_sensor, &tlmtry.cpuTemp);
			tlmtry.freeHeap = esp_get_free_heap_size();
			MAX17048_ReadAll();
			ModemUSB_GetSignalQuality();
			SendJsonTelemetry(&tlmtry);
			webStatsSendTimer = xTaskGetTickCount();	
		}
		
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

telemetry_t *Telemetry_Start(telemetry_t *_tlmtry)
{
	tlmtry.modemData = _tlmtry->modemData;
	
	tlmtry.max17048_data = MAX17048_I2CInit();
	xTaskCreate(Telemetry_Routine, "TelemetryTask", 4096, NULL, tskIDLE_PRIORITY, &telemetryTaskHandler);
	
	return &tlmtry;
}
