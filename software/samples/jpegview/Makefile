ifeq ($(OS),Windows_NT)
	ifeq ($(MSYSTEM), MINGW32)
		UNAME := MSYS
	else
		UNAME := Windows
	endif
else
	UNAME := $(shell uname)
endif

TARGET = jpegview.elf

default: $(TARGET)

# Directories

src_dir = .
corelib_dir = ../../SDK
nanojpeg_libs = ../../3rdparty/nanojpeg

# Rules

RISCV_OBJDUMP ?= $(RISCV_PREFIX)objdump

ifeq ($(UNAME), Windows)
RISCV_PREFIX ?= riscv32-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -Wl,--strip-all -lgcc -lm
else
RISCV_PREFIX ?= riscv64-unknown-elf-
RISCV_GCC ?= $(RISCV_PREFIX)g++
RISCV_GCC_OPTS ?= -mcmodel=medany -std=c++20 --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wall -Ofast -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -Wl,--strip-all -lgcc -lm
endif

incs  += -I$(src_dir) -I$(corelib_dir) -I$(nanojpeg_libs)/ $(addprefix -I$(src_dir)/, $(folders))
libs += $(wildcard $(corelib_dir)/*.S) $(wildcard $(corelib_dir)/*.c) $(wildcard $(nanojpeg_libs)/*.c)
objs  := 

$(TARGET):
	$(RISCV_GCC) $(incs) -o $(TARGET) $(wildcard $(src_dir)/*.cpp) $(libs) $(RISCV_GCC_OPTS)

clean:
ifeq ($(UNAME), Windows)
	del $(TARGET)
else
	rm -rf $(TARGET)
endif
