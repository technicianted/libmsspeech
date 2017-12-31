# libmsspeech
Library for Microsoft Cognitive Services [speech recognition](https://docs.microsoft.com/en-us/azure/cognitive-services/speech/home). For more details about usage, take a look at my [blog post](https://hashifdef.wordpress.com/2017/05/29/getting-started-with-microsoft-speech-recognition-under-unix/).

> __This is the very first version that works. Do not use it in any serious application yet!__

## Prerequisites
* [libwebsockets](https://libwebsockets.org) at least v2.1-stable, v2.2-stable.
* [json-c](https://github.com/json-c/json-c).
* [libuuid](https://sourceforge.net/projects/libuuid/)

## Building
```
autoreconf --force --install
./configure
make
```

## Using
Start by running `exampleProgram` to learn how to use the library:
```
Usage: exampleProgram [OPTION...] <key> <language>
  -d			Produce debug output.
  -f FILE		Audio input file, stdin if omitted.
  -m MODE		Recognition mode:
  -p MODE		Set profanity handling mode {raw|masked|removed}. Default is masked.
			{interactive|dictation|conversation}. Default is interactive.
  -t			Request detailed recognition output.

```

To recognize a file:
```
exampleProgram -f <path to wav> -m interactive <your subscription key> en-us
```

On Linux, you can stream audio directly from microphone using Debian `alsa-utils`:
```
arecord -c 1 -r 16000 -f S16_LE | ./exampleProgram -m interactive <your subscription key> en-us
```
or perform long dictation on Steve Jobs Standford University commencement speech:
```
curl -L -s https://archive.org/download/SteveJobsSpeechAtStanfordUniversity/SteveJobsSpeech_64kb.mp3 | \
mpg123 -w - -m -r 16000 -e s16 - | \
./exampleProgram -m dictation <your subscription key> en-us
```

More explanation and details on how to use the library can be found in this [blog post](https://hashifdef.wordpress.com/2017/05/29/getting-started-with-microsoft-speech-recognition-under-unix/).
