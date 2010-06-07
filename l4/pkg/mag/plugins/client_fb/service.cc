/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/ipc_server>
#include <l4/re/util/object_registry>
#include <l4/re/env>
#include <l4/re/protocols>
#include <l4/re/console>
#include <l4/re/util/cap_alloc>
#include <l4/re/error_helper>
#include <l4/re/rm>
#include <l4/re/dataspace>
#include <l4/re/mem_alloc>
#include <l4/re/util/meta>

#include <cstdio>
#include <cstring>
#include <algorithm>

#include "client_fb.h"
#include <l4/mag-gfx/canvas>
#include <l4/mag/server/user_state>
#include <l4/mag/server/factory>

#include "service.h"
#include <l4/sys/kdebug.h>

namespace Mag_server {

using L4Re::Util::Auto_cap;

void Service::start(Core_api *core)
{
  _core = core;
  if (!reg()->register_obj(cxx::Ref_ptr<Service>(this), "svc").is_valid())
    printf("Service registration failed.\n");
  else
    printf("Plugin: Frame-buffer service started\n");
}


int
Service::create(char const *_msg, L4::Ipc_iostream &ios)
{
  int w, h;

  if (sscanf(_msg, "%dx%d", &w, &h) != 2)
    return -L4_EINVAL;

  if (w <= 0 || h <= 0 || h >= 10000 || w >= 10000)
    return -L4_ERANGE;

  Area res(w, h);
  Auto_cap<L4Re::Dataspace>::Cap ds(
      L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>());

  Screen_factory *sf = dynamic_cast<Screen_factory*>(ust()->vstack()->canvas()->type()->factory);

  L4Re::chksys(L4Re::Env::env()->mem_alloc()->alloc(sf->get_texture_size(res), ds.get()));

  L4Re::Rm::Auto_region<void *> dsa;
  L4Re::chksys(L4Re::Env::env()->rm()->attach(&dsa, ds->size(), L4Re::Rm::Search_addr, ds.get(), 0, L4_SUPERPAGESHIFT));

  Texture *smpl = sf->create_texture(res, dsa.get());

  cxx::Ref_ptr<Client_fb> x(new Client_fb(_core, Rect(Point(50, 50), Area(res.w(), res.h() + 16)), Point(0, 0), smpl, ds.get()));

  reg()->register_obj(x);
  x->obj_cap()->dec_refcnt(1);

  ust()->vstack()->push_top(x.ptr());

  ds.release();
  dsa.release();
  ios << x->obj_cap();
  return 0;
}

int
Service::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  switch (tag.label())
    {
    case L4::Meta::Protocol:
      return L4Re::Util::handle_meta_request<L4::Factory>(ios);
    case L4::Factory::Protocol:
      if (L4::kobject_typeid<L4Re::Console>()->
	    has_proto(L4::Ipc::read<L4::Factory::Proto>(ios)))
	{
	  L4::Ipc::Varg opt;
	  ios.get(&opt);

	  if (!opt.is_of<char const *>())
	    return -L4_EINVAL;

	  char _msg[50];
	  int _l = sizeof(_msg) - 1;

          _l =  std::min(_l, opt.length());
	  strncpy(_msg, opt.value<char const *>(), _l);
          _msg[_l] = 0;

	  return create(_msg, ios);
	}
      return -L4_ENODEV;
    default:
      return -L4_EBADPROTO;
    }

}

void
Service::destroy()
{
  printf("MAG: destroying the fb_client_service, holla\n");
}

Service::~Service()
{
  enter_kdebug("X");
  printf("MAG: destroy FB svc\n");
}

static Service _fb_service("frame-buffer");

}
