/*
 * telemetry.h
 *
 *  Created on: Jun 23, 2025
 *      Author: DIIV
 */

#ifndef MAIN_INC_TELEMETRY_H_
#define MAIN_INC_TELEMETRY_H_

#include <string.h>
#include "Bat_MAX17048.h"
#include "iot_usbh_modem.h"

typedef struct{
	uint64_t runtime;
	max17048_data_t *max17048_data;
	modemData_t *modemData;
	float cpuTemp;
	uint32_t freeHeap;
}telemetry_t;

telemetry_t *Telemetry_Start(telemetry_t *_tlmtry);
#endif /* MAIN_INC_TELEMETRY_H_ */
