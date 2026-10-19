CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -g -Iincludes $(shell pkg-config fuse --cflags)
LDLIBS = $(shell pkg-config fuse --libs)

SOURCES = $(wildcard src/*.c)
PROGRAMS = $(patsubst src/%.c,bin/%,$(SOURCES))

.PHONY: all clean

all: $(PROGRAMS)

bin/%: src/%.c $(wildcard includes/*.h)
	@mkdir -p bin
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -rf bin
