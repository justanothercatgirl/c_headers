#define CONTAINER_IMPLEMENTATION
#define UTILTY_EXPOSE_HELPERS
#include "../include/container.h"
#include <stdio.h>
#include <stdlib.h>

/// To test for different types
#if defined(TT_LONG)
	#define TEST_TYPE long
#elif defined(TT_INT)
	#define TEST_TYPE int
#elif defined(TT_SHORT)
	#define TEST_TYPE short
#elif defined(TT_CHAR)
	#define TEST_TYPE char
#endif 

void print_arr(TEST_TYPE *a, int line, const char* message) 
{
	printf("%20s@%i: %-16s: ", __FILE__, line, message);
	printf("%p: {", (void*)a);
	for (size_t i = 0UL; i < array_size(a); ++i) { 
#if defined(TT_CHAR)
		printf("%hhi, ", a[i]);
#elif defined(TT_SHORT)
		printf("%hi, ", a[i]);
#elif defined(TT_INT)
		printf("%i, ", a[i]);
#elif defined(TT_LONG)
		printf("%li, ", a[i]);
#else
#pragma error "PROVIDE A VALID SIGNED INTEGRAL TYPE"
#endif
	}
	printf("} size: %ld, cap: %ld, element_size: %ld\n", 
		array_size(a), array_capacity(a), array_element_size(a)
	);
}

int main(void)
{
	TEST_TYPE *arr = array_new(TEST_TYPE, 10);	// size: 10, cap: 10
	for (size_t i = 0; i < array_size(arr); ++i) { arr[i] = 127 - 2 * i; }
	print_arr(arr, __LINE__, "new array");	
	array_resize(arr, 21);				// size: 21, cap: 32
	print_arr(arr, __LINE__, "resize more");
	array_resize(arr, 3);				// size: 3, cap: 4
	print_arr(arr, __LINE__, "resize less");
	array_push(arr, 42);				// size: 4, cap: 4
	array_push(arr, 69);				// size: 5, cap: 8
	print_arr(arr, __LINE__, "pushes");
	array_shrink(arr);				// size: 5: cap: 5
	print_arr(arr, __LINE__, "shrinking");
	array_qsort_integral(arr);			// size: 5, cap: 5
	/* qsort(arr, array_size(arr), array_element_size(arr), __default_int_cmp); */
	print_arr(arr, __LINE__, "qsort");
	TEST_TYPE __inserted = 20;
	array_insert(arr, __inserted, 1);			// size: 6, cap: 8
	print_arr(arr, __LINE__, "1 insert");

	for (size_t i = 0; i <= array_size(arr); i += 2) {
		TEST_TYPE val = 0; // This is crucial. see comment above array_insert declaration.
		array_insert(arr, val, i);
	}						// size: 13, cap: 16
	print_arr(arr, __LINE__, "multiple inserts");

	TEST_TYPE test_arr_data[] =  {0, 42, 0, 20, 0, 69, 0, 123, 0, 125, 0, 127, 0, };
	TEST_TYPE* test_arr = array_new_buffer_copy(TEST_TYPE, test_arr_data, sizeof(test_arr_data) / sizeof(test_arr_data[0]));
	if (array_compare(test_arr, arr, __qsort_cmps[sizeof(TEST_TYPE)])) {
		puts("\x1b[31mTest for \"" stringify(TEST_TYPE)"\" failed\x1b[0m");
		goto exit_fail;
	} else {
		puts("\x1b[32mTest for \"" stringify(TEST_TYPE)"\" passed\x1b[0m");
	}
	TEST_TYPE* elem;
	if ((elem = array_at(arr, 1)) != NULL) {
		if (*elem == 42) {
			puts("array_at(1) = 42");
		}
		else {
			printf("%s@%i: array_at failed\n", __FILE__, __LINE__);
			goto exit_fail;
		}
	} else {
		printf("%s@%i: array_at failed\n", __FILE__, __LINE__);
		goto exit_fail;

	}
	if (( elem = array_at(arr, 69420)) != NULL) {
		printf("%s@%i: array_at failed\n", __FILE__, __LINE__);		
		goto exit_fail;
	}

//exit:
	array_free(arr);
	array_free(test_arr);
	exit(EXIT_SUCCESS);
exit_fail:
	array_free(arr);
	array_free(test_arr);
	exit(EXIT_FAILURE);
}
