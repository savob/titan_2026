/*
 * scd4x.h
 *
 *  Created on: Nov 9, 2025
 *      Author: savo
 */

#ifndef INC_INA219_H_
#define INC_INA219_H_

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

HAL_StatusTypeDef scd4x_setup(I2C_HandleTypeDef* bus, uint32_t timeout);
HAL_StatusTypeDef scd4x_read_co2(uint16_t* result_ppm);

#endif /* INC_INA219_H_ */
