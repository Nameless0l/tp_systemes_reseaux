CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -g -Iincludes

# Toutes les classes du projet (tout src/*.c sauf le main)
CLASSES = $(filter-out src/main.c,$(wildcard src/*.c))
HEADERS = $(wildcard includes/*.h)

# Executable par fichier tests/test_*.c
TEST_SRCS = $(wildcard tests/test_*.c)
TESTS = $(patsubst tests/%.c,bin/%.exe,$(TEST_SRCS))

.PHONY: all run test sanitize valgrind clean

all: bin/main.exe

bin/main.exe: src/main.c $(CLASSES) $(HEADERS)
	mkdir -p bin
	$(CC) $(CFLAGS) src/main.c $(CLASSES) -o $@

# Regle generique : bin/test_xxx.exe est construit depuis tests/test_xxx.c
bin/%.exe: tests/%.c $(wildcard tests/*.h) $(CLASSES) $(HEADERS)
	mkdir -p bin
	$(CC) $(CFLAGS) -Itests $< $(CLASSES) -o $@

run: bin/main.exe
	./bin/main.exe

test: $(TESTS)
	@for t in $(TESTS); do echo "=== $$t ==="; ./$$t || exit 1; done

# Verification des fuites memoire et des comportements indefinis
sanitize: $(CLASSES) $(HEADERS)
	mkdir -p bin
	@for src in $(TEST_SRCS); do \
		out=bin/sanitize_$$(basename $$src .c).exe; \
		echo "=== build $$out ==="; \
		$(CC) $(CFLAGS) -fsanitize=address,undefined -Itests $$src $(CLASSES) -o $$out || exit 1; \
		./$$out || exit 1; \
	done

valgrind: $(TESTS)
	@for t in $(TESTS); do \
		echo "=== valgrind $$t ==="; \
		valgrind --leak-check=full --error-exitcode=1 ./$$t || exit 1; \
	done

clean:
	rm -rf bin
