/*
 * ComWS.c
 *
 *  Created on: Apr 7, 2025
 *      Author: DIIV
 */

#include <stdio.h>
#include "esp_log.h"
#include "cJSON.h"
//#include <cjson/cJSON.h>
#include "ComWS.h"
//#include "main.h"
#include "SoftAP_STA.h"
#include "Telemetry.h"
#include "OTA.h"
#include "Storage.h"
#include "LogsToWeb.h"
#include <freeRTOS_Stats.h>

httpd_handle_t hd = NULL;

extern webServData_t webServData;
TaskHandle_t logsTask = NULL;

static const char *TAG = "WebSocket Server"; // TAG for debug

static void ws_async_send(void *arg)
{
    httpd_ws_frame_t ws_pkt;
    char *jsonString = (char*)arg;
    
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)jsonString;
    ws_pkt.len = strlen(jsonString);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
//    static size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    size_t fds = CONFIG_LWIP_MAX_LISTENING_TCP;
    int client_fds[CONFIG_LWIP_MAX_LISTENING_TCP] = {0};

    esp_err_t ret = httpd_get_client_list(hd, &fds, client_fds);

    if (ret == ESP_OK && fds){
	    for (int i = 0; i < fds; i++){
	        httpd_ws_client_info_t client_info = httpd_ws_get_fd_info(hd, client_fds[i]);
	        if(client_info == HTTPD_WS_CLIENT_WEBSOCKET) httpd_ws_send_frame_async(hd, client_fds[i], &ws_pkt);
	    }
    }
    else hd = NULL;
    
    cJSON_free(jsonString);
}



void SendJsonTelemetry(telemetry_t *tlmtry)
{
	if(hd != NULL){
		cJSON *root = NULL;
	
//		ESP_LOGI(TAG, "RSSI: %d", tlmtry->modemData->rssi);
		
		root = cJSON_CreateObject();
		if(root != NULL){
			cJSON_AddStringToObject(root, "type", "info");
			cJSON_AddStringToObject(root, "operator", tlmtry->modemData->operatorName);
			cJSON_AddNumberToObject(root, "rssi", (double)tlmtry->modemData->rssi);
			cJSON_AddNumberToObject(root, "ber", tlmtry->modemData->ber);
			cJSON_AddNumberToObject(root, "vBat", tlmtry->max17048_data->vCell);
			cJSON_AddNumberToObject(root, "vBatPerc", tlmtry->max17048_data->soc);
			cJSON_AddNumberToObject(root, "freeHeap", tlmtry->freeHeap);
			cJSON_AddNumberToObject(root, "cpuTemp", tlmtry->cpuTemp);
		}
		char *jsonString = cJSON_Print(root);
		
		if(jsonString != NULL)ESP_ERROR_CHECK(httpd_queue_work(hd, ws_async_send, jsonString));
		else{
    		cJSON_free(jsonString);
			ESP_LOGE(TAG, "Cannot malloc for JSON string!");
		} 
		cJSON_Delete(root);
	}
}

//void SendInputsJson(fsmDrInputsMap_t *inOnly)
//{
//	if(hd != NULL){
//		cJSON *root = NULL;
////		char toggleInputString[13];
//	
//		root = cJSON_CreateObject();
//		if(root != NULL){
//			cJSON_AddStringToObject(root, "type", "inputs");
//			
//			if(inOnly->in1)cJSON_AddTrueToObject(root, "toggleInput1");
//			else cJSON_AddFalseToObject(root, "toggleInput1");
//			if(inOnly->inletWater)cJSON_AddTrueToObject(root, "toggleInput2");
//			else cJSON_AddFalseToObject(root, "toggleInput2");
//			if(inOnly->highPressure)cJSON_AddTrueToObject(root, "toggleInput3");
//			else cJSON_AddFalseToObject(root, "toggleInput3");
//			if(inOnly->in4)cJSON_AddTrueToObject(root, "toggleInput4");
//			else cJSON_AddFalseToObject(root, "toggleInput4");
//			if(inOnly->in5)cJSON_AddTrueToObject(root, "toggleInput5");
//			else cJSON_AddFalseToObject(root, "toggleInput5");
//			if(inOnly->in6)cJSON_AddTrueToObject(root, "toggleInput6");
//			else cJSON_AddFalseToObject(root, "toggleInput6");
//			if(inOnly->in7)cJSON_AddTrueToObject(root, "toggleInput7");
//			else cJSON_AddFalseToObject(root, "toggleInput7");
//			if(inOnly->waterCounter)cJSON_AddTrueToObject(root, "toggleInput8");
//			else cJSON_AddFalseToObject(root, "toggleInput8");
//			
////			for(uint8_t i = 0; i < 8; i++)
////			{
////				sprintf(toggleInputString, "toggleInput%d", i + 1);
////				if(inOnly) cJSON_AddTrueToObject(root, "netStatus");
////			}
//		}
//		char *jsonString = cJSON_Print(root);
//		
//		if(jsonString != NULL)ESP_ERROR_CHECK(httpd_queue_work(hd, ws_async_send, jsonString));
//		else{
//    		cJSON_free(jsonString);
//			ESP_LOGE(TAG, "Cannot malloc for JSON string!");
//		} 
//		cJSON_Delete(root);
//	}
//}

void SendJsonData(jsonType_t jsonType)
{
	if(hd != NULL){
		cJSON *root = NULL;
	
		root = cJSON_CreateObject();
		if(root != NULL){
			switch(jsonType)
			{
				case BasicInfo:
				{
//					cJSON_AddNumberToObject(root, "operator", *webServData.stationId);
				}
				break;
				case Settings:
				{
					settings_t *settings = StorageSettings_Get();
					cJSON_AddStringToObject(root, "type", "settings");
					
					cJSON_AddStringToObject(root, "wgPrivateKey", settings->wgConnData.private_key);
					cJSON_AddStringToObject(root, "wgLocalIP", settings->wgConnData.allowed_ip);
					cJSON_AddStringToObject(root, "wgLocalMask", settings->wgConnData.allowed_ip_mask);
					cJSON_AddStringToObject(root, "wgPublicPeerKey", settings->wgConnData.public_key);
					cJSON_AddStringToObject(root, "wgPeerAddress", settings->wgConnData.endpoint);
					cJSON_AddNumberToObject(root, "wgPeerPort", settings->wgConnData.port);
					
					cJSON_AddStringToObject(root, "wifiStaName", (char*)settings->wifi_sta_config.ssid);
					cJSON_AddStringToObject(root, "wifiStaPass", (char*)settings->wifi_sta_config.password);
					cJSON_AddStringToObject(root, "wifiApName", (char*)settings->wifi_ap_config.ssid);
					cJSON_AddStringToObject(root, "wifiApPass", (char*)settings->wifi_ap_config.password);
					cJSON_AddNumberToObject(root, "wifiApCh", settings->wifi_ap_config.channel);
				}
				break;
			}
		}
		char *jsonString = cJSON_Print(root);
		
		if(jsonString != NULL)ESP_ERROR_CHECK(httpd_queue_work(hd, ws_async_send, jsonString));
		else{
    		cJSON_free(jsonString);
			ESP_LOGE(TAG, "Cannot malloc for JSON string!");
		} 
		cJSON_Delete(root);
	}
}

static void JsonParser(char *payload)
{
	cJSON *root = cJSON_Parse(payload);
	
	if(root != NULL){
		cJSON *typeJson = cJSON_GetObjectItemCaseSensitive(root, "typeJson");
		if (cJSON_IsString(typeJson) && (typeJson->valuestring != NULL)){
			if(strcmp((char *)typeJson->valuestring, "wifiAp") == 0 || strcmp((char *)typeJson->valuestring, "wifiSta") == 0 || 
			   strcmp((char *)typeJson->valuestring, "wireguard") == 0){
				settings_t *settings = StorageSettings_Get();
				
				cJSON *wgPrivateKey = cJSON_GetObjectItemCaseSensitive(root, "wgPrivateKey");
				if(cJSON_IsString(wgPrivateKey)) strcpy(settings->wgConnData.private_key, cJSON_GetStringValue(wgPrivateKey));
				cJSON *wgLocalIP = cJSON_GetObjectItemCaseSensitive(root, "wgLocalIP");
				if(cJSON_IsString(wgLocalIP)) strcpy(settings->wgConnData.allowed_ip, cJSON_GetStringValue(wgLocalIP));
				cJSON *wgLocalMask = cJSON_GetObjectItemCaseSensitive(root, "wgLocalMask");
				if(cJSON_IsString(wgLocalMask)) strcpy(settings->wgConnData.allowed_ip_mask, cJSON_GetStringValue(wgLocalMask));
				cJSON *wgPublicPeerKey = cJSON_GetObjectItemCaseSensitive(root, "wgPublicPeerKey");
				if(cJSON_IsString(wgPublicPeerKey)) strcpy(settings->wgConnData.public_key, cJSON_GetStringValue(wgPublicPeerKey));
				cJSON *wgPeerAddress = cJSON_GetObjectItemCaseSensitive(root, "wgPeerAddress");
				if(cJSON_IsString(wgPeerAddress)) strcpy(settings->wgConnData.endpoint, cJSON_GetStringValue(wgPeerAddress));
				cJSON *wgPeerPort = cJSON_GetObjectItemCaseSensitive(root, "wgPeerPort");
				if(cJSON_IsNumber(wgPeerPort)) settings->wgConnData.port = cJSON_GetNumberValue(wgPeerPort);
				
				cJSON *wifiStaName = cJSON_GetObjectItemCaseSensitive(root, "wifiStaName");
				if(cJSON_IsString(wifiStaName)) strcpy((char*)settings->wifi_sta_config.ssid, cJSON_GetStringValue(wifiStaName));
				cJSON *wifiStaPass = cJSON_GetObjectItemCaseSensitive(root, "wifiStaPass");
				if(cJSON_IsString(wifiStaPass)) strcpy((char*)settings->wifi_sta_config.password, cJSON_GetStringValue(wifiStaPass));
				cJSON *wifiApName = cJSON_GetObjectItemCaseSensitive(root, "wifiApName");
				if(cJSON_IsString(wifiApName)) strcpy((char*)settings->wifi_ap_config.ssid, cJSON_GetStringValue(wifiApName));
				cJSON *wifiApPass = cJSON_GetObjectItemCaseSensitive(root, "wifiApPass");
				if(cJSON_IsString(wifiApPass)) strcpy((char*)settings->wifi_ap_config.password, cJSON_GetStringValue(wifiApPass));
				cJSON *wifiApCh = cJSON_GetObjectItemCaseSensitive(root, "wifiApCh");
				if(cJSON_IsNumber(wifiApCh)) settings->wifi_ap_config.channel = cJSON_GetNumberValue(wifiApCh);
				
				StorageSettings_Save(NULL);
				
	  			SendJsonData(Settings);
			}
		}
		
		cJSON *softAP = cJSON_GetObjectItemCaseSensitive(root, "SoftAP");
		if(cJSON_IsBool(softAP)){
			if(cJSON_IsTrue(softAP))WiFi_StartAP();
			else WiFi_StopAP();
		}

		cJSON *only2G = cJSON_GetObjectItemCaseSensitive(root, "Only2G");
		if(cJSON_IsBool(only2G)){
			if(cJSON_IsTrue(only2G))ModemUSB_SetNetMode(A7670E_ONLY_2G);
			else ModemUSB_SetNetMode(A7670E_PREFER_4G);
		}
	}
	cJSON_Delete(root);
}

void ws_parse(httpd_req_t *req, httpd_ws_frame_t *ws_pkt)
{
    if (ws_pkt->type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt->payload, "getReadings") == 0)
    {
		if(hd == NULL){
	        ESP_LOGI(TAG, "Starting task to interval send readings");
			hd = req->handle;
	  	}
	  	SendJsonData(BasicInfo);
//	  	SendJsonData(IoNaming);
    }
    if (ws_pkt->type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt->payload, "getLogs") == 0)
    {
		xTaskCreate(LogsToWeb_SendingTask, "LogsSendingsTask", 8192, NULL, tskIDLE_PRIORITY, NULL);
    }
    if (ws_pkt->type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt->payload, "updateFW") == 0)
    {
		StartOTA();
    }
    if (ws_pkt->type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt->payload, "printStats") == 0)
    {
		PrintSysStats();
    }
    if (ws_pkt->type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt->payload, "getSettings") == 0)
    {
	  	SendJsonData(Settings);
    }
    if (ws_pkt->type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt->payload, "setDiagMode") == 0)
    {
//	  	SendJsonData(IoNaming);
    }
    if (ws_pkt->type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt->payload, "reboot") == 0)
    {
		vTaskDelay(pdMS_TO_TICKS(1000));
		
		esp_restart();
    }
    if (ws_pkt->type == HTTPD_WS_TYPE_TEXT && strcmp((char *)ws_pkt->payload, "storageFlush") == 0)
    {
	  	StorageFlush();
    }
    else if (ws_pkt->type == HTTPD_WS_TYPE_TEXT)
    {
		JsonParser((char *)ws_pkt->payload);
    }
}

void LogSend(char *logString, uint16_t len)
{
	if(hd != NULL){
		cJSON *root = cJSON_CreateObject();
		
		if(root != NULL){
			cJSON_AddStringToObject(root, "type", "log");
			cJSON_AddNumberToObject(root, "len", len);
			cJSON_AddStringToObject(root, "log", logString);
			
			char *jsonString = cJSON_Print(root);
			ws_async_send(jsonString);
			
			cJSON_Delete(root);
		}
	}
}