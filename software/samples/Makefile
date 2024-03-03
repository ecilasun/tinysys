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

XLEN ?= 32

# Directories

src_dir = .
corelib_dir = ../SDK
memtest_libs = ../3rdparty/memtest
nanojpeg_libs = ../3rdparty/nanojpeg
lodepng_libs = ../3rdparty/lodepng

# Executable folders

folders = \
	adc \
	audio \
	dmatest \
	dump \
	gamepad \
	helloworld \
	joystick \
	jpegview \
	julia \
	led \
	mandelbrot \
	memtest \
	more \
	mouse \
	oxygen \
	raster \
	reflections \
	syscalltest \
	tinyrt \
	tinyrtty \
	usbinfo \
	vgmplayer \

# Rules

RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump

ifeq ($(UNAME), Windows)
RISCV_PREFIX ?= riscv32-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Warray-bounds=0 -Wstringop-overflow=0 -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
else
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -fPIC -lgcc -lm
endif

incs  += -I$(src_dir) -I$(corelib_dir) -I$(memtest_libs)/ -I$(nanojpeg_libs)/ -I$(lodepng_libs) $(addprefix -I$(src_dir)/, $(folders))
libs += $(wildcard $(corelib_dir)/*.S) $(wildcard $(corelib_dir)/*.c) $(wildcard $(memtest_libs)/*.c) $(wildcard $(nanojpeg_libs)/*.c) $(wildcard $(lodepng_libs)/*.c)
objs  :=

define compile_template
$(1).elf: $(wildcard $(src_dir)/$(1)/*) $(wildcard $(src_dir)/*)
	$$(RISCV_GCC) $$(incs) -o $$@ $(wildcard $(src_dir)/$(1)/*.cpp) $$(libs) $$(RISCV_GCC_OPTS)
$(1).txt: $(1).elf
	$$(RISCV_OBJDUMP) -x -D -S $$< >> $$@
endef

$(foreach folder,$(folders),$(eval $(call compile_template,$(folder))))

# Build

folders_riscv_bin  = $(addsuffix .elf,  $(folders))
folders_riscv_txt  = $(addsuffix .txt,  $(folders))

executables: $(folders_riscv_bin)
textfiles: $(folders_riscv_txt)

junk += $(folders_riscv_bin)

# Default

all: executables textfiles

# Clean

clean:
ifeq ($(UNAME), Windows)
	del $(objs) $(junk)
else
	rm -rf $(objs) $(junk)
endif
