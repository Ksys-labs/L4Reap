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
#include <l4/sys/factory>
#include <l4/re/util/meta>

#include <cstdlib>

#include "obj_reg.h"
#include "alloc.h"
//#include "globals.h"
//#include "slab_alloc.h"
#include "region.h"
#include "name_space.h"
#include "log.h"
#include "sched_proxy.h"



Allocator::~Allocator()
{
}



class LLog : public Ldr::Log
{
public:
  LLog(char const *t, int l, unsigned char col) : Log()
  {
    if (l > 32)
      l = 32;

    set_tag(cxx::String(t, l));
    set_color(col);
  }

  virtual ~LLog() {}
};

int
Allocator::disp_factory(l4_umword_t, L4::Ipc::Iostream &ios)
{
  L4::Factory::Proto o;
  ios >> o;
  switch (o)
    {
    case L4Re::Protocol::Namespace:
      ios << Gate_alloc::registry.register_obj(new Ldr::Name_space());
      return L4_EOK;

    case L4Re::Protocol::Rm:
      ios << Gate_alloc::registry.register_obj(new Region_map());
      return L4_EOK;


    case L4_PROTO_LOG:
	{
	  L4::Ipc::Istream_copy is(ios);
	  L4::Ipc::Varg tag;
	  is.get(&tag);

	  if (!tag.is_of<char const *>())
	    return -L4_EINVAL;

	  L4::Ipc::Varg col;
	  is.get(&col);

	  int color;
	  if (col.is_of<char const *>())
	    color = LLog::color_value(cxx::String(col.value<char const*>(),
		                                  col.length()));
	  else if(col.is_of_int()) // ignore sign
	    color = col.value<long>();
	  else
	    color = 7;

	  Ldr::Log *l = new LLog(tag.value<char const*>(), tag.length(), color);

	  ios << Gate_alloc::registry.register_obj(l);
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

	  if (!p_max.is_of_int() || !p_base.is_of_int()) // ignore sign
	    return -L4_EINVAL;

	  if (p_max.value<l4_mword_t>() > _sched_prio_limit
	      || p_base.value<l4_mword_t>() > _sched_prio_limit)
	    return -L4_ERANGE;

	  if (p_max.value<l4_umword_t>() <= p_base.value<l4_umword_t>())
	    return -L4_EINVAL;

	  l4_umword_t cpu_mask = ~0UL;

	  if (!cpus.is_of<void>() && cpus.is_of_int())
	    cpu_mask = cpus.value<l4_umword_t>();

	  Sched_proxy *o = new Sched_proxy();
	  o->set_prio(p_base.value<l4_mword_t>(), p_max.value<l4_mword_t>());
	  o->restrict_cpus(cpu_mask);

	  ios << Gate_alloc::registry.register_obj(o);
	  return L4_EOK;
	}

    default:
      return -L4_ENODEV;
    }
}

int
Allocator::dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;
  switch (tag.label())
    {
    case L4::Meta::Protocol:
      return L4Re::Util::handle_meta_request<L4::Factory>(ios);
    case L4::Factory::Protocol:
      return disp_factory(obj, ios);

    default:
      return -L4_EBADPROTO;
    }
}


Allocator *
Allocator::root_allocator()
{
  static Allocator ra(300000);
  return &ra;
}
