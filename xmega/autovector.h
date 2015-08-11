
#ifndef AUTOVECTOR_H
#define AUTOVECTOR_H

void sudav_isr(void)     __interrupt;
void sof_isr(void)       __interrupt;
void sutok_isr(void)     __interrupt;
void suspend_isr(void)   __interrupt;
void usbreset_isr(void)  __interrupt;
void hispeed_isr(void)   __interrupt;
void ep0ack_isr(void)    __interrupt;
void spare_isr(void)     __interrupt;
void ep0in_isr(void)     __interrupt;
void ep0out_isr(void)    __interrupt;
void ep1in_isr(void)     __interrupt;
void ep1out_isr(void)    __interrupt;
void ep2_isr(void)       __interrupt;
void ep4_isr(void)       __interrupt;
void ep6_isr(void)       __interrupt;
void ep8_isr(void)       __interrupt;
void ibn_isr(void)       __interrupt;
void ep0ping_isr(void)   __interrupt;
void ep1ping_isr(void)   __interrupt;
void ep2ping_isr(void)   __interrupt;
void ep4ping_isr(void)   __interrupt;
void ep6ping_isr(void)   __interrupt;
void ep8ping_isr(void)   __interrupt;
void errlimit_isr(void)  __interrupt;
void ep2isoerr_isr(void) __interrupt;
void ep4isoerr_isr(void) __interrupt;
void ep6isoerr_isr(void) __interrupt;
void ep8isoerr_isr(void) __interrupt;
void ep2pf_isr(void)     __interrupt;
void ep4pf_isr(void)     __interrupt;
void ep6pf_isr(void)     __interrupt;
void ep8pf_isr(void)     __interrupt;
void ep2ef_isr(void)     __interrupt;
void ep4ef_isr(void)     __interrupt;
void ep6ef_isr(void)     __interrupt;
void ep8ef_isr(void)     __interrupt;
void ep2ff_isr(void)     __interrupt;
void ep4ff_isr(void)     __interrupt;
void ep6ff_isr(void)     __interrupt;
void ep8ff_isr(void)     __interrupt;
void gpifdone_isr(void)  __interrupt;
void gpifwf_isr(void)    __interrupt;

#endif
