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

#ifndef ms_speech_logging_h
#define ms_speech_logging_h

#include <libwebsockets.h>

/**
 * \typedef ms_speech_log_level_t
 * \brief Logging levels.
 */
typedef enum {
	MS_SPEECH_LOG_ERR = LLL_ERR,
	MS_SPEECH_LOG_WARN = LLL_WARN,
	MS_SPEECH_LOG_NOTICE = LLL_NOTICE,
	MS_SPEECH_LOG_INFO = LLL_INFO,
	MS_SPEECH_LOG_DEBUG = LLL_DEBUG,
	MS_SPEECH_LOG_PARSER = LLL_PARSER,
	MS_SPEECH_LOG_HEADER = LLL_HEADER,
	MS_SPEECH_LOG_EXT = LLL_EXT,
	MS_SPEECH_LOG_CLIENT = LLL_CLIENT,
	MS_SPEECH_LOG_LATENCY = LLL_LATENCY,
	MS_SPEECH_LOG_UER = LLL_USER,
} ms_speech_log_level_t;

/**
 * \typedef ms_speech_global_log_t
 * \brief Callback definition for logging.
 *
 * \param level requested message log level.
 * \param message log message.
 */
typedef void (*ms_speech_global_log_t)(ms_speech_log_level_t level, const char *message);

typedef struct ms_speech_connection_st * ms_speech_connection_t;
/**
 * \brief Set logging callback.
 *
 * \param levels requested log levels, bitwise ORed.
 * \param callback loging callback.
 */
void ms_speech_set_logging(int levels, ms_speech_global_log_t callback);

#endif /* ms_speech_logging_h */
