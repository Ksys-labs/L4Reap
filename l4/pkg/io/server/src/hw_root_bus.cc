/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "hw_root_bus.h"
#include "phys_space.h"
#include "resource.h"
#include "cfg.h"

namespace {

// --- Root address space for IRQs -----------------------------------------
class Root_irq_rs : public Resource_space
{
public:
  bool request(Resource *parent, Device *, Resource *child, Device *)
  {
    child->parent(parent);

    return true;
  };

  bool alloc(Resource *, Device *, Resource *, Device *, bool)
  { return false; }

  ~Root_irq_rs() {}
};

// --- Root address space for IO-Ports --------------------------------------
class Root_io_rs : public Resource_space
{
public:
  bool request(Resource *parent, Device *, Resource *child, Device *)
  {
    child->parent(parent);

    return true;
  }

  bool alloc(Resource *parent, Device *, Resource *child, Device *, bool)
  {
    child->parent(parent);

    return true;
  }

  ~Root_io_rs() {}
};


// --- Root address space for MMIO -----------------------------------------
class Root_mmio_rs : public Resource_space
{
public:
  bool request(Resource *parent, Device *, Resource *child, Device *);
  bool alloc(Resource *parent, Device *, Resource *child, Device *, bool);
  ~Root_mmio_rs() {}
};

bool
Root_mmio_rs::request(Resource *parent, Device *, Resource *child, Device *)
{
  //printf("request resource at root level: "); child->dump();
  Adr_resource *x = dynamic_cast<Adr_resource*>(child);

  if (x
      && Phys_space::space.alloc(Phys_space::Phys_region(x->start(), x->end())))
    {
      child->parent(parent);
      return true;
    }

#if 1
  printf("WARNING: phys mmio resource allocation failed\n");
  child->dump();
#endif
  return false;
}


bool
Root_mmio_rs::alloc(Resource *parent, Device *, Resource *child, Device *,
                    bool /*resize*/)
{

  Adr_resource *cld = dynamic_cast<Adr_resource *>(child);

  if (!cld)
    return false;

  Adr_resource::Size align = cxx::max<Adr_resource::Size>(cld->alignment(),  L4_PAGESIZE - 1);
  Phys_space::Phys_region phys = Phys_space::space.alloc(cld->size(), align);
  if (!phys.valid())
    {
#if 0
      printf("ERROR: could not reserve physical space for resource\n");
      r->dump();
#endif
      cld->disable();
      return false;
    }

  cld->start(phys.start());
  child->parent(parent);

  if (Io_config::cfg->verbose())
    {
      printf("allocated resource: ");
      cld->dump();
    }
  return true;
}

// --- End Root address space for MMIO --------------------------------------
}

namespace Hw {

Root_bus::Root_bus(char const *name)
: Hw::Device(), _name(name)
{
  // add root resource for IRQs
  Root_resource *r = new Root_resource(Resource::Irq_res, new Root_irq_rs());
  add_resource(r);

  Resource_space *rs_mmio = new Root_mmio_rs();
  // add root resource for non-prefetchable MMIO resources
  r = new Root_resource(Resource::Mmio_res, rs_mmio);
  r->add_flags(Adr_resource::F_width_64bit);
  add_resource(r);

  // add root resource for prefetchable MMIO resources
  r = new Root_resource(Resource::Mmio_res | Resource::F_prefetchable, rs_mmio);
  r->add_flags(Adr_resource::F_width_64bit);
  add_resource(r);

  // add root resource for IO ports
  r = new Root_resource(Resource::Io_res, new Root_io_rs());
  add_resource(r);
}

}
