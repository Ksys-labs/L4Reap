/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/vbus/vbus_pci.h>
#include <l4/vbus/vbus_generic>
#include <l4/cxx/ipc_stream>

int L4_CV
l4vbus_pci_cfg_read(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                    l4_uint32_t bus, l4_uint32_t devfn,
                    l4_uint32_t reg, l4_uint32_t *value, l4_uint32_t width)
{
  L4::Ipc::Iostream s(l4_utcb());
  l4vbus_device_msg(handle, 0, s);
  s << bus << devfn << reg << width;
  int err = l4_error(s.call(vbus));
  if (err < 0)
    return err;
  s >> *value;
  return 0;
}

int L4_CV
l4vbus_pci_cfg_write(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                     l4_uint32_t bus, l4_uint32_t devfn,
                     l4_uint32_t reg, l4_uint32_t value, l4_uint32_t width)
{
  L4::Ipc::Iostream s(l4_utcb());
  l4vbus_device_msg(handle, 1, s);
  s << bus << devfn << reg << value << width;
  return l4_error(s.call(vbus));
}

int L4_CV
l4vbus_pci_irq_enable(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                      l4_uint32_t bus, l4_uint32_t devfn,
                      int pin, unsigned char *trigger, unsigned char *polarity)
{
  int irq;
  L4::Ipc::Iostream s(l4_utcb());
  l4vbus_device_msg(handle, 2, s);
  s << bus << devfn << pin;
  int err = l4_error(s.call(vbus));
  if (err < 0)
    return err;

  s >> irq >> *trigger >> *polarity;
  return irq;
}


