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
