/**
 * \file
 * \brief  Region map client stub implementation
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
#include <l4/re/rm>
#include <l4/re/rm-sys.h>
#include <l4/re/dataspace>
#include <l4/re/protocols>

#include <l4/cxx/ipc_helper>
#include <l4/cxx/ipc_stream>

#include <l4/sys/task>
#include <l4/sys/err.h>


namespace L4Re
{

using L4::Opcode;

long
Rm::reserve_area(l4_addr_t *start, unsigned long size, unsigned flags,
                 unsigned char align) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Rm_::Attach_area) << *start << size << flags << align;
  long err = l4_error(io.call(cap(), L4Re::Protocol::Rm));
  if (EXPECT_FALSE(err < 0))
    return err;

  io >> *start;
  return err;
}

long
Rm::free_area(l4_addr_t addr) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Rm_::Detach_area) << addr;
  return l4_error(io.call(cap(), L4Re::Protocol::Rm));
}

long
Rm::attach(l4_addr_t *start, unsigned long size, unsigned long flags,
           L4::Cap<Dataspace> mem, l4_addr_t offs,
           unsigned char align) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Rm_::Attach) << l4_addr_t(*start) << size << flags
     << offs << align;

  if (!(flags & Reserved))
    io << mem.cap() << mem;

  long err = l4_error(io.call(cap(), L4Re::Protocol::Rm));
  if (EXPECT_FALSE(err < 0))
    return err;

  io >> *reinterpret_cast<l4_addr_t*>(start);

  if (flags & Eager_map)
    {
      unsigned long fl = (flags & Read_only)
	? Dataspace::Map_ro
	: Dataspace::Map_rw;
      err = mem->map_region(offs, fl, *start, *start + size);
    }
  return err;
}

int
Rm::_detach(l4_addr_t addr, unsigned long size, L4::Cap<Dataspace> *mem,
            L4::Cap<L4::Task> task, unsigned flags) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Rm_::Detach) << addr << size << flags;
  long err = l4_error(io.call(cap(), L4Re::Protocol::Rm));
  if (EXPECT_FALSE(err < 0))
    return err;

  l4_addr_t start;
  unsigned long rsize;
  io >> start >> rsize;

  if (mem)
    io >> *reinterpret_cast<l4_cap_idx_t*>(mem);

  if (!task.is_valid())
    return err;

  rsize = l4_round_page(rsize);
  unsigned order = L4_LOG2_PAGESIZE;
  unsigned long sz = (1UL << order);
  for (unsigned long p = start; rsize; p += sz, rsize -= sz)
    {
      while (sz > rsize)
	{
	  --order;
	  sz >>= 1;
	}

      for (;;)
	{
	  unsigned long m = sz << 1;
	  if (m > rsize)
	    break;

	  if (p & (m - 1))
	    break;

	  ++order;
	  sz <<= 1;
	}

      task->unmap(l4_fpage(p, order, L4_FPAGE_RWX),
                  L4_FP_ALL_SPACES);
    }

  return err;
}


int
Rm::find(l4_addr_t *addr, unsigned long *size, unsigned long *offset,
         unsigned *flags, L4::Cap<Dataspace> *m) throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Rm_::Find) << *addr << *size;
  long err = l4_error(io.call(cap(), L4Re::Protocol::Rm));
  if (EXPECT_FALSE(err < 0))
    return err;

  l4_cap_idx_t c;
  io >> *addr >> *size >> *flags >> *offset >> c;
  *m = L4::Cap<Dataspace>(c);

  return err;
}

int
Rm::get_regions(l4_addr_t start, Region **regions) throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Rm_::Get_regions) << start;
  long err = l4_error(io.call(cap(), L4Re::Protocol::Rm));
  if (err > 0)
    *regions = reinterpret_cast<Region*>(&l4_utcb_mr()->mr[0]);
  return err;
}

int
Rm::get_areas(l4_addr_t start, Area **areas) throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Rm_::Get_areas) << start;
  long err = l4_error(io.call(cap(), L4Re::Protocol::Rm));
  if (err > 0)
    *areas = reinterpret_cast<Area*>(&l4_utcb_mr()->mr[0]);
  return err;
}

}
