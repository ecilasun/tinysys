CC = clang

CFLAGS=-L/usr/lib/x86_64-linux-gnu/ -lX11 -lstdc++ -Wall -std=c++20 -Ofast -flto -fomit-frame-pointer -Iincludes

SOURCES := keyserver.cpp

all: keyserver

#NOTE: Please use sudo apt install libx11-dev to grab x11 dev libs to compile

keyserver: $(addprefix ./,$(SOURCES))
	$(CC) $(CFLAGS) -o $@ $(addprefix ./,$(SOURCES))

clean:
	rm -f keyserver *.o

.PHONY: all clean