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

#include <pthread.h>

#include "compat.h"
#include "ms_speech_priv.h"
#include "ms_speech_guid.h"
#include "client_messages.h"
#include "response_messages_priv.h"
#include "ms_speech_logging_priv.h"
#include "ms_speech_status_control.h"
#include "ms_speech_telemetry.h"

const char * ms_speech_version = "0.0.3";

static const char *MS_SPEECH_CONNECTION_ID_HEADER = "X-ConnectionId";

static int handle_handshake_headers(ms_speech_connection_t connection, void *in, size_t len);
static int ws_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
static int handle_writable(ms_speech_connection_t connection);
static int write_message(ms_speech_connection_t connection, ms_speech_message * message);
static int ms_speech_handle_speech_config(ms_speech_connection_t connection, ms_speech_message *message);
static int ms_speech_handle_streaming(ms_speech_connection_t connection, ms_speech_message *message);
static int ms_speech_handle_telemetry(ms_speech_connection_t connection, ms_speech_message *message);

static const struct lws_protocols protocols[] = {
	{
		NULL,
		&ws_service_callback,
		0,
		65536,
	},
	{ NULL, NULL, 0, 0 } /* end */
};

static const struct lws_extension exts[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_no_context_takeover"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{ NULL, NULL, NULL /* terminator */ }
};

ms_speech_context_t ms_speech_create_context()
{
	ms_speech_context_t context = (ms_speech_context_t)malloc(sizeof(struct ms_speech_context_st));

	memset(context, 0, sizeof(struct ms_speech_context_st));
	context->info.port = CONTEXT_PORT_NO_LISTEN;
	context->info.iface = NULL;
	context->info.protocols = protocols;
	context->info.ssl_cert_filepath = NULL;
	context->info.ssl_private_key_filepath = NULL;
	context->info.extensions = exts;
	context->info.gid = -1;
	context->info.uid = -1;
	context->info.count_threads = 1;
	context->info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	context->context = lws_create_context(&context->info);

	return context;
}

void ms_speech_destroy_context(ms_speech_context_t context)
{
	lws_context_destroy(context->context);
	free(context);
}

int ms_speech_connect(ms_speech_context_t context, const char *uri, ms_speech_client_callbacks_t *callbacks, ms_speech_connection_t *conn)
{
	*conn = NULL;

	ms_speech_connection_t connection = (ms_speech_connection_t)malloc(sizeof(struct ms_speech_connection_st));
	memset(connection, 0, sizeof(struct ms_speech_connection_st));
	connection->callbacks = (ms_speech_client_callbacks_t *)malloc(sizeof(ms_speech_client_callbacks_t));
	memcpy(connection->callbacks, callbacks, sizeof(ms_speech_client_callbacks_t));
	connection->context = context;
	
	connection->uri = strdup(uri);
	
	struct lws_client_connect_info i;
	memset(&i, 0, sizeof(i));

	ms_speech_telemetry_initialize(connection);
	
	const char *prot;
	const char *path;
	if (lws_parse_uri(connection->uri,
					  &prot,
					  &i.address,
					  &i.port,
					  &path))
		return -EINVAL;
	
	if (strcasecmp(prot, "wss") && strcasecmp(prot, "ws"))
		return -EINVAL;
	
	size_t uri_length = strlen(uri);
	connection->path = (char *)malloc(uri_length);
	strcpy(connection->path, path);
	
	/* add back the leading / on path */
	connection->path[0] = '/';
	strncpy(connection->path + 1, path, uri_length - 1);
	i.path = connection->path;
	
	if (!strcasecmp(prot, "wss"))
		i.ssl_connection = LCCSCF_USE_SSL;
	ms_speech_connection_log(connection,
							 MS_SPEECH_LOG_DEBUG,
							 "URI parsed: proto: %s, address: %s, port: %d, path: %s, ssl: %d",
							 prot,
							 i.address,
							 i.port,
							 i.path,
							 i.ssl_connection);
	
	i.context = context->context;
	i.host = i.address;
	i.origin = i.address;
	i.ietf_version_or_minus_one = -1;
	i.userdata = connection;
	connection->wsi = lws_client_connect_via_info(&i);

	ms_speech_set_connection_status(connection, MS_SPEECH_CLIENT_CONNECTING);
	connection->status = MS_SPEECH_CLIENT_NONE;

	*conn = connection;
	
	return 0;
}

int ms_speech_disconnect(ms_speech_connection_t connection)
{
	ms_speech_connection_log(connection,
							 MS_SPEECH_LOG_DEBUG,
							 "Disconnecting");
	
	ms_speech_handle_connection_cleanup(connection);
	
	return 0;
}

void ms_speech_service_step(ms_speech_context_t context, int timeout_ms)
{
	lws_service(context->context, timeout_ms);
}

void ms_speech_service_cancel_step(ms_speech_context_t context)
{
	lws_cancel_service(context->context);
}

int ms_speech_start_stream(ms_speech_connection_t connection, ms_speech_audio_stream_callback stream_callback, const char *request_id, void *stream_user_data)
{
	char buffer[48];
	if (request_id) {
		int r = ms_speech_sanitize_guid(request_id, buffer, sizeof(buffer), 0);
		if (r == -1) {
			ms_speech_connection_log(connection,
									MS_SPEECH_LOG_ERR,
									"Cannot start streaming: request ID is in incorrect format: %s", request_id);
			return -1;
		}
	} else {
		ms_speech_generate_guid(buffer, sizeof(buffer), 0);
	}

	if (connection->connection_status != MS_SPEECH_CLIENT_CONNECTED ||
		connection->connection_status != MS_SPEECH_CLIENT_IDLE) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_WARN,
								 "Cannot start streaming since connection is in an invalid state %d", connection->connection_status);
		return EPERM;
	}
	
	ms_speech_connection_log(connection,
							 MS_SPEECH_LOG_DEBUG,
							 "Starting streaming");
	
	connection->streaming_info = (ms_speech_streaming_info_t *)malloc(sizeof(ms_speech_streaming_info_t));
	memset(connection->streaming_info, 0, sizeof(ms_speech_streaming_info_t));
	strcpy(connection->streaming_info->request_id, buffer);
	connection->streaming_info->stream_callback = stream_callback;
	connection->streaming_info->stream_user_data = stream_user_data;
	ms_speech_set_status(connection, MS_SPEECH_CLIENT_STREAMING);
	
	lws_callback_on_writable(connection->wsi);
	
	ms_speech_telemetry_handle_stream_start_request(connection);
	
	return 0;
}

int ms_speech_resume_stream(ms_speech_connection_t connection)
{
	if (connection->connection_status != MS_SPEECH_CLIENT_CONNECTED ||
		connection->status != MS_SPEECH_CLIENT_STREAMING_BLOCKED) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_WARN,
								 "Cannot resume streaming since we are in invalid state: connection %d, status: %d",
								 connection->connection_status,
								 connection->status);
		return EPERM;
	}
	
	ms_speech_connection_log(connection,
							 MS_SPEECH_LOG_DEBUG,
							 "Resuming streaming");
	
	ms_speech_set_status(connection, MS_SPEECH_CLIENT_STREAMING);
	
	lws_callback_on_writable(connection->wsi);
	
	return 0;
}

static int ws_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	ms_speech_connection_t conn = (ms_speech_connection_t)user;

	int r = 0;
	switch (reason) {
			
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
		{
			ms_speech_set_connection_status(conn, conn->connection_status = MS_SPEECH_CLIENT_CONNECTED);
			
			ms_speech_connection_log(conn,
									 MS_SPEECH_LOG_INFO,
									 "Successfully connected to service");
			
			if (conn->callbacks->connection_established)
				conn->callbacks->connection_established(conn, conn->callbacks->user_data);

			ms_speech_set_status(conn, MS_SPEECH_CLIENT_SPEECH_CONFIG_PENDING);
			lws_callback_on_writable(conn->wsi);
			
			break;
		}
			
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		{
			unsigned int http_status = lws_http_client_http_response(conn->wsi);
			ms_speech_set_connection_status(conn, MS_SPEECH_CLIENT_DISCONNECTED);
			ms_speech_set_status(conn, MS_SPEECH_CLIENT_IDLE);

			ms_speech_connection_log(conn,
									 MS_SPEECH_LOG_INFO,
									 "Failed to connect to service: %s. HTTP status: %d",
									 in ? (char *)in : "(null)",
									 http_status);

			if (conn->callbacks->connection_error)
				conn->callbacks->connection_error(conn, http_status, (char *)in, conn->callbacks->user_data);
			break;
		}
			
		case LWS_CALLBACK_CLOSED:
			ms_speech_set_connection_status(conn, MS_SPEECH_CLIENT_DISCONNECTED);
			
			ms_speech_connection_log(conn,
									 MS_SPEECH_LOG_INFO,
									 "Connection closed");

			ms_speech_set_connection_status(conn, MS_SPEECH_CLIENT_DISCONNECTED);
			ms_speech_set_status(conn, MS_SPEECH_CLIENT_IDLE);

			if (conn->callbacks->connection_closed)
				conn->callbacks->connection_closed(conn, conn->callbacks->user_data);
			break;

		case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
			r = handle_handshake_headers(conn, in, len);
			break;
			
		case LWS_CALLBACK_CLIENT_WRITEABLE:
			r = handle_writable(conn);
			break;

		case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		{
			short server_code = 0;
			if (len > sizeof(short)) {
				memcpy(&server_code, in, sizeof(short));
				server_code = ntohs(server_code);
				in += sizeof(short);
				len -= sizeof(short);
			}
			ms_speech_connection_log(conn,
									 MS_SPEECH_LOG_ERR,
									 "Service closed connection: %d: %.*s",
									 server_code,
									 len,
									 in ? (char *)in : "");
			break;
		}
			
		case LWS_CALLBACK_CLIENT_RECEIVE:
		{
			ms_speech_connection_log(conn,
									 MS_SPEECH_LOG_DEBUG,
									 "Received data: %.*s",
									 len,
									 in ? (char *)in : "");
			r = ms_speech_handle_resonse_message(conn, in, len);
			if (r == -EAGAIN) {
				lws_callback_on_writable(conn->wsi);
				r = 0;
			}
			break;
		}

		case LWS_CALLBACK_GET_THREAD_ID:
		{
			r = pthread_self();
			break;
		}
			
		default:
			break;
	}
	
	return r;
}

static int handle_handshake_headers(ms_speech_connection_t connection, void *in, size_t len)
{
	int r = 0;
	
	char **p = (char **)in;
	
	// add connection ID
	size_t header_len = strlen(MS_SPEECH_CONNECTION_ID_HEADER) + 32 + 4;
	if (len < header_len) {
		ms_speech_connection_log(connection,
								 MS_SPEECH_LOG_ERR | MS_SPEECH_LOG_HEADER,
								 "Not enough buffer for connection ID header: %d/%d",
								 header_len,
								 len);
		r = -EINVAL;
	}
	
	int written_len = sprintf(*p, "%s: ", MS_SPEECH_CONNECTION_ID_HEADER);
	*p += written_len;
	len -= written_len;
	written_len = ms_speech_generate_guid(*p, len, 0);
	strncpy(connection->connection_id, *p, sizeof(connection->connection_id));
	*p += written_len;
	len -= written_len;
	strncat(*p, "\r\n", len);
	*p += 2;
	len -= 2;
	
	if (connection->callbacks->provide_authentication_header)
	{
		const char *header = connection->callbacks->provide_authentication_header(connection, connection->callbacks->user_data, len);
		size_t header_length = strlen(header);
		if (header_length > len) {
			ms_speech_connection_log(connection,
									 MS_SPEECH_LOG_ERR | MS_SPEECH_LOG_HEADER,
									 "Requested authorization is too long: %d/%d",
									 header_length,
									 len);
			r = -EINVAL;
		} else {
			*p += sprintf(*p, "%s\x0d\x0a", header);
		}
	}

	return r;
}

static int handle_writable(ms_speech_connection_t connection)
{
	ms_speech_message *message = NULL;
	int r = 0;
	
	switch(connection->status)
	{
		case MS_SPEECH_CLIENT_SPEECH_CONFIG_PENDING:
			message = ms_speech_create_new_message();
			r = ms_speech_handle_speech_config(connection, message);
			break;
			
		case MS_SPEECH_CLIENT_STREAMING:
			message = ms_speech_create_new_message();
			r = ms_speech_handle_streaming(connection, message);
			break;

		case MS_SPEECH_CLIENT_TELEMETRY_PENDING:
			message = ms_speech_create_new_message();
			r = ms_speech_handle_telemetry(connection, message);
			break;

		default:
			break;
	}

	if (r == -EAGAIN) {
		// handler is asking for writes
		lws_callback_on_writable(connection->wsi);
		r = 0;
	}
	
	if (message != NULL)
		ms_speech_destroy_message(message);
	
	return r;
}

static int write_message(ms_speech_connection_t connection, ms_speech_message * message)
{
	ms_speech_set_message_time(message);

	char *buffer = NULL;
	int len = ms_speech_serialize_message(message, &buffer);
	ms_speech_connection_log(connection,
							 MS_SPEECH_LOG_DEBUG,
							 "Sending: %.*s",
							 len,
							 buffer);
	lws_write(connection->wsi,
			  (unsigned char *)buffer,
			  len,
			  message->binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT);
	
	return 0;
}

static int ms_speech_handle_telemetry(ms_speech_connection_t connection, ms_speech_message *message)
{
	ms_speech_telemetry_set_message(connection, message);
	int r = write_message(connection, message);
	if (!r) {
		ms_speech_set_status(connection, MS_SPEECH_CLIENT_IDLE);
	}
	return r;
}

static int ms_speech_handle_speech_config(ms_speech_connection_t connection, ms_speech_message *message)
{
	ms_speech_set_message_speech_config(connection, message);
	int r = write_message(connection, message);
	if (!r) {
		ms_speech_set_status(connection, MS_SPEECH_CLIENT_IDLE);
		connection->callbacks->client_ready(connection, connection->callbacks->user_data);
	}
	return r;
}

static int ms_speech_handle_streaming(ms_speech_connection_t connection, ms_speech_message *message)
{
	ms_speech_connection_log(connection,
							 MS_SPEECH_LOG_DEBUG,
							 "Invoking streaming callback");
	
	int r = connection->streaming_info->stream_callback(connection,
														connection->streaming_info->buffer,
														sizeof(connection->streaming_info->buffer),
														connection->streaming_info->stream_user_data);
	
	ms_speech_connection_log(connection,
							 MS_SPEECH_LOG_DEBUG,
							 "Streaming callback return: %d",
							 r);
	strcpy(message->request_id, connection->streaming_info->request_id);
	
	if (r == -EAGAIN) {
		// callback saying there is no data now
		// set pending and wait for explicit continuation
		ms_speech_set_status(connection, MS_SPEECH_CLIENT_STREAMING_BLOCKED);
		r = 0;
	} else if (r > 0) {
		ms_speech_set_message_audio(connection,
									message,
									connection->streaming_info->buffer,
									r);
		r = write_message(connection, message);
		if (!r) {
			// need more writes
			r = -EAGAIN;
		}

		connection->streaming_info->packet_num++;
	} else {
		// 0 or error return
		// end of speech or callback returned error
		int user_error = r;
		ms_speech_set_message_audio(connection,
									message,
									connection->streaming_info->buffer,
									r);
		r = write_message(connection, message);
		if (!r) {
			ms_speech_set_status(connection, MS_SPEECH_CLIENT_IDLE);
		}
		
		ms_speech_telemetry_handle_stream_stop_request(connection, user_error);
	}
	
	return r;
}
