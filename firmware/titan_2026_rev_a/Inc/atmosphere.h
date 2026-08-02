/*
 * atmosphere.h
 *
 *  Created on: Nov 12, 2025
 *      Author: savo
 */

#ifndef INC_ATMOSPHERE_H_
#define INC_ATMOSPHERE_H_

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"
#include "titan_data.h"

HAL_StatusTypeDef atmo_setup(I2C_HandleTypeDef* bus, uint32_t i2c_timeout_ms);

HAL_StatusTypeDef atmo_conditions_update(struct TitanSummary* target, uint16_t mh_z19_co2_ppm);

#endif /* INC_ATMOSPHERE_H_ */
