/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

inline void
switch_stack(unsigned long stack, void (*func)())
{
  asm volatile ( "mov %[stack], %%esp   \n\t"
		 "xor %%ebp, %%ebp      \n\t"
                 "jmp *%[func]          \n\t"
		 : : [stack] "r" (stack), [func] "r" (func), "d" (0)
		 : "memory", "cc");
}

