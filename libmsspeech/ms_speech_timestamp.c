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

#include <sys/time.h>
#include <time.h>
#include <stdio.h>

#include "ms_speech_timestamp.h"

int ms_speech_get_timestamp(char *buffer, size_t len)
{
	struct timeval tv;
	struct tm info;
	
	gettimeofday(&tv, NULL);
	gmtime_r(&tv.tv_sec, &info);
	double seconds = (double)info.tm_sec + tv.tv_usec / 1000000.0;
	return snprintf(buffer, len, "%04d-%02d-%02dT%02d:%02d:%0.7f",
					info.tm_year + 1900,
					info.tm_mon + 1,
					info.tm_mday,
					info.tm_hour,
					info.tm_min,
					seconds);
}
