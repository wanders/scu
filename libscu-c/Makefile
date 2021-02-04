CFLAGS:=-Wall -Wextra -Werror -I. -Wstrict-prototypes -Wmissing-prototypes

ifneq ($(SCU_HAVE_VALGRIND),)
CFLAGS+=-DSCU_HAVE_VALGRIND=1
ifneq ($(SCU_VALGRIND_CFLAGS),)
CFLAGS+=$(SCU_VALGRIND_CFLAGS)
endif
endif

.PHONY: clean

libscu-c.a: src/scu.o
	$(AR) rcs $@ $^

clean::
	rm -f src/scu.o libscu-c.a

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
