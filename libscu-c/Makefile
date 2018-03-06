CC=gcc
CFLAGS:=-I./src -Wall -Werror -I.

OBJECTS=$(patsubst %.c,%.o,$(wildcard src/*.c))

.PHONY: clean

libscu-c.a: $(OBJECTS)
	$(AR) rcs $@ $^

clean::
	rm -f $(OBJECTS) libscu-c.a

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
