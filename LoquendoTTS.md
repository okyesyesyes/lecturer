TomTom provides owners of the Go 730 and 930 GPS navigation units with so-called "computer voices". These are in fact complete text-to-speech systems by [Loquendo](http://www.loquendo.com/) that can be used in custom applications as well. These voices are installed in the directory `LoquendoTTS` in your unit's internal storage. You need to copy that directory off your device if you want to build Lecturer for x30 systems. The voices are not available for public download.

A bigger problem is that the necessary SDK is not freely available, so you have to guess function prototypes and calling conventions. Very few code using this TTS is available on the net. The only [example program for version 7](http://www.voip-info.org/wiki/view/Loquendo+TTS) that I have come across has proven to be extremely helpful, however. The [current implementation in Lecturer](http://code.google.com/p/lecturer/source/browse/trunk/speech_loq.c) should give you more hints, for instance on using callbacks.

Both implementations, however, only use a small subset of the functions available, while the TTS contains a lot of useful functionality that currently goes to waste for lack of information on how to use it:

  * Enumeration of available voices (this can probably be emulated by scanning the directories; see [issue #5](https://code.google.com/p/lecturer/issues/detail?id=#5))
  * Language detection
  * Adding additional words to the lexicon

There is probably a lot more. If you come across any freely available information on LTTS7, please leave a note.