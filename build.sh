#!/bin/bash
# I tried using GNU make for this, but it completely failed to behave like expected, even
# though I copied the code almost verbatim from another Makefile of mine. Screw it!
BINARY=lecturer
set -e
set -x
echo "#define VERSION \"`svn info|grep ^Revision`\"" >version.h
CFLAGS="-O2 -g -Wall -I."
COMMON="main.c font.c ui.c conf.c screen_common.c encoding.c"
arm-linux-gcc $CFLAGS -o ${BINARY}_tomtom screen_fb.c input_ts.c speech_loq.c $COMMON -LLoquendoTTS/bin -lLoqTTS7 -lLTTS7Util
gcc $CFLAGS `sdl-config --cflags` -o ${BINARY}_sdl screen_sdl.c input_sdl.c speech_null.c $COMMON `sdl-config --libs`
