/*
 * (c) 2008-2009 Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

__l4sys_direct_syscalls = 0xffffffffeacff000
.hidden __l4sys_direct_syscalls

	.macro	direct_syscall syscall,offset
\syscall = __l4sys_direct_syscalls + \offset
.global \syscall
	.endm

	direct_syscall	__l4sys_fpage_unmap_direct, 0x200
	direct_syscall	__l4sys_id_nearest_direct, 0x100
	direct_syscall	__l4sys_ipc_direct, 0x000
	direct_syscall	__l4sys_lthread_ex_regs_direct, 0x500
	direct_syscall	__l4sys_privctrl_direct, 0x700
	direct_syscall	__l4sys_task_new_direct, 0x600
	direct_syscall	__l4sys_thread_schedule_direct, 0x400
	direct_syscall	__l4sys_thread_switch_direct, 0x300
