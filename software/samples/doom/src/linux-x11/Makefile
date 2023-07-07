CC=gcc
CFLAGS=-g -m32 -Wall -funsigned-char
LDFLAGS=-m32 -L/usr/X11R6/lib
LIBS=-lXext -lX11 -lm

CFLAGS+=\
	-DNORMALUNIX \
	-DLINUX \
	-DDEBUG \
	-DRANGECHECK \
	-DSNDSERV \
	$(NULL)


include ../sources.mk

SOURCES_doom_arch = \
	i_main.c \
	i_net.c \
	i_sound.c \
	i_system.c \
	i_video.c \
	$(NULL)
	
OBJS += $(addprefix objs/, \
	$(SOURCES_doom:.c=.o) \
	$(SOURCES_doom_arch:.c=.o) \
)


all: doom-linux-x11

doom-linux-x11: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f objs/*.o doom-linux-x11

objs/%.o: %.c | objs
	$(CC) $(CFLAGS) -I.. -c -o $@ $<

objs/%.o: ../%.c | objs
	$(CC) $(CFLAGS) -c -o $@ $<

objs:
	mkdir objs

.PHONY: all clean
