/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/mag-gfx/geometry>
#include <l4/mag-gfx/canvas>
#include "factory"

#include <l4/util/util.h>

#include <l4/cxx/iostream>
#include <l4/cxx/exceptions>
#include <l4/re/util/cap_alloc>
#include <l4/re/error_helper>
#include <l4/re/env>
#include <l4/re/rm>
#include <l4/re/video/goos>
#include <l4/re/util/video/goos_fb>

#include <cassert>
#include <cstdio>
#include <cstring>

#include "background.h"
#include "big_mouse.h"
#include "input_driver"
#include "object_gc.h"

#include "plugin"

#include <dlfcn.h>

using namespace Mag_server;

static Core_api *_core_api;

namespace Mag_server {

class Plugin_manager
{
public:
  static void start_plugins(Core_api *core)
  {
    for (Plugin *p = Plugin::_first; p; p = p->_next)
      if (!p->started())
	p->start(core);
  }
};

}

namespace {


class My_reg : public Registry, private Object_gc
{
private:
  class Del_handler : public L4::Server_object
  {
  private:
    Object_gc *gc;

  public:
    explicit Del_handler(Object_gc *gc) : gc(gc) {}

    int dispatch(l4_umword_t, L4::Ipc_iostream &s)
    {
      l4_msgtag_t t;
      s >> t;
      if (t.label() != L4_PROTO_IRQ)
	return -L4_EBADPROTO;

      gc->gc_step();
      return 0;
    }
  };

  L4::Cap<L4::Irq> _del_irq;

public:
  My_reg() : Registry()
  {
    _del_irq = register_irq_obj(new Del_handler(this));
    assert (_del_irq);
    _server->register_del_irq(_del_irq);
  };

  void add_gc_obj(Object *o)
  {
    L4::Cap<L4::Kobject> c(o->obj_cap());
    Object_gc::add_obj(o);
  }
};


static void
poll_input(Core_api *core)
{
  for (Input_driver *i = core->input_drivers(); i; i = i->next_active())
      i->poll_events();
}

static L4::Cap<void> rcv_cap;

class Loop_hooks : public L4::Ipc_svr::Ignore_errors
{
public:
  l4_kernel_clock_t to;
  Loop_hooks()
  {
    to = l4re_kip()->clock + 40000;
  }

  l4_timeout_t timeout()
  { return l4_timeout(L4_IPC_TIMEOUT_0, l4_timeout_abs(to, 8)); }

  void setup_wait(L4::Ipc_istream &istr, L4::Ipc_svr::Reply_mode reply_mode)
  {
    if (to < l4re_kip()->clock
	&& reply_mode == L4::Ipc_svr::Reply_separate)
    {
      poll_input(_core_api);
      _core_api->user_state()->vstack()->flush();
      to += 40000;
      while (to - 10000 < l4re_kip()->clock)
	to += 20000;
    }

    istr.reset();
    istr << L4::Small_buf(rcv_cap.cap(), L4_RCV_ITEM_LOCAL_ID);
    l4_utcb_br_u(istr.utcb())->bdr = 0;
    l4_timeout_abs(to, 8);
  }

  L4::Ipc_svr::Reply_mode before_reply(long, L4::Ipc_iostream &)
  {
    if (to < l4re_kip()->clock)
      return L4::Ipc_svr::Reply_separate;
    return L4::Ipc_svr::Reply_compound;
  }
};

static My_reg registry;
static L4::Server<Loop_hooks> server(l4_utcb());

using L4Re::Util::Auto_cap;
using L4Re::chksys;
using L4Re::chkcap;
#if 0
static void test_texture(Texture *t)
{
  char *tb = (char *)t->pixels();
  for (int y = 0; y < t->size().h(); ++y)
    for (int x = 0; x < t->size().w(); ++x)
      {
	t->type()->set(tb, Pixel_info::Col(x*400, y*300, x*y, 0));
	tb += t->type()->bytes;
      }
}
#endif

int load_plugin(Core_api *core_api, char const *name)
{
  static char const *const pfx = "libmag-";
  static char const *const sfx = ".so";
  char *n = new char [strlen(name) + strlen(pfx) + strlen(sfx) + 1];
  strcpy(n, pfx);
  strcpy(n + strlen(pfx), name);
  strcpy(n + strlen(pfx) + strlen(name), sfx);

  printf("loading '%s'\n", n);

  void *pl = dlopen(n, RTLD_LAZY);
  if (!pl)
    {
      delete [] n;
      printf("ERROR: loading '%s': %s\n", n, dlerror());
      return -1;
    }
  else
    {
      void (*ini)(Core_api*) = (void (*)(Core_api*))dlsym(pl, "init_plugin");
      ini(core_api);
    }
  delete [] n;
  return 0;
}

int run(int argc, char const *argv[])
{
  printf("Hello from MAG\n");
  L4Re::Env const *env = L4Re::Env::env();

  L4::Cap<L4Re::Video::Goos> fb
    = chkcap(env->get_cap<L4Re::Video::Goos>("fb"), "requesting frame-buffer", 0);

  L4Re::Util::Video::Goos_fb goos_fb(fb);
  L4Re::Video::View::Info view_i;
  chksys(goos_fb.view_info(&view_i), "requesting frame-buffer info");

  L4Re::Rm::Auto_region<char *> fb_addr;
  chksys(env->rm()->attach(&fb_addr, goos_fb.buffer()->size(),
	L4Re::Rm::Search_addr, goos_fb.buffer(), 0, L4_SUPERPAGESHIFT));

  printf("mapped frame buffer at %p\n", fb_addr.get());

  Screen_factory *f = dynamic_cast<Screen_factory*>(Screen_factory::set.find(view_i.pixel_info));
  if (!f)
    {
      printf("ERROR: could not start screen driver for given video mode.\n"
             "       Maybe unsupoported pixel format... exiting\n");
      exit(1);
    }

  Canvas *screen = f->create_canvas(fb_addr.get() + view_i.buffer_offset,
      Area(view_i.width, view_i.height), view_i.bytes_per_line);

  view_i.dump(L4::cout)
    << "  memory " << (void*)fb_addr.get()
    << '-' << (void*)(fb_addr.get() + goos_fb.buffer()->size()) << '\n';

  if (!screen)
    {
      printf("ERROR: could not start screen driver for given video mode.\n"
             "       Maybe unsupoported pixel format... exiting\n");
      exit(1);
    }

  rcv_cap = L4Re::Util::cap_alloc.alloc<void>();
  if (!rcv_cap.is_valid())
    {
      printf("ERROR: Out of caps\n");
      exit(1);
    }

  View *cursor = f->create_cursor(big_mouse);
  Background bg(screen->size());

  static User_state user_state(screen, cursor, &bg);
  static Core_api core_api(&registry, &user_state, rcv_cap);

  Plugin_manager::start_plugins(&core_api);

  for (int i = 1; i < argc; ++i)
    load_plugin(&core_api, argv[i]);

  _core_api = &core_api;

  server.loop(registry);

  return 0;
}
}

int main(int argc, char const *argv[])
{
  try
    {
      return run(argc, argv);
    }
  catch (L4::Base_exception const &e)
    {
      L4::cerr << "Error: " << e << '\n';
      return -1;
    }
  catch (std::exception const &e)
    {
      L4::cerr << "Error: " << e.what() << '\n';
    }

  return 0;
}
