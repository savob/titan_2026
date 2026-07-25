/*
 * ina219.c
 *
 *  Created on: Nov 9, 2025
 *      Author: savo
 */

#include "ina219.h"
#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

enum INA219RegAdd {
	INA_REG_CONFIG			= 0x00,
	INA_REG_SHUNT_VOLTAGE	= 0x01,
	INA_REG_BUS_VOLTAGE		= 0x02,
	INA_REG_POWER			= 0x03,
	INA_REG_CURRENT 		= 0x04,
	INA_REG_CALIBRATION		= 0x05,
	INA_REG_INVALID
};

enum INA219Mode {
	INA_MODE_POWER_DOWN	= 0x00,
	INA_MODE_SHUNT_TRIG	= 0x01,
	INA_MODE_BUS_V_TRIG	= 0x02,
	INA_MODE_BOTH_TRIG	= 0x03,
	INA_MODE_ADC_OFF	= 0x04,
	INA_MODE_SHUNT_CONT	= 0x05,
	INA_MODE_BUS_V_CONT	= 0x06,
	INA_MODE_BOTH_CONT	= 0x07,
};

enum INA219ADCSetting {
	INA_ADC_9_BIT		= 0x00,
	INA_ADC_10_BIT		= 0x01,
	INA_ADC_11_BIT		= 0x02,
	INA_ADC_12_BIT		= 0x03,
	INA_ADC_2_SAMPLES	= 0x09,
	INA_ADC_4_SAMPLES	= 0x0A,
	INA_ADC_8_SAMPLES	= 0x0B,
	INA_ADC_16_SAMPLES	= 0x0C,
	INA_ADC_32_SAMPLES	= 0x0D,
	INA_ADC_64_SAMPLES	= 0x0E,
	INA_ADC_128_SAMPLES	= 0x0F,
};

enum INA219ShuntRange {
	INA_SHUNT_RANGE_40_MV	= 0x00,
	INA_SHUNT_RANGE_80_MV	= 0x01,
	INA_SHUNT_RANGE_160_MV	= 0x02,
	INA_SHUNT_RANGE_320_MV	= 0x03,
};

enum INA219BusRange {
	INA_BUS_RANGE_16_V	= 0x00,
	INA_BUS_RANGE_32_V	= 0x01,
};

union INA219ConfigReg {
	uint16_t entire_register;
	struct {
		enum INA219Mode mode : 3;
		enum INA219ADCSetting shunt_adc_setting : 4;
		enum INA219ADCSetting bus_adc_setting : 4;
		enum INA219ShuntRange shunt_range : 2;
		enum INA219BusRange bus_range : 1;
		uint8_t RESERVED : 1;
		bool reset : 1;
	};

};

static enum INA219RegAdd last_reg_pointed_to = INA_REG_INVALID; // Used to speed up repeated reads

// Must be left shifted before use in I2C HAL
const uint8_t PRIM_INA = 0x40 << 1; // Address for primary board's INA219
const uint8_t SEC_INA = 0x44 << 1;	// Address for secondary board's INA219

static I2C_HandleTypeDef* ina219_i2c;
static uint32_t ina219_timeout = 1000;
static float current_lsb_a = 0; // Calculated value of current per LSB in register

static const float SHUNT_RESISTANCE_OHM = 0.01;
static const float EXPECTED_MAX_CURRENT_A = 5.0;

static HAL_StatusTypeDef ina219_read_register(const uint8_t INA_ADD, enum INA219RegAdd target, uint16_t* destination) {
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t temp_buffer[2];

	// Write register address/pointer as needed
	if (last_reg_pointed_to != target) {
		temp_buffer[0] = target;
		ret = HAL_I2C_Master_Transmit(ina219_i2c, INA_ADD, temp_buffer, 1, ina219_timeout);
		if (ret != HAL_OK) return ret;
		last_reg_pointed_to = target; // Only update on successful transfers
	}

	// Get data
	ret = HAL_I2C_Master_Receive(ina219_i2c, INA_ADD, temp_buffer, 2, ina219_timeout);
	if (ret != HAL_OK) return ret;

	uint16_t reconstructed;
	reconstructed = (temp_buffer[0] << 8);
	reconstructed |= (temp_buffer[1] << 0);

	*destination = reconstructed;
	return HAL_OK;
}

static HAL_StatusTypeDef ina219_write_register(const uint8_t INA_ADD, enum INA219RegAdd target, uint16_t value) {
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t temp_buffer[3];
	temp_buffer[0] = target;
	temp_buffer[1] = (uint8_t) (value >> 8) & 0xFF;
	temp_buffer[2] = (uint8_t) (value >> 0) & 0xFF;
	ret = HAL_I2C_Master_Transmit(ina219_i2c, INA_ADD, temp_buffer, 3, ina219_timeout);

	if (ret != HAL_OK) return ret;
	last_reg_pointed_to = target; // Only update on successful transfers

	return HAL_OK;
}

HAL_StatusTypeDef ina219_update_calibration(const uint8_t INA_ADD, float expected_max_current_a, float shunt_resistance_ohm) {
	// All this math is from 8.5.1 in the data sheet
	current_lsb_a = expected_max_current_a / 32768.0;

	float calibration_reg = 0.04096 / (current_lsb_a * shunt_resistance_ohm);

	uint16_t truncated_calibration_reg = (uint16_t) calibration_reg;
	truncated_calibration_reg = truncated_calibration_reg & 0xFFFE; // Last bit cannot be set, see 8.6.4.1 in data sheet

	return ina219_write_register(INA_ADD, INA_REG_CALIBRATION, truncated_calibration_reg);
}

HAL_StatusTypeDef ina219_setup(const uint8_t INA_ADD, I2C_HandleTypeDef* bus, uint32_t timeout) {
	ina219_i2c = bus;
	ina219_timeout = timeout;

	const uint32_t TRIAL_COUNT = 5;

	HAL_StatusTypeDef ret = HAL_OK;
	ret = HAL_I2C_IsDeviceReady(ina219_i2c, INA_ADD, TRIAL_COUNT, ina219_timeout);
	if (ret != HAL_OK) return ret;

	union INA219ConfigReg default_config;

	default_config.reset = false;
	default_config.mode = INA_MODE_BOTH_CONT;
	default_config.bus_range = INA_BUS_RANGE_16_V; // Motherboard won't survive past 17 V so no need to really go to 32 V
	default_config.bus_adc_setting = INA_ADC_12_BIT;
	default_config.shunt_range = INA_SHUNT_RANGE_80_MV;
	default_config.shunt_adc_setting = INA_ADC_12_BIT;
	default_config.RESERVED = 0;

	ret = ina219_write_register(INA_ADD, INA_REG_CONFIG, default_config.entire_register);
	if (ret != HAL_OK) return ret;

	return ina219_update_calibration(INA_ADD, EXPECTED_MAX_CURRENT_A, SHUNT_RESISTANCE_OHM);
}

HAL_StatusTypeDef ina219_read_power(const uint8_t INA_ADD, float * result_w) {
	HAL_StatusTypeDef ret;
	uint16_t raw_reading;
	ret = ina219_read_register(INA_ADD, INA_REG_POWER, &raw_reading);
	if (ret != HAL_OK) return ret;

	*result_w = raw_reading * (current_lsb_a * 20.0);

	return HAL_OK;
}

HAL_StatusTypeDef ina219_read_bus_voltage(const uint8_t INA_ADD, float * result_v) {
	HAL_StatusTypeDef ret;
	uint16_t raw_reading;
	ret = ina219_read_register(INA_ADD, INA_REG_BUS_VOLTAGE, &raw_reading);
	if (ret != HAL_OK) return ret;

	// We don't really make use of these flags but here's them getting collected
	bool math_overflow_error = (raw_reading & 0x01) != 0;
	bool conversion_ready = (raw_reading & 0x02) != 0;

	// Convert voltage as per data sheet
	raw_reading = (raw_reading >> 3) & 0x3FFF; // Ensure we don't accidentally extend the "sign" with a leading 1
	*result_v = raw_reading * 0.004;

	return HAL_OK;
}

HAL_StatusTypeDef ina219_read_bus_current(const uint8_t INA_ADD, float * result_a) {
	HAL_StatusTypeDef ret;
	uint16_t raw_reading;
	ret = ina219_read_register(INA_ADD, INA_REG_CURRENT, &raw_reading);
	if (ret != HAL_OK) return ret;

	*result_a = raw_reading * current_lsb_a;

	return HAL_OK;
}
