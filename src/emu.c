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

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#include "emu.h"
#include "conf.h"
#include "menu.h"
#include "event.h"
#include "timer.h"
#include "sound.h"
#include "memory.h"
#include "screen.h"
#include "pd4990a.h"
#include "neocrypt.h"
#include "messages.h"
#include "frame_skip.h"
#include "ym2610_interf.h"

#include "rumble.h"
//#include <SDL_gfxPrimitives.h>

int frame;
int nb_interlace = 256;
int current_line;

extern char skip_next_frame;
extern SDL_Rect visible_area;
extern SDL_Surface *buffer;

void setup_misc_patch(char *name)
{

  if(!strcmp(name, "ssideki")) {
    WRITE_WORD_ROM(&memory.rom.cpu_m68k.p[0x2240], 0x4e71);
  }

  //if (!strcmp(name, "fatfury3")) {
  //	WRITE_WORD_ROM(memory.rom.cpu_m68k.p, 0x0010);
  //}

  if(!strcmp(name, "mslugx")) {
    /* patch out protection checks */
    int i;
    uint8_t *RAM = memory.rom.cpu_m68k.p;
    for(i = 0; i < memory.rom.cpu_m68k.size; i += 2) {
      if((READ_WORD_ROM(&RAM[i + 0]) == 0x0243)
          && (READ_WORD_ROM(&RAM[i + 2]) == 0x0001) && /* andi.w  #$1, D3 */
          (READ_WORD_ROM(&RAM[i + 4]) == 0x6600)) { /* bne xxxx */

        WRITE_WORD_ROM(&RAM[i + 4], 0x4e71);
        WRITE_WORD_ROM(&RAM[i + 6], 0x4e71);
      }
    }

    WRITE_WORD_ROM(&RAM[0x3bdc], 0x4e71);
    WRITE_WORD_ROM(&RAM[0x3bde], 0x4e71);
    WRITE_WORD_ROM(&RAM[0x3be0], 0x4e71);
    WRITE_WORD_ROM(&RAM[0x3c0c], 0x4e71);
    WRITE_WORD_ROM(&RAM[0x3c0e], 0x4e71);
    WRITE_WORD_ROM(&RAM[0x3c10], 0x4e71);

    WRITE_WORD_ROM(&RAM[0x3c36], 0x4e71);
    WRITE_WORD_ROM(&RAM[0x3c38], 0x4e71);
  }

}

void neogeo_reset(void)
{
  sram_lock = 0;
  sound_code = 0;
  pending_command = 0;
  result_code = 0;
#ifdef ENABLE_940T
  shared_ctl->sound_code = sound_code;
  shared_ctl->pending_command = pending_command;
  shared_ctl->result_code = result_code;
#endif
  if(memory.rom.cpu_m68k.size > 0x100000) {
    cpu_68k_bankswitch(0x100000);
  }
  else {
    cpu_68k_bankswitch(0);
  }
  cpu_68k_reset();

}

void init_sound(void)
{

  if(conf.sound) {
    init_sdl_audio();
  }

  cpu_z80_init();

  YM2610_sh_start();
  if(conf.sound)	{
    pause_audio(0);
  }

}

void init_neo(void)
{
  neogeo_init_save_state();
  cpu_68k_init();
  pd4990a_init();
  init_sound();
  neogeo_reset();
}

static void take_screenshot(void)
{
  time_t ltime;
  struct tm *today;
  char buf[256];
  char date_str[101];
  //static SDL_Rect buf_rect    =	{16, 16, 304, 224};
  static SDL_Rect screen_rect = { 0, 0, 304, 224 };
  static SDL_Surface *shoot;

  screen_rect.w = visible_area.w;
  screen_rect.h = visible_area.h;

  if(shoot == NULL)
    shoot = SDL_CreateRGBSurface(SDL_SWSURFACE, visible_area.w,
                                 visible_area.h, 16, 0xF800, 0x7E0, 0x1F, 0);

  time(&ltime);
  today = localtime(&ltime);
  strftime(date_str, 100, "%a_%b_%d_%T_%Y", today);
  snprintf(buf, 255, "%s/%s_%s.bmp", getenv("HOME"), conf.game, date_str);
  printf("save to %s\n", buf);

  SDL_BlitSurface(buffer, &visible_area, shoot, &screen_rect);
  SDL_SaveBMP(shoot, buf);
}

static int fc;
static int last_line;
static int skip_this_frame = 0;

static inline int neo_interrupt(void)
{
  static int frames;

  pd4990a_addretrace();
  if(!(memory.vid.irq2control & 0x8)) {
    if(fc >= neogeo_frame_counter_speed) {
      fc = 0;
      neogeo_frame_counter++;
    }
    fc++;
  }

  skip_this_frame = skip_next_frame;
  skip_next_frame = frame_skip();

  if(!skip_this_frame) {
    draw_screen();
  }
  return 1;
}

static inline void update_screen(void)
{
  if(memory.vid.irq2control & 0x40) {
    memory.vid.irq2start = (memory.vid.irq2pos + 3) / 0x180;  /* ridhero gives 0x17d */
  }
  else {
    memory.vid.irq2start = 1000;
  }

  if(!skip_this_frame) {
    if(last_line < 21) {
      draw_screen();
    }
    else {
      draw_screen_scanline(last_line - 21, 262, 1);
    }
  }

  last_line = 0;

  pd4990a_addretrace();
  if(fc >= neogeo_frame_counter_speed) {
    fc = 0;
    neogeo_frame_counter++;
  }
  fc++;

  skip_this_frame = skip_next_frame;
  skip_next_frame = frame_skip();
}

static inline int update_scanline(void)
{
  memory.vid.irq2taken = 0;

  if(memory.vid.irq2control & 0x10) {

    if(current_line == memory.vid.irq2start) {
      if(memory.vid.irq2control & 0x80) {
        memory.vid.irq2start += (memory.vid.irq2pos + 3) / 0x180;
      }
      memory.vid.irq2taken = 1;
    }
  }

  if(memory.vid.irq2taken) {
    if(!skip_this_frame) {
      if(last_line < 21) {
        last_line = 21;
      }
      if(current_line < 20) {
        current_line = 20;
      }
      draw_screen_scanline(last_line - 21, current_line - 20, 0);
    }
    last_line = current_line;
  }
  current_line++;
  return memory.vid.irq2taken;
}

static int slow_motion = 0;

void main_loop(void)
{
  int neo_emu_done = 0;
  int m68k_overclk = 0;
  int z80_overclk = 0;
  int a, i;
  static SDL_Rect buf_rect = { 24, 16, 304, 224 };
  static SDL_Rect screen_rect = { 0, 0, 304, 224 };
  uint32_t cpu_68k_timeslice = (m68k_overclk == 0 ? 200000 : 200000 + (m68k_overclk * 200000 / 100.0));
  uint32_t cpu_68k_timeslice_scanline = cpu_68k_timeslice / 264.0;
  uint32_t cpu_z80_timeslice = (z80_overclk == 0 ? 73333 : 73333 + (z80_overclk * 73333 / 100.0));
  uint32_t tm_cycle = 0;
  uint32_t cpu_z80_timeslice_interlace = cpu_z80_timeslice / (float) nb_interlace;

  reset_frame_skip();
  my_timer();

  while(!neo_emu_done) 
  {
    if(handle_event())//input
    {

      SDL_BlitSurface(buffer, &buf_rect, state_img, &screen_rect);

      if(conf.sound) 
      {
        pause_audio(1);
      }

      if(run_menu() == 2) 
      {
        neo_emu_done = 1;
        return;
      }

      if(conf.sound) 
      {
        pause_audio(0);
      }

      reset_frame_skip();

    }//end of handle_event

    if(conf.sound) 
    {
      for(i = 0; i < nb_interlace; i++) {
        cpu_z80_run(cpu_z80_timeslice_interlace);
        my_timer();
      }
    }

    tm_cycle = cpu_68k_run(cpu_68k_timeslice - tm_cycle);
    a = neo_interrupt();
    memory.watchdog++;
    if(memory.watchdog > 7) 
    {  /* Watchdog reset after ~0.13 == ~7.8 frames */
      printf("WATCHDOG RESET %d\n", memory.watchdog);
      cpu_68k_reset();
    }

    if(a) 
    {
      cpu_68k_interrupt(a);
    }

    uint32_t currentPC=cpu_68k_getpc();

    printf("&d",currentPC);

//    #define testingAddr 0x9898//0x9c50
//    #define doingDamagePC 0x15e62

//    for(int i=-32;i<32;i++)if(currentPC==doingDamagePC+i)break;

    rumble_kof97_training();

    //rumble_temporary_visualization();

    rumble_checkMemory();

  }//end of while loop
  pause_audio(1);
}

void cpu_68k_dpg_step(void)
{
  static uint32_t nb_cycle;
  static uint32_t line_cycle;
  uint32_t cpu_68k_timeslice = 200000;
  uint32_t cpu_68k_timeslice_scanline = 200000 / (float) 262;
  uint32_t cycle;
  if(nb_cycle == 0) {
    main_loop(); /* update event etc. */
  }
  cycle = cpu_68k_run_step();
  //add_bt(cpu_68k_getpc());
  line_cycle += cycle;
  nb_cycle += cycle;
  if(nb_cycle >= cpu_68k_timeslice) {
    nb_cycle = line_cycle = 0;
    neo_interrupt();
    cpu_68k_interrupt(1);
  }
  else {
    if(line_cycle >= cpu_68k_timeslice_scanline) {
      line_cycle = 0;
    }
  }
}

