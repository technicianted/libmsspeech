# libmsspeech
Library for Microsoft Cognitive Services [speech recognition](https://docs.microsoft.com/en-us/azure/cognitive-services/speech/home).

> __This is the very first version that works. Do not use it in any serious application yet!__

## Prerequisites
* [libwebsockets](https://libwebsockets.org) at least v2.1-stable, v2.2-stable.
* [json-c](https://github.com/json-c/json-c).

## Building
```
autoreconf --force --install
./configure
make
```

## Using
Start by running `exampleProgram` to learn how to use the library:
```
exampleProgram --help
Usage: exampleProgram [OPTION...]
<key> <language>

  -d, --debug                Produce debug output
  -f, --file=FILE            Audio input file, stdin if omitted
  -m, --mode=MODE            Recognition mode:
                             {interactive|dictation|conversation}. Default is
                             interactive
  -t, --details              Request detailed recognition output
  -?, --help                 Give this help list
      --usage                Give a short usage message

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

```

To recognize a file:
```
exampleProgram -f <path to wav> -m interactive <your subscription key> en-us
```

On Linux, you can stream audio directly from microphone using Debian `alsa-utils`:
```
arecord -c 1 -r 16000 -f S16_LE | ./exampleProgram -m interactive <your subscription key> en-us
```
or perform long dictation:
```
arecord -c 1 -r 16000 -f S16_LE | ./exampleProgram -m dictation <your subscription key> en-us
```
