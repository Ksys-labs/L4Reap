/*
 *  linux/arch/arm/kernel/sys_arm.c
 *
 *  Copyright (C) People who wrote linux/arch/i386/kernel/sys_i386.c
 *  Copyright (C) 1995, 1996 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  This file contains various random system calls that
 *  have a non-standard calling sequence on the Linux/arm
 *  platform.
 */
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/stat.h>
#include <linux/syscalls.h>
#include <linux/mman.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/ipc.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include <l4/sys/utcb.h>
#include <l4/sys/kdebug.h>

#include <asm/api/macros.h>
#include <asm/generic/dispatch.h>
#include <asm/generic/process.h>
#include <asm/generic/task.h>
#include <asm/generic/stack_id.h>
#include <asm/l4lxapi/task.h>


/* Fork a new task - this creates a new program thread.
 * This is called indirectly via a small wrapper
 */
asmlinkage int sys_fork(void)
{
#ifdef CONFIG_MMU
	struct pt_regs *regs = L4X_THREAD_REGSP(&current->thread);
	return do_fork(SIGCHLD, regs->ARM_sp, regs, COPY_THREAD_STACK_SIZE___FLAG_USER, NULL, NULL);
#else
	/* can not support in nommu mode */
	return(-EINVAL);
#endif
}

/* Clone a task - this clones the calling program thread.
 * This is called indirectly via a small wrapper
 */
asmlinkage int sys_clone(unsigned long clone_flags, unsigned long newsp,
			 int __user *parent_tidptr, int tls_val,
			 int __user *child_tidptr)
{
	struct pt_regs *regs = L4X_THREAD_REGSP(&current->thread);

	if (!newsp)
		newsp = regs->ARM_sp;

	return do_fork(clone_flags, newsp, regs, COPY_THREAD_STACK_SIZE___FLAG_USER, parent_tidptr, child_tidptr);
}

asmlinkage int sys_vfork(void)
{
	struct pt_regs *regs = L4X_THREAD_REGSP(&current->thread);
	return do_fork(CLONE_VFORK | CLONE_VM | SIGCHLD, regs->ARM_sp, regs, COPY_THREAD_STACK_SIZE___FLAG_USER, NULL, NULL);
}

/* sys_execve() executes a new program.
 * This is called indirectly via a small wrapper
 */
asmlinkage int sys_execve(const char __user *filenamei,
			  const char __user *const __user *argv,
			  const char __user *const __user *envp, struct pt_regs *regs)
{
	int error;
	char * filename;

	filename = getname(filenamei);
	error = PTR_ERR(filename);
	if (IS_ERR(filename))
		goto out;
	error = do_execve(filename, argv, envp, L4X_THREAD_REGSP(&current->thread));
	putname(filename);
out:
	return error;
}

int kernel_execve(const char *filename,
		  const char *const argv[],
		  const char *const envp[])
{
#ifndef CONFIG_L4_VCPU
	struct thread_struct *t = &current->thread;
	struct pt_regs *regs = L4X_THREAD_REGSP(t);
	int ret;

	BUG_ON(!l4_is_invalid_cap(current->thread.user_thread_id));

	memset(regs, 0, sizeof(struct pt_regs));
	ret = do_execve(filename,
	                (const char __user * const __user *)argv,
			(const char __user * const __user *)envp, regs);
	if (ret < 0) {
		/* we failed -- become a kernel thread again */
		if (!l4_is_invalid_cap(t->user_thread_id))
			l4lx_task_number_free(t->user_thread_id);
		set_fs(KERNEL_DS);
		t->user_thread_id = L4_INVALID_CAP;

		ret = -EBUSY;
		goto out;
	}

	l4x_user_dispatcher();

	enter_kdebug("Returned exec?");
#else
	struct pt_regs regs;
	int ret;

	memset(&regs, 0, sizeof(struct pt_regs));
	ret = do_execve(filename,
			(const char __user *const __user *)argv,
			(const char __user *const __user *)envp, &regs);
	if (ret < 0)
		goto out;

	/*
	 * Save argc to the register structure for userspace.
	 */
	regs.ARM_r0 = ret;

	/*
	 * We were successful.  We won't be returning to our caller, but
	 * instead to user space by manipulating the kernel stack.
	 */
	asm(	"add	r0, %0, %1\n\t"
		"mov	r1, %2\n\t"
		"mov	r2, %3\n\t"
		"bl	memmove\n\t"	/* copy regs to top of stack */
		"mov	r8, #0\n\t"	/* not a syscall */
		"mov	r9, %0\n\t"	/* thread structure */
		"mov	sp, r0\n\t"	/* reposition stack pointer */
		"b	l4x_vcpu_ret_from_fork"
		:
		: "r" (current_thread_info()),
		  "Ir" (THREAD_START_SP - sizeof(regs)),
		  "r" (&regs),
		  "Ir" (sizeof(regs))
		: "r0", "r1", "r2", "r3", "ip", "lr", "memory");
#endif

 out:
	return ret;
}
EXPORT_SYMBOL(kernel_execve);

/*
 * Since loff_t is a 64 bit type we avoid a lot of ABI hassle
 * with a different argument ordering.
 */
asmlinkage long sys_arm_fadvise64_64(int fd, int advice,
				     loff_t offset, loff_t len)
{
	return sys_fadvise64_64(fd, offset, len, advice);
}
void sys_syscall(void)
{
	/* The dispatch loop should catch this syscall, so that we never get
	 * here... */
	enter_kdebug("should not come here!");
}
