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

#ifndef __ROMS__
#define __ROMS__

#include <stdint.h>
#include <SDL.h>

#define REGION_AUDIO_CPU_BIOS        0
#define REGION_AUDIO_CPU_CARTRIDGE   1
#define REGION_AUDIO_CPU_ENCRYPTED   2
#define REGION_AUDIO_DATA_1          3
#define REGION_AUDIO_DATA_2          4
#define REGION_FIXED_LAYER_BIOS      5
#define REGION_FIXED_LAYER_CARTRIDGE 6
#define REGION_MAIN_CPU_BIOS         7
#define REGION_MAIN_CPU_CARTRIDGE    8
#define REGION_SPRITES               9
#define REGION_SPR_USAGE             10
#define REGION_GAME_FIX_USAGE        11

#define HAS_CUSTOM_CPU_BIOS          1
#define HAS_CUSTOM_AUDIO_BIOS        2
#define HAS_CUSTOM_SFIX_BIOS         4

typedef struct ROM_DEF {
  char name[32];
  char parent[32];
  char longname[128];
  uint32_t year;
  uint32_t romsize[10];
  uint32_t nb_romfile;
  struct romfile {
    char filename[32];
    uint8_t region;
    uint32_t src;
    uint32_t dest;
    uint32_t size;
    uint32_t crc;
  } rom[32];
} ROM_DEF;

typedef struct GAME_INFO {
  char *name;
  char *longname;
  int year;
  uint32_t flags;
} GAME_INFO;

typedef struct ROM_REGION {
  uint8_t *p;
  uint32_t size;
} ROM_REGION;

typedef struct GAME_ROMS {
  GAME_INFO info;
  ROM_REGION cpu_m68k;
  ROM_REGION cpu_z80;
  ROM_REGION tiles;
  ROM_REGION game_sfix;
  ROM_REGION bios_sfix;
  ROM_REGION bios_audio;
  ROM_REGION zoom_table;
  ROM_REGION bios_m68k;
  ROM_REGION adpcma;
  ROM_REGION adpcmb;
  ROM_REGION spr_usage;
  ROM_REGION gfix_usage;
  ROM_REGION bfix_usage;
  ROM_REGION cpu_z80c;
} GAME_ROMS;

int dr_load_roms(GAME_ROMS *r, char *rom_path, char *name);
void dr_free_roms(GAME_ROMS *r);
int dr_save_gno(GAME_ROMS *r, char *filename);
int dr_load_game(char *zip);
ROM_DEF *dr_check_zip(const char *filename);
char *dr_gno_romname(char *filename);
int dr_open_gno(char *filename);
int init_game(char *rom_name);
int close_game(void);
char *remove_path_and_extension(char *mystr, char dot, char sep);

#endif

