/*
 * wheels.c
 *
 *  Created on: Aug 2, 2026
 *      Author: savo
 */
#include "main.h"
#include <math.h>
#include "titan_data.h"
#include "cmsis_gcc.h" // Needed to disable and enable interrupts
#include <stdlib.h>
#include "mlx90614.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>

static const float BRAKE_DISK_EMISSIVITY = 0.9; // Should be between 0 and 1.0

// MLX sensors default to 0x5A (90)
static const uint8_t FRONT_ADDRESS = 0x01;
static const uint8_t REAR_ADDRESS = 0x02;
static struct MLXDevice front_brake;
static struct MLXDevice rear_brake;

static bool rear_active = false;
static bool front_active = false;

HAL_StatusTypeDef setup_brake_disk_sensors(I2C_HandleTypeDef* bus, uint32_t timeout) {
	UNUSED(timeout);

	HAL_StatusTypeDef ret_overall = HAL_OK;

	front_brake.address = FRONT_ADDRESS;
	front_brake.i2c_bus = bus;
	rear_brake.address = REAR_ADDRESS;
	rear_brake.i2c_bus = bus;

	HAL_StatusTypeDef ret = mlx_setup(front_brake, BRAKE_DISK_EMISSIVITY);
	if (ret == HAL_OK) {
		front_active = true;
		printf("Front brake disk setup success at address 0x%02X\n\r", front_brake.address);
	}
	else {
		front_active = false;
		ret_overall = HAL_ERROR;
		printf("Front brake disk setup failed, error code: %d (Address: 0x%02X)\n\r", ret, front_brake.address);
	}

	ret = mlx_setup(rear_brake, BRAKE_DISK_EMISSIVITY);
	if (ret == HAL_OK) {
		rear_active = true;
		printf("Rear brake disk setup success at address 0x%02X\n\r", rear_brake.address);
	}
	else {
		rear_active = false;
		ret_overall = HAL_ERROR;
		printf("Rear brake disk setup failed, error code: %d (Address: 0x%02X)\n\r", ret, rear_brake.address);
	}

	return ret_overall;
}

HAL_StatusTypeDef operate_brake_disk_sensors(float* front_temp_c, float* rear_temp_c) {
	static uint32_t next_tick_mark = 0;
	const uint32_t PERIOD_MS = 100;
	if (HAL_GetTick() < next_tick_mark) return HAL_OK;
	next_tick_mark = HAL_GetTick() + PERIOD_MS;

	HAL_StatusTypeDef ret_overall = HAL_OK;
	HAL_StatusTypeDef ret = HAL_OK;

	if (front_active) {
		ret = mlx_read_object_temperature_deg_c(front_brake, front_temp_c);
		if (ret != HAL_OK) ret_overall = ret;
	}
	else *front_temp_c = 0;

	if (rear_active) {
		ret = mlx_read_object_temperature_deg_c(rear_brake, rear_temp_c);
		if (ret != HAL_OK) ret_overall = ret;
	}
	else *rear_temp_c = 0;

	return ret_overall;
}

void summarize_wheel_data(volatile struct TitanSummary* summary) {
	// Generally use average, but if delta grows too large then trust larger value since one sensor may be disconnected
	// Since these values are volatile and change frequently they all need to be read quickly and cached for accurate comparison

	__disable_irq();
	int32_t front_rotations = summary->front_wheel.rotations;
	int32_t rear_rotations = summary->rear_wheel.rotations;
	float front_speed = summary->front_wheel.speed_kmph;
	float rear_speed = summary->rear_wheel.speed_kmph;
	__enable_irq();

	const int32_t ROTATION_TOLERANCE = 50;
	int32_t delta_rotations = front_rotations - rear_rotations;
	if (delta_rotations < abs(ROTATION_TOLERANCE)) summary->effective_rotations = (front_rotations + rear_rotations) >> 2; // Shift to speed up division
	else {
		if (front_rotations > rear_rotations) summary->effective_rotations = front_rotations;
		else summary->effective_rotations = rear_rotations;
	}

	const float SPEED_TOLERANCE_KMPH = 3.0;
	float delta_speed = front_speed - rear_speed;
	if (fabsf(delta_speed) < SPEED_TOLERANCE_KMPH) summary->effective_speed_kmph = (front_speed + rear_speed) / 2.0;
	else {
		if (front_speed > rear_speed) summary->effective_speed_kmph = front_speed;
		else summary->effective_speed_kmph = rear_speed;
	}
}
