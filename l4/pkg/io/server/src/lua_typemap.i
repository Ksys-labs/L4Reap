// vi:ft=cpp
/*
 * (c) 2011 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

%{
#define SWIG_RECURSIVE_DEFINED defined
%}

/* Refined typemaps for our L4 types and our lua lib with integer support */

%typemap(typecheck,precedence=SWIG_TYPECHECK_INTEGER) l4_int8_t,l4int16_t,l4_int32_t,
  l4_mword_t,l4uint8_t,l4_uint16_t,l4_uint32_t,l4_umword_t,
  l4_size_t,l4_addr_t
{$1 = lua_isnumber(L, $input);}

%typemap(in,checkfn="lua_isnumber") l4_int32_t, int, short, long, signed char
%{$1 = ($type)lua_tointeger(L, $input);%}

%typemap(in,checkfn="lua_isnumber") l4_addr_t, l4_umword_t, unsigned int, unsigned short,
        unsigned long, unsigned char
%{//SWIG_contract_assert((lua_tonumber(L,$input)>=0),"number must not be negative")
$1 = ($type)lua_tointeger(L, $input);%}

%typemap(in,checkfn="lua_isnumber") enum SWIGTYPE
%{$1 = ($type)(int)lua_tointeger(L, $input);%}

%typemap(out) enum SWIGTYPE
%{  lua_pushinteger(L, (int)($1)); SWIG_arg++;%}

%typemap(out) l4_addr_t,l4_umword_t,l4_int32_t,int,short,long,
             unsigned int,unsigned short,unsigned long,
             signed char,unsigned char
%{  lua_pushinteger(L, $1); SWIG_arg++;%}

%typemap(out) l4_int32_t*,int*,short*,long*,
             unsigned int*,unsigned short*,unsigned long*,
             signed char*,unsigned char*
%{  lua_pushinteger(L, *$1); SWIG_arg++;%}

%typemap(typecheck,precedence=SWIG_TYPECHECK_INTEGER) l4_addr_t,l4_umword_t,l4_int32_t {
   $1 = lua_isnumber(L, $input);
}

%typemap(typecheck,precedence=SWIG_TYPECHECK_STRING) cxx::String, cxx::String const & {
   $1 = lua_isstring(L, $input);
}

%typemap(in,checkfn="lua_isstring") cxx::String
%{$1 = cxx::String(lua_tostring(L, $input), lua_rawlen(L, $input));%}

%typemap(in,checkfn="lua_isstring") cxx::String const & (cxx::String tmp)
%{tmp = cxx::String(lua_tostring(L, $input), lua_rawlen(L, $input)); $1 = &tmp;%}


// typemap for big integers
%define %l4re_lua_bigint(name, type32, type, fmt, conversion)
  %typemap(typecheck,precedence=SWIG_TYPECHECK_INTEGER) name
  { $1 = lua_isnumber(L, $input) || lua_isstring(L, $input); }

  %typemap(in) name
  %{

  #if SWIG_RECURSIVE_DEFINED LNUM_INT32
    if (lua_isinteger(L, $input))
      $1 = (type32)lua_tointeger(L, $input);
    else
  #elif SWIG_RECURSIVE_DEFINED LNUM_INT64
    if (lua_isinteger(L, $input))
      $1 = lua_tointeger(L, $input);
    else
  #endif
    if (lua_isstring(L, $input))
      {
        char *e = 0;
        char const *s = lua_tostring(L, $input);
        type a = conversion(s, &e, 0);
        if (s == e || *e != 0)
          {
            lua_pushfstring(L, "big number conversion error '%s'", s);
            SWIG_fail;
          }
        $1 = a;
      }
    else if (lua_isnumber(L, $input))
      $1 = lua_tonumber(L, $input);
    else
      {
        lua_pushfstring(L, "big number expected (string or number) got %s",
                        lua_typename(L, lua_type(L, $input)));
        SWIG_fail;
      }
  %}

  %typemap(out) name
  %{lua_pushfstring(L, fmt, $1); ++SWIG_arg;%}
%enddef
%l4re_lua_bigint(SWIG_BIGUINT, l4_uint32_t, l4_uint64_t, "0x%llx", strtoull)
%l4re_lua_bigint(SWIG_BIGINT, l4_int32_t, l4_int64_t, "0x%llx", strtoll)

%apply SWIG_BIGINT { l4_int64_t }
%apply SWIG_BIGUINT { l4_uint64_t }

