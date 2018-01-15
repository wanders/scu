TESTCASES:=file framework crash crash-at-shutdown

CFLAGS=-Wall -Werror -std=gnu11 -g

SCU_DIR=..
include $(SCU_DIR)/libscu-c/Makefile.scu

clean::
	rm -f temp.txt core valgrind.crash.log.core.*
