/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "vproxy_dev.h"
#include "vbus_factory.h"

namespace Vi {

Proxy_dev::Proxy_dev(Hw::Device *d)
: _hwd(d)
{
  // suck features from real dev
  for (Hw::Device::Feature_list::const_iterator i = d->features()->begin();
       i != d->features()->end(); ++i)
    {
      Dev_feature *vf = Feature_factory::create(*i);
      if (vf)
	add_feature(vf);
    }

  // suck resources from our real dev
  for (Resource_list::iterator i = d->resources()->begin();
       i != d->resources()->end(); ++i)
    {
      if (i->disabled())
	continue;

      add_resource(*i);
    }
}

namespace {
  static Dev_factory_t<Proxy_dev, Hw::Device> __ghwdf;
}

}