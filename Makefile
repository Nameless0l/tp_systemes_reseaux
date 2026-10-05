CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -g

SOURCES = $(wildcard src/*.c)
PROGRAMS = $(patsubst src/%.c,bin/%,$(SOURCES))

.PHONY: all clean

all: $(PROGRAMS)

bin/%: src/%.c
	@mkdir -p bin
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf bin
