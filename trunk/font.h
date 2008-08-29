/*
    Lecturer: font.h
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#ifndef _FONT_H
#define _FONT_H

typedef unsigned char* FONT;

#define FONT_SMALL 0
#define FONT_10 1
#define FONT_12 2
#define FONT_14 3
#define FONT_16 4
#define FONT_MAX 5

struct glyph {
  unsigned short x;
  unsigned short y;
  int offset;
};

struct glyph* getglyph(FONT font, int c);
int glyphwidth(FONT font, int c);
int stringwidth(FONT font, char* str);
int fontheight(FONT font);
int render_char(FONT font, int x, int y, int c);
int render_char_ex(FONT font, int x, int y, int c, int contrast);
int render_string(FONT font, int x, int y, char* str);
int render_string_ex(FONT font, int x, int y, char* str, int contrast);
FONT get_font(int idx);

#endif
