/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "input_source"
#include "lua"
#include "plugin"

#include <lua.h>
#include <lauxlib.h>

namespace Mag_server {

namespace {

  struct Lua_event_stream_info : public L4Re::Event_stream_info
  {
    static char const *const _class;
    static luaL_Reg _ops[];

    static int get_bit(lua_State *l, unsigned long *bits, unsigned max, unsigned bit);
    template< typename S >
    static int get_x_bit(lua_State *l);

    template< typename F >
    static inline int _invoke(lua_State *l);

    int get_device_id(lua_State *l)
    {
      lua_pushinteger(l, id.bustype);
      lua_pushinteger(l, id.vendor);
      lua_pushinteger(l, id.product);
      lua_pushinteger(l, id.version);
      return 4;
    }
  };

  int
  Lua_event_stream_info::get_bit(lua_State *l, unsigned long *bits, unsigned max, unsigned bit)
  {
#define __M ((max + sizeof(unsigned long)*8 -1) / (sizeof(unsigned long)*8))
#if 0
    printf("max=%d bit=%d longs=%d...\n", max, bit, __M);
    for (unsigned x = 0; x < __M; ++x)
      {
	printf("(%02d) %08lx ", x, bits[x]);
      }
    printf("\n");
#endif
    if (bit >= max)
      lua_pushboolean(l, false);
    else
      lua_pushboolean(l, bits[bit / (sizeof(unsigned long)*8)] & (1UL << (bit % (sizeof(unsigned long)*8))));
#undef __M
    return 1;
  }

  template< typename F > inline
  int
  Lua_event_stream_info::_invoke(lua_State *l)
  {
    Lua_event_stream_info *s = lua_check_class<Lua_event_stream_info>(l, 1);
    return F::f(s, l);
  }

  template< typename S >
  int
  Lua_event_stream_info::get_x_bit(lua_State *l)
  {
    Lua_event_stream_info *s = lua_check_class<Lua_event_stream_info>(l, 1);
    int bit = lua_tointeger(l, 2);
    return get_bit(l, S::bits(s), S::Max, bit);
  }

  struct S_EV
  {
    static unsigned long *bits(Lua_event_stream_info *s) { return s->evbits; }
    enum { Max = L4RE_EVENT_EV_MAX };
  };

  struct S_KEY
  {
    static unsigned long *bits(Lua_event_stream_info *s) { return s->keybits; }
    enum { Max = L4RE_EVENT_KEY_MAX };
  };

  struct S_REL
  {
    static unsigned long *bits(Lua_event_stream_info *s) { return s->relbits; }
    enum { Max = L4RE_EVENT_REL_MAX };
  };

  struct S_ABS
  {
    static unsigned long *bits(Lua_event_stream_info *s) { return s->absbits; }
    enum { Max = L4RE_EVENT_ABS_MAX };
  };

  struct X_DEV_ID
  {
    template<typename S> static inline int f(S s, lua_State *l)
    { return s->get_device_id(l); }
  };

  struct X_ABS_INFO
  {
    template<typename S> static inline int f(S s, lua_State *l)
    { return s->get_abs_info(l); }
  };

  char const *const Lua_event_stream_info::_class = "Mag_server.Event_stream_info_class";
  luaL_Reg Lua_event_stream_info::_ops[] = {
      { "get_evbit", &Lua_event_stream_info::get_x_bit<S_EV> },
      { "get_keybit", &Lua_event_stream_info::get_x_bit<S_KEY> },
      { "get_relbit", &Lua_event_stream_info::get_x_bit<S_REL> },
      { "get_absbit", &Lua_event_stream_info::get_x_bit<S_ABS> },
      { "get_device_id", &Lua_event_stream_info::_invoke<X_DEV_ID> },
      { 0, 0 }
  };

  struct Lua_input_source
  {
    Input_source *s;
    static char const *const _class;
    static luaL_Reg _ops[];

    static int get_stream_info(lua_State *l)
    {
      Lua_input_source *i = lua_check_class<Lua_input_source>(l, 1);
      void *id = lua_touserdata(l, 2);
      L4Re::Event_stream_info *info = lua_alloc_class<Lua_event_stream_info>(l);
      int res = i->s->get_stream_info((l4_umword_t)id, info);
      lua_pushinteger(l, res);
      lua_insert(l, -2);
      return 2;
    }

    static int get_abs_info(lua_State *l)
    {
      int args = lua_gettop(l);
      if (args < 3)
	luaL_error(l, "get_abs_info needs at least 3 arguments\n");

      Lua_input_source *i = lua_check_class<Lua_input_source>(l, 1);
      void *id = lua_touserdata(l, 2);

      unsigned naxes = args - 2;
      unsigned axes[naxes];
      L4Re::Event_absinfo info[naxes];

      for (unsigned a = 0; a < naxes; ++a)
	axes[a] = lua_tointeger(l, 3 + a);

      int res = i->s->get_axis_info((l4_umword_t)id, naxes, axes, info);
      lua_pushinteger(l, res);

      if (res < 0)
	  return 1;

      for (unsigned a = 0; a < naxes; ++a)
	{
	  lua_createtable(l, 0, 6);
	  lua_pushstring(l, "value");
	  lua_pushinteger(l, info[a].value);
	  lua_rawset(l, -3);

	  lua_pushstring(l, "min");
	  lua_pushinteger(l, info[a].min);
	  lua_rawset(l, -3);

	  lua_pushstring(l, "max");
	  lua_pushinteger(l, info[a].max);
	  lua_rawset(l, -3);

	  lua_pushstring(l, "fuzz");
	  lua_pushinteger(l, info[a].fuzz);
	  lua_rawset(l, -3);

	  lua_pushstring(l, "flat");
	  lua_pushinteger(l, info[a].flat);
	  lua_rawset(l, -3);

	  lua_pushstring(l, "resolution");
	  lua_pushinteger(l, info[a].resolution);
	  lua_rawset(l, -3);
	}
      return naxes + 1;
    }
  };

  char const *const Lua_input_source::_class = "Mag_server.Input_source_class";
  luaL_Reg Lua_input_source::_ops[] = {
      { "get_stream_info", &Lua_input_source::get_stream_info },
      { "get_abs_info", &Lua_input_source::get_abs_info },
      { 0, 0 }
  };
}

void
Input_source::post_event(L4Re::Event_buffer::Event const *e)
{
  lua_State *l = _core->lua_state();
  lua_getfield(l, LUA_GLOBALSINDEX, "handle_event");
  lua_createtable(l, 6, 0);
  lua_pushinteger(l, e->payload.type);
  lua_rawseti(l, -2, 1);
  lua_pushinteger(l, e->payload.code);
  lua_rawseti(l, -2, 2);
  lua_pushinteger(l, e->payload.value);
  lua_rawseti(l, -2, 3);
  lua_pushnumber(l, e->time);
  lua_rawseti(l, -2, 4);
  lua_pushlightuserdata(l, (void*)(e->payload.stream_id));
  lua_rawseti(l, -2, 5);
  _core->get_refs_table();
  lua_rawgeti(l, -1, _ref);
  lua_remove(l, -2);
  lua_rawseti(l, -2, 6);

  if (lua_pcall(l, 1, 0, 0))
    {
      fprintf(stderr, "ERROR: lua event handling returned: %s.\n", lua_tostring(l, -1));
      lua_pop(l, 1);
    }
}


void
Input_source::add_lua_input_source(lua_State *l, int ref_table)
{
  Lua_input_source *d = lua_alloc_class<Lua_input_source>(l);
  d->s = this;
  _ref = luaL_ref(l, ref_table < 0 ? ref_table -1 : ref_table);
}

}
