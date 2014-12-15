
CFLAGS=-Wall

.PHONY: clean all

all: runner test_0.so test_1.so test_2.so

runner: runner.o
	$(CC) -o $@ runner.o -lcunit -ldl

test_%.so: test_%.o
	$(CC) -o $@ --shared $< -Wl,-T,testcase.lds

clean:
	rm -f runner test_*.so *.o runner.o *~

%.o: %.c
	$(CC) -c -o $@ $< -Wall -fPIC
