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

#ifndef ms_speech_h
#define ms_speech_h

#ifdef __cplusplus
extern "C" {
#endif

#include <json-c/json.h>

#include "ms_speech/response_messages.h"
#include "ms_speech/ms_speech_logging.h"

extern const char * ms_speech_version;

struct ms_speech_context_st;
struct ms_speech_connection_st;

typedef struct ms_speech_context_st * ms_speech_context_t;
typedef struct ms_speech_connection_st * ms_speech_connection_t;

/**
 * \typedef ms_speech_audio_stream_callback
 * \brief Callback used to provide streamed audio bytes.
 * 
 * This method should be provided by users to give the client the ability
 * to request audio bytes to be sent to the service.
 * Users are required to copy their audio into the provided buffer. Users
 * may indicate the inavailability of audio. In which case, users are 
 * required to reinitiate the audio loop by calling ms_speech_resume_stream().
 * 
 * \param connection connection object that requests audio.
 * \param buffer buffer to fill with audio bytes.
 * \param buffer_len length of provided buffer.
 * \param stream_user_data
 * \return number of audio bytes copied to buffer, 0 to indicate end of audio or -EAGAIN to indicate
 * that no audio is available at this time.
 */
typedef int (*ms_speech_audio_stream_callback)(ms_speech_connection_t connection, unsigned char *buffer, int buffer_len, void *stream_user_data);

/** 
 * \typedef ms_speech_user_message_type
 * \brief Enumeration for message type in user callback.
 */
typedef enum {
	// speech.config message
	MS_SPEECH_MESSAGE_SPEECH_CONFIG
} ms_speech_user_message_type;

/**
 * \typedef ms_speech_client_callbacks_t
 * \brief Structure to define user callbacks and their data.
 */
typedef struct {
	// Callback data. It will be passed to all callbacks.
	void * user_data;
	
	/**
	 * \brief Called when connection to speech service is established.
	 *
	 * \remark This call only indicates network connection, but not client
	 * readiness.
	 *
	 * \param connection connection reference for this callback.
	 * \param user_data user data.
	 */
	void (*connection_established)(ms_speech_connection_t connection, void *user_data);
	/**
	 * \brief Called when an error occurred while establishing connection.
	 *
	 * \remark This call indicates both network and HTTP failures.
	 *
	 * \param connection connection reference for this callback.
	 * \param http_status HTTP status error code in case of HTTP failure.
	 * \param error_message error message in text.
	 * \param user_data user data.
	 */
	void (*connection_error)(ms_speech_connection_t connection, unsigned int http_status, const char *error_message, void *user_data);
	/**
	 * \brief Called when the connection has been closed.
	 *
	 * \param connection connection reference for this callback.
	 * \param user_data user data.
	 */
	void (*connection_closed)(ms_speech_connection_t connection, void *user_data);
	/**
	 * \brief Called when the client is ready for audio streaming.
	 *
	 * \param connection connection reference for this callback.
	 * \param user_data user data.
	 */
	void (*client_ready)(ms_speech_connection_t connection, void *user_data);
	/**
	 * \brief Called to provide authentication header.
	 *
	 * \param connection connection reference for this callback.
	 * \param user_data user data.
	 * \return a pointer to <header>: <value> authentication header. Value is copied.
	 */
	const char * (*provide_authentication_header)(ms_speech_connection_t connection, void *user_data, size_t max_len);
	/**
	 * \brief Called to provide any addition content in outgoing messages.
	 *
	 * Use this call back to provide any custom configurations that you need
	 * for this connection. For example speech.config.
	 *
	 * \param connection connection reference for this callback.
	 * \param type outgoing message type.
	 * \param body message body json.
	 * \param user_data user data.
	 */
	void (*message_overlay)(ms_speech_connection_t connection, ms_speech_user_message_type type, json_object *body, void *user_data);
	/**
	 * \brief Called when when speech.startDetected is received.
	 *
	 * \param connection connection reference for this callback.
	 * \param message parsed message contents.
	 * \param user_data user data.
	 */
	void (*speech_startdetected)(ms_speech_connection_t connection, ms_speech_startdetected_message_t *message, void *user_data);
	/**
	 * \brief Called when when speech.endDetected is received.
	 *
	 * \param connection connection reference for this callback.
	 * \param message parsed message contents.
	 * \param user_data user data.
	 */
	void (*speech_enddetected)(ms_speech_connection_t connection, ms_speech_enddetected_message_t *message, void *user_data);
	/**
	 * \brief Called when when speech.hypothesis is received.
	 *
	 * \param connection connection reference for this callback.
	 * \param message parsed message contents.
	 * \param user_data user data.
	 */
	void (*speech_hypothesis)(ms_speech_connection_t connection, ms_speech_hypothesis_message_t *message, void *user_data);
	/**
	 * \brief Called when when speech.fragment dictation is received.
	 *
	 * \param connection connection reference for this callback.
	 * \param message parsed message contents.
	 * \param user_data user data.
	 */
	void (*speech_fragment)(ms_speech_connection_t connection, ms_speech_fragment_message_t *message, void *user_data);
	/**
	 * \brief Called when when speech.phrase is received.
	 *
	 * \param connection connection reference for this callback.
	 * \param message parsed message contents.
	 * \param user_data user data.
	 */
	void (*speech_result)(ms_speech_connection_t connection, ms_speech_result_message_t *message, void *user_data);
	/**
	 * \brief Called when when turn.start is received.
	 *
	 * \param connection connection reference for this callback.
	 * \param message parsed message contents.
	 * \param user_data user data.
	 */
	void (*turn_start)(ms_speech_connection_t connection, ms_speech_turn_start_message_t *message, void *user_data);
	/**
	 * \brief Called when when turn.end is received.
	 *
	 * \param connection connection reference for this callback.
	 * \param message parsed message contents.
	 * \param user_data user data.
	 */
	void (*turn_end)(ms_speech_connection_t connection, ms_speech_turn_end_message_t *message, void *user_data);
	/**
	 * \brief Called to provide output log.
	 *
	 * \param connection connection reference for this callback.
	 * \param user_data user data.
	 * \param level log level.
	 * \param message log message.
	 */
	void (*log)(ms_speech_connection_t connection, void *user_data, ms_speech_log_level_t level, const char *message);
} ms_speech_client_callbacks_t;

/**
 * \brief Create a new client context.
 *
 * Use this method to create a new client context before making any
 * new connections. A context can be thought of as a connection manager
 * that is able to manage multiple client connections.
 * Client contexts must be animated by calling ms_speech_service_step on them
 * to perform an event loop cycle.
 * Client context must be destroyed by calling ms_speech_destroy_context().
 *
 * \return New client context.
 */
ms_speech_context_t ms_speech_create_context();
/**
 * \brief Destroy client context.
 *
 * Use this method to destroy a client context causing any active connections
 * to be disconnected.
 *
 * \param context client context.
 */
void ms_speech_destroy_context(ms_speech_context_t context);

/**
 * \brief Initiate a new connection to the service.
 *
 * Use this method to initiate a connection with Microsoft Speech Service. this
 * function will return immediately and the
 * remaining of the connection life time will be managed asynchronously.
 * Callers will be notified about connection progress by callbacks.
 * Users must use ms_speech_disconnect() to cleanup connections.
 * 
 * \param context client context.
 * \param uri service URI.
 * \param callbacks callbacks structure pointing to user callbacks.
 * \param conn connection object to be filled out by method return.
 * \return nonzero on failure.
 */
int ms_speech_connect(ms_speech_context_t context, const char *uri, ms_speech_client_callbacks_t *callbacks, ms_speech_connection_t *conn);
/**
 * \brief Disconnects and destroys the connection object.
 *
 * Use this method to destroy the connection object. 
 * 
 * \param connection connection object.
 * \return nonzero on failure.
 */
int ms_speech_disconnect(ms_speech_connection_t connection);
/**
 * \brief Performs a client context step.
 * 
 * msspeech library works completely asynchronously. This method should be 
 * called in a loop to perform an event handling cycle and potentially calling
 * one or more callback.
 * 
 * \param context client context.
 * \param timeout_ms loop timeout in milliseconds. 0 only processes non-blocking events.
 */
void ms_speech_service_step(ms_speech_context_t context, int timeout_ms);
/**
 * \brief Cancel current service run loop step. Used for multithreading scenarios.
 * 
 * \param context client context.
 */
void ms_speech_service_cancel_step(ms_speech_context_t context);
/**
 * \brief Request start of audio streaming.
 * 
 * Use this method to initiate audio streaming. The method will return immediately
 * and the provided callback method will be called whenever the client is ready to 
 * send more audio.
 * This method will fail if the client is not in the proper state.
 * 
 * \param connection connection object.
 * \param stream_callback callback to be used whenever the client is ready to send out audio.
 * \param request_id request ID in UUID non-cannonical format, NULL to auto generate.
 * \param strea_user_data callback user data.
 * \return nonzero on failure.
 */
int ms_speech_start_stream(ms_speech_connection_t connection, ms_speech_audio_stream_callback stream_callback, const char *request_id, void *stream_user_data);
/**
 * \brief Resume a previously paused stream.
 * 
 * Use this method to resume streaming after the callback has indicated that it does
 * not have enough audio at the moment.
 * Streaming must have been started by calling ms_speech_start_stream() and has been
 * paused by returning -EAGAIN.
 *
 * \param connection connection object.
 * \return nonzero on failure.
 */
int ms_speech_resume_stream(ms_speech_connection_t connection);

#ifdef __cplusplus
}
#endif

#endif /* ms_speech_h */
