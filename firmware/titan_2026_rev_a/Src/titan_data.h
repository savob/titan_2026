/*
 * titan_data.h
 *
 *  Created on: Aug 1, 2026
 *      Author: savo
 */

#ifndef TITAN_DATA_H_
#define TITAN_DATA_H_

#include <stdint.h>

struct RiderBiometrics {
	int16_t heartrate_bpm;
	int16_t cadence_rpm;
	int16_t power_w;
};

struct WheelStatus {
	float speed_kmph;
	float brake_disk_temperature_c;
	uint16_t rotations;
};

struct GPSData {
	float latitude_deg;
	float longitude_deg;
	float speed_kmph;
	float altitude_m;

	float distance_from_start_km;
	float start_latitude_deg;
	float start_longitude_deg;
};

struct TitanSummary {
	int8_t primary_battery_soc;
	int8_t secondary_battery_soc;

	struct RiderBiometrics front_rider;
	struct RiderBiometrics rear_rider;

	struct WheelStatus front_wheel;
	struct WheelStatus rear_wheel;
	int32_t effective_rotations; // Allow upper level logic to decide what gets communicated as the rotations
	float effective_speed_kmph;

	struct GPSData gps;

	float temperature_c;
	int16_t co2_ppm;
	float humidity_percent;
};

#endif /* TITAN_DATA_H_ */
