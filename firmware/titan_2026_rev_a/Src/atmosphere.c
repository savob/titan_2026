/*
 * atmosphere.c
 *
 *  Created on: Nov 12, 2025
 *      Author: savo
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "atmosphere.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

#include "bosch_wrappers.h"
#include "../../BME280_SensorAPI/bme280.h"
#include "../../BME280_SensorAPI/bme280_defs.h"
#include "titan_data.h"
#include "scd4x.h"
#include <stdbool.h>

static bool scd_available = false;

static const uint8_t BME280_ADDRESS = 0x77;
static struct BoschI2C bme280_interface;
static struct bme280_dev dev_bme280 = {
		.chip_id = BME280_CHIP_ID,
		.intf = BME280_I2C_INTF,
		.intf_ptr = &bme280_interface,
		.read = bosch_read_i2c_by_intf,
		.write = bosch_write_i2c_by_intf,
		.delay_us = bosch_delay_us,
};

HAL_StatusTypeDef atmo_setup(I2C_HandleTypeDef* bus, uint32_t i2c_timeout_ms) {
	bool any_error = false;

	HAL_StatusTypeDef ret;

	ret = scd4x_setup(bus, i2c_timeout_ms);
	scd_available = ret == HAL_OK;
	if (ret == HAL_OK) printf("SCD41 setup success\n\r");
	else {
		printf("SCD41 setup failed, error code: %d. SCD sensor will NOT be used for CO2.\n\r", ret);
		// Not bothering to mark this as an error because it isn't severe, likely using off board sensor
	}

	bosch_adjust_i2c_timeout(i2c_timeout_ms);
	bme280_interface.i2c_handle = bus;
	bme280_interface.address = BME280_ADDRESS;

	uint8_t bme_init = bme280_init(&dev_bme280);
	if (bme_init < 0) {
#ifdef DEBUG
		printf("BME280 setup issue: %d. Further BME280 configuration skipped.\n\r", bme_init);
#endif
		any_error = true;
	}
	else {
		// Only bother with setup if chip initialized ok
		struct bme280_settings settings = {
				.osr_p = BME280_OVERSAMPLING_4X,
				.osr_t = BME280_OVERSAMPLING_4X,
				.osr_h = BME280_OVERSAMPLING_4X,
				.filter = BME280_FILTER_COEFF_4,
				.standby_time= BME280_STANDBY_TIME_0_5_MS,
		};
		bme_init = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &dev_bme280);
		if (bme_init < 0) {
#ifdef DEBUG
			printf("BME280 settings issue: %d\n\r", bme_init);
#endif
			any_error = true;
		}
		bme_init = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev_bme280);
		if (bme_init < 0) {
#ifdef DEBUG
			printf("BME280 mode issue: %d\n\r", bme_init);
#endif
			any_error = true;
		}
	}

	if (any_error) return HAL_ERROR;
	return HAL_OK;
}

HAL_StatusTypeDef atmo_conditions_update(struct TitanSummary* target, uint16_t mh_z19_co2_ppm) {
	static uint32_t next_tick_mark = 0;
	const uint32_t PERIOD_MS = 250;
	if (HAL_GetTick() < next_tick_mark) return HAL_OK;
	next_tick_mark = HAL_GetTick() + PERIOD_MS;

	bool any_error = false; // Used to keep track if any interface has errors

    struct bme280_data bme_data;
    int8_t ret_bme280 = bme280_get_sensor_data(BME280_ALL, &bme_data, &dev_bme280);
    if (ret_bme280 == BME280_OK) {

		target->humidity_percent = bme_data.humidity;
		target->pressure_pa = bme_data.pressure;
		target->temperature_c = bme_data.temperature;
    }
    else if (ret_bme280 < 0) {
		any_error = true;
#ifdef DEBUG
		printf("BME280 read error: %d\r\n", ret_bme280);
#endif
	}
#ifdef DEBUG
	else printf("BME280 read warning: %d\r\n", ret_bme280);
#endif

	// Gather combined CO2 reading
	if (scd_available) {
		HAL_StatusTypeDef ret_scd4x = HAL_OK;
		ret_scd4x = scd4x_read_co2(&target->co2_ppm);
		if (ret_scd4x != HAL_OK) {
			printf("CO2 update issue: %d\n\r", ret_scd4x);
			any_error = true;
		}
	}
	if (mh_z19_co2_ppm > 0) { // Include MH sensor reading too if valid
		if (target->co2_ppm <= 0) target->co2_ppm = (uint16_t)mh_z19_co2_ppm;
		else target->co2_ppm = (target->co2_ppm + (uint16_t)mh_z19_co2_ppm) / 2;
	}

#ifdef DEBUG
	printf("[%d] T: %.2f\tH: %.2f\tP: %.2f\tCO2: %d\n\r", any_error, target->temperature_c, target->humidity_percent, target->pressure_pa, target->co2_ppm);
#endif

    if (any_error) return HAL_ERROR;
    return HAL_OK;
}
