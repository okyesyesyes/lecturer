/*
    Lecturer: conf.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include "conf.h"
#include "font.h"
#include "screen.h"

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
  .justify = 1,
};

struct conf_s default_conf;

void read_global_conf(void)
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
  default_conf = conf;
}

void read_file_conf(char* filename)
{
  char* confname = malloc(strlen(filename) + strlen(".conf") + 1);
  sprintf(confname, "%s.conf", filename);
  FILE* fp = fopen(confname, "r");
  if (fp) {
    if (fread(&conf, 1, sizeof(conf), fp) != sizeof(conf))
      msg("Error reading config file; continuing with default settings.");
    fclose(fp);
  }
  free(confname);
}

static void write_conf(FILE* fp, struct conf_s* c, unsigned char* textpos, int currentpage)
{
  conf.starttextpos = textpos;
  conf.startpage = currentpage;
  fwrite(c, sizeof(struct conf_s), 1, fp);
}

void write_global_conf(void)
{
  FILE* fp = fopen("conf","w");
  write_conf(fp, &default_conf, NULL, 0);
  fputs(filename, fp);
  fputc('\n', fp);
  fclose(fp);
}

void write_file_conf(char* filename, unsigned char* textpos, int currentpage)
{
  char* confname = malloc(strlen(filename) + strlen(".conf") + 1);
  sprintf(confname, "%s.conf", filename);
  FILE* fp = fopen(confname, "w");
  write_conf(fp, &conf, textpos, currentpage);
  fclose(fp);
  free(confname);
}
