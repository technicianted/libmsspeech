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

#include <json-c/json.h>
#include <sys/utsname.h>
#include <sys/types.h>

#include "ms_speech_priv.h"
#include "ms_speech_logging.h"
#include "response_messages_priv.h"
#include "message_constants.h"
#include "ms_speech_timestamp.h"
#include "ms_speech_guid.h"

static const int ms_speech_maximum_header_size = 512;

ms_speech_message *ms_speech_create_new_message()
{
	ms_speech_message *message = (ms_speech_message *)malloc(sizeof(ms_speech_message));
	memset(message, 0, sizeof(ms_speech_message));
	
	return message;
}

void ms_speech_destroy_message(ms_speech_message *message)
{
	if (message->body)
		free(message->body);

	free(message);
}

void ms_speech_set_message_time(ms_speech_message *message)
{
	ms_speech_get_timestamp(message->request_time, sizeof(message->request_time));
}

void ms_speech_set_message_request_id(ms_speech_message *message)
{
	ms_speech_generate_guid(message->request_id, sizeof(message->request_id), 0);
}

void ms_speech_set_message_body(ms_speech_message *message, const unsigned char *body, size_t body_length)
{
	message->body = (char *)malloc(body_length);
	memcpy(message->body, body, body_length);
	message->body_length = body_length;
}

int ms_speech_serialize_message(ms_speech_message *message, char **buffer)
{
	*buffer = NULL;
	char *b = (char *)malloc(sizeof(unsigned short) +
							 message->body_length +
							 ms_speech_maximum_header_size +
							 LWS_PRE);
	char *buffer_start = b + LWS_PRE;
	char *p = buffer_start;
	if (message->binary)
		p += sizeof(unsigned short);
	
	// required headers
	sprintf(p,
			"%s: %s\r\n"
			"%s: %s\r\n"
			"%s: %s\r\n"
			"%s: %s\r\n",
			MS_SPEECH_PATH_HEADER, message->path,
			MS_SPEECH_TIMESTAMP_HEADER, message->request_time,
			MS_SPEECH_REQUEST_ID_HEADER, message->request_id,
			MS_SPEECH_CONTENT_TYPE_HEADER, message->content_type);
	// finalize
	strcat(p, "\r\n");
	
	unsigned short headers_length = strlen(p);
	p += headers_length;
	memcpy(p, message->body, message->body_length);

	size_t total_length =
	headers_length +
	message->body_length;
	
	if (message->binary) {
		// prepend header size
		unsigned short bit_headers_length = htons(headers_length);
		memcpy(buffer_start, &bit_headers_length, sizeof(unsigned short));
		total_length += sizeof(unsigned short);
	}
	
	*buffer = buffer_start;
	
	return (int)total_length;
}

int ms_speech_set_message_speech_config(ms_speech_connection_t connection, ms_speech_message *message)
{
	json_object *context = json_object_new_object();

	{
		json_object *system = json_object_new_object();
		json_object_object_add(system, "version", json_object_new_string(ms_speech_version));
		
		json_object_object_add(context, "system", system);
	}
	
	{
		struct utsname name;
		if (uname(&name))
			return errno;
		
		json_object *os = json_object_new_object();
		json_object_object_add(os, "platform", json_object_new_string(name.sysname));
		json_object_object_add(os, "name", json_object_new_string(name.version));
		json_object_object_add(os, "version", json_object_new_string(name.release));
		
		json_object_object_add(context, "os", os);
	}
	
	{
		json_object *device = json_object_new_object();
		json_object_object_add(device, "manufacturer", json_object_new_string(""));
		json_object_object_add(device, "model", json_object_new_string(""));
		json_object_object_add(device, "version", json_object_new_string(""));
		
		json_object_object_add(context, "device", device);
	}
	
	if (connection->callbacks->message_overlay)
		connection->callbacks->message_overlay(connection, MS_SPEECH_MESSAGE_SPEECH_CONFIG, context, connection->callbacks->user_data);
	
	message->path = MS_SPEECH_MESSAGE_PATH_SPEECH_CONFIG;
	message->binary = 0;
	message->content_type = MS_SPEECH_MESSAGE_CONTENT_TYPE_JSON;
	const char * json_string = json_object_to_json_string_ext(context, JSON_C_TO_STRING_PLAIN);
	ms_speech_set_message_body(message,
							   (const unsigned char *)json_string,
							   strlen(json_string));
	
	return 0;
}

int ms_speech_set_message_audio(ms_speech_connection_t connection,
								ms_speech_message *message,
								const unsigned char *audio_buffer,
								int len)
{
	message->path = MS_SPEECH_MESSAGE_PATH_AUDIO;
	message->binary = 1;
	// TODO: obtain proper format
	message->content_type = "audio/x-wav";
	ms_speech_set_message_body(message,
							   audio_buffer,
							   len);
	
	return 0;
}


