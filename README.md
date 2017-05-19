# libmsspeech
Library for Microsoft Cognitive Services [speech recognition](https://docs.microsoft.com/en-us/azure/cognitive-services/speech/home).

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
exampleProgram <wav file>
```

