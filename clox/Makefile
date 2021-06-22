# Makefile

PROG = clox

CC = gcc
CFLAGS = -Wall -pedantic -ggdb -std=gnu11

OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

$(PROG) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f $(PROG)

.PHONY: clean
