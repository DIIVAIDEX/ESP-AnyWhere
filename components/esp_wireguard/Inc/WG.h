/*
 * WG.h
 *
 *  Created on: Sep 15, 2024
 *      Author: DIIV
 */

#ifndef MAIN_INC_WG_H_
#define MAIN_INC_WG_H_

#include "esp_system.h"
#include <esp_wireguard.h>

esp_err_t WG_Connect(wireguard_config_t *wgConnData);
#endif /* MAIN_INC_WG_H_ */
