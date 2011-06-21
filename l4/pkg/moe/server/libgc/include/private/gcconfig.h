#pragma once

#include <l4/sys/types.h>
#include <l4/sys/kip.h>

extern l4_kernel_info_t *_current_kip;

#ifndef __L4
#  define __L4
#endif

#define CLOCK_TYPE l4_cpu_time_t
#define CLOCKS_PER_SEC 1000000
#define MS_TIME_DIFF(a,b) ((long)((a)-(b)))
#define GET_TIME(x) x = _current_kip->clock

#define OS_TYPE "L4"
#define MARK_BIT_PER_GRANULE
#undef MARK_BIT_PER_OBJ

#define NO_DEBUGGING
//#define USE_MUNMAP
#define USE_MARK_BITS
#define IF_CANCEL(x)

#define ALL_INTERIOR_POINTERS
#define JAVA_FINALIZATION
#define NO_CLOCK
#define NO_GETENV
#define DEFAULT_VDB

#if defined(GC_ADD_CALLER)
# undef GC_ADD_CALLER
#endif

#if defined(SAVE_CALL_CHAIN)
#undef SAVE_CALL_CHAIN
#endif

#define STACK_GROWS_DOWN

extern void *__libc_stack_end;
//#define STACKBOTTOM __libc_stack_end


#if defined(L4_ARCH_amd64)
# define CPP_WORDSZ 64
# define ALIGNMENT  8
#else
# define CPP_WORDSZ 32
# define ALIGNMENT  4
#endif

#define USE_MMAP

# ifndef STATIC
#   ifndef NO_DEBUGGING
#     define STATIC /* ignore to aid profiling and possibly debugging */
#   else
#     define STATIC static
#   endif
# endif

struct hblk *GC_get_mem(size_t bytes);
#define GET_MEM(bytes) GC_get_mem(bytes)

#if defined(FIXUP_POINTER)
# define NEED_FIXUP_POINTER 1
#else
# define NEED_FIXUP_POINTER 0
# define FIXUP_POINTER(p)
#endif

#ifndef PREFETCH
# define PREFETCH(x)
# define NO_PREFETCH
#endif

#ifndef PREFETCH_FOR_WRITE
# define PREFETCH_FOR_WRITE(x)
# define NO_PREFETCH_FOR_WRITE
#endif

# ifndef CACHE_LINE_SIZE
#   define CACHE_LINE_SIZE 32   /* Wild guess   */
# endif



# ifndef CLEAR_DOUBLE
#   define CLEAR_DOUBLE(x) \
                ((word*)x)[0] = 0; \
                ((word*)x)[1] = 0;
# endif /* CLEAR_DOUBLE */

