/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
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

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <cassert>
#include <cstdio>
#include <cstring>

#include <unistd.h>

#include "background.h"
#include "big_mouse.h"
#include "input_driver"
#include "object_gc.h"

#include "core_api"

#include <dlfcn.h>

using namespace Mag_server;

static Core_api_impl *_core_api;
extern char const _binary_mag_lua_start[];
extern char const _binary_mag_lua_end[];
extern char const _binary_default_tff_start[];

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

    int dispatch(l4_umword_t, L4::Ipc::Iostream &)
    {
      gc->gc_step();
      return -L4_ENOREPLY;
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
poll_input(Core_api_impl *core)
{
  for (Input_source *i = core->input_sources(); i; i = i->next())
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

  void setup_wait(L4::Ipc::Istream &istr, L4::Ipc_svr::Reply_mode reply_mode)
  {
    if (to < l4re_kip()->clock
	&& reply_mode == L4::Ipc_svr::Reply_separate)
    {
      poll_input(_core_api);
      _core_api->user_state()->vstack()->flush();
      _core_api->tick();
      to += 40000;
      while (to - 10000 < l4re_kip()->clock)
	to += 20000;
    }

    istr.reset();
    istr << L4::Ipc::Small_buf(rcv_cap.cap(), L4_RCV_ITEM_LOCAL_ID);
    l4_utcb_br_u(istr.utcb())->bdr = 0;
    l4_timeout_abs(to, 8);
  }

  L4::Ipc_svr::Reply_mode before_reply(long, L4::Ipc::Iostream &)
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

int load_lua_plugin(Core_api *core_api, char const *name)
{
  char const *n = name;

  if (access(n, F_OK) != 0)
    return 1;

  printf("loading '%s'\n", n);

  lua_State *_l = core_api->lua_state();
  int err = luaL_dofile(_l, n);
  if (err)
    {
      printf("ERROR: loading '%s': %s\n", n, lua_tostring(_l, -1));
      lua_pop(_l, lua_gettop(_l));
      return -1;
    }

  lua_pop(_l, lua_gettop(_l));
  return 0;
}

int load_so_plugin(Core_api *core_api, char const *name)
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

static const luaL_Reg libs[] =
{
  { "", luaopen_base },
// { LUA_IOLIBNAME, luaopen_io },
  { LUA_STRLIBNAME, luaopen_string },
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_DBLIBNAME, luaopen_debug},
  { NULL, NULL }
};

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

  L4Re::Video::View *screen_view = 0;

    {
      L4Re::Video::Goos::Info i;
      goos_fb.goos()->info(&i);
      if (!i.auto_refresh())
	screen_view = goos_fb.view();
    }

  lua_State *lua = luaL_newstate();

  if (!lua)
    {
      printf("ERROR: cannot allocate Lua state\n");
      exit(1);
    }

  for (int i = 0; libs[i].func; ++i)
    {
      lua_pushcfunction(lua, libs[i].func);
      lua_pushstring(lua,libs[i].name);
      lua_call(lua, 1, 0);
    }

  int err;
  if ((err = luaL_loadbuffer(lua, _binary_mag_lua_start, _binary_mag_lua_end - _binary_mag_lua_start, "@mag.lua")))
    {
      fprintf(stderr, "lua error: %s.\n", lua_tostring(lua, -1));
      lua_pop(lua, lua_gettop(lua));
      if (err == LUA_ERRSYNTAX)
	throw L4::Runtime_error(L4_EINVAL, lua_tostring(lua, -1));
      else
	throw L4::Out_of_memory(lua_tostring(lua, -1));
    }

  if ((err = lua_pcall(lua, 0, 1, 0)))
    {
      fprintf(stderr, "lua error: %s.\n", lua_tostring(lua, -1));
      lua_pop(lua, lua_gettop(lua));
      if (err == LUA_ERRSYNTAX)
	throw L4::Runtime_error(L4_EINVAL, lua_tostring(lua, -1));
      else
	throw L4::Out_of_memory(lua_tostring(lua, -1));
    }

  lua_pop(lua, lua_gettop(lua));
  static Font label_font(&_binary_default_tff_start[0]);
  static View_stack vstack(screen, screen_view, &bg, &label_font);
  static User_state user_state(lua, &vstack, cursor);
  static Core_api_impl core_api(&registry, lua, &user_state, rcv_cap, fb, &label_font);

  Plugin_manager::start_plugins(&core_api);

  for (int i = 1; i < argc; ++i)
    {
      if (load_lua_plugin(&core_api, argv[i]) == 1)
        load_so_plugin(&core_api, argv[i]);
    }

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
  catch (L4::Runtime_error const &e)
    {
      L4::cerr << "Error: " << e << '\n';
    }
  catch (L4::Base_exception const &e)
    {
      L4::cerr << "Error: " << e << '\n';
    }
  catch (std::exception const &e)
    {
      L4::cerr << "Error: " << e.what() << '\n';
    }

  return -1;
}
