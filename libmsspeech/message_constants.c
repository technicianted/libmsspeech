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

#include "message_constants.h"

const char *MS_SPEECH_PATH_HEADER = "Path";
const char *MS_SPEECH_REQUEST_ID_HEADER = "X-RequestId";
const char *MS_SPEECH_TIMESTAMP_HEADER = "X-Timestamp";
const char *MS_SPEECH_CONTENT_TYPE_HEADER = "Content-Type";

const char *MS_SPEECH_MESSAGE_CONTENT_TYPE_JSON = "application/json;charset=utf-8";

const char *MS_SPEECH_MESSAGE_PATH_SPEECH_CONFIG = "speech.config";
const char *MS_SPEECH_MESSAGE_PATH_AUDIO = "audio";
const char *MS_SPEECH_MESSAGE_PATH_SPEECH_STARTDETECTED = "speech.startDetected";
const char *MS_SPEECH_MESSAGE_PATH_SPEECH_ENDDETECTED = "speech.endDetected";
const char *MS_SPEECH_MESSAGE_PATH_SPEECH_HYPOTHESIS = "speech.hypothesis";
const char *MS_SPEECH_MESSAGE_PATH_SPEECH_PHRASE = "speech.phrase";
const char *MS_SPEECH_MESSAGE_PATH_TURN_START = "turn.start";
const char *MS_SPEECH_MESSAGE_PATH_TURN_END = "turn.end";
const char *MS_SPEECH_MESSAGE_PATH_TELEMETRY = "telemetry";

const char *MS_SPEECH_MESSAGE_KEY_TEXT = "Text";
const char *MS_SPEECH_MESSAGE_KEY_RECOGNITION_STATUS = "RecognitionStatus";
const char *MS_SPEECH_MESSAGE_KEY_DISPLAY_TEXT = "DisplayText";
const char *MS_SPEECH_MESSAGE_KEY_NBEST = "NBest";
const char *MS_SPEECH_MESSAGE_KEY_CONFIDENCE = "Confidence";
const char *MS_SPEECH_MESSAGE_KEY_LEXICAL = "Lexical";
const char *MS_SPEECH_MESSAGE_KEY_ITN = "ITN";
const char *MS_SPEECH_MESSAGE_KEY_MASKED_ITN = "MaskedITN";
const char *MS_SPEECH_MESSAGE_KEY_DISPLAY = "Display";
const char *MS_SPEECH_MESSAGE_KEY_CONTEXT = "context";
const char *MS_SPEECH_MESSAGE_KEY_SERVICE_TAG = "serviceTag";
const char *MS_SPEECH_MESSAGE_KEY_PHRASE_OFFSET = "Offset";
const char *MS_SPEECH_MESSAGE_KEY_PHRASE_DURATION = "Duration";

const char *MS_SPEECH_RECO_STATUS_SUCCESS = "Success";
const char *MS_SPEECH_RECO_STATUS_DICTATION_END = "EndOfDictation";
const char *MS_SPEECH_RECO_STATUS_NO_MATCH = "NoMatch";
const char *MS_SPEECH_RECO_STATUS_INITIAL_SILENCE_TIMEOUT = "InitialSilenceTimeout";
const char *MS_SPEECH_RECO_STATUS_BABBLE_TIMEOUT = "BabbleTimeout";
const char *MS_SPEECH_RECO_STATUS_ERROR = "Error";
