/*
 * iot_usbh_modem.h
 *
 *  Created on: Jun 23, 2025
 *      Author: DIIV
 */

#ifndef COMPONENTS_IOT_USBH_MODEM_INCLUDE_IOT_USBH_MODEM_H_
#define COMPONENTS_IOT_USBH_MODEM_INCLUDE_IOT_USBH_MODEM_H_

#define OPERATOR_NAME_LEN	10

#define A7670E_ONLY_2G		"13"
#define A7670E_PREFER_4G	"38"
#define A7670E_NET_AUTO		"2"

typedef struct {
	char operatorName[OPERATOR_NAME_LEN];
	int rssi;
	int ber;
	void *dce;
}modemData_t;


modemData_t *ModemUSB_Init(void);
modemData_t *ModemUSB_GetSignalQuality(void);
void ModemUSB_SetNetMode(char netMode[2]);
#endif /* COMPONENTS_IOT_USBH_MODEM_INCLUDE_IOT_USBH_MODEM_H_ */
