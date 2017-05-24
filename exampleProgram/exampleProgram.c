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
#include <argp.h>

#include "ms_speech.h"

int arg_num = 0;
int wav_fd = 0;
int log_level = 0;
int done = 0;
int detailed = 0;
const char *subscription_key = NULL;
const char *language = NULL;
const char *endpoint = "wss://speech.platform.bing.com/speech/recognition/interactive/cognitiveservices/v1";
const char *input_file = NULL;

const char * auth_token(ms_speech_connection_t connection, void *user_data, size_t max_len);
void connection_error(ms_speech_connection_t connection, unsigned int http_status, const char *error_message, void *user_data);
void client_ready(ms_speech_connection_t connection, void *user_data);
void global_log(ms_speech_log_level_t level, const char *message);
void connection_log(ms_speech_connection_t connection, void *user_data, ms_speech_log_level_t level, const char *message);
void speech_startdetected(ms_speech_connection_t connection, ms_speech_startdetected_message_t *message, void *user_data);
void speech_enddetected(ms_speech_connection_t connection, ms_speech_enddetected_message_t *message, void *user_data);
void speech_hypothesis(ms_speech_connection_t connection, ms_speech_hypothesis_message_t *message, void *user_data);
void speech_result(ms_speech_connection_t connection, ms_speech_result_message_t *message, void *user_data);
void turn_start(ms_speech_connection_t connection, ms_speech_turn_start_message_t *message, void *user_data);
void turn_end(ms_speech_connection_t connection, ms_speech_turn_end_message_t *message, void *user_data);

static struct argp_option options[] = {
  	{ "debug", 'd', 0, 0, "Produce debug output", 0 },
	{ "mode", 'm', "MODE", 0, "Recognition mode: {interactive|dictation|conversation}. Default is interactive", 0 },
	{ "file", 'f', "FILE", 0, "Audio input file, stdin if omitted", 0 },
	{ "details", 't', 0, 0, "Request detailed recognition output", 0 },
	{ 0 } };

static int parse_opt(int key, char *arg, struct argp_state *state)
{
	switch (key) {
		case 'd':
			log_level = 65535;
			break;

		case 'm':
			if (!strcmp(arg, "interactive")) {
				endpoint = "wss://speech.platform.bing.com/speech/recognition/interactive/cognitiveservices/v1";
			} else if (!strcmp(arg, "dictation")) {
				endpoint = "wss://speech.platform.bing.com/speech/recognition/dictation/cognitiveservices/v1";
			} else if (!strcmp(arg, "conversation")) {
				endpoint = "wss://speech.platform.bing.com/speech/recognition/conversation/cognitiveservices/v1";
			} else {
				printf("Invalid mode '%s'\n", arg);
				argp_usage (state);
			}
			break;

		case 't':
			detailed = 1;
			break;

		case 'f':
			input_file = arg;
			break;

		case ARGP_KEY_ARG:
			if (arg_num == 0) {
				subscription_key = arg;
			}
			else if (arg_num == 1) {
				language = arg;
			}
			else {
				argp_usage(state);
			}

			arg_num++;
			break;

		case ARGP_KEY_END:
			if (arg_num != 2)
				argp_usage(state);
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = { options, parse_opt, NULL, "<key> <language>" };

int main(int argc, char * argv[]) {
	argp_parse(&argp, argc, argv, 0, 0, &options);

	ms_speech_set_logging(log_level, &global_log);

	ms_speech_client_callbacks_t callbacks;
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.log = &connection_log;
	callbacks.provide_authentication_header = &auth_token;
	callbacks.connection_error = &connection_error;
	callbacks.client_ready = &client_ready;
	callbacks.speech_startdetected = &speech_startdetected;
	callbacks.speech_enddetected = &speech_enddetected;
	callbacks.speech_hypothesis = &speech_hypothesis;
	callbacks.speech_result = &speech_result;
	callbacks.turn_start = &turn_start;
	callbacks.turn_end = &turn_end;

	char full_uri[1024];
	sprintf(full_uri, "%s?language=%s", endpoint, language);
	if (detailed)
		strcat(full_uri, "&format=detailed");

	ms_speech_connection_t connection;
	ms_speech_context_t context = ms_speech_create_context();
	ms_speech_connect(context, full_uri, &callbacks, &connection);

	if (input_file != NULL) {
		wav_fd = open(input_file, O_RDONLY);
		if (wav_fd < 0) {
			perror("Unable to open input file");
		}
	}

	printf("Connecting to: %s\n", full_uri);
	while(!done) {
			ms_speech_service_step(context, 500);
	}

	ms_speech_destroy_context(context);

	return 0;
}

void connection_error(ms_speech_connection_t connection, unsigned int http_status, const char *error_message, void *user_data)
{
	printf("Connection to %s failed: %d\n", endpoint, http_status);
	exit(1);
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
	static char buffer[1024];
	sprintf(buffer, "Ocp-Apim-Subscription-Key: %s", subscription_key);
	return buffer;
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
        printf(" nbest: %d\n", message->num_phrase_results);
        for(int i=0; i<message->num_phrase_results; i++) {
                printf(" %d\n", i);
                if (message->is_detailed) {
                	printf("  confidence: %f\n", message->phrase_results[i].confidence);
                	printf("  lexical: %s\n", message->phrase_results[i].lexical);
                	printf("  itn: %s\n", message->phrase_results[i].itn);
                	printf("  masked itn: %s\n", message->phrase_results[i].masked_itn);
                }
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
