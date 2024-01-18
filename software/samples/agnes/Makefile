# Setup

ifeq ($(OS),Windows_NT)
	ifeq ($(MSYSTEM), MINGW32)
		UNAME := MSYS
	else
		UNAME := Windows
	endif
else
	UNAME := $(shell uname)
endif

default: all

# Directories

src_dir = .
corelib_dir = ../../SDK

# Rules

OBJDUMP ?= riscv64-unknown-elf-
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)gcc
ifeq ($(UNAME), Windows)
RISCV_GCC_OPTS ?= -mcmodel=medany --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Ofast -march=rv32im -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
else
RISCV_GCC_OPTS ?= -mcmodel=medany --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Ofast -march=rv32im_zicsr_zifencei -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
endif

incs  += -I$(src_dir) -I$(corelib_dir)/ -I$(src_dir)/.
libs += $(wildcard $(corelib_dir)/*.c) 
objs  := agnes.elf

$(foreach folder,$(folders),$(eval $(call compile_template,$(folder))))

# Build

agnes.elf: $(wildcard $(src_dir)/*)
	$(RISCV_GCC) $(incs) -o $@ $(wildcard $(src_dir)/*.c) $(libs) $(RISCV_GCC_OPTS)

junk += $(folders_riscv_bin)

# Default

all: agnes.elf agnes.txt

%.txt: %.elf
	$(OBJDUMP)objdump -x -D -S $< >> $@

# Clean

clean:
ifeq ($(UNAME), Windows)
	del $(objs) $(junk)
else
	rm -rf $(objs) $(junk)
endif
