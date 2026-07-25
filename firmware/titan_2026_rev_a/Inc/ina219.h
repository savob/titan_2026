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

HAL_StatusTypeDef ina219_setup(I2C_HandleTypeDef* bus, uint32_t timeout);
HAL_StatusTypeDef ina219_read_power(float * result_w);
HAL_StatusTypeDef ina219_read_bus_voltage(float * result_v);
HAL_StatusTypeDef ina219_read_bus_current(float * result_a);
HAL_StatusTypeDef ina219_update_calibration(float expected_max_current_a, float shunt_resistance_ohm);

#endif /* INC_INA219_H_ */
