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

#ifndef ms_speech_priv_h
#define ms_speech_priv_h

#define MS_SPEECH_STREAM_BUFFER_SIZE 4096

#include <json-c/json.h>

#include "libwebsockets.h"
#include "ms_speech.h"

typedef enum {
	MS_SPEECH_CLIENT_DISCONNECTED,
	MS_SPEECH_CLIENT_CONNECTING,
	MS_SPEECH_CLIENT_CONNECTED
} client_connection_status_t;

typedef enum {
	MS_SPEECH_CLIENT_NONE,
	MS_SPEECH_CLIENT_SPEECH_CONFIG_PENDING,
	MS_SPEECH_CLIENT_IDLE,
	MS_SPEECH_CLIENT_STREAMING,
	MS_SPEECH_CLIENT_STREAMING_BLOCKED,
	MS_SPEECH_CLIENT_TELEMETRY_PENDING
} client_status_t;

struct ms_speech_context_st {
	struct lws_context *context;
	struct lws_context_creation_info info;
};

typedef struct
{
	const char *path;
	int binary;
	char request_time[32];
	char request_id[48];
	const char *content_type;
	
	char *body;
	size_t body_length;
} ms_speech_message;

typedef struct
{
	ms_speech_audio_stream_callback stream_callback;
	unsigned char buffer[MS_SPEECH_STREAM_BUFFER_SIZE];
	char request_id[48];
	int packet_num;
} ms_speech_streaming_info_t;

struct ms_speech_telemetry_st;
typedef struct ms_speech_telemetry_st ms_speech_telemetry_t;

struct ms_speech_connection_st {
	ms_speech_context_t context;
	struct lws *wsi;

	char *uri;
	char *path;
	
	client_connection_status_t connection_status;
	client_status_t status;
	char connection_id[48];

	json_tokener *json_tokenizer;
	ms_speech_parsed_message_t *current_parsed_message;
	
	ms_speech_streaming_info_t *streaming_info;
	
	ms_speech_client_callbacks_t *callbacks;
	
	ms_speech_telemetry_t *telemetry;
};

#endif /* ms_speech_h */
