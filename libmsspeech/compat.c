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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char *compat_strnstr(const char *haystack, const char *needle, size_t len)
{
        int i;
        size_t needle_len;

        if (0 == (needle_len = strlen(needle)))
                return (char *)haystack;

        for (i=0; i<=(int)(len-needle_len); i++)
        {
                if ((haystack[0] == needle[0]) &&
                        (0 == strncmp(haystack, needle, needle_len)))
                        return (char *)haystack;

                haystack++;
        }
        return NULL;
}

int compat_strcasecmp(const char *s1, const char *s2)
{
    int i, ret;
    char *cp1, *cp2;

    cp1 = (char *)malloc(strlen(s1) + 1);
    cp2 = (char *)malloc(strlen(s2) + 1);

    for (i = 0; i < strlen(s1) + 1; i++)
        cp1[i] = tolower((int) (unsigned char) s1[i]);
    for (i = 0; i < strlen(s2) + 1; i++)
        cp2[i] = tolower((int) (unsigned char) s2[i]);

    ret = strcmp(cp1, cp2);

    free(cp1);
    free(cp2);

    return ret;
}

char *compat_strdup(const char *s)
{
	size_t len = strlen(s);
	char *buf = (char *)malloc(len + 1);
	strcpy(buf, s);
	return buf;
}
