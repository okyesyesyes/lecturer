/*
    Lecturer: input_sdl.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include <SDL/SDL.h>

extern SDL_Surface* surf;

void input_init(void)
{
}

int get_click(int* x, int* y, int* breakon)
{
  static int clicked = 0;
  SDL_Event ev = {0};
  SDL_Flip(surf);
  if (clicked) while (ev.type != SDL_MOUSEBUTTONUP) {}
  clicked = 0;
  while (!clicked) {
    SDL_WaitEvent(&ev);
    if (ev.type == SDL_MOUSEBUTTONDOWN) {
      SDL_GetMouseState(x, y);
      return 1;
    }
  }
  return 0;
}
