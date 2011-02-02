/*
 * Example of a C program that interfaces with Lua.
 * Based on Lua 5.0 code by Pedro Martelletto in November, 2003.
 * Updated to Lua 5.1. David Manura, January 2007.
 *
 * L4: Made it just call the interpreter.
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
  int status, result;
  lua_State *L;

  if (argc < 2)
    {
      fprintf(stderr, "Need arg\n");
      return 1;
    }

  /*
   * All Lua contexts are held in this structure. We work with it almost
   * all the time.
   */
  L = luaL_newstate();

  luaL_openlibs(L); /* Load Lua libraries */

  /* Load the file containing the script we are going to run */
  status = luaL_loadfile(L, argv[1]);
  if (status)
    {
      (void)fprintf(stderr, "bad, bad file\n");
      exit(1);
    }

  /* Ask Lua to run our little script */
  result = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (result)
    {
      fprintf(stderr, "%s\n", lua_pushfstring(L, "error calling "
            "'print' (%s)", lua_tostring(L, -1)));
      exit(1);
    }

  lua_pop(L, 1);  /* Take  the returned value out of the stack */
  lua_close(L);   /* Cya, Lua */

  return 0;
}
