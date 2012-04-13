#include <linux/module.h>

#include <asm/linkage.h>
#include <asm/api/macros.h>
#include <asm/uaccess.h>
#include <asm/generic/memory.h>

/* #define DEBUG_PUT_USER */
/* #define DEBUG_GET_USER */

/* #define LOG_EFAULT */
#ifdef LOG_EFAULT
#include <l4/sys/kdebug.h>
static void log_efault(const char *str, const void *address)
{
	pte_t *ptep = lookup_pte((pgd_t *)current->mm->pgd,
				 (unsigned long)address);

	printk("%s returning efault, address: %p, \n"
	       "  task: %s (%p, " PRINTF_L4TASK_FORM
	       "), pdir: %p, ptep: %p, pte: %lx\n",
	       str, address, current->comm, current,
	       PRINTF_L4TASK_ARG(current->thread.user_thread_id),
	       current->mm->pgd, ptep, ptep ? pte_val(*ptep) : 0);
	enter_kdebug("log_efault");
}
#else
#define log_efault(str, address)
#endif

long __get_user_1(unsigned char *val, const void __user *address)
{
	unsigned long page, offset;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		*val = *(unsigned char*)address;
		return 0;
	}

	page = parse_ptabs_read((unsigned long)address, &offset);
	if (page != -EFAULT) {
		*val = *(unsigned char *)(page + offset);
		return 0;
	}
	log_efault(__func__, address);
	return -EFAULT;
}
EXPORT_SYMBOL(__get_user_1);

long __get_user_2(unsigned short *val, const void __user *address)
{
	unsigned long page, offset;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		*val = *(unsigned short*)address;
		return 0;
	}

	if (((unsigned long)address & ~PAGE_MASK) != ~PAGE_MASK) {
		page = parse_ptabs_read((unsigned long)address, &offset);
		if (page != -EFAULT) {
			*val = *(unsigned short *)(page + offset);
			return 0;
		}
	} else {
		unsigned char low, high;
		if ((__get_user_1(&low,  address)     != -EFAULT) &&
		    (__get_user_1(&high, address + 1) != -EFAULT)) {
			*val = (unsigned short)low +
				((unsigned short)high << 8);
			return 0;
		}
	}
	log_efault(__func__, address);
	return -EFAULT;
}
EXPORT_SYMBOL(__get_user_2);

long __get_user_4(unsigned int *val, const void __user *address)
{
	unsigned long page, offset;
#ifdef DEBUG_GET_USER
	printk("%d (%s): get_user_long %p, tid=" PRINTF_L4TASK_FORM
	       " frm: %p\n",
	       current->pid, current->comm, address,
	       PRINTF_L4TASK_ARG(current->thread.user_thread_id),
	       __builtin_return_address(0));
#endif

	if (segment_eq(get_fs(), KERNEL_DS)) {
		*val = *(unsigned long*)address;
		return 0;
	}

	if (((unsigned long)address & ~PAGE_MASK)
			<= (PAGE_SIZE - sizeof(*val))) {
		page = parse_ptabs_read((unsigned long)address, &offset);
		if (page != -EFAULT) {
			*val = *(unsigned long *)(page + offset);
			return 0;
		}
	} else {
		switch ((unsigned long)address & 3) {
		case 1:
		case 3: {
			unsigned char first, third;
			unsigned short second;
			if ((__get_user_1(&first,  address)     != -EFAULT) &&
			    (__get_user_2(&second, address + 1) != -EFAULT) &&
			    (__get_user_1(&third,  address + 3) != -EFAULT)) {
				*val = first +
					((unsigned long)second << 8) +
					((unsigned long)third << 24);
				return 0;
			}
		}
		case 2: {
			unsigned short first, second;
			if ((__get_user_2(&first,  address)     != -EFAULT) &&
			    (__get_user_2(&second, address + 2) != -EFAULT)) {
				*val = first + ((unsigned long)second << 16);
				return 0;
			}
		}
		}
	}
	log_efault(__func__, address);
	return -EFAULT;
}
EXPORT_SYMBOL(__get_user_4);

long __get_user_8(unsigned long long *val, const void __user *address)
{
	unsigned long page, offset;

#ifdef DEBUG_GET_USER
	printk("%d (%s): get_user_8 %p, tid=" PRINTF_L4TASK_FORM
	       " frm: %p\n",
	       current->pid, current->comm, address,
	       PRINTF_L4TASK_ARG(current->thread.user_thread_id),
	       __builtin_return_address(0));
#endif

	if (segment_eq(get_fs(), KERNEL_DS)) {
		*val = *(unsigned long*)address;
		return 0;
	}

	if (((unsigned long)address & ~PAGE_MASK)
			<= (PAGE_SIZE - sizeof(*val))) {
		page = parse_ptabs_read((unsigned long)address, &offset);
		if (page != -EFAULT) {
			*val = *(unsigned long *)(page + offset);
			return 0;
		}
	} else {
		unsigned first, second;
		if ((__get_user_4(&first,  address)     != -EFAULT) &&
		    (__get_user_4(&second, address + 4) != -EFAULT)) {
			*val = first | ((unsigned long long)second << 32);
			return 0;
		}
	}
	log_efault(__func__, address);
	return -EFAULT;
}
EXPORT_SYMBOL(__get_user_8);

long __put_user_1(unsigned char val, const void __user *address)
{
	unsigned long page, offset;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		*(unsigned char*)address = val;
		return 0;
	}

	page = parse_ptabs_write((unsigned long)address, &offset);
	if (page != -EFAULT) {
		*(unsigned char *)(page + offset) = val;
		return 0;
	}
	log_efault(__func__, address);
	return -EFAULT;
}
EXPORT_SYMBOL(__put_user_1);

long __put_user_2(unsigned short val, const void __user *address)
{
	unsigned long page, offset;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		*(unsigned short*)address = val;
		return 0;
	}

	if (((unsigned long)address & ~PAGE_MASK) != ~PAGE_MASK) {
		page = parse_ptabs_write((unsigned long)address, &offset);
		if (page != -EFAULT) {
			*(unsigned short *)(page + offset) = val;
			return 0;
		}
	} else {
		if ((__put_user_1((unsigned char)val, address) != -EFAULT)
		    &&
		    (__put_user_1((unsigned char)(val >> 8), address+1)
			!= -EFAULT)) {
			return 0;
		}
	}
	log_efault(__func__, address);
	return -EFAULT;
}
EXPORT_SYMBOL(__put_user_2);

long __put_user_4(unsigned int val, const void __user *address)
{
	unsigned long page, offset;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		*(unsigned long*)address = val;
		return 0;
	}

	if (((unsigned long)address & ~PAGE_MASK) <= (PAGE_SIZE - sizeof(val))) {
		page = parse_ptabs_write((unsigned long)address, &offset);
		if (page != -EFAULT) {
			*(unsigned long *)(page + offset) = val;
			return 0;
		}
	} else {
		switch ((unsigned long)address &3) {
		case 1:
		case 3:
			{
			if ((__put_user_1((unsigned char)val,
					  address)     != -EFAULT) &&
			    (__put_user_2((unsigned short)(val >> 8),
					  address + 1) != -EFAULT) &&
			    (__put_user_1((unsigned char)(val >> 24),
					  address + 3) != -EFAULT))
				return 0;
		}
		case 2:
			{
			if ((__put_user_2((unsigned short)val,
					  address)     != -EFAULT) &&
			    (__put_user_2((unsigned short)(val >> 16),
					  address + 2) != -EFAULT))
				return 0;
		}
		}
	}
	log_efault(__func__, address);
	return -EFAULT;
}
EXPORT_SYMBOL(__put_user_4);

long __put_user_8(unsigned long long val, const void __user *address)
{
	unsigned long page, offset;

	if (segment_eq(get_fs(), KERNEL_DS)) {
		*(unsigned long long*)address = val;
		return 0;
	}

	if (((unsigned long)address & ~PAGE_MASK) <= (PAGE_SIZE - sizeof(val))) {
		page = parse_ptabs_write((unsigned long)address, &offset);
		if (page != -EFAULT) {
			*(unsigned long long*)(page + offset) = val;
			return 0;
		}
	} else {
		if (__put_user_4((unsigned long)val,         address) != -EFAULT &&
		    __put_user_4((unsigned long)(val >> 32), address + 4) != -EFAULT)
			return 0;
	}
	log_efault(__func__, address);
	return -EFAULT;
}
EXPORT_SYMBOL(__put_user_8);
