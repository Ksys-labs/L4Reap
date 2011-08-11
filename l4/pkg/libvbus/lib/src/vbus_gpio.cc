/*
 * (c) 2009 Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/ipc_stream>
#include <l4/vbus/vbus_generic>
#include <l4/vbus/vbus_gpio.h>
#include <l4/vbus/vdevice-ops.h>

int L4_CV
l4vbus_gpio_write(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                  int pin, int val)
{
  L4::Ipc::Iostream s(l4_utcb());
  l4vbus_device_msg(handle, L4vbus_gpio_write, s);
  s << pin << val;
  int err = l4_error(s.call(vbus));
  return err;
}

int L4_CV
l4vbus_gpio_read(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                 int  pin, int *val)
{
  L4::Ipc::Iostream s(l4_utcb());
  l4vbus_device_msg(handle, L4vbus_gpio_read, s);
  s << pin;
  int err = l4_error(s.call(vbus));
  s >> *val;
  return err;
}
