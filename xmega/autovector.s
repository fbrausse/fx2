
	.module AUTOVECTOR
	.globl  _int2jt
	.globl  _int4jt

	.area INT2AV (ABS,OVR)
	.org  0x43
_INT2AV = #. + 2;
	ljmp _int2jt

	.area INT4AV (ABS,OVR)
	.org  0x53
_INT4AV = #. + 2
	ljmp _int4jt

	.area INT2JT	(CODE)
_int2jt:
	ljmp _sudav_isr
	.db 0
	ljmp _sof_isr
	.db 0
	ljmp _sutok_isr
	.db 0
	ljmp _suspend_isr
	.db 0
	ljmp _usbreset_isr
	.db 0
	ljmp _hispeed_isr
	.db 0
	ljmp _ep0ack_isr
	.db 0
	ljmp _spare_isr
	.db 0
	ljmp _ep0in_isr
	.db 0
	ljmp _ep0out_isr
	.db 0
	ljmp _ep1in_isr
	.db 0
	ljmp _ep1out_isr
	.db 0
	ljmp _ep2_isr
	.db 0
	ljmp _ep4_isr
	.db 0
	ljmp _ep6_isr
	.db 0
	ljmp _ep8_isr
	.db 0
	ljmp _ibn_isr
	.db 0
	ljmp _spare_isr
	.db 0
	ljmp _ep0ping_isr
	.db 0
	ljmp _ep1ping_isr
	.db 0
	ljmp _ep2ping_isr
	.db 0
	ljmp _ep4ping_isr
	.db 0
	ljmp _ep6ping_isr
	.db 0
	ljmp _ep8ping_isr
	.db 0
	ljmp _errlimit_isr
	.db 0
	ljmp _spare_isr
	.db 0
	ljmp _spare_isr
	.db 0
	ljmp _spare_isr
	.db 0
	ljmp _ep2isoerr_isr
	.db 0
	ljmp _ep4isoerr_isr
	.db 0
	ljmp _ep6isoerr_isr
	.db 0
	ljmp _ep8isoerr_isr
	.db 0

	.area INT4JT	(CODE)
_int4jt:
	ljmp _ep2pf_isr
	.db 0
	ljmp _ep4pf_isr
	.db 0
	ljmp _ep6pf_isr
	.db 0
	ljmp _ep8pf_isr
	.db 0
	ljmp _ep2ef_isr
	.db 0
	ljmp _ep4ef_isr
	.db 0
	ljmp _ep6ef_isr
	.db 0
	ljmp _ep8ef_isr
	.db 0
	ljmp _ep2ff_isr
	.db 0
	ljmp _ep4ff_isr
	.db 0
	ljmp _ep6ff_isr
	.db 0
	ljmp _ep8ff_isr
	.db 0
	ljmp _gpifdone_isr
	.db 0
	ljmp _gpifwf_isr
	.db 0
