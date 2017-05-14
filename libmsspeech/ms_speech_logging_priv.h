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

#ifndef ms_speech_logging_priv_h
#define ms_speech_logging_priv_h

void ms_speech_log(ms_speech_log_level_t level, const char *format, ...);
void ms_speech_connection_log(ms_speech_connection_t connection, ms_speech_log_level_t level, const char *format, ...);

#endif /* ms_speech_logging_priv_h */
