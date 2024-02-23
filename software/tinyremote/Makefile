CC = clang

# Release
CFLAGS=-L/usr/lib/x86_64-linux-gnu/ -lX11 -lstdc++ -lv4l2 -Wall -std=c++20 -Ofast -flto -fomit-frame-pointer -Iincludes
# Debug
# CFLAGS=-L/usr/lib/x86_64-linux-gnu/ -lX11 -lstdc++ -lv4l2 -Wall -std=c++20 -Og -g -Iincludes

SOURCES := tinyremote.cpp

all: tinyremote

#NOTE: Please use sudo apt install libx11-dev to grab x11 dev libs to compile

tinyremote: $(addprefix ./,$(SOURCES))
	$(CC) $(CFLAGS) -o $@ $(addprefix ./,$(SOURCES))

clean:
	rm -f tinyremote *.o

.PHONY: all clean