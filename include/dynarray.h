#ifndef JUSTANOTHERCATGIRL_HEADERS_DYNARRAY
#define JUSTANOTHERCATGIRL_HEADERS_DYNARRAY

//! A dynamic array implementation
//! As this is a header-only library, you have to define `DYNARRAY_IMPLEMENTATION` macro the first time you include this
//! header. This implementation uses the idea of storing metadata to the left of the array data, so that the user can
//! think of it (and use it) as normal array. Most of the API functions are defined as macros that either do some work
//! or call the implementation. They are not all-caps because they are more like functions.
//! As source is the best documentation, you should check yourself what is macro and what is not. generally, everything
//! prefixed with `array_` is a macro. Some of the macros evaluate their arguments more than once (the multiline ones), 
//! so as a rule of thumb, you should only ever pass evaluated identifiers or expressions with no side-effects (like `ptr+2`)
//! to all of the macros in this library.

#include "utility.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// size of the array header. should not be used directly, unless you know what you are doing
#define DYNARRAY_HEADER_SIZE sizeof(struct _dynarray_header)

/// allocate new array of <size> elements of type <type>.
#define array_new(type, size)	  _alloc_dynarray(sizeof(type), size)
/// get amount of elements in the array
#define array_size(array)	  (((struct _dynarray_header *)((byte *)(array)-DYNARRAY_HEADER_SIZE))->size)
/// get array capacity (it's maximum size unitl reallocation)
#define array_capacity(array)	  (((struct _dynarray_header *)((byte *)(array)-DYNARRAY_HEADER_SIZE))->capacity)
/// get element size of an array (same as sizeof(array[0]), except not constant-expression)
#define array_element_size(array) (((struct _dynarray_header *)((byte *)(array)-DYNARRAY_HEADER_SIZE))->memb_size)
/// free a dynamic array
#define array_free(array)	  free(array_header(array))
/// get dynamic array header. should not be used directly, unless you know what you are doing.
#define array_header(array)	  ((struct _dynarray_header *)(((byte *)array) - DYNARRAY_HEADER_SIZE))
/// allocate new array of <size> element of type <type> and initialize it from buffer
#define array_new_buffer_copy(type, buffer, size) _alloc_dynarray_buffer(sizeof(type), size, buffer, sizeof(type) * size)
/// add one new element to the end of the array. May invalidate previous pointer if size+1 > capacity.
#define array_push(array, element)									\
	do {												\
		struct _dynarray_header *_ahdr = array_header(array);					\
		if (_ahdr->size + 1 > _ahdr->capacity) {						\
			array = _memreserve_dynarray(array, 1L << upper_2_power_64(_ahdr->size + 1));	\
			_ahdr = array_header(array);							\
		}											\
		array[_ahdr->size] = (element);								\
		++_ahdr->size;										\
	} while (0)
/// inserts element at given index element is evaluated once.
/// Note: `element` HAS to be of same type as the array during initialization. The expression has to be of the same type.
/// That means calling `array_insert(arr, 0, 5)`, where array is of type `long`, WILL lead to errors. because typeof(0) is
/// int, and it will put it in an `int` variable and copy 8 bytes from there, which will be some random stack garbage. 
#define array_insert(array, element, idx)					\
	do {									\
		typeof(element) __tmp_el_var = (element);			\
		array = _insert_to_index_dynarray(array, &__tmp_el_var, idx);	\
	} while(0)
/// removes last element from the array
#define array_pop(array) --array_header(array)->size
/// reserve length elements so that subsequent (length - current_size) operations require no reallocation
#define array_reserve(array, length) 	if (typeof(array) __tmp; (__tmp = _memreserve_dynarray(array, length)) != NULL) array = __tmp
/// change size of an array. the capacity is set to lowest possible power of 2 that is greater than length
#define array_resize(array, length)	array = _force_resize_dynarray(array, length);
/// set capacity to minimum possible value
#define array_shrink(array)	array = _memshrink_array(array)
/// bound-checks and returns a pointer to that element. on error returns NULL 
#define array_at(array, idx)	(idx < array_size(array) ? (typeof(array))((byte*)array + idx*array_size(array)) : NULL)
/// sorts the array using compare_func for comparison
#define array_qsort(array, compare_func) qsort(array, array_size(array), array_element_size(array), compare_func)
/// sorts the array using pre-defined compariton functions for signed integers based on size (1, 2, 4, 8)
#define array_qsort_integral(array) array_qsort(array, __qsort_cmps[array_element_size(array)])
/// Placed here because is not a macro. compares 2 arrays and returns the same way as `strcmp`
int array_compare(const void *const a1, const void *const a2, qsort_cmp_t comp);


/// for internal use
enum _dynarray_header_idx {
	_dah_idx_member_size	= 0,
	_dah_idx_capacity	= 1,
	_dah_idx_size		= 2,
};
/// basically for structurization
struct _dynarray_header {
	size_t memb_size;
	size_t capacity;
	size_t size;
};

void *_alloc_dynarray(size_t el_size, size_t len);
void *_alloc_dynarray_buffer(size_t el_size, size_t alen, void *buffer, size_t blen);
/// Forcibly change size AND capacity of the array to new value
/// ignores the power-of-2-capacity rule
void *_force_resize_dynarray(void *dynarray, size_t new_size);
void *_memreserve_dynarray(void *dynarray, size_t reserved);
void *_memshrink_array(void *dynarray);
void *_insert_to_index_dynarray(void *const dynarray, const void *const element, size_t index);


#ifdef DYNARRAY_IMPLEMENTATION

void *_alloc_dynarray(size_t el_size, size_t len)
{
	byte *data = (byte *)malloc(el_size * len + DYNARRAY_HEADER_SIZE);
	if (data == NULL) return NULL;
	struct _dynarray_header *header = (struct _dynarray_header *)data;
	header->size = len;
	header->capacity = len;
	header->memb_size = el_size;
	return data + DYNARRAY_HEADER_SIZE;
}

/// assumes that element size in the buffer is the same as element size in the array
void *_alloc_dynarray_buffer(size_t el_size, size_t alen, void *buffer, size_t blen) { 
	void *array = _alloc_dynarray(el_size, alen);
	memcpy(array, buffer, el_size * blen);
	return array;
}

void *_memreserve_dynarray(void * const dynarray, size_t reserved)
{
	struct _dynarray_header *hdr = array_header(dynarray);
	if (hdr->capacity >= reserved) return dynarray;
	byte *data = (byte *)hdr;
	data = (byte *)realloc(data, reserved * hdr->memb_size + DYNARRAY_HEADER_SIZE);
	if (data == NULL) return NULL;
	hdr = (struct _dynarray_header *)data;
	hdr->capacity = reserved;
	return data + DYNARRAY_HEADER_SIZE;
}

void *_memshrink_array(void * const dynarray)
{
	struct _dynarray_header *hdr = array_header(dynarray);
	byte *data = (byte *)hdr;
	data = (byte*) realloc(data, hdr->size * hdr->memb_size + DYNARRAY_HEADER_SIZE);
	if (data == NULL) return NULL;
	hdr = (struct _dynarray_header *) data;
	hdr->capacity = hdr->size;
	return data + DYNARRAY_HEADER_SIZE;
}


void *_force_resize_dynarray(void * const dynarray, size_t new_size)
{
	struct _dynarray_header *hdr = array_header(dynarray);
	if (new_size < hdr->size) {
		hdr->size = new_size;
		hdr->capacity = 1 << upper_2_power_64(new_size);
		byte *arr = (byte*) realloc(hdr, hdr->memb_size * hdr->capacity + DYNARRAY_HEADER_SIZE);
		if (arr == NULL) return NULL;
		return arr + DYNARRAY_HEADER_SIZE;

	} else {
		void* arr;
		if ((arr = _memreserve_dynarray(dynarray, 1 << upper_2_power_64(new_size))) == NULL) return NULL;
		hdr = array_header(arr);
		hdr->size = new_size;
		return arr;
	}
}

void *_insert_to_index_dynarray(void *const dynarray, const void *const element, size_t index) {
	struct _dynarray_header* hdr = array_header(dynarray);
	byte* array = (byte*) _memreserve_dynarray(dynarray, 1 << upper_2_power_64(hdr->size + 1));
	hdr = array_header(array);
	++hdr->size;
	memmove(array + hdr->memb_size * (index+1), array + hdr->memb_size * (index), hdr->memb_size * (hdr->size - 1 - index));
	memcpy(array + hdr->memb_size * index, element, hdr->memb_size);
	return array;
}

int array_compare(const void *const a1, const void *const a2, qsort_cmp_t comp) {
	struct _dynarray_header *hdr = array_header(a1);
	struct _dynarray_header *hdr2 = array_header(a2);
	assert(hdr->memb_size == hdr2->memb_size && "The arrays being compared do not have same element size");
	const size_t s1 = hdr->size, s2 = hdr->size;
	const size_t elsize = hdr->memb_size;
	size_t i;
	for (i = 0; i < s1 && i < s2; ++i) {
		int result = comp((byte*) a1 + elsize*i, (byte*) a2 + elsize*i);
		if (result) return result;
	}
	if (s1 == s2) return 0;
	if (i == s1) return -1;
	if (i == s2) return 1;
#define NDEBUG
	assert(0);
#undef 	NDEBUG
}

#endif // DYNARRAY_IMPLEMENTATION

#endif // JUSTANOTHERCATGIRL_HEADERS_DYNARRAY
