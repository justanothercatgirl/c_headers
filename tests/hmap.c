#define CONTAINER_IMPLEMENTATION
#define HMAP_MAX_BUCKET_SIZE 1
#define HMAP_INIT_SIZE 1
#include "../include/container.h"


// Necessary for hash map to operate
int int_eqals(const struct hmap_pair* a, const struct hmap_pair* b) {
	return *(int*)(a->key) - *(int*)(b->key);
}

// Just a regular hash algorithm
size_t int_hash(const int* a) {
	return *a ^ (*a << 1);
}

// A macro for fun
#define free_pair(pair) do { free(pair.key); free(pair.val); } while(0)

// Here is an example of how you might iterate elements in hash map
void print_hmap(struct hash_map* map) {
	printf("hash map at %p\n", (void*)map);
	struct hash_map_iter iter;
	for (hmapi_begin(map, &iter); !hmapi_end(&iter); hmapi_next(&iter)) {
		printf("key: %i, value: %i\n", *(int*)hmapi_get_key(&iter), *(int*)hmapi_get_val(&iter));
	}
}
struct hmap_pair mkpr(int key, int val) {
	struct hmap_pair ret = {malloc(sizeof(int)), malloc(sizeof(int))};
	*(int*)ret.key = key;
	*(int*)ret.val = val;
	return ret;
}
void freepr(struct hmap_pair* pr) {
	free(pr->key);
	free(pr->val);
}

int main(void) {
	// Casting functions to the right type should not break it
	struct hmap_pair pairs[] = {mkpr(1, 2), mkpr(2, 3), mkpr(3, 4), mkpr(3, 5), mkpr(3, 6), mkpr(6, 8)};
	struct hash_map map = hmap_create_from_buffer(
			sizeof(int), sizeof(int), 
			(hmap_equal_fn)int_eqals, (hmap_hash_fn)int_hash,
			pairs, sizeof(pairs)/sizeof(*pairs));
	
	struct hmap_pair w1 = {malloc(sizeof(int)), malloc(sizeof(int))};
	*(int*)(w1.key) = 10;
	*(int*)(w1.val) = 15;
	struct hmap_pair w2 = {malloc(sizeof(int)), malloc(sizeof(int))};
	*(int*)(w2.key) = 10;
	*(int*)(w2.val) = 15;
	struct hmap_pair w3 = {malloc(sizeof(int)), malloc(sizeof(int))};
	*(int*)(w3.key) = -69;
	*(int*)(w3.val) = 12;
	
	// Ensure there are no memory leaks
	hmap_insert_mallocated(&map, w1);
	hmap_insert_mallocated(&map, w2);
	hmap_insert_mallocated(&map, w3);
	
	int key;
	int val;
	struct hmap_pair p1 = {.key = &key, .val = &val};
	key = 274;
	val = 349;
	hmap_insert_copy(&map, p1);
	key = 1234;
	val = 5678;
	hmap_insert_copy(&map, p1);
	key = 42;
	val = 727;
	hmap_insert_copy(&map, p1);
	hmap_insert_copy(&map, p1); // Second insert does not do anything
	
	int zero = 0;
	hmap_update(&map, &key, &zero);

	int ten = 10;
	printf("hmap at %i: %i\n", ten, *(int*)hmap_get(&map, &ten));
	hmap_remove(&map, &ten);

	struct hmap_pair pairs_2[] = {mkpr(100, 200), mkpr(200, 600), mkpr(600, 300), mkpr(300, 420)};
	hmap_insert_range(&map, pairs_2, sizeof(pairs_2)/sizeof(*pairs_2));
	
	hmap_iter iter;
	void** rmkeys = array_new(void*, 0);
	for (hmapi_begin(&map, &iter); !hmapi_end(&iter); hmapi_next(&iter)) {
		if (*(int*)hmapi_get_key(&iter) % 100 != 0) 
			array_push(rmkeys, hmapi_get_key(&iter));
	}
	for (size_t i = 0; i < array_size(rmkeys); ++i)
		hmap_remove(&map, rmkeys[i]);
	array_free(rmkeys);

	print_hmap(&map);
	hmap_free(&map);
	for (size_t i = 0; i < sizeof(pairs)/sizeof(*pairs); ++i) {
		freepr(pairs + i);
	}
	for (size_t i = 0; i < sizeof(pairs_2)/sizeof(*pairs_2); ++i) {
		freepr(pairs_2 + i);
	}
	return 0;
}
