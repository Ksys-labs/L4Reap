// vi:ft=cpp
/*
 * (c) 2011 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

/* Refined typemaps for our L4 types and our lua lib with integer support */

%typemap(in,checkfn="lua_isnumber") l4_int32_t, int, short, long, signed char
%{$1 = ($type)lua_tointeger(L, $input);%}

%typemap(in,checkfn="lua_isnumber") l4_umword_t, unsigned int, unsigned short,
        unsigned long, unsigned char
%{SWIG_contract_assert((lua_tointeger(L,$input)>=0),"number must not be negative")
$1 = ($type)lua_tointeger(L, $input);%}

%typemap(in,checkfn="lua_isnumber") enum SWIGTYPE
%{$1 = ($type)(int)lua_tointeger(L, $input);%}

%typemap(out) enum SWIGTYPE
%{  lua_pushinteger(L, (int)($1)); SWIG_arg++;%}

%typemap(out) l4_umword_t,l4_int32_t,int,short,long,
             unsigned int,unsigned short,unsigned long,
             signed char,unsigned char
%{  lua_pushinteger(L, $1); SWIG_arg++;%}

%typemap(out) l4_int32_t*,int*,short*,long*,
             unsigned int*,unsigned short*,unsigned long*,
             signed char*,unsigned char*
%{  lua_pushinteger(L, *$1); SWIG_arg++;%}

