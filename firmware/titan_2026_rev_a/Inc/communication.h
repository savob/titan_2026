#ifndef TITAN_COMMUNICATION_HEADER
#define TITAN_COMMUNICATION_HEADER

#include <stdint.h>
#include "titan_data.h"

enum MessageStatus {
	MESSAGE_PARSED_OK_NO_RESPONSE,		// Parsed message successfully without a need to reply
	MESSAGE_PARSED_OK_SEND_RESPONSE,	// Parsed message and prepared response to send without issue
	MESSAGE_NOT_RECOGNIZED,				// Not recognized message type
	MESSAGE_PARSING_ISSUE,				// Issue parsing a recognized message type
	MESSAGE_RESPONSE_ISSUE,				// Issue crafting response to parsed message, likely buffer overflow
};

enum MessageStatus process_message(struct TitanSummary* summary, const uint8_t msg_in[], const uint8_t msg_in_length, uint8_t msg_out[], uint8_t msg_out_buf_size, uint16_t* length_to_send);

#endif
