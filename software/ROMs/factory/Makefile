# Setup

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
corelib_dir = ../../SDK
filesystemlib_dir = ../../3rdparty/fat32

# Executable folders

# TODO: Make sure FETCH always builds in non-compressed mode

folders = \
	rom

# Rules

#override CFLAGS += -Xlinker --defsym=__stack_size=0x800

RISCV_GCC ?= $(RISCV_PREFIX)gcc
RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump
RISCV_PREFIX ?= riscv32-unknown-elf-
ifeq ($(UNAME), WINDOWS)
RISCVTOOL ?= ..\..\build\release\riscvtool.exe
RISCV_PREFIX ?= riscv64-unknown-elf-
else
RISCVTOOL ?= ../../build/release/riscvtool
endif
# No filesystem when defined
# FILESYSTEM ?= -DDISABLE_FILESYSTEM
FILESYSTEM ?= -DENABLE_FILESYSTEM -DLFS_NO_MALLOC -DLFS_NO_DEBUG
# The MEM file will be using 16 byte (128bit) output width to match the ROM layout
# (cache controller reads x4 16byte chunks for each line)
ROMWORDSIZE ?= 16
# 512 bit cache lines makes 64 bytes per line
# 256 cache lines makes total cache size 16 Kbytes
ifeq ($(UNAME), WINDOWS)
RISCV_GCC_OPTS ?= -mcmodel=medany -std=gnu99 -march=rv32i -mabi=ilp32 --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Warray-bounds=0 -Wstringop-overflow=0 -Wno-format -Os -static -ffreestanding -nostartfiles -fomit-frame-pointer -ffunction-sections -fdata-sections -Wl,-gc-sections,--strip-debug
else
RISCV_GCC_OPTS ?= -mcmodel=medany -std=gnu99 -march=rv32i -mabi=ilp32 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Wno-format -Os -static -ffreestanding -nostartfiles -fomit-frame-pointer -ffunction-sections -fdata-sections -Wl,-gc-sections,--strip-debug
endif

incs  += -I$(src_dir) -I$(corelib_dir) -I$(filesystemlib_dir)/ $(addprefix -I$(src_dir)/, $(folders))
objs  :=
libs  := $(wildcard $(filesystemlib_dir)/*.c)

# NOTE: not linking to $(wildcard $(corelib_dir)/*.c) since we only need headers
define compile_ROM
$(1).elf: $(wildcard $(src_dir)/$(1)/*) $(wildcard $(src_dir)/*)
	$$(RISCV_GCC) -DBUILDING_ROM $$(FILESYSTEM) $$(incs) $$(RISCV_GCC_OPTS) -o $$@ $(wildcard $(src_dir)/$(1)/*.c) $$(libs) -Wl,-T$(1)/rom.lds
	$$(RISCVTOOL) -makerom $(src_dir)/$(1).elf 4 $(src_dir)/$(1).coe
	$$(RISCV_OBJDUMP) $(src_dir)/$(1).elf -x -D -S >> $(src_dir)/$(1).txt
endef

$(foreach folder,$(folders),$(eval $(call compile_ROM,$(folder))))

# Build

folders_riscv_elf  = $(addsuffix .elf,  $(folders))
folders_riscv_coe = $(addsuffix .coe, $(folders))
folders_riscv_disasm = $(addsuffix .txt, $(folders))

executables: $(folders_riscv_elf)
romimages: $(folders_riscv_mem)
overlayimage: $(folders_riscv_coe)
disassembly: $(folders_riscv_disasm)

junk += $(folders_riscv_elf) $(folders_riscv_mem) $(folders_riscv_coe) $(folders_riscv_disasm)

# Default

all: executables romimages overlayimage disassembly

# Clean

clean:
ifeq ($(UNAME), WINDOWS)
	del $(objs) $(junk)
else
	rm -rf $(objs) $(junk)
endif
