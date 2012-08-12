/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
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


namespace {
  Session::Property_handler const _client_fb_opts[] =
    { { "g",         true,  &Client_fb::set_geometry_prop },
      { "geometry",  true,  &Client_fb::set_geometry_prop },
      { "focus",     false, &Client_fb::set_flags_prop },
      { "shaded",    false, &Client_fb::set_flags_prop },
      { "fixed",     false, &Client_fb::set_flags_prop },
      { "barheight", true,  &Client_fb::set_bar_height_prop },
      { 0, 0, 0 }
    };
};

int
Service::dispatch(l4_umword_t, L4::Ipc::Iostream &ios)
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
	  L4::Ipc::Istream_copy cp_is = ios;
	  cxx::Ref_ptr<Client_fb> x(new Client_fb(_core));
	  _core->set_session_options(x.get(), cp_is, _client_fb_opts);
	  x->setup();

	  _core->register_session(x.get());
	  ust()->vstack()->push_top(x.get());
          x->view_setup();

	  reg()->register_obj(x);
	  x->obj_cap()->dec_refcnt(1);
	  ios << x->obj_cap();
	  return 0;
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
  printf("MAG: destroy FB svc\n");
}

static Service _fb_service("frame-buffer");

}
