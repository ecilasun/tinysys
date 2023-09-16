CROSS ?= riscv64-unknown-elf-

CC = $(CROSS)gcc
OBJDUMP ?= $(CROSS)objdump
SIZE = $(CROSS)size

#CFLAGS = -mcmodel=medany -std=gnu99 --param l1-cache-line-size=64 --param l1-cache-size=16 -Wall -Ofast -flto -march=rv32imc_zicsr_zifencei -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -I.. -I../../../../SDK/

CFLAGS = -mcmodel=medany -std=gnu99 --param l1-cache-line-size=64 --param l1-cache-size=16 -Wall -O2 -flto -march=rv32imc -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -I.. -I../../../../SDK/

CFLAGS += \
	-DNORMALUNIX \
	$(NULL)

CLIBS = -lgcc -lm

include ../sources.mk

# Filter out d_main, we provide our own simplified one
SOURCES_doom := $(filter-out d_main.c,$(SOURCES_doom))

SOURCES_doom_arch := \
	../../../../SDK/core.c \
	../../../../SDK/basesystem.c \
	../../../../SDK/apu.c \
	../../../../SDK/gpu.c \
	../../../../SDK/dma.c \
	../../../../SDK/usbhost.c \
	../../../../SDK/keyboard.c \
	mini-printf.c \
	d_main.c \
	i_main.c \
	i_net.c \
	i_sound.c \
	i_system.c \
	i_video.c \
	console.c  \
	$(NULL)

all: doom.elf doom.txt

doom.elf: $(addprefix ../,$(SOURCES_doom)) $(SOURCES_doom_arch)
	$(CC) $(CFLAGS) -o $@ $(addprefix ../,$(SOURCES_doom)) $(SOURCES_doom_arch) $(CLIBS)
	$(SIZE) $@

clean:
	del *.elf *.o *.gen.h doom.txt

#rm -f *.elf *.o *.gen.h doom.txt

%.txt: %.elf
	$(OBJDUMP) -x -D -S $< >> $@

.PHONY: all clean upload
