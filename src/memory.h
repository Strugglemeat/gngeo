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

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <SDL.h>
#include <SDL_endian.h>

#include "video.h"
#include "ym2610_interf.h"
#include "state.h"
#include "roms.h"

#define READ_WORD(a)          (*(uint16_t *)(a))
#define WRITE_WORD(a,d)       (*(uint16_t *)(a) = (d))
#define READ_BYTE(a)          (*(uint8_t *)(a))
#define WRITE_BYTE(a,d)       (*(uint8_t *)(a) = (d))
#define SWAP_BYTE_ADDRESS(a)  ((uint32_t)(a)^1)
#define SWAP16(y) SDL_Swap16(y)
#define SWAP32(y) SDL_Swap32(y)

#  undef WRITE_WORD_ROM
#  undef READ_WORD_ROM
#  undef WRITE_BYTE_ROM
#  undef READ_BYTE_ROM

#  define WRITE_WORD_ROM WRITE_WORD
#  define READ_WORD_ROM READ_WORD

#  define WRITE_BYTE_ROM(a,d) WRITE_BYTE(SWAP_BYTE_ADDRESS(a),(d))
#  define READ_BYTE_ROM(a) READ_BYTE(SWAP_BYTE_ADDRESS(a))


#define GFX_MAPPED 1
#define GZX_MAPPED 2



typedef struct neo_mem {
  GAME_ROMS rom;
  uint8_t ram[0x10000];
  VIDEO vid;
  uint8_t *ng_lo;                          /* Put it in memory.vid? use zoom table in rom */

  uint32_t nb_of_tiles;

  uint8_t sram[0x10000];

  //	uint32_t *pen_usage;                      /* TODO: it's also in rom  */
  uint8_t fix_board_usage[4096];
  uint8_t *fix_game_usage;

  uint8_t z80_ram[0x800];
  uint8_t game_vector[0x80];
  int   current_vector;

  uint8_t intern_p1, intern_p2, intern_coin, intern_start;
  uint32_t bksw_handler;

  uint8_t *bksw_unscramble;
  int *bksw_offset;
  uint16_t sma_rng_addr;
  uint8_t memcard[0x800];

  uint32_t watchdog;
} neo_mem;

extern neo_mem memory;

/* video related */
//extern int irq2start, irq2control;
extern uint8_t *current_pal;
extern uint32_t *current_pc_pal;
extern uint8_t *current_fix;
extern uint8_t *fix_usage;

/* sram */
extern uint8_t sram_lock;
//uint32_t sram_protection_hack;
//int sram_protection_hack;

/* Sound control */
extern uint8_t sound_code;
extern uint8_t pending_command;
extern uint8_t result_code;


/* 68k cpu Banking control */
extern uint32_t bankaddress;		/* current bank */
//uint8_t current_cpu_bank;
extern uint16_t z80_bank[4];

/* misc utility func */
void update_all_pal(void);
void dump_hardware_reg(void);

/* cpu 68k interface */
int cpu_68k_getcycle(void);
void cpu_68k_init(void);
void cpu_68k_reset(void);
int cpu_68k_run(uint32_t nb_cycle);
void cpu_68k_interrupt(int a);
void cpu_68k_bankswitch(uint32_t address);
void cpu_68k_disassemble(int pc, int nb_instr);
void cpu_68k_dumpreg(void);
int cpu_68k_run_step(void);
uint32_t cpu_68k_getpc(void);
int cpu_68k_debuger(void (*execstep)(void), void (*dump)(void));


/* cpu z80 interface */
void cpu_z80_run(int nbcycle);
void cpu_z80_nmi(void);
void cpu_z80_raise_irq(int l);
void cpu_z80_lower_irq(void);
void cpu_z80_init(void);
void cpu_z80_switchbank(uint8_t bank, uint16_t PortNo);
uint8_t z80_port_read(uint16_t PortNo);
void z80_port_write(uint16_t PortNb, uint8_t Value);

/* memory handler prototype */
void neogeo_sound_irq(int irq);


#define LONG_FETCH(fetchname) uint32_t fetchname ## _long(uint32_t addr) { \
      return (fetchname ## _word(addr) << 16) |	fetchname ## _word(addr+2); \
}

#define LONG_STORE(storename) void storename ## _long(uint32_t addr, uint32_t data) { \
      storename ## _word(addr,data>>16); \
      storename ## _word(addr+2,data & 0xffff); \
}

/* 68k fetching function */
uint8_t mem68k_fetch_ram_byte(uint32_t addr);
uint16_t mem68k_fetch_ram_word(uint32_t addr);
uint32_t mem68k_fetch_ram_long(uint32_t addr);
uint8_t mem68k_fetch_invalid_byte(uint32_t addr);
uint16_t mem68k_fetch_invalid_word(uint32_t addr);
uint32_t mem68k_fetch_invalid_long(uint32_t addr);
uint8_t mem68k_fetch_bk_normal_byte(uint32_t addr);
uint16_t mem68k_fetch_bk_normal_word(uint32_t addr);
uint32_t mem68k_fetch_bk_normal_long(uint32_t addr);
uint8_t mem68k_fetch_cpu_byte(uint32_t addr);
uint16_t mem68k_fetch_cpu_word(uint32_t addr);
uint32_t mem68k_fetch_cpu_long(uint32_t addr);
uint8_t mem68k_fetch_bios_byte(uint32_t addr);
uint16_t mem68k_fetch_bios_word(uint32_t addr);
uint32_t mem68k_fetch_bios_long(uint32_t addr);
uint8_t mem68k_fetch_sram_byte(uint32_t addr);
uint16_t mem68k_fetch_sram_word(uint32_t addr);
uint32_t mem68k_fetch_sram_long(uint32_t addr);
uint8_t mem68k_fetch_pal_byte(uint32_t addr);
uint16_t mem68k_fetch_pal_word(uint32_t addr);
uint32_t mem68k_fetch_pal_long(uint32_t addr);
uint8_t mem68k_fetch_video_byte(uint32_t addr);
uint16_t mem68k_fetch_video_word(uint32_t addr);
uint32_t mem68k_fetch_video_long(uint32_t addr);
uint8_t mem68k_fetch_ctl1_byte(uint32_t addr);
uint16_t mem68k_fetch_ctl1_word(uint32_t addr);
uint32_t mem68k_fetch_ctl1_long(uint32_t addr);
uint8_t mem68k_fetch_ctl2_byte(uint32_t addr);
uint16_t mem68k_fetch_ctl2_word(uint32_t addr);
uint32_t mem68k_fetch_ctl2_long(uint32_t addr);
uint8_t mem68k_fetch_ctl3_byte(uint32_t addr);
uint16_t mem68k_fetch_ctl3_word(uint32_t addr);
uint32_t mem68k_fetch_ctl3_long(uint32_t addr);
uint8_t mem68k_fetch_coin_byte(uint32_t addr);
uint16_t mem68k_fetch_coin_word(uint32_t addr);
uint32_t mem68k_fetch_coin_long(uint32_t addr);
uint8_t mem68k_fetch_memcrd_byte(uint32_t addr);
uint16_t mem68k_fetch_memcrd_word(uint32_t addr);
uint32_t mem68k_fetch_memcrd_long(uint32_t addr);
uint8_t mem68k_fetch_bk_kof2003_byte(uint32_t addr);
uint16_t mem68k_fetch_bk_kof2003_word(uint32_t addr);
uint32_t mem68k_fetch_bk_kof2003_long(uint32_t addr);

/* 68k storring function */
void mem68k_store_invalid_byte(uint32_t addr, uint8_t data);
void mem68k_store_invalid_word(uint32_t addr, uint16_t data);
void mem68k_store_invalid_long(uint32_t addr, uint32_t data);
void mem68k_store_ram_byte(uint32_t addr, uint8_t data);
void mem68k_store_ram_word(uint32_t addr, uint16_t data);
void mem68k_store_ram_long(uint32_t addr, uint32_t data);
void mem68k_store_bk_normal_byte(uint32_t addr, uint8_t data);
void mem68k_store_bk_normal_word(uint32_t addr, uint16_t data);
void mem68k_store_bk_normal_long(uint32_t addr, uint32_t data);
void mem68k_store_sram_byte(uint32_t addr, uint8_t data);
void mem68k_store_sram_word(uint32_t addr, uint16_t data);
void mem68k_store_sram_long(uint32_t addr, uint32_t data);
void mem68k_store_pal_byte(uint32_t addr, uint8_t data);
void mem68k_store_pal_word(uint32_t addr, uint16_t data);
void mem68k_store_pal_long(uint32_t addr, uint32_t data);
void mem68k_store_video_byte(uint32_t addr, uint8_t data);
void mem68k_store_video_word(uint32_t addr, uint16_t data);
void mem68k_store_video_long(uint32_t addr, uint32_t data);
void mem68k_store_pd4990_byte(uint32_t addr, uint8_t data);
void mem68k_store_pd4990_word(uint32_t addr, uint16_t data);
void mem68k_store_pd4990_long(uint32_t addr, uint32_t data);
void mem68k_store_z80_byte(uint32_t addr, uint8_t data);
void mem68k_store_z80_word(uint32_t addr, uint16_t data);
void mem68k_store_z80_long(uint32_t addr, uint32_t data);
void mem68k_store_setting_byte(uint32_t addr, uint8_t data);
void mem68k_store_setting_word(uint32_t addr, uint16_t data);
void mem68k_store_setting_long(uint32_t addr, uint32_t data);
void mem68k_store_memcrd_byte(uint32_t addr, uint8_t data);
void mem68k_store_memcrd_word(uint32_t addr, uint16_t data);
void mem68k_store_memcrd_long(uint32_t addr, uint32_t data);
void mem68k_store_bk_kof2003_byte(uint32_t addr, uint8_t data);
void mem68k_store_bk_kof2003_word(uint32_t addr, uint16_t data);
void mem68k_store_bk_kof2003_long(uint32_t addr, uint32_t data);

extern uint8_t(*mem68k_fetch_bksw_byte)(uint32_t);
extern uint16_t(*mem68k_fetch_bksw_word)(uint32_t);
extern uint32_t(*mem68k_fetch_bksw_long)(uint32_t);
extern void (*mem68k_store_bksw_byte)(uint32_t, uint8_t);
extern void (*mem68k_store_bksw_word)(uint32_t, uint16_t);
extern void (*mem68k_store_bksw_long)(uint32_t, uint32_t);
void write_neo_control(uint16_t data);
void write_irq2pos(uint32_t data);

#endif
