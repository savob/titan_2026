#ifndef TITAN_COMMUNICATION_HEADER
#define TITAN_COMMUNICATION_HEADER

#include <stdint.h>
#include "titan_data.h"
#include <stm32f1xx_hal.h>

enum MessageStatus {
	MESSAGE_PARSED_OK_NO_RESPONSE,		// Parsed message successfully without a need to reply
	MESSAGE_PARSED_OK_SEND_RESPONSE,	// Parsed message and prepared response to send without issue
	MESSAGE_NOT_RECOGNIZED,				// Not recognized message type
	MESSAGE_PARSING_ISSUE,				// Issue parsing a recognized message type
	MESSAGE_RESPONSE_ISSUE,				// Issue crafting response to parsed message, likely buffer overflow
};

enum InterfaceType {
	INTERFACE_UART_TITAN,
	INTERFACE_UART_GPS,
	INTERFACE_RADIO
};

struct CommunicationInterface {
	enum InterfaceType type;
	char name[10]; // Store name for nicer debug messages

	char* buffer_in;
	uint16_t buffer_in_length;
	volatile uint16_t* available_bytes_to_read;

	char* buffer_out;
	uint16_t buffer_out_length;

	UART_HandleTypeDef* uart;
	HAL_StatusTypeDef (*send_function)(UART_HandleTypeDef *, const uint8_t *, uint16_t);
	HAL_StatusTypeDef (*prime_read_function)(UART_HandleTypeDef *, uint8_t *, uint16_t);
};

enum MessageStatus process_message(volatile struct TitanSummary* summary, const uint8_t msg_in[], const uint8_t msg_in_length, uint8_t msg_out[], uint8_t msg_out_buf_size, uint16_t* length_to_send);

HAL_StatusTypeDef setup_interface(struct CommunicationInterface* interface);
HAL_StatusTypeDef operate_interface(struct CommunicationInterface* interface, volatile struct TitanSummary* summary);
#endif
