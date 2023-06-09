
/* DrZ80 - Header File */

/* All code (c) Copyright 2004 Reesy, All rights reserved. */
/* DrZ80 is free for non-commercial use. */

/* For commercial use, separate licencing terms must be obtained. */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DRZ80_H
#define DRZ80_H

extern int DrZ80Ver; /* Version number of library */

struct DrZ80 {
  unsigned int Z80PC;           /*0x00 - PC Program Counter (Memory Base + PC) */
  unsigned int Z80A;            /*0x04 - A Register:   0xAA------ */
  unsigned int Z80F;            /*0x08 - F Register:   0xFF------ */
  unsigned int Z80BC;           /*0x0C - BC Registers: 0xBBCC---- */
  unsigned int Z80DE;           /*0x10 - DE Registers: 0xDDEE---- */
  unsigned int Z80HL;           /*0x14 - HL Registers: 0xHHLL---- */
  unsigned int Z80SP;           /*0x18 - SP Stack Pointer (Memory Base + PC) */
  unsigned int Z80PC_BASE;      /*0x1C - PC Program Counter (Memory Base) */
  unsigned int Z80SP_BASE;      /*0x20 - SP Stack Pointer (Memory Base) */
  unsigned int Z80IX;           /*0x24 - IX Index Register */
  unsigned int Z80IY;           /*0x28 - IY Index Register */
  unsigned int Z80I;            /*0x2C - I Interrupt Register */
  unsigned int Z80A2;           /*0x30 - A' Register:    0xAA------ */
  unsigned int Z80F2;           /*0x34 - F' Register:    0xFF------ */
  unsigned int Z80BC2;          /*0x38 - B'C' Registers: 0xBBCC---- */
  unsigned int Z80DE2;          /*0x3C - D'E' Registers: 0xDDEE---- */
  unsigned int Z80HL2;          /*0x40 - H'L' Registers: 0xHHLL---- */
  //unsigned int cycles;
  //unsigned int previouspc;
  unsigned char Z80_IRQ;        /*0x44 - Set IRQ Number */
  unsigned char Z80IF;          /*0x48 - Interrupt Flags:  bit1=_IFF1, bit2=_IFF2, bit3=_HALT */
  unsigned char Z80IM;          /*0x4C - Set IRQ Mode */
  unsigned char spare;          /*0x4C - N/A */
  unsigned int z80irqvector;    /*0x38 - Set IRQ Vector i.e. 0xFF=RST */
  void (*z80_irq_callback)(void);
  void (*z80_write8)(unsigned char d, unsigned short a);
  void (*z80_write16)(unsigned short d, unsigned short a);
  unsigned char (*z80_in)(unsigned short p);
  void (*z80_out)(unsigned short p, unsigned char d);
  unsigned char (*z80_read8)(unsigned short a);
  unsigned short (*z80_read16)(unsigned short a);
  unsigned int (*z80_rebaseSP)(unsigned short new_sp);
  unsigned int (*z80_rebasePC)(unsigned short new_pc);
};

extern void DrZ80Run(struct DrZ80 *pcy, unsigned int cyc);

#endif

#ifdef __cplusplus
} /* End of extern "C" */
#endif
