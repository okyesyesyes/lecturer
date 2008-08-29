/*
    Lecturer: screen_common.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include <string.h>
#include "conf.h"
#include "font.h"
#include "screen.h"
#include "input.h"

void clear_screen(void)
{
  memset(fb, 0xff, screeny * pitch);
}

int screenlines(FONT font)
{
  return (screeny - conf.marginy[0] - conf.marginy[1]) / (fontheight(font) + conf.linespacing * fontheight(font) / 100);
}

void msg(char* text)
{
  int x, y;
  clear_screen();
  render_string(get_font(FONT_12), 0, 0, text);
  while (!get_click(&x, &y, NULL)) {}
}
