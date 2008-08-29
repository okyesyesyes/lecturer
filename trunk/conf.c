/*
    Lecturer: conf.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include "conf.h"
#include "font.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* filename;

struct conf_s conf = {
  .marginx = {10, 10},
  .marginy = {10, 10},
  .linespacing = 8,
  .paraspacing = 16,
  .paraindentation = 50,
  .emptylineheight = 50,
  .progressbar = 1,
  .pagenumber = 1,
  .textfont = FONT_12,
  .starttextpos = NULL,
  .startpage = 0,
};

void read_conf(void)
{
  FILE* fp = fopen("conf", "r");
  if (fp) {
    struct conf_s newconf;
    if (fread(&newconf, 1, sizeof(conf), fp) == sizeof(conf)) {
      conf = newconf;
    }
    filename = malloc(256);
    if (!fgets(filename, 256, fp)) {
      free(filename);
      filename = NULL;
    }
    else {
      filename[strlen(filename) - 1] = 0; /* strip LF */
    }
    fclose(fp);
  }
}

void write_conf(unsigned char* textpos, int currentpage)
{
  FILE* fp = fopen("conf","w");
  conf.starttextpos = textpos;
  conf.startpage = currentpage;
  fwrite(&conf, sizeof(conf), 1, fp);
  fputs(filename, fp);
  fputc('\n', fp);
  fclose(fp);
}
