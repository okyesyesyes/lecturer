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
  int justify;
};

extern struct conf_s conf;
extern struct conf_s default_conf;
extern char* filename;

void read_global_conf(void);
void read_file_conf(char* filename);
void write_global_conf();
void write_file_conf(char* filename, unsigned char* textpos, int currentpage);

