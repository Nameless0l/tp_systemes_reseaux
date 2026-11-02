CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -g -Iincludes
LDFLAGS = -Llib
LDLIBS = -lseg

SOURCES = $(wildcard src/*.c)
PROGRAMS = $(patsubst src/%.c,bin/%,$(SOURCES))

.PHONY: all clean

all: $(PROGRAMS)

bin/%: src/%.c $(wildcard includes/*.h) lib/libseg.a
	@mkdir -p bin
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -rf bin
