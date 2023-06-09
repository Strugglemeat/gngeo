
// Cyclone 68000 Emulator - Header File

// (c) Copyright 2004 Dave, All rights reserved.
// (c) 2005-2007 notaz
// Cyclone 68000 is free for non-commercial use.

// For commercial use, separate licencing terms must be obtained.


#ifndef __CYCLONE_H__
#define __CYCLONE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int CycloneVer; // Version number of library

struct Cyclone {
  unsigned int d[8];    // [r7,#0x00]
  unsigned int a[8];    // [r7,#0x20]
  unsigned int pc;      // [r7,#0x40] Memory Base (.membase) + 68k PC
  unsigned char srh;    // [r7,#0x44] Status Register high (T_S__III)
  unsigned char unused; // [r7,#0x45] Unused
  unsigned char flags;  // [r7,#0x46] Flags (ARM order: ____NZCV) [68k order is XNZVC]
  unsigned char irq;    // [r7,#0x47] IRQ level
  unsigned int osp;     // [r7,#0x48] Other Stack Pointer (USP/SSP)
  unsigned int xc;      // [r7,#0x4c] Extend flag (bit29: ??X? _)
  unsigned int prev_pc; // [r7,#0x50] Set to start address of currently executed opcode + 2 (if enabled in config.h)
  unsigned int reserved;// [r7,#0x54] Reserved for possible future use
  int state_flags;      // [r7,#0x58] bit: 0: stopped state, 1: trace state, 2: activity bit, 3: addr error, 4: fatal halt
  int cycles;           // [r7,#0x5c] Number of cycles to execute - 1. Updates to cycles left after CycloneRun()
  int membase;          // [r7,#0x60] Memory Base (ARM address minus 68000 address)
  unsigned int (*checkpc)(unsigned int pc); // [r7,#0x64] called to recalc Memory Base+pc
  unsigned int (*read8)(unsigned int a);    // [r7,#0x68]
  unsigned int (*read16)(unsigned int a);   // [r7,#0x6c]
  unsigned int (*read32)(unsigned int a);   // [r7,#0x70]
  void (*write8)(uint32_t a, uint32_t  d);  // [r7,#0x74]
  void (*write16)(uint32_t a, uint32_t d); // [r7,#0x78]
  void (*write32)(uint32_t a, uint32_t   d); // [r7,#0x7c]
  unsigned int (*fetch8)(unsigned int a);   // [r7,#0x80]
  unsigned int (*fetch16)(unsigned int a);  // [r7,#0x84]
  unsigned int (*fetch32)(unsigned int a);  // [r7,#0x88]
  int (*IrqCallback)(int int_level);        // [r7,#0x8c] optional irq callback function, see config.h
  void (*ResetCallback)(void);              // [r7,#0x90] if enabled in config.h, calls this whenever RESET opcode is encountered.
  int (*UnrecognizedCallback)(void);        // [r7,#0x94] if enabled in config.h, calls this whenever unrecognized opcode is encountered.
  unsigned int internal[6];                 // [r7,#0x98] reserved for internal use, do not change.
};

// Initialize. Used only if Cyclone was compiled with compressed jumptable, see config.h
void CycloneInit(void);

// Run cyclone. Cycles should be specified in context (pcy->cycles)
void CycloneRun(struct Cyclone *pcy);

// Utility functions to get and set SR
void CycloneSetSr(struct Cyclone *pcy, unsigned int sr);
unsigned int CycloneGetSr(const struct Cyclone *pcy);

// Generates irq exception if needed (if pcy->irq > mask).
// Returns cycles used for exception if it was generated, 0 otherwise.
int CycloneFlushIrq(struct Cyclone *pcy);

// Functions for saving and restoring state.
// CycloneUnpack() uses checkpc(), so it must be initialized.
// save_buffer must point to buffer of 128 (0x80) bytes of size.
void CyclonePack(const struct Cyclone *pcy, void *save_buffer);
void CycloneUnpack(struct Cyclone *pcy, const void *save_buffer);

// genesis: if 1, switch to normal TAS handlers
void CycloneSetRealTAS(int use_real);


// These values are special return values for IrqCallback.

// Causes an interrupt autovector (0x18 + interrupt level) to be taken.
// This happens in a real 68K if VPA or AVEC is asserted during an interrupt
// acknowledge cycle instead of DTACK (the most common situation).
#define CYCLONE_INT_ACK_AUTOVECTOR    -1

// Causes the spurious interrupt vector (0x18) to be taken
// This happens in a real 68K if BERR is asserted during the interrupt
// acknowledge cycle (i.e. no devices responded to the acknowledge).
#define CYCLONE_INT_ACK_SPURIOUS      -2


#ifdef __cplusplus
} // End of extern "C"
#endif

#endif // __CYCLONE_H__

