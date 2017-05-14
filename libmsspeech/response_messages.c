/*

Copyright 2017 technicianted

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

*/

#include <ctype.h>
#include <math.h>
#include <strings.h>

#include "compat.h"
#include "response_messages.h"
#include "message_constants.h"
#include "response_messages_priv.h"
#include "ms_speech_logging_priv.h"
#include "ms_speech_telemetry.h"

static int ms_speech_handle_speech_startdetected(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message);
static int ms_speech_handle_speech_enddetected(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message);
static int ms_speech_handle_speech_hypothesis(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message);
static int ms_speech_handle_speech_result(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message);
static int ms_speech_handle_turn_start(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message);
static int ms_speech_handle_turn_end(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message);

static int ms_speech_parse_response_message(ms_speech_connection_t connection, void *buffer, size_t len, ms_speech_parsed_message_t **parsed_message);
static void ms_speech_destroy_parsed_message(ms_speech_parsed_message_t *parsed_message);
static char * trim_string(char *string);

int ms_speech_handle_resonse_message(ms_speech_connection_t connection, void *buffer, size_t len)
{
	ms_speech_parsed_message_t *parsed_message = NULL;
	int r = ms_speech_parse_response_message(connection, buffer, len, &parsed_message);
	if (r == -EAGAIN) {
		// partial message, return success
		return 0;
	} else if (r) {
		return r;
	}

	ms_speech_telemetry_handle_response_message(connection, parsed_message);
	
	if (!strcasecmp(parsed_message->path, MS_SPEECH_MESSAGE_PATH_SPEECH_STARTDETECTED))
		r = ms_speech_handle_speech_startdetected(connection, parsed_message);
	else if (!strcasecmp(parsed_message->path, MS_SPEECH_MESSAGE_PATH_SPEECH_ENDDETECTED))
		r = ms_speech_handle_speech_enddetected(connection, parsed_message);
	else if (!strcasecmp(parsed_message->path, MS_SPEECH_MESSAGE_PATH_SPEECH_HYPOTHESIS))
		r = ms_speech_handle_speech_hypothesis(connection, parsed_message);
	else if (!strcasecmp(parsed_message->path, MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE))
		r = ms_speech_handle_speech_result(connection, parsed_message);
	else if (!strcasecmp(parsed_message->path, MS_SPEECH_MESSAGE_PATH_TURN_START))
		r = ms_speech_handle_turn_start(connection, parsed_message);
	else if (!strcasecmp(parsed_message->path, MS_SPEECH_MESSAGE_PATH_TURN_END))
		r = ms_speech_handle_turn_end(connection, parsed_message);
	
	ms_speech_destroy_parsed_message(parsed_message);
	
	return r;
}

void ms_speech_handle_connection_cleanup(ms_speech_connection_t connection)
{
	if (connection->json_tokenizer != NULL) {
		json_tokener_free(connection->json_tokenizer);
		connection->json_tokenizer = NULL;
	}
	if (connection->current_parsed_message != NULL) {
		ms_speech_destroy_parsed_message(connection->current_parsed_message);
		connection->current_parsed_message = NULL;
	}
}

static int ms_speech_handle_speech_startdetected(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message)
{
	ms_speech_startdetected_message_t message;
	message.parsed_message = parsed_message;
	
	struct json_object *value_json = NULL;
	if (!json_object_object_get_ex(parsed_message->json_payload, MS_SPEECH_MESSAGE_KEY_PHRASE_OFFSET, &value_json) ||
		(json_object_get_type(value_json) !=json_type_int)) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR,
								 "%s: offset is not of type int",
								 MS_SPEECH_MESSAGE_PATH_SPEECH_STARTDETECTED);

		return -EINVAL;
	} else {
		message.offset = json_object_get_double(value_json) / 10000000.0;
	}

	if (connection->callbacks->speech_startdetected)
		connection->callbacks->speech_startdetected(connection, &message, connection->callbacks->user_data);
	
	return 0;
}

static int ms_speech_handle_speech_enddetected(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message)
{
	ms_speech_enddetected_message_t message;
	message.parsed_message = parsed_message;
	
	struct json_object *value_json = NULL;
	if (!json_object_object_get_ex(parsed_message->json_payload, MS_SPEECH_MESSAGE_KEY_PHRASE_OFFSET, &value_json) ||
		(json_object_get_type(value_json) !=json_type_int)) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR,
								 "%s: offset is not of type int",
								 MS_SPEECH_MESSAGE_PATH_SPEECH_ENDDETECTED);

//		return -EINVAL;
	} else {
		message.offset = json_object_get_double(value_json) / 10000000.0;
	}

	if (connection->callbacks->speech_enddetected)
		connection->callbacks->speech_enddetected(connection, &message, connection->callbacks->user_data);
	
	return 0;
}

static int ms_speech_extract_phrase_time(ms_speech_connection_t connection, struct json_object *json, ms_speech_phrase_timing_t *timing)
{
	struct json_object *value_json = NULL;
	if (!json_object_object_get_ex(json, MS_SPEECH_MESSAGE_KEY_PHRASE_OFFSET, &value_json) ||
		(json_object_get_type(value_json) !=json_type_int)) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR,
								 "offset is not of type int");

		return -EINVAL;
	} else {
		timing->offset = json_object_get_double(value_json) / 10000000.0;
	}

	if (!json_object_object_get_ex(json, MS_SPEECH_MESSAGE_KEY_PHRASE_DURATION, &value_json) ||
		(json_object_get_type(value_json) !=json_type_int)) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR,
								 "duration is not of type int");

		return -EINVAL;
	} else {
		timing->duration = json_object_get_double(value_json) / 10000000.0;
	}

	return 0;
}

static int ms_speech_handle_speech_hypothesis(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message)
{
	if (parsed_message->json_payload == NULL) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR,
								 "%s has no payload",
								 MS_SPEECH_MESSAGE_PATH_SPEECH_HYPOTHESIS);
		
		return -EINVAL;
	}
	
	ms_speech_hypothesis_message_t message;
	message.parsed_message = parsed_message;
	
	struct json_object *value_json = NULL;
	if (!json_object_object_get_ex(parsed_message->json_payload, MS_SPEECH_MESSAGE_KEY_TEXT, &value_json) ||
		(json_object_get_type(value_json) != json_type_string)) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR,
								 "%s: text is not of type string",
								 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);

		return -EINVAL;
	}
	message.text = json_object_get_string(value_json);
	if (ms_speech_extract_phrase_time(connection, parsed_message->json_payload, &message.time))
		return -EINVAL;

	if (connection->callbacks->speech_hypothesis)
		connection->callbacks->speech_hypothesis(connection, &message, connection->callbacks->user_data);
	
	return 0;
}

static int ms_speech_handle_speech_result(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message)
{
	if ((parsed_message->json_payload == NULL) ||
		json_object_get_type(parsed_message->json_payload) != json_type_object) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR,
								 "%s json payload is not of type object",
								 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);

		return -EINVAL;
	}
	
	ms_speech_result_message_t message;
	memset(&message, 0, sizeof(message));
	message.parsed_message = parsed_message;
	
	struct json_object *reco_status_json = NULL;
	if (!json_object_object_get_ex(parsed_message->json_payload, MS_SPEECH_MESSAGE_KEY_RECOGNITION_STATUS, &reco_status_json) ||
		(json_object_get_type(reco_status_json) !=json_type_string)) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR,
								 "%s recognition status is not of type string",
								 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);
		return -EINVAL;
	}
	const char *reco_status = json_object_get_string(reco_status_json);
	if (!strcasecmp(reco_status, MS_SPEECH_RECO_STATUS_SUCCESS)) {
		message.status = MS_SPEECH_RECO_SUCCESS;
	} else if (!strcasecmp(reco_status, MS_SPEECH_RECO_STATUS_DICTATION_END)) {
		message.status = MS_SPEECH_DICTATION_END;
	} else if (!strcasecmp(reco_status, MS_SPEECH_RECO_STATUS_NO_MATCH)) {
		message.status = MS_SPEECH_NO_MATCH;
	} else if (!strcasecmp(reco_status, MS_SPEECH_RECO_STATUS_INITIAL_SILENCE_TIMEOUT)) {
		message.status = MS_SPEECH_INITIAL_SILENCE_TIMEOUT;
	} else if (!strcasecmp(reco_status, MS_SPEECH_RECO_STATUS_BABBLE_TIMEOUT)) {
		message.status = MS_SPEECH_BABBLE_TIMEOUT;
	} else if (!strcasecmp(reco_status, MS_SPEECH_RECO_STATUS_ERROR)) {
		message.status = MS_SPEECH_ERROR;
	} else {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR,
								 "%s recognition status is of unknown value: %s",
								 reco_status,
								 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);
		
		return -EINVAL;
	}
	
	int r = 0;

	// at this point, if it is not success, don't parse anything else
	if (message.status == MS_SPEECH_RECO_SUCCESS) {
		// check if it is detailed
		struct json_object *simple_display_text_json = NULL;
		message.is_detailed = !json_object_object_get_ex(parsed_message->json_payload, MS_SPEECH_MESSAGE_KEY_DISPLAY_TEXT, &simple_display_text_json);
		if (!message.is_detailed) {
			message.num_phrase_results = 1;
			message.phrase_results = (ms_speech_phrase_result_t *)malloc(sizeof(ms_speech_phrase_result_t));
			memset(message.phrase_results, 0, sizeof(ms_speech_phrase_result_t));
			message.phrase_results[0].confidence = NAN;
			message.phrase_results[0].display = json_object_get_string(simple_display_text_json);
			if (ms_speech_extract_phrase_time(connection, parsed_message->json_payload, &message.phrase_results[0].time)) {
				ms_speech_connection_log(connection,
										 MS_SPEECH_LOG_ERR,
										 "%s unable to parse phrase time",
										 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);

				r = -EINVAL;
			}

		} else {
			struct json_object *nbest_json = NULL;
			if (!json_object_object_get_ex(parsed_message->json_payload, MS_SPEECH_MESSAGE_KEY_NBEST, &nbest_json) ||
				(json_object_get_type(nbest_json) != json_type_array)) {
				ms_speech_connection_log(connection,
										 MS_SPEECH_LOG_ERR,
										 "%s nbest is not of type array",
										 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);
				
				r = -EINVAL;
			}
			else {
				message.num_phrase_results = json_object_array_length(nbest_json);
				message.phrase_results = (ms_speech_phrase_result_t *)malloc(sizeof(ms_speech_phrase_result_t) * message.num_phrase_results);
				memset(message.phrase_results, 0, sizeof(ms_speech_phrase_result_t) * message.num_phrase_results);
				for(int i=0; i<message.num_phrase_results; i++) {
					struct json_object *entry_json = json_object_array_get_idx(nbest_json, i);
					if (json_object_get_type(entry_json) != json_type_object) {
						ms_speech_connection_log(connection,
												 MS_SPEECH_LOG_ERR,
												 "%s nbest entry %d is not of type object",
												 i,
												 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);
						
						r = -EINVAL;
						break;
					}
					
					// extract confidence
					struct json_object *value_json = NULL;
					if (!json_object_object_get_ex(entry_json, MS_SPEECH_MESSAGE_KEY_CONFIDENCE, &value_json) ||
						(json_object_get_type(value_json) != json_type_double)) {
						ms_speech_connection_log(connection,
												 MS_SPEECH_LOG_ERR,
												 "%s nbest entry %d: confidence is not of type double",
												 i,
												 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);
						r = -EINVAL;
						break;
					}
					message.phrase_results[i].confidence = json_object_get_double(value_json);
					
					// extract lexical
					value_json = NULL;
					if (!json_object_object_get_ex(entry_json, MS_SPEECH_MESSAGE_KEY_LEXICAL, &value_json) ||
						(json_object_get_type(value_json) != json_type_string)) {
						ms_speech_connection_log(connection,
												 MS_SPEECH_LOG_ERR,
												 "%s nbest entry %d: lexical is not of type string",
												 i,
												 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);
						
						r = -EINVAL;
						break;
					}
					message.phrase_results[i].lexical = json_object_get_string(value_json);
					
					// extract itn
					value_json = NULL;
					if (!json_object_object_get_ex(entry_json, MS_SPEECH_MESSAGE_KEY_ITN, &value_json) ||
						(json_object_get_type(value_json) != json_type_string)) {
						ms_speech_connection_log(connection,
												 MS_SPEECH_LOG_ERR,
												 "%s nbest entry %d: itn is not of type string",
												 i,
												 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);
						
						r = -EINVAL;
						break;
					}
					message.phrase_results[i].itn = json_object_get_string(value_json);
					
					// extract masked itn
					value_json = NULL;
					if (!json_object_object_get_ex(entry_json, MS_SPEECH_MESSAGE_KEY_MASKED_ITN, &value_json) ||
						(json_object_get_type(value_json) != json_type_string)) {
						ms_speech_connection_log(connection,
												 MS_SPEECH_LOG_ERR,
												 "%s nbest entry %d: masked itn is not of type string",
												 i,
												 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);
						
						r = -EINVAL;
						break;
					}
					message.phrase_results[i].masked_itn = json_object_get_string(value_json);
					
					// extract display
					value_json = NULL;
					if (!json_object_object_get_ex(entry_json, MS_SPEECH_MESSAGE_KEY_DISPLAY, &value_json) ||
						(json_object_get_type(value_json) != json_type_string)) {
						ms_speech_connection_log(connection,
												 MS_SPEECH_LOG_ERR,
												 "%s nbest entry %d: display is not of type string",
												 i,
												 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);
						
						r = -EINVAL;
						break;
					}
					message.phrase_results[i].display = json_object_get_string(value_json);

					if (ms_speech_extract_phrase_time(connection, parsed_message->json_payload, &message.phrase_results[i].time)) {
						ms_speech_connection_log(connection,
												 MS_SPEECH_LOG_ERR,
												 "%s unable to parse phrase time",
												 MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE);

						r = -EINVAL;
						break;
					}
				}
			}
		}
	}
	
	if (!r) {
		if (connection->callbacks->speech_result)
			connection->callbacks->speech_result(connection, &message, connection->callbacks->user_data);
	}
	
	if (message.phrase_results != NULL)
		free(message.phrase_results);
	
	return r;
}

static int ms_speech_handle_turn_start(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message)
{
	ms_speech_turn_start_message_t message;
	message.parsed_message = parsed_message;


	int r = 0;
	struct json_object *context_json = NULL;
	if (!json_object_object_get_ex(parsed_message->json_payload, MS_SPEECH_MESSAGE_KEY_CONTEXT, &context_json) ||
		(json_object_get_type(context_json) != json_type_object)) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR,
								 "%s Context is not of type object",
								 MS_SPEECH_MESSAGE_PATH_TURN_END);

		r = -EINVAL;
	} else {
		struct json_object *value_json = NULL;
		if (!json_object_object_get_ex(context_json, MS_SPEECH_MESSAGE_KEY_SERVICE_TAG, &value_json) ||
				(json_object_get_type(value_json) != json_type_string)) {
			ms_speech_connection_log(connection,
									 MS_SPEECH_LOG_ERR,
									 "%s Context.serviceTag is not of type string",
									 MS_SPEECH_MESSAGE_PATH_TURN_END);

			r = -EINVAL;
		}
		else {
			message.context.service_tag = json_object_get_string(value_json);
		}
	}

	if (!r) {
		if (connection->callbacks->turn_start)
			connection->callbacks->turn_start(connection, &message, connection->callbacks->user_data);
	}

	return r;
}

static int ms_speech_handle_turn_end(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message)
{
	ms_speech_turn_end_message_t message;
	message.parsed_message = parsed_message;

	if (connection->callbacks->turn_end)
		connection->callbacks->turn_end(connection, &message, connection->callbacks->user_data);
	
	// we'll send telemetry in all cases
	connection->status = MS_SPEECH_CLIENT_TELEMETRY_PENDING;	
	return -EAGAIN;
}

static int ms_speech_extract_headder_fields(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message)
{
	for(int i=0; i<parsed_message->num_headers; i++) {
		if (!strcasecmp(parsed_message->headers[i].name, MS_SPEECH_PATH_HEADER))
			parsed_message->path = parsed_message->headers[i].value;
		else if (!strcasecmp(parsed_message->headers[i].name, MS_SPEECH_REQUEST_ID_HEADER))
			parsed_message->request_id = parsed_message->headers[i].value;
		else if (!strcasecmp(parsed_message->headers[i].name, MS_SPEECH_CONTENT_TYPE_HEADER))
			parsed_message->content_type = parsed_message->headers[i].value;
	}
	
	if (!parsed_message->path) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR | MS_SPEECH_LOG_HEADER,
								 "%s header does not exist",
								 MS_SPEECH_PATH_HEADER);

		return -EINVAL;
	}
	if (!parsed_message->request_id) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR | MS_SPEECH_LOG_HEADER,
								 "%s header does not exist",
								 MS_SPEECH_REQUEST_ID_HEADER);

		return -EINVAL;
	}
	if (!parsed_message->content_type) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR | MS_SPEECH_LOG_HEADER,
								 "%s header does not exist",
								 MS_SPEECH_CONTENT_TYPE_HEADER);
		return -EINVAL;
	}
	
	return 0;
}

static int ms_speech_extract_headers(ms_speech_connection_t connection, char *start, size_t len, ms_speech_header_t **parsed_headers, int *num)
{
	*parsed_headers = NULL;
	*num = 0;
	ms_speech_header_t *headers = NULL;
	
	int num_headers = 0;
	char *end = NULL;
	while((end = strnstr(start, "\r\n", len))) {
		char *sep = strnstr(start, ":", end - start);
		if (!sep) {
			ms_speech_connection_log(connection,
									 MS_SPEECH_LOG_WARN | MS_SPEECH_LOG_HEADER,
									 "Skipping invalid header format: %.*s",
									 end - start,
									 start);
			// TODO: be more strict?
			continue;
		}
		char *name = start;
		*sep = '\0';
		char *value = sep + 1;
		*end = '\0';
		
		headers = (ms_speech_header_t *)realloc(headers, sizeof(ms_speech_header_t) * (num_headers + 1));
		headers[num_headers].name = strdup(trim_string(name));
		headers[num_headers].value = strdup(trim_string(value));
		
		len -= end - start + 2;
		start = end + 2;
		num_headers++;
	}
	
	*parsed_headers = headers;
	*num = num_headers;
	
	return 0;
}

static int ms_speech_parse_payload(ms_speech_connection_t connection, char *payload, size_t len, struct json_object **json_payload)
{
	*json_payload = NULL;
	
	if (connection->json_tokenizer == NULL)
		connection->json_tokenizer = json_tokener_new();
	
	*json_payload = json_tokener_parse_ex(connection->json_tokenizer, payload, (int)len);
	enum json_tokener_error jerr = json_tokener_get_error(connection->json_tokenizer);
	// if there more json is expected, this must not be our final fragment
	// TODO: current we use it as string, is this the right thing to do?
	if ((jerr == json_tokener_continue) &&
		!lws_is_final_fragment(connection->wsi)) {
		// partial message
		return -EAGAIN;
	}
	if (jerr != json_tokener_success) {
		// non-json payload assume string
		// TODO: handle properly
		*json_payload = json_object_new_string_len(payload, (int)len);
	}
	json_tokener_reset(connection->json_tokenizer);
	
	return 0;
}

static int ms_speech_parse_response_message(ms_speech_connection_t connection, void *buffer, size_t len, ms_speech_parsed_message_t **parsed_message)
{
	*parsed_message = NULL;
	
	char *payload_start = NULL;
	size_t payload_size = 0;
	int r = 0;

	// check if this is a continuation
	if (connection->current_parsed_message == NULL) {
		// new message
		connection->current_parsed_message = (ms_speech_parsed_message_t *)malloc(sizeof(ms_speech_parsed_message_t));
		memset(connection->current_parsed_message, 0, sizeof(ms_speech_parsed_message_t));

		char *start = (char *) buffer;
		char *headers_end = strnstr(start, "\r\n\r\n", len);
		if (!headers_end || (start == headers_end)) {
			// no headers
			return -EINVAL;
		}
	
		size_t headers_len = (headers_end - start) + 2;
		ms_speech_header_t *parsed_headers = NULL;
		int num_headers = 0;
		r = ms_speech_extract_headers(connection, start, headers_len, &parsed_headers, &num_headers);
		if (r) {
			ms_speech_connection_log(connection,
									 MS_SPEECH_LOG_ERR | MS_SPEECH_LOG_HEADER,
									 "Failed to extract headers");
			return r;
		}
		
		connection->current_parsed_message->headers = parsed_headers;
		connection->current_parsed_message->num_headers = num_headers;
		r = ms_speech_extract_headder_fields(connection, connection->current_parsed_message);

		payload_start = headers_end + 4;
		payload_size = len - (payload_start - start);
	} else {
		// continuation
		payload_start = (char *) buffer;
		payload_size = len;
	}
	
	if (!r) {
		struct json_object *json_payload = NULL;
		if (payload_size > 0) {
			// parse json payload
			r = ms_speech_parse_payload(connection, payload_start, payload_size, &json_payload);
		}
		
		if (r == -EAGAIN) {
			// not a final message
			ms_speech_connection_log(connection,
									 MS_SPEECH_LOG_DEBUG,
									 "Partial payload");
			return r;
		}
		else if (!r) {
			connection->current_parsed_message->json_payload = json_payload;
		}
	}
	if (r) {
		// cleanup
		ms_speech_destroy_parsed_message(connection->current_parsed_message);
		connection->current_parsed_message = NULL;
	} else {
		*parsed_message = connection->current_parsed_message;
		connection->current_parsed_message = NULL;
	}
	
	return r;
}

static void ms_speech_destroy_parsed_message(ms_speech_parsed_message_t *parsed_message)
{
	if (parsed_message->headers != NULL) {
		for(int i=0; i<parsed_message->num_headers; i++) {
			free(parsed_message->headers[i].name);
			free(parsed_message->headers[i].value);
		}
		free(parsed_message->headers);
	}
	if (parsed_message->json_payload != NULL) {
		int r = json_object_put(parsed_message->json_payload);
		if (r != 1) {
			// didn't free json. shouldn't happen
			ms_speech_log(MS_SPEECH_LOG_WARN,
						  "Failed to extract headers");
		}
	}
	free(parsed_message);
}

static char * trim_string(char *string)
{
	char *start = string;
	char *end = start + strlen(string);
	while(isspace(*start) && (start != end)) start++;
	if (start != end) {
		do {
			end--;
		} while(isspace(*end) && (end != start));
		end[1] = '\0';
	}
	
	return start;
}
