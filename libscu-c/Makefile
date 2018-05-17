CFLAGS:=-Wall -Werror -I. -Wstrict-prototypes -Wmissing-prototypes

.PHONY: clean

libscu-c.a: src/scu.o
	$(AR) rcs $@ $^

clean::
	rm -f src/scu.o libscu-c.a

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
