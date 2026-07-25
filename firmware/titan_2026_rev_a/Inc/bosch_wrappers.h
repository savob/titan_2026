/*
 * bosch_wrappers.h
 *
 *  Created on: Nov 10, 2025
 *      Author: savo
 */

#ifndef INC_BOSCH_WRAPPERS_H_
#define INC_BOSCH_WRAPPERS_H_

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

struct BoschI2C {
	I2C_HandleTypeDef* i2c_handle;
	uint8_t address;
};

void bosch_adjust_i2c_timeout(uint32_t timeout_i2c_ms);
void bosch_delay_us(uint32_t period_us, void *intf_ptr);
int8_t bosch_write_i2c_by_intf(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t bosch_read_i2c_by_intf(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

#endif /* INC_BOSCH_WRAPPERS_H_ */
