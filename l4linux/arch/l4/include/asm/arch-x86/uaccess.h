#ifndef __ASM_L4__ARCH_I386__UACCESS_H__
#define __ASM_L4__ARCH_I386__UACCESS_H__
/*
 * User space memory access functions
 */
#include <linux/errno.h>
#include <linux/compiler.h>
#include <linux/thread_info.h>
#include <linux/string.h>
#include <asm/asm.h>
#include <asm/page.h>

#define VERIFY_READ 0
#define VERIFY_WRITE 1

/*
 * The fs value determines whether argument validity checking should be
 * performed or not.  If get_fs() == USER_DS, checking is performed, with
 * get_fs() == KERNEL_DS, checking is bypassed.
 *
 * For historical reasons, these macros are grossly misnamed.
 */

#define MAKE_MM_SEG(s)	((mm_segment_t) { (s) })

#define KERNEL_DS	MAKE_MM_SEG(-1UL)
#define USER_DS 	MAKE_MM_SEG(TASK_SIZE_MAX)

#define get_ds()	(KERNEL_DS)
#define get_fs()	(current_thread_info()->addr_limit)
#define set_fs(x)	(current_thread_info()->addr_limit = (x))

#define segment_eq(a, b)	((a).seg == (b).seg)

#if 0
#define __addr_ok(addr)					\
	((unsigned long __force)(addr) <		\
	 (current_thread_info()->addr_limit.seg))

/*
 * Test whether a block of memory is a valid user space address.
 * Returns 0 if the range is valid, nonzero otherwise.
 *
 * This is equivalent to the following test:
 * (u33)addr + (u33)size > (u33)current->addr_limit.seg (u65 for x86_64)
 *
 * This needs 33-bit (65-bit for x86_64) arithmetic. We have a carry...
 */

#define __range_not_ok(addr, size)					\
({									\
	unsigned long flag, roksum;					\
	__chk_user_ptr(addr);						\
	asm("add %3,%1 ; sbb %0,%0 ; cmp %1,%4 ; sbb $0,%0"		\
	    : "=&r" (flag), "=r" (roksum)				\
	    : "1" (addr), "g" ((long)(size)),				\
	      "rm" (current_thread_info()->addr_limit.seg));		\
	flag;								\
})
#endif

/**
 * access_ok: - Checks if a user space pointer is valid
 * @type: Type of access: %VERIFY_READ or %VERIFY_WRITE.  Note that
 *        %VERIFY_WRITE is a superset of %VERIFY_READ - if it is safe
 *        to write to a block, it is always safe to read from it.
 * @addr: User space pointer to start of block to check
 * @size: Size of block to check
 *
 * Context: User context only.  This function may sleep.
 *
 * Checks if a pointer to a block of memory in user space is valid.
 *
 * Returns true (nonzero) if the memory block may be valid, false (zero)
 * if it is definitely invalid.
 *
 * Note that, depending on architecture, this function probably just
 * checks that the pointer is in the user space range - after calling
 * this function, memory access functions may still return -EFAULT.
 */
//#define access_ok(type, addr, size) (likely(__range_not_ok(addr, size) == 0))
#define access_ok(type, addr, size) ((void)(addr), (void)(size), 1)

/*
 * The exception table consists of pairs of addresses: the first is the
 * address of an instruction that is allowed to fault, and the second is
 * the address at which the program should continue.  No registers are
 * modified, so it is entirely up to the continuation code to figure out
 * what to do.
 *
 * All the routines below use bits of fixup code that are out of line
 * with the main instruction path.  This means when everything is well,
 * we don't even have to jump over them.  Further, they do not intrude
 * on our cache or tlb entries.
 */

struct exception_table_entry {
	unsigned long insn, fixup;
};

extern int fixup_exception(struct pt_regs *regs);

/*
 * These are the main single-value transfer routines.  They automatically
 * use the right size if we just have the right pointer type.
 *
 * This gets kind of ugly. We want to return _two_ values in "get_user()"
 * and yet we don't want to do any pointers, because that is too much
 * of a performance impact. Thus we have a few rather ugly macros here,
 * and hide all the ugliness from the user.
 *
 * The "__xxx" versions of the user access functions are versions that
 * do not verify the address space, that must have been done previously
 * with a separate "access_ok()" call (this is used when we do multiple
 * accesses to the same area of user memory).
 */

extern long __get_user_1(unsigned char      *val, const void *address);
extern long __get_user_2(unsigned short     *val, const void *address);
extern long __get_user_4(unsigned int       *val, const void *address);
extern long __get_user_8(unsigned long long *val, const void *address);
extern long __get_user_bad(void);

/* Careful: we have to cast the result to the type of the pointer
 * for sign reasons */

/**
 * get_user: - Get a simple variable from user space.
 * @x:   Variable to store result.
 * @ptr: Source address, in user space.
 *
 * Context: User context only.  This function may sleep.
 *
 * This macro copies a single simple variable from user space to kernel
 * space.  It supports simple types like char and int, but not larger
 * data types like structures or arrays.
 *
 * @ptr must have pointer-to-simple-variable type, and the result of
 * dereferencing @ptr must be assignable to @x without a cast.
 *
 * Returns zero on success, or -EFAULT on error.
 * On error, the variable @x is set to zero.
 */
#define get_user(x,ptr)							\
({	int __ret_gu;							\
 	unsigned long __val_gu;						\
	__chk_user_ptr(ptr);						\
	switch(sizeof (*(ptr))) {					\
	case 1:  __ret_gu = __get_user_1((unsigned char      *)&__val_gu,ptr); break;		\
	case 2:  __ret_gu = __get_user_2((unsigned short     *)&__val_gu,ptr); break;		\
	case 4:  __ret_gu = __get_user_4((unsigned int       *)&__val_gu,ptr); break;		\
	case 8:  __ret_gu = __get_user_8((unsigned long long *)&__val_gu,ptr); break;		\
	default: __ret_gu = __get_user_bad(); break;		\
	}								\
	(x) = (__typeof__(*(ptr)))__val_gu;				\
	__ret_gu;							\
})

#define __get_user_size_ex(x, ptr, size)				\
do {									\
	__chk_user_ptr(ptr);						\
	switch(size) {							\
	case 1:  __get_user_1((unsigned char      *)&x,ptr); break;		\
	case 2:  __get_user_2((unsigned short     *)&x,ptr); break;		\
	case 4:  __get_user_4((unsigned int       *)&x,ptr); break;		\
	case 8:  __get_user_8((unsigned long long *)&x,ptr); break;		\
	default: __get_user_bad(); break;			\
	}								\
} while (0)



extern long __put_user_1(unsigned char 	    val, const void *address);
extern long __put_user_2(unsigned short     val, const void *address);
extern long __put_user_4(unsigned int       val, const void *address);
extern long __put_user_8(unsigned long long val, const void *address);
extern long __put_user_bad(void);



/**
 * put_user: - Write a simple value into user space.
 * @x:   Value to copy to user space.
 * @ptr: Destination address, in user space.
 *
 * Context: User context only.  This function may sleep.
 *
 * This macro copies a single simple value from kernel space to user
 * space.  It supports simple types like char and int, but not larger
 * data types like structures or arrays.
 *
 * @ptr must have pointer-to-simple-variable type, and @x must be assignable
 * to the result of dereferencing @ptr.
 *
 * Caller must check the pointer with access_ok() before calling this
 * function.
 *
 * Returns zero on success, or -EFAULT on error.
 */
#if 0
#define put_user(x,ptr)							\
  __put_user_check((__typeof__(*(ptr)))(x),(ptr),sizeof(*(ptr)))
#endif

/*
 * To avoid warnings from the compiler (in case we want to cast a pointer to
 * u64) we use this dirty asm wrapper trick.
 */
#ifdef CONFIG_X86_32
#define __put_user_8_wrap(x, addr, retval)				\
	({ unsigned long dummy;                                         \
	__asm__ __volatile__ (						\
		"call __put_user_8	\n\t"				\
		: "=a" (retval), "=d" (dummy), "=c" (dummy)		\
		: "A" (x), "c" (addr));                                 \
	 })
#else
#define __put_user_8_wrap(x, addr, retval)				\
        retval = __put_user_8((u64)x,addr);
#endif

#define put_user_macro(x,ptr)						\
	({								\
	int __ret_pu;							\
	__typeof__(*(ptr)) __pu_val;					\
	__chk_user_ptr(ptr);						\
	__pu_val = (__typeof__(*(ptr)))(x);							\
	switch (sizeof(*(ptr))) {					\
	case 1:  __ret_pu = __put_user_1(( u8)((unsigned long)__pu_val),ptr); break; \
	case 2:  __ret_pu = __put_user_2((u16)((unsigned long)__pu_val),ptr); break; \
	case 4:  __ret_pu = __put_user_4((u32)(__pu_val),ptr); break; \
	case 8:  __put_user_8_wrap((__pu_val), ptr, __ret_pu); break; \
	default: __ret_pu = __put_user_bad(); break;			\
	}								\
	__ret_pu;							\
	})
	

#define __get_user get_user
#define __put_user(x,ptr) put_user_macro((__typeof__(*(ptr)))(x), (ptr))
#define put_user put_user_macro


struct __large_struct { unsigned long buf[100]; };
#define __m(x) (*(struct __large_struct __user *)(x))


/*
 * uaccess_try and catch
 */
#define uaccess_try	do {						\
	int prev_err = current_thread_info()->uaccess_err;		\
	current_thread_info()->uaccess_err = 0;				\
	barrier();

#define uaccess_catch(err)						\
	(err) |= current_thread_info()->uaccess_err;			\
	current_thread_info()->uaccess_err = prev_err;			\
} while (0)


#define __copy_to_user copy_to_user
#define __copy_from_user copy_from_user
#define __copy_to_user_inatomic copy_to_user
#define __copy_from_user_inatomic __copy_from_user

unsigned long __must_check __copy_to_user_ll
		(void __user *to, const void *from, unsigned long n);
unsigned long __must_check __copy_from_user_ll
		(void *to, const void __user *from, unsigned long n);

/**
 * Here we special-case 1, 2 and 4-byte copy_*_user invocations.  On a fault
 * we return the initial request size (1, 2 or 4), as copy_*_user should do.
 * If a store crosses a page boundary and gets a fault, the x86 will not write
 * anything, so this is accurate.
 */

unsigned long __must_check copy_to_user(void __user *to,
					const void *from, unsigned long n);
unsigned long __must_check copy_from_user(void *to,
					  const void __user *from,
					  unsigned long n);
long __must_check strncpy_from_user(char *dst, const char __user *src,
				    long count);
long __must_check __strncpy_from_user(char *dst,
				      const char __user *src, long count);

/**
 * strlen_user: - Get the size of a string in user space.
 * @str: The string to measure.
 *
 * Context: User context only.  This function may sleep.
 *
 * Get the size of a NUL-terminated string in user space.
 *
 * Returns the size of the string INCLUDING the terminating NUL.
 * On exception, returns 0.
 *
 * If there is a limit on the length of a valid string, you may wish to
 * consider using strnlen_user() instead.
 */
#define strlen_user(str) strnlen_user(str, LONG_MAX)

long strnlen_user(const char __user *str, long n);
unsigned long __must_check clear_user(void __user *mem, unsigned long len);
unsigned long __must_check __clear_user(void __user *mem, unsigned long len);

/*
 * {get|put}_user_try and catch
 *
 * get_user_try {
 *	get_user_ex(...);
 * } get_user_catch(err)
 */
#define get_user_try		uaccess_try
#define get_user_catch(err)	uaccess_catch(err)

#define get_user_ex(x, ptr)	do {					\
	unsigned long __gue_val;					\
	__get_user_size_ex((__gue_val), (ptr), (sizeof(*(ptr))));	\
	(x) = (__force __typeof__(*(ptr)))__gue_val;			\
} while (0)

//l4/#ifdef CONFIG_X86_WP_WORKS_OK
#ifdef TAKE_THE_OTHER_FOR_L4

#define put_user_try		uaccess_try
#define put_user_catch(err)	uaccess_catch(err)

#define put_user_ex(x, ptr)						\
	__put_user_size_ex((__typeof__(*(ptr)))(x), (ptr), sizeof(*(ptr)))

#else /* !CONFIG_X86_WP_WORKS_OK */

#define put_user_try		do {		\
	int __uaccess_err = 0;

#define put_user_catch(err)			\
	(err) |= __uaccess_err;			\
} while (0)

#define put_user_ex(x, ptr)	do {		\
	__uaccess_err |= __put_user(x, ptr);	\
} while (0)

#endif /* CONFIG_X86_WP_WORKS_OK */

/*
 * movsl can be slow when source and dest are not both 8-byte aligned
 */
#ifdef CONFIG_X86_INTEL_USERCOPY
extern struct movsl_mask {
	int mask;
} ____cacheline_aligned_in_smp movsl_mask;
#endif


#endif

