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

#ifndef ms_speech_status_control_h
#define ms_speech_status_control_h

#include "ms_speech_priv.h"

void ms_speech_set_status(ms_speech_connection_t connection, client_status_t status);
void ms_speech_set_connection_status(ms_speech_connection_t connection, client_connection_status_t status);

#endif /* ms_speech_status_control_h */
