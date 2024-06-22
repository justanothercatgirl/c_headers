#ifndef JUSTANOTHERCATGIRL_HEADERS_CONTAINER
#define JUSTANOTHERCATGIRL_HEADERS_CONTAINER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* ----------------------------------------------------------------- */
/* -----------------UTILITY HEADER---------------------------------- */
/* ----------------------------------------------------------------- */
#define stringify(val) _stringify_helper(val)
#define _stringify_helper(val) #val
#define string_concat_separator(first, ...) stringify(first) ";" string_concat_separator(__VA_ARGS__)

#define struct_member_size(type, member) sizeof(( (type*)0 )->member)
typedef unsigned char byte;

#ifdef __GNUC__
	#define upper_2_power_32(number) (32 - __builtin_clz(number))
	#define upper_2_power_64(number) (64 - __builtin_clzl(number))
#else
	#include <intrin.h>
	#include <stdint.h>
	#define upper_2_power_32(number) __bit_scan_32(number)
	#define upper_2_power_64(number) __bit_scan_64(number)
	unsigned long __bit_scan_32(int32_t number);
	unsigned long __bit_scan_64(int64_t number);
#endif

#if defined(__clang__) || defined(__GNUC__)
#define FALLTHROUGH __attribute__((fallthrough))
#elif defined(_MSC_VER)
#define FALLTHROUGH __fallthrough()
#else
#define FALLTHROUGH ((void)0)
#endif

#define TODO (void)(*(volatile char*)0)

typedef int(*qsort_cmp_t)(const void*, const void*);
#define get_qsort_cmp(type) __qsort_cmps[sizeof(type)]
extern const qsort_cmp_t __qsort_cmps[64];

#ifdef CONTAINER_EXPOSE_HELPERS
	#define _CONTAINER_STATIC 
	int __default_char_cmp(const void* a, const void* b);
	int __default_short_cmp(const void* a, const void* b);
	int __default_int_cmp(const void* a, const void* b);
	int __default_long_cmp(const void* a, const void* b);
	int __default_long_long_cmp(const void* a, const void* b);
#else 
	#define _CONTAINER_STATIC static
#endif // CONTAINER_EXPOSE_HELPERS


/* ----------------------------------------------------------------- */
/* -------------------------ARRAY HEADER---------------------------- */
/* ----------------------------------------------------------------- */
/* A dynamic array implementation 
 * As this is a header-only library, you have to define `CONTAINER_IMPLEMENTATION` macro the first time you include this
 * header. This implementation uses the idea of storing metadata to the left of the array data, so that the user can
 * think of it (and use it) as normal array. Most of the API functions are defined as macros that either do some work
 * or call the implementation. They are not all-caps because they are more like functions.
 * As source is the best documentation, you should check yourself what is macro and what is not. generally, everything
 * prefixed with `array_` is a macro. Some of the macros evaluate their arguments more than once (the multiline ones), 
 * so as a rule of thumb, you should only ever pass evaluated identifiers or expressions with no side-effects (like `ptr+2`)
 * to all of the macros in this library.
 */

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
#define array_insert(array, element, idx)								\
	do {												\
		typeof(element) __tmp_el_var = (element);						\
		array = _insert_to_index_dynarray(array, &__tmp_el_var, sizeof(__tmp_el_var), idx);	\
	} while(0)
/// removes last element from the array
#define array_pop(array) --array_header(array)->size
/// reserve length elements so that subsequent (length - current_size) pushes require no reallocation
#define array_reserve(array, length) 	if (typeof(array) __tmp; (__tmp = _memreserve_dynarray(array, length)) != NULL) array = __tmp
/// change size of an array. the capacity is set to lowest power of 2 that is greater than length
#define array_resize(array, length)	array = _force_resize_dynarray(array, length);
/// set capacity to minimum possible value
#define array_shrink(array)	array = _memshrink_array(array)
/// bound-checks and returns a pointer to that element. on error returns NULL 
#define array_at(array, idx)	(idx < array_size(array) ? (void*)((byte*)array + idx*array_element_size(array)) : NULL)
/// sorts the array using compare_func for comparison
#define array_qsort(array, compare_func) qsort(array, array_size(array), array_element_size(array), compare_func)
/// sorts the array using pre-defined compariton functions for signed integers based on size (1, 2, 4, 8)
#define array_qsort_integral(array) array_qsort(array, __qsort_cmps[array_element_size(array)])
/// Placed here because is not a macro. compares 2 arrays and returns the same way as `strcmp`
int array_compare(const void *const a1, const void *const a2, qsort_cmp_t comp);
/// Exact copy of the array. you have to free the pointer as well.
void* array_copy(void* old);
char array_binary_search(void* array, void* element, qsort_cmp_t cmp);

/* ----------------------------------------------------------------- */
/* ----------------------LINKED LIST HEADER------------------------- */
/* ----------------------------------------------------------------- */
// TODO!

/* ------------------------------------------------------------------------- */
/* ------From now on, the rest of the header is implementation details------ */
/* -------------------The API and documentation end here-------------------- */
/* ------------------------------------------------------------------------- */

// for internal use
enum _dynarray_header_idx {
	_dah_idx_member_size	= 0,
	_dah_idx_capacity	= 1,
	_dah_idx_size		= 2,
};
// for structurization
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
void *_insert_to_index_dynarray(void *const dynarray, const void *const element, size_t el_size, size_t index);


// uncomment in dev mode so that LSP highlights the code
/*#define CONTAINER_IMPLEMENTATION*/
#ifdef CONTAINER_IMPLEMENTATION

/* ----------------------------------------------------------------- */
/* ---------------------UTILITY IMPLEMENTATION---------------------- */
/* ----------------------------------------------------------------- */

#ifndef __GNUC__
unsigned long __bit_scan_32(int32_t number) {
	#ifdef _MSC_VER
		unsigned long index;
		if (_BitScanReverse(&index, number)) {
			return index + 1;
		} else {
			return 1;
		}
	#else // _MSC_VER
		unsigned long count;
		for (count = 0; number; number >>= 1) { ++count; }
		return count;
	#endif // _MSC_VER
}
unsigned long __bit_scan_64(int64_t number) {
	#ifdef _MSC_VER
		unsigned long index;
		if (_BitScanReverse64(&index, number)) {
			return index + 1;
		} else {
			return 1;
		}
	#else // _MSC_VER
		unsigned long count;
		for (count = 0; number; number >>= 1) { ++count; }
		return count;
	#endif // _MSC_VER
}
#endif // __GNUC__


_CONTAINER_STATIC int __default_char_cmp(const void* a, const void* b) {
	char x = *(char*)a, y = *(char*)b;
	if (x < y) return -1;
	if (x > y) return 1;
	return 0;
}
_CONTAINER_STATIC int __default_short_cmp(const void* a, const void* b) {
	short x = *(short*)a, y = *(short*)b;
	if (x < y) return -1;
	if (x > y) return 1;
	return 0;
}
_CONTAINER_STATIC int __default_int_cmp(const void* a, const void* b) {
	int x = *(int*)a, y = *(int*)b;
	if (x < y) return -1;
	if (x > y) return 1;
	return 0;
}
_CONTAINER_STATIC  int __default_long_long_cmp(const void* a, const void* b) {
	long long x = *(long long*)a, y = *(long long*)b;
	if (x < y) return -1;
	if (x > y) return 1;
	return 0;
}
_CONTAINER_STATIC int __default_long_cmp(const void* a, const void* b) {
	long x = *(long*)a, y = *(long*)b;
	if (x < y) return -1;
	if (x > y) return 1;
	return 0;
}

#ifdef __GNUC__
// cope
#pragma GCC diagnostic ignored "-Woverride-init" // Is is meant to override it on different platforms
const qsort_cmp_t __qsort_cmps[64] = {
	[sizeof(char)] = __default_char_cmp,
	[sizeof(short)] = __default_short_cmp,
	[sizeof(int)] = __default_int_cmp,
	[sizeof(long)] = __default_long_cmp,
	[sizeof(long long)] = __default_long_long_cmp,
	[63] = 0,
};
#pragma GCC diagnostic warning "-Woverride-init"
#else 	// not __GNUC__
const qsort_cmp_t __qsort_cmps[64] = {
	0, 
	__default_char_cmp, __default_short_cmp, 0, __default_int_cmp, 
	0, 0, 0, __default_long_long_cmp,
};
#endif 	// __GNUC__


/* ----------------------------------------------------------------- */
/* ----------------------ARRAY IMPLEMENTATION----------------------- */
/* ----------------------------------------------------------------- */

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

void *_insert_to_index_dynarray(void *const dynarray, const void *const element, size_t el_size, size_t index) {
	struct _dynarray_header* hdr = array_header(dynarray);
	byte* array = (byte*) _memreserve_dynarray(dynarray, 1 << upper_2_power_64(hdr->size + 1));
	hdr = array_header(array);
	++hdr->size;
	memmove(array + hdr->memb_size * (index+1), array + hdr->memb_size * (index), hdr->memb_size * (hdr->size - 1 - index));
	if (el_size != hdr->memb_size) { /* TODO: warning? */ }
	memcpy(array + hdr->memb_size * index, element, el_size);
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

/*void* array_copy(void* old) {*/
/*	//TODO!*/
/*}*/

char array_binary_search(void* array, void* element, qsort_cmp_t cmp) {
	ssize_t index1 = -1,
	        index2 = array_size(array);
	if (index2 == 0) return 0;
	while (1) {
		ssize_t index3 = (index1+index2)/2;
		if (index3 == index2 || index3 == index1 || index1 == index2) return 0;
		int result = cmp(element, (byte*)array + array_element_size(array) * index3);
		if (result == 0) return 1;
		if (result < 0) index2 = index3;
		else if (result > 0) index1 = index3;
	}
}


#endif // CONTAINER_IMPLEMENTATION

#endif // JUSTANOTHERCATGIRL_HEADERS_CONTAINER
