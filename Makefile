CC = gcc

CFLAGS = -std=c99 -pedantic -O2 -Wall -Wextra
LDFLAGS = -static

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))
CORE = $(filter-out build/ppc.o,$(OBJ))

ppc: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

build/ctests: tests/ctests.c $(CORE) | build
	$(CC) $(CFLAGS) -Isrc $(LDFLAGS) -o $@ $< $(CORE)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c -o $@ $<

build:
	mkdir -p build

test: build/ctests
	./build/ctests

clean:
	rm -rf build ppc

.PHONY: test clean
