/*
    Lecturer: input_ts.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include <barcelona/Barc_ts.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>

static int tsfd;

static int TsScreen_pen(int *x,int *y,int *pen) {
  if(tsfd<0) return 0;
  int read_len;
  TS_EVENT new_event;
 
  static int have_previous = 0;
  static TS_EVENT prev_event;

  if (!have_previous) {
    read_len = read(tsfd, &prev_event, sizeof(TS_EVENT));
    if (read_len == sizeof(TS_EVENT)) have_previous=1;
  }
  // if we still don't have an event, there are no events pending, and we can just return
  if(!have_previous) return 0;
  
  // We have an event
  memcpy(&new_event, &prev_event,sizeof(TS_EVENT));
  have_previous = 0;

#if 0
  read_len = read(tsfd, &prev_event, sizeof(TS_EVENT));
  if(read_len==sizeof(TS_EVENT)) have_previous=1;

  while (have_previous && (prev_event.pressure != 0) == (new_event.pressure != 0)) {
    memcpy(&new_event, &prev_event,sizeof(TS_EVENT));
    have_previous = 0;
    read_len = read(tsfd, &prev_event, sizeof(TS_EVENT));
    if (read_len == sizeof(TS_EVENT)) have_previous=1;
  }
#endif  
  *x = new_event.x;
  *y = new_event.y;
  *pen = new_event.pressure;
  return 1;
}

static int TsScreen_waitevent(int timeout) {
  fd_set active_fd_set,read_fd_set;;
  int retval;
  struct timeval tv;
  
  if(tsfd<0) return 0;
  FD_ZERO (&active_fd_set);
  FD_SET (tsfd, &active_fd_set); 

    /* Wait up to five seconds. */
    tv.tv_sec = 0;
    tv.tv_usec = timeout;
      read_fd_set = active_fd_set;

#if DEBUG
  if(timeout) g_outs("Waiting for an event.\n");
  else g_outs("Reading an event.\n");
#endif
  retval = select (FD_SETSIZE, &read_fd_set, NULL, NULL, &tv); 
  if(retval<0) perror("select failed!\n");
  else if(!retval) {
#if DEBUG
    if(timeout) {
      fprintf(stderr,"++timeout waiting for touchscreen event!\n");
    }
#endif
    return(0);
  } else {
    if(FD_ISSET(tsfd,&read_fd_set)) {
      return(1);
    } else fprintf(stderr,"Hier ist was komisch\n");
  }
  return(0);
}

void input_init(void)
{
        tsfd = open("/dev/ts", O_RDWR|O_NOCTTY|O_NONBLOCK);
        if (tsfd < 0) {
          if ((tsfd = open("/dev/ts", O_RDONLY|O_NOCTTY|O_NONBLOCK)) < 0) {
            perror("failed to open touchscreen");
            exit(1);
          }
        }
        ioctl(tsfd, TS_SET_RAW_OFF, NULL);
}

int get_click(int *x, int *y, int* breakon)
{
  static int clicked = 0;
  int tspen;
  while (clicked) {
    while (!TsScreen_waitevent(100000)) {}
    TsScreen_pen(x, y, &tspen);
    if (tspen < 100) clicked = 0;
  }
  while (!clicked) {
    while (!TsScreen_waitevent(100000)) {
      if (breakon && *breakon) return 0;
    }
    TsScreen_pen(x, y, &tspen);
    if (tspen > 200) clicked = 1;
  }
  return 1;
}
