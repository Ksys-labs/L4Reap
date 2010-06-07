/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "vdevice.h"
#include "vpci.h"

// ------------------------------------------------------------------------
// Proxy for a real PCI root bridge
// ------------------------------------------------------------------------

class VPci_root : public VDevice
{
public:

  int vbus_dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc_iostream &ios);

  explicit VPci_root() : VDevice() {}

  ~VPci_root() {}

  char const *hid() const
  {
    return "PNP0A03";
  }

public:
  int cfg_read(L4::Ipc_iostream &ios);
  int cfg_write(L4::Ipc_iostream &ios);
  int irq_enable(L4::Ipc_iostream &ios);
};


// IMPLEMENTATION --------------------------------------------------------

int
VPci_root::cfg_read(L4::Ipc_iostream &ios)
{
  l4_uint32_t bus;
  l4_uint32_t devfn;
  l4_uint32_t reg;
  l4_uint32_t value;
  l4_uint32_t width;

  ios >> bus >> devfn >> reg >> width;

  int res = pci_root_bridge(0)->cfg_read(bus, devfn, reg, &value, Pci::cfg_w_to_o(width));
  if (res < 0)
    return res;

  ios << value;
  return L4_EOK;
}

int
VPci_root::irq_enable(L4::Ipc_iostream &ios)
{
#if 0
  l4_uint32_t bus;
  l4_uint32_t devfn;
  int pin;

  ios >> bus >> devfn >> pin;
  // printf("get IRQ for %02x:%02x.%x: pin INT%c\n", bus, devfn >> 16, devfn & 0xffff, pin + 'A');
  struct acpica_pci_irq *irq = 0;
  int res = acpica_pci_irq_find(0, bus, devfn >> 16, pin, &irq);
  if (res < 0)
    {
      ios << (int)-1;
      return res;
    }

  if (!irq)
    {
      ios << (int)-1;
      return -L4_EINVAL;
    }

  ios << irq->irq << irq->trigger << irq->polarity;
#endif
  return L4_EOK;
}

int
VPci_root::cfg_write(L4::Ipc_iostream &ios)
{
  l4_uint32_t bus;
  l4_uint32_t devfn;
  l4_uint32_t reg;
  l4_uint32_t value;
  l4_uint32_t width;

  ios >> bus >> devfn >> reg >> value >> width;

  int res = pci_root_bridge(0)->cfg_write(bus, devfn, reg, value, Pci::cfg_w_to_o(width));
  if (res < 0)
    return res;

  return L4_EOK;
}

int
VPci_root::vbus_dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc_iostream &ios)
{
  switch (func)
    {
    case 0: return cfg_read(ios);
    case 1: return cfg_write(ios);
    case 2: return irq_enable(ios);
    default: return -L4_ENOSYS;
    }
}

