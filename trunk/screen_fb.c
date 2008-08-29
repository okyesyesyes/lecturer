/*
    Lecturer: screen_fb.c
    Copyright (C) 2008 Ulrich Hecht <uli@suse.de>

    This program can be distributed under the terms of the GNU GPL v2.
    See the file COPYING.
*/

#include <linux/fb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/ioctl.h>

unsigned char* fb;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
int screenx;
int screeny;
int bpp;
int pitch;

void screen_init(void)
{
        int fd;
        if (!(fd = open("/dev/fb", O_RDWR))) {
          perror("failed to open framebuffer");
          exit(1);
        }
        if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
          perror("failed to get fixed screen info");
          exit(1);
        }
        if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
          perror("failed to get variable screen info");
          exit(1);
        }
        screenx = vinfo.xres;
        screeny = vinfo.yres;
        bpp = vinfo.bits_per_pixel / 8;
        pitch = vinfo.xres * bpp;
        
        printf("x %d y %d bpp %d pitch %d\n",screenx,screeny,bpp,pitch);
        fb = mmap(0, screenx * screeny * bpp, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if ((long)fb == -1) {
          perror("failed to mmap framebuffer");
          exit(1);
        }
}

