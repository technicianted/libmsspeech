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

#ifndef response_messages_h
#define response_messages_h

#include <json-c/json.h>

/**
 * \typedef ms_speech_reco_status_t.
 *
 * \see https://docs.microsoft.com/en-us/azure/cognitive-services/speech/api-reference-rest/websocketprotocol
 */
typedef enum
{
	MS_SPEECH_RECO_SUCCESS,
	MS_SPEECH_DICTATION_END,
	MS_SPEECH_NO_MATCH,
	MS_SPEECH_INITIAL_SILENCE_TIMEOUT,
	MS_SPEECH_BABBLE_TIMEOUT,
	MS_SPEECH_ERROR
} ms_speech_reco_status_t;

/**
 * \typedef ms_speech_phrase_timing_t
 * \brief Structure for phrase timing information.
 */
typedef struct {
	// Phrase start offset in seconds.
	double offset;
	// Phrase duration in seconds.
	double duration;
} ms_speech_phrase_timing_t;

/**
 * \typedef ms_speech_header_t
 * \brief An HTTP header entry.
 */
typedef struct {
	// Header name.
	char *name;
	// Header value.
	char *value;
} ms_speech_header_t;

/**
 * \typedef ms_speech_parsed_message_t
 * \brief Received message common fields.
 */
typedef struct {
	// Number of headers extracted in this message.
	int num_headers;
	// Array of headers.
	ms_speech_header_t *headers;
	// Message raw json payload.
	struct json_object *json_payload;
	
	// Message path (i.e. speech.phrase).
	const char *path;
	// Message request ID.
	const char *request_id;
	// Message content type.
	const char *content_type;
} ms_speech_parsed_message_t;

/**
 * \typedef ms_speech_startdetected_message_t
 * \brief speech.startDetected message.
 */
typedef struct
{
	ms_speech_parsed_message_t *parsed_message;

	// Audio offset in seconds.
	double offset;
} ms_speech_startdetected_message_t;

/**
 * \typedef ms_speech_enddetected_message_t
 * \brief speech.endDetected message.
 */
typedef struct
{
	ms_speech_parsed_message_t *parsed_message;

	// Audio offset in seconds.
	double offset;
} ms_speech_enddetected_message_t;

/**
 * \typedef ms_speech_turn_end_context_t
 * \brief turn.end message context.
 */
typedef struct
{
	// Service tag identifying this request.
	const char *service_tag;
} ms_speech_turn_end_context_t;

/**
 * \typedef ms_speech_turn_start_message_t
 * \brief turn.start message.
 */
typedef struct
{
	ms_speech_parsed_message_t *parsed_message;
	
	// Message context.
	ms_speech_turn_end_context_t context;
} ms_speech_turn_start_message_t;

/**
 * \typedef ms_speech_turn_end_message_t
 * \brief turn.end message.
 */
typedef struct
{
	ms_speech_parsed_message_t *parsed_message;
} ms_speech_turn_end_message_t;

/**
 * \typedef ms_speech_hypothesis_message_t.
 * \brief speech.hypothesis message.
 */
typedef struct
{
	ms_speech_parsed_message_t *parsed_message;

	// Recognized text.
	const char *text;
	// Text timing information.
	ms_speech_phrase_timing_t time;
} ms_speech_hypothesis_message_t;

/**
 * \typedef ms_speech_phrase_result_t
 * \brief A single phrase result.
 */
typedef struct
{
	// Raw json.
	json_object *json;

	// Speech recognition confidence.
	double confidence;
	// Recognized text lexical form.
	const char *lexical;
	// Recognized text ITN form.
	const char *itn;
	// Recognized text masked ITN form.
	const char *masked_itn;
	// Recognized text display form.
	const char *display;
	// Phrase timing information.
	ms_speech_phrase_timing_t time;
} ms_speech_phrase_result_t;

/**
 * \typedef ms_speech_result_message_t
 * \brief speech.phrase message.
 */
typedef struct
{
	ms_speech_parsed_message_t *parsed_message;

	// Recognition status.
	ms_speech_reco_status_t status;
	// Whether this response was detailed or not.
	int is_detailed;
	// Number of nbest results.
	int num_phrase_results;
	ms_speech_phrase_result_t *phrase_results;
} ms_speech_result_message_t;

#endif /* response_messages_h */
