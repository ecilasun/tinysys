ifeq ($(OS),Windows_NT)
	ifeq ($(MSYSTEM), MINGW32)
		UNAME := MSYS
	else
		UNAME := Windows
	endif
else
	UNAME := $(shell uname)
endif

CROSS ?= riscv64-unknown-elf-

CC = $(CROSS)gcc
OBJDUMP ?= $(CROSS)objdump
SIZE = $(CROSS)size

#CFLAGS = -mcmodel=medany -std=gnu99 --param l1-cache-line-size=64 --param l1-cache-size=16 -Wall -Ofast -flto -march=rv32im_zicsr_zifencei -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -I.. -I../../../../SDK/

ifeq ($(UNAME), Windows)
CFLAGS = -mcmodel=medany -std=gnu99 --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -O2 -flto -march=rv32im -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -I.. -I../../../../SDK/
else
CFLAGS = -mcmodel=medany -std=gnu99 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -O2 -flto -march=rv32im_zicsr_zifencei -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -I.. -I../../../../SDK/
endif

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
	../../../../SDK/vpu.c \
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
	$(NULL)

all: doom.elf doom.txt

doom.elf: $(addprefix ../,$(SOURCES_doom)) $(SOURCES_doom_arch)
	$(CC) $(CFLAGS) -o $@ $(addprefix ../,$(SOURCES_doom)) $(SOURCES_doom_arch) $(CLIBS)
	$(SIZE) $@

clean:
ifeq ($(UNAME), Windows)
	del *.elf *.o *.gen.h doom.txt
else
	rm -rf *.elf *.o *.gen.h doom.txt
endif

%.txt: %.elf
	$(OBJDUMP) -x -D -S $< >> $@

.PHONY: all clean upload
