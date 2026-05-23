N64_INST = /n64_toolchain
PROG_NAME = vi_timing_test

# Select a timing preset at build time. e.g.:
#   make                     - NTSC_PROG (default)
#   make PRESET=MPAL_INT     - MPAL_INT (MPAL interlaced preset)
PRESET ?= NTSC_PROG

CFLAGS += -DPRESET_$(PRESET)

OUTPUT_NAME = $(PROG_NAME)_$(PRESET)

all: $(OUTPUT_NAME).z64

OBJS = main.o

include $(N64_INST)/include/n64.mk

$(OUTPUT_NAME).elf: $(OBJS)

clean:
	rm -f *.o *.elf *.z64

.PHONY: all clean
