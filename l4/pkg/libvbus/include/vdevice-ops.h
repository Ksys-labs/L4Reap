/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

enum
{
  L4vbus_vdevice_generic = 0x80000000,
  L4vbus_vdevice_hid     = 0x80000000,
  L4vbus_vdevice_adr     = 0x80000001,
  L4vbus_vdevice_get_by_hid,
  L4vbus_vdevice_get_next,
  L4vbus_vdevice_get_resource,
};

enum {
  L4vbus_vbus_request_resource,
  L4vbus_vbus_release_resource,
};

enum
{
  L4vbus_vicu_get_cap
};

enum
{
  L4vbus_gpio_read,
  L4vbus_gpio_write,
};

enum
{
  L4vbus_i2c_write,
  L4vbus_i2c_read,
};

enum
{
  L4vbus_mcspi_read,
  L4vbus_mcspi_write,
};

