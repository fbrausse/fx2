
#ifndef FW_COMMON_H
#define FW_COMMON_H

#define ALLOCATE_EXTERN
#include "fx2regs.h"
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef unsigned long DWORD;

__sfr __at (0x92) _XPAGE; /* Cypress EZ-USB family, Texas Instruments (Chipcon) a.k.a. MPAGE */

#define NOP  __asm nop __endasm

#endif
