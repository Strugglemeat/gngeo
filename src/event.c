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

#include <SDL.h>

#include "screen.h"
#include "event.h"
#include "conf.h"
#include "emu.h"
#include "memory.h"
#include "gnutil.h"
#include "messages.h"

void set_key(uint32_t k, uint32_t down)
{
  if(down) {
    memory.intern_p1 &= ~(1 << k);
  }
  else {
    memory.intern_p1 |= (1 << k);
  }
}

void set_abxylr(uint32_t v, uint32_t down)
{
  //char *lr_btn_string[] = {"None", "A", "B", "C", "D", "A+B", "A+C", "A+D", "B+C", "B+D", "C+D", "A+B+C", "A+B+D", "A+C+D", "B+C+D", "A+B+C+D"};
  switch(v) {
  case 0:
    break;
  case 1:
    set_key(KEY_A, down);
    break;
  case 2:
    set_key(KEY_B, down);
    break;
  case 3:
    set_key(KEY_C, down);
    break;
  case 4:
    set_key(KEY_D, down);
    break;
  case 5:
    set_key(KEY_A, down);
    set_key(KEY_B, down);
    break;
  case 6:
    set_key(KEY_A, down);
    set_key(KEY_C, down);
    break;
  case 7:
    set_key(KEY_A, down);
    set_key(KEY_D, down);
    break;
  case 8:
    set_key(KEY_B, down);
    set_key(KEY_C, down);
    break;
  case 9:
    set_key(KEY_B, down);
    set_key(KEY_D, down);
    break;
  case 10:
    set_key(KEY_C, down);
    set_key(KEY_D, down);
    break;
  case 11:
    set_key(KEY_A, down);
    set_key(KEY_B, down);
    set_key(KEY_C, down);
    break;
  case 12:
    set_key(KEY_A, down);
    set_key(KEY_B, down);
    set_key(KEY_D, down);
    break;
  case 13:
    set_key(KEY_A, down);
    set_key(KEY_C, down);
    set_key(KEY_D, down);
    break;
  case 14:
    set_key(KEY_B, down);
    set_key(KEY_C, down);
    set_key(KEY_D, down);
    break;
  case 15:
    set_key(KEY_A, down);
    set_key(KEY_B, down);
    set_key(KEY_C, down);
    set_key(KEY_D, down);
    break;
  }
}

int handle_event(void)
{
  SDL_Event event = {0};

  while(SDL_PollEvent(&event)) {
    switch(event.type) {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
      switch(event.key.keysym.sym) {
      case SDLK_UP:
        set_key(KEY_UP, event.type == SDL_KEYDOWN ? 1 : 0);
        break;
      case SDLK_DOWN:
        set_key(KEY_DOWN, event.type == SDL_KEYDOWN ? 1 : 0);
        break;
      case SDLK_LEFT:
        set_key(KEY_LEFT, event.type == SDL_KEYDOWN ? 1 : 0);
        break;
      case SDLK_RIGHT:
        set_key(KEY_RIGHT, event.type == SDL_KEYDOWN ? 1 : 0);
        break;
      case SDLK_LCTRL:
        set_abxylr(conf.a_btn, event.type == SDL_KEYDOWN ? 1 : 0);
        break;
      case SDLK_LALT:
        set_abxylr(conf.b_btn, event.type == SDL_KEYDOWN ? 1 : 0);
        break;
      case SDLK_SPACE:
        set_abxylr(conf.x_btn, event.type == SDL_KEYDOWN ? 1 : 0);
        break;
      case SDLK_LSHIFT:
        set_abxylr(conf.y_btn, event.type == SDL_KEYDOWN ? 1 : 0);
        break;
      case SDLK_ESCAPE:
        if(event.type == SDL_KEYDOWN) {
          memory.intern_coin &= ~(1 << 0);
        }
        else {
          memory.intern_coin |= (1 << 0);
        }
        break;
      case SDLK_RETURN:
        if(event.type == SDL_KEYDOWN) {
          memory.intern_start &= ~(1 << 0);
        }
        else {
          memory.intern_start |= (1 << 0);
        }
        break;
      case SDLK_TAB:
        set_abxylr(conf.l_btn, event.type == SDL_KEYDOWN ? 1 : 0);
        break;
      case SDLK_BACKSPACE:
        set_abxylr(conf.r_btn, event.type == SDL_KEYDOWN ? 1 : 0);
        break;
      case SDLK_RCTRL:
        return 1;
      }
      break;
    }
  }
  return 0;
}

