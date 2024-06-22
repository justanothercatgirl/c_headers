#define CONTAINER_IMPLEMENTATION
#include "../include/container.h"
#include <stdio.h>

void print_linked_list(struct linked_list* list) {
	int i = 0;
	printf("linked list at %p, size = %zu\n", list, list->meta.assumed_size);
	for (struct linked_list_node* current = list->first; current != NULL; current = current->next) {
		printf("[%i] = {%i, %p}\n", i, *(int*)current->data, current->next);
		++i;
	}
}

int main() {
	struct linked_list list = ll_create(sizeof(int));
	int a = 10;
	ll_insert_at(&list, &a, 0);
	a = 11;
	ll_insert_at(&list, &a, 0);
	a = 14;
	ll_insert_at(&list, &a, 2);
	ll_insert_at(&list, &a, 3);
	ll_insert_at(&list, &a, 1);

	print_linked_list(&list);
	ll_remove_back(&list);
	ll_remove_front(&list);
	print_linked_list(&list);
	ll_remove_at(&list, 1);
	ll_remove_at(&list, 0);
	ll_remove_at(&list, 0);
	print_linked_list(&list);
	ll_free(&list);
	return 0;
}
