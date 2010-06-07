/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/capability>
#include <l4/re/dataspace>


#include <l4/sys/types.h>
#include <l4/re/l4aux.h>
#include <l4/libloader/remote_mem>

#include <cstddef>

class Region_map;

class Stack : public Ldr::Remote_stack<>
{
public:
  explicit Stack(char *p = 0) : Ldr::Remote_stack<>(p) {}
  l4_addr_t add(l4_addr_t start, l4_umword_t size, Region_map *rm,
                L4::Cap<L4Re::Dataspace> m, unsigned long offs,
                unsigned flags, unsigned char align,
                char const *what);
};
