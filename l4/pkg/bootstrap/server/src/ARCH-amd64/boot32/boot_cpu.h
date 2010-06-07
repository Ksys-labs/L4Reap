/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef BOOT_CPU_H
#define BOOT_CPU_H

#include "types.h"

void base_paging_init (l4_uint64_t);
void base_cpu_setup (void);

extern struct ptab64_mem_info_t ptab64_mem_info;

#endif
