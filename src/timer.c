/*  gngeo a neogeo emulator
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include "emu.h"
#include "timer.h"
#include "state.h"
#include "ym2610.h"
#ifdef GP2X
#include "ym2610-940/940shared.h"
#endif

double timer_count;
//Uint32 timer_count;

timer_struct *timer_list;
#define MAX_TIMER 3
timer_struct timers[MAX_TIMER];
extern int nb_interlace;
//int nb_timer=0;

double timer_get_time(void)
{
//Uint32 timer_get_time(void) {
  return timer_count;
}

timer_struct *insert_timer(double duration, int param, void (*func)(int))
//timer_struct *insert_timer(Uint32 duration, int param, void (*func) (int))
{
  int i;
  for(i = 0; i < MAX_TIMER; i++) {
    if(timers[i].del_it) {
      timers[i].time = timer_count + duration;
      timers[i].param = param;
      timers[i].func = func;
      //printf("Insert_timer %d duration=%f param=%d\n",i,duration,timers[i].param);
      timers[i].del_it = 0;
      return &timers[i];
    }
  }
  printf("YM2610: No timer free!\n");
  return NULL;		/* No timer free */
}

void free_all_timer(void)
{
  int i;
  for(i = 0; i < MAX_TIMER; i++) {
    timers[i].del_it = 1;
  }
}

#ifdef ENABLE_940T
void timer_callback_2610(int param) {}
#else
void timer_callback_2610(int param);
#endif

void del_timer(timer_struct *ts)
{
  ts->del_it = 1;
}

static double inc;
//static Uint32 inc;

void my_timer(void)
{
  static int init = 1;
  int i;

  if(init) {
    //timer_init_save_state();
    init = 0;

    inc = ((double)(0.01666) / nb_interlace);  /* *(1<<TIMER_SH); */
    for(i = 0; i < MAX_TIMER; i++) {
      timers[i].del_it = 1;
    }
  }

  timer_count += inc;		/* 16ms par frame */

  for(i = 0; i < MAX_TIMER; i++) {
    if(timer_count >= timers[i].time && timers[i].del_it == 0) {
      //printf("Timer_expire %d duration=%f param=%d\n",i,timers[i].time,timers[i].param);
      if(timers[i].func) {
        timers[i].func(timers[i].param);
      }
      timers[i].del_it = 1;
    }
  }
}
