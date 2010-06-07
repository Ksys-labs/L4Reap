/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/compiler.h>
#include <l4/vbus/vbus_types.h>
#include <l4/sys/types.h>

__BEGIN_DECLS

/**
 * \brief Read from the vPCI configuration space.
 *
 * \param  vbus		capability of the system bus
 * \param  handle       device handle
 * \param  bus          bus id
 * \param  devfn        devfn
 * \param  reg          register
 * \retval value        Value that has been read
 * \param  width        Width to read in bytes
 *
 * \return 0 on succes, else failure
 */
int L4_CV
l4vbus_pci_cfg_read(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                    l4_uint32_t bus, l4_uint32_t devfn,
                    l4_uint32_t reg, l4_uint32_t *value, l4_uint32_t width);

/**
 * \brief Write to the vPCI configuration space.
 *
 * \param  vbus		capability of the system bus
 * \param  handle       device handle
 * \param  bus          bus id
 * \param  devfn        devfn
 * \param  reg          register
 * \param  value        Value to write
 * \param  width        Width to write in bytes
 */
int L4_CV
l4vbus_pci_cfg_write(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                     l4_uint32_t bus, l4_uint32_t devfn,
                     l4_uint32_t reg, l4_uint32_t value, l4_uint32_t width);

/**
 * \brief Enable PCI interrupt.
 *
 * \param  vbus		capability of the system bus
 * \param  handle       device handle
 * \param  bus          bus id
 * \param  devfn        devfn
 * \param  pin          Pin
 * \retval trigger      Trigger
 * \retval polarity     Polarity
 */
int L4_CV
l4vbus_pci_irq_enable(l4_cap_idx_t vbus, l4vbus_device_handle_t handle,
                      l4_uint32_t bus, l4_uint32_t devfn,
                      int pin, unsigned char *trigger,
                      unsigned char *polarity);

__END_DECLS
