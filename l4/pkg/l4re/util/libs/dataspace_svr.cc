// vi:ft=cpp
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
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
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <l4/sys/types.h>
#include <l4/cxx/list>
#include <l4/cxx/ipc_server>
#include <l4/cxx/ipc_stream>
#include <l4/cxx/minmax>
#include <l4/re/dataspace>
#include <l4/re/dataspace-sys.h>
#include <l4/re/protocols>
#include <l4/re/util/dataspace_svr>

#if 0
inline
L4::Ipc_ostream &operator << (L4::Ipc_ostream &s,
                              L4Re::Dataspace::Stats const &st)
{ s.put(st); return s; }
#endif

namespace L4Re { namespace Util {

int
Dataspace_svr::map(l4_addr_t offs, l4_addr_t hot_spot, unsigned long flags,
                    l4_addr_t min, l4_addr_t max, L4::Snd_fpage &memory)
{
  int err = map_hook(offs, flags, min, max);
  if (err < 0)
    return err;

  memory = L4::Snd_fpage();

  offs     = l4_trunc_page(offs);
  hot_spot = l4_trunc_page(hot_spot);

  if (!check_limit(offs))
    {
#if 0
      printf("limit failed: off=%lx sz=%lx\n", offs, size());
#endif
      return -L4_ERANGE;
    }

  min = l4_trunc_page(min);
  max = l4_round_page(max);

  l4_addr_t adr = _ds_start + offs;
  unsigned char order = L4_PAGESHIFT;

  while (order < 30 /* limit to 1GB flexpage */)
    {
      l4_addr_t map_base = l4_trunc_size(adr, order + 1);
      if (map_base < _ds_start)
	break;

      if (map_base + (1UL << (order + 1)) -1 > (_ds_start + round_size() - 1))
	break;

      map_base = l4_trunc_size(hot_spot, order + 1);
      if (map_base < min)
	break;

      if (map_base + (1UL << (order + 1)) -1 > max -1)
	break;

      l4_addr_t mask = ~(~0UL << (order + 1));
      if (hot_spot == ~0UL || ((adr ^ hot_spot) & mask))
	break;

      ++order;
    }

  l4_addr_t map_base = l4_trunc_size(adr, order);
  //l4_addr_t map_offs = adr & ~(~0UL << order);

  l4_fpage_t fpage = l4_fpage(map_base, order, flags && is_writable() ?  L4_FPAGE_RWX : L4_FPAGE_RX);
  
  memory = L4::Snd_fpage(fpage, hot_spot, _map_flags, _cache_flags);

  return L4_EOK;
}

long
Dataspace_svr::clear(l4_addr_t offs, unsigned long _size) const throw()
{
  if (!check_limit(offs))
    return -L4_ERANGE;

  unsigned long sz = _size = cxx::min(_size, round_size()-offs);

  while (sz)
    {
      unsigned long b_adr = _ds_start + offs;
      unsigned long b_sz = cxx::min(_size - offs, sz);

      memset((void *)b_adr, 0, b_sz);

      offs += b_sz;
      sz -= b_sz;
    }

  return _size;
}

int
Dataspace_svr::phys(l4_addr_t /*offset*/, l4_addr_t &/*phys_addr*/, l4_size_t &/*phys_size*/) throw()
{
  return -L4_EINVAL;
}

int
Dataspace_svr::dispatch(l4_umword_t obj, L4::Ipc_iostream &ios)
{
  L4::Opcode op;
  ios >> op;
#if 0
  L4::cout << "Dataspace_svr: DS: op=" << L4::n_hex(op) << "\n";
#endif
  l4_msgtag_t tag;
  ios >> tag;

  if (tag.label() != L4Re::Protocol::Dataspace)
    return -L4_EBADPROTO;

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

	if (read_only && (flags & 1))
	  return -L4_EPERM;

	long int ret = map(offset, spot, flags & 1, 0, ~0, fp);
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
	if ((obj & 1) /* read only*/
	    || is_static() || !is_writable())
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
	s.flags = (rw_flags() & ~Writable) | (!(obj & 1) && is_writable());
	ios << s;
	return L4_EOK;
      }
    case L4Re::Dataspace_::Copy:
      {
	l4_addr_t dst_offs;
	l4_addr_t src_offs;
	unsigned long sz;
	L4::Snd_fpage src_cap;

	ios >> dst_offs >> src_offs >> sz >> src_cap;

	if (!src_cap.id_received())
	  return -L4_EINVAL;

	if (!(obj & 1))
	  return -L4_EACCESS;

	if (sz == 0)
	  return L4_EOK;

	copy(dst_offs, src_cap.data(), src_offs, sz);

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
    case L4Re::Dataspace_::Take:
      take();
      //L4::cout << "Dataspace_svr: T[" << this << "]: refs=" << ref_cnt() << '\n';
      return L4_EOK;
    case L4Re::Dataspace_::Release:
      if ((release() == 0) && !is_static())
	{
	  //L4::cout << "Dataspace_svr: R[" << this << "]: refs=" << ref_cnt() << '\n';
	  delete this;
	  return 0;
	}
      //L4::cout << "Dataspace_svr: R[" << this << "]: refs=" << ref_cnt() << '\n';

      return 1;
    default:
      return -L4_ENOSYS;
    }
}

}}
