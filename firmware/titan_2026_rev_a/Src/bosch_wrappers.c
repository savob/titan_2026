/*
 * bosch_wrappers.c
 *
 *  Created on: Nov 10, 2025
 *      Author: savo
 */
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "bosch_wrappers.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

static uint32_t I2C_TIMEOUT_MS = 100;

void bosch_adjust_i2c_timeout(uint32_t timeout_i2c_ms) {
	I2C_TIMEOUT_MS = timeout_i2c_ms;
}

void bosch_delay_us(uint32_t period_us, void *intf_ptr) {
	(void) intf_ptr; // Not used

	// Approximate microseconds with rounding. Hopefully this isn't needed much
	uint32_t milliseconds_roughly = (period_us + 500) / 1000;
	HAL_Delay(milliseconds_roughly);
}

int8_t bosch_write_i2c_by_intf(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
	struct BoschI2C* interface = intf_ptr;
	return HAL_I2C_Mem_Write(interface->i2c_handle, interface->address << 1, reg_addr, 1, reg_data, len, I2C_TIMEOUT_MS);
}

int8_t bosch_read_i2c_by_intf(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
	struct BoschI2C* interface = intf_ptr;
	return HAL_I2C_Mem_Read(interface->i2c_handle, interface->address << 1, reg_addr, 1, reg_data, len, I2C_TIMEOUT_MS);
}
