/*
 * (c) 2011 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>
#include <l4/vbus/vbus_types.h>

__BEGIN_DECLS

enum {
  L4VBUS_GPIO_SETUP_INPUT  = 0x100,
  L4VBUS_GPIO_SETUP_OUTPUT = 0x200,
  L4VBUS_GPIO_SETUP_IRQ    = 0x300,
};


int L4_CV
l4vbus_gpio_setup(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                  unsigned pin, unsigned mode, int outvalue);

int L4_CV
l4vbus_gpio_config_pad(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                       unsigned pin, unsigned func, unsigned value);

int L4_CV
l4vbus_gpio_get(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                unsigned pin);

int L4_CV
l4vbus_gpio_set(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                unsigned pin, int value);

int L4_CV
l4vbus_gpio_multi_setup(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                        unsigned mask, unsigned mode, unsigned value);

int L4_CV
l4vbus_gpio_multi_config_pad(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                              unsigned mask, unsigned func, unsigned value);

int L4_CV
l4vbus_gpio_multi_get(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                      unsigned *data);

int L4_CV
l4vbus_gpio_multi_set(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                      unsigned mask, unsigned data);

int L4_CV
l4vbus_gpio_to_irq(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                   unsigned pin);

__END_DECLS
