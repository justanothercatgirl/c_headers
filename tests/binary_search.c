#define CONTAINER_IMPLEMENTATION
#include "../include/container.h"

#define prints(el) \
	do {printf("search %i: %s\n", el, (array_binary_search(array, &el, __default_int_cmp)) ? "true" : "false");} while(0)

int main(void) {
	int data[] = {9, 5, 2, 7, 6, 10, 5, 9, 7}; // no 8 there
	int* array = array_new(int, sizeof(data)/sizeof(data[0]));
	memcpy(array, data, sizeof(data));
	array_qsort_integral(array);
	int minusten = -10;
	int two = 2;
	int eight = 8;
	int five = 5;
	int ten = 10;
	int sixtynine = 69;
	prints(minusten);
	prints(two);
	prints(five);
	prints(eight);
	prints(ten);
	prints(sixtynine);
	if (array_binary_search(array, &minusten, __default_int_cmp) == 1 ||
	    array_binary_search(array, &two, __default_int_cmp) == 0 	||
	    array_binary_search(array, &eight, __default_int_cmp) == 1 	|| 
	    array_binary_search(array, &five, __default_int_cmp) == 0 	||
	    array_binary_search(array, &ten, __default_int_cmp) == 0 	||
	    array_binary_search(array, &sixtynine, __default_int_cmp) == 1 ) {
		return EXIT_FAILURE;
	} 
	return EXIT_SUCCESS;
}
