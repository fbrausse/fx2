
/* to see the logs run
 *   $ fxprog/bulk -c 04b4:8613 -i 0 -a 1 -C -1 0x86 512
 *
 * it should print (something like):
 *
 *   logbuf inited
 *   OE_PDI: 00, PDI: ff, pdi_state: 00, c: 06, d: 02, s: 01
 *   OE_PDI: 00, PDI: fd, pdi_state: 00, c: 06
 *   ident: 1e 97 4c, rev: 07
 *   lotnum: 30 46 32 33 33, wafer: 12 at x: 0a 00 / y: 06 00
 */

#include "fw-common.h"
#include "xmega-pdi.h"

#define	SYNCDELAY	do { NOP; NOP; NOP; } while (0)

         __sbit __at(0xd0+5) F0;

/* sleeps for ADD cycles longer than us microsends.
 * ADD = (1+(us>>8))*3 + [3,4]+2+2+1+1+3+[2,9]+4+4+4
 *     =    (us>>8) *3 + [29,37]
 *
 * so method sleeps longer for:
 *
 *                           |     deviation
 * us>>8    min     max      |   min       max
 *   0      2.41   12.34  us | 241.67%  1233.4%
 *   1      2.66   13.34  us |   1.04%     2.6%
 *  255    66.16  267.34  us |    .01%      .5%
 *                           |
 * max for 20 <= us <= 255:  |  12.08%    66.7%
 *
 * precondition: 1 <= us <= 0xffff */
void usleep(u16 us)
{
	(void)us;
	__asm			; [3 or 4] for acall/lcall
	mov r0, dpl		; [2]
	mov r1, dph		; [2]
	inc r0			; [1]
	inc r1			; [1]

	mov dptr, #_CPUCS	; [3]
	movx a, @dptr		; [2 to 9]
	jb acc.5, usleep_12mhz	; [4]
	jb acc.4, usleep_6mhz	; [4]

usleep_3mhz:
	djnz r0, usleep_3mhz
	djnz r1, usleep_3mhz	; [3]
	ret			; [4]

usleep_6mhz:
	nop
	nop
	nop
	djnz r0, usleep_6mhz
	djnz r1, usleep_6mhz	; [3]
	ret			; [4]

usleep_12mhz:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	djnz r0, usleep_12mhz
	djnz r1, usleep_12mhz	; [3]
				; [4] for ret
	__endasm;
}

#if 0
#define IRQ_SUDAV	0x00	/* SETUP Data Available */
#define IRQ_SOF		0x04	/* Start of Frame (or microframe) */
#define IRQ_SUTOK	0x08	/* Setup Token Received */
#define IRQ_SUSPEND	0x0c	/* USB Suspend request */
#define IRQ_USB_RESET	0x10	/* Bus reset */
#define IRQ_HISPEED	0x14	/* Entered high-speed operation* */
#define IRQ_EP0ACK	0x18	/* EZ-USB ACK'd the CONTROL Handshake */
/* 0x1c reserved */
#define IRQ_EP0IN	0x20	/* EP0-IN ready to be loaded with data */
#define IRQ_EP0OUT	0x24	/* EP0-OUT has USB data */
#define IRQ_EP1IN	0x28	/* EP1-IN ready to be loaded with data */
#define IRQ_EP1OUT	0x2c	/* EP1-OUT has USB data */
#define IRQ_EP2		0x30	/* IN: buffer available, OUT: buffer has data */
#define IRQ_EP4		0x34	/* IN: buffer available, OUT: buffer has data */
#define IRQ_EP6		0x38	/* IN: buffer available, OUT: buffer has data */
#define IRQ_EP8		0x3c	/* IN: buffer available, OUT: buffer has data */
#define IRQ_IBN		0x40	/* IN-Bulk-NAK (any IN endpoint) */
/* 0x44 reserved */
#define IRQ_EP0PING	0x48	/* EP0 OUT was Pinged and it NAK'd */
#define IRQ_EP1PING	0x4c	/* EP1 OUT was Pinged and it NAK'd */
#define IRQ_EP2PING	0x50	/* EP2 OUT was Pinged and it NAK'd */
#define IRQ_EP4PING	0x54	/* EP4 OUT was Pinged and it NAK'd */
#define IRQ_EP6PING	0x58	/* EP6 OUT was Pinged and it NAK'd */
#define IRQ_EP8PING	0x5c	/* EP8 OUT was Pinged and it NAK'd */
#define IRQ_ERRLIMIT	0x60	/* Bus errors exceeded the programmed limit */
/* 0x64 reserved */
/* 0x68 reserved */
/* 0x6c reserved */
#define IRQ_EP2ISOERR	0x70	/* ISO EP2 OUT PID sequence error */
#define IRQ_EP4ISOERR	0x74	/* ISO EP4 OUT PID sequence error */
#define IRQ_EP6ISOERR	0x78	/* ISO EP6 OUT PID sequence error */
#define IRQ_EP8ISOERR	0x7c	/* ISO EP8 OUT PID sequence error */

static void usb_isr(void) __interrupt(8) __naked
{
	__asm
	push acc
	push psw
	push dpl
	push dph
	push ar0

	mov dptr, #_INT2IVEC	; get IRQ idx
	movx a, @dptr
	mov r0, a

	anl _EXIF, #~(1 << 4)	; first clear USB (INT2) interrupt request
	mov dptr, #_USBERRIRQ	; next clear specific USB interrupt request

	; logbuf sent
	mov a, (_logbuf + 4)
	rl a
	add a, #0x30
	cjne a, ar0, 0081$

	mov a, (_logbuf + 4)	; 0000, 0010, 0100, 0110
	rr a			; 0000, 0001, 0010, 0011
	add a, #3
	movc a, @a+pc
	movx @dptr, a		; clear EP interrupt request

	; all sent, reset logbuf
	

	sjmp 0080$
	.db 0x10
	.db 0x20
	.db 0x40
	.db 0x80
0081$:
	; unknown irq, clear all
	movx a, @dptr
	movx @dptr, a
0080$:
	pop ar0
	pop dph
	pop dpl
	pop psw
	pop acc
	reti

	__endasm;
}
#endif

static void fx2_init(void)
{
	CPUCS = 0x10; /* no #RD/#WR strobes for PORTC, 48 MHz, no clk-out signal */
	SYNCDELAY;

	IFCONFIG = 0xc0; /* internal IFCLK, 48 MHz for FIFOs/GPIF */
	SYNCDELAY;

	REVCTL = 0x03; /* rec. by Cypress; enable DYN_OUT, ENH_PKT */
	SYNCDELAY;

	EP6CFG=0xe2;  // 1110 0010 (bulk IN, 512 bytes, double-buffered)
	SYNCDELAY;

	/* USB initialization */
	EXIF &= ~(1 << 4); /* clear USB interrupt request */
	EIE &= ~(1 << 0); /* disable USB interrupt */

	FIFORESET = 0x80; SYNCDELAY;  // NAK all requests from host. 
	FIFORESET = 0x82; SYNCDELAY;  // Reset individual EP (2,4,6,8)
	FIFORESET = 0x84; SYNCDELAY;
	FIFORESET = 0x86; SYNCDELAY;
	FIFORESET = 0x88; SYNCDELAY;
	FIFORESET = 0x00; SYNCDELAY;  // Resume normal operation. 

	CKCO &= ~0x07; /* set wait delay for MOVX to 0 */

	OEA = OEB = OEC = OED = OEE = 0; /* tri-state outputs */

	EP6CS &= ~(1 << 0); /* reset STALL flag on EP2 */
}

#define LOGBUF_SZ	512

static struct logbuf {
	u16 valid;
	u8  ep_no_minus2;
} __data logbuf;

#define ep2468_buf(no_minus2)	((__xdata u8 *)(0xf000 | (no_minus2) << 8+1))

#include <stdio.h>

#define ENTER_USB_CRITICAL	\
	__asm			\
	push _EIE		\
	anl _EIE, #~(1 << 0)	\
	__endasm
#define LEAVE_USB_CRITICAL	\
	__asm			\
	pop _EIE		\
	__endasm

void logbuf_init(u8 ep_no)
{
	logbuf.valid = 0;
	logbuf.ep_no_minus2 = ep_no - 2;
}

static const __code u8 EP2468_FULL_MASK[] = {
	(1 << 1), (1 << 3), (1 << 5), (1 << 7)
};

volatile __xdata __at(0xe690) u8 EPxBCHL[4*4]; /* EPxBCH, EPxBCL, rsvd, rsvd */

void flush(void)
{
	/* flush: commit buffer to USB */
	__xdata u8 *bc = EPxBCHL + (u8)(logbuf.ep_no_minus2 << 1);
	*bc++ = logbuf.valid >> 8;
	*bc++ = logbuf.valid;
	logbuf.valid = 0;
}

int putchar(int c)
{
	u8 mask = EP2468_FULL_MASK[logbuf.ep_no_minus2 >> 1];
	while (EP2468STAT & mask);

	ENTER_USB_CRITICAL;	/* disable USB interrupt for critical section */
	if (logbuf.valid >= LOGBUF_SZ)
		flush();
	ep2468_buf(logbuf.ep_no_minus2)[logbuf.valid] = c;
	logbuf.valid++;
	if (c == '\n')
		flush();
	LEAVE_USB_CRITICAL;
}

static void set_timer0_100us(void) __naked
{
	__asm
	push acc
	push dph
	push dpl

	mov dptr, #_CPUCS
	movx a, @dptr

	jnb acc.4, 0003$
	mov _TH0, #(-(100 << 2) >> 8)
	mov _TL0, #(-(100 << 2))
	sjmp 0001$
0003$:
	jnb acc.3, 0002$
	mov _TH0, #(-(100 << 1) >> 8)
	mov _TL0, #(-(100 << 1))
	sjmp 0001$
0002$:
	mov _TH0, #(-(100 << 0) >> 8)
	mov _TL0, #(-(100 << 0))
0001$:
	anl _TMOD, #~(1 << 2 | 3)	; Mode 0: 13 bit, by CLKOUT ...
	anl _CKCO, #~(1 << 3)		; ... by CLKOUT/12

	pop dpl
	pop dph
	pop acc
	ret
	__endasm;
}

int main()
{
	u8 c = 0, d, s;
	u8 i = 0;
	__xdata u8 ident[4];
	__xdata u8 lotnum[6], wafnum, coordxy[4];

	fx2_init();
	logbuf_init(6);

	printf("logbuf inited\n");

	nvm_init();

	pdi_tx(PDI_LDCS(PDI_CSREG_CTRL));
	c = pdi_rx();
	pdi_tx(PDI_LDCS(PDI_CSREG_STATUS));
	d = pdi_rx();
	pdi_tx(PDI_LDCS(PDI_CSREG_RESET));
	s = pdi_rx();
	/* within 100us: tfer sth via PDI, otherwise it shuts down */

	nvm_read(PDI_ADDR_DATAMEM(0x90), ident, 4);

	nvm_read(PDI_ADDR_SIG_PROD(0x08), lotnum, 6);
	nvm_read(PDI_ADDR_SIG_PROD(0x10), &wafnum, 1);
	nvm_read(PDI_ADDR_SIG_PROD(0x12), coordxy, 4);

	pdi_exit();

	printf("OE_PDI: %02x, PDI: %02x, pdi_state: %02x, c: %02x, d: %02x, s: %02x\n",
	       OE_PDI, IOA, pdi_state, c, d, s);
	printf("OE_PDI: %02x, PDI: %02x, pdi_state: %02x, c: %02x\n", OE_PDI, IOA, pdi_state, c);

	printf("ident: %02x %02x %02x, rev: %02x\n", ident[0], ident[1], ident[2], ident[3]);
	printf("lotnum: %02x %02x %02x %02x %02x", lotnum[0], lotnum[1], lotnum[2], lotnum[3], lotnum[4], lotnum[5]);
	printf(", wafer: %02x at x: %02x %02x / y: %02x %02x\n", wafnum, coordxy[0], coordxy[1], coordxy[2], coordxy[3]);

#if 0
	do {
		printf("ehehe: %hhu\n", (0x100-i) & 0xff);
	} while (--i);

	printf("so, finished!\n");
	while (1);

#endif
	while (1);
}
