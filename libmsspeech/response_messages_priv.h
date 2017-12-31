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

#ifndef response_messages_priv_h
#define response_messages_priv_h

#include "ms_speech_priv.h"
#include "ms_speech/response_messages.h"

int ms_speech_handle_resonse_message(ms_speech_connection_t connection, void *buffer, size_t len);
void ms_speech_handle_connection_cleanup(ms_speech_connection_t connection);

#endif /* response_messages_priv_h */
