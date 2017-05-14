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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "ms_speech.h"

int wav_fd = 0;
int done = 0;

const char * auth_token(ms_speech_connection_t connection, void *user_data, size_t max_len);
void client_ready(ms_speech_connection_t connection, void *user_data);
void global_log(ms_speech_log_level_t level, const char *message);
void connection_log(ms_speech_connection_t connection, void *user_data, ms_speech_log_level_t level, const char *message);
void speech_startdetected(ms_speech_connection_t connection, ms_speech_startdetected_message_t *message, void *user_data);
void speech_enddetected(ms_speech_connection_t connection, ms_speech_enddetected_message_t *message, void *user_data);
void speech_hypothesis(ms_speech_connection_t connection, ms_speech_hypothesis_message_t *message, void *user_data);
void speech_result(ms_speech_connection_t connection, ms_speech_result_message_t *message, void *user_data);
void turn_start(ms_speech_connection_t connection, ms_speech_turn_start_message_t *message, void *user_data);
void turn_end(ms_speech_connection_t connection, ms_speech_turn_end_message_t *message, void *user_data);

int main(int argc, const char * argv[]) {

        ms_speech_set_logging(0, &global_log);

        ms_speech_client_callbacks_t callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.log = &connection_log;
        callbacks.provide_authentication_header = &auth_token;
        callbacks.client_ready = &client_ready;

        callbacks.speech_startdetected = &speech_startdetected;
        callbacks.speech_enddetected = &speech_enddetected;
        callbacks.speech_hypothesis = &speech_hypothesis;
        callbacks.speech_result = &speech_result;
        callbacks.turn_start = &turn_start;
        callbacks.turn_end = &turn_end;

        ms_speech_connection_t connection;
        ms_speech_context_t context = ms_speech_create_context();
        ms_speech_connect(context, "wss://speech.platform.bing.com/speech/recognition/interactive/cognitiveservices/v1?format=detailed&language=en-us", &callbacks, &connection);

        wav_fd = open(argv[1], O_RDONLY);

        while(!done) {
                ms_speech_service_step(context, 500);
        }

        return 0;
}

int stream_callback(ms_speech_connection_t connection, unsigned char *buffer, int len)
{
        ssize_t r = read(wav_fd, buffer, len);
        return (int)r;
}

void client_ready(ms_speech_connection_t connection, void *user_data)
{
       ms_speech_start_stream(connection, &stream_callback);
}

const char * auth_token(ms_speech_connection_t connection, void *user_data, size_t max_len)
{
	return "Ocp-Apim-Subscription-Key: e399fdcbd2cc4091845299cd9d7288c6";
}

void print_message_header(ms_speech_parsed_message_t *parsed_message)
{
        printf(" path: %s\n", parsed_message->path);
        printf(" request_id: %s\n", parsed_message->request_id);
        printf(" content_type: %s\n", parsed_message->content_type);
}

void speech_startdetected(ms_speech_connection_t connection, ms_speech_startdetected_message_t *message, void *user_data)
{
        printf("============= RESPONSE: speech.startDetected\n");
        print_message_header(message->parsed_message);
        printf(" offset: %f\n", message->offset);
}

void speech_enddetected(ms_speech_connection_t connection, ms_speech_enddetected_message_t *message, void *user_data)
{
        printf("============= RESPONSE: speech.endDetected\n");
        print_message_header(message->parsed_message);
        printf(" offset: %f\n", message->offset);
}

void speech_hypothesis(ms_speech_connection_t connection, ms_speech_hypothesis_message_t *message, void *user_data)
{
        printf("============= RESPONSE: speech.hypothesis\n");
        print_message_header(message->parsed_message);
        printf(" text: %s\n", message->text);
        printf(" time: offset: %f, duration: %f\n",
        	message->time.offset,
		message->time.duration);
}

void speech_result(ms_speech_connection_t connection, ms_speech_result_message_t *message, void *user_data)
{
        printf("============= RESPONSE: speech.phrase\n");
        print_message_header(message->parsed_message);
        printf(" status: %d\n", message->status);
        printf(" detailed: %d\n", message->is_detailed);
        printf(" nbest: %d\n", message->num_phrase_results);
        for(int i=0; i<message->num_phrase_results; i++) {
                printf(" %d\n", i);
                printf("  confidence: %f\n", message->phrase_results[i].confidence);
                printf("  lexical: %s\n", message->phrase_results[i].lexical);
                printf("  itn: %s\n", message->phrase_results[i].itn);
                printf("  masked itn: %s\n", message->phrase_results[i].masked_itn);
                printf("  display: %s\n", message->phrase_results[i].display);
                printf("  time: offset: %f, duration: %f\n",
                		message->phrase_results[i].time.offset,
						message->phrase_results[i].time.duration);
        }
}

void turn_start(ms_speech_connection_t connection, ms_speech_turn_start_message_t *message, void *user_data)
{
        printf("============= RESPONSE: turn.start\n");
        print_message_header(message->parsed_message);
        printf(" serviceTag: %s\n", message->context.service_tag);
}

void turn_end(ms_speech_connection_t connection, ms_speech_turn_end_message_t *message, void *user_data)
{
        printf("============= RESPONSE: turn.end\n");
        print_message_header(message->parsed_message);

        // stop streaming, if we are
        close(wav_fd);
        wav_fd = 0;
        done = 1;
}

void global_log(ms_speech_log_level_t level, const char *message)
{
        printf("%s\n", message);
}

void connection_log(ms_speech_connection_t connection, void *user_data, ms_speech_log_level_t level, const char *message)
{
        printf("%s\n", message);
}
