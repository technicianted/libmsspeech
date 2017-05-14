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

#ifndef ms_speech_telemetry_h
#define ms_speech_telemetry_h

#include <stdio.h>
#include "ms_speech_priv.h"

struct ms_speech_telemetry_st {
	json_object *received_messages;
	json_object *microphone;
};

void ms_speech_telemetry_initialize(ms_speech_connection_t connection);
void ms_speech_telemetry_destroy(ms_speech_connection_t connection);
void ms_speech_telemetry_handle_response_message(ms_speech_connection_t connection, ms_speech_parsed_message_t *parsed_message);
void ms_speech_telemetry_handle_stream_start_request(ms_speech_connection_t connection);
void ms_speech_telemetry_handle_stream_stop_request(ms_speech_connection_t connection, int user_error);
int ms_speech_telemetry_set_message(ms_speech_connection_t connection, ms_speech_message *message);

#endif /* ms_speech_telemetry_h */
