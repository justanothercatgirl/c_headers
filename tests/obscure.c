#include <stdio.h>

#define CONTAINER_IMPLEMENTATION
#include "../include/container.h"

typedef char* packet_t;

int main(void) {
	packet_t *data = array_new(packet_t, 0);
	packet_t el_0 = malloc(128);
	memset(el_0, 42, 128);
	packet_t el_1 = malloc(128);
	memset(el_1, 69, 128);
	array_push(data, el_0);
	array_push(data, el_1);
	for (size_t i = 0; i < array_size(data); ++i) {
		for (size_t j = 0; j < 128; ++j) {
			printf("%.2X ", data[i][j]);
		}
		putchar('\n');
	}
	return (data[0][69] == 42 && data[1][42] == 69 ? 0 : 1);
	// OS will free it for me
}
