/*
 * mlx90614.h
 *
 *  Created on: Jul 26, 2026
 *      Author: savo
 */

#ifndef MLX90614_H_
#define MLX90614_H_

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

struct MLXDevice {
	uint8_t address; // Address of device (not left shifted)
	I2C_HandleTypeDef* i2c_bus;
};

HAL_StatusTypeDef mlx_setup(struct MLXDevice target, float emissivity);
HAL_StatusTypeDef mlx_write_emissivity(struct MLXDevice target, float emissivity);
HAL_StatusTypeDef mlx_read_object_temperature_deg_c(struct MLXDevice target, float* destination_deg_c);
HAL_StatusTypeDef mlx_read_ambient_temperature_deg_c(struct MLXDevice target, float* destination_deg_c);
HAL_StatusTypeDef mlx_change_address(struct MLXDevice* target, uint8_t new_address);

#endif /* MLX90614_H_ */
