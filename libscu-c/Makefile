CFLAGS:=-Wall -Wextra -Werror -I. -Wstrict-prototypes -Wmissing-prototypes

ifneq ($(SCU_HAVE_VALGRIND),)
CFLAGS+=-DSCU_HAVE_VALGRIND=1
endif

ifneq ($(SCU_VALGRIND_LEAK_LEVEL),)
CFLAGS+=-DSCU_VALGRIND_LEAK_LEVEL=$(SCU_VALGRIND_LEAK_LEVEL)
endif

.PHONY: clean

libscu-c.a: src/scu.o
	$(AR) rcs $@ $^

clean::
	rm -f src/scu.o libscu-c.a

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
