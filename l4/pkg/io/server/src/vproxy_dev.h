/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#pragma once

#include "vdevice.h"
#include "hw_device.h"

namespace Vi {

class Proxy_dev : public Device
{
public:
  explicit Proxy_dev(Hw::Device *d);

  char const *hid() const { return _hwd->hid(); }
private:
  Hw::Device *_hwd;
};

}
