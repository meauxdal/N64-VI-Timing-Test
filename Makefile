N64_INST = /n64_toolchain
PROG_NAME = vi_timing_test

# Select a timing preset at build time:
#   make                      — NTSC (default)
#   make PRESET=MPAL_MATH     — MPAL progressive, lidnariq calculated values
#   make PRESET=MPAL_OLD      — MPAL progressive, older libdragon/libultra values
#   make PRESET=MPAL_PREVIEW  — MPAL, libdragon preview (interlaced profile for both)
#   make PRESET=PAL_1996      — PAL progressive, SGI 1996 / libdragon values
#   make PRESET=PAL_1997      — PAL progressive, OS2.0H+ values
PRESET ?= NTSC
CFLAGS += -DPRESET_$(PRESET)

all: $(PROG_NAME).z64

OBJS = main.o

include $(N64_INST)/include/n64.mk

$(PROG_NAME).elf: $(OBJS)

clean:
	rm -f *.o *.elf *.z64

.PHONY: all clean
