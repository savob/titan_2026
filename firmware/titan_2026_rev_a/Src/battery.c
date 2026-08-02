/*
 * battery.c
 *
 *  Created on: Aug 2, 2026
 *      Author: savo
 */
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"
#include <stdio.h>
#include "ina219.h"

// Must be left shifted before use in I2C HAL
const uint8_t PRIM_INA = 0x40 << 1; // Address for primary board's INA219
const uint8_t SEC_INA = 0x44 << 1;	// Address for secondary board's INA219


#define countof(x) (sizeof(x)/sizeof(x[0]))

static int8_t stack_voltage_to_soc(float stack_voltage) {

	const uint8_t level_marks[] = {100, 99, 90, 70, 40, 30, 20, 17, 14, 9, 0}; // Percentages linked to voltages
	const float voltage_marks[] = {3.6, 3.35, 3.325, 3.3, 3.275, 3.25, 3.225, 3.2, 3.125, 3, 2.5}; // Cell voltages

	const float CELL_COUNT = 3.0;
	float cell_voltage = stack_voltage / CELL_COUNT;

	if (cell_voltage < voltage_marks[countof(voltage_marks) - 1]) return 0;

	// Run though set point from
	for (uint8_t i = 1; i <= countof(voltage_marks); i++) {
		if (cell_voltage > voltage_marks[i]) {
			// If the reading is in the region
			float temp = 0; // Used in calculations

			// Linear interpolation formula
			temp = (cell_voltage - voltage_marks[i]) * (float)(level_marks[i - 1] - level_marks[i]);
			temp /= (float)(voltage_marks[i - 1] - voltage_marks[i]);
			temp += level_marks[i];

			cell_voltage = temp; // Stores the result in the reading variable
			break;
		}
	}

	if (cell_voltage > 100) return 100;
	return (uint8_t) cell_voltage;
}

HAL_StatusTypeDef setup_battery_monitoring(I2C_HandleTypeDef* bus, uint32_t timeout) {
	HAL_StatusTypeDef ret_overall, ret;
	ret_overall = HAL_OK;

	ret = ina219_setup(PRIM_INA, bus, timeout);
	if (ret != HAL_OK) {
#ifdef DEBUG
		printf("Primary INA219 setup error: %d\n\r", ret);
#endif
		ret_overall = ret;
	}
	ret = ina219_setup(SEC_INA, bus, timeout);
	if (ret != HAL_OK) {
#ifdef DEBUG
		printf("Secondary INA219 setup error: %d\n\r", ret);
#endif
		ret_overall = ret;
	}

	return ret_overall;
}

HAL_StatusTypeDef read_battery_level(int8_t* primary_soc, int8_t* secondary_soc) {
	static uint32_t next_tick_mark = 0;
	const uint32_t PERIOD_MS = 250;
	if (HAL_GetTick() < next_tick_mark) return HAL_OK;
	next_tick_mark = HAL_GetTick() + PERIOD_MS;

	HAL_StatusTypeDef ret_overall, ret;
	ret_overall = HAL_OK;

	float voltage;

	ret = ina219_read_bus_voltage(PRIM_INA, &voltage);
	if (ret == HAL_OK) {
		*primary_soc = stack_voltage_to_soc(voltage);
	}
	else {
#ifdef DEBUG
		printf("Primary INA219 read error: %d\n\r", ret);
#endif
		ret_overall = ret;
	}

	ret = ina219_read_bus_voltage(SEC_INA, &voltage);
	if (ret == HAL_OK) {
		*secondary_soc = stack_voltage_to_soc(voltage);
	}
	else {
#ifdef DEBUG
		printf("Secondary INA219 read error: %d\n\r", ret);
#endif
		ret_overall = ret;
	}

	return ret_overall;
}
