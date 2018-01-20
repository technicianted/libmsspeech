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

#include "ms_speech_guid.h"

#include <errno.h>

int ms_speech_generate_guid(char *buffer, size_t len, int canonical_form)
{
	uuid_t uuid;
	uuid_generate(uuid);
	return ms_speech_uuid_to_char(uuid, buffer, len, canonical_form);
}

int ms_speech_sanitize_guid(const char *input, char *buffer, size_t len, int canonical_form)
{
	uuid_t uuid;
	int r = uuid_parse(input, uuid);
	if (r == -1)
		return r;
	return ms_speech_uuid_to_char(uuid, buffer, len, canonical_form);
}

int ms_speech_uuid_to_char(uuid_t uuid, char *buffer, size_t len, int canonical_form)
{
	int r = 0;
	if (canonical_form) {
		if (len < 37)
			r = -EINVAL;
		else {
			uuid_unparse(uuid, buffer);
			r = 36;
		}
	} else {
		if (len < 33)
			r = -EINVAL;
		else {
			for(int i=0; i<sizeof(uuid_t); i++)
				sprintf(buffer + i*2, "%02x", uuid[i]);
			r = 32;
		}
	}
	
	return r;
}
