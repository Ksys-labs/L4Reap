/*
 * (c) 2011 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <cstdio>
#include <cstdlib>

#include "debug.h"
#include "pci.h"
#include "acpi_l4.h"
#include "__acpi.h"
#include "cfg.h"
#include "main.h"
#include "phys_space.h"

extern "C" {
#include "acpi.h"
#include "accommon.h"
#include "acresrc.h"
#include "acnamesp.h"
}

#include <errno.h>
#include <l4/cxx/list>

#define _COMPONENT		ACPI_BUS_COMPONENT
ACPI_MODULE_NAME("l4main");

namespace {

struct Prt_entry : public cxx::List_item
{
  unsigned slot;
  unsigned char pin;
  acpica_pci_irq irq;
};

class Acpi_pci_irq_router_rs : public Resource_space
{
public:
  Prt_entry *_prt;

public:
  Acpi_pci_irq_router_rs() : _prt(0) {}

  int add_prt_entry(ACPI_HANDLE obj, ACPI_PCI_ROUTING_TABLE *e);
  int find(int device, int pin, struct acpica_pci_irq **irq);
  bool request(Resource *parent, Device *, Resource *child, Device *cdev);
  bool alloc(Resource *, Device *, Resource *, Device *, bool)
  { return false; }
};


bool
Acpi_pci_irq_router_rs::request(Resource *parent, Device *,
                                Resource *child, Device *cdev)
{
  if (dlevel(DBG_ALL))
    {
      printf("requesting IRQ resource: ");
      cdev->dump(2);
      child->dump(2);
      printf(" at ACPI IRQ routing resource\n");
    }

  Hw::Device *cd = dynamic_cast<Hw::Device*>(cdev);

  if (!cd)
    return false;

  struct acpica_pci_irq *irq = 0;

  if (find(cd->adr() >> 16, child->start(), &irq) < 0)
    return false;

  if (!irq)
    return false;

  child->del_flags(Resource::F_relative);
  child->start(irq->irq);
  child->del_flags(Resource::Irq_info_base * 3);
  unsigned flags = 0;
  flags |= (!irq->trigger) * Resource::Irq_info_base;
  flags |= (!!irq->polarity) * Resource::Irq_info_base * 2;
  child->add_flags(flags);

  child->parent(parent);

  return true;
}

enum Acpi_irq_model_id {
	ACPI_IRQ_MODEL_PIC = 0,
	ACPI_IRQ_MODEL_IOAPIC,
	ACPI_IRQ_MODEL_IOSAPIC,
	ACPI_IRQ_MODEL_PLATFORM,
	ACPI_IRQ_MODEL_COUNT
};

static ACPI_STATUS
get_irq_cb(ACPI_RESOURCE *res, void *ctxt)
{
  acpica_pci_irq *irq = (acpica_pci_irq*)ctxt;
  if (!res)
    return AE_OK;

  switch (res->Type)
    {
    case ACPI_RESOURCE_TYPE_IRQ:
      irq->irq = res->Data.Irq.Interrupts[0];
      irq->polarity = res->Data.Irq.Polarity;
      irq->trigger  = res->Data.Irq.Triggering;
      return AE_OK;

    case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
      irq->irq = res->Data.ExtendedIrq.Interrupts[0];
      irq->polarity = res->Data.ExtendedIrq.Polarity;
      irq->trigger  = res->Data.ExtendedIrq.Triggering;
      return AE_OK;

    default:
      return AE_OK;
    }
}

int
Acpi_pci_irq_router_rs::add_prt_entry(ACPI_HANDLE obj,
                                      ACPI_PCI_ROUTING_TABLE *e)
{
  if (!e)
    return -EINVAL;

  Prt_entry *ne = new Prt_entry();
  if (!ne)
    return -ENOMEM;

  ne->slot = (e->Address >> 16) & 0xffff;
  ne->pin = e->Pin;

  ne->irq.irq = e->SourceIndex;
  ne->irq.polarity = ACPI_ACTIVE_LOW;
  ne->irq.trigger = ACPI_LEVEL_SENSITIVE;
  if (e->Source[0])
    {
      ACPI_HANDLE link;
      d_printf(DBG_DEBUG, " (dev[%s][%d]) ", e->Source, e->SourceIndex);
      ACPI_STATUS status;
      status = AcpiGetHandle(obj, e->Source, &link);
      if (ACPI_FAILURE(status))
	{
	  d_printf(DBG_WARN, "\nWARNING: Could not find PCI IRQ Link Device...\n");
	  return -ENODEV;
	}

      status = AcpiWalkResources(link, (char*)"_CRS", get_irq_cb, &ne->irq);
      if (ACPI_FAILURE(status))
	{
	  d_printf(DBG_WARN, "\nWARNING: Could not evaluate _CRS of PCI IRQ Link Device\n");
	  return -ENODEV;
	}
    }

  _prt = cxx::List_item::push_back(_prt, ne);
  return 0;
}

int
Acpi_pci_irq_router_rs::find(int device, int pin, struct acpica_pci_irq **irq)
{
  Prt_entry::T_iter<Prt_entry> c = _prt;
  while (*c)
    {
      if (c->slot == (unsigned)device && c->pin == pin)
	{
	  *irq = &c->irq;
	  return 0;
	}

      ++c;
    }

  return -ENODEV;
}


static int acpi_bus_init_irq(void)
{
  ACPI_STATUS status = AE_OK;
  ACPI_OBJECT arg = { ACPI_TYPE_INTEGER };
  ACPI_OBJECT_LIST arg_list = { 1, &arg };
  char const *message = NULL;


  //int acpi_irq_model = ACPI_IRQ_MODEL_PIC;
  int acpi_irq_model = ACPI_IRQ_MODEL_IOAPIC;
  /*
   * Let the system know what interrupt model we are using by
   * evaluating the \_PIC object, if exists.
   */

  switch (acpi_irq_model) {
    case ACPI_IRQ_MODEL_PIC:
      message = "PIC";
      break;
    case ACPI_IRQ_MODEL_IOAPIC:
      message = "IOAPIC";
      break;
    case ACPI_IRQ_MODEL_IOSAPIC:
      message = "IOSAPIC";
      break;
    case ACPI_IRQ_MODEL_PLATFORM:
      message = "platform specific model";
      break;
    default:
      d_printf(DBG_ERR, "ERROR: Unknown interrupt routing model\n");
      return -1;
  }

  d_printf(DBG_INFO, "Using %s for interrupt routing\n", message);

  arg.Integer.Value = acpi_irq_model;

  status = AcpiEvaluateObject(NULL, (char*)"\\_PIC", &arg_list, NULL);
  if (ACPI_FAILURE(status) && (status != AE_NOT_FOUND)) {
      ACPI_EXCEPTION((AE_INFO, status, "Evaluating _PIC"));
      return -1;
  }

  return 0;
}


struct Discover_ctxt
{
  Hw::Device *last_device;
  Hw::Device *current_bus;
  unsigned level;
};

class Acpi_res_discover : public Hw::Discover_res_if
{
public:
  Acpi_res_discover(ACPI_HANDLE obj) : obj(obj) {}

  void discover_resources(Hw::Device *host);
  void setup_resources(Hw::Device *) {}

private:
  void discover_prt(Hw::Device *host);
  void discover_crs(Hw::Device *host);

  ACPI_HANDLE obj;
};

void
Acpi_res_discover::discover_prt(Hw::Device *host)
{
  ACPI_BUFFER buf;
  ACPI_STATUS status;
  ACPI_HANDLE handle;

  status = AcpiGetHandle(obj, (char*)"_PRT", &handle);

  // no PRT!!
  if (ACPI_FAILURE(status))
    {
      Resource *r = new Pci_irq_router_res<Pci_pci_bridge_irq_router_rs>();
      host->add_resource(r);
      return;
    }

#if 0
  ret_buf.Length = sizeof (buffer);
  ret_buf.Pointer = buffer;

  status = AcpiGetName (obj, ACPI_FULL_PATHNAME, &ret_buf);

  if (ACPI_FAILURE (status))
    AcpiOsPrintf ("Could not convert name to pathname\n");
  else
    AcpiOsPrintf ("ACPI: PCI IRQ routing [%s._PRT]\n", buffer);
#endif

  buf.Length = ACPI_ALLOCATE_BUFFER;

  status = AcpiGetIrqRoutingTable(obj, &buf);

  if (ACPI_FAILURE(status))
    {
      d_printf(DBG_ERR, "ERROR: while getting PRT for [%s]\n", "buffer");
      Resource *r = new Pci_irq_router_res<Pci_pci_bridge_irq_router_rs>();
      host->add_resource(r);
      return;
    }

  typedef Pci_irq_router_res<Acpi_pci_irq_router_rs> Irq_res;
  Irq_res *r = new Irq_res();

  char *p = (char*)buf.Pointer;
  char *e = (char*)buf.Pointer + buf.Length;
  while (1)
    {
      ACPI_PCI_ROUTING_TABLE *prt = (ACPI_PCI_ROUTING_TABLE *)p;
      if (prt->Length == 0)
	break;

      if (p + prt->Length > e)
	break;

      int err = r->provided()->add_prt_entry(obj, prt);
      if (err < 0)
	return;

      p += prt->Length;
    }

  host->add_resource(r);
}

static unsigned acpi_adr_t_to_f(unsigned art)
{
  switch (art)
    {
    case ACPI_MEMORY_RANGE: return Resource::Mmio_res;
    case ACPI_IO_RANGE: return Resource::Io_res;
    case ACPI_BUS_NUMBER_RANGE: return Resource::Bus_res;
    default: return ~0;
    }
}

static void
acpi_adr_res(Hw::Device *host, ACPI_RESOURCE_ADDRESS const *ar, l4_uint64_t s, l4_uint64_t l,
             bool qw)
{
  unsigned flags = acpi_adr_t_to_f(ar->ResourceType)
                   | Resource::F_fixed_size | Resource::F_fixed_addr;

  if (flags == ~0U)
    return;

  if (qw)
    flags |= Resource::F_width_64bit;

  Resource *r;
  if (ar->ProducerConsumer == ACPI_PRODUCER)
    r = new Resource_provider(flags, s, s + l - 1);
  else
    r = new Resource(flags, s, s + l -1);

  host->add_resource(r);
}

void
Acpi_res_discover::discover_crs(Hw::Device *host)
{
  ACPI_BUFFER buf;
  buf.Length = ACPI_ALLOCATE_BUFFER;

  if (ACPI_FAILURE(AcpiGetCurrentResources(obj, &buf)))
    return;

  char const *p = (char const *)buf.Pointer;
  while (p)
    {
      ACPI_RESOURCE const *r = (ACPI_RESOURCE const *)p;
      ACPI_RESOURCE_DATA const *d = &r->Data;
      unsigned flags = 0;

      switch (r->Type)
	{
	case ACPI_RESOURCE_TYPE_END_TAG:
	  AcpiOsFree(buf.Pointer);
	  return;

	case ACPI_RESOURCE_TYPE_IRQ:
	  flags = Resource::Irq_res;
	  flags |= (!d->Irq.Triggering) * Resource::Irq_info_base;
	  flags |= (!!d->Irq.Polarity) * Resource::Irq_info_base * 2;
	  for (unsigned c = 0; c < d->Irq.InterruptCount; ++c)
	    host->add_resource(new Resource(flags, d->Irq.Interrupts[c],
		                                d->Irq.Interrupts[c]));
	  break;

	case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
	  flags = Resource::Irq_res;
	  flags |= (!d->ExtendedIrq.Triggering) * Resource::Irq_info_base;
	  flags |= (!!d->ExtendedIrq.Polarity) * Resource::Irq_info_base * 2;
	  if (d->ExtendedIrq.ResourceSource.StringPtr)
	    {
	      d_printf(DBG_DEBUG2, "hoo indirect IRQ resource found src=%s idx=%d\n",
		      d->ExtendedIrq.ResourceSource.StringPtr,
		      d->ExtendedIrq.ResourceSource.Index);
	    }
	  else
	    {
	      for (unsigned c = 0; c < d->ExtendedIrq.InterruptCount; ++c)
		host->add_resource(new Resource(flags, d->ExtendedIrq.Interrupts[c],
		      d->ExtendedIrq.Interrupts[c]));
	    }
	  break;

	case ACPI_RESOURCE_TYPE_IO:
	  flags = Resource::Io_res | Resource::F_fixed_size | Resource::F_fixed_addr;
	  host->add_resource(new Resource(flags, d->Io.Minimum,
		                          d->Io.Minimum + d->Io.AddressLength - 1));
	  break;

	case ACPI_RESOURCE_TYPE_FIXED_IO:
	  flags = Resource::Io_res | Resource::F_fixed_size | Resource::F_fixed_addr;
	  host->add_resource(new Resource(flags, d->FixedIo.Address,
		                          d->FixedIo.Address + d->FixedIo.AddressLength - 1));
	  break;

	case ACPI_RESOURCE_TYPE_MEMORY24:
	  flags = Resource::Mmio_res | Resource::F_fixed_size | Resource::F_fixed_addr;
	  host->add_resource(new Resource(flags, d->Memory24.Minimum,
		                          d->Memory24.Minimum + d->Memory24.AddressLength - 1));
	  break;

	case ACPI_RESOURCE_TYPE_MEMORY32:
	  flags = Resource::Mmio_res | Resource::F_fixed_size | Resource::F_fixed_addr;
	  host->add_resource(new Resource(flags, d->Memory32.Minimum,
		                          d->Memory32.Minimum + d->Memory32.AddressLength - 1));
	  break;

	case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:
	  flags = Resource::Mmio_res | Resource::F_fixed_size | Resource::F_fixed_addr;
	  host->add_resource(new Resource(flags, d->FixedMemory32.Address,
		               d->FixedMemory32.Address + d->FixedMemory32.AddressLength - 1));
	  break;

	case ACPI_RESOURCE_TYPE_ADDRESS16:
	  acpi_adr_res(host, &d->Address, d->Address16.Minimum, d->Address16.AddressLength, 0);
	  break;

	case ACPI_RESOURCE_TYPE_ADDRESS32:
	  acpi_adr_res(host, &d->Address, d->Address32.Minimum, d->Address32.AddressLength, 0);
	  break;

	case ACPI_RESOURCE_TYPE_ADDRESS64:
	  acpi_adr_res(host, &d->Address, d->Address64.Minimum, d->Address64.AddressLength, 1);
	  break;

	default:
	  d_printf(DBG_WARN, "WARNING: ignoring ACPI recource (unkown type: %d)\n", r->Type);
	  break;


	}

      p += r->Length;
    }

  AcpiOsFree(buf.Pointer);
}

void
Acpi_res_discover::discover_resources(Hw::Device *host)
{
  Pci_bridge *bridge = dynamic_cast<Pci_bridge*>(host->discover_bus_if());
  if (bridge)
    discover_prt(host);

  discover_crs(host);

  if (bridge)
    {
      for (Resource_list::const_iterator i = host->resources()->begin();
	  i != host->resources()->end(); ++i)
	{
	  if ((*i)->type() == Resource::Bus_res)
	    bridge->num = bridge->subordinate = (*i)->start();
	}
    }
}


static l4_uint32_t
get_adr(ACPI_HANDLE dev)
{
  ACPI_HANDLE adr;
  if (ACPI_FAILURE(AcpiGetHandle(dev, const_cast<char*>("_ADR"), &adr)))
    return ~0U;

  ACPI_OBJECT adro;
  ACPI_BUFFER ret_buf;
  ret_buf.Pointer = &adro;
  ret_buf.Length = sizeof(adro);
  if (ACPI_SUCCESS(AcpiEvaluateObject(adr, NULL, NULL, &ret_buf)))
    {
      switch (adro.Type)
	{
	case ACPI_TYPE_INTEGER:
	  return adro.Integer.Value;
	default:
	  return ~0;
	}
    }

  return ~0;
}

static void
get_name(ACPI_HANDLE dev, Hw::Device *hd)
{
  char str[5];
  ACPI_BUFFER buf;
  buf.Pointer = &str;
  buf.Length = sizeof(str);
  if (ACPI_SUCCESS(AcpiGetName(dev, ACPI_SINGLE_NAME, &buf)))
    hd->set_name(str);
}


static ACPI_STATUS
discover_pre_cb(ACPI_HANDLE obj, UINT32 nl, void *ctxt, void **)
{
  Discover_ctxt *c = reinterpret_cast<Discover_ctxt*>(ctxt);

  if (nl == 1)
    return AE_OK;

  if (nl > c->level)
    {
      c->current_bus = c->last_device;
      c->level = nl;
    }

  l4_uint32_t adr = get_adr(obj);

  Hw::Device *nd = c->current_bus->get_child_dev_uid((l4_umword_t)obj, adr, true);
  c->last_device = nd;

  get_name(obj, nd);

  ACPI_PNP_DEVICE_ID *hid = 0;
  ACPI_PNP_DEVICE_ID_LIST *cid = 0;
  bool pci_rb = false;
#if 0
  if (ACPI_FAILURE(AcpiUtAcquireMutex(ACPI_MTX_NAMESPACE)))
    return AE_OK;
#endif

  ACPI_NAMESPACE_NODE *node = AcpiNsValidateHandle(obj);
  if (!node)
    {
      //AcpiUtReleaseMutex(ACPI_MTX_NAMESPACE);
      return AE_OK;
    }

  if (ACPI_SUCCESS(AcpiUtExecute_HID(node, &hid)))
    {
      nd->set_hid(hid->String);
      pci_rb |= AcpiUtIsPciRootBridge(hid->String);
      ACPI_FREE(hid);
    }

  if (ACPI_SUCCESS(AcpiUtExecute_CID(node, &cid)))
    {
      for (unsigned i = 0; i < cid->Count; ++i)
	{
	  nd->add_cid(cid->Ids[i].String);
          pci_rb |= AcpiUtIsPciRootBridge(cid->Ids[i].String);
	}
      ACPI_FREE(cid);
    }


  //AcpiUtReleaseMutex(ACPI_MTX_NAMESPACE);

  // hm, this seems very specific for PCI
  if (pci_rb)
    {
      d_printf(DBG_DEBUG, "Found PCI root bridge...\n");
      if (Pci_root_bridge *rb = pci_root_bridge(0))
	{
	  if (rb->host())
	    {
	      // we found a second root bridge
	      // create a new root pridge instance
	      rb = new Pci_port_root_bridge(-1, nd);
	    }
	  else
	    rb->set_host(nd);

	  nd->set_discover_bus_if(rb);
	}
      else
	d_printf(DBG_ERR, "ERROR: there is no PCI bus driver for this platform\n");
    }

  nd->add_resource_discoverer(new Acpi_res_discover(obj));

  return AE_OK;
}

static ACPI_STATUS
discover_post_cb(ACPI_HANDLE, UINT32 nl, void *ctxt, void **)
{
  Discover_ctxt *c = reinterpret_cast<Discover_ctxt*>(ctxt);
  if (nl < c->level)
    {
      c->level = nl;
      c->current_bus = c->current_bus->parent();
    }
  return AE_OK;
}

}


int acpica_init()
{
  d_printf(DBG_INFO, "Hello from L4-ACPICA\n");

  pci_register_root_bridge(0, new Pci_port_root_bridge(0, 0));

  AcpiDbgLevel =
      ACPI_LV_INIT
    //| ACPI_LV_INFO
   // | ACPI_LV_FUNCTIONSkern/irq.cpp
   //     | ACPI_LV_ALL_EXCEPTIONS 
    //    | ACPI_LV_LOAD 
    //    | ACPI_LV_INIT_NAMES 
        | ACPI_LV_TABLES 
    //    | ACPI_LV_RESOURCES 
    //    | ACPI_LV_NAMES
    //    | ACPI_LV_VALUES 
    //    | ACPI_LV_OPREGION  
        | ACPI_LV_VERBOSE_INFO 
    //    | ACPI_LV_PARSE
    //    | ACPI_LV_DISPATCH
    //    | ACPI_LV_EXEC
    //    | ACPI_LV_IO
    ;

//0. enable workarounds, see include/acglobals.h
  AcpiGbl_EnableInterpreterSlack = (1==1);
  ACPI_STATUS status;

  status = AcpiInitializeSubsystem();
  if(status!=AE_OK)
    return status;

  status = AcpiInitializeTables (0, 0, TRUE);
  if(status!=AE_OK)
    return status;

  status = AcpiReallocateRootTable ();
//  if(status!=AE_OK)
//    return status;

  status = AcpiLoadTables ();

  if(ACPI_FAILURE(status))
    return status;

  d_printf(DBG_DEBUG, "enable ACPI subsystem\n");
  status = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);

  if (ACPI_FAILURE(status))
    {
      d_printf(DBG_ERR, "Unable to start the ACPI Interpreter\n");
      exit(status);
    }

  d_printf(DBG_DEBUG, "initialize ACPI objects\n");
  status = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
  if (ACPI_FAILURE(status)) {
      d_printf(DBG_ERR, "Unable to initialize ACPI objects\n");
      exit(status);
  }

  d_printf(DBG_DEBUG, "Interpreter enabled\n");

  Discover_ctxt c;
  c.last_device = system_bus();
  c.current_bus = system_bus();
  c.level = 1;


  d_printf(DBG_DEBUG, "scanning for PCI root bridge\n");
  status = AcpiWalkNamespace(ACPI_TYPE_DEVICE, ACPI_ROOT_OBJECT,
                             ACPI_UINT32_MAX,
                             discover_pre_cb, discover_post_cb, &c, 0);

  /*
   * Get the system interrupt model and evaluate \_PIC.
   */
  int result = acpi_bus_init_irq();
  if (result)
    {
      d_printf(DBG_ERR, "Could not initialize ACPI IRQ stuff\n");
      exit(1);
    }
  status = AcpiSubsystemStatus();

  if (ACPI_FAILURE(status))
      exit(status);

  d_printf(DBG_INFO, "ACPI subsystem initialized\n");

  ACPI_BUFFER ret_buffer;
  ret_buffer.Length = ACPI_ALLOCATE_BUFFER;

  status = AcpiGetSystemInfo(&ret_buffer);

  if(ACPI_FAILURE(status))
    exit(status);

  acpi_print_system_info(ret_buffer.Pointer);

  AcpiOsFree(ret_buffer.Pointer);

  return 0;
}

