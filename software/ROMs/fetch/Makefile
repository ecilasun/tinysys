# Setup

XLEN ?= 32

ifeq ($(OS),Windows_NT)
	ifeq ($(MSYSTEM), MINGW32)
		UNAME := MSYS
	else ifeq ($(MSYSTEM), DARWIN)
		UNAME := DARWIN
	else
		UNAME := WINDOWS
	endif
else
	UNAME := $(shell uname)
endif

default: all

# Directories

src_dir = .
corelib_dir = ../SDK

# Executable folders

# TODO: Make sure FETCH always builds in non-compressed mode

folders = \
	rom

# Rules

#override CFLAGS += -Xlinker --defsym=__stack_size=0x800

ifeq ($(UNAME), WINDOWS)
RISCV_PREFIX ?= riscv32-unknown-elf-
RISCVTOOL ?= ..\..\riscvtool.exe
else
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCVTOOL ?= ../../riscvtool
endif
RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump

# No filesystem when defined
# FILESYSTEM ?= -DDISABLE_FILESYSTEM
FILESYSTEM ?= -DENABLE_FILESYSTEM -DLFS_NO_MALLOC -DLFS_NO_DEBUG
ROMWORDSIZE ?= 4
# 512 bit cache lines makes 64 bytes per line
# 256 cache lines makes total cache size 16 Kbytes
RISCV_GCC_OPTS ?= -mcmodel=medany -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -O0 -static -ffreestanding -nostartfiles -fomit-frame-pointer -ffunction-sections -fdata-sections -Wl,-gc-sections,--strip-debug

incs  += -I$(src_dir) -I$(corelib_dir) $(addprefix -I$(src_dir)/, $(folders))
objs  :=
libs  := 

define compile_ROM
$(1).elf: $(wildcard $(src_dir)/$(1)/*) $(wildcard $(src_dir)/*)
	$$(RISCV_GCC) -DBUILDING_ROM $$(FILESYSTEM) $$(incs) $$(RISCV_GCC_OPTS) -o $$@ $(wildcard $(src_dir)/$(1)/*.c) $(wildcard $(corelib_dir)/*.c) $$(libs) -Wl,-T$(1)/rom.lds

$(1).mem: $(wildcard $(src_dir)/$(1).elf)
	$$(RISCVTOOL) -makemem $(src_dir)/$(1).elf $$(ROMWORDSIZE) $(src_dir)/$(1).mem

$(1).txt: $(wildcard $(src_dir)/$(1).elf)
	$$(RISCV_OBJDUMP) $(src_dir)/$(1).elf -x -D -S >>  $(src_dir)/$(1).txt
endef

$(foreach folder,$(folders),$(eval $(call compile_ROM,$(folder))))

# Build

folders_riscv_bin  = $(addsuffix .elf,  $(folders))
folders_riscv_mem = $(addsuffix .mem, $(folders))
folders_riscv_disasm = $(addsuffix .txt, $(folders))

executables: $(folders_riscv_bin)
romimages: $(folders_riscv_mem)
disassembly: $(folders_riscv_disasm)

junk += $(folders_riscv_bin) $(folders_riscv_mem) $(folders_riscv_disasm)

# Default

all: executables romimages disassembly

# Clean

clean:
	del $(objs) $(junk)

# rm -rf $(objs) $(junk)