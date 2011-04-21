/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/protocols>

#include <l4/cxx/ipc_server>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>

#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/dataspace-sys.h>

#include <l4/re/error_helper>

#include <l4/vbus/vbus_types.h>
#include <l4/vbus/vdevice-ops.h>

#include <cstdio>

#include "debug.h"
#include "hw_msi.h"
#include "vbus.h"
#include "vmsi.h"
#include "vicu.h"
#include "server.h"
#include "res.h"
#include "cfg.h"
#include "vbus_factory.h"

namespace {

class Root_irq_rs : public Resource_space
{
private:
  Vi::System_bus *_bus;
  Vi::Sw_icu *_icu;

public:
  Root_irq_rs(Vi::System_bus *bus) : Resource_space(), _bus(bus), _icu(0)
  {}

  bool request(Resource *parent, Device *, Resource *child, Device *)
  {
    // printf("VBUS: IRQ resource request: "); child->dump();
    Adr_resource *r = dynamic_cast<Adr_resource*>(child);
    if (!r || !parent)
      return false;

    if (!_icu)
      {
	_icu = new Vi::Sw_icu();
	_bus->add_child(_icu);
      }

    d_printf(DBG_DEBUG2, "Add IRQ resources to vbus: ");
    if (dlevel(DBG_DEBUG2))
      child->dump();

    _icu->add_irqs(r);
    _bus->resource_set()->insert(r);

    return true;
  };

  bool alloc(Resource *parent, Device *, Resource *child, Device *, bool)
  {
    d_printf(DBG_DEBUG2, "Allocate virtual IRQ resource ...\n");
    if (dlevel(DBG_DEBUG2))
      child->dump();

    Vi::Msi_resource *msi = dynamic_cast<Vi::Msi_resource*>(child);
    if (!msi || !parent)
      return false;

    d_printf(DBG_DEBUG2, "  Allocate Virtual MSI...\n");

    if (!_icu)
      {
	_icu = new Vi::Sw_icu();
	_bus->add_child(_icu);
      }

    int nr = _icu->alloc_irq(msi->flags(), msi->hw_msi());
    if (nr < 0)
      {
	d_printf(DBG_ERR, "ERROR: cannot allocate MSI resource\n");
	return false;
      }

    msi->start_end(nr, nr);
    msi->del_flags(Resource::F_disabled);

    if (dlevel(DBG_DEBUG2))
      {
	msi->dump(4);
	msi->hw_msi()->dump(4);
      }

    _bus->resource_set()->insert(msi);
    return true;
  }

  ~Root_irq_rs() {}
};

class Root_x_rs : public Resource_space
{
private:
  Vi::System_bus *_bus;

public:
  Root_x_rs(Vi::System_bus *bus) : Resource_space(), _bus(bus)
  {}

  bool request(Resource *parent, Device *, Resource *child, Device *)
  {
    //printf("VBUS: X resource request: "); child->dump();
    Adr_resource *r = dynamic_cast<Adr_resource*>(child);
    if (!r || !parent)
      return false;


    _bus->resource_set()->insert(r);
    return true;
  }

  bool alloc(Resource *, Device *, Resource *, Device *, bool)
  { return false; }

  ~Root_x_rs() {}
};
}



namespace Vi {

bool
System_bus::resource_allocated(Resource const *_r) const
{
  Adr_resource const *r = dynamic_cast<Adr_resource const *>(_r);
  if (!r)
    return false;

  Resource_set::const_iterator i = _resources.find(const_cast<Adr_resource*>(r));
  if (i == _resources.end())
    return false;

  if ((*i)->data().start() <= r->data().start()
      && (*i)->data().end() >= r->data().end())
    return true;

  return false;
}


System_bus::System_bus()
{
  add_feature(this);
  add_resource(new Root_resource(Resource::Irq_res, new Root_irq_rs(this)));
  Resource_space *x = new Root_x_rs(this);
  add_resource(new Root_resource(Resource::Mmio_res, x));
  add_resource(new Root_resource(Resource::Mmio_res | Resource::F_prefetchable, x));
  add_resource(new Root_resource(Resource::Io_res, x));
}

System_bus::~System_bus()
{
  registry->unregister_obj(this);
  // FIXME: must delete all devices
}


void
System_bus::dump_resources() const
{
  for (Resource_set::const_iterator i = _resources.begin(); i != _resources.end(); ++i)
    (*i)->dump();
}

int
System_bus::request_resource(L4::Ipc_iostream &ios)
{
  l4vbus_resource_t res;
  ios.get(res);

  ::Adr_resource ires(res.type, res.start, res.end);
  if (dlevel(DBG_DEBUG2))
    {
      printf("request resource: ");
      Adr_resource(ires).dump();
      puts("");
    }

  Resource_set::const_iterator i = _resources.find(&ires);
#if 0
  for (Resource_set::Const_iterator m = _resources.begin(); m != _resources.end(); ++m)
    {
      m->dump();
      puts("");
    }
#endif

  if (i == _resources.end() || !(*i)->contains(ires))
    return -L4_ENOENT;

#if 0
  if (Io_config::cfg->verbose() > 1)
    {
      printf("  found resource: ");
      i->dump();
      puts("");
    }
#endif

  if (res.type == L4VBUS_RESOURCE_PORT)
    {
      l4_uint64_t sz = res.end + 1 - res.start;

      int szl2 = 0;
      while ((1UL << szl2) < sz)
	++szl2;

      if ((1UL << szl2) > sz)
	--szl2;

      ios << L4::Snd_fpage::io(res.start, szl2, L4_FPAGE_RWX);
      return L4_EOK;
    }


  return -L4_ENOENT;
}

int
System_bus::request_iomem(L4::Ipc_iostream &ios)
{
  L4::Opcode op;
  ios >> op;
  switch (op)
    {
    case L4Re::Dataspace_::Map:
	{
	  l4_addr_t offset, spot;
	  unsigned long flags;
	  ios >> offset >> spot >> flags;

//	  printf("map iomem: %lx...\n", offset);
	  Adr_resource pivot(L4VBUS_RESOURCE_MEM, offset, offset);
	  Resource_set::iterator r = _resources.find(&pivot);

	  if (r == _resources.end())
	    return -L4_ERANGE;

	  offset = l4_trunc_page(offset);

	  l4_addr_t st = l4_trunc_page((*r)->start());
	  l4_addr_t adr = (*r)->map_iomem();

          if (!adr)
            return -L4_ENOMEM;

          adr = l4_trunc_page(adr);

          l4_addr_t addr = offset - st + adr;
          unsigned char order
            = l4_fpage_max_order(L4_PAGESHIFT,
                                 addr, addr, addr + (*r)->size(), spot);

          // we also might want to do WB instead of UNCACHED...
          ios << L4::Snd_fpage::mem(l4_trunc_size(addr, order), order,
                                    L4_FPAGE_RWX, l4_trunc_page(spot),
                                    L4::Snd_fpage::Map,
                                    L4::Snd_fpage::Uncached);
	  return L4_EOK;
	}
    }
  return -L4_ENOSYS;
};

int
System_bus::dispatch(l4_umword_t obj, L4::Ipc_iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  if (tag.label() == 0)
    {
      l4vbus_device_handle_t devid;
      l4_uint32_t func;
      ios >> devid >> func;
      Device *dev = get_dev_by_id(devid);
      if (!dev)
	return -L4_ENODEV;
      return dev->vdevice_dispatch(obj, func, ios);
    }

  if (tag.label() == L4Re::Protocol::Dataspace)
    return request_iomem(ios);

  return -L4_EBADPROTO;

}

int
System_bus::dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc_iostream &ios)
{
  switch (func)
    {
    case L4vbus_vbus_request_resource:
      return request_resource(ios);
    default:
      return -L4_ENOSYS;
    }
}


static Dev_factory_t<System_bus> __sb_root_factory("System_bus");

}
