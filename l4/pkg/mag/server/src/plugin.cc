/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "plugin"
#include "lua"

#include <lua.h>
#include <lauxlib.h>


namespace Mag_server {

Plugin *Plugin::_first;

Core_api::Core_api(Registry *r, lua_State *lua, User_state *u,
                   L4::Cap<void> rcvc, L4::Cap<L4Re::Video::Goos> fb)
: _reg(r), _ust(u), _rcv_cap(rcvc), _fb(fb), _lua(lua)
{
  lua_pushlightuserdata(_lua, this);
  lua_newtable(_lua);
  lua_rawset(_lua, LUA_REGISTRYINDEX);
}

void
Core_api::get_refs_table() const
{
  lua_pushlightuserdata(_lua, const_cast<Core_api*>(this));
  lua_rawget(_lua, LUA_REGISTRYINDEX);
}


void
Core_api::add_input_source(Input_source *i)
{
  i->_next_active = _input;
  _input = i;
  get_refs_table();
  i->add_lua_input_source(_lua, -1);
  lua_pop(_lua, 1);
}


}

