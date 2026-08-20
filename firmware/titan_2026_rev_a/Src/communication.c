#include "titan_data.h"
#include "communication.h"
#include "main.h"
#include "minmea.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

struct bulkDataStruct {
	char messageType;
	char messageLength;
	uint16_t distGPS;;
	uint32_t speedEncoder __attribute__((packed));
	uint32_t speedGPS __attribute__((packed));
	uint16_t rotations __attribute__((packed));
	uint16_t frontBrakeT __attribute__((packed));
	uint16_t rearBrakeT __attribute__((packed));
	uint8_t fBatt;
	uint8_t rBatt;
	uint8_t humid;
	uint8_t temp;
	uint16_t CO2 __attribute__((packed));
	uint8_t fhr;
	uint8_t rhr;
	uint8_t fcad;
	uint8_t rcad;
	uint16_t fpwr;
	uint16_t rpwr;
};

static uint8_t LENGTH_CHARACTER_OFFSET = 31; // Basically shift non-zero values into ASCII printable character range, hold over from early days
static const uint8_t ATTEMPT_LIMIT = 3;
static const uint8_t ATTEMPT_PERIOD_MS = 3;

static uint8_t msg_length_to_bus(uint8_t length) {
	uint16_t temp = length + LENGTH_CHARACTER_OFFSET;
	if (temp > UINT8_MAX) return UINT8_MAX;
	return (uint8_t) temp;
}

static uint8_t bus_length_to_actual(uint8_t bus_length) {
	if (bus_length < LENGTH_CHARACTER_OFFSET) return 0;
	return (uint8_t) bus_length - LENGTH_CHARACTER_OFFSET;
}

static uint8_t temperature_in_message(float temperature_c) {
	return (temperature_c + 50.0) * 2;
}

static float temperature_from_message(uint8_t temperature_message) {
	float temp = temperature_message;
	return (temp / 2.0) - 50.0;
}

static uint8_t humidity_in_message(float humidity_percent) {
	return humidity_percent * 2;
}

static float humidity_from_message(uint8_t humidity_message) {
	float temp = humidity_message;
	return (temp / 2.0);
}

enum MessageStatus process_message(volatile struct TitanSummary* summary, const uint8_t msg_in[], const uint8_t msg_in_length, uint8_t msg_out[], uint8_t msg_out_buf_size, uint16_t* length_to_send) {
	/* Gets in a value for which line to process, then processes it.
	Requests characters are lowercase, setting is uppercase
	All floating point values are presented to four decimal points or less

	a - Heart rate (front) (BPM)
	b - Heart rate (rear) (BPM)
	c - Cadence (front) (RPM)
	d - Cadence (rear) (RPM)
	e - Power (front) (W)
	f - Power (rear) (W)
	g - All ANT data (delimited, in order a-f, zero padded to three digits from RPi but not from micro)

	i - Front battery %
	j - Rear battery %

	h - Humidity (R.H.%) (sent as machine encoded byte, NOT plaintext - RH% * 2)
	t - Temperature (C) (sent as machine encoded byte, NOT plaintext - (C + 50) * 2)
	k - CO2 (ppm)

	s - Speed (km/h)
	q - Distance (number of rotations)

	l - Latitude (degrees)
	m - Longitude (degrees)
	n - Altitude (m)
	o - GPS speed (km/h)
	p - GPS distance (km)
	u - Starting longitude (degrees)
	v - Starting latitude (degrees)

	w - Front brake temperature (C)
	x - Rear brake temperature (C)

	y - Testing byte
	z - Testing byte
	*/
	static uint8_t test_y = 0;	// Used solely for echo tests
	static uint8_t test_z = 0;	// Used solely for echo tests

	uint8_t message_in_type = msg_in[0];
	const uint8_t IN_PAYLOAD_LENGTH = (msg_in_length > 1) ? bus_length_to_actual(msg_in[1]) : 0;
	const char* IN_PAYLOAD = (IN_PAYLOAD_LENGTH > 0) ? (char*) &msg_in[2] : NULL;
	int32_t values_parsed = 0;

	int32_t out_payload_length = 0;
	char* out_payload = (char*) &msg_out[1];
	int32_t out_payload_allowed_length = msg_out_buf_size - 1; // Typical payload limit (to account for leading length character)

	*length_to_send = 0;

	// Handling the summary messages first since they're more common
	if (message_in_type == '{') {
		if (msg_out_buf_size < sizeof(struct bulkDataStruct)) return MESSAGE_RESPONSE_ISSUE;

		struct bulkDataStruct dataLoad = {0};
		dataLoad.messageType = '[';
		dataLoad.messageLength = sizeof(dataLoad) - 2 + 31;
		dataLoad.distGPS = summary->gps.distance_from_start_km * 1000;
		dataLoad.speedEncoder = summary->effective_speed_kmph * 1000;
		dataLoad.speedGPS = summary->gps.speed_kmph * 1000;
		dataLoad.rotations = summary->effective_rotations;
		dataLoad.frontBrakeT = summary->front_wheel.brake_disk_temperature_c * 100;
		dataLoad.rearBrakeT = summary->rear_wheel.brake_disk_temperature_c * 100;
		dataLoad.fBatt = summary->primary_battery_soc;
		dataLoad.rBatt = summary->secondary_battery_soc;
		dataLoad.humid = humidity_in_message(summary->humidity_percent);
		dataLoad.temp = temperature_in_message(summary->temperature_c);
		dataLoad.CO2 = summary->co2_ppm;
		dataLoad.fhr = summary->front_rider.heartrate_bpm;
		dataLoad.rhr = summary->rear_rider.heartrate_bpm;
		dataLoad.fcad = summary->front_rider.cadence_rpm;
		dataLoad.rcad = summary->rear_rider.cadence_rpm;
		dataLoad.fpwr = summary->front_rider.power_w;
		dataLoad.rpwr = summary->rear_rider.power_w;

		memcpy(msg_out, &dataLoad, sizeof(struct bulkDataStruct));
		*length_to_send = sizeof(struct bulkDataStruct);

		return MESSAGE_PARSED_OK_SEND_RESPONSE;
	}

	// Handle the less common piece-wsie message
	switch (message_in_type) {
	case 'a': // Heart rates
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%hd", summary->front_rider.heartrate_bpm);
		break;
	case 'A':
		values_parsed = sscanf(IN_PAYLOAD, "%hd", &summary->front_rider.heartrate_bpm);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'b':
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%hd", summary->rear_rider.heartrate_bpm);
		break;
	case 'B':
		values_parsed = sscanf(IN_PAYLOAD, "%hd", &summary->rear_rider.heartrate_bpm);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;

	case 'c': // Cadences
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%d", summary->front_rider.cadence_rpm);
		break;
	case 'C':
		values_parsed = sscanf(IN_PAYLOAD, "%hd", &summary->front_rider.cadence_rpm);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'd':
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%hd", summary->rear_rider.cadence_rpm);
		break;
	case 'D':
		values_parsed = sscanf(IN_PAYLOAD, "%hd", &summary->rear_rider.cadence_rpm);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;

	case 'e': // Powers
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%hd", summary->front_rider.power_w);
		break;
	case 'E':
		values_parsed = sscanf(IN_PAYLOAD, "%hd", &summary->front_rider.power_w);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'f':
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%hd", summary->rear_rider.power_w);
		break;
	case 'F':
		values_parsed = sscanf(IN_PAYLOAD, "%hd", &summary->rear_rider.power_w);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;

	case 'g':
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%hd,%hd,%hd,%hd,%hd,%hd",
				summary->front_rider.heartrate_bpm, summary->rear_rider.heartrate_bpm,
				summary->front_rider.cadence_rpm, summary->rear_rider.cadence_rpm,
				summary->front_rider.power_w, summary->rear_rider.power_w);
		break;
	case 'G':
		for (uint8_t i = 0; i < 6; i++) {
			int temp_int = 100 * (IN_PAYLOAD[i * 4] - '0');
			temp_int += 10 * (IN_PAYLOAD[i * 4 + 1] - '0');
			temp_int += IN_PAYLOAD[i * 4 + 2] - '0';

			switch (i) {
			case 0:
				summary->front_rider.heartrate_bpm = temp_int;
				break;
			case 1:
				summary->rear_rider.heartrate_bpm = temp_int;
				break;
			case 2:
				summary->front_rider.cadence_rpm = temp_int;
				break;
			case 3:
				summary->rear_rider.cadence_rpm = temp_int;
				break;
			case 4:
				summary->front_rider.power_w = temp_int;
				break;
			case 5:
				summary->rear_rider.power_w = temp_int;
				break;
			}
		}
		break;

	case 'i': // Batteries
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%hd", summary->primary_battery_soc);
		break;
	case 'I':
		values_parsed = sscanf(IN_PAYLOAD, "%hhd", &summary->primary_battery_soc);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'j':
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%hd", summary->secondary_battery_soc);
		break;
	case 'J':
		values_parsed = sscanf(IN_PAYLOAD, "%hhd", &summary->secondary_battery_soc);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;

	case 'k': // CO2
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%hd", summary->co2_ppm);
		break;
	case 'K':
		values_parsed = sscanf(IN_PAYLOAD, "%hd", &summary->co2_ppm);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'h': // Humidity
		if (out_payload_allowed_length < 1) return MESSAGE_RESPONSE_ISSUE;
		out_payload[0] = humidity_in_message(summary->humidity_percent);
		out_payload_length = 1;
		break;
	case 'H':
		if (IN_PAYLOAD_LENGTH < 1) return MESSAGE_PARSING_ISSUE;
		summary->humidity_percent = humidity_from_message(IN_PAYLOAD[0]);
		break;
	case 't': // Temperature
		if (out_payload_allowed_length < 1) return MESSAGE_RESPONSE_ISSUE;
		out_payload[0] = temperature_in_message(summary->temperature_c);
		out_payload_length = 1;
		break;
	case 'T':
		if (IN_PAYLOAD_LENGTH < 1) return MESSAGE_PARSING_ISSUE;
		summary->temperature_c = temperature_from_message(IN_PAYLOAD[0]);
		break;

	case 's': // Speed
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%.3f", summary->effective_speed_kmph);
		break;
	case 'S':
		values_parsed = sscanf(IN_PAYLOAD, "%f", &summary->effective_speed_kmph);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'q': // Distance
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%ld", summary->effective_rotations);
		break;
	case 'Q':
		values_parsed = sscanf(IN_PAYLOAD, "%ld", &summary->effective_rotations);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;

	case 'l': // Latitude
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%.4f", summary->gps.latitude_deg);
		break;
	case 'L':
		values_parsed = sscanf(IN_PAYLOAD, "%f", &summary->gps.latitude_deg);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'm': // Longitude
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%.4f", summary->gps.longitude_deg);
		break;
	case 'M':
		values_parsed = sscanf(IN_PAYLOAD, "%f", &summary->gps.longitude_deg);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'n': // Altitude
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%.1f", summary->gps.altitude_m);
		break;
	case 'N':
		values_parsed = sscanf(IN_PAYLOAD, "%f", &summary->gps.altitude_m);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'o': // Altitude
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%.3f", summary->gps.speed_kmph);
		break;
	case 'O':
		values_parsed = sscanf(IN_PAYLOAD, "%f", &summary->gps.speed_kmph);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'p': // Distance by GPS
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%.3f", summary->gps.distance_from_start_km);
		break;
	case 'P':
		values_parsed = sscanf(IN_PAYLOAD, "%f", &summary->gps.distance_from_start_km);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;

	case 'u': // Starting location
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%.4f", summary->gps.start_longitude_deg);
		break;
	case 'U':
		values_parsed = sscanf(IN_PAYLOAD, "%f", &summary->gps.start_longitude_deg);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'v':
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%.4f", summary->gps.start_latitude_deg);
		break;
	case 'V':
		values_parsed = sscanf(IN_PAYLOAD, "%f", &summary->gps.start_latitude_deg);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;

	case 'w': // Front Brake Temperature
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%.2f", summary->front_wheel.brake_disk_temperature_c);
		break;
	case 'W':
		values_parsed = sscanf(IN_PAYLOAD, "%f", &summary->front_wheel.brake_disk_temperature_c);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;
	case 'x': // Rear Brake Temperature
		out_payload_length = snprintf(out_payload, out_payload_allowed_length, "%.2f", summary->rear_wheel.brake_disk_temperature_c);
		break;
	case 'X':
		values_parsed = sscanf(IN_PAYLOAD, "%f", &summary->rear_wheel.brake_disk_temperature_c);
		if (values_parsed != 1) return MESSAGE_PARSING_ISSUE;
		break;

	case 'y':
		if (out_payload_allowed_length < 1) return MESSAGE_RESPONSE_ISSUE;
		out_payload[0] = test_y; // Test return
		out_payload_length = 1;
		break;
	case 'Y':
		if (IN_PAYLOAD_LENGTH < 1) return MESSAGE_PARSING_ISSUE;
		test_y = IN_PAYLOAD[0];
		break;
	case 'z':
		if (out_payload_allowed_length < 1) return MESSAGE_RESPONSE_ISSUE;
		out_payload[0] = test_z; // Test return
		out_payload_length = 1;
		break;
	case 'Z':
		if (IN_PAYLOAD_LENGTH < 1) return MESSAGE_PARSING_ISSUE;
		test_z = IN_PAYLOAD[0];
		break;

	default:
		return MESSAGE_NOT_RECOGNIZED;
	}

	if (out_payload_length == 0) return MESSAGE_PARSED_OK_NO_RESPONSE;

	if (out_payload_length > 0) {
		msg_out[0] = msg_length_to_bus(out_payload_length);
		*length_to_send = out_payload_length + 1; // Include the leading length character
		return MESSAGE_PARSED_OK_SEND_RESPONSE;
	}
	else if (out_payload_length < 0) {
		msg_out[0] = 0; // Null to ensure no message is sent
		return MESSAGE_RESPONSE_ISSUE;
	}
	return MESSAGE_PARSED_OK_NO_RESPONSE;
}

HAL_StatusTypeDef setup_interface(struct CommunicationInterface* interface) {
	HAL_StatusTypeDef ret = HAL_ERROR;


	switch (interface->type) {
	case INTERFACE_UART_GPS:
	case INTERFACE_UART_TITAN:
		if (interface->prime_read_function == NULL) return HAL_OK;

		int attempts = 0;
		for (; attempts < ATTEMPT_LIMIT && ret != HAL_OK; attempts++) {
			ret = interface->prime_read_function(interface->uart, (uint8_t*)interface->buffer_in, interface->buffer_in_length);
			if (ret != HAL_OK) HAL_Delay(ATTEMPT_PERIOD_MS);
		}

		if (ret != HAL_OK) {
			printf("Failed to start %s input buffer error code: %d\n\r", interface->name, ret);
			Error_Handler();
		}
		else if (attempts > 1){
			printf("Took %d attempts to start RX on %s\n\r", attempts, interface->name);
		}
		break;
	default:
		return HAL_ERROR; // Unimplemented
	}

	return ret;
}

HAL_StatusTypeDef operate_interface(struct CommunicationInterface* interface, volatile struct TitanSummary* summary) {
	if (*interface->available_bytes_to_read == 0) return HAL_OK;

	uint16_t length_to_send;
	enum MessageStatus status = MESSAGE_PARSED_OK_NO_RESPONSE;
	HAL_StatusTypeDef ret = HAL_ERROR; // Ensure we try RX/TX functions once by starting with non-OK

	switch (interface->type) {
	case INTERFACE_UART_TITAN:
		status = process_message(summary, (uint8_t*)interface->buffer_in, *interface->available_bytes_to_read, (uint8_t*)interface->buffer_out, interface->buffer_out_length, &length_to_send);
		break;
	case INTERFACE_UART_GPS:
		if (minmea_process_buffer(interface->buffer_in, (size_t)*interface->available_bytes_to_read, &(summary->gps))) {
			status = MESSAGE_PARSED_OK_NO_RESPONSE;
		}
		else status = MESSAGE_PARSING_ISSUE;
		break;
	default:
		return HAL_ERROR; // Unhandled
	}

	if (interface->prime_read_function != NULL) {
		// Clear received buffer and immediately reattempt receiving
		memset(interface->buffer_in, 0, *interface->available_bytes_to_read);
		*interface->available_bytes_to_read = 0;

		for (int attempts = 0; attempts < ATTEMPT_LIMIT && ret != HAL_OK; attempts++) {
			ret = interface->prime_read_function(interface->uart, (uint8_t*)interface->buffer_in, interface->buffer_in_length);
			if (ret != HAL_OK) HAL_Delay(ATTEMPT_PERIOD_MS);
		}
		if (ret != HAL_OK) {
			printf("Failed to start %s input buffer. Error code: %d\n\r", interface->name, ret);
			Error_Handler(); // Not restarting RX is critical
		}
	}

	switch (status) {
	case MESSAGE_PARSED_OK_NO_RESPONSE:
		return HAL_OK; // No further action needed
	case MESSAGE_PARSED_OK_SEND_RESPONSE:
		if (interface->send_function == NULL) {
#ifdef DEBUG
			printf("No TX function provided for %s\r\n", interface->name);
#endif
			return HAL_ERROR;
		}

		ret = HAL_ERROR; // Ensure we attempt a TX
		for (int attempts = 0; attempts < ATTEMPT_LIMIT && ret != HAL_OK; attempts++) {
			ret = interface->send_function(interface->uart, (uint8_t*)interface->buffer_out, length_to_send);
			if (ret != HAL_OK) HAL_Delay(ATTEMPT_PERIOD_MS);
		}
#ifndef DEBUG
		return ret;
	default:
		return HAL_ERROR;
#else
		if (ret != HAL_OK) {
			printf("Failed to send %s output buffer error code: %d\n\r", interface->name, ret);
		}
		return ret;
	case MESSAGE_NOT_RECOGNIZED:
		printf("Failed to recognize message type \'%c\' in \"%s\" from %s\r\n", interface->buffer_in[0], interface->buffer_in, interface->name);
		return HAL_ERROR;
	case MESSAGE_PARSING_ISSUE:
		printf("Failed to parse \"%s\" for message type \'%c\' from %s\r\n", &interface->buffer_in[1], interface->buffer_in[0], interface->name);
		return HAL_ERROR;
	case MESSAGE_RESPONSE_ISSUE:
		printf("Failed to prepare response to message type \'%c\' for %s\r\n", interface->buffer_in[0], interface->name);
		return HAL_ERROR;
	default:
		printf("Error %d: processing \"%s\" from %s\r\n", status, interface->buffer_in, interface->name);
		return HAL_ERROR;
#endif
	}
	return ret;
}
