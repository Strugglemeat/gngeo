/*
 *  Copyright (C) 2021 Steward Fu
 *  Copyright (C) 2001 Peponas Mathieu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <math.h>
#include "SDL.h"
#include "frame_skip.h"
#include "messages.h"
#include "emu.h"
#include "conf.h"
#include "gnutil.h"

#define TICKS_PER_SEC 1000000UL
static uint32_t F;

#define MAX_FRAMESKIP 10

char fps_str[32];
static char init_frame_skip = 1;
char skip_next_frame = 0;
static int CPU_FPS = 60;
static struct timeval init_tv = { 0, 0 };

uint32_t get_ticks(void)
{
  struct timeval tv;

  gettimeofday(&tv, 0);
  if(init_tv.tv_sec == 0) {
    init_tv = tv;
  }
  return (tv.tv_sec - init_tv.tv_sec) * TICKS_PER_SEC + tv.tv_usec - init_tv.tv_usec;

}

void reset_frame_skip(void)
{
  init_tv.tv_usec = 0;
  init_tv.tv_sec = 0;
  skip_next_frame = 0;
  init_frame_skip = 1;
  F = (uint32_t)((double) TICKS_PER_SEC / CPU_FPS);
}

static int __attribute__((noinline)) get_cpu_ticks(void)
{
  static int fd = 0;
  static unsigned long last_utime = 0;

  char buf[128] = {0};
  unsigned long utime = 0, ret = 0;

  if(fd == 0) {
    fd = open("/proc/self/stat", O_RDONLY);
  }
  lseek(fd, 0, SEEK_SET);
  buf[0] = 0;
  read(fd, buf, sizeof(buf));
  buf[sizeof(buf) - 1] = 0;

  sscanf(buf, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu", &utime);
  ret = utime - last_utime;
  if(ret > 200) {
    ret = 0;
  }
  last_utime = utime;
  return ret;
}

int frame_skip(void)
{
  static int f2skip;
  static uint32_t sec = 0;
  static uint32_t rfd;
  static uint32_t target;
  static int nbFrame = 0;
  static unsigned int nbFrame_moy = 0;
  static int nbFrame_min = 1000;
  static int nbFrame_max = 0;
  static int skpFrm = 0;
  static int count = 0;
  static int moy = 60;

  if(init_frame_skip) {
    init_frame_skip = 0;
    target = get_ticks();
    nbFrame = 0;
    sec = 0;
    return 0;
  }

  target += F;
  if(f2skip > 0) {
    f2skip -= 1;
    skpFrm += 1;
    return 1;;
  }
  else {
    skpFrm = 0;
  }

  rfd = get_ticks();
  if(rfd < target && f2skip == 0)
    while(get_ticks() < target) {
      usleep(5);
    }
  else {
    f2skip = (rfd - target) / (double) F;
    if(f2skip > MAX_FRAMESKIP) {
      f2skip = MAX_FRAMESKIP;
      reset_frame_skip();
    }
  }

  nbFrame++;
  nbFrame_moy++;
  if(conf.show_fps) {
    if((get_ticks() - sec) >= TICKS_PER_SEC) {
      sprintf(fps_str, "FPS %2d / CPU %2d%%", nbFrame - 1, get_cpu_ticks());
      nbFrame = 0;
      sec = get_ticks();
    }
  }
  return 0;
}
