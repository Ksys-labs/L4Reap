/**
 * \file
 * \brief  Dataspace client stub implementation
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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
#include <l4/re/dataspace>
#include <l4/re/dataspace-sys.h>
#include <l4/re/protocols>

#include <l4/cxx/exceptions>
#include <l4/cxx/ipc_helper>
#include <l4/cxx/ipc_stream>

inline
L4::Ipc_istream &operator >> (L4::Ipc_istream &s, L4Re::Dataspace::Stats &v)
{ s.get(v); return s; }

namespace L4Re {

long
Dataspace::__map(unsigned long offset, unsigned char *size, unsigned long flags,
                 l4_addr_t local_addr) const throw()
{
  l4_addr_t spot = local_addr & ~(~0UL << l4_umword_t(*size));
  l4_addr_t base = local_addr & (~0UL << l4_umword_t(*size));
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dataspace_::Map) << offset << spot << flags;
  io << L4::Rcv_fpage::mem(base, *size, 0);
  long err = l4_error(io.call(cap(), L4Re::Protocol::Dataspace));
  if (err < 0)
    return err;

  L4::Snd_fpage fp;
  io >> fp;
  *size = fp.rcv_order();
  return err;
}

static inline
unsigned char max_order(unsigned char order, l4_addr_t addr,
                        l4_addr_t min_addr, l4_addr_t max_addr)
{
  while (order < 30 /* limit to 1GB flexpages */)
    {
      l4_addr_t base = l4_trunc_size(addr, order + 1);
      if (base < min_addr)
	return order;

      if (base + (1UL << (order + 1)) -1 > max_addr -1)
	return order;

      ++order;
    }
  return order;
}

long
Dataspace::map_region(l4_addr_t offset, unsigned long flags,
                      l4_addr_t min_addr, l4_addr_t max_addr) const throw()
{
  min_addr   = l4_trunc_page(min_addr);
  max_addr   = l4_round_page(max_addr);
  unsigned char order = L4_LOG2_PAGESIZE;

  long err = 0;

  while (min_addr < max_addr)
    {
      unsigned char order_mapped;
      order_mapped = order = max_order(order, min_addr, min_addr, max_addr);
      err = __map(offset, &order_mapped, flags, min_addr);
      if (EXPECT_FALSE(err < 0))
	return err;

      if (order > order_mapped)
	order = order_mapped;

      min_addr += 1UL << order;
      offset   += 1UL << order;

      if (min_addr >= max_addr)
	return 0;

      while (min_addr != l4_trunc_size(min_addr, order)
             || max_addr < l4_round_size(min_addr + 1,order))
	--order;
    }

  return 0;
}


long
Dataspace::map(l4_addr_t offset, unsigned long flags,
               l4_addr_t local_addr,
               l4_addr_t min_addr, l4_addr_t max_addr) const throw()
{
  min_addr   = l4_trunc_page(min_addr);
  max_addr   = l4_round_page(max_addr);
  local_addr = l4_trunc_page(local_addr);
  unsigned char order
    = max_order(L4_LOG2_PAGESIZE, local_addr, min_addr, max_addr);

  return __map(offset, &order, flags, local_addr);
}

long
Dataspace::clear(unsigned long offset, unsigned long size) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dataspace_::Clear) << offset << size;
  long err = l4_error(io.call(cap(), L4Re::Protocol::Dataspace));
  if (EXPECT_FALSE(err < 0))
    return err;

  long sz;
  io >> sz;
  return sz;
}

int
Dataspace::info(Stats *stats) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dataspace_::Stats);
  long err = l4_error(io.call(cap(), L4Re::Protocol::Dataspace));
  if (EXPECT_FALSE(err < 0))
    return err;

  io >> *stats;
  return 0;
}

long
Dataspace::size() const throw()
{
  Stats stats;
  int err = info(&stats);
  if (err < 0)
    return err;
  return stats.size;
}

long
Dataspace::flags() const throw()
{
  Stats stats;
  int err = info(&stats);
  if (err < 0)
    return err;
  return stats.flags;
}

long
Dataspace::copy_in(unsigned long dst_offs, L4::Cap<Dataspace> src,
                   unsigned long src_offs, unsigned long size) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dataspace_::Copy) << dst_offs << src_offs << size << src;
  return l4_error(io.call(cap(), L4Re::Protocol::Dataspace));
}

long
Dataspace::phys(l4_addr_t offset, l4_addr_t &phys_addr, l4_size_t &phys_size) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dataspace_::Phys) << offset;
  long err = l4_error(io.call(cap(), L4Re::Protocol::Dataspace));
  if (EXPECT_FALSE(err < 0))
    return err;

  io >> phys_addr >> phys_size;
  return 0;
}

long
Dataspace::allocate(l4_addr_t offset, l4_size_t size) throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dataspace_::Allocate) << offset << size;
  return l4_error(io.call(cap(), L4Re::Protocol::Dataspace));
}


long
Dataspace::take() const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dataspace_::Take);
  return l4_error(io.call(cap(), L4Re::Protocol::Dataspace));
}

long
Dataspace::release() const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dataspace_::Release);
  return l4_error(io.call(cap(), L4Re::Protocol::Dataspace));
}

};
