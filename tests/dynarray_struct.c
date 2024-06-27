
#define CONTAINER_IMPLEMENTATION
#include "../include/container.h"

#include <stdio.h>

struct point {
	float x, y, z, w;
};

void print_point(struct point* p) {
	printf("{ .x=%.2f, .y=%.2f, .z=%.2f, .w=%.2f };\n", p->x, p->y, p->z, p->w);
}

void print_array(struct point* p) {
	for (size_t i = 0; i < array_size(p); ++i) {
		printf("%zu: ", i);
		print_point(p+i);
	}
}

int point_abs(const void* a, const void* b) {
	struct point* c = (struct point*) a;
	struct point* d = (struct point*) b;
	float absa = c->x * c->x + c->y * c->y + c->z * c->z + c->w * c->w;
	float absb = d->x * d->x + d->y * d->y + d->z * d->z + d->w * d->w;
	return absa-absb;
}

int main(void)
{
	struct point *points = array_new(struct point, 4);
	for (size_t i = 0; i < array_size(points); ++i) points[i] = (struct point){.x = i, .y = 2 * i, .z = 4 * i, .w = 8 * i};
	array_push(points, ((struct point){.0f, 2.5f, 3.0, 4.0}) );
	array_insert(points, ((struct point){0}), 3);
	array_qsort(points, point_abs);
	print_array(points);
	array_free(points);
	return 0;
}
