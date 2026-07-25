/*
 * scd4x.c
 *
 *  Created on: Nov 9, 2025
 *      Author: savo
 */

#include "scd4x.h"
#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

enum SCD4XCommands {
	SCD_CMD_START_PERIODIC_MEASUREMENT						= 0x21B1,
	SCD_CMD_READ_MEASUREMENT								= 0xEC05,
	SCD_CMD_STOP_PERIODIC_MEASUREMENT						= 0x3F86,
	SCD_CMD_SET_TEMPERATURE_OFFSET							= 0x241D,
	SCD_CMD_GET_TEMPERATURE_OFFSET							= 0x2318,
	SCD_CMD_SET_SENSOR_ALTITUDE								= 0x2427,
	SCD_CMD_GET_SENSOR_ALTITUDE								= 0x2322,
	SCD_CMD_GET_SET_AMBIENT_PRESSURE						= 0xE000,
	SCD_CMD_PERFORM_FORCED_CALIBRATION						= 0x362F,
	SCD_CMD_SET_AUTO_SELF_CALIBRATION_ENABLED				= 0x2416,
	SCD_CMD_GET_AUTO_SELF_CALIBRATION_ENABLED				= 0x2313,
	SCD_CMD_SET_AUTO_SELF_CALIBRATION_TARGET				= 0x243A,
	SCD_CMD_GET_AUTO_SELF_CALIBRATION_TARGET				= 0x233F,
	SCD_CMD_START_LOW_POWER_PERIODIC_MEASUREMENT			= 0x21AC,
	SCD_CMD_GET_DATA_READY_STATUS							= 0xE4B8,
	SCD_CMD_PERSIST_SETTINGS								= 0x3615,
	SCD_CMD_GET_SERIAL_NUMBER								= 0x3682,
	SCD_CMD_PERFORM_SELF_TEST								= 0x3639,
	SCD_CMD_PERFORM_FACTORY_RESET							= 0x3632,
	SCD_CMD_REINIT											= 0x3646,
	SCD_CMD_GET_SENSOR_VARIANT								= 0x202F,
	SCD_CMD_MEASURE_SINGLE_SHOT								= 0x219D,
	SCD_CMD_MEASURE_SINGLE_SHOT_RHT_ONLY					= 0x2196,
	SCD_CMD_POWER_DOWN										= 0x36E0,
	SCD_CMD_WAKE_UP											= 0x36F6,
	SCD_CMD_SET_AUTOMATIC_SELF_CALIBRATION_INITIAL_PERIOD	= 0x2445,
	SCD_CMD_GET_AUTOMATIC_SELF_CALIBRATION_INITIAL_PERIOD	= 0x2340,
	SCD_CMD_SET_AUTOMATIC_SELF_CALIBRATION_STANDARD_PERIOD	= 0x244E,
	SCD_CMD_GET_AUTOMATIC_SELF_CALIBRATION_STANDARD_PERIOD	= 0x234B,
	SCD_CMD_INVALID
};


static const uint8_t SCD4X_ADD = 0x62 << 1; // Hard wired address. Must be left shifted before use in I2C HAL.
static I2C_HandleTypeDef* scd4x_i2c;
static uint32_t scd4x_timeout = 100;

static uint8_t scd4x_get_crc(uint8_t data[2]) {
	uint8_t crc = 0xFF;
	for(int i = 0; i < 2; i++) {
		crc ^= data[i];
		for(uint8_t bit = 8; bit > 0; --bit) {
			if(crc & 0x80) {
				crc = (crc << 1) ^ 0x31u;
			}
			else crc = (crc << 1);
		}
	}
	return crc;
}

static HAL_StatusTypeDef scd4x_issue_command(enum SCD4XCommands cmd) {
	uint8_t buffer[2];
	buffer[0] = (cmd >> 8) & 0xFF;
	buffer[1] = (cmd >> 0) & 0xFF;
	return HAL_I2C_Master_Transmit(scd4x_i2c, SCD4X_ADD, buffer, 2, scd4x_timeout);
}

HAL_StatusTypeDef scd4x_setup(I2C_HandleTypeDef* bus, uint32_t timeout) {
	scd4x_i2c = bus;
	scd4x_timeout = timeout;

	const uint32_t TRIAL_COUNT = 5;

	HAL_StatusTypeDef ret = HAL_OK;
	ret = HAL_I2C_IsDeviceReady(scd4x_i2c, SCD4X_ADD, TRIAL_COUNT, scd4x_timeout);
	if (ret != HAL_OK) return ret;

	// Need to stop periodic measurements before potentially restarting them
	// This is to prevent issues when rebooting MCU but not the sensor itself
	ret = scd4x_issue_command(SCD_CMD_STOP_PERIODIC_MEASUREMENT);
	if (ret != HAL_OK) return ret;
	HAL_Delay(10); // Need to allow sensor to process command

	return scd4x_issue_command(SCD_CMD_START_PERIODIC_MEASUREMENT);
}

HAL_StatusTypeDef scd4x_read_co2(uint16_t* result_ppm) {
	static uint16_t last_reading_ppm = 0;

	// Don't waste time reading data that won't be updated since it updates once every 5 seconds
	const uint32_t MEASUREMENT_PERIOD_TICKS = 5000; // 5000 ms equivalent
	static uint32_t next_mark_ticks = 0;
	uint32_t current_tick = HAL_GetTick();

	if (current_tick < next_mark_ticks) {
		*result_ppm = last_reading_ppm;
		return HAL_OK;
	}
	next_mark_ticks = current_tick + MEASUREMENT_PERIOD_TICKS;

	HAL_StatusTypeDef ret = HAL_OK;
	ret = scd4x_issue_command(SCD_CMD_READ_MEASUREMENT);
	if (ret != HAL_OK) return ret;

#define BYTES_TO_READ 3 // 3 is needed for CO2 and its CRC, 9 to get temperature and humidity too
	uint8_t temp_buffer[BYTES_TO_READ];
	ret = HAL_I2C_Master_Receive(scd4x_i2c, SCD4X_ADD, temp_buffer, BYTES_TO_READ, scd4x_timeout);
	if (ret != HAL_OK) return ret;
#undef BYTES_TO_READ

	// Default to previous reading on CRC failure
	if (temp_buffer[2] != scd4x_get_crc(&temp_buffer[0])) {
		*result_ppm = last_reading_ppm;
		return HAL_ERROR;
	}

	last_reading_ppm = (temp_buffer[0] << 8) | temp_buffer[1];
	*result_ppm = last_reading_ppm;

	return HAL_OK;
}
