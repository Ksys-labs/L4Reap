/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "user_state"
#include "lua"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <l4/cxx/exceptions>
#include <l4/re/event_enums.h>
#include <errno.h>


namespace Mag_server {

void
User_state::set_pointer(int x, int y)
{
  Point p(x, y);
  Rect scr(Point(0, 0), vstack()->canvas()->size());
  p = p.min(scr.p2());
  p = p.max(scr.p1());

  if (_mouse_pos != p)
    {
      _mouse_pos = p;
      vstack()->viewport(_mouse_cursor, Rect(_mouse_pos, _mouse_cursor->size()), true);
    }
}

namespace {

namespace Lua {

class Axis_buf
{
private:
  unsigned char s;
  l4_int32_t v[0];

public:
  Axis_buf(unsigned char _s) : s(_s-1) { assert ((_s & s) == 0); }
  l4_int32_t get(unsigned char idx) const { return v[idx & s]; }
  void set(unsigned char idx, l4_int32_t val) { v[idx & s] = val; }

  static char const *const _class;
  static luaL_Reg const _ops[];
};

static int ab_get(lua_State *l)
{
  Axis_buf *s = lua_check_class<Axis_buf>(l, 1);
  int idx = lua_tointeger(l, 2);
  lua_pushinteger(l, s->get(idx));
  return 1;
}

static int ab_set(lua_State *l)
{
  Axis_buf *s = lua_check_class<Axis_buf>(l, 1);
  int idx = lua_tointeger(l, 2);
  s->set(idx, lua_tointeger(l, 3));
  return 0;
}

static int ab_copy(lua_State *l)
{
  Axis_buf *s = lua_check_class<Axis_buf>(l, 1);
  int idx1 = lua_tointeger(l, 2);
  int idx2 = lua_tointeger(l, 3);
  s->set(idx1, s->get(idx2));
  return 0;
}

static int ab_create(lua_State *l)
{
  unsigned long sz = lua_tointeger(l, 1);
  unsigned long z;
  for (z = 0; z < 8 && (1UL << z) < sz; ++z)
    ;
  sz = (1UL << z);
  unsigned long msz = sizeof(Axis_buf) + sizeof(l4_int32_t) * sz;
  new (lua_newuserdata(l, msz)) Axis_buf(sz);

  if (luaL_newmetatable(l, Axis_buf::_class))
    {
      lua_pushcfunction(l, &ab_set);
      lua_setfield(l, -2, "__newindex");
      lua_newtable(l);
      Lua_register_ops<Axis_buf>::init(l);
      lua_setfield(l, -2, "__index");
    }
  lua_setmetatable(l, -2);
  return 1;
}

char const *const Axis_buf::_class = "Mag_server.Lua.Axis_buf";
luaL_Reg const Axis_buf::_ops[] = {
      { "get", &ab_get },
      { "set", &ab_set },
      { "copy", &ab_copy },
      { 0, 0 }
};


}


struct Lua_user_state
{
  User_state *u;
  static char const *const _class;
  static luaL_Reg const _ops[];
  explicit Lua_user_state(User_state *u) : u(u) {}
};

struct Lua_view_proxy : public User_state::View_proxy
{
  explicit Lua_view_proxy(User_state *u) : View_proxy(u) {}
  static char const *const _class;
  static luaL_Reg const _ops[];
};

static int user_state_post_event(lua_State *l)
{
  int top = lua_gettop(l);
  User_state *ust = lua_check_class<Lua_user_state>(l, 1)->u;
  View *v = ust->kbd_focus();
  if (top >= 2 && !lua_isnil(l, 2))
    v = lua_check_class<Lua_view_proxy>(l, 2)->view();

  User_state::Event e;
  e.time = lua_tonumber(l, 4);
  e.payload.stream_id = (l4_umword_t)lua_touserdata(l, 3);
  e.payload.type = lua_tointeger(l, 5);
  e.payload.code = lua_tointeger(l, 6);
  e.payload.value = lua_tointeger(l, 7);

  if (top >= 8 && lua_toboolean(l, 8))
    ust->vstack()->update_all_views();

  if (v && (!ust->vstack()->mode().kill() || v->super_view()))
    v->handle_event(e, ust->mouse_pos());

  return 0;
}

static int user_state_post_pointer_event(lua_State *l)
{
  User_state *ust = lua_check_class<Lua_user_state>(l, 1)->u;
 

  View *v = ust->kbd_focus();
  if (!lua_isnil(l, 2))
    v = lua_check_class<Lua_view_proxy>(l, 2)->view();

  if (!v)
    return 0;

  if (ust->vstack()->mode().kill() && !v->super_view())
    return 0;

  Point m = ust->mouse_pos();
  User_state::Event e;
  e.time = lua_tonumber(l, 4);
  e.payload.stream_id = (l4_umword_t)lua_touserdata(l, 3);
  e.payload.type = L4RE_EV_ABS;
  e.payload.code = L4RE_ABS_X;
  e.payload.value = m.x();
  v->handle_event(e, ust->mouse_pos());
  e.payload.code = L4RE_ABS_Y;
  e.payload.value = m.y();
  v->handle_event(e, ust->mouse_pos());

  return 0;
}

static int user_state_set_pointer(lua_State *l)
{
  User_state *ust = lua_check_class<Lua_user_state>(l, 1)->u;

  int x, y;
  x = lua_tointeger(l, 2);
  y = lua_tointeger(l, 3);
  ust->set_pointer(x, y);
  return 0;
}

static int user_state_move_pointer(lua_State *l)
{
  User_state *ust = lua_check_class<Lua_user_state>(l, 1)->u;

  int x, y;
  x = lua_tointeger(l, 2);
  y = lua_tointeger(l, 3);
  ust->move_pointer(x, y);
  return 0;
}


static int user_state_toggle_mode(lua_State *l)
{
  User_state *ust = lua_check_class<Lua_user_state>(l, 1)->u;

  int x;
  x = lua_tointeger(l, 2);
  ust->vstack()->toggle_mode((Mag_server::Mode::Mode_flag)x);
  return 0;
}

static int user_state_find_pointed_view(lua_State *l)
{
  User_state *ust = lua_check_class<Lua_user_state>(l, 1)->u;
  Lua_view_proxy *vp = lua_check_class<Lua_view_proxy>(l, 2);
  vp->view(ust->vstack()->find(ust->mouse_pos()));
  return 0;
}

static int user_state_create_view_proxy(lua_State *l)
{
  User_state *ust = lua_check_class<Lua_user_state>(l, 1)->u;
  if (lua_alloc_class<Lua_view_proxy>(l, ust))
    return 1;

  return 0;
}


static int lua_view_proxy_set(lua_State *l)
{
  int top = lua_gettop(l);
  Lua_view_proxy *d = lua_check_class<Lua_view_proxy>(l, 1);
  View *sv = 0;
  if (top >= 2 || !lua_isnil(l, 2))
    sv = lua_check_class<Lua_view_proxy>(l, 2)->view();
  d->view(sv);
  return 0;
}

static int user_state_set_kbd_focus(lua_State *l)
{
  int top = lua_gettop(l);
  User_state *ust = lua_check_class<Lua_user_state>(l, 1)->u;
  View *v = 0;
  if (top >= 2 && !lua_isnil(l,2))
    v = lua_check_class<Lua_view_proxy>(l, 2)->view();

  lua_pushboolean(l, ust->set_focus(v));
  return 1;
}


char const *const Lua_user_state::_class = "Mag_server.User_state_class";
char const *const Lua_view_proxy::_class = "Mag_server.View_proxy_class";


luaL_Reg const Lua_user_state::_ops[] =
{ { "set_pointer", &user_state_set_pointer },
  { "move_pointer", &user_state_move_pointer },
  { "set_kbd_focus", &user_state_set_kbd_focus },
  { "post_event", &user_state_post_event },
  { "post_pointer_event", &user_state_post_pointer_event },
  { "toggle_mode", &user_state_toggle_mode },
  { "create_view_proxy", &user_state_create_view_proxy },
  { "find_pointed_view", &user_state_find_pointed_view },
  { 0, 0 }
};

luaL_Reg const Lua_view_proxy::_ops[] =
{ { "set", &lua_view_proxy_set },
  { 0, 0 }
};



}

static void dump_stack(lua_State *l)
{
  int i = lua_gettop(l);
  while (i)
    {
      int t = lua_type(l, i);
      switch (t)
	{
	case LUA_TSTRING:
	  printf("#%02d: '%s'\n", i, lua_tostring(l, i));
	  break;
	case LUA_TBOOLEAN:
	  printf("#%02d: %s\n", i, lua_toboolean(l, i) ? "true" : "false");
	  break;
	case LUA_TNUMBER:
	  printf("#%02d: %g\n", i, lua_tonumber(l, i));
	  break;
	default:
	  printf("#%02d: [%s] %p\n", i, lua_typename(l, t), lua_topointer(l, i));
	  break;
	}

      --i;
    }
}

template<>
struct Lua_register_ops<Lua_user_state>
{
  static void init(lua_State *l)
  {
    luaL_register(l, NULL, Lua_user_state::_ops);
    Lua_user_state *u = (Lua_user_state*)lua_touserdata(l, -3);
    Area sz = u->u->vstack()->canvas()->size();
    lua_pushinteger(l, sz.w());
    lua_setfield(l, -2, "width");
    lua_pushinteger(l, sz.h());
    lua_setfield(l, -2, "height");
  }
};


User_state::User_state(lua_State *lua, View_stack *_vstack, View *cursor)
: _vstack(_vstack), _mouse_pos(0,0), _keyboard_focus(0),
  _mouse_cursor(cursor), _l(lua)
{
  lua_getglobal(_l, "Mag");
  lua_alloc_class<Lua_user_state>(_l, this);
  lua_setfield(_l, -2, "user_state");

  lua_pushcfunction(_l, &Lua::ab_create);
  lua_setfield(_l, -2, "Axis_buf");

  lua_pop(_l, 1);

  if (_mouse_cursor)
    vstack()->push_top(_mouse_cursor, true);
  vstack()->update_all_views();
}

User_state::~User_state()
{
  lua_getglobal(_l, "Mag");
  lua_pushnil(_l);
  lua_setfield(_l, -2, "user_state");
  lua_pop(_l, 1);
}

void
User_state::forget_view(View *v)
{
  vstack()->forget_view(v);
  for (View_proxy *p = _view_proxies; p; p = p->_n)
    p->forget(v);

  if (_keyboard_focus == v)
    _keyboard_focus = 0;

  if (_vstack->focused() == v)
    _vstack->set_focused(0);
}

bool
User_state::set_focus(View *v)
{
  if (_keyboard_focus == v)
    return false;

  if (_keyboard_focus)
    _keyboard_focus->set_focus(false);
  _keyboard_focus = v;
  _vstack->set_focused(v);

  if (v)
    v->set_focus(true);

  return true;
}

void
User_state::handle_event(Event const &e)
{
  lua_getfield(_l, LUA_GLOBALSINDEX, "handle_event");
  lua_pushlightuserdata(_l, const_cast<Event *>(&e));
  if (lua_pcall(_l, 1, 0, 0))
    {
      fprintf(stderr, "ERROR: lua event handling returned: %s.\n", lua_tostring(_l, -1));
      lua_pop(_l, 1);
    }
}

}
