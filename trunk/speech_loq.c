/*
    Lecturer: speech_loq.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include "screen.h"
#include "conf.h"
#include "speech.h"
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

/* These constants and prototypes are guesswork and may be wrong. */
typedef void* ttsHandleType;
typedef int ttsResultType;
#define ttsOK 0
#define ttsFALSE 0
#define ttsTRUE 1

ttsResultType ttsNewSession(ttsHandleType* session, char* session_file);
char* ttsGetErrorMessage(ttsResultType);
void ttsDeleteSession(ttsHandleType); /* no idea what kind of handle this takes; we always use it with
                                         NULL, which seems to destroy everything */
ttsResultType ttsNewReader(ttsHandleType* reader, ttsHandleType session);
ttsResultType ttsLoadPersona(ttsHandleType reader, char* voice_name, void*, void*); /* last two params unknown; we use NULL */
ttsResultType ttsSetAudio(ttsHandleType reader, char* module_name,
                          char* output_file, /* a file name or a device; LTTS7AudioBoard on the Go 730 seems to ignore this */
                          int sample_rate,
                          int sample_type,   /* text2audio.c uses tts_LINEAR; no idea what that defines to; we use 0 (which works) */
                          int channels,	     /* text2audio.c uses tts_MONO; we use 1 */
                          int);		     /* unknown; we use 0 */
ttsResultType ttsRead(ttsHandleType reader,
                      char* text_or_file,    /* either the text to read or the name of a file with the text to read */
                      int noblock,	     /* if set to ttsFALSE, ttsRead() blocks until it has finished talking; otherwise, it returns immediately */
                      int read_from_file,    /* if set to ttsTRUE, text_or_file is treated as a file name, otherwise as the text to read */
                      void*);		     /* unknown, but most probably a pointer (text2audio.c uses NULL) */
ttsResultType ttsWaitForEndOfSpeech(ttsHandleType);	/* pure guesswork */
ttsResultType ttsStop(ttsHandleType reader); /* stops talking immediately */
ttsResultType ttsSetCallback(ttsHandleType reader,
                             void* callback,		/* function called when certain events occur */
                             void* user,		/* user pointer passed to the callback (don't remember how, though) */
                             int type);			/* type of event; only 0 seems to yield a result; there is one other
                                                           legal type (don't remember which) that never calls, and two
                                                           that yield an error saying that they are only implemented under Windows */

ttsHandleType hSession = NULL;
ttsHandleType hReader = NULL;
int current_lang = -1;

int speech_timeout;

static int callback(int eins, int zwei, int drei, int vier, int fuenf)
{
  if (zwei == 1) {
    /* This event occurs at some point near the end of speech. It is usually
       a few words early. One possible explanation is that the audio backend
       module reports completion once it has sent the audio data to the
       master control program via IPC, but that does not necessarily mean
       that the same program has already output them. Not sure, though.
       In any case, no other event is triggered at the end, so this is the
       best we can get. */
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
  if (hSession && current_lang == conf.speech_lang)
    return;
  splash_msg("Initializing TTS...");
  speech_timeout = 0;
  if (hSession) {
    cleanup();
    hSession = hReader = NULL;
  }
  r = ttsNewSession(&hSession, "/mnt/sdcard/LoquendoTTS/default7.session");
  if (r != ttsOK) {
    modal_msg(ttsGetErrorMessage(r));
    ttsDeleteSession(NULL);
    return;
  }
  r = ttsNewReader(&hReader, hSession);
  if (r != ttsOK) {
    modal_msg(ttsGetErrorMessage(r));
    ttsDeleteSession(NULL);
    return;
  }
  get_languages(); /* corrects conf.speech_lang if necessary */
  printf("loading voice %s\n", get_languages()[conf.speech_lang]);
  r = ttsLoadPersona(hReader, get_languages()[conf.speech_lang], NULL, NULL);
  if (r != ttsOK) {
    modal_msg(ttsGetErrorMessage(r));
    ttsDeleteSession(NULL);
    return;
  }
  current_lang = conf.speech_lang;
  r = ttsSetAudio(hReader, "LTTS7AudioBoard", "bla", 16000, 0, 1, 0);
  if (r != ttsOK) {
    modal_msg(ttsGetErrorMessage(r));
    ttsDeleteSession(NULL);
    return;
  }
  r = ttsSetCallback(hReader, callback, (void*)0xdeadbeef, 0);
  if (r != ttsOK) {
    modal_msg(ttsGetErrorMessage(r));
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
    modal_msg(ttsGetErrorMessage(r));
  }
}

void stop_speech(void)
{
  ttsStop(hReader);
}

static char** langs = NULL;
char** get_languages(void)
{
  if (!langs) {
    int lcount = 0;
    DIR* d = opendir("/mnt/sdcard/LoquendoTTS/data");
    struct dirent* de;
    if (!d) {
      modal_msg("failed to open /mnt/sdcard/LoquendoTTS/data");
      exit(1);
    }
    while ((de = readdir(d))) {
      if (strlen(de->d_name) <= 4 || strcmp(de->d_name + strlen(de->d_name) - 4, ".vcf"))
        continue;
      de->d_name[strlen(de->d_name) - 4] = 0;
      langs = realloc(langs, (lcount + 2) * sizeof(char*));
      langs[lcount] = strdup(de->d_name);
      printf("found language %s\n", langs[lcount]);
      lcount++;
    }
    langs[lcount] = NULL;
    if (conf.speech_lang > lcount - 1) conf.speech_lang = 0;
    printf("%d languages found\n", lcount);
    closedir(d);
  }
  return langs;
}
