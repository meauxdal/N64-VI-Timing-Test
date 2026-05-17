N64_INST = /n64_toolchain
PROG_NAME = vi_timing_test

# Select a timing preset at build time:
#   make                      — NTSC (default)
#   make PRESET=MPAL_MATH
#   make PRESET=MPAL_OLD
#   make PRESET=MPAL_PREVIEW
#   make PRESET=PAL_1996
#   make PRESET=PAL_1997
PRESET ?= NTSC

CFLAGS += -DPRESET_$(PRESET)

OUTPUT_NAME = $(PROG_NAME)_$(PRESET)

all: $(OUTPUT_NAME).z64

OBJS = main.o

include $(N64_INST)/include/n64.mk

$(OUTPUT_NAME).elf: $(OBJS)

clean:
	rm -f *.o *.elf *.z64

.PHONY: all clean
