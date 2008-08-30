/*
    Lecturer: screen.h
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include "font.h"

extern int screenx;
extern int screeny;
extern int bpp;
extern int pitch;
extern unsigned char* fb;

void screen_init(void);

void clear_screen(void);
int screenlines(FONT font);

