/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/exceptions>
#include <l4/cxx/auto_ptr>
#include <l4/cxx/l4iostream>

#include <l4/re/protocols>
#include <l4/re/mem_alloc-sys.h>
#include <l4/re/mem_alloc>
#include <l4/re/util/meta>
#include <l4/sys/factory>

#include <cstdlib>

#include "alloc.h"
#include "dataspace_annon.h"
#include "dataspace_noncont.h"
#include "globals.h"
#include "page_alloc.h"
#include "slab_alloc.h"
#include "quota.h"
#include "region.h"
#include "name_space.h"
#include "log.h"
#include "sched_proxy.h"

typedef Moe::Q_alloc<Allocator, Slab_alloc> Alloc;

template< typename C >
class Quota_obj : public C
{
private:
  typedef Moe::Q_alloc<Quota_obj<C>, Slab_alloc> Alloc;
  static Alloc *_allocator()
  {
    static Alloc _a;
    return &_a;
  }

public:
  virtual ~Quota_obj() {}
  void *operator new (size_t size, Moe::Quota *q)
  {
    if (size != sizeof(Quota_obj))
      throw L4::Out_of_memory();

    return _allocator()->alloc(q);
  }

  void operator delete(void *o)
  {
    _allocator()->free((Quota_obj*)o);
  }

  Quota_obj() : C() {}

  template< typename A1 >
  Quota_obj(A1 a1) : C(a1) {}

  template< typename A1, typename A2 >
  Quota_obj(A1 a1, A2 a2) : C(a1, a2) {}

  template< typename A1, typename A2, typename A3 >
  Quota_obj(A1 a1, A2 a2, A3 a3) : C(a1, a2, a3) {}


};

static Alloc *slab()
{
  static Alloc _slab;
  return &_slab;
}

void *Allocator::operator new (size_t, Moe::Quota *q, size_t limit)
{
  return slab()->alloc(q, limit);
}

void Allocator::operator delete (void *m) throw()
{
  slab()->free((Allocator*)m);
}

Moe::Dataspace *
Allocator::alloc(unsigned long size, unsigned long flags, unsigned long align)
{
  if (size == 0)
    throw L4::Bounds_error("stack too small");

  //L4::cout << "A: \n";
  Moe::Dataspace *mo;
  if (flags & L4Re::Mem_alloc::Continuous
      || flags & L4Re::Mem_alloc::Pinned)
    {
      if (flags & L4Re::Mem_alloc::Super_pages)
        align = cxx::max<unsigned long>(align, L4_SUPERPAGESHIFT);
      else
        align = cxx::max<unsigned long>(align, L4_PAGESHIFT);

      mo = new (&_quota) Moe::Dataspace_annon(size, true, align);
    }
  else
    mo = Moe::Dataspace_noncont::create(&_quota, size);

  // L4::cout << "A: mo=" << mo << "\n";

  //L4::cout << "A[" << this << "]: allocated(" << mo << " " << mo->obj_cap() << ")\n";
  return mo;
}

Allocator::~Allocator()
{
  if (Q_object::quota())
    Q_object::quota()->free(_quota.limit());
}

L4::Server_object *
Allocator::open(int, cxx::String const *argv)
{
  unsigned long sz = strtol(argv[0].start(), 0, 0);
  if(!sz)
    {
      //L4::cerr << "could not parse quota value!\n";
      return 0;
    }
  Allocator *child = new (&_quota, sz) Allocator(sz);
  object_pool.cap_alloc()->alloc(child);
  return child;
}


class LLog : public Moe::Log
{
private:
  char _tag[32];

public:
  LLog(char const *t, int l, unsigned char col) : Log()
  {
    if (l > 32)
      l = 32;

    memcpy(_tag, t, l);

    set_tag(_tag, l);

    set_color(col);
  }

  virtual ~LLog() {}
};

int
Allocator::disp_factory(l4_umword_t r, L4::Ipc::Iostream &ios)
{
  if (!(r & L4_CAP_FPAGE_S))
    return -L4_EPERM;

  L4::Factory::Proto o;
  ios >> o;

  L4::Cap<L4::Kobject> ko;

  switch (o)
    {
    case L4Re::Protocol::Namespace:
      ko = object_pool.cap_alloc()->alloc(new (&_quota) Quota_obj<Moe::Name_space>());
      ko->dec_refcnt(1);
      ios << ko;
      return L4_EOK;

    case L4Re::Protocol::Rm:
      ko = object_pool.cap_alloc()->alloc(new (&_quota) Quota_obj<Region_map>());
      ko->dec_refcnt(1);
      ios << ko;
      return L4_EOK;

    case L4::Factory::Protocol:
    case L4Re::Protocol::Mem_alloc:
	{
	  L4::Ipc::Varg tag;
	  ios.get(&tag);

	  if (!tag.is_of_int()) // ignore sign
	    return -L4_EINVAL;
	  Allocator *child = new (&_quota, tag.value<long>()) Allocator(tag.value<long>());
	  ko = object_pool.cap_alloc()->alloc(child);
	  ko->dec_refcnt(1);
	  ios << ko;

	  return 0;
	}

    case L4_PROTO_LOG:
	{
	  L4::Ipc::Varg tag;
	  ios.get(&tag);

	  if (!tag.is_of<char const *>())
	    return -L4_EINVAL;

	  L4::Ipc::Varg col;
	  ios.get(&col);

	  int color;
	  if (col.is_of<char const *>())
	    color = LLog::color_value(cxx::String(col.value<char const*>(),
		                                  col.length()));
	  else if(col.is_of_int())
	    color = col.value<l4_mword_t>();
	  else
	    color = 7;

	  Moe::Log *l = new (&_quota) Quota_obj<LLog>(tag.value<char const*>(), tag.length(), color);
	  ko = object_pool.cap_alloc()->alloc(l);
	  ko->dec_refcnt(1);

	  ios << ko;
	  return L4_EOK;
	}

    case L4::Scheduler::Protocol:
	{
	  if (!_sched_prio_limit)
	    return -L4_ENODEV;

	  L4::Ipc::Varg p_max, p_base, cpus;
	  ios.get(&p_max);
	  ios.get(&p_base);
	  ios.get(&cpus);

	  if (!p_max.is_of_int() || !p_base.is_of_int())
	    return -L4_EINVAL;

	  if (p_max.value<l4_mword_t>() > _sched_prio_limit
	      || p_base.value<l4_mword_t>() > _sched_prio_limit)
	    return -L4_ERANGE;

	  if (p_max.value<l4_mword_t>() <= p_base.value<l4_mword_t>())
	    return -L4_EINVAL;

	  l4_umword_t cpu_mask = ~0UL;

	  if (!cpus.is_of<void>() && cpus.is_of_int())
	    cpu_mask = cpus.value<l4_umword_t>();

	  Sched_proxy *o = new (&_quota) Quota_obj<Sched_proxy>();
	  o->set_prio(p_base.value<l4_mword_t>(), p_max.value<l4_mword_t>());
	  o->restrict_cpus(cpu_mask);
	  ko = object_pool.cap_alloc()->alloc(o);
	  ko->dec_refcnt(1);

	  ios << ko;
	  return L4_EOK;
	}
    case L4Re::Dataspace::Protocol:
	{
	  L4::Ipc::Varg size, flags, align;
	  ios >> size >> flags >> align;

	  if (!size.is_of_int())
	    return -L4_EINVAL;

	  // L4::cout << "MEM: alloc ... " << size.value<l4_umword_t>() << "; " << flags.value<l4_umword_t>() << "\n";
	  cxx::Auto_ptr<Moe::Dataspace> mo(alloc(size.value<l4_umword_t>(),
		flags.is_of_int() ? flags.value<l4_umword_t>() : 0,
                align.is_of_int() ? align.value<l4_umword_t>() : 0));

	  // L4::cout << "MO=" << mo.get() << "\n";
	  ko = object_pool.cap_alloc()->alloc(mo.get());
	  ko->dec_refcnt(1);
	  // L4::cout << "MO_CAP=" << mo->obj_cap() << "\n";
	  ios << ko;
	  mo.release();
	  return L4_EOK;
	}

    default:
      return -L4_ENODEV;
    }
}

// create out annonymous combination of a memory allocctaor and a factory
class Moe_allocator :
  public L4::Kobject_2t<Moe_allocator, L4::Factory, L4Re::Mem_alloc>
{};

int
Allocator::dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
{
  L4::Opcode op;

  l4_msgtag_t tag;
  ios >> tag;
  switch (tag.label())
    {
    case L4::Meta::Protocol:
      return L4Re::Util::handle_meta_request<Moe_allocator>(ios);
    case L4::Factory::Protocol:
      return disp_factory(obj, ios);

    case L4Re::Protocol::Mem_alloc:
      {
	ios >> op;
	switch (op)
	  {
	  case L4Re::Mem_alloc_::Alloc:
	    {
	      unsigned long size;
	      unsigned long flags;
	      ios >> size >> flags;

              printf("MOE: WARNING: Using deprecated interface L4Re::Mem_alloc_::Alloc\n");
	      //L4::cout << "MEM: alloc ... " << size << "; " << flags << "\n";
              cxx::Auto_ptr<Moe::Dataspace> mo(alloc(size,flags));

	      // L4::cout << "MO=" << mo.get() << "\n";
	      L4::Cap<void> o = object_pool.cap_alloc()->alloc(mo.get());
	      //L4::cout << "MO_CAP=" << mo->obj_cap() << "\n";
	      ios << o;
              mo.release();
	      return L4_EOK;
	    }
	  case L4Re::Mem_alloc_::Free:
	    {
	      l4_umword_t rc1, rc2;

	      Moe::Dataspace *mo = 0;
	      ios >> rc1 >> rc2;
	      if ((rc1 & 0xe) == 0xc) // XXX: change with cap.id_received()
		mo = dynamic_cast<Moe::Dataspace*>(object_pool.find(rc2));


	      // FIXME: check if dataspace comes from this allocator 

	      if (!mo || mo->is_static())
		return -L4_EINVAL;

	      object_pool.cap_alloc()->free(mo);
	      mo->unmap();
	      delete mo;
	      return L4_EOK;
	    }
	  default:
	    return -L4_ENOSYS;
	  }
      }
    case L4Re::Protocol::Debug:
      {
        printf("MOE: mem_alloc: quota: limit=%zd Byte, used=%zd Byte\n",
               _quota.limit(), _quota.used());
        printf("MOE: mem_alloc: global: avail=%ld Byte\n",
               Single_page_alloc_base::_avail());
        return L4_EOK;
      }
    default:
      return -L4_EBADPROTO;
    }
}


Allocator *
Allocator::root_allocator()
{
  static Allocator ra(~0, 300000);
  return &ra;
}
