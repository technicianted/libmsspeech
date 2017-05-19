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
exampleProgram <azure subscription key> <wav file>
```

