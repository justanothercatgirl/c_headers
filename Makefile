
CC := gcc
CFLAGS := -g -Wall -Wextra -Wpedantic -Werror


tests: dynarray


dynarray: tests/dynarray.c
	$(CC) $(CFLAGS) -o $@ $^ -DTT_CHAR
	./dynarray
	$(CC) $(CFLAGS) -o $@ $^ -DTT_SHORT
	./dynarray
	$(CC) $(CFLAGS) -o $@ $^ -DTT_INT
	./dynarray
	$(CC) $(CFLAGS) -o $@ $^ -DTT_LONG
	./dynarray

	rm dynarray

