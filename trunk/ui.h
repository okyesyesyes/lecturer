/*
    Lecturer: ui.h
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include "font.h"

extern FONT uifont;
extern FONT smallfont;

void init_ui(void);
char* file_dialog(char* dir);
int config_dialog(void);

#define UI_TOP_LEFT 0
#define UI_TOP_RIGHT 1
#define UI_BOTTOM_LEFT 2
#define UI_BOTTOM_RIGHT 3
#define UI_TOP_CENTER 4

#define UI_TOP_AREA 40
#define UI_RIGHT_AREA (screenx - 80)
#define UI_LEFT_AREA 80
#define UI_BOTTOM_AREA (screeny - 40)
#define UI_CENTER_AREA_LEFT (screenx / 2 - 40)
#define UI_CENTER_AREA_RIGHT (screenx / 2 + 40)

void draw_button(int position, char* label);

#define REDO_PAGINATION (1 << 0)
#define REDO_SPEECHINIT (1 << 1)

void modal_msg(char* text);
void splash_msg(char* text);
