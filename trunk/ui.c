/*
    Lecturer: ui.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include "conf.h"
#include "ui.h"

#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "screen.h"
#include "input.h"

FONT uifont;
FONT diafont;
FONT smallfont;

void init_ui(void)
{
  uifont = get_font(FONT_12);
  diafont = get_font(FONT_16);
  smallfont = get_font(FONT_SMALL);
}

static int compdirent(const void* one, const void* two)
{
  return strcmp(((struct dirent*)one)->d_name, ((struct dirent*)two)->d_name);
}

char* file_dialog(char* dir)
{
  char* curdir = strdup(dir);
  struct dirent entries[screenlines(diafont)];
  struct dirent* de;
restart:
  de = NULL;
  DIR* d = opendir(curdir);
  if (!d) {
    perror("opendir");
    exit(1);
  }
  for(;;) {
    int x = conf.marginx[0];
    int y = conf.marginy[0];
    int dirp = 0;
    int i;
    int tsx, tsy;
    clear_screen();
    draw_button(UI_TOP_RIGHT, "Back");
    memset(entries, 0, sizeof(entries));
    while (dirp < screenlines(diafont) && (de = readdir(d))) {
      if ( (strlen(de->d_name) > 5 && !strcmp(de->d_name + strlen(de->d_name) - 5, ".conf")) || /* hide .conf files */
           de->d_name[0] == '.' ) /* hide hidden entries */
        continue;
      entries[dirp] = *de;
      dirp++;
    }
    qsort((void*)entries, dirp, sizeof(struct dirent), compdirent);
    for (i = 0; i < dirp; i++) {
      render_string(diafont, x, y, entries[i].d_name);
      y += fontheight(diafont) + conf.linespacing * fontheight(diafont) / 100;
    }
    if (de) draw_button(UI_BOTTOM_RIGHT, "More");
click_again:
    while (!get_click(&tsx, &tsy, NULL)) {}
    if (tsx > UI_RIGHT_AREA && tsy < UI_TOP_AREA) {
      goto out;
    }
    if (tsx < UI_RIGHT_AREA || tsy < UI_BOTTOM_AREA) {
      if (tsx > conf.marginx[0] && tsx < screenx - conf.marginx[1] && tsy > conf.marginy[0] && tsy < screeny - conf.marginy[1]) {
        tsy -= conf.marginy[0];
        tsy /= fontheight(diafont) + conf.linespacing * fontheight(diafont) / 100;
        struct dirent* entry = &entries[tsy];
        if (entry->d_type == DT_DIR) {
          char* newdir = malloc(strlen(curdir) + 1 /* sep */ + strlen(entry->d_name) + 1 /* null */);
          sprintf(newdir, "%s/%s", curdir, entry->d_name);
          free(curdir);
          curdir = newdir;
          goto restart;
        }
        if (!entry->d_type == DT_UNKNOWN) {
          char* path = malloc(strlen(entries[tsy].d_name) + 1 /* slash */ + strlen(curdir) + 1 /* null byte */);
          sprintf(path, "%s/%s", curdir, entries[tsy].d_name);
          closedir(d);
          free(curdir);
          return path;
        }
        else goto click_again;
      }
    }
    if (!de) rewinddir(d);
  }
out:
  free(curdir);
  closedir(d);
  return NULL;
}

static void box(int x, int y, int w, int h)
{
  int xx;
  int yy;
  for (xx = x ; xx < x + w; xx++) {
    fb[y*pitch + xx * 2] = 0;
    fb[(y+h)*pitch + xx * 2] = 0;
    fb[y*pitch + xx * 2 + 1] = 0;
    fb[(y+h)*pitch + xx * 2 + 1] = 0;
  }
  for (yy = y; yy < y + h; yy++) {
    fb[yy*pitch + x * 2] = 0;
    fb[yy*pitch + (x+w) * 2] = 0;
    fb[yy*pitch + x * 2 + 1] = 0;
    fb[yy*pitch + (x+w) * 2 + 1] = 0;
  }
}

static void hline(int x, int y, int len)
{
  int xx;
  for(xx = x; xx < x + len; xx++) {
    fb[y*pitch + xx*2] = 0;
    fb[y*pitch + xx*2 + 1] = 0;
  }
}

static void vline(int x, int y, int len)
{
  int yy;
  for(yy = y; yy < y + len; yy++) {
    fb[yy*pitch + x*2] = 0;
    fb[yy*pitch + x*2 + 1] = 0;
  }
}

#define BOX_SIZE 30
#define SIGN_SIZE 20

static void draw_plus(int x, int y)
{
    box(x, y, BOX_SIZE, BOX_SIZE);
    hline(x + (BOX_SIZE - SIGN_SIZE) / 2, y + BOX_SIZE / 2, SIGN_SIZE);
    vline(x + BOX_SIZE / 2, y + (BOX_SIZE - SIGN_SIZE) / 2, SIGN_SIZE);
}

static void draw_minus(int x, int y)
{
    box(x, y, BOX_SIZE, BOX_SIZE);
    hline(x + (BOX_SIZE - SIGN_SIZE) / 2, y + BOX_SIZE / 2, SIGN_SIZE);
}

#define CHECKBOX_SIZE 20

static void draw_checkbox(int x, int y, int ticked)
{
    if (ticked)
      render_string(diafont, x + (CHECKBOX_SIZE / 2) - getglyph(diafont, 'X')->x / 2, y, "X");
    box(x, y, CHECKBOX_SIZE, CHECKBOX_SIZE);
}

void config_dialog(void)
{
  char buf[256];
  int x,y;
#define SPACING 30
#define LOWSPACING 20
#define BOXSPACING 40
#define TITLE 20
#define TITLE_X 400
#define LEFT 50
#define MARGINS (TITLE)
#define MARGINS_BTN (MARGINS+LOWSPACING)
#define MARGINS_1 (LEFT)
#define MARGINS_1_NUM (LEFT+40)
#define MARGINS_1_PLUS (MARGINS_1_NUM+40)
#define MARGINS_1_MINUS (MARGINS_1_PLUS+40)
#define MARGINS_2 (MARGINS_1_MINUS+50)
#define MARGINS_2_NUM (MARGINS_2+70)
#define MARGINS_2_PLUS (MARGINS_2_NUM+40)
#define MARGINS_2_MINUS (MARGINS_2_PLUS+40)
#define MARGINSB_BTN (MARGINS_BTN+BOXSPACING)
#define SPACINGS (MARGINSB_BTN+SPACING)
#define SPACINGS_BTN (SPACINGS+LOWSPACING)
#define INDENTLINE (SPACINGS_BTN+BOXSPACING)
#define INDENTLINE_BTN (INDENTLINE + LOWSPACING)
#define CENTER (LEFT + 120)
#define RIGHT (CENTER + 150)
#define INDENTLINE_LEFT_PLUS (LEFT + 30)
#define INDENTLINE_LEFT_MINUS (INDENTLINE_LEFT_PLUS + 40)
#define INDENTLINE_CENTER_PLUS (CENTER + 30)
#define INDENTLINE_CENTER_MINUS (INDENTLINE_CENTER_PLUS + 40)
#define INDENTLINE_RIGHT_PLUS (RIGHT + 30)
#define INDENTLINE_RIGHT_MINUS (INDENTLINE_RIGHT_PLUS + 40)
#define OPTIONS (INDENTLINE_BTN + BOXSPACING)

  for(;;) {
    clear_screen();
    render_string(diafont, TITLE_X, TITLE, "Options");
    render_string(uifont, LEFT, MARGINS, "Margins");

    render_string(uifont, MARGINS_1, MARGINS_BTN, "Left");

    sprintf(buf, "%3d", conf.marginx[0]);
    render_string(uifont, MARGINS_1_NUM, MARGINS_BTN, buf);
    draw_plus(MARGINS_1_PLUS, MARGINS_BTN);
    draw_minus(MARGINS_1_MINUS, MARGINS_BTN);

    render_string(uifont, MARGINS_2, MARGINS_BTN, "Right");

    sprintf(buf, "%3d", conf.marginx[1]);
    render_string(uifont, MARGINS_2_NUM, MARGINS_BTN, buf);
    draw_plus(MARGINS_2_PLUS, MARGINS_BTN);
    draw_minus(MARGINS_2_MINUS, MARGINS_BTN);

    render_string(uifont, MARGINS_1, MARGINSB_BTN, "Top");

    sprintf(buf, "%3d", conf.marginy[0]);
    render_string(uifont, MARGINS_1_NUM, MARGINSB_BTN, buf);
    draw_plus(MARGINS_1_PLUS, MARGINSB_BTN);
    draw_minus(MARGINS_1_MINUS, MARGINSB_BTN);

    render_string(uifont, MARGINS_2, MARGINSB_BTN, "Bottom");

    sprintf(buf, "%3d", conf.marginy[1]);
    render_string(uifont, MARGINS_2_NUM, MARGINSB_BTN, buf);
    draw_plus(MARGINS_2_PLUS, MARGINSB_BTN);
    draw_minus(MARGINS_2_MINUS, MARGINSB_BTN);

    render_string(uifont, LEFT, SPACINGS, "Spacing");
    
    render_string(uifont, MARGINS_1, SPACINGS_BTN, "Line");
    
    sprintf(buf, "%3d%%", conf.linespacing);
    render_string(uifont, MARGINS_1_NUM, SPACINGS_BTN, buf);
    draw_plus(MARGINS_1_PLUS, SPACINGS_BTN);
    draw_minus(MARGINS_1_MINUS, SPACINGS_BTN);
    
    render_string(uifont, MARGINS_2, SPACINGS_BTN, "Paragraph");
    
    sprintf(buf, "%3d%%", conf.paraspacing);
    render_string(uifont, MARGINS_2_NUM, SPACINGS_BTN, buf);
    draw_plus(MARGINS_2_PLUS, SPACINGS_BTN);
    draw_minus(MARGINS_2_MINUS, SPACINGS_BTN);
    
    render_string(uifont, LEFT, INDENTLINE, "Indentation");
    render_string(uifont, CENTER, INDENTLINE, "Empty line height");
    render_string(uifont, RIGHT, INDENTLINE, "Font size");
    
    sprintf(buf, "%2d%%", conf.paraindentation);
    render_string(uifont, LEFT, INDENTLINE_BTN, buf);
    draw_plus(INDENTLINE_LEFT_PLUS, INDENTLINE_BTN);
    draw_minus(INDENTLINE_LEFT_MINUS, INDENTLINE_BTN);
    
    sprintf(buf, "%2d%%", conf.emptylineheight);
    render_string(uifont, CENTER, INDENTLINE_BTN, buf);
    draw_plus(INDENTLINE_CENTER_PLUS, INDENTLINE_BTN);
    draw_minus(INDENTLINE_CENTER_MINUS, INDENTLINE_BTN);
    
    render_char(get_font(conf.textfont), RIGHT, INDENTLINE_BTN, 'A');
    draw_plus(INDENTLINE_RIGHT_PLUS, INDENTLINE_BTN);
    draw_minus(INDENTLINE_RIGHT_MINUS, INDENTLINE_BTN);
    
    draw_checkbox(LEFT, OPTIONS, conf.progressbar);
    render_string(uifont, LEFT + CHECKBOX_SIZE + 10, OPTIONS, "Progress bar");
    
    draw_checkbox(CENTER, OPTIONS, conf.pagenumber);
    render_string(uifont, CENTER + CHECKBOX_SIZE + 10, OPTIONS, "Page numbering");
    
    draw_checkbox(RIGHT, OPTIONS, conf.justify);
    render_string(uifont, RIGHT + CHECKBOX_SIZE + 10, OPTIONS, "Justification");
    
    draw_button(UI_TOP_RIGHT, "Back");
    draw_button(UI_TOP_LEFT, "Save as default");
    draw_button(UI_BOTTOM_LEFT, "Reset");

    box(conf.marginx[0], conf.marginy[0], screenx - conf.marginx[0] - conf.marginx[1], screeny - conf.marginy[0] - conf.marginy[1]);
    while (!get_click(&x, &y, NULL)) {}
    if (y > OPTIONS && y < OPTIONS + CHECKBOX_SIZE) {
      /* options */
      if (x > LEFT && x < LEFT+CHECKBOX_SIZE) conf.progressbar = -(conf.progressbar - 1);
      else if (x > CENTER && x < CENTER + CHECKBOX_SIZE) conf.pagenumber = -(conf.pagenumber - 1);
      else if (x > RIGHT && x < RIGHT + CHECKBOX_SIZE) conf.justify = -(conf.justify - 1);
    }
    else if (y > MARGINS_BTN && y < MARGINS_BTN + BOX_SIZE) {
      /* left/right margin */
      if (x > MARGINS_1_PLUS && x < MARGINS_1_PLUS + BOX_SIZE && conf.marginx[0] < screenx / 3)
        conf.marginx[0]++;
      else if (x > MARGINS_1_MINUS && x < MARGINS_1_MINUS + BOX_SIZE && conf.marginx[0] > 0)
        conf.marginx[0]--;
      else if (x > MARGINS_2_PLUS && x < MARGINS_2_PLUS + BOX_SIZE && conf.marginx[1] < screenx / 3)
        conf.marginx[1]++;
      else if (x > MARGINS_2_MINUS && x < MARGINS_2_MINUS + BOX_SIZE && conf.marginx[1] > 0)
        conf.marginx[1]--;
    }
    else if (y > MARGINSB_BTN && y < MARGINSB_BTN + BOX_SIZE) {
      /* top/bottom margin */
      if (x > MARGINS_1_PLUS && x < MARGINS_1_PLUS + BOX_SIZE && conf.marginy[0] < screeny / 3)
        conf.marginy[0]++;
      else if (x > MARGINS_1_MINUS && x < MARGINS_1_MINUS + BOX_SIZE && conf.marginy[0] > 0)
        conf.marginy[0]--;
      else if (x > MARGINS_2_PLUS && x < MARGINS_2_PLUS + BOX_SIZE && conf.marginy[1] < screeny / 3)
        conf.marginy[1]++;
      else if (x > MARGINS_2_MINUS && x < MARGINS_2_MINUS + BOX_SIZE && conf.marginy[1] > 0)
        conf.marginy[1]--;
    }
    else if (y > SPACINGS_BTN && y < SPACINGS_BTN + BOX_SIZE) {
      /* spacings */
      if (x > MARGINS_1_PLUS && x < MARGINS_1_PLUS + BOX_SIZE && conf.linespacing < 99)
        conf.linespacing++;
      else if (x > MARGINS_1_MINUS && x < MARGINS_1_MINUS + BOX_SIZE && conf.linespacing > -99)
        conf.linespacing--;
      else if (x > MARGINS_2_PLUS && x < MARGINS_2_PLUS + BOX_SIZE && conf.paraspacing < 99)
        conf.paraspacing++;
      else if (x > MARGINS_2_MINUS && x < MARGINS_2_MINUS + BOX_SIZE && conf.paraspacing > -99)
        conf.paraspacing--;
    }
    else if (y > INDENTLINE_BTN && y < INDENTLINE_BTN + BOX_SIZE) {
      /* indentation, empty line height, font size */
      if (x > INDENTLINE_LEFT_PLUS && x < INDENTLINE_LEFT_PLUS + BOX_SIZE && conf.paraindentation < 200)
        conf.paraindentation++;
      else if (x > INDENTLINE_LEFT_MINUS && x < INDENTLINE_LEFT_MINUS + BOX_SIZE && conf.paraindentation > 0)
        conf.paraindentation--;
      else if (x > INDENTLINE_CENTER_PLUS && x < INDENTLINE_CENTER_PLUS + BOX_SIZE && conf.emptylineheight < 200)
        conf.emptylineheight++;
      else if (x > INDENTLINE_CENTER_MINUS && x < INDENTLINE_CENTER_MINUS + BOX_SIZE && conf.emptylineheight > 0)
        conf.emptylineheight--;
      else if (x > INDENTLINE_RIGHT_PLUS && x < INDENTLINE_RIGHT_PLUS + BOX_SIZE && conf.textfont < FONT_16)
        conf.textfont++;
      else if (x > INDENTLINE_RIGHT_MINUS && x < INDENTLINE_RIGHT_MINUS + BOX_SIZE && conf.textfont > FONT_10)
        conf.textfont--;
      printf("textfont %d\n",conf.textfont);
    }
    else if (y < UI_TOP_AREA && x > UI_RIGHT_AREA) {
      /* back */
      break;
    }
    else if (y < UI_TOP_AREA && x < UI_LEFT_AREA) {
      default_conf = conf;
      write_global_conf();
      msg("Saved.");
    }
    else if (y > UI_BOTTOM_AREA && x < UI_LEFT_AREA) {
      conf = default_conf;
    }
  }
}

void draw_button(int position, char* label)
{
  int x, y;
  if (position == UI_TOP_LEFT || position == UI_TOP_RIGHT || position == UI_TOP_CENTER) y = 0;
  else y = screeny - fontheight(smallfont);
  if (position == UI_TOP_LEFT || position == UI_BOTTOM_LEFT) x = 0;
  else if (position == UI_TOP_CENTER) x = (screenx - stringwidth(smallfont, label)) / 2;
  else x = screenx - stringwidth(smallfont, label);
  render_string_ex(smallfont, x, y, label,100);
}
