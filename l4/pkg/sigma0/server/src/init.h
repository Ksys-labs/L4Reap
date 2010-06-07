/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef INIT_H
#define INIT_H

#include <l4/sys/compiler.h>

#define INIT_SECTION __attribute__((section (".init")))

#ifdef __cplusplus
extern "C" 
#endif
void init(l4_kernel_info_t *info) L4_NORETURN;

#endif
