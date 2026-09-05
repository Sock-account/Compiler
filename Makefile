CC     := gcc
CFLAGS := -std=c11 -Wall -Wextra -g

BINS := Compiler T_test Compiler_debug

.PHONY: all run test debug clean

all: Compiler T_test

Compiler_debug: main.c
	$(CC) $(CFLAGS) -fsanitize=address,undefined -o $@ $<

debug: Compiler_debug
	./Compiler_debug

Compiler: main.c
	$(CC) $(CFLAGS) -o $@ $<

T_test: test.c
	$(CC) $(CFLAGS) -o $@ $<

run: Compiler
	./Compiler

test: T_test
	./T_test

clean:
	rm -f $(BINS)
