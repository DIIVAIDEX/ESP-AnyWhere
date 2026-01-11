/*
 * LogsToWeb.c
 *
 *  Created on: Sep 2, 2025
 *      Author: DIIV
 */ 
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "esp_log.h"
#include "ComWS.h"

RingbufHandle_t log_buffer;

static const char *TAG = "Telemetry";

int vprintf_dual(const char *fmt, va_list args) {
    char buffer[256];
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    
    // Отправка в UART0 (стандартный вывод)
    vprintf(fmt, args);
    
    // Запись в кольцевой буфер
    UBaseType_t res = xRingbufferSend(log_buffer, buffer, len + 1, pdMS_TO_TICKS(100));
    if (res != pdTRUE) ESP_LOGE(TAG, "Log ringbuf is FULL!!");
    return len;
}


void LogsToWeb_Init(void)
{
    log_buffer = xRingbufferCreate(128*1024, RINGBUF_TYPE_NOSPLIT);
    if(log_buffer != NULL) esp_log_set_vprintf(vprintf_dual);
    else ESP_LOGE(TAG, "Error to create log ringbuf!");
    
}

void LogsToWeb_SendingTask(void *arg)
{
	char *data = NULL;
	size_t item_size;
	
	do{
		data = (char*)xRingbufferReceive(log_buffer, &item_size, pdMS_TO_TICKS(100));
		
		if(data != NULL){
			LogSend(data, item_size);
	        vRingbufferReturnItem(log_buffer, data);
		}
		
	}while(data != NULL);
	
    vTaskDelete(NULL);
}