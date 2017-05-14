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

#include "ms_speech_logging.h"
#include "ms_speech_priv.h"
#include "ms_speech_logging_priv.h"

static int ms_speech_log_levels = 0;
static ms_speech_global_log_t ms_speech_global_log_callback = NULL;

void ms_speech_set_logging(int levels, ms_speech_global_log_t callback)
{
	ms_speech_log_levels = levels;
	ms_speech_global_log_callback = callback;
	lws_set_log_level(levels, (void (*)(int, const char *))&ms_speech_log);
}

void ms_speech_log(ms_speech_log_level_t level, const char *format, ...)
{
	if (ms_speech_global_log_callback && (level & ms_speech_log_levels)) {
		va_list args;
		va_start(args, format);
		
		char *buffer = NULL;
		vasprintf(&buffer, format, args);
		ms_speech_global_log_callback(level, buffer);
		free(buffer);
		
		va_end(args);
	}
}

void ms_speech_connection_log(ms_speech_connection_t connection, ms_speech_log_level_t level, const char *format, ...)
{
	if (level & ms_speech_log_levels) {
		va_list args;
		va_start(args, format);
	
		char *buffer = NULL;
		vasprintf(&buffer, format, args);

		if (connection->callbacks->log)
			connection->callbacks->log(connection, connection->callbacks->user_data, level, buffer);
		else if (ms_speech_global_log_callback)
			ms_speech_global_log_callback(level, buffer);
		
		free(buffer);
		
		va_end(args);
	}
}
