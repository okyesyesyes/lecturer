/*
    Lecturer: speech_null.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include <stdio.h>

int speech_timeout;

void init_speech(void)
{
  speech_timeout = 0;
}

void read_text(char* text)
{
  printf("reading: %s\n", text);
}

void stop_speech(void)
{
}
