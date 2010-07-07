/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "globals.h"

namespace Global
{
  Region_map *local_rm;
  L4::Cap<L4Re::Mem_alloc> allocator;
  Cap_alloc cap_alloc(L4Re::Env::env()->first_free_cap());
  char const *const *argv;
  char const *const *envp;
  int argc;
  l4re_aux_t *l4re_aux;
};
