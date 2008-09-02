/*
    Lecturer: encoding.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include "encoding.h"

#include <stdio.h>
#include <stdlib.h>

unsigned char* utf8_to_latin1(unsigned char* text, int* length)
{
  int i;
  int singlebyte = 0;
  int multibyte = 0;
  if (*length < 3) return NULL;
  for (i = 0; i < *length-3; i++) {
    if (text[i] >= 128) {
      if (text[i+1] < 128)
        singlebyte++;
      else
        multibyte++;
      while(text[i] >= 128) i++;
    }
  }
  printf("singlebyte %d multibyte %d\n", singlebyte, multibyte);
  if (singlebyte > multibyte) return NULL; /* assuming ISO 8859-1 text */
  
  printf("converting utf-8 -> latin1\n");
  /* convert from UTF-8 to ISO 8859-1 */
  unsigned char* convtext = malloc(*length); /* single-byte encoding is always smaller */
  unsigned char* cc = convtext;
  unsigned char* tt = text;
  int ch;
  while (tt < text + *length) {
    if (*tt < 128) {	/* ASCII character, use as is */
      *cc++ = *tt++;
      continue;
    }
    if (*tt > 0xe0) {	/* unicode character > 0x7ff -> play dumb */
      *cc++ = '?';
      tt++;
      continue;
    }
    if (tt + 1 >= text + *length) { /* text end in mid-character */
      *cc++ = '?';
      break;
    }
    ch = ((tt[0] & 0x1f) << 6) | (tt[1] & 0x3f);
    if (ch > 255) {
      *cc++ = '?';
    }
    else *cc++ = ch;
    tt += 2;
  }
  printf("previous length %d\n", *length);
  *length = cc - convtext;
  printf("new length %d\n", *length);
  return convtext;
}
