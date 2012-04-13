#include <asm/page.h>
#include <asm/generic/memory.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <linux/errno.h>
#include <linux/module.h>

#include <asm/api/macros.h>

//#define DEBUG_MEMCPY_TOFS
//#define DEBUG_MEMCPY_FROMFS
//#define DEBUG_MEMCPY_KERNEL
//#define DEBUG_KDEBUG_EFAULT
////#define DEBUG_LOG_EFAULT

// from user_copy_32.c
#ifdef CONFIG_X86_INTEL_USERCOPY
/*
 * Alignment at which movsl is preferred for bulk memory copies.
 */
struct movsl_mask movsl_mask __read_mostly;
#endif

#ifdef DEBUG_LOG_EFAULT
static void log_efault(const char *str, const void *user_addr,
		       const void *kernel_addr, unsigned long size)
{
	pte_t *ptep = lookup_pte((pgd_t *)current->mm->pgd,
				 (unsigned long)user_addr);

	printk("%s returning efault, \n"
	       "  user_addr: %p, kernel_addr: %p, size: %08lx\n"
	       "  task: %s (%p) " PRINTF_L4TASK_FORM
	       ", pdir: %p, ptep: %p, pte: %lx\n",
	       str, user_addr, kernel_addr, size,
	       current->comm, current,
	       PRINTF_L4TASK_ARG(current->thread.user_thread_id),
	       current->mm->pgd, ptep, ptep ? pte_val(*ptep) : 0);
#ifdef DEBUG_KDEBUG_EFAULT
	enter_kdebug("log_efault");
#endif
}
#else
#define log_efault(str, to, from, size)
#endif

static inline int __copy_to_user_page(void *to, const void *from,
				      unsigned long n)
{
	unsigned long page, offset;

#ifdef DEBUG_MEMCPY_TOFS
	printk("__copy_to_user_page to: %p, from: %p, len: %08lx\n",
	       to, from, n);
#endif
	if ((page = parse_ptabs_write((unsigned long)to, &offset)) != -EFAULT) {
#ifdef DEBUG_MEMCPY_TOFS
		printk("    __copy_to_user_page writing to: %08lx\n",
		       (page + offset));
#endif
		memcpy((void *)(page + offset), from, n);
		return 0;
	}
	log_efault("__copy_to_user_page", to, from, n);
	return -EFAULT;
}

unsigned long __must_check
copy_to_user(void __user *to, const void *from, unsigned long n)
{
	unsigned long copy_size = (unsigned long)to & ~PAGE_MASK;

#ifdef DEBUG_MEMCPY_TOFS
	printk("copy_to_user called from: %08lx to: %p, "
	       "from: %p, len: %08lx\n",
	       *((unsigned long *)&to - 1), to, from, n);
#endif

	/* kernel access */
	if (segment_eq(get_fs(), KERNEL_DS)) {
		memcpy(to, from, n);
		return 0;
	}

	if (copy_size) {
		copy_size = min(PAGE_SIZE - copy_size, n);
		if (__copy_to_user_page(to, from, copy_size) == -EFAULT)
			return n;
		n -= copy_size;
	}

	while (n) {
		from +=copy_size;
		to += copy_size;
		copy_size = min(PAGE_SIZE, n);
		if (__copy_to_user_page(to, from, copy_size) == -EFAULT)
			return n;
		n -= copy_size;
	}
	return 0;
}
EXPORT_SYMBOL(copy_to_user);

static inline int __copy_from_user_page(void *to, const void *from,
					unsigned long n)
{
	unsigned long page, offset;

#ifdef DEBUG_MEMCPY_FROMFS
	printk("%s to: %p, from: %p, len: %08lx\n", __func__, to, from, n);
#endif
	if ((page = parse_ptabs_read((unsigned long)from, &offset)) != -EFAULT) {
#ifdef DEBUG_MEMCPY_FROMFS
		printk("  %s reading from: %08lx\n",
		       __func__, (page + offset));
#endif
		memcpy(to, (void *)(page + offset), n);
		return 0;
	}
	log_efault(__func__, from, to, n);
	return -EFAULT;
}

unsigned long
copy_from_user(void *to, const void __user *from, unsigned long n)
{
	unsigned long copy_size = (unsigned long)from & ~PAGE_MASK;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		memcpy(to, from, n);
		return 0;
	}

#ifdef DEBUG_MEMCPY_FROMFS
	printk("copy_from_user called from: %08lx "
	       "to: %p, from: %p, len: %08lx\n",
	       *((unsigned long *)&to - 1), to, from, n);
#endif
	if (copy_size) {
		copy_size = min(PAGE_SIZE - copy_size, n);
		if (__copy_from_user_page(to, from, copy_size) == -EFAULT) {
			memset(to, 0, n);
			return n;
		}
		n -= copy_size;
	}
	while (n) {
		from +=copy_size;
		to += copy_size;
		copy_size = min(PAGE_SIZE, n);
		if (__copy_from_user_page(to, from, copy_size) == -EFAULT) {
			memset(to, 0, n);
			return n;
		}
		n -= copy_size;
	}
	return 0;
}
EXPORT_SYMBOL(copy_from_user);

#ifdef ARCH_x86
unsigned long
__copy_from_user_ll_nozero(void *to, const void __user *from, unsigned long n)
{
	return copy_from_user(to, from, n);
}
EXPORT_SYMBOL(__copy_from_user_ll_nozero);
#endif

static inline int __clear_user_page(void * address, unsigned long n)
{
	unsigned long page, offset;

#ifdef DEBUG_MEMCPY_TOFS
	printk("%s: %p, len: %08lx\n", __func__, address, n);
#endif
	page = parse_ptabs_write((unsigned long)address, &offset);
	if (page != -EFAULT) {
#ifdef DEBUG_MEMCPY_TOFS
		printk("    writing to: %08lx\n", (page + offset));
#endif
		memset((void *)(page + offset), 0, n);
		return 0;
	}
	log_efault(__func__, address, 0, n);
	return -EFAULT;
}
unsigned long clear_user(void *address, unsigned long n)
{
	unsigned long clear_size = (unsigned long)address & ~PAGE_MASK;

#ifdef DEBUG_MEMCPY_TOFS
	printk("%s called from: %08lx to: %p, len: %08lx\n",
	       __func__, *((unsigned long *)&address - 1), address, n);
#endif

	if (segment_eq(get_fs(), KERNEL_DS)) {
		memset(address, 0, n);
		return 0;
	}

	if (clear_size) {
		clear_size = min(PAGE_SIZE - clear_size, n);
		if (__clear_user_page(address, clear_size) == -EFAULT)
			return n;
		n -= clear_size;
	}
	while (n) {
		address += clear_size;
		clear_size = min(PAGE_SIZE, n);
		if (__clear_user_page(address, clear_size) == -EFAULT)
			return n;
		n -= clear_size;
	}
	return 0;
}
EXPORT_SYMBOL(clear_user);

unsigned long
__clear_user(void __user *to, unsigned long n)
{
	return clear_user(to, n);
}
EXPORT_SYMBOL(__clear_user);

/*
 * Copy a null terminated string from userspace.
 */

#ifdef CONFIG_X86_32
#define __do_strncpy_from_user_page(dst,src,count)			   \
do {									   \
	int __d0, __d1, __d2;						   \
	__asm__ __volatile__(						   \
		"	testl %0,%0\n"					   \
		"	jz 1f\n"					   \
		"0:	lodsb\n"					   \
		"	stosb\n"					   \
		"	testb %%al,%%al\n"				   \
		"	jz 1f\n"					   \
		"	decl %0\n"					   \
		"	jnz 0b\n"					   \
		"1:\n"							   \
		: "=c"(count), "=&a" (__d0), "=&S" (__d1),		   \
		  "=&D" (__d2)						   \
		: "0"(count), "2"(src), "3"(dst)			   \
		: "memory");						   \
} while (0)
#else
#define __do_strncpy_from_user_page(dst, src, count)                    \
	do {								\
		char *_src = src;					\
		while (count) {						\
			*dst = *_src;					\
			if (!*_src)					\
				break;					\
			dst++;						\
			_src++;						\
			count--;					\
		}							\
	} while (0)
#endif

static inline int __strncpy_from_user_page(char * to, const char * from,
					   unsigned long n)
{
	unsigned long page, offset;

#ifdef DEBUG_MEMCPY_FROMFS
	printk("%s: to: %p, from: %p, len: %08lx\n",
	       __func__, to, from, n);
#endif
	page = parse_ptabs_read((unsigned long)from, &offset);
	if (page != -EFAULT) {
#ifdef DEBUG_MEMCPY_FROMFS
		printk("    %s reading from: %08lx len: 0x%lx\n",
		       __func__, (page + offset), n);
#endif
		/* after finishing the copy operation count is either
		 * - zero: max number of bytes copied or
		 * - non zero: end of string reached, n containing the
		 *             number of remaining bytes
		 */
		__do_strncpy_from_user_page(to, (char *)(page + offset), n);
		return n;
	}
	log_efault(__func__, from, to, n);
	return -EFAULT;
}

/* strncpy returns the number of bytes copied. We calculate the number
 * simply by substracting the number of bytes remaining from the
 * maximal length. The number of bytes remaining is (n + res) with n
 * beeing the number of bytes to copy from the next pages and res the
 * number of remaining bytes after reaching the '\0' */

long strncpy_from_user(char *dst, const char *src, long count)
{
	unsigned long copy_size = (unsigned long)src & ~PAGE_MASK;
	long res;
	unsigned long n = count;

#ifdef DEBUG_MEMCPY_FROMFS
	printk("strncpy_from_user called from: %08lx "
	       "to: %p, from: %p, len: 0x%lx (copy_size: 0x%lx)\n",
	       *((unsigned long *)&dst - 1), dst, src, n, copy_size);
#endif
	if (segment_eq(get_fs(), KERNEL_DS)) {
		/* strncpy the data but deliver back the bytes copied */
		long c = 0;
		while (c++ < count && (*dst++ = *src++) != '\0')
			/* nothing */;
		return c;
	}

	if (copy_size) {
		copy_size = min(PAGE_SIZE - copy_size, n);
		res = __strncpy_from_user_page(dst, src, copy_size);
		n -= copy_size;
		if (res == -EFAULT) {
			return -EFAULT;
		}
		else if (res)
			return count - (n + res);
	}
	while (n) {
		src += copy_size;
		dst += copy_size;
		copy_size = min(PAGE_SIZE, n);
		n -= copy_size;
		res = __strncpy_from_user_page(dst, src, copy_size);
		if (res == -EFAULT) {
			return -EFAULT;
		}
		else if (res)
			return count - (n + res);
	}
	return count;
}
EXPORT_SYMBOL(strncpy_from_user);

static inline int __strnlen_from_user_page(const char *from,
					   unsigned long n, unsigned long *len)
{
	unsigned long page, offset;

#ifdef DEBUG_MEMCPY_FROMFS
	printk("%s from: %p, len: %08lx\n", __func__, from, n);
#endif
	page = parse_ptabs_read((unsigned long)from, &offset);
	if (page != -EFAULT) {
		int end;
#ifdef ARCH_x86
		int res;
#endif
#ifdef DEBUG_MEMCPY_FROMFS
		printk("    %s reading from: %08lx\n",
		       __func__, (page + offset));
#endif
#ifdef ARCH_x86
		__asm__ __volatile__(
			"0:	repne; scasb\n"
			"	sete %b1\n"
			:"=c" (res),  "=a" (end)
			:"0" (n), "1" (0), "D" ((page + offset))
			);
		/* after finishing the search operation 'end' is either
		 * - zero: max number of bytes searched
		 * - non zero: end of string reached, res containing
		 *      the number of remaining bytes
		 */
		*len += n - res;
#endif
#ifdef ARCH_arm
		{
			unsigned long i, p = page + offset;
			end = 0;
			for (i = 0; i < n; i++) {
				if (*(unsigned char *)(p + i) == 0) {
					end = 1;
					i++; /* Include the zero */
					break;
				}
			}
			*len += i;
		}
#endif
		return end;
	}
	log_efault(__func__, from, 0, n);
	return -EFAULT;
}

/* strnlen returns the number of bytes in a string. We calculate the number
 * simply by substracting the number of bytes remaining from the
 * maximal length. The number of bytes remaining is (n + res) with n
 * being the number of bytes to copy from the next pages and res the
 * number of remaining bytes after reaching the '\0' */
long strnlen_user(const char *src, long n)
{
	unsigned long search_size = PAGE_SIZE - ((unsigned long)src & ~PAGE_MASK);
	int res;
	unsigned long len=0;

#ifdef DEBUG_MEMCPY_FROMFS
	printk("strnlen_user called from: %08lx, from: %p, %ld\n",
	       *((unsigned long *)&src - 1), src, n);
#endif

	if (segment_eq(get_fs(), KERNEL_DS)) {
		len = strnlen(src, n);
#ifdef DEBUG_MEMCPY_KERNEL
		printk("kernel strnlen_user %p, %ld = %ld\n", src, n, len);
#endif
		return len + 1;
	}

	if (!search_size)
		search_size = PAGE_SIZE;
	if (search_size > n)
		search_size = n;

	while (n > 0) {
		res = __strnlen_from_user_page(src, search_size, &len);
		if (res == -EFAULT)
			return 0; /* EFAULT */
		else if (res)
			return len;

		src += search_size;
		n   -= search_size;
		search_size = PAGE_SIZE;
	}

	return 0; /* EFAULT */
}
EXPORT_SYMBOL(strnlen_user);
