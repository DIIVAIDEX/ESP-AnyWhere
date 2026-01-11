//#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "Bat_MAX17048.h"

static const char *TAG = "I2C_MAX17048";

#define I2C_MASTER_SCL_IO           GPIO_NUM_2	/*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           GPIO_NUM_3	/*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0	/*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          400000		/*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0			/*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0			/*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

max17048_data_t max17048_data;
i2c_master_dev_handle_t i2cMax17048;

max17048_data_t *MAX17048_I2CInit(void)
{    
    i2c_master_bus_handle_t bus_handle;
    
	i2c_master_bus_config_t bus_config = {
	    .i2c_port = I2C_MASTER_NUM,
	    .sda_io_num = I2C_MASTER_SDA_IO,
	    .scl_io_num = I2C_MASTER_SCL_IO,
	    .clk_source = I2C_CLK_SRC_DEFAULT,
	    .glitch_ignore_cnt = 7,
	    .flags.enable_internal_pullup = true,
	};
	ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));
	
	i2c_device_config_t dev_config = {
	    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
	    .device_address = MAX17048ADDR,
	    .scl_speed_hz = I2C_MASTER_FREQ_HZ,
	};
	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &i2cMax17048));
	
	return &max17048_data;
}

uint16_t BytesReorder(uint16_t byteToReorder)
{
	uint8_t tempByte = (uint8_t)byteToReorder;
	return (byteToReorder << 8) + tempByte;
}

void MAX17048_ReadAll(void)
{	uint16_t dataRx;
	esp_err_t ret;
	uint8_t dataTx = RegMax17048_vCell;
	ret = i2c_master_transmit_receive(i2cMax17048, &dataTx, 1, (uint8_t*)&dataRx, sizeof(uint16_t), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
	max17048_data.vCell = MAX17048_CALC_VCELL(BytesReorder(dataRx));
	
	dataTx = RegMax17048_SOC;
	ret = i2c_master_transmit_receive(i2cMax17048, &dataTx, 1, (uint8_t*)&dataRx, sizeof(uint16_t), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
	max17048_data.soc = MAX17048_CALC_SOC(BytesReorder(dataRx));
	
	dataTx = RegMax17048_cRate;
	ret = i2c_master_transmit_receive(i2cMax17048, &dataTx, 1, (uint8_t*)&dataRx, sizeof(uint16_t), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
	max17048_data.cRate = MAX17048_CALC_CRATE(BytesReorder(dataRx));
	
	if(ret != ESP_OK)ESP_LOGE(TAG, "I2C Transmit&Recieve failed at error: %d", ret);
}