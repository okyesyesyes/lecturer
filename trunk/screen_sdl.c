/*
    Lecturer: screen_sdl.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include <SDL/SDL.h>

int screenx, screeny, bpp, pitch;
unsigned char* fb;
SDL_Surface* surf;

void screen_init(void)
{
  SDL_Init(SDL_INIT_VIDEO);
  atexit(SDL_Quit);
  surf = SDL_SetVideoMode(480, 272, 16, SDL_SWSURFACE);
  if (!surf) {
    fprintf(stderr,"SDL_SetVideoMode: %s\n", SDL_GetError());
    exit(1);
  }
  fb = surf->pixels;
  screenx = surf->w;
  screeny = surf->h;
  pitch = surf->pitch;
  bpp = 2;
}
