
CC := gcc
CFLAGS := -g -Wall -Wextra -Wpedantic -Werror -Wno-language-extension-token


tests: container


container: tests/dynarray.c tests/dynarray_struct.c tests/obscure.c tests/binary_search.c tests/types.c
	$(CC) $(CFLAGS) -o $@ $< -DTT_CHAR
	./$@
	$(CC) $(CFLAGS) -o $@ $< -DTT_SHORT
	./$@
	$(CC) $(CFLAGS) -o $@ $< -DTT_INT
	./$@
	$(CC) $(CFLAGS) -o $@ $< -DTT_LONG
	./$@
	$(CC) $(CFLAGS) -o $@ tests/dynarray_struct.c
	./$@
	$(CC) $(CFLAGS) -o $@ tests/obscure.c
	./$@
	$(CC) $(CFLAGS) -o $@ tests/binary_search.c
	./$@
	$(CC) $(CFLAGS) -o $@ tests/types.c
	./$@
	$(CC) $(CFLAGS) -o $@ tests/linked_list.c
	./$@

	rm $@

