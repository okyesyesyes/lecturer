/*
    Lecturer: speech.h
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

void init_speech(void);
void read_text(char* text);
void stop_speech(void);

extern int speech_timeout;
