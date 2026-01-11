/*
 * ap_to_pppos.h
 *
 *  Created on: Mar 16, 2025
 *      Author: DIIV
 */

#ifndef MAX17048_H_
#define MAX17048_H_

#include <stdint.h>

#define MAX17048ADDR         		0x36
#define MAX17048_CALC_VCELL(raw)	(float)((float)raw * (78.125 * 1e-6))
#define MAX17048_CALC_SOC(raw)		(float)((float)raw / 256)
#define MAX17048_CALC_CRATE(raw)	(float)((float)raw * 0.208)
enum{
	RegMax17048_vCell	= (uint8_t)0x02,
	RegMax17048_SOC		= 0x04,
	RegMax17048_cRate	= 0x16
};

typedef struct{
	float vCell;
	float soc;
//	uint16_t mode;
//	uint16_t ver;
//	uint16_t hibrt;
//	uint16_t conf;
//	uint16_t valRT;
	float cRate;
//	uint16_t vResetID;
//	uint16_t status;
}max17048_data_t;

max17048_data_t *MAX17048_I2CInit(void);
void MAX17048_ReadAll(void);
#endif /* MAIN_AP_TO_PPPOS_INC_AP_TO_PPPOS_H_ */
