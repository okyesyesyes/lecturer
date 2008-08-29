/*
    Lecturer: font.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include "font.h"
#include "screen.h"

#include <assert.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

const char* font_names[] = {
  "fonts/font8", "fonts/font10", "fonts/font12", "fonts/font14", "fonts/font16"
};

FONT fonts[] = {NULL, NULL, NULL, NULL, NULL};

struct glyph* getglyph(unsigned char* font, int c)
{
  assert(c >= 32);
  struct glyph* g = (struct glyph*)(font + 8);
  return &g[c - 32];
}

int glyphwidth(unsigned char* font, int c)
{
  assert(c >= 32);
  return getglyph(font, c)->x;
}

int stringwidth(unsigned char* font, char* str)
{
  int size = 0;
  for(;*str;str++)
    if (*str >= ' ') size += glyphwidth(font, *str);
  return size;
}

int fontheight(unsigned char* font)
{
  return getglyph(font, ' ')->y;
}

int render_char_ex(unsigned char* font, int x, int y, int c, int contrast)
{
  int i,j;
  struct glyph* curglyph = getglyph(font, c);
  //printf("curglyph x %d y %d offset %d\n",curglyph->x,curglyph->y, curglyph->offset);
  for(i = 0; i < curglyph->y ; i++) {
    for(j = 0; j < curglyph->x; j++) {
      //printf("%d ",font[curglyph->offset+j+i*curglyph->y]);
      unsigned int gray = (255 - ((255 - font[curglyph->offset + j + i * curglyph->x]) * contrast / 256)) >> 2;
      *((unsigned short*)(fb + (y+i) * pitch + (x + j) * 2)) = ((gray >> 1) << 11) | (gray << 5) | (gray >> 1);
      //fb[(fby+i)*pitch+fbx*2+j*2] = font[curglyph->offset+j+i*curglyph->x];
      //fb[(fby+i)*pitch+fbx*2+j*2+1] = font[curglyph->offset+j+i*curglyph->x];
    }
  }
  return curglyph->x;
}

int render_char(unsigned char* font, int x, int y, int c)
{
  return render_char_ex(font, x, y, c, 256);
}

int render_string_ex(unsigned char* font, int x, int y, char* str, int contrast)
{
  for(; *str; str++) {
    x += render_char_ex(font, x, y, *str, contrast);
  }
  return x;
}

int render_string(unsigned char* font, int x, int y, char* str)
{
  return render_string_ex(font, x, y, str, 256);
}

FONT get_font(int idx)
{
  int fd;
  struct stat stbuf;
  if (fonts[idx]) return fonts[idx];
  if (stat(font_names[idx], &stbuf) < 0) {
    perror("statting font");
    exit(1);
  }
  fd = open(font_names[idx],O_RDONLY);
  if (fd < 0) {
    perror("opening font");
    exit(1);
  }
  fonts[idx] = mmap(0, stbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (!fonts[idx]) {
    perror("mmapping font");
    exit(1);
  }
  return fonts[idx];
}
