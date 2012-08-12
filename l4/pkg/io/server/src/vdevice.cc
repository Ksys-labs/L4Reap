/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "vdevice.h"

#include <cstring>
#include <cstdio>

#include "device.h"
#include <l4/vbus/vdevice-ops.h>
#include <l4/vbus/vbus_types.h>

namespace Vi {

Device::Dev_set Device::__devs;

bool
Device::resource_allocated(Resource const *r) const
{
  if (!parent())
    return false;

  return parent()->resource_allocated(r);
}


Device *
Device::get_dev_by_id(l4vbus_device_handle_t id)
{
  if (id == 0)
    return this;
  else if (__devs.find(id) != __devs.end())
    return (Device*)id;
  else
    return 0;
}

struct Match_hid
{
  char const *hid;
  Device mutable *dev;

  int operator () (Device *d) const
  {
    char const *h = d->hid();
    if (h && strcmp(h, hid) == 0)
      {
        dev = d;
	return 1;
      }
    return 0;
  }
};

int
Device::get_by_hid(L4::Ipc::Iostream &ios)
{
  l4vbus_device_handle_t child;
  unsigned long sz;
  char const *hid = 0;

  int depth;

  ios >> child >> depth >> L4::Ipc::Buf_in<char const>(hid, sz);

  //printf("look for '%s' in %p(%x) from %x\n", hid, this, _id, start);
  if (!hid || !sz)
    return -L4_ENOENT;

  iterator c;
  if (!child)
    c = begin(depth);
  else if ((c = iterator(this, get_dev_by_id(child), depth)) != end())
    ++c;

  if (c == end())
    return -L4_ENODEV;


  Match_hid mh;
  mh.hid = hid;
  mh.dev = 0;

  for (; c != end(); ++c)
    if (mh(*c) == 1)
      return dynamic_cast<Device*>(*c)->vbus_get_device(ios);

  return -L4_ENOENT;
}


int
Device::vbus_get_device(L4::Ipc::Iostream &ios)
{
  l4vbus_device_t inf;
  inf.num_resources = resources()->size();
  if (hid())
    {
      strncpy(inf.name, name(), sizeof(inf.name));
      inf.name[sizeof(inf.name) - 1] = 0;
    }
  else
    *inf.name = 0;
  inf.type = ~0;
  inf.flags = 0;
  if (children())
    inf.flags |= L4VBUS_DEVICE_F_CHILDREN;

  ios << l4vbus_device_handle_t(this);
  ios.put(inf);
  return L4_EOK;
}

int
Device::vdevice_dispatch(l4_umword_t obj, l4_uint32_t func, L4::Ipc::Iostream &ios)
{
  if (func & L4vbus_vdevice_generic)
    {
      switch (func)
	{
	case L4vbus_vdevice_hid:
	    {
	      char const *h = hid();
	      if (!h)
		return -L4_ENOSYS;

	      ios << h;
	      return L4_EOK;
	    }
	case L4vbus_vdevice_adr:
	    {
	      l4_uint32_t a = adr();
	      if (a == l4_uint32_t(~0))
		return -L4_ENOSYS;

	      ios << a;
	      return L4_EOK;
	    }
	case L4vbus_vdevice_get_by_hid:
	  return get_by_hid(ios);

	case L4vbus_vdevice_get_next:
	    {
	      l4vbus_device_handle_t child;
	      int depth;
	      ios >> child >> depth;

	      iterator c;
	      if (!child)
		c = begin(depth);
	      else
		c = ++iterator(this, get_dev_by_id(child), depth);

	      if (c == end())
		return -L4_ENODEV;

	      return dynamic_cast<Device*>(*c)->vbus_get_device(ios);
	    }

	case L4vbus_vdevice_get_resource:
	    {
	      int res_idx;
	      ios >> res_idx;
	      Adr_resource *r = dynamic_cast<Adr_resource*>(resources()->elem(res_idx));

	      if (!r)
		return -L4_ENOENT;

	      l4vbus_resource_t res;
	      Adr_resource::Data d = r->data();
	      res.start = d.start();
	      res.end = d.end();
	      res.type = r->type();
	      res.flags = 0;
	      ios.put(res);
	      return L4_EOK;
	    }

	default: return -L4_ENOSYS;
	}
    }

  for (Feature_list::const_iterator i = _features.begin();
       i != _features.end(); ++i)
    {
      int e = (*i)->dispatch(obj, func, ios);
      if (e != -L4_ENOSYS)
	return e;
    }

  return -L4_ENOSYS;
}

}
