#ifndef _I386_TYPES_H
#define _I386_TYPES_H

//typedef signed char __s8;
//typedef unsigned char __u8;

typedef signed short __s16;
typedef unsigned short __u16;

typedef signed int __s32;
typedef unsigned int __u32;

//typedef signed long long __s64;
//typedef unsigned long long __u64;

#if defined (ARCH_x86)
#define BITS_PER_LONG 32
#elif defined (ARCH_amd64)
#define BITS_PER_LONG 64
#elif defined (ARCH_arm)
#define BITS_PER_LONG 32
#elif defined (ARCH_ppc32)
#define BITS_PER_LONG 32
#elif defined (ARCH_sparc)
#define BITS_PER_LONG 32
#else
#define Add this arch here
#endif

//typedef signed char s8;
typedef unsigned char u8;

//typedef signed short s16;
//typedef unsigned short u16;

//typedef signed int s32;
typedef unsigned int u32;

//typedef signed long long s64;
typedef unsigned long long u64;

#endif

