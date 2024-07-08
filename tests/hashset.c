#define CONTAINER_IMPLEMENTATION
#include "../include/container.h"
#include <stdio.h>

struct user {
	int id;
	const char* name;
};

size_t user_hash(const struct user* u) {
	return u->id;
}

int user_eq(const struct user* a, const struct user* b) {
	return !(a->id == b->id && strcmp(a->name, b->name) == 0);
}

struct user user(int id, const char* name) {
	return (struct user) {.id = id, .name = name};
}

void printuser(struct user* u) {
	printf("(struct user) {.id=%i, .name=\"%s\"};\n", u->id, u->name);
}

int main(void) {
	struct hash_set set = hset_create(sizeof(struct user), (hset_equal_fn)user_eq, (hset_hash_fn)user_hash);
	struct user u0 = user(0, "Jhon");
	struct user u1 = user(1, "John");
	struct user u2 = user(2, "John");
	struct user uw = user(2, "Jnoh");
	struct user ud = user(2, "John");
	hset_insert_copy(&set, &u0);
	hset_insert_copy(&set, &u1);
	hset_insert_copy(&set, &u2);
	hset_insert_copy(&set, &uw);
	hset_insert_copy(&set, &ud);
	hset_insert_copy(&set, &ud);
	hset_insert_copy(&set, &ud);
	hset_insert_copy(&set, &ud);
	hset_insert_copy(&set, &ud);
	hset_insert_copy(&set, &ud);
	hset_insert_copy(&set, &ud);

	hset_iter iter;
	for (hseti_begin(&set, &iter); !hseti_end(&iter); hseti_next(&iter)) {
		struct user* u = hseti_get(&iter);
		printuser(u);
	}

	hset_free(&set);
	return 0;
}
