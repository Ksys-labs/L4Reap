/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#pragma once

#include <sys/cdefs.h>
#include <l4/sys/types.h>

__BEGIN_DECLS

void libsig_be_add_thread(l4_cap_idx_t t);
void libsig_be_set_dbg_name(const char *name);

__END_DECLS
