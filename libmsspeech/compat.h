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

#ifndef COMPAT_H_
#define COMPAT_H_

#include <string.h>
#include <stddef.h>

#ifndef strnstr
char *compat_strnstr(const char *haystack, const char *needle, size_t len);
#define strnstr compat_strnstr
#endif

#ifndef strcasecmp
int compat_strcasecmp(const char *s1, const char *s2);
#define strcasecmp compat_strcasecmp
#endif

#ifndef strdup
char *compat_strdup(const char *s);
#define strdup compat_strdup
#endif

#endif /* COMPAT_H_ */
