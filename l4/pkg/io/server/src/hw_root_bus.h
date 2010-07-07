/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "hw_device.h"
namespace Hw {

class Root_bus : public Device
{
private:
  char const *_name;

public:
  explicit Root_bus(char const *name);
  char const *name() const { return _name; }

};

}
