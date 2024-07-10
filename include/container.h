#ifndef JUSTANOTHERCATGIRL_HEADERS_CONTAINER
#define JUSTANOTHERCATGIRL_HEADERS_CONTAINER

#include <stdint.h>
#include <stdarg.h>
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

/// Find closest powers of 2
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

/// define FALLTHROUGH macro
#if defined(__clang__) || defined(__GNUC__)
#define FALLTHROUGH __attribute__((fallthrough))
#elif defined(_MSC_VER)
#define FALLTHROUGH __fallthrough()
#else
#define FALLTHROUGH ((void)0)
#endif

/// define TODO macro
#define TODO (void)(*(volatile char*)0)
/// typedefs for array qsort
typedef int(*qsort_cmp_t)(const void*, const void*);
#define get_qsort_cmp(type) __qsort_cmps[sizeof(type)]
extern const qsort_cmp_t __qsort_cmps[64];

typedef void*(*memcpy_t)(void* restrict, const void*, size_t);

/// default qsort_cmp_t functions to be used with integral types.
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

/// size of the array header. Should not be used directly, unless you know what you are doing
#define DYNARRAY_HEADER_SIZE sizeof(struct _dynarray_header)

/// allocate new array of <size> elements of type <type>.
#define array_new(type, size)	  _alloc_dynarray(sizeof(type), size)
/// get amount of elements in the array
#define array_size(array)	  (((struct _dynarray_header *)((byte *)(array)-DYNARRAY_HEADER_SIZE))->size)
/// get array capacity (it's maximum size until reallocation)
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
#define array_extend(array, buffer, len) do {array = _array_extend(array, buffer, len); } while(0)
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
#define array_reserve(array, length) 	do { array = _memreserve_dynarray(array, length); } while (0)
/// change size of an array. the capacity is set to lowest power of 2 that is greater than length
#define array_resize(array, length)	do { array = _force_resize_dynarray(array, length); } while(0)
/// set capacity to minimum possible value
#define array_shrink(array)	do { array = _memshrink_array(array); } while(0)
/// bound-checks and returns a pointer to that element. on error returns NULL 
#define array_at(array, idx)	(idx < array_size(array) ? (void*)((byte*)array + idx*array_element_size(array)) : NULL)
/// sorts the array using compare_func for comparison
#define array_qsort(array, compare_func) qsort(array, array_size(array), array_element_size(array), compare_func)
/// sorts the array using pre-defined comparison functions for signed integers based on size (1, 2, 4, 8)
#define array_qsort_integral(array) array_qsort(array, __qsort_cmps[array_element_size(array)])
/// Placed here because is not a macro. compares 2 arrays and returns the same way as `strcmp`
int array_compare(const void *const a1, const void *const a2, qsort_cmp_t comp);
/// Exact copy of the array. you have to free the pointer as well.
void* array_copy(void* old);
/// Search for element `element` in array `array` using comparator `cmp`. 
/// Returns bool. Assumes that the array is sorted.
char array_binary_search(void* array, void* element, qsort_cmp_t cmp);
struct linked_list array_to_ll(void* array);

/* ----------------------------------------------------------------- */
/* ----------------------LINKED LIST HEADER------------------------- */
/* ----------------------------------------------------------------- */

typedef void(*free_t)(void*);
struct __linked_list_meta {
	size_t element_size;
	size_t assumed_size;
	free_t free_function;
};
struct linked_list_node {
	void* data;
	struct linked_list_node *next;
};
/// holds meta about list and first element. you should not use it's members.
struct linked_list {
	struct __linked_list_meta meta;
	struct linked_list_node *first;
	struct linked_list_node *last;
};

/// creates an empty linked list. `memb_size` should usually be sizeof(element type), 
/// O(1)
struct linked_list ll_create(size_t memb_size);
/// Created linked list with elements as in buf. Copies the memory with `memcpy`. 
/// Assumes buffer size is >= buff_len * memb_size
/// O(buff_len)
struct linked_list ll_create_from_buffer(size_t memb_size, const void* buf, size_t buff_len);
/// iteratively frees a linked list
/// O(n)
void ll_free(struct linked_list* list);
/// searches an `element` in the list using `cmp`. Returns whether it found it or not.
/// O(n)
char ll_search(const struct linked_list* list, const void* element, qsort_cmp_t cmp);
/// searches for `element` in the list using `cmp`. Returns the node if it was found, else returns NULL
/// O(n)
struct linked_list_node* ll_find(const struct linked_list* list, const void* element, qsort_cmp_t cmp);
/// get a pointer to node at index i. DO NOT FREE THIS POINTER. Returns NULL if
/// list is not long enough.
/// O(i)
struct linked_list_node* ll_at(const struct linked_list* list, const size_t i);
/// truncate a list starting from index i. if i==0, frees the list and you have to initialize a new one.
/// O(n)
void ll_truncate(struct linked_list* list, const size_t i);
/// returns a size of a list as stored in metadata. 
/// Should be same as `ll_definite_size` unless user modifies nodes directly. Which you should not do.
/// O(1)
size_t ll_probably_size(const struct linked_list* list);
/// Counts elements in linked list, updating size in metadata if necessary.
/// if `ll_probably_size` is greater than `ll_definite_size`, there was a memory leak.
// if it is equal, everything is fine
/// else, the creator of the library is dumb.
/// O(n)
size_t ll_definite_size(struct linked_list *list);
/// creates a new node at the end of list and copies memory from element to new node.
/// The amount of memory copied is `memb_size` that was passed in `ll_create*` function.
/// If list->last->next == NULL, O(1) (which is guaranteed if you don't touch the nodes).
void ll_append(struct linked_list *list, const void* restrict element);
/// same as `ll_append`, except it does not perform memcpy and directly assigns pointer to node.
/// the only reqirement to this function is that `pointer` must be freeable with `free` function.
/// ONLY USE THIS IF YOU ARE 100% SURE THAT POINTER CAME UNMODIFIED FROM MALLOC, 
/// OTHERWISE THE `ll_free` FUNCTION MAY DOUBLE-FREE, LEAK MEMORY AND OTHER TERRIBLE THINGS.
/// Complexity same as `ll_append`, usually O(1)
void ll_append_mallocated_pointer(struct linked_list *list, void* pointer);
/// inserts an element to the front of the list.
/// O(1)
void ll_insert_front(struct linked_list *list, const void* restrict element);
/// This to `ll_insert_front` is same as `ll_append_mallocated_pointer` to `ll_append`.
/// Copies `pointer` into newly created node. Same security considerations 
/// as in `ll_append_mallocated_pointer`.
/// O(1)
void ll_insert_front_mallocated_pointer(struct linked_list *list, void* pointer);
/// inserts an element at index I if it exists. Returns 1 if insertion was successfull,
/// 0 if list is not long enough.
/// O(i)
char ll_insert_at(struct linked_list *list, const void* restrict element, size_t i);
/// copies a pointer directly into newly created node. see `ll_insert_at` 
/// and `ll_append_mallocated_pointer` for more details.
/// O(i)
char ll_insert_at_mallocated_pointer(struct linked_list *list, void* pointer, size_t i);
/// removes the element from the front, or, if the list is empty, does nothing 
/// O(1)
void ll_remove_front(struct linked_list* list);
/// removes the element from the back, or, if the list is empty, does nothing
/// O(n)
void ll_remove_back(struct linked_list* list);
/// Removes element at index i. if there is no such index, does nothing.
/// O(n)
char ll_remove_at(struct linked_list *list, size_t i);
/// removes first occurrence of element `elem` from list.
void ll_remove_elem(struct linked_list* list, const void* elem, qsort_cmp_t cmp);
/// Removes ALL occurrences of element "elem" in a list
/// WARNING: SOMETHING IS WRONG WITH THIS FUNCTION AND IT SHOULD NOT BE USED EVER
/// UNLESS I FIX IT
/// FOR NOW I WILL ADD TODO MARK THERE
/// O(n)
void ll_remove_all(struct linked_list* list, const void* elem, qsort_cmp_t cmp);
/// splits a linked list into two, the second one starting from element at index `i`.
/// List that you passed stays valid, only it's size changes. return value is the second list.
/// O(n)
struct linked_list ll_split_at(struct linked_list *list, size_t i);
/// merges 2 lists into one, with elements of second list appearing at the end of first list
/// invalidates second list, leaving only first one valid.
/// O(1)
void ll_merge(struct linked_list* fst, struct linked_list* snd);
/// splits linked list into N linked lists at indexes given in varargs.
/// varargs type: size_t
/// INDEXES MUST BE IN ASCENDING ORDER, OR ELSE ARE IGNORED.
/// Returns array (as defined in array implementation above).
/// O(n)
struct linked_list* ll_split_n(struct linked_list* list, size_t splits, ...);
/// merges N linked lists into one.
/// varargs type: struct linked_list*
/// Leaves `result` list valid, all other ones are invalidated. 
/// Lists are appended in the order they are given in varargs.
/// O(`merges`)
void ll_merge_n(struct linked_list* result, size_t merges, ...);
/// creates array (not an array of pointers, array of elements! it uses memcpy to copy elements) 
/// Leaves linked list unchanged
void* ll_to_array(const struct linked_list* list);
/// Same as `ll_to_array`, except it accepts function used for copying. Use if you think
/// that your data should not be `memcpy`'d
void* ll_to_array_custom_cpy(const struct linked_list* list, memcpy_t cpy);
/// literally does: 
/// 1. linked list to array
/// 2. sort the array using qsort
/// 3. array to linked list
/// since linked list in not random-access, i think this is literally more efficient. 
/// Also, I am not in a mood to write sorting algorithms today
void ll_sort(struct linked_list *list, qsort_cmp_t cmp);
/// Deallocates a node and returns next one;
struct linked_list_node* ll_free_node(struct linked_list* list, struct linked_list_node* node) ;
/// Sets a custom free function for the data of the list
void ll_set_free(struct linked_list* list, free_t new_free);
/// Creates a deep copy of the list, meaning that it must be free'd as any other list
/// Uses `cpy` function to copy data to new location. If NULL, memcpy is used.
struct linked_list ll_deep_copy(const struct linked_list* list, memcpy_t cpy);

/* ----------------------------------------------------------------- */
/* ------------------------HASH MAP HEADER-------------------------- */
/* ----------------------------------------------------------------- */

#ifndef HMAP_MAX_BUCKET_SIZE
	#define HMAP_MAX_BUCKET_SIZE 8
#endif
#ifndef HMAP_INIT_SIZE
	#define HMAP_INIT_SIZE 32
#endif
/// should return `0` only if first argument is equal to second,
/// otherwise should return `1` or any other integer
/// OPERATES ON `struct hmap_pair*` !!!!!!!!!! NOT ON KEYS!
typedef int(*hmap_equal_fn)(const void*, const void*);
typedef size_t(*hmap_hash_fn)(const void*);
struct hmap_pair {
	void* key;
	void* val;
};
struct hash_map {
	struct linked_list* buckets;
	hmap_equal_fn eq;
	hmap_hash_fn hash;
	size_t key_size;
	size_t val_size;
};
enum hmapi_cmp_res {
	hmap_lt = -1,
	hmap_eq = 0,
	hmap_gt = 1,
	hmap_no = 0xB19A55,
};
struct hash_map_iter {
	const struct hash_map* map;
	size_t bucket_pos;
	struct linked_list_node* current_node;
};
typedef struct hash_map_iter hmap_iter;
struct hash_map hmap_new(const size_t key_size, const size_t val_size, hmap_equal_fn eq, hmap_hash_fn hash);
/// copies data from pointers
struct hash_map hmap_new_from_buffer(size_t key_size, size_t val_size, hmap_equal_fn eq, hmap_hash_fn hash, const struct hmap_pair* buf, size_t buf_len);
void hmap_free(struct hash_map* map);
/// copy the contents of the pair into hash map (using `memcpy`). 
/// The element is left unchanged, the memory pointed to by `element` may be anything, from stack to FFI allocator
void hmap_insert_copy(struct hash_map* map, const struct hmap_pair element);
/// returns 1 if an object was inserted, 0 if it was not. This is done to be able to free uninserted pointer.
char hmap_insert_mallocated(struct hash_map* map, struct hmap_pair element);
/// pair is a pointer
#define hmap_insert_pair(map, pair) hmap_insert_range(map, pair, 1);
/// copies data from pointers
void hmap_insert_range(struct hash_map* map, const struct hmap_pair* elements, size_t elements_len);
void* hmap_get(struct hash_map* map, void* key);
/// returns 1 if value was updates successfully, otherwise 0 (if key does not exist)
char hmap_update(struct hash_map* map, void* key, void* new_val);
void hmap_remove(struct hash_map* map, void* key);
/// double the size of a hash map
void hmap_rehash(struct hash_map* map);
void hmap_rehash_to_size(struct hash_map* map, size_t new_size);

/// initialize iterator `iter`.
void hmapi_begin(const struct hash_map* map, struct hash_map_iter* iter);
/// set iter to next element. If there is no next element, `hash_map_iter.current_node` is set to NULL
void hmapi_next(struct hash_map_iter* iter);
/// Get a key-value pair at the iterator iter. 
/// If iter is pointing beyond hash map (after the last element), `(struct hmap_pair){NULL, NULL}` is returned
struct hmap_pair hmapi_get_data(const struct hash_map_iter* iter);
void* hmapi_get_key(const struct hash_map_iter* iter);
void* hmapi_get_val(const struct hash_map_iter* iter);
/// the function does comparison the same way `qsort_cmp_t` might do it, except this is wrapped in an enum. 
/// This is done because 2 iterators might be incomparable (come from different hash maps), in this case `hampi_no` is returned.
enum hmapi_cmp_res hmapi_cmp(const struct hash_map_iter *a, const struct hash_map_iter* b);
/// 1 if a > b, 0 otherwise.
char hmapi_gt(const struct hash_map_iter *a, const struct hash_map_iter* b);
/// 1 if a = b, 0 otherwise
char hmapi_eq(const struct hash_map_iter *a, const struct hash_map_iter* b);
/// 1 if a < b, 0 otherwise
char hmapi_lt(const struct hash_map_iter *a, const struct hash_map_iter* b);
/// 1 if a >= b, 0 otherwise 
char hmapi_ge(const struct hash_map_iter *a, const struct hash_map_iter* b);
/// 1 if a <= b, 0 otherwise 
char hmapi_le(const struct hash_map_iter *a, const struct hash_map_iter* b);
/// 1 if a != b, otherwise 0
char hmapi_ne(const struct hash_map_iter *a, const struct hash_map_iter* b);
/// 1 if `iter` is an end iterator, 0 otherwise
char hmapi_end(const struct hash_map_iter* iter);

/* ----------------------------------------------------------------- */
/* ------------------------HASH SET HEADER-------------------------- */
/* ----------------------------------------------------------------- */

#ifndef HSET_MAX_BUCKET_SIZE
	#define HSET_MAX_BUCKET_SIZE 8
#endif
#ifndef HSET_INIT_SIZE
	#define HSET_INIT_SIZE 32
#endif
/// should return `0` only if first argument is equal to second,
/// otherwise should return `1` or any other integer
/// unlike hmap, operates on data structures directly
typedef int(*hset_equal_fn)(const void*, const void*);
typedef size_t(*hset_hash_fn)(const void*);
struct hash_set {
	struct linked_list* buckets;
	hset_equal_fn eq;
	hset_hash_fn hash;
	size_t elem_size;
};
enum hseti_cmp_res {
	hset_lt = -1,
	hset_eq = 0,
	hset_gt = 1,
	hset_no = 0xB19A55,
};
struct hash_set_iter {
	const struct hash_set* set;
	size_t bucket_pos;
	struct linked_list_node* current_node;
};
typedef struct hash_set_iter hset_iter;
struct hash_set hset_new(const size_t el_size, hset_equal_fn eq, hset_hash_fn hash);
/// copies data from pointers
struct hash_set hset_new_from_buffer(size_t el_size, hset_equal_fn eq, hset_hash_fn hash, const void* buf, size_t buf_len);
char hset_ok(struct hash_set set);
void hset_free(struct hash_set* set);
/// copy the contents of the pair into hash map (using `memcpy`). 
/// The element is left unchanged, the memory pointed to by `element` may be anything, from stack to FFI allocator
void hset_insert_copy(struct hash_set* set, const void* element);
/// returns 1 if an object was inserted, 0 if it was not. This is done to be able to free uninserted pointer.
char hset_insert_mallocated(struct hash_set* set, void* element);
/// get a pointer to an element with same hash and modify it directly
void* hset_at(struct hash_set* set, void* element);
/// returns 1 if value was updated successfully, otherwise 0 (if key does not exist)
void hset_remove(struct hash_set* set, void* elem);
/// double the size of a hash set
void hset_rehash(struct hash_set* set);
void hset_rehash_to_size(struct hash_set* set, size_t new_size);
size_t hset_count(struct hash_set* set);

/// initialize iterator `iter`.
void hseti_begin(const struct hash_set* map, struct hash_set_iter* iter);
/// set iter to next element. If there is no next element, `hash_set_iter.current_node` is set to NULL
void hseti_next(struct hash_set_iter* iter);
/// If iter is pointing beyond hash set (after the last element), NULL is returned
void* hseti_get(const struct hash_set_iter* iter);
/// the function does comparison the same way `qsort_cmp_t` might do it, except this is wrapped in an enum. 
/// This is done because 2 iterators might be incomparable (come from different hash maps), in this case `hampi_no` is returned.
enum hseti_cmp_res hseti_cmp(const struct hash_set_iter *a, const struct hash_set_iter* b);
/// 1 if a > b, 0 otherwise.
char hseti_gt(const struct hash_set_iter *a, const struct hash_set_iter* b);
/// 1 if a = b, 0 otherwise
char hseti_eq(const struct hash_set_iter *a, const struct hash_set_iter* b);
/// 1 if a < b, 0 otherwise
char hseti_lt(const struct hash_set_iter *a, const struct hash_set_iter* b);
/// 1 if a >= b, 0 otherwise 
char hseti_ge(const struct hash_set_iter *a, const struct hash_set_iter* b);
/// 1 if a <= b, 0 otherwise 
char hseti_le(const struct hash_set_iter *a, const struct hash_set_iter* b);
/// 1 if a != b, otherwise 0
char hseti_ne(const struct hash_set_iter *a, const struct hash_set_iter* b);
/// 1 if `iter` is an end iterator, 0 otherwise
char hseti_end(const struct hash_set_iter* iter);

/* ------------------------------------------------------------------------- */
/* ------From now on, the rest of the header is implementation details------ */
/* -------------------the API and documentation end here-------------------- */
/* ------------------------------------------------------------------------- */

enum _dynarray_header_idx {
	_dah_idx_member_size	= 0,
	_dah_idx_capacity	= 1,
	_dah_idx_size		= 2,
};
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
void* _array_extend(void* array, void* buffer, size_t len);

// uncomment in dev mode so that LSP highlights the code
/* #define CONTAINER_IMPLEMENTATION */
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
void* _array_extend(void* array, void* buffer, size_t len) {
	struct _dynarray_header* header = array_header(array);
	array = _memreserve_dynarray(array, (header->size + len) * header->memb_size);
	memcpy((byte*)array + header->size*header->memb_size, buffer, len * header->memb_size);
	return array;
}
int array_compare(const void *const a1, const void *const a2, qsort_cmp_t comp) {
	struct _dynarray_header *hdr = array_header(a1);
	struct _dynarray_header *hdr2 = array_header(a2);
	const size_t s1 = hdr->size, s2 = hdr2->size;
	const size_t elsize = hdr->memb_size;
	size_t i;
	for (i = 0; i < s1 && i < s2; ++i) {
		int result = comp((byte*) a1 + elsize*i, (byte*) a2 + elsize*i);
		if (result) return result;
	}
	if (s1 == s2) return 0;
	if (i == s1) return -1;
	if (i == s2) return 1;
	TODO;
	__builtin_unreachable();
}
void* array_copy(void* old) {
	return _alloc_dynarray_buffer(array_element_size(old), array_size(old), old, array_size(old) * array_element_size(old));
}
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
struct linked_list array_to_ll(void* array) {
	struct linked_list ret = ll_create_from_buffer(array_element_size(array), array, array_size(array));
	return ret;
}

/* ----------------------------------------------------------------- */
/* ------------------LINKED LIST IMPLEMENTATION--------------------- */
/* ----------------------------------------------------------------- */
struct linked_list ll_create(size_t memb_size) {
	struct __linked_list_meta meta = {
		.assumed_size = 0,
		.element_size = memb_size,
		.free_function = free,
	};
	struct linked_list ret = {
		.meta = meta,
		.first = NULL,
		.last = NULL,
	};
	return ret;
}
struct linked_list ll_create_from_buffer(size_t memb_size, const void* buf, size_t buff_len) {
	struct __linked_list_meta meta = {
		.assumed_size = buff_len,
		.element_size = memb_size,
		.free_function = free,
	};
	struct linked_list_node *current_node = calloc(1, sizeof(struct linked_list_node));
	struct linked_list ret = { .meta = meta, .first = current_node };
	for (size_t i = 0; i < buff_len; ++i) {
		current_node->data = malloc(memb_size);
		memcpy(current_node->data, (const byte *)buf + memb_size * i, memb_size);
		if (i != buff_len - 1) {
			current_node->next = calloc(1, sizeof(struct linked_list_node));
			current_node = current_node->next;
		}
	}
	ret.last = current_node;
	return ret;
}
/// An older, more ugly implementation
void ll_free_(struct linked_list* list) {
	struct linked_list_node *to_dealloc = NULL;
	for (struct linked_list_node *current = list->first; current != NULL; current = current->next) {
		free(to_dealloc);
		free(current->data);
		to_dealloc = current;
	}
	free(to_dealloc);
	memset(list, 0, sizeof(*list));
}
void ll_free(struct linked_list* list) {
	struct linked_list_node* node = list->first;
	while ((node = ll_free_node(list, node)) != NULL) {}
	memset(list, 0, sizeof(*list));
}
char ll_search(const struct linked_list* list, const void* element, qsort_cmp_t cmp) {
	for (struct linked_list_node *current = list->first; current != NULL; current = current->next) {
		if (cmp(element, current->data) == 0) return 1;
	}
	return 0;
}
struct linked_list_node* ll_find(const struct linked_list* list, const void* element, qsort_cmp_t cmp) {
	for (struct linked_list_node *current = list->first; current != NULL; current = current->next) {
		if (cmp(element, current->data) == 0) return current;
	}
	return NULL;
}
struct linked_list_node* ll_at(const struct linked_list* list, const size_t i) {
	size_t j = 0;
	if (i == list->meta.assumed_size - 1) return list->last;
	if (i >= list->meta.assumed_size) return NULL;
	for (struct linked_list_node *current = list->first; current != NULL; current = current->next) {
		if (i == j) return current; 
		++j;
	}
	return NULL;
}
void ll_truncate(struct linked_list* list, const size_t i) {
	if (i == 0) {
		ll_free(list);
		return;
	}
	struct linked_list_node *start = ll_at(list, i-1);
	if (start == NULL) return;
	struct linked_list freeable = {
		.meta = {list->meta.element_size, list->meta.assumed_size - i, .free_function = list->meta.free_function,
},
		.first = start->next,
	};
	ll_free(&freeable);
	list->meta.assumed_size = i;
	start->next = NULL;
	list->last = start;
}
size_t ll_probably_size(const struct linked_list* list) {
	return list->meta.assumed_size;
}
size_t ll_definite_size(struct linked_list *list) {
	int i = 0;
	for (struct linked_list_node *current = list->first; current != NULL; current = current->next)
		++i;
	list->meta.assumed_size = i;
	return i;
}
void ll_append(struct linked_list *list, const void* restrict element) {
	struct linked_list_node *new = calloc(1, sizeof(struct linked_list_node));
	new->data = malloc(list->meta.element_size);
	memcpy(new->data, element, list->meta.element_size);
	if (list->last == NULL) list->first = new;
	else list->last->next = new;
	++list->meta.assumed_size;
	list->last = new;
}
void ll_append_mallocated_pointer(struct linked_list *list, void* pointer) {
	struct linked_list_node *new = malloc(sizeof(struct linked_list_node));
	new->data = pointer;
	new->next = NULL;
	if (list->last == NULL) list->first = list->last = new;
	else list->last->next = new;
	++list->meta.assumed_size;
	list->last = new;
}
void ll_insert_front(struct linked_list *list, const void* restrict element) {
	struct linked_list_node *new = calloc(1, sizeof(struct linked_list_node));
	new->data = malloc(list->meta.element_size);
	memcpy(new->data, element, list->meta.element_size);
	if (list->first == NULL) list->first = list->last = new;
	else {
		new->next = list->first;
		list->first = new;
	}
	++list->meta.assumed_size;
}
void ll_insert_front_mallocated_pointer(struct linked_list *list, void* pointer) {
	struct linked_list_node *new = calloc(1, sizeof(struct linked_list_node));
	new->data = pointer;
	if (list->first == NULL) list->first = list->last = new;
	else {
		new->next = list->first;
		list->first = new;
	}
	++list->meta.assumed_size;
}
char ll_insert_at(struct linked_list *list, const void* restrict element, size_t i) {
	if (i == 0) {
		ll_insert_front(list, element);
		return 1;
	}
	if (i == list->meta.assumed_size) {
		ll_append(list, element);
		return 1;
	}
	if (list->last == NULL) return 0;

	struct linked_list_node* delim;
	if ( (delim = ll_at(list, i-1)) == NULL ) return 0;
	struct linked_list_node* new = malloc(sizeof(struct linked_list_node));
	new->next = delim->next;
	delim->next = new;
	new->data = malloc(list->meta.element_size);
	memcpy(new->data, element, list->meta.element_size);
	++list->meta.assumed_size;
	return 1;
}
char ll_insert_at_mallocated_pointer(struct linked_list *list, void* pointer, size_t i) {
	if (i == 0) {
		ll_insert_front_mallocated_pointer(list, pointer);
		return 1;
	}
	if (i == list->meta.assumed_size) {
		ll_append_mallocated_pointer(list, pointer);
		return 1;
	}
	if (list->last == NULL) return 0;

	struct linked_list_node* delim;
	if ((delim = ll_at(list, i-1)) == NULL) return 0;
	struct linked_list_node* new = malloc(sizeof(struct linked_list_node));
	new->next = delim->next;
	delim->next = new;
	new->data = pointer;
	++list->meta.assumed_size;
	return 1;
}
void ll_remove_front(struct linked_list* list) {
	if (list->first == NULL) return;
	list->first = ll_free_node(list, list->first);
	--list->meta.assumed_size;
	if (list->meta.assumed_size <= 1) list->last = list->first;
}
void ll_remove_back(struct linked_list* list) {
	if (list->meta.assumed_size == 0) return;
	if (list->meta.assumed_size == 1) {
		list->first = list->last = ll_free_node(list, list->first);
		goto common;
	}
	if (list->meta.assumed_size == 2) {
		list->first->next = ll_free_node(list, list->last);
		list->last = list->first;
		goto common;
	}
	struct linked_list_node* prelast;
	if ((prelast = ll_at(list, list->meta.assumed_size-1-1)) == NULL) return;
	ll_free_node(list, list->last);
	prelast->next = NULL;
	list->last = prelast;
common:
	--list->meta.assumed_size;
}
char ll_remove_at(struct linked_list *list, size_t i) {
	if (list->first == NULL) return 0;
	if (i == 0) {
		ll_remove_front(list);
		return 1;
	}
	if (i == list->meta.assumed_size - 1) {
		ll_remove_back(list);
		return 1;
	}
	struct linked_list_node *before_removed;
	if ((before_removed = ll_at(list, i-1)) == NULL) return 0;
	before_removed->next = ll_free_node(list, before_removed->next);
	--list->meta.assumed_size;
	return 1;
}
void ll_remove_elem(struct linked_list* list, const void* elem, qsort_cmp_t cmp) {
	if (list->first == NULL) return;
	if (cmp(list->first->data, elem) == 0) {
		ll_remove_front(list);
		return;
	}
	struct linked_list_node *fst = list->first, 
				*snd = fst->next;
	while (snd != NULL) {
		if (cmp(snd->data, elem) == 0) {
			fst->next = ll_free_node(list, snd);
			return;
		}
	}
	if (cmp(fst->data, elem) == 0) ll_remove_back(list);
}
void ll_remove_all(struct linked_list* list, const void* elem, qsort_cmp_t cmp) {
	printf("DON'T USE THIS FUNCTION!!!!!\n");
	TODO;
	__builtin_unreachable();

	if (list->first == NULL) return;
	while (1) {
		if (list->first == NULL) return;
		if (cmp(list->first->data, elem) == 0) ll_remove_front(list);
		else break;
	}
	struct linked_list_node *prev = list->first,
			        *current;
	for (current = prev->next; current != NULL; current = prev->next) {
		if (cmp(current->data, elem) != 0) {
			prev = current;
			continue;
		}
		if (current == list->last) {
			ll_remove_back(list);
			return;
		}
		prev->next = ll_free_node(list, current);
		--list->meta.assumed_size;
	}
}
struct linked_list ll_split_at(struct linked_list *list, size_t i) {
	struct linked_list ret = {.meta = list->meta };
	ret.meta.element_size = list->meta.element_size;
	if (i == 0) {
		ret = *list;
		list->first = list->last = NULL;
		list->meta.assumed_size = 0;
		return ret;
	}
	if (i == list->meta.assumed_size) return ll_create(list->meta.element_size);
	struct linked_list_node* spl_prev = ll_at(list, i-1);
	if (spl_prev == NULL) return ret;
	ret.first = spl_prev->next;
	ret.last = list->last;
	spl_prev->next = NULL;
	list->last = spl_prev;
	ret.meta.assumed_size = list->meta.assumed_size - i;
	list->meta.assumed_size = i;
	return ret;
}
void ll_merge(struct linked_list* fst, struct linked_list* snd) {
	fst->last->next = snd->first;
	fst->last = snd->last;
	fst->meta.assumed_size += snd->meta.assumed_size;
	memset(snd, 0, sizeof(*snd));
}
struct linked_list* ll_split_n(struct linked_list* list, size_t splits, ...) {
	struct linked_list* ret = array_new(struct linked_list, splits + 1);
	ret[0] = *list;
	size_t accum_index = 0;
	va_list args;
	va_start(args, splits);
	for (size_t i = 1; i <= splits; ++i) {
		size_t split_index = va_arg(args, size_t);
		ret[i] = ll_split_at(ret+i-1, split_index-accum_index);
		accum_index = split_index;
	}
	return ret;
}
void ll_merge_n(struct linked_list* result, size_t merges, ...) {
	va_list args;
	va_start(args, merges);
	for (size_t i = 0; i < merges; ++i) {
		struct linked_list* list = va_arg(args, struct linked_list*);
		ll_merge(result, list);
	}
}
void* ll_to_array(const struct linked_list* list) {
	return ll_to_array_custom_cpy(list, memcpy);
}
void* ll_to_array_custom_cpy(const struct linked_list* list, memcpy_t cpy) {
	void* array = _alloc_dynarray(list->meta.element_size, list->meta.assumed_size);
	struct linked_list_node* iter = list->first;
	for (size_t i = 0; (iter = iter->next) != NULL; ++i) {
		cpy((byte*) array + list->meta.element_size * i, iter->data, list->meta.element_size);
	}
	return array;
}
void ll_sort(struct linked_list *list, qsort_cmp_t cmp) {
	void* array = ll_to_array(list);
	array_qsort(array, cmp);
	ll_free(list);
	*list = array_to_ll(array);
	array_free(array);
}
struct linked_list_node* ll_free_node(struct linked_list* list, struct linked_list_node* node) {
	if (node == NULL) return NULL;
	struct linked_list_node* ret = node->next;
	list->meta.free_function(node->data);
	free(node);
	return ret;
}
void ll_set_free(struct linked_list* list, free_t new_free) {
	list->meta.free_function = new_free;
}
struct linked_list ll_deep_copy(const struct linked_list* list, memcpy_t cpy) {
	if (cpy == NULL) cpy = memcpy;
	struct linked_list ret = *list;
	ret.meta.assumed_size = 0;
	ret.last = ret.first = NULL;
	struct linked_list_node* iter = list->first;
	void* memory = alloca(list->meta.element_size);
	memset(memory, 0, list->meta.element_size);
	while (iter != NULL) {
		ll_append(&ret, memory);
		cpy(iter->data, ret.last->data, list->meta.element_size);
		iter = iter->next;
	}
	return ret;
}


/* ----------------------------------------------------------------- */
/* ---------------------HASH MAP IMPLEMENTATION--------------------- */
/* ----------------------------------------------------------------- */

void __hmap_ll_custom_free(void* data) {
	struct hmap_pair *pair = data;
	free(pair->key);
	free(pair->val);
	free(data);
}
struct hash_map hmap_new(const size_t key_size, const size_t val_size, hmap_equal_fn eq, hmap_hash_fn hash) {
	struct hash_map ret = {
		.buckets = array_new(struct linked_list, HMAP_INIT_SIZE),
		.key_size = key_size,
		.val_size = val_size,
		.eq = eq,
		.hash = hash,
	};
	const struct linked_list def = {
		.meta = {
			.element_size = sizeof(struct hmap_pair),
			.assumed_size = 0, 
			.free_function = __hmap_ll_custom_free,
		}, 
		.first = NULL, 
		.last = NULL,
	};
	for (size_t i = 0; i < HMAP_INIT_SIZE; ++i) ret.buckets[i] = def;
	return ret;
}
struct hash_map hmap_new_from_buffer(size_t key_size, size_t val_size, hmap_equal_fn eq, hmap_hash_fn hash, const struct hmap_pair* buf, size_t buf_len) {
	struct hash_map ret = hmap_new(key_size, val_size, eq, hash);
	hmap_rehash_to_size(&ret, buf_len / HMAP_MAX_BUCKET_SIZE);
	for (size_t i = 0; i < buf_len; ++i) hmap_insert_copy(&ret, buf[i]);
	return ret;
}
void hmap_free(struct hash_map* map) {
	for (size_t i = 0; i < array_size(map->buckets); ++i) ll_free(map->buckets + i);
	array_free(map->buckets);
	memset(map, 0, sizeof(*map));
}
void hmap_insert_copy(struct hash_map* map, const struct hmap_pair element) {
	size_t index = map->hash(element.key) % array_size(map->buckets);
	if (ll_search(map->buckets + index, &element, map->eq)) {
		struct hmap_pair* target = ll_find(map->buckets + index, &element, map->eq)->data;
		memcpy(target->val, element.val, map->val_size);
		return;
	}
	struct hmap_pair copy = {malloc(map->key_size), malloc(map->val_size)};
	memcpy(copy.key, element.key, map->key_size);
	memcpy(copy.val, element.val, map->val_size);
	ll_append(map->buckets + index, &copy);
	if (map->buckets[index].meta.assumed_size > HMAP_MAX_BUCKET_SIZE) hmap_rehash(map);
}
char hmap_insert_mallocated(struct hash_map* map, struct hmap_pair element) {
	size_t index = map->hash(element.key) % array_size(map->buckets);
	if (ll_search(map->buckets + index, &element, map->eq)) {
		struct hmap_pair* target = ll_find(map->buckets + index, &element, map->eq)->data;
		free(target->val);
		target->val = element.val;
		free(element.key);
		return 0;
	}
	ll_append(map->buckets + index, &element);
	if (map->buckets[index].meta.assumed_size > HMAP_MAX_BUCKET_SIZE) hmap_rehash(map);
	return 1;
}
void hmap_insert_range(struct hash_map* map, const struct hmap_pair* elements, size_t elements_len) {
	for (size_t i = 0; i < elements_len; ++i) hmap_insert_copy(map, elements[i]);
}
void* hmap_get(struct hash_map* map, void* key) {
	size_t index = map->hash(key) % array_size(map->buckets);
	struct linked_list* target_list = map->buckets + index;
	struct hmap_pair pr = {.key = key, .val = NULL };
	struct linked_list_node* node = ll_find(target_list, &pr, map->eq);
	if (node == NULL) return NULL;
	struct hmap_pair *pair = node->data;
	return pair->val;
}
char hmap_update(struct hash_map* map, void* key, void* new_val) {
	size_t index = map->hash(key) % array_size(map->buckets);
	struct linked_list* target_list = map->buckets + index;
	struct hmap_pair pr = {.key = key, .val = NULL };
	struct linked_list_node* node = ll_find(target_list, &pr, map->eq);
	if (node == NULL) return 0;
	memcpy(((struct hmap_pair*)node->data)->val, new_val, map->val_size);
	return 1;
}
void hmap_remove(struct hash_map* map, void* key) {
	size_t index = map->hash(key) % array_size(map->buckets);
	struct linked_list* target_list = map->buckets + index;
	struct hmap_pair tmppr = {.key = key, .val = NULL};
	ll_remove_elem(target_list, &tmppr, map->eq);
}
void hmap_rehash(struct hash_map* map) {
	hmap_rehash_to_size(map, array_size(map->buckets) << 1);
}
/// double the size of a hash map
void hmap_rehash_to_size(struct hash_map* map, size_t new_size) {
	struct linked_list* new_buckets = array_new(struct linked_list, new_size);
	for (size_t i = 0; i < new_size; ++i) {
		new_buckets[i] = ll_create(sizeof(struct hmap_pair));
		new_buckets[i].meta.free_function = __hmap_ll_custom_free;
	}
	struct hash_map_iter iter;
	for (hmapi_begin(map, &iter); !hmapi_end(&iter); hmapi_next(&iter)) {
		size_t hash_pos = map->hash(hmapi_get_key(&iter)) % new_size;
		struct hmap_pair hash_data = hmapi_get_data(&iter);
		ll_append(new_buckets + hash_pos, &hash_data);
	}
	for (size_t i = 0; i < array_size(map->buckets); ++i) {
		map->buckets[i].meta.free_function = free;
		ll_free(map->buckets + i);
	}
	array_free(map->buckets);
	map->buckets = new_buckets;
}
void hmapi_begin(const struct hash_map* map, struct hash_map_iter* iter) {
	memset(iter, 0, sizeof(struct hash_map_iter));
	iter->map = map;
	struct linked_list* buckets = map->buckets;
	for (size_t i = 0; i < array_size(buckets); ++i) {
		if (buckets[i].first == NULL) continue;
		iter->bucket_pos = i;
		iter->current_node = buckets[i].first;
		return;
	}
	iter->bucket_pos = SIZE_MAX;
	iter->current_node = NULL;
}
void hmapi_next(struct hash_map_iter* iter) {
	if (iter->current_node == NULL) return;
	if (iter->current_node->next != NULL) {
		iter->current_node = iter->current_node->next;
		return;
	}
	const struct linked_list *buckets = iter->map->buckets;
	for (size_t i = iter->bucket_pos + 1; i < array_size(buckets); ++i) {
		if (buckets[i].first == NULL) continue;
		iter->bucket_pos = i;
		iter->current_node = buckets[i].first;
		return;
	}
	iter->bucket_pos = SIZE_MAX;
	iter->current_node = NULL;
}
struct hmap_pair hmapi_get_data(const struct hash_map_iter* iter) {
	if (iter->current_node == NULL) return (struct hmap_pair){NULL, NULL};
	struct hmap_pair ret = *(struct hmap_pair*)(iter->current_node->data);
	return ret;
}
void* hmapi_get_key(const struct hash_map_iter* iter) {
	if (iter->current_node == NULL) return NULL;
	struct hmap_pair *pair = iter->current_node->data;
	return pair->key;
}
void* hmapi_get_val(const struct hash_map_iter* iter) {
	if (iter->current_node == NULL) return NULL;
	struct hmap_pair *pair = iter->current_node->data;
	return pair->val;
}
enum hmapi_cmp_res hmapi_cmp(const struct hash_map_iter *a, const struct hash_map_iter* b) {
	if (a->map != b->map) return hmap_no;
	if (a->bucket_pos != b->bucket_pos) return (a->bucket_pos > b->bucket_pos) ? hmap_gt : hmap_lt;
	if (a->current_node == b->current_node) return hmap_eq;
	for (const struct linked_list_node* nd = a->current_node; nd != NULL; nd = nd->next) {
		if (nd == b->current_node) return hmap_lt;
	}
	return hmap_gt;
}
char hmapi_gt(const struct hash_map_iter *a, const struct hash_map_iter* b) {
	return hmapi_cmp(a, b) == hmap_gt;
}
char hmapi_eq(const struct hash_map_iter *a, const struct hash_map_iter* b) {
	return hmapi_cmp(a, b) == hmap_eq;
}
char hmapi_lt(const struct hash_map_iter *a, const struct hash_map_iter* b) {
	return hmapi_cmp(a, b) == hmap_lt;
}
char hmapi_ge(const struct hash_map_iter *a, const struct hash_map_iter* b) {
	enum hmapi_cmp_res res = hmapi_cmp(a, b);
	return res == hmap_gt || res == hmap_eq;
}
char hmapi_le(const struct hash_map_iter *a, const struct hash_map_iter* b) {
	enum hmapi_cmp_res res = hmapi_cmp(a, b);
	return res == hmap_lt || res == hmap_eq;
}
char hmapi_ne(const struct hash_map_iter *a, const struct hash_map_iter* b) {
	return hmapi_cmp(a, b) != hmap_eq;
}
char hmapi_end(const struct hash_map_iter* iter) {
	return iter->current_node == NULL || iter->bucket_pos == SIZE_MAX;
}

/* ----------------------------------------------------------------- */
/* ---------------------HASH SET IMPLEMENTATION--------------------- */
/* ----------------------------------------------------------------- */


struct hash_set hset_new(const size_t el_size, hset_equal_fn eq, hset_hash_fn hash) {
	struct hash_set ret = {
		.buckets = array_new(struct linked_list, HMAP_INIT_SIZE),
		.elem_size = el_size,
		.eq = eq,
		.hash = hash,
	};
	const struct linked_list def = {
		.meta = {
			.element_size = el_size,
			.assumed_size = 0, 
			.free_function = &free,
		}, 
		.first = NULL, 
		.last = NULL,
	};
	for (size_t i = 0; i < HSET_INIT_SIZE; ++i) ret.buckets[i] = def;
	return ret;
}
struct hash_set hset_new_from_buffer(size_t el_size, hset_equal_fn eq, hset_hash_fn hash, const void* buf, size_t buf_len) {
	struct hash_set ret = hset_new(el_size, eq, hash);
	hset_rehash_to_size(&ret, buf_len / HSET_MAX_BUCKET_SIZE);
	for (size_t i = 0; i < buf_len; ++i) hset_insert_copy(&ret, (byte*)buf + i * el_size);
	return ret;
}
char hset_ok(struct hash_set set) {
	return set.buckets != NULL && set.elem_size != 0;
}
void hset_free(struct hash_set* set) {
	for (size_t i = 0; i < array_size(set->buckets); ++i) ll_free(set->buckets + i);
	array_free(set->buckets);
	memset(set, 0, sizeof(*set));
}
void hset_insert_copy(struct hash_set* set, const void* element) {
	size_t index = set->hash(element) % array_size(set->buckets);
	if (ll_search(set->buckets + index, element, set->eq)) {
		void* target = ll_find(set->buckets + index, element, set->eq)->data;
		memcpy(target, element, set->elem_size);
		return;
	}
	ll_append(set->buckets + index, element);
	if (set->buckets[index].meta.assumed_size > HSET_MAX_BUCKET_SIZE) hset_rehash(set);
}
char hset_insert_mallocated(struct hash_set* set, void* element) {
	size_t index = set->hash(element) % array_size(set->buckets);
	if (ll_search(set->buckets + index, element, set->eq)) {
		void* target = ll_find(set->buckets + index, element, set->eq)->data;
		free(target);
		target = element;
		return 0;
	}
	ll_append(set->buckets + index, element);
	if (set->buckets[index].meta.assumed_size > HSET_MAX_BUCKET_SIZE) hset_rehash(set);
	return 1;

}
void* hset_at(struct hash_set* set, void* element) {
	size_t index = set->hash(element) % array_size(set->buckets);
	if (ll_search(set->buckets + index, element, set->eq))
		return ll_find(set->buckets + index, element, set->eq)->data;
	return NULL;
}
void hset_remove(struct hash_set* set, void* elem) {
	size_t index = set->hash(elem) % array_size(set->buckets);
	struct linked_list* target_list = set->buckets + index;
	ll_remove_elem(target_list, elem, set->eq);
}
void hset_rehash(struct hash_set* set) {
	hset_rehash_to_size(set, array_size(set->buckets) << 1);
}
void hset_rehash_to_size(struct hash_set* set, size_t new_size) {
	struct linked_list* new_buckets = array_new(struct linked_list, new_size);
	for (size_t i = 0; i < new_size; ++i) {
		new_buckets[i] = ll_create(set->elem_size);
		/* new_buckets[i].meta.free_function = free; */
	}
	struct hash_set_iter iter;
	for (hseti_begin(set, &iter); !hseti_end(&iter); hseti_next(&iter)) {
		void* hash_data = hseti_get(&iter);
		size_t hash_pos = set->hash(hash_data) % new_size;
		ll_append(new_buckets + hash_pos, hash_data);
	}
	for (size_t i = 0; i < array_size(set->buckets); ++i) {
		ll_free(set->buckets + i);
	}
	array_free(set->buckets);
	set->buckets = new_buckets;
}
size_t hset_count(struct hash_set* set) {
	size_t res = 0;
	for (size_t i = 0; i < array_size(set->buckets); ++i) {
		res += (set->buckets)[i].meta.assumed_size;
	}
	return res;
}

void hseti_begin(const struct hash_set* set, struct hash_set_iter* iter) {
	memset(iter, 0, sizeof(struct hash_map_iter));
	iter->set = set;
	struct linked_list* buckets = set->buckets;
	for (size_t i = 0; i < array_size(buckets); ++i) {
		if (buckets[i].first == NULL) continue;
		iter->bucket_pos = i;
		iter->current_node = buckets[i].first;
		return;
	}
	iter->bucket_pos = SIZE_MAX;
	iter->current_node = NULL;
}
void hseti_next(struct hash_set_iter* iter) {
	if (iter->current_node == NULL) return;
	if (iter->current_node->next != NULL) {
		iter->current_node = iter->current_node->next;
		return;
	}
	const struct linked_list *buckets = iter->set->buckets;
	for (size_t i = iter->bucket_pos + 1; i < array_size(buckets); ++i) {
		if (buckets[i].first == NULL) continue;
		iter->bucket_pos = i;
		iter->current_node = buckets[i].first;
		return;
	}
	iter->bucket_pos = SIZE_MAX;
	iter->current_node = NULL;
}
void* hseti_get(const struct hash_set_iter* iter) {
	if (iter->current_node == NULL) return NULL;
	return iter->current_node->data;
}
enum hseti_cmp_res hseti_cmp(const struct hash_set_iter *a, const struct hash_set_iter* b) {
	if (a->set != b->set) return hset_no;
	if (a->bucket_pos != b->bucket_pos) return (a->bucket_pos > b->bucket_pos) ? hset_gt : hset_lt;
	if (a->current_node == b->current_node) return hset_eq;
	for (const struct linked_list_node* nd = a->current_node; nd != NULL; nd = nd->next) {
		if (nd == b->current_node) return hset_lt;
	}
	return hset_gt;
}
char hseti_gt(const struct hash_set_iter *a, const struct hash_set_iter* b) {
	return hseti_cmp(a, b) == hset_gt;
}
char hseti_eq(const struct hash_set_iter *a, const struct hash_set_iter* b) {
	return hseti_cmp(a, b) == hset_eq;
}
char hseti_lt(const struct hash_set_iter *a, const struct hash_set_iter* b) {
	return hseti_cmp(a, b) == hset_lt;
}
char hseti_ge(const struct hash_set_iter *a, const struct hash_set_iter* b) {
	enum hseti_cmp_res cmp = hseti_cmp(a, b);
	return cmp == hset_gt || cmp == hset_eq;
}
char hseti_le(const struct hash_set_iter *a, const struct hash_set_iter* b) {
	enum hseti_cmp_res cmp = hseti_cmp(a, b);
	return cmp == hset_lt || cmp == hset_eq;
}
char hseti_ne(const struct hash_set_iter *a, const struct hash_set_iter* b) {
	enum hseti_cmp_res cmp = hseti_cmp(a, b);
	return cmp != hset_eq;
}
char hseti_end(const struct hash_set_iter* iter) {
	return iter->current_node == NULL || iter->bucket_pos == SIZE_MAX;
}
#endif // CONTAINER_IMPLEMENTATION

#endif // JUSTANOTHERCATGIRL_HEADERS_CONTAINER
