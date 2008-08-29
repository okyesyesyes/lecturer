/*
    Lecturer: speech_loq.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include "screen.h"
#include <stdio.h>
#include <stdlib.h>

/* These constants and prototypes are guesswork and may be wrong. */
typedef void* ttsHandleType;
typedef int ttsResultType;
#define ttsOK 0
#define ttsFALSE 0
#define ttsTRUE 1

#define tts_LINEAR 0
#define tts_MONO 0

ttsResultType ttsNewSession(ttsHandleType*, char*);
char* ttsGetErrorMessage(ttsResultType);
void ttsDeleteSession(ttsHandleType);
ttsResultType ttsNewReader(ttsHandleType*, ttsHandleType);
ttsResultType ttsLoadPersona(ttsHandleType, char*, void*, void*);
ttsResultType ttsSetAudio(ttsHandleType, char*, char*, int, int, int, int);
ttsResultType ttsRead(ttsHandleType, char*, int, int, void*);
ttsResultType ttsWaitForEndOfSpeech(ttsHandleType);
ttsResultType ttsStop(ttsHandleType);
ttsResultType ttsSetCallback(ttsHandleType, void* callback, void* user, int type);

ttsHandleType hSession = NULL;
ttsHandleType hReader = NULL;

int speech_timeout;

static int callback(int eins, int zwei, int drei, int vier, int fuenf)
{
  if (zwei == 1) {
    ttsWaitForEndOfSpeech(hReader);
    speech_timeout = 1;
  }
  return 0;
}

static void cleanup(void)
{
  ttsDeleteSession(NULL);
}

void init_speech(void)
{
  int r;
  speech_timeout = 0;
  r = ttsNewSession(&hSession, "/mnt/sdcard/LoquendoTTS/default7.session");
  if (r != ttsOK) {
    msg(ttsGetErrorMessage(r));
    ttsDeleteSession(NULL);
    return;
  }
  r = ttsNewReader(&hReader, hSession);
  if (r != ttsOK) {
    msg(ttsGetErrorMessage(r));
    ttsDeleteSession(NULL);
    return;
  }
  r = ttsLoadPersona(hReader, "Katrin", NULL, NULL);
  if (r != ttsOK) {
    msg(ttsGetErrorMessage(r));
    ttsDeleteSession(NULL);
    return;
  }
  r = ttsSetAudio(hReader, "LTTS7AudioBoard", "bla", 16000, 0, 1, 0);
  if (r != ttsOK) {
    msg(ttsGetErrorMessage(r));
    ttsDeleteSession(NULL);
    return;
  }
  r = ttsSetCallback(hReader, callback, (void*)0xdeadbeef, 0);
  if (r != ttsOK) {
    msg(ttsGetErrorMessage(r));
    ttsDeleteSession(NULL);
    return;
  }
  atexit(cleanup);
}

void read_text(char* text)
{
  int r;
  r = ttsRead(hReader, text, ttsTRUE, ttsFALSE, NULL);
  if (r != ttsOK) {
    msg(ttsGetErrorMessage(r));
  }
}

void stop_speech(void)
{
  ttsStop(hReader);
}
