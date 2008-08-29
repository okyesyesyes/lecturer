/*
    Lecturer: main.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "screen.h"
#include "input.h"
#include "font.h"
#include "conf.h"
#include "ui.h"
#include "speech.h"

FONT textfont;

unsigned char* text;

int main(int argc, char** argv)
{
  int fd,fd2;
  struct stat stbuf;
  unsigned char* textpos;
  unsigned char* lastspace;
  int lastspacewidth;
  unsigned char* linepos;
  int currentpage = 0;
  int tsx, tsy;
  int textlen = 0;
  int pass;
  int do_speech = 0;
  int speech_on = 0;
  int r;
  
  fd = fd2 = 0;
  
  assert(sizeof(struct glyph) == 8);
  
  screen_init();
  input_init();
  textfont = get_font(conf.textfont);
  init_ui();
  
  init_speech();
  
  read_conf();
  
  if (!filename) filename = file_dialog(".");

reload:
  if (!filename && !text) exit(0);
  if (filename) {
    if (stat(filename, &stbuf) < 0) {
      perror("statting text");
      exit(1);
    }
    textlen = stbuf.st_size;
    if (fd2) close(fd2);
    fd2 = open(filename,O_RDONLY);
    if (fd2 < 0) {
      perror("opening text");
      exit(1);
    }
    text = mmap(0, textlen, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd2, 0);
    if (!text) {
      perror("mmapping text");
      exit(1);
    }
  }
  textpos = text;
  int fbx = 0;
  int fby = 0;
  unsigned char** page = NULL;
#define PASS_PAGINATE 0
#define PASS_RENDER 1
#define NUM_PASSES 2

#define PROGRESS_LINE (screeny - 5)
#define PROGRESS_WIDTH 3
  int probarw = screenx - conf.marginx[0] - conf.marginx[1] - getglyph(smallfont, '0')->x * 4;
  int probarp = PROGRESS_LINE * pitch + conf.marginx[0];

  int pages;
repaginate:  
  pages = 0;
  for(pass = 0; pass < NUM_PASSES; pass++) {
    currentpage = 0;
    textpos = text;
    if (pass == PASS_PAGINATE) {
      char msg[] = "Paginating file...";
      clear_screen();
      render_string(textfont, screenx / 2 - stringwidth(textfont, msg) / 2, screeny / 2, msg);
    }
    if (pass == PASS_RENDER) {
      if (conf.starttextpos && conf.startpage) {
        textpos = conf.starttextpos;
        currentpage = conf.startpage;
      }
    }
    for(;;) {
      if (pass == PASS_PAGINATE && textpos >= text + textlen) break;
      fby = conf.marginy[0];
      if (pass == PASS_RENDER) {
        clear_screen();
        draw_button(UI_TOP_LEFT,"Quit");
        draw_button(UI_TOP_RIGHT, "File");
        draw_button(UI_TOP_CENTER, "Options");
        textpos = page[currentpage];
      }
      else {
        pages++;
        //printf("reallocing for %d pages\n",pages);
        page = realloc(page, sizeof(unsigned char*) * pages);
        page[pages - 1] = textpos;
      }
      int extraindent = 0;
      while(fby < screeny - fontheight(textfont) - conf.marginy[1] && textpos < text + textlen) {
        int linewidth = 0;
        int nextlinespace = conf.linespacing * fontheight(textfont) / 100;
        fbx = conf.marginx[0] + extraindent;
        extraindent = 0;
        linepos = textpos;
        lastspace = NULL;
        while(linewidth < screenx - fbx - conf.marginx[1] && linepos < text + textlen) {
          if (isspace(*linepos)) {
            lastspacewidth = linewidth;
            lastspace = linepos;
          }
          if (*linepos < ' ') {	/* covers all non-printable characters, especially LF and FF */
            lastspacewidth = linewidth;
            lastspace = linepos;
            nextlinespace = conf.paraspacing * fontheight(textfont) / 100;
            extraindent = conf.paraindentation * fontheight(textfont) / 100;
            break;
          }
          linewidth += glyphwidth(textfont, *linepos);
          linepos++;
        }
        if(!lastspace) {
          lastspace = linepos;
          lastspacewidth = linewidth;
        }
        //printf("linewidth %d\n",linewidth);
        if (pass == PASS_RENDER) for(linepos = textpos; linepos < lastspace ; linepos++) {
          fbx += render_char(textfont, fbx, fby, *linepos);
        }
        fby += fontheight(textfont) + nextlinespace;
        textpos = lastspace;
        if (*textpos == '\n' && textpos + 1 < text + textlen && textpos[1] == '\n') fby += conf.emptylineheight * fontheight(textfont) / 100;	/* preserve at most one empty line */
        while (textpos < text + textlen && (isspace(*textpos) || *textpos < ' ')) textpos++;
      }
      if (pass == PASS_RENDER) {
        if (conf.progressbar) {
          int i;
          if (conf.progressbar && conf.marginy[1] < screeny - PROGRESS_LINE) conf.marginy[1] = screeny - PROGRESS_LINE;
          for (i = 0; i < PROGRESS_WIDTH; i++)
            memset(fb + probarp + i*pitch, 0, (probarw * (currentpage + 1) / pages) * bpp);
        }
        if (conf.pagenumber) {
          char pageno[5];
          sprintf(pageno, "%4d", currentpage + 1);
          render_string(smallfont, screenx - stringwidth(smallfont, pageno) - conf.marginx[1], screeny - fontheight(smallfont) - 1, pageno);
        }
        //printf("ts x %d y %d pen %d\n",tsx,tsy,tspen);

        if (speech_on && do_speech) {
            char* speech = malloc(page[currentpage+1] - page[currentpage] + 1);
            memcpy(speech, page[currentpage], page[currentpage+1] - page[currentpage]);
            speech[page[currentpage+1] - page[currentpage]] = 0;
            unsigned char* s = (unsigned char*)speech;
            while(*s) {
              if (*s == 0253 || *s == 0273)
                *s = '"';		/* replace German/French quotation marks */
              s++;
            }
            read_text(speech);
            free(speech);
            do_speech = 0;
        }
        r = get_click(&tsx, &tsy, &speech_timeout);
        if (r) {
          if (tsx < UI_LEFT_AREA && tsy < UI_TOP_AREA) goto out;
          else if (tsx > UI_CENTER_AREA_LEFT && tsx < UI_CENTER_AREA_RIGHT && tsy < UI_TOP_AREA) {
            config_dialog();
            if(textfont != get_font(conf.textfont)) {
              textfont = get_font(conf.textfont);
              printf("repaginating\n");
              goto repaginate;
            }
          }
          else if (tsx > UI_RIGHT_AREA && tsy < UI_TOP_AREA) {
            char* oldname = strdup(filename);
            free(filename);
            filename = file_dialog(".");
            if (filename) {
              munmap(text, textlen);
              text = NULL;
              if (page) free(page);
              conf.starttextpos = 0; conf.startpage = 0;
              free(oldname);
              goto reload;
            }
            else filename = oldname;
          }
          else if (tsx > UI_CENTER_AREA_LEFT && tsx < UI_CENTER_AREA_RIGHT && tsy > UI_BOTTOM_AREA) {
            if (speech_on) {
              speech_on = 0;
              do_speech = 0;
              stop_speech();
            }
            else {
              speech_on = 1;
              do_speech = 1;
            }
          }
          else if (tsy < 135 && currentpage > 0) currentpage--;
          else if (tsy > 135 && currentpage < pages - 1) currentpage++;
        }
        else {
          /* speech timeout */
          speech_timeout = 0;
          if (speech_on && currentpage < pages - 1) {
            currentpage++;
            do_speech = 1;
          }
        }
      }
    }
    if (pass == PASS_PAGINATE) printf("%d pages\n",pages);
  }
    //printf("ts x %d y %d pen %d\n",tsx,tsy,tspen);
    for(;;) {
      while (!get_click(&tsx, &tsy, NULL)) {}
      if (tsx < 40 && tsy < 40) break;
    }
out:  
  write_conf(textpos, currentpage);
  if (page) free(page);
  return 0;
}
