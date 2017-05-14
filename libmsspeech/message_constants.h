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

#ifndef message_constants_h
#define message_constants_h

#include <stdio.h>

extern const char *MS_SPEECH_PATH_HEADER;
extern const char *MS_SPEECH_REQUEST_ID_HEADER;
extern const char *MS_SPEECH_TIMESTAMP_HEADER;
extern const char *MS_SPEECH_CONTENT_TYPE_HEADER;

extern const char *MS_SPEECH_MESSAGE_CONTENT_TYPE_JSON;

extern const char *MS_SPEECH_MESSAGE_PATH_SPEECH_CONFIG;
extern const char *MS_SPEECH_MESSAGE_PATH_AUDIO;
extern const char *MS_SPEECH_MESSAGE_PATH_SPEECH_STARTDETECTED;
extern const char *MS_SPEECH_MESSAGE_PATH_SPEECH_ENDDETECTED;
extern const char *MS_SPEECH_MESSAGE_PATH_SPEECH_HYPOTHESIS;
extern const char *MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE;
extern const char *MS_SPEECH_MESSAGE_PATH_TURN_START;
extern const char *MS_SPEECH_MESSAGE_PATH_TURN_END;
extern const char *MS_SPEECH_MESSAGE_PATH_TELEMETRY;

extern const char *MS_SPEECH_MESSAGE_KEY_TEXT;
extern const char *MS_SPEECH_MESSAGE_KEY_DISPLAY_TEXT;
extern const char *MS_SPEECH_MESSAGE_KEY_RECOGNITION_STATUS;
extern const char *MS_SPEECH_MESSAGE_KEY_NBEST;
extern const char *MS_SPEECH_MESSAGE_KEY_CONFIDENCE;
extern const char *MS_SPEECH_MESSAGE_KEY_LEXICAL;
extern const char *MS_SPEECH_MESSAGE_KEY_ITN;
extern const char *MS_SPEECH_MESSAGE_KEY_MASKED_ITN;
extern const char *MS_SPEECH_MESSAGE_KEY_DISPLAY;
extern const char *MS_SPEECH_MESSAGE_KEY_CONTEXT;
extern const char *MS_SPEECH_MESSAGE_KEY_SERVICE_TAG;
extern const char *MS_SPEECH_MESSAGE_KEY_PHRASE_OFFSET;
extern const char *MS_SPEECH_MESSAGE_KEY_PHRASE_DURATION;

extern const char *MS_SPEECH_RECO_STATUS_SUCCESS;
extern const char *MS_SPEECH_RECO_STATUS_DICTATION_END;
extern const char *MS_SPEECH_RECO_STATUS_NO_MATCH;
extern const char *MS_SPEECH_RECO_STATUS_INITIAL_SILENCE_TIMEOUT;
extern const char *MS_SPEECH_RECO_STATUS_BABBLE_TIMEOUT;
extern const char *MS_SPEECH_RECO_STATUS_ERROR;

#endif /* message_constants_h */
