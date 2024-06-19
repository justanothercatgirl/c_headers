
CC := gcc
CFLAGS := -g -Wall -Wextra -Wpedantic -Werror -Wno-language-extension-token


tests: dynarray


dynarray: tests/dynarray.c tests/dynarray_struct.c
	$(CC) $(CFLAGS) -o $@ $< -DTT_CHAR
	./dynarray
	$(CC) $(CFLAGS) -o $@ $< -DTT_SHORT
	./dynarray
	$(CC) $(CFLAGS) -o $@ $< -DTT_INT
	./dynarray
	$(CC) $(CFLAGS) -o $@ $< -DTT_LONG
	./dynarray
	$(CC) $(CFLAGS) -o $@ tests/dynarray_struct.c
	./dynarray

	rm $@

