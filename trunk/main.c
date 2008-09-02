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
#include "encoding.h"

#define TEXT_DIR "text"

FONT textfont;

unsigned char* rawtext;
unsigned char* text;

int main(int argc, char** argv)
{
  int fd,fd2;
  struct stat stbuf;
  unsigned char* textpos;
  unsigned char* lastspace;
  int lastspacewidth = 0;
  unsigned char* linepos;
  int currentpage = 0;
  int currentposrel = -1;
  int tsx, tsy;
  int textlen = 0;
  int pass;
  int do_speech = 0;
  int speech_on = 0;
  int r;
  int free_text = 0; /* set if memory pointed to by text has to be free()d */
  
  fd = fd2 = 0;
  
  assert(sizeof(struct glyph) == 8);
  
  screen_init();
  input_init();
  init_ui();
  
  read_global_conf();
  

  textfont = get_font(conf.textfont);

  if (stat(filename, &stbuf) < 0) {
    free(filename);
    filename = NULL;
  }
  
  if (!filename) filename = file_dialog(TEXT_DIR);

reload:
  if (!filename && !text) exit(0);
  if (filename) {
    printf("loading file %s\n", filename);
    read_file_conf(filename);
    textfont = get_font(conf.textfont);
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
    rawtext = mmap(0, textlen, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd2, 0);
    if (!rawtext) {
      perror("mmapping text");
      exit(1);
    }
    text = utf8_to_latin1(rawtext, &textlen);
    if (text) free_text = 1;
    else {	/* could not convert from UTF-8, assuming ISO 8859-1 */
      free_text = 0;
      text = rawtext;
    }
  }

  init_speech();

  textpos = text;
  int fbx = 0;
  int fby = 0;
  unsigned char** page = NULL;
#define PASS_PAGINATE 0
#define PASS_RENDER 1
#define NUM_PASSES 2

#define PROGRESS_LINE (screeny - 5)
#define PROGRESS_WIDTH 3
  int probar_left_margin = conf.marginx[0] + stringwidth(smallfont, "Talk ");
  int probar_right_margin = screenx - conf.marginx[1] - getglyph(smallfont, '0')->x * 4;
  int probarw = probar_right_margin - probar_left_margin;
  int probarp = PROGRESS_LINE * pitch + probar_left_margin * bpp;

  int pages;
repaginate:  
  pages = 0;
  for(pass = 0; pass < NUM_PASSES; pass++) {
    currentpage = 0;
    textpos = text;
    if (pass == PASS_PAGINATE) {
      splash_msg("Paginating file...");
    }
    if (pass == PASS_RENDER) {
      if (conf.starttextpos && conf.startpage) {
        textpos = conf.starttextpos;
        currentpage = conf.startpage;
        conf.starttextpos = NULL;
        conf.startpage = 0;
      }
      if (currentposrel != -1) {
        currentpage = currentposrel * pages / 100000;
        if (currentpage < pages) currentpage++; /* compensate rounding error */
        currentposrel = -1;
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
        draw_button(UI_BOTTOM_LEFT, "Talk");
        textpos = page[currentpage];
      }
      else {
        pages++;
        //printf("reallocing for %d pages\n",pages);
        page = realloc(page, sizeof(unsigned char*) * pages);
        page[pages - 1] = textpos;
      }
      int extraindent = 0; /* set to conf.paraspacing on paragraph breaks */
      while(fby < screeny - fontheight(textfont) - conf.marginy[1] && textpos < text + textlen) {
        int linewidth = 0;	/* width of text on this line in pixels */
        int nextlinespace = conf.linespacing * fontheight(textfont) / 100;	/* linespacing in pixels at current font setting */
        int spacecount = 0;	/* whitespace counter (used for justification) */
        fbx = conf.marginx[0] + extraindent;
        extraindent = 0;	/* extra indentation; set to conf.paraindentation on new paragraphs */
        linepos = textpos;	/* text position on current line */
        lastspace = NULL;	/* position of the last space character on this line */
        int dontjustify = 0;	/* set to turn off justification temporarily, e.g. on paragraph breaks */
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
            dontjustify = 1;
            break;
          }
          if (*linepos == ' ')
            spacecount++;
          linewidth += glyphwidth(textfont, *linepos);
          linepos++;
        }
        if(!lastspace) {
          lastspace = linepos;
          lastspacewidth = linewidth;
        }
        int spacewidth = 0;
        spacecount--; /* The last space before a linebreak is not drawn. */
        if (spacecount > 0) {
          spacewidth = (screenx - lastspacewidth - fbx - conf.marginx[1])	/* remaining space on this line in pixels */
                       + glyphwidth(textfont, ' ') * spacecount;		/* whitespace already accounted for */
          //if (!dontjustify) printf("spacewidth %d num %d\n", spacewidth, spacecount);
        }
        else dontjustify = 1;
        //printf("linewidth %d\n",linewidth);
        if (pass == PASS_RENDER) for(linepos = textpos; linepos < lastspace ; linepos++) {
          if (conf.justify && !dontjustify && *linepos == ' ') {
            /* doing this division for every space keeps rounding errors from accumulating */
            int spacesize = spacewidth / spacecount;
            fbx += spacesize;
            spacewidth -= spacesize;
            spacecount--;
            //printf("%d px space remaining for %d spaces\n", spacewidth, spacecount);
          }
          else fbx += render_char(textfont, fbx, fby, *linepos);
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
            unsigned char* endspeech;
            if (currentpage + 1 < pages) endspeech = page[currentpage+1];
            else {
              endspeech = text + textlen;
              speech_on = 0;
            }
            char* speech = malloc(endspeech - page[currentpage] + 1);
            memcpy(speech, page[currentpage], endspeech - page[currentpage]);
            speech[endspeech - page[currentpage]] = 0;
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
            int redo = config_dialog();
            if (redo & REDO_SPEECHINIT) init_speech();
            if (redo & REDO_PAGINATION) {
              currentposrel = currentpage * 100000 / pages;
              textfont = get_font(conf.textfont);
              printf("repaginating\n");
              goto repaginate;
            }
          }
          else if (tsx > UI_RIGHT_AREA && tsy < UI_TOP_AREA) {
            char* oldname = strdup(filename);
            free(filename);
            filename = file_dialog(TEXT_DIR);
            if (filename) {
              write_file_conf(oldname, textpos, currentpage);
              if (free_text) free(text);
              munmap(rawtext, textlen);
              text = rawtext = NULL;
              if (page) free(page);
              conf.starttextpos = 0; conf.startpage = 0;
              free(oldname);
              goto reload;
            }
            else filename = oldname;
          }
          else if (tsx < probar_left_margin && tsy > UI_BOTTOM_AREA) {
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
          else if (conf.progressbar && tsx > probar_left_margin && tsx < probar_right_margin && tsy > screeny - 20) {
            currentpage = (tsx - probar_left_margin) * pages / probarw;
            //printf("tsx %d pages %d probarw %d currentpage %d\n", tsx, pages, probarw,currentpage);
          }
          else if (tsx < (screenx / 2) && currentpage > 0) currentpage--;
          else if (tsx > (screenx / 2) && currentpage < pages - 1) currentpage++;
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
  if (filename) {
    write_file_conf(filename, textpos, currentpage);
  }
  write_global_conf();
  if (page) free(page);
  if (filename) free(filename);
  return 0;
}
