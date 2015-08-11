
#include "fw-common.h"
#define NEXTERN
#include "xmega-pdi.h"

void usleep(u16 us);
volatile __sbit __at(0xd0+0) PF; /* parity flag in PSW register */

/* doc8282, section 3.3 */
static __code const char pdi_nvm_key[8] = {
	'\x12','\x89','\xAB','\x45','\xCD','\xD8','\x88','\xFF'
};

u8 pdi_state;

static void pdi_cycle_clk(u8 cycles)
{
	do {
		PDI_CLK = 0;
		PDI_CLK = 1;
	} while (--cycles);
}

/* PDI init:
 * 1. PDI_DATA low -> high
 * 2. wait for "longer than the equivalent external reset minimum pulse width"
 *    (source: doc8282, section 3.2); datasheet sec 34.12: typ 90ns
 * 3. then, within 100us: start cycling PDI_CLK
 */
void pdi_init(void)
{
	/* enable outputs */
	PDI_CLK = 1;
	PDI_DATA = 0;
	OE_PDI |= PDI_DATA_MASK | PDI_CLK_MASK;

	/* 1. */
	PDI_DATA = 1;

	/* 2. wait for >= "external reset minimul pulse width" (typ. 90ns) */
	/* 166 ns <= 2 cycles <= 666 ns */
	NOP;
	NOP;

	/* 3. 16 cycles w/ PDI_DATA high */
	pdi_cycle_clk(16);
	pdi_state = 0;

	return;
}

#if 0
/* change to: fx2 <- xmega */
static void pdi_tx2rx(void)
{
	OE_PDI &= ~PDI_DATA_MASK;
	//PDI_DATA = 0;
	/* wait for configurable # cycles (2-128), see PDI CTRL register */
}

/* change to: fx2 -> xmega */
static void pdi_rx2tx(void)
{
	/* doc8077, XMEGA manual: 29.3.7:
	 * When the programmer changes from RX mode to TX mode, a single IDLE
	 * bit, at minimum, should be inserted before the start bit is
	 * transmitted. */
	PDI_DATA = 1;
	pdi_cycle_clk(1);
	OE_PDI |= PDI_DATA_MASK;
}

void pdi_break(void)
{
	if (!(OE_PDI & PDI_DATA_MASK))
		pdi_rx2tx();
	PDI_DATA = 0;
	pdi_cycle_clk(12);
	PDI_DATA = 1;
	pdi_cycle_clk(1);
	PDI_DATA = 0;
	pdi_cycle_clk(12);
	PDI_DATA = 1;
}
#endif

/* doc8282, section 3.5 */
/* PDI idle: PDI_CLK (= /RESET) high, PDI_DATA low
 * PDI exit: wait 100 us; PDI idle */
void pdi_exit(void)
{
	PDI_CLK = 1;
	PDI_DATA = 0;
	usleep(100);
	OEA &= ~(PDI_DATA_MASK | PDI_CLK_MASK);
	return;
}

/* PDI samples on rising PDI_CLK edge */
#if 0
void pdi_tx(u8 b)
{
#define PDI_BIT(b)	do { PDI_CLK = 0; PDI_DATA = (b); PDI_CLK = 1; p ^= (b); } while (0)
	u8 p = 0;
	OEA |= 2;			/* enable PDI_DATA output */
	PDI_BIT(0);			/* stop bit */
	PDI_BIT(b & 1); b >>= 1;	/* b0 */
	PDI_BIT(b & 1); b >>= 1;	/* b1 */
	PDI_BIT(b & 1); b >>= 1;	/* b2 */
	PDI_BIT(b & 1); b >>= 1;	/* b3 */
	PDI_BIT(b & 1); b >>= 1;	/* b4 */
	PDI_BIT(b & 1); b >>= 1;	/* b5 */
	PDI_BIT(b & 1); b >>= 1;	/* b6 */
	PDI_BIT(b & 1);			/* b7 */
	PDI_BIT(p & 1);			/* parity */
	PDI_BIT(1);			/* stop bit 1 */
	PDI_BIT(1);			/* stop bit 2 */
#undef PDI_BIT
}
#else
void pdi_tx(u8 b) __naked
{
	(void)b;
	__asm
#define PDI_BITc		\
	clr _PDI_CLK		\
	mov _PDI_DATA, c	\
	setb _PDI_CLK
#define PDI_BIT			\
	rrc a			\
	PDI_BITc

	push psw
	push acc

	mov a, _OE_PDI
	jb acc.1, pdi_tx_send_frame
	; doc8077, XMEGA manual: 29.3.7:
	; When the programmer changes from RX mode to TX mode, a single IDLE
	; bit, at minimum, should be inserted before the start bit is
	; transmitted.
	setb _PDI_DATA
	clr _PDI_CLK
	setb _PDI_CLK
	orl _OE_PDI, #PDI_DATA_MASK

pdi_tx_send_frame:
	mov a, dpl

	clr c
	PDI_BITc

	PDI_BIT
	PDI_BIT
	PDI_BIT
	PDI_BIT

	PDI_BIT
	PDI_BIT
	PDI_BIT
	PDI_BIT

	rrc a		; original byte to tfer in back a

	mov c, _PF
	PDI_BITc	; parity

	setb c
	PDI_BITc	; stop bit 1
	PDI_BITc	; stop bit 2

	mov dpl, a

	pop acc
	pop psw
	ret

#undef PDI_BIT
#undef PDI_BITc
	__endasm;
}
#endif

#if 0
u8 pdi_rx(void)
{
	u8 b;
	pdi_state &= ~PDI_STATE_ERR_MASK;
	for (b=0; IOA & 1; b--)
		if (!b) {
			pdi_state |= PDI_ERR_RX_TIMEOUT;
			return 0;
		}

}
#else
u8 pdi_rx(void) __naked
{
	__asm
#define PDI_BITc		\
	clr _PDI_CLK		\
	setb _PDI_CLK		\
	mov c, _PDI_DATA
#define PDI_BIT			\
	PDI_BITc		\
	rrc a

	push psw
	push acc
	push ar0

	clr a
	mov r0, a

	anl _pdi_state, #~(PDI_STATE_TOK_MASK | PDI_STATE_ERR_MASK)

	mov a, #PDI_DATA_MASK
	anl a, _OE_PDI
	jz pdi_rx_wait_start_frame
	cpl a
	anl _OE_PDI, a
	clr _PDI_CLK
	setb _PDI_CLK

pdi_rx_wait_start_frame:
	PDI_BITc
	jnc pdi_rx_started_frame
	djnz r0, pdi_rx_wait_start_frame

	; waited 256 PDI_CLK cycles, no frame start => timeout
	orl _pdi_state, #(PDI_TOK_ERR | PDI_ERR_RX_TIMEOUT)
	sjmp pdi_rx_ret

pdi_rx_started_frame:
	PDI_BIT
	PDI_BIT
	PDI_BIT
	PDI_BIT

	PDI_BIT
	PDI_BIT
	PDI_BIT
	PDI_BIT

	; save result
	mov dpl, a

	; check parity -> acc.0
	mov c, _PF
	clr a
	rlc a
	PDI_BITc
	addc a, #0x00

	; read stop bits
	PDI_BITc
	rlc a
	PDI_BITc
	anl c, acc.0
	cpl c
	mov acc.0, c

	; acc.1: parity failed
	; acc.0: NAND-ed stop bits = stop failed
	anl a, #(PDI_ERR_RX_PARITY_MASK | PDI_ERR_RX_STOP_MASK)
	jz pdi_rx_ret
	orl a, #(PDI_TOK_ERR)
	orl _pdi_state, a

pdi_rx_ret:
	pop ar0
	pop acc
	pop psw
	ret

#undef PDI_BIT
#undef PDI_BITc
	__endasm;
}
#endif

void nvm_init(void)
{
	u8 c, i;

	pdi_init();
	pdi_tx(PDI_STCS(PDI_CSREG_CTRL));
	pdi_tx(PDI_CSREG_CTRL_GUARDTIME_2B);

	pdi_tx(PDI_STCS(PDI_CSREG_RESET));
	pdi_tx(PDI_CSREG_RESET_MAGIC);

	pdi_tx(PDI_KEY());
	for (i=8; i;)
		pdi_tx(pdi_nvm_key[--i]);

	/* wait until NVM is available */
	i = 0;
	do {
		pdi_tx(PDI_LDCS(PDI_CSREG_STATUS));
		c = pdi_rx();
		if (pdi_state & PDI_STATE_ERR_MASK ||
		    c & PDI_CSREG_STATUS_NVMEN_MASK)
			break;
	} while (--i);

	if (!(c & PDI_CSREG_STATUS_NVMEN_MASK))
		pdi_state |= NVM_ERR_INIT_TIMEOUT;
}

#if 0
void pdi_set_ptr(u32 addr)
{
	pdi_tx(PDI_ST(2,3));
	pdi_tx( addr        & 0xff);
	pdi_tx((addr >>  8) & 0xff);
	pdi_tx((addr >> 16) & 0xff);
	pdi_tx((addr >> 24) & 0xff);
}
#else
void pdi_set_ptr(u32 addr) __naked
{
	(void)addr;
	__asm
	push dpl
	mov dpl, #PDI_ST(2,3)
	lcall _pdi_tx
	pop dpl
	lcall _pdi_tx
	mov dpl, dph
	lcall _pdi_tx
	mov dpl, b
	lcall _pdi_tx
	mov dpl, a
	ljmp _pdi_tx
	__endasm;
}
#endif

void pdi_set_ptr_io(u16 addr) __naked
{
	(void)addr;
	__asm
	push dpl
	mov dpl, #PDI_ST(2,3)
	lcall _pdi_tx
	pop dpl
	lcall _pdi_tx
	mov dpl, dph
	lcall _pdi_tx
	mov dpl, #(PDI_BASE_DATAMEM >> 16)
	lcall _pdi_tx
	mov dpl, #(PDI_BASE_DATAMEM >> 24)
	ljmp _pdi_tx
	__endasm;
}

u8 nvm_read(u32 addr, __xdata u8 *buf, u8 len)
{
#if 1
	(void)addr;
	(void)buf;
	(void)len;
	__asm
	mov r0, _nvm_read_PARM_3
	cjne r0, #0x00, 0002$
0000$:
	mov a, _nvm_read_PARM_3
	clr c
	subb a, r0
	mov dpl, a
	ret
0002$:
	mov r1, dpl
	mov r2, dph
	mov dptr, #NVM_REG_CMD
	lcall _pdi_set_ptr_io
	mov dpl, #PDI_ST(0,0)
	lcall _pdi_tx
	mov dpl, #NVM_READ_NVM
	lcall _pdi_tx

	mov dph, r2
	mov dpl, r1
	lcall _pdi_set_ptr
	mov dpl, #PDI_REPEAT(0)
	lcall _pdi_tx
	mov dpl, r0
	dec dpl
	lcall _pdi_tx
	mov dpl, #PDI_LD(1,0)
	lcall _pdi_tx

	mov _DPL1, (_nvm_read_PARM_2    )
	mov _DPH1, (_nvm_read_PARM_2 + 1)
0001$:
	lcall _pdi_rx
	mov a, dpl
	inc _DPS
	movx @dptr, a
	inc dptr
	dec _DPS
	mov a, _pdi_state
	anl a, #(PDI_STATE_ERR_MASK | PDI_STATE_NVM_MASK)
	jnz 0000$
	djnz r0, 0001$
	sjmp 0000$
	__endasm;
#else
	u8 olen = len;

	if (!len)
		return 0;

	pdi_set_ptr_io(NVM_REG_CMD);
	pdi_tx(PDI_ST(0,0));
	pdi_tx(NVM_READ_NVM);

	pdi_set_ptr(addr);
	pdi_tx(PDI_REPEAT(0));
	pdi_tx(len-1);
	pdi_tx(PDI_LD(1,0));

	do {
		*buf++ = pdi_rx();
		if (pdi_state & (PDI_STATE_ERR_MASK | PDI_STATE_NVM_MASK))
			break;
	} while (--len);

	return olen - len;
#endif
}
