#ifndef JUSTANOTHERCATGITL_RSTYPES_H
#define JUSTANOTHERCATGITL_RSTYPES_H

#include <stdint.h>

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;

typedef float f32;
typedef double f64;


#if defined(__SIZEOF_LONG_DOUBLE__) && __SIZEOF_LONG_DOUBLE__ == 128/8
typedef long double f128;
#else // __SIZEOF_LONG_DOUBLE__
	#if defined(__clang__) || defined(__GNUC__)
	typedef __float128 f128;
	#else 
	#define f128 "128-bit floats are not supported on this platform/compiler"
	#endif // __clang__ || __GNUC__
#endif // __SIZEOF_LONG_DOUBLE__

#ifdef RS_TYPES_USE_128
	#if defined(__clang__) || defined(__GNUC__)
	typedef __int128 i128;
	typedef unsigned __int128 u128;
	#else // __clang__ || __GNUC__
	#define i128 "128-bit integers are not supported in this compiler"
	#define u128 "128-bit integers are not supported in this compiler"
	#endif // __clang__ || __GNUC__
#endif // RS_TYPES_USE_128

#endif // JUSTANOTHERCATGITL_RSTYPES_H
