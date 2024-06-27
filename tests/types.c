#include <stdio.h>
#define RS_TYPES_USE_128
#include "../include/rstypes.h"

i32 main(void) {
	u8 a = 215;
	i8 b = -127;
	u16 c = 65535;
	i16 d = -32000;
	u32 e = 0xFFFFFFF;
	i32 f = -1;
	u64 g = 0xFAFAFAFAFAFAFAFA;
	i64 h = -4000000000000000000;
	u128 i = 0xFFFFFFFFFFFFFFFF;
	i128 j = 0;
	i <<= 64; i |= 0xFFFFFFFFFFFFFFFF;
	j = i;

	f32 k = 5e-23;
	f64 l = -2e300;
	f128 m = 3e300;
	m *= 1e100;

	printf( "a: %hhu, b: %hhi\n"
		"c: %hu, d: %hi\n"
		"e: %u, f: %d\n"
		"g: %lu, h: %li\n",
		a, b, c, d, e, f, g, h);
	printf("i: %lX%.16lX, jL %lx%.16lx\n", 
		(unsigned long) (i >> 64), (unsigned long) i, 
		(long) (j >> 64), (long) j );
	
	printf("k: %.2E\nl: %.2lE\nm: %.2LE\n", k, l, m);
	
	return (j == -1 && i+1 == 0 ? 0 : 1);
}
