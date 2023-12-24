# Setup

XLEN ?= 32

ifeq ($(OS), Windows_NT)
UNAME := Windows
else
UNAME := $(shell uname)
endif

default: all

# Directories

src_dir = .
corelib_dir = ../../SDK
thirdparty_libs = ../../3rdparty

# Executable folders

# TODO: Make sure FETCH always builds in non-compressed mode

folders = \
	rom

# Rules

#override CFLAGS += -Xlinker --defsym=__stack_size=0x800

RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump
ifeq ($(UNAME), Linux)
RISCVTOOL ?= ../../build/release/riscvtool
else
RISCVTOOL ?= ..\..\build\release\riscvtool.exe
endif
# No filesystem when defined
# FILESYSTEM ?= -DDISABLE_FILESYSTEM
FILESYSTEM ?= -DENABLE_FILESYSTEM -DLFS_NO_MALLOC -DLFS_NO_DEBUG
# The MEM file will be using 16 byte (128bit) output width to match the ROM layout
# (cache controller reads x4 16byte chunks for each line)
ROMWORDSIZE ?= 16
# 512 bit cache lines makes 64 bytes per line
# 256 cache lines makes total cache size 16 Kbytes
ifeq ($(UNAME), Linux)
RISCV_GCC_OPTS ?= -mcmodel=medany -std=gnu99 -march=rv32imc_zicsr_zifencei -mabi=ilp32 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Wno-format -Os -static -ffreestanding -nostartfiles -fomit-frame-pointer -ffunction-sections -fdata-sections -Wl,-gc-sections,--strip-debug
else
RISCV_GCC_OPTS ?= -mcmodel=medany -std=gnu99 -march=rv32imc -mabi=ilp32 --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Wno-format -Os -static -ffreestanding -nostartfiles -fomit-frame-pointer -ffunction-sections -fdata-sections -Wl,-gc-sections,--strip-debug
endif

incs  += -I$(src_dir) -I$(corelib_dir) -I$(thirdparty_libs)/ $(addprefix -I$(src_dir)/, $(folders))
objs  :=
libs  := $(wildcard $(thirdparty_libs)/**/*.c)

define compile_ROM
$(1).elf: $(wildcard $(src_dir)/$(1)/*) $(wildcard $(src_dir)/*)
	$$(RISCV_GCC) -DBUILDING_ROM $$(FILESYSTEM) $$(incs) $$(RISCV_GCC_OPTS) -o $$@ $(wildcard $(src_dir)/$(1)/*.c) $(wildcard $(corelib_dir)/*.c) $$(libs) -Wl,-T$(1)/rom.lds

$(1).mem: $(wildcard $(src_dir)/$(1).elf)
	$$(RISCVTOOL) $(src_dir)/$(1).elf -makemem $$(ROMWORDSIZE) $(src_dir)/$(1).mem

$(1).bin: $(wildcard $(src_dir)/$(1).elf)
	$$(RISCVTOOL) $(src_dir)/$(1).elf -makebin 4 $(src_dir)/$(1).bin

$(1).txt: $(wildcard $(src_dir)/$(1).elf)
	$$(RISCV_OBJDUMP) $(src_dir)/$(1).elf -x -D -S >> $(src_dir)/$(1).txt
endef

$(foreach folder,$(folders),$(eval $(call compile_ROM,$(folder))))

# Build

folders_riscv_elf  = $(addsuffix .elf,  $(folders))
folders_riscv_mem = $(addsuffix .mem, $(folders))
folders_riscv_bin = $(addsuffix .bin, $(folders))
folders_riscv_disasm = $(addsuffix .txt, $(folders))

executables: $(folders_riscv_elf)
romimages: $(folders_riscv_mem)
overlayimage: $(folders_riscv_bin)
disassembly: $(folders_riscv_disasm)

junk += $(folders_riscv_elf) $(folders_riscv_mem) $(folders_riscv_bin) $(folders_riscv_disasm)

# Default

all: executables romimages overlayimage disassembly

# Clean

clean:
ifeq ($(UNAME), Linux)
	rm -rf $(objs) $(junk)
else
	del $(objs) $(junk)
endif
