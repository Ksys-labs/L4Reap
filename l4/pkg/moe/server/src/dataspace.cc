/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "dataspace.h"
#include "dataspace_util.h"
#include "globals.h"
#include "page_alloc.h"


#include <l4/re/dataspace-sys.h>
#include <l4/re/protocols>
#include <l4/re/util/meta>

#include <l4/cxx/ipc_stream>
#include <l4/cxx/slab_alloc>
#include <l4/cxx/minmax>

#include <l4/sys/capability>
#include <l4/sys/err.h>

#include <cstring>
using cxx::min;

int
Moe::Dataspace::map(l4_addr_t offs, l4_addr_t hot_spot, bool _rw,
                    l4_addr_t min, l4_addr_t max, L4::Snd_fpage &memory)
{
  memory = L4::Snd_fpage();

  offs     = l4_trunc_page(offs);
  hot_spot = l4_trunc_page(hot_spot);

  if (!check_limit(offs))
    {
#if 1
      L4::cout << "MOE: ds access out of bounds: offset=" << L4::n_hex(offs)
	       << " size=" << L4::n_hex(size()) << "\n";
#endif
      return -L4_ERANGE;
    }

  Ds_rw rw = _rw ? Writable : Read_only;
  Address adr = address(offs, rw, hot_spot, min, max);
  if (adr.is_nil())
    return -L4_EPERM;

  memory = L4::Snd_fpage(adr.fp(), hot_spot, L4::Snd_fpage::Map,
                         (L4::Snd_fpage::Cacheopt)((_flags >> 12) & (7 << 4)));

  return L4_EOK;
}

inline
L4::Ipc_ostream &operator << (L4::Ipc_ostream &s,
                              L4Re::Dataspace::Stats const &st)
{ s.put(st); return s; }

int
Moe::Dataspace::dispatch(l4_umword_t obj, L4::Ipc_iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  if (tag.label() == L4::Meta::Protocol)
    return L4Re::Util::handle_meta_request<L4Re::Dataspace>(ios);

  if (tag.label() != L4Re::Protocol::Dataspace)
    return -L4_EBADPROTO;

  L4::Opcode op;
  ios >> op;
#if 0
  L4::cout << "MOE: DS: op=" << L4::n_hex(op) << "\n";
#endif

  switch (op)
    {
    case L4Re::Dataspace_::Map:
      {
	// L4_FPAGE_X means writable for DSs!
	bool read_only = !is_writable() || !(obj & L4_FPAGE_X);
	l4_addr_t offset, spot;
	unsigned long flags;
	L4::Snd_fpage fp;
	ios >> offset >> spot >> flags;
#if 0
	L4::cout << "MAPrq: " << L4::hex << offset << ", " << spot << ", "
	         << flags << "\n";
#endif

	if (read_only && (flags & Writable))
	  return -L4_EPERM;

	long int ret = map(offset, spot, flags & Writable, 0, ~0, fp);
#if 0
	L4::cout << "MAP: " << L4::hex << reinterpret_cast<unsigned long *>(&fp)[0]
	         << ", " << reinterpret_cast<unsigned long *>(&fp)[1]
		 << ", " << flags << ", " << (!read_only && (flags & 1))
		 << ", ret=" << ret << '\n';
#endif
	if (ret == L4_EOK)
	  ios << fp;

	return ret;
      }
    case L4Re::Dataspace_::Clear:
      {
	if (!(obj & L4_FPAGE_X) /* read only*/
	    || !is_writable())
	  return -L4_EACCESS;

	l4_addr_t offs;
	unsigned long sz;

	ios >> offs >> sz;
	return clear(offs, sz);
      }
    case L4Re::Dataspace_::Stats:
      {
	L4Re::Dataspace::Stats s;
	s.size = size();
	// only return writable if really writable
	s.flags = flags() & ~Writable;
        if ((obj & L4_FPAGE_X) && is_writable())
          s.flags |= Writable;

	ios << s;
	return L4_EOK;
      }
    case L4Re::Dataspace_::Copy:
      {
	l4_addr_t dst_offs;
	Moe::Dataspace *src = 0;
	l4_addr_t src_offs;
	unsigned long sz;
	L4::Snd_fpage src_cap;

	ios >> dst_offs >> src_offs >> sz >> src_cap;

	if (src_cap.id_received())
	  src = dynamic_cast<Moe::Dataspace*>(object_pool.find(src_cap.data()));

	if (!(obj & L4_FPAGE_X))
	  return -L4_EACCESS;

	if (!src)
	  return -L4_EINVAL;

	if (sz == 0)
	  return L4_EOK;

	Dataspace_util::copy(this, dst_offs, src, src_offs, sz);

	return L4_EOK;
      }
    case L4Re::Dataspace_::Phys:
      {
	l4_addr_t offset;
	l4_addr_t phys_addr;
	l4_size_t phys_size;

	ios >> offset;

	int ret = phys(offset, phys_addr, phys_size);
	if (ret)
	  return -L4_EINVAL;

	ios << phys_addr << phys_size;

        return L4_EOK;
      }
    case L4Re::Dataspace_::Allocate:
      {
	l4_addr_t offset;
	l4_size_t size;
	ios >> offset >> size;
	return pre_allocate(offset, size, obj & 3);
      }
    case L4Re::Dataspace_::Take:
      take();
      //L4::cout << "MOE: T[" << this << "]: refs=" << ref_cnt() << '\n';
      return L4_EOK;
    case L4Re::Dataspace_::Release:
      if (release() == 0 && !is_static())
	{
	  //L4::cout << "MOE: R[" << this << "]: refs=" << ref_cnt() << '\n';
	  delete this;
	  return 0;
	}
      //L4::cout << "MOE: R[" << this << "]: refs=" << ref_cnt() << '\n';

      return 1;
    default:
      return -L4_ENOSYS;
    }
}

long
Moe::Dataspace::clear(l4_addr_t offs, unsigned long _size) const throw()
{
  if (!check_limit(offs))
    return -L4_ERANGE;

  unsigned long sz = _size = min(_size, round_size()-offs);

  while (sz)
    {
      Address dst_a = address(offs, Writable);
      unsigned long b_sz = min(dst_a.sz() - dst_a.of(), sz);

      memset(dst_a.adr(), 0, b_sz);

      offs += b_sz;
      sz -= b_sz;
    }

  return _size;
}

int
Moe::Dataspace::phys(l4_addr_t /*offset*/, l4_addr_t &/*phys_addr*/, l4_size_t &/*phys_size*/) throw()
{
  return -L4_EINVAL;
}
