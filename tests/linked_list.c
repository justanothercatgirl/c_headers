#define CONTAINER_IMPLEMENTATION
#include "../include/container.h"
#include <stdio.h>

void print_linked_list(struct linked_list* list) {
	int i = 0;
	printf("linked list at %p, size = %zu\n", list, list->meta.assumed_size);
	printf("first=%p, last=%p\n", list->first, list->last);
	for (struct linked_list_node* current = list->first; current != NULL; current = current->next) {
		printf("[%i] = {%i, %p}\n", i, *(int*)current->data, current->next);
		++i;
	}
}

int main() {
	int buf[] = {0, 1, 3, 2, 7, 8, 6, 0, 18, 1};
	struct linked_list list = ll_create_from_buffer(sizeof(int), buf, sizeof(buf)/sizeof(*buf));

	ll_sort(&list, __default_int_cmp);

	ll_remove_front(&list);
	const int one = 1;
	ll_insert_front(&list, &one);
	ll_insert_front(&list, &one);
	ll_append(&list, &one);
	ll_append(&list, &one);
	ll_remove_all(&list, &one, __default_int_cmp);
	print_linked_list(&list);
	ll_remove_at(&list, 3);
	ll_remove_back(&list);
	ll_remove_front(&list);
	struct linked_list list_2 = ll_deep_copy(&list, NULL);
	print_linked_list(&list);
	print_linked_list(&list_2);

	while (list.first != NULL && list.last != NULL) ll_remove_front(&list);
	while (list_2.first != NULL && list_2.last != NULL) ll_remove_back(&list_2);

	print_linked_list(&list);
	print_linked_list(&list_2);

	if (list.meta.assumed_size != 0 || list_2.meta.assumed_size != 0) return 1;

	ll_free(&list);
	ll_free(&list_2);
	return 0;
}
