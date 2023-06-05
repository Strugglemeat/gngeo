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

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <SDL.h>

enum {
  TILE_NORMAL,
  TILE_INVISIBLE,
  TILE_TRANSPARENT25,
  TILE_TRANSPARENT50,
};

typedef struct gfx_cache {
  uint8_t *data;
  uint32_t size;
  uint32_t total_bank;
  uint8_t **ptr;
  int max_slot;
  int slot_size;
  int *usage;
  FILE *gno;
  uint32_t *offset;
  uint8_t *in_buf;
} GFX_CACHE;

typedef struct VIDEO {
  uint8_t ram[0x20000];
  uint8_t pal_neo[2][0x2000];
  uint8_t pal_host[2][0x4000];
  uint8_t currentpal;
  uint8_t currentfix;
  uint16_t rbuf;

  uint32_t fc;
  uint32_t fc_speed;

  uint32_t vptr;
  int16_t modulo;

  uint32_t current_line;

  uint32_t irq2control;
  uint32_t irq2taken;
  uint32_t irq2start;
  uint32_t irq2pos;

  GFX_CACHE spr_cache;
} VIDEO;

#define RASTER_LINES 261

extern unsigned int neogeo_frame_counter;
extern unsigned int neogeo_frame_counter_speed;

void init_video(void);
void draw_screen_scanline(int start_line, int end_line, int refresh);
void draw_screen(void);
int init_sprite_cache(uint32_t size, uint32_t bsize);
void free_sprite_cache(void);

#ifdef ARM
void draw_tile_arm_xflip_norm(uint32_t, uint32_t, void *, uint32_t);
void draw_tile_arm_yflip_norm(uint32_t, uint32_t, void *, uint32_t);
void draw_tile_arm_xyflip_norm(uint32_t, uint32_t, void *, uint32_t);
void draw_tile_arm_xzoom(uint32_t, uint32_t, void *, uint32_t);
void draw_tile_arm_xflip_xzoom(uint32_t, uint32_t, void *, uint32_t);
void draw_tile_arm_yflip_xzoom(uint32_t, uint32_t, void *, uint32_t);
void draw_tile_arm_xyflip_xzoom(uint32_t, uint32_t, void *, uint32_t);
void draw_one_char_arm(int byte1, int byte2, unsigned short *br);
int draw_tile_arm_norm(unsigned int tileno, int color, void *bmp, int zy);
#endif

#endif
