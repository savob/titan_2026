/*
 * wheels.c
 *
 *  Created on: Aug 2, 2026
 *      Author: savo
 */
#include <math.h>
#include "titan_data.h"
#include "cmsis_gcc.h" // Needed to disable and enable interrupts
#include <stdlib.h>

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
