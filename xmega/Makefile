
ISIZE = 256
CSIZE = 10240
XLOC  = $(CSIZE)
XCSIZE= 16384
XSIZE = $(shell echo $(XCSIZE)-$(XLOC) | bc)
#XLOC  = 0xe000
#XSIZE = 0x0200

CC := sdcc
AS := sdas8051
SDCC_FLAGS = \
	-Wa,-p --less-pedantic -mmcs51 \
	--code-size $(CSIZE) \
	--xram-loc $(XLOC) \
	--xram-size $(XSIZE) \
	--iram-size $(ISIZE)

# Disable pagination in .lst file
SDAS_FLAGS = -plosgff

OBJS := fw.rel xmega-pdi.rel

OSUFFIXES = .rel .rst .lst .sym
LSUFFIXES = .map .lk .mem .ihx

ifeq ($(AUTOVECTOR),1)
	OBJS += autovector.rel
	SDCC_FLAGS += -Wl"-b INT2JT = 0x3f00" -Wl"-b INT4JT = 0x3f80"
endif

.PHONY: all clean

all: load-fw.ihx

load-fw.ihx: $(OBJS)
	$(CC) $(SDCC_FLAGS) $(CFLAGS) $(OBJS) -o $@

%.rel: %.c $(wildcard *.h)
	$(CC) $(SDCC_FLAGS) $(CFLAGS) -c -o $@ $<

%.rel: %.s
	$(AS) $(SDAS_FLAGS) $(ASFLAGS) $@ $<

clean:
	$(RM) $(OBJS) load-fw.ihx
