CC = gcc

CPPFLAGS = -I/usr/include/lua5.4
CFLAGS = -std=c99 -pedantic -O2 -Wall -Wextra
LDFLAGS = -static -L/usr/lib/lua5.4
LDLIBS = -llua -lm

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))

ppc: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

build/%.o: src/%.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

build:
	mkdir -p build

clean:
	rm -rf build ppc

.PHONY: clean

.PHONY: test

test: ppc
	@set -e; \
	for file in tests/*.lua; do \
		printf 'testing %s ...' "$$file"; \
		./ppc "$$file"; \
		echo OK; \
	done
