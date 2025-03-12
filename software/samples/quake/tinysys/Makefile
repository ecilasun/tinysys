ifeq ($(OS),Windows_NT)
	ifeq ($(MSYSTEM), MINGW32)
		UNAME := MSYS
	else
		UNAME := Windows
	endif
else
	UNAME := $(shell uname)
endif

ifeq ($(UNAME), Windows)
CROSS ?= riscv32-unknown-elf-
else
CROSS ?= riscv64-unknown-elf-
endif

CC = $(CROSS)gcc
SIZE = $(CROSS)size

ifeq ($(UNAME), Windows)
CFLAGS = -mcmodel=medany --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wdouble-promotion -fno-common -Ofast -flto -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -Wl,--strip-all -I../include/ -I../winquake/ -I../../../SDK/ -I../
else
CFLAGS = -mcmodel=medany --param "min-pagesize=0" --param "l1-cache-line-size=64" --param "l1-cache-size=16" -Wdouble-promotion -fno-common -Ofast -flto -march=rv32im_zicsr_zifencei_zfinx -mabi=ilp32 -ffunction-sections -fdata-sections -Wl,-gc-sections -Wl,--strip-all -I../include/ -I../winquake/ -I../../../SDK/  -I../
endif

CFLAGS += \
	-DNORMALUNIX \
	$(NULL)

CLIBS = -lgcc -lm

RISCV_OBJDUMP ?= $(CROSS)objdump

SOURCES_quake_arch := \
	../../../SDK/core.c \
	../../../SDK/basesystem.c \
	../../../SDK/apu.c \
	../../../SDK/vpu.c \
	../../../SDK/uart.c \
	../../../SDK/mini-printf.c \
	../../../SDK/serialinringbuffer.c \
	../../../SDK/keyboard.c \
	../../../SDK/task.c \
	display.c \
	fio.c \
	main.c \
	cd_null.c \
	in_port.c \
	snd_null.c \
	sys_port.c \
	vid_port.c \
	../winquake/chase.c \
	../winquake/cmd.c \
	../winquake/common.c \
	../winquake/console.c \
	../winquake/crc.c \
	../winquake/cvar.c \
	../winquake/draw.c \
	../winquake/host.c \
	../winquake/host_cmd.c \
	../winquake/keys.c \
	../winquake/mathlib.c \
	../winquake/menu.c \
	../winquake/model.c \
	../winquake/nonintel.c \
	../winquake/screen.c \
	../winquake/sbar.c \
	../winquake/zone.c \
	../winquake/view.c \
	../winquake/wad.c \
	../winquake/world.c \
	../winquake/cl_demo.c \
	../winquake/cl_input.c \
	../winquake/cl_main.c \
	../winquake/cl_parse.c \
	../winquake/cl_tent.c \
	../winquake/d_edge.c \
	../winquake/d_fill.c \
	../winquake/d_init.c \
	../winquake/d_modech.c \
	../winquake/d_part.c \
	../winquake/d_polyse.c \
	../winquake/d_scan.c \
	../winquake/d_sky.c \
	../winquake/d_sprite.c \
	../winquake/d_surf.c \
	../winquake/d_vars.c \
	../winquake/d_zpoint.c \
	../winquake/net_loop.c \
	../winquake/net_main.c \
	../winquake/pr_cmds.c \
	../winquake/pr_edict.c \
	../winquake/pr_exec.c \
	../winquake/r_aclip.c \
	../winquake/r_alias.c \
	../winquake/r_bsp.c \
	../winquake/r_light.c \
	../winquake/r_draw.c \
	../winquake/r_efrag.c \
	../winquake/r_edge.c \
	../winquake/r_misc.c \
	../winquake/r_main.c \
	../winquake/r_sky.c \
	../winquake/r_sprite.c \
	../winquake/r_surf.c \
	../winquake/r_part.c \
	../winquake/r_vars.c \
	../winquake/sv_main.c \
	../winquake/sv_phys.c \
	../winquake/sv_move.c \
	../winquake/sv_user.c \
	../winquake/net_none.c \
	$(NULL)

all: quake.elf

quake.elf: $(addprefix ../,$(SOURCES_quake)) $(SOURCES_quake_arch)
	$(CC) $(CFLAGS) -o ../$@ $(addprefix ../,$(SOURCES_quake)) $(SOURCES_quake_arch) $(CLIBS)

dump: quake.elf
	$(RISCV_OBJDUMP) ../quake.elf -x -D -S >> ../quake.elf.txt

clean:
ifeq ($(UNAME), Windows)
	del *.elf *.o *.gen.h *.txt
else
	rm -rf *.elf *.o *.gen.h *.txt
endif

.PHONY: all clean
