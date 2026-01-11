/*
 * ComWS.h
 *
 *  Created on: Apr 7, 2025
 *      Author: DIIV
 */

#ifndef COMPONENTS_WEBSERVERWS_INC_COMWS_H_
#define COMPONENTS_WEBSERVERWS_INC_COMWS_H_

#include <esp_http_server.h>
#include "Telemetry.h"
//#include "PPPoS.h"
//#include "MainFSM.h"

typedef struct{
	uint8_t input1:1;
	uint8_t input2:1;
	uint8_t input3:1;
	uint8_t input4:1;
	uint8_t input5:1;
	uint8_t input6:1;
	uint8_t input7:1;
	uint8_t input8:1;
}jsonInputs_t;

typedef enum{
	BasicInfo	= 0,
	Settings
}jsonType_t;

typedef struct{
//	modemData_t *modemData;
	uint16_t *stationId;
	char* base_path;
}webServData_t;

//void SendInputsJson(fsmDrInputsMap_t *inOnly);
void SendJsonTelemetry(telemetry_t *tlmtry);
void ws_parse(httpd_req_t *req, httpd_ws_frame_t *ws_pkt);
void LogSend(char *logString, uint16_t len);
#endif /* COMPONENTS_WEBSERVERWS_INC_COMWS_H_ */
