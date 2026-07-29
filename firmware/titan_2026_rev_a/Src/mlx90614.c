/*
 * mlx90614.c
 *
 *  Created on: Jul 26, 2026
 *      Author: savo
 */

#include "mlx90614.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

enum MLXRegisterAddress {
	// RAM
	MLX_REG_RAWIR1     = 0x04,
	MLX_REG_RAWIR2     = 0x05,
	MLX_REG_TA         = 0x06,
	MLX_REG_TOBJ1      = 0x07,
	MLX_REG_TOBJ2      = 0x08,
	// EEPROM
	MLX_REG_TOMAX      = 0x20,
	MLX_REG_TOMIN      = 0x21,
	MLX_REG_PWMCTRL    = 0x22,
	MLX_REG_TARANGE    = 0x23,
	MLX_REG_EMISS      = 0x24,
	MLX_REG_CONFIG     = 0x25,
	MLX_REG_ADDR       = 0x2E,
	MLX_REG_ID1        = 0x3C,
	MLX_REG_ID2        = 0x3D,
	MLX_REG_ID3        = 0x3E,
	MLX_REG_ID4        = 0x3F,
};

static const uint32_t I2C_TIMEOUT = 1;

static uint8_t _crc8(uint8_t* addr, uint8_t len) {
	// The PEC calculation includes all bits except the START, REPEATED START, STOP,
	// ACK, and NACK bits. The PEC is a CRC-8 with polynomial X8+X2+X1+1.
	uint8_t crc = 0;
	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t carry = (crc ^ inbyte) & 0x80;
			crc <<= 1;
			if (carry) crc ^= 0x7;
			inbyte <<= 1;
		}
	}
	return crc;
}

static HAL_StatusTypeDef _write_16(struct MLXDevice target, uint8_t reg_address, uint16_t value) {
	uint8_t buffer[5];

	buffer[0] = target.address << 1;
	buffer[1] = reg_address;
	buffer[2] = value & 0xff;
	buffer[3] = value >> 8;
	buffer[4] = _crc8(buffer, 4);

	return HAL_I2C_Master_Transmit(target.i2c_bus, target.address << 1, &buffer[1], 4, I2C_TIMEOUT);
}

static HAL_StatusTypeDef _write_16_eeprom(struct MLXDevice target, uint8_t reg_address, uint16_t value) {
	HAL_StatusTypeDef ret = _write_16(target, reg_address, 0); // erase
	if (ret != HAL_OK) return ret;
	HAL_Delay(10);
	ret = _write_16(target, reg_address, value);
	if (ret != HAL_OK) return ret;

	HAL_Delay(10);
	return HAL_OK;
}

static HAL_StatusTypeDef _read_16(struct MLXDevice target, uint8_t reg_address, uint16_t* destination) {
	uint8_t buffer[3];

	uint16_t temp_address = reg_address;
	HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(target.i2c_bus, target.address << 1, temp_address, 1, buffer, 3, I2C_TIMEOUT);
	// TODO: Solve the write/reading here, we do need a repeated start

	if (ret != HAL_OK) {
		*destination = 0;
		return ret;
	}

	// Not bothering with PEC checking
	*destination = (uint16_t)(buffer[0]) | ((uint16_t)(buffer[1]) << 8);
	return HAL_OK;
}

HAL_StatusTypeDef mlx_setup(struct MLXDevice target, float emissivity) {
	HAL_StatusTypeDef ret = HAL_OK;

	const uint8_t TRIALS = 3;
	ret = HAL_I2C_IsDeviceReady(target.i2c_bus, target.address << 1, TRIALS, I2C_TIMEOUT);
	if (ret != HAL_OK) return ret;

	if (emissivity == 0) return ret;

	return mlx_write_emissivity(target, emissivity);
}

HAL_StatusTypeDef mlx_write_emissivity(struct MLXDevice target, float emissivity) {
	uint16_t ereg = (uint16_t)(0xffff * emissivity);

	return _write_16_eeprom(target, MLX_REG_EMISS, ereg);
}

static HAL_StatusTypeDef mlx_read_temperature_deg_c(struct MLXDevice target, float* destination_deg_c, uint8_t reg) {

	uint16_t value;
	HAL_StatusTypeDef ret = _read_16(target, reg, &value);
	if (ret != HAL_OK) {
		*destination_deg_c = 0;
		return ret;
	}

	if (value == 0) {
		*destination_deg_c = 0;
		return HAL_ERROR;
	}

	*destination_deg_c = ((float)value * .02) - 273.15;
	return HAL_OK;
}

HAL_StatusTypeDef mlx_read_object_temperature_deg_c(struct MLXDevice target, float* destination_deg_c) {
	return mlx_read_temperature_deg_c(target, destination_deg_c, MLX_REG_TOBJ1);
}

HAL_StatusTypeDef mlx_read_ambient_temperature_deg_c(struct MLXDevice target, float* destination_deg_c) {
	return mlx_read_temperature_deg_c(target, destination_deg_c, MLX_REG_TA);
}

HAL_StatusTypeDef mlx_change_address(struct MLXDevice* target, uint8_t new_address) {
	if ((new_address >= 0x80) || (new_address == 0x00)) return HAL_ERROR; // Return fail if out of range

	uint16_t original_address_value;
	HAL_StatusTypeDef ret = _read_16(*target, MLX_REG_ADDR, &original_address_value);
	if (ret != HAL_OK) return ret;

	original_address_value &= 0xFF00; // Mask out the old address preserving MSB
	original_address_value |= new_address;

	ret = _write_16_eeprom(*target, MLX_REG_ADDR, original_address_value);
	if (ret == HAL_OK) target->address = new_address;

	return ret;
}
