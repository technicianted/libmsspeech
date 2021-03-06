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

#ifndef ms_speech_guid_h
#define ms_speech_guid_h

#include <stdio.h>
#include <uuid/uuid.h>

int ms_speech_generate_guid(char *buffer, size_t len, int canonical_form);
int ms_speech_sanitize_guid(const char *input, char *buffer, size_t len, int canonical_form);
int ms_speech_uuid_to_char(uuid_t uuid, char *buffer, size_t len, int canonical_form);

#endif /* ms_speech_guid_h */
