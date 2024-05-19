#ifndef JUSTANOTHERCATGIRL_HEADERS_UTILITY
#define JUSTANOTHERCATGIRL_HEADERS_UTILITY

#define stringify(val) _stringify_helper(val)
#define _stringify_helper(val) #val

#define struct_member_size(type, member) sizeof(( (type*)0 )->member)
typedef unsigned char byte;
#define upper_2_power_32(number) (32 - __builtin_clz(number))
#define upper_2_power_64(number) (64 - __builtin_clzl(number))

typedef int(*qsort_cmp_t)(const void*, const void*);
#define get_qsort_cmp(type) __qsort_cmps[sizeof(type)]
extern const qsort_cmp_t __qsort_cmps[64];

#ifdef UTILITY_EXPOSE_HELPERS
	#define _UTILITY_STATIC 
	int __default_char_cmp(const void* a, const void* b);
	int __default_short_cmp(const void* a, const void* b);
	int __default_int_cmp(const void* a, const void* b);
	int __default_long_cmp(const void* a, const void* b);
	int __default_long_long_cmp(const void* a, const void* b);
#else 
	#define _UTILITY_STATIC static
#endif // UTILITY_EXPOSE_HELPERS


#ifdef UTILITY_IMPLEMENTATION

_UTILITY_STATIC int __default_char_cmp(const void* a, const void* b) {
	char x = *(char*)a, y = *(char*)b;
	if (x < y) return -1;
	if (x > y) return 1;
	return 0;
}
_UTILITY_STATIC int __default_short_cmp(const void* a, const void* b) {
	short x = *(short*)a, y = *(short*)b;
	if (x < y) return -1;
	if (x > y) return 1;
	return 0;
}
_UTILITY_STATIC int __default_int_cmp(const void* a, const void* b) {
	int x = *(int*)a, y = *(int*)b;
	if (x < y) return -1;
	if (x > y) return 1;
	return 0;
}
_UTILITY_STATIC  int __default_long_long_cmp(const void* a, const void* b) {
	long long x = *(long long*)a, y = *(long long*)b;
	if (x < y) return -1;
	if (x > y) return 1;
	return 0;
}
_UTILITY_STATIC int __default_long_cmp(const void* a, const void* b) {
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


#endif // UTILITY_IMPLEMENTATION


#endif //JUSTANOTHERCATGIRL_HEADERS_UTILITY
