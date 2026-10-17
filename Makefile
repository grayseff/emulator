CC = cproc

CPPFLAGS = -I/usr/include/lua5.4
LDFLAGS = -L/usr/lib/lua5.4
LDLIBS = -llua

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

ppc: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(OBJ) ppc

.PHONY: clean

.PHONY: test

test: ppc
	@set -e; \
	for file in tests/*.lua; do \
		printf 'testing %s ...' "$$file"; \
		./ppc "$$file"; \
		echo OK; \
	done
