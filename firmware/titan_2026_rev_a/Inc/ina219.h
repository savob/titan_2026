/*
 * ina219.h
 *
 *  Created on: Nov 9, 2025
 *      Author: savo
 */

#ifndef INC_INA219_H_
#define INC_INA219_H_

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

HAL_StatusTypeDef ina219_setup(const uint8_t INA_ADD, I2C_HandleTypeDef* bus, uint32_t timeout);
HAL_StatusTypeDef ina219_read_power(const uint8_t INA_ADD, float * result_w);
HAL_StatusTypeDef ina219_read_bus_voltage(const uint8_t INA_ADD, float * result_v);
HAL_StatusTypeDef ina219_read_bus_current(const uint8_t INA_ADD, float * result_a);
HAL_StatusTypeDef ina219_update_calibration(const uint8_t INA_ADD, float expected_max_current_a, float shunt_resistance_ohm);

extern const uint8_t PRIM_INA; 	// Address for primary board's INA219
extern const uint8_t SEC_INA;	// Address for secondary board's INA219

#endif /* INC_INA219_H_ */
