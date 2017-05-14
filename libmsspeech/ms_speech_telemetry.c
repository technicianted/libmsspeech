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

#include "ms_speech_telemetry.h"
#include "ms_speech_timestamp.h"
#include "message_constants.h"
#include "client_messages.h"

const char *MS_SPEECH_TELEMETRY_KEY_RECEIVED_MESSAGE = "ReceivedMessages";
const char *MS_SPEECH_TELEMETRY_KEY_METRICS = "Metrics";
const char *MS_SPEECH_TELEMETRY_KEY_METRIC_NAME = "Name";
const char *MS_SPEECH_TELEMETRY_KEY_MICROPHONE = "Microphone";
const char *MS_SPEECH_TELEMETRY_KEY_START_TIME = "Start";
const char *MS_SPEECH_TELEMETRY_KEY_END_TIME = "End";
const char *MS_SPEECH_TELEMETRY_KEY_ERROR = "Error";

void ms_speech_telemetry_initialize(ms_speech_connection_t connection)
{
	connection->telemetry = (ms_speech_telemetry_t *)malloc(sizeof(ms_speech_telemetry_t));
	connection->telemetry->received_messages = json_object_new_object();
	connection->telemetry->microphone = json_object_new_object();
	json_object_object_add(connection->telemetry->microphone,
						   MS_SPEECH_TELEMETRY_KEY_METRIC_NAME,
						   json_object_new_string(MS_SPEECH_TELEMETRY_KEY_MICROPHONE));
}

void ms_speech_telemetry_destroy(ms_speech_connection_t connection)
{
	if (connection->telemetry) {
		if (connection->telemetry->received_messages)
			json_object_put(connection->telemetry->received_messages);
		if (connection->telemetry->microphone)
			json_object_put(connection->telemetry->microphone);
		free(connection->telemetry);
		connection->telemetry = NULL;
	}
}

void ms_speech_telemetry_handle_response_message(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message)
{
	json_object *path_object = NULL;
	if (!json_object_object_get_ex(connection->telemetry->received_messages,
								   parsed_message->path,
								   &path_object)) {
		path_object = json_object_new_array();
		json_object_object_add(connection->telemetry->received_messages,
							   parsed_message->path,
							   path_object);
	}
	
	char buffer[32];
	ms_speech_get_timestamp(buffer, sizeof(buffer));
	json_object *timestamp_json = json_object_new_string(buffer);
	json_object_array_add(path_object,
						  timestamp_json);
}

void ms_speech_telemetry_handle_stream_start_request(ms_speech_connection_t connection)
{
	char buffer[32];
	ms_speech_get_timestamp(buffer, sizeof(buffer));
	json_object *timestamp_json = json_object_new_string(buffer);
	json_object_object_add(connection->telemetry->microphone,
						   MS_SPEECH_TELEMETRY_KEY_START_TIME,
						   timestamp_json);
}

void ms_speech_telemetry_handle_stream_stop_request(ms_speech_connection_t connection, int user_error)
{
	char buffer[32];
	ms_speech_get_timestamp(buffer, sizeof(buffer));
	json_object *timestamp_json = json_object_new_string(buffer);
	json_object_object_add(connection->telemetry->microphone,
						   MS_SPEECH_TELEMETRY_KEY_END_TIME,
						   timestamp_json);
	if (user_error) {
		json_object *error_json = json_object_new_string(strerror(user_error));
		json_object_object_add(connection->telemetry->microphone,
							   MS_SPEECH_TELEMETRY_KEY_ERROR,
							   error_json);
	} else if (json_object_object_get_ex(connection->telemetry->microphone,
										 MS_SPEECH_TELEMETRY_KEY_ERROR,
										 NULL)) {
		json_object_object_del(connection->telemetry->microphone,
							   MS_SPEECH_TELEMETRY_KEY_ERROR);
	}
}

int ms_speech_telemetry_set_message(ms_speech_connection_t connection, ms_speech_message *message)
{
	message->path = MS_SPEECH_MESSAGE_PATH_TELEMETRY;
	message->binary = 0;
	message->content_type = MS_SPEECH_MESSAGE_CONTENT_TYPE_JSON;
	
	json_object *telemetry_json = json_object_new_object();
	json_object_object_add(telemetry_json,
						   MS_SPEECH_TELEMETRY_KEY_RECEIVED_MESSAGE,
						   connection->telemetry->received_messages);
	json_object *metrics_array = json_object_new_array();
	json_object_array_add(metrics_array,
						  connection->telemetry->microphone);
	json_object_object_add(telemetry_json,
						   MS_SPEECH_TELEMETRY_KEY_METRICS,
						   metrics_array);
	const char * json_string = json_object_to_json_string_ext(telemetry_json, JSON_C_TO_STRING_PLAIN);
	ms_speech_set_message_body(message,
							   (const unsigned char *)json_string,
							   strlen(json_string));
	
	return 0;
}

