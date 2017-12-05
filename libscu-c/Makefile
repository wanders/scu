
CC=gcc

CFLAGS:=-Wall -Werror -I.

libscu-c.a: src/scu.o
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)
