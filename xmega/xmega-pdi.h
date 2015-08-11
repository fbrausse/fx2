
#ifndef XMEGA_PDI_H
#define XMEGA_PDI_H

/* PDI uses UART frames: 8O2 (1 start bit, 8 data bits, 1 parity, 2 stop bits) */

#define PDI_CMD_LDS		0 /* load data from data space, direct addr */
#define PDI_CMD_LD		1 /* load data from data space, indirect addr */
#define PDI_CMD_STS		2 /* store data to data space, direct addr */
#define PDI_CMD_ST		3 /* store data to data space, indirect addr */
#define PDI_CMD_LDCS		4 /* LDS Control/Status */
#define PDI_CMD_REPEAT		5 /* set instr set repeat counter */
#define PDI_CMD_STCS		6 /* STS Control/Status */
#define PDI_CMD_KEY		7 /* set activation key, 8 bytes */

/* cmd: one of the above 8 PDI_*
 * [ab]: 0-3 |    0    |    1    |    2    |       3       |
 * ----------+---------+---------+---------+---------------+
 * size_[ab] | 1 byte  | 2 bytes | 3 bytes | 4 bytes       | (a: addr, b: data)
 * ptr       | *ptr    | *ptr++  | ptr     | ptr++ (resvd) |
 * cs_addr   | reg 0   | reg 1   | reg 2   | resvd...      |
 *
 * multi-byte values use big endian
 * ptr addresses are little-endian
 */
#define PDI_LDS(sz_a,sz_b)	((PDI_CMD_LDS   ) << 5 | (sz_a) << 2 | (sz_b ))
#define PDI_STS(sz_a,sz_b)	((PDI_CMD_STS   ) << 5 | (sz_a) << 2 | (sz_b ))
#define PDI_LD(ptr,sz_ab)	((PDI_CMD_LD    ) << 5 | (ptr ) << 2 | (sz_ab))
#define PDI_ST(ptr,sz_ab)	((PDI_CMD_ST    ) << 5 | (ptr ) << 2 | (sz_ab))
#define PDI_LDCS(cs_addr)	((PDI_CMD_LDCS  ) << 5 | (cs_addr            ))
#define PDI_STCS(cs_addr)	((PDI_CMD_STCS  ) << 5 | (cs_addr            ))
#define PDI_REPEAT(sz_b)	((PDI_CMD_REPEAT) << 5 |               (sz_b ))
#define PDI_KEY()		((PDI_CMD_KEY   ) << 5                        )

/* doc8077, XMEGA manual, section 29.7; via LDCS and STCS commands */
#define PDI_CSREG_STATUS		0x00
#define PDI_CSREG_STATUS_NVMEN_MASK	(1 << 1)
#define PDI_CSREG_RESET			0x01
#define PDI_CSREG_RESET_MAGIC		0x59
#define PDI_CSREG_RESET_MASK		0x01
#define PDI_CSREG_CTRL			0x02
#define PDI_CSREG_CTRL_GUARDTIME_MASK	(7 << 0)
#define PDI_CSREG_CTRL_GUARDTIME_128B	(0 << 0)
#define PDI_CSREG_CTRL_GUARDTIME_64B	(1 << 0)
#define PDI_CSREG_CTRL_GUARDTIME_32B	(2 << 0)
#define PDI_CSREG_CTRL_GUARDTIME_16B	(3 << 0)
#define PDI_CSREG_CTRL_GUARDTIME_8B	(4 << 0)
#define PDI_CSREG_CTRL_GUARDTIME_4B	(5 << 0)
#define PDI_CSREG_CTRL_GUARDTIME_2B	(6 << 0)

#define PDI_CLK_BIT	0
#define PDI_DATA_BIT	1
#define PDI_CLK_MASK	(1 << PDI_CLK_BIT)
#define PDI_DATA_MASK	(1 << PDI_DATA_BIT)
volatile __sbit __at(0x80) PDI_CLK;  /* PA0, = /RESET (pulled up internally by XMEGA) */
volatile __sbit __at(0x81) PDI_DATA; /* PA1         (pulled down internally by XMEGA) */
volatile __sfr  __at(0xb2) OE_PDI; /* OEA */

#define PDI_STATE_ERR_SHIFT	0
#define PDI_STATE_ERR_MASK	(0x03 << PDI_STATE_ERR_SHIFT)
#define PDI_ERR_RX_TIMEOUT	(0 << PDI_STATE_ERR_SHIFT)
#define PDI_ERR_RX_STOP_MASK	(1 << PDI_STATE_ERR_SHIFT)
#define PDI_ERR_RX_PARITY_MASK	(2 << PDI_STATE_ERR_SHIFT)
#define PDI_STATE_TOK_SHIFT	2
#define PDI_STATE_TOK_MASK	(0x03 << PDI_STATE_TOK_SHIFT)
#define PDI_TOK_DATA		(0 << PDI_STATE_TOK_SHIFT)
#define PDI_TOK_IDLE		(1 << PDI_STATE_TOK_SHIFT)
#define PDI_TOK_BREAK		(2 << PDI_STATE_TOK_SHIFT)
#define PDI_TOK_ERR		(3 << PDI_STATE_TOK_SHIFT)
#define PDI_STATE_NVM_SHIFT	4
#define PDI_STATE_NVM_MASK	(0x0f << PDI_STATE_NVM_SHIFT)
#define NVM_ERR_INIT_TIMEOUT	(1 << PDI_STATE_NVM_MASK)

#ifndef NEXTERN
/* 1:0 = error
 * 3:2 = token
 * 7:4 = nvm errs
 */
extern u8 pdi_state;
#endif

void pdi_init(void);
// void pdi_break(void);
void pdi_tx(u8 b) __naked;
u8   pdi_rx(void) __naked;
void pdi_exit(void);

#define PDI_BASE_DATAMEM	0x01000000	/* size: 0x1000000 */ /* mapped IO/SRAM */
#define PDI_BASE_FUSE		0x008f0020
#define PDI_BASE_SIGNATURE_USER	0x008e0400
#define PDI_BASE_SIGNATURE_PROD	0x008e0200
#define PDI_BASE_EEPROM		0x008c0000	/* size: 0x1000 */
#define PDI_BASE_FLASH		0x00800000

#define PDI_BASE_BOOT		(PDI_BASE_FLASH + SIZE_APPL)

#define PDI_ADDR_DATAMEM(a)	((a) + PDI_BASE_DATAMEM)
#define PDI_ADDR_FUSE(a)	((a) + PDI_BASE_FUSE)
#define PDI_ADDR_SIG_USER(a)	((a) + PDI_BASE_SIGNATURE_USER)
#define PDI_ADDR_SIG_PROD(a)	((a) + PDI_BASE_SIGNATURE_PROD)
#define PDI_ADDR_EEPROM(a)	((a) + PDI_BASE_EEPROM)
#define PDI_ADDR_FLASH(a)	((a) + PDI_BASE_FLASH)

#define NVM_REG_BASE		0x01c0
#define NVM_REG_ADDR0		(NVM_REG_BASE + 0x00)
#define NVM_REG_ADDR1		(NVM_REG_BASE + 0x01)
#define NVM_REG_ADDR2		(NVM_REG_BASE + 0x02)
#define NVM_REG_DATA0		(NVM_REG_BASE + 0x04)
#define NVM_REG_DATA1		(NVM_REG_BASE + 0x05)
#define NVM_REG_DATA2		(NVM_REG_BASE + 0x06)
#define NVM_REG_CMD		(NVM_REG_BASE + 0x0a)
#define NVM_REG_CTRLA		(NVM_REG_BASE + 0x0b)
#define NVM_REG_CTRLB		(NVM_REG_BASE + 0x0c)
#define NVM_REG_INTCTRL		(NVM_REG_BASE + 0x0d)
#define NVM_REG_STATUS		(NVM_REG_BASE + 0x0f)
#define NVM_REG_LOCKBITS	(NVM_REG_BASE + 0x10)

						/*             change    nvm  */
						/* trigger  , protected, busy */
#define NVM_NOP				0x00
#define NVM_CHIP_ERASE			0x40	/* CMDEX    ,     y    ,  y */
#define NVM_READ_NVM			0x43	/* PDI read ,     n    ,  n */
/* flash page buffer */
#define NVM_LD_FLASH_PAGE_BUF		0x23	/* PDI write,     n    ,  n */
#define NVM_ER_FLASH_PAGE_BUF		0x26	/* CMDEX    ,     y    ,  y */
/* flash */
#define NVM_ER_FLASH_PAGE		0x2b	/* PDI write,     n    ,  y */
#define NVM_WR_FLASH_PAGE		0x2e	/* PDI write,     n    ,  y */
#define NVM_EW_FLASH_PAGE		0x2f	/* PDI write,     n    ,  y */
#define NVM_CRC_FLASH			0x78	/* CMDEX    ,     y    ,  y */
/* application section */
#define NVM_ER_APP_SECTION		0x20	/* PDI write,     n    ,  y */
#define NVM_ER_APP_SECTION_PAGE		0x22	/* PDI write,     n    ,  y */
#define NVM_WR_APP_SECTION_PAGE		0x24	/* PDI write,     n    ,  y */
#define NVM_EW_APP_SECTION_PAGE		0x25	/* PDI write,     n    ,  y */
#define NVM_CRC_APP_SECTION		0x38	/* CMDEX    ,     y    ,  y */
/* boot loader section */
#define NVM_ER_BOOT_SECTION		0x68	/* PDI write,     n    ,  y */
#define NVM_ER_BOOT_SECTION_PAGE	0x2a	/* PDI write,     n    ,  y */
#define NVM_WR_BOOT_SECTION_PAGE	0x2c	/* PDI write,     n    ,  y */
#define NVM_EW_BOOT_SECTION_PAGE	0x2d	/* PDI write,     n    ,  y */
#define NVM_CRC_BOOT_SECTION		0x39	/* NVMAA    ,     y    ,  y */
/* production signature (calibration) and user signature sections */
#define NVM_RD_USER_SIG_ROW		0x01	/* PDI read ,     n    ,  n */
#define NVM_ER_USER_SIG_ROW		0x18	/* PDI write,     n    ,  y */
#define NVM_WR_USER_SIG_ROW		0x1a	/* PDI write,     n    ,  y */
#define NVM_RD_PROD_SIG_ROW		0x02	/* PDI read ,     n    ,  n */
/* fuses and lock bits */
#define NVM_RD_FUSE			0x07	/* PDI read ,     n    ,  n */
#define NVM_WR_FUSE			0x4c	/* PDI write,     n    ,  y */
#define NVM_WR_LOCK_BITS		0x08	/* CMDEX    ,     y    ,  y */
/* EEPROM page buffer */
#define NVM_LD_EEPROM_PAGE_BUF		0x33	/* PDI write,     n    ,  n */
#define NVM_ER_EEPROM_PAGE_BUF		0x36	/* CMDEX    ,     y    ,  y */
/* EEPROM */
#define NVM_ER_EEPROM			0x30	/* CMDEX    ,     y    ,  y */
#define NVM_ER_EEPROM_PAGE		0x32	/* PDI write,     n    ,  y */
#define NVM_WR_EEPROM_PAGE		0x34	/* PDI write,     n    ,  y */
#define NVM_EW_EEPROM_PAGE		0x35	/* PDI write,     n    ,  y */
#define NVM_RD_EEPROM			0x06	/* PDI read ,     n    ,  n */

/* STS NVM-commands in NVM CMD register */

void pdi_set_ptr(u32 addr) __naked;
void pdi_set_ptr_io(u16 addr) __naked;

void nvm_init(void);
u8   nvm_read(u32 addr, __xdata u8 *buf, u8 len);

#endif
