/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/sys/err.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <getopt.h>

#ifdef USE_READLINE
#include <readline/history.h>
#include <readline/readline.h>
#endif

#include "lua_cap.h"
#include "lua.h"

static Lua::Lib *_lua_init;

extern char const _binary_ned_lua_start[];
extern char const _binary_ned_lua_end[];

Lua::Lib::Lib(Prio prio) : _prio(prio), _next(0)
{
  Lib **f = &_lua_init;
  while (*f && (*f)->prio() < prio)
    f = &(*f)->_next;

  _next = *f;
  *f = this;
}

void
Lua::Lib::run_init(lua_State *l)
{
  for (Lib *c = _lua_init; c; c = c->_next)
    c->init(l);
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

static char const *const options = "+i";
static struct option const loptions[] =
{{"interactive", 0, NULL, 'i' },
 {0, 0, 0, 0}};

int lua(int argc, char const *const *argv)
{

  printf("Ned says: Hi World!\n");

  bool interactive = false;

  if (argc < 2)
    interactive = true;

  int opt;
  while ((opt = getopt_long(argc, const_cast<char *const*>(argv), options, loptions, NULL)) != -1)
    {
      switch (opt)
	{
	case 'i': interactive = true; break;
	default: break;
	}
    }


  if (optind >= argc)
    interactive = true;

  lua_State *L;

  L = luaL_newstate();

  if (!L)
    return 1;

  for (int i = 0; libs[i].func; ++i)
    {
      lua_pushcfunction(L, libs[i].func);
      lua_pushstring(L,libs[i].name);
      lua_call(L, 1, 0);
    }
  Lua::init_lua_cap(L);
  Lua::Lib::run_init(L);

  if (luaL_loadbuffer(L, _binary_ned_lua_start, _binary_ned_lua_end - _binary_ned_lua_start, "@ned.lua"))
    {
      printf("Ned: script: ---\n%.*s\n---", (int)(_binary_ned_lua_end - _binary_ned_lua_start), _binary_ned_lua_start);
      fprintf(stderr, "lua error: %s.\n", lua_tostring(L, -1));
      lua_pop(L, lua_gettop(L));
      return 0;
    }

  if (lua_pcall(L, 0, 1, 0))
    {
      fprintf(stderr, "lua error: %s.\n", lua_tostring(L, -1));
      lua_pop(L, lua_gettop(L));
      return 0;
    }

  lua_pop(L, lua_gettop(L));

  for (int c = optind; c < argc; ++c)
    {
      printf("Ned: loading file: '%s'\n", argv[c]);
      int e = luaL_dofile(L, argv[c]);
      if (e)
	{
	  char const *error = lua_tostring(L, -1);
	  printf("Ned: ERROR: %s\n", error);
	}
      lua_pop(L, lua_gettop(L));
    }

  lua_gc(L, LUA_GCCOLLECT, 0);

  if (!interactive)
    return 0;

#ifndef USE_READLINE
  fprintf(stderr, "Ned: Interactive mode not compiled in.\n");
#else
  printf("Ned: Interactive mode.\n");
  const char *cmd;
  do {
    cmd = readline((char*)"Ned: ");

    //printf("INPUT: %s\n", cmd);

    if (!cmd)
      break;

    if (luaL_loadbuffer(L, cmd, strlen(cmd), "argument"))
      {
        fprintf(stderr, "lua couldn't parse '%s': %s.\n",
                cmd, lua_tostring(L, -1));
        lua_pop(L, 1);
      }
    else
      {
        if (lua_pcall(L, 0, 1, 0))
          {
            fprintf(stderr, "lua couldn't execute '%s': %s.\n",
                    cmd, lua_tostring(L, -1));
            lua_pop(L, 1);
          }
        else
          lua_pop(L, lua_gettop(L));
      }

  //  cmd = 0;
  } while (cmd);
#endif

  return 0;
}
