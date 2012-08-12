#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define liolib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include <l4/sys/ipc.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/namespace.h>

static int ipc_call(lua_State *L)
{
  l4_cap_idx_t object = luaL_checknumber(L, 1);
  l4_msgtag_t tag;
  l4_timeout_t timeout;
  l4_utcb_t *utcb = l4_utcb();

  tag.raw = luaL_checknumber(L, 2);
  timeout.raw = luaL_checknumber(L, 3);

  //printf("ipc_call(%lx, %p, %lx, %x)\n", object, utcb, tag.raw, timeout.raw);

  unsigned long res;
  res = l4_ipc_error(l4_ipc_call(object, utcb, tag, timeout), utcb);

  lua_pushinteger(L, res);
  return 1;
}

static int ipc_reply_and_wait(lua_State *L)
{
  l4_msgtag_t tag;
  l4_timeout_t timeout;
  l4_umword_t label;
  l4_utcb_t *utcb = l4_utcb();

  tag.raw     = luaL_checknumber(L, 1);
  timeout.raw = luaL_checknumber(L, 2);

  //printf("ipc_reply_and_wait(%p, %lx, &label, %x)\n",
  //       utcb, tag.raw, timeout.raw);

  unsigned long res;
  res = l4_ipc_error(l4_ipc_reply_and_wait(utcb, tag, &label, timeout),
                     utcb);

  lua_pushinteger(L, res);
  lua_pushinteger(L, label);
  return 2;
}

static int ipc_wait(lua_State *L)
{
  l4_timeout_t timeout;
  l4_umword_t label;
  l4_utcb_t *utcb = l4_utcb();

  timeout.raw = luaL_checknumber(L, 1);

  unsigned long res;
  res = l4_ipc_error(l4_ipc_wait(utcb, &label, timeout), utcb);

  lua_pushinteger(L, res);
  lua_pushinteger(L, label);
  return 2;
}

static int timeout_never(lua_State *L)
{
  l4_timeout_t t = L4_IPC_NEVER;
  lua_pushinteger(L, t.raw);
  return 1;
}

static int re_cap_alloc(lua_State *L)
{
  l4_cap_idx_t c = l4re_util_cap_alloc();
  if (l4_is_invalid_cap(c))
    return 0;

  lua_pushinteger(L, c);
  return 1;
}

static int re_cap_free(lua_State *L)
{
  l4_cap_idx_t c = luaL_checknumber(L, 1);
  if (l4_is_invalid_cap(c))
    return 0;

  l4re_util_cap_free(c);
  return 1;
}

static int cap_is_valid(lua_State *L)
{
  l4_cap_idx_t c = luaL_checknumber(L, 1);
  lua_pushinteger(L, !l4_is_invalid_cap(c));
  return 1;
}

static int re_ns_query_srv(lua_State *L)
{
  int r;
  size_t len;
  const char *s;
  l4_cap_idx_t srv, cap;

  srv = luaL_checknumber(L, 1);
  s   = luaL_checklstring(L, 2, &len);
  cap = luaL_checknumber(L, 3);

  r = l4re_ns_query_srv(srv, s, cap);
  lua_pushinteger(L, r);
  return 1;
}

static int re_get_env_cap(lua_State *L)
{
  const char *s;
  size_t len;
  l4_cap_idx_t cap;

  s = luaL_checklstring(L, 1, &len);

  cap = l4re_env_get_cap_l(s, len, l4re_env())->cap;

  lua_pushinteger(L, cap);
  return 1;
}

static int utcb_mr_put(lua_State *L)
{
  int mr_i;
  int i = 2;
  l4_umword_t val;

  mr_i   = luaL_checknumber(L, 1);
  while (lua_type(L, i) != LUA_TNONE)
    {
      val = luaL_checknumber(L, i);
      l4_utcb_mr()->mr[mr_i] = val;
      ++mr_i;
      ++i;
    }

  return 0;
}

static int utcb_mr_get(lua_State *L)
{
  int i = luaL_checknumber(L, 1);
  lua_pushinteger(L, l4_utcb_mr()->mr[i]);
  return 1;
}

static int msgtag(lua_State *L)
{
  long label     = luaL_checknumber(L, 1);;
  unsigned words = luaL_checknumber(L, 2);;
  unsigned items = luaL_checknumber(L, 3);;
  unsigned flags = luaL_checknumber(L, 4);;

  lua_pushinteger(L, l4_msgtag(label, words, items, flags).raw);
  return 1;
}

static const luaL_Reg l4lib[] = {
  { "ipc_call",           ipc_call },
  { "ipc_reply_and_wait", ipc_reply_and_wait },
  { "ipc_wait",           ipc_wait },
  { "timeout_never",      timeout_never },
  { "re_ns_query_srv",    re_ns_query_srv },
  { "re_get_env_cap",     re_get_env_cap },
  { "re_cap_alloc",       re_cap_alloc },
  { "re_cap_free",        re_cap_free },
  { "cap_is_valid",       cap_is_valid },
  { "utcb_mr_put",        utcb_mr_put },
  { "utcb_mr_get",        utcb_mr_get },
  { "msgtag",             msgtag },
  { NULL, NULL },
};

LUALIB_API int luaopen_l4 (lua_State *L)
{
  luaL_register(L, LUA_L4LIBNAME, l4lib);
  return 1;
}
