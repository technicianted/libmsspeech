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

#ifndef client_messages_h
#define client_messages_h

ms_speech_message *ms_speech_create_new_message();

void ms_speech_destroy_message(ms_speech_message *message);
void ms_speech_set_message_time(ms_speech_message *message);
void ms_speech_set_message_request_id(ms_speech_message *message);
void ms_speech_set_message_body(ms_speech_message *message, const unsigned char *body, size_t body_length);

int ms_speech_set_message_speech_config(ms_speech_connection_t connection, ms_speech_message *message);
int ms_speech_set_message_audio(ms_speech_connection_t connection, ms_speech_message *message, const unsigned char *audio_buffer, int len);

int ms_speech_serialize_message(ms_speech_message *message, char **buffer);

#endif /* client_messages_h */
