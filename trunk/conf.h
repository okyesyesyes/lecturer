/*
    Lecturer: conf.h
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

struct conf_s {
  int marginx[2];
  int marginy[2];
  int linespacing;
  int paraspacing;
  int paraindentation;
  int emptylineheight;
  int progressbar;
  int pagenumber;
  int textfont;
  unsigned char* starttextpos;
  int startpage;
};

extern struct conf_s conf;
extern char* filename;

void read_conf(void);
void write_conf(unsigned char* textpos, int currentpage);
