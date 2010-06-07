// Predefined symbols and macros -*- C++ -*-

// Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
// 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file c++config.h
 *  This is an internal header file, included by other library headers.
 *  You should not attempt to use it directly.
 */

#ifndef _GLIBCXX_CXX_CONFIG_H
#define _GLIBCXX_CXX_CONFIG_H 1

// The current version of the C++ library in compressed ISO date format.
#define __GLIBCXX__ 20100311

// Macros for visibility.
// _GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY
// _GLIBCXX_VISIBILITY_ATTR
#define _GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY default

#if _GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY
# define _GLIBCXX_VISIBILITY_ATTR(V) __attribute__ ((__visibility__ (#V)))
#else
// If this is not supplied by the OS-specific or CPU-specific
// headers included below, it will be defined to an empty default.
# define _GLIBCXX_VISIBILITY_ATTR(V) _GLIBCXX_PSEUDO_VISIBILITY(V)
#endif

// Macros for deprecated.
// _GLIBCXX_DEPRECATED
// _GLIBCXX_DEPRECATED_ATTR
#ifndef _GLIBCXX_DEPRECATED
# define _GLIBCXX_DEPRECATED 1
#endif

#if defined(__DEPRECATED) && defined(__GXX_EXPERIMENTAL_CXX0X__)
# define _GLIBCXX_DEPRECATED_ATTR __attribute__ ((__deprecated__))
#else
# define _GLIBCXX_DEPRECATED_ATTR
#endif

// Macros for activating various namespace association modes.
// _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG
// _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL
// _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION

// Guide to libstdc++ namespaces.
/*
  namespace std
  {
    namespace __debug { }
    namespace __parallel { }
    namespace __norm { } // __normative, __shadow, __replaced
    namespace __cxx1998 { }

    namespace tr1 { }
  }
*/
#if __cplusplus

#ifdef _GLIBCXX_DEBUG
# define _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG 1
#endif

#ifdef _GLIBCXX_PARALLEL
# define _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL 1
#endif

// Namespace association for profile
#ifdef _GLIBCXX_PROFILE
# define _GLIBCXX_NAMESPACE_ASSOCIATION_PROFILE 1
#endif

#define _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION 0

// Defined if any namespace association modes are active.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG \
  || _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL \
  || _GLIBCXX_NAMESPACE_ASSOCIATION_PROFILE \
  || _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION
# define _GLIBCXX_USE_NAMESPACE_ASSOCIATION 1
#endif

// Macros for namespace scope. Either namespace std:: or the name
// of some nested namespace within it.
// _GLIBCXX_STD
// _GLIBCXX_STD_D
// _GLIBCXX_STD_P
//
// Macros for enclosing namespaces and possibly nested namespaces.
// _GLIBCXX_BEGIN_NAMESPACE
// _GLIBCXX_END_NAMESPACE
// _GLIBCXX_BEGIN_NESTED_NAMESPACE
// _GLIBCXX_END_NESTED_NAMESPACE
#ifndef _GLIBCXX_USE_NAMESPACE_ASSOCIATION
# define _GLIBCXX_STD_D _GLIBCXX_STD
# define _GLIBCXX_STD_P _GLIBCXX_STD
# define _GLIBCXX_STD_PR _GLIBCXX_STD
# define _GLIBCXX_STD std
# define _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, Y) _GLIBCXX_BEGIN_NAMESPACE(X)
# define _GLIBCXX_END_NESTED_NAMESPACE _GLIBCXX_END_NAMESPACE
# define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X _GLIBCXX_VISIBILITY_ATTR(default) {
# define _GLIBCXX_END_NAMESPACE }
#else

# if _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION // && not anything else
#  define _GLIBCXX_STD_D _GLIBCXX_STD
#  define _GLIBCXX_STD_P _GLIBCXX_STD
#  define _GLIBCXX_STD _6
#  define _GLIBCXX_BEGIN_NAMESPACE(X) _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, _6)
#  define _GLIBCXX_END_NAMESPACE _GLIBCXX_END_NESTED_NAMESPACE
# endif

//  debug
# if _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG && !_GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL && !_GLIBCXX_NAMESPACE_ASSOCIATION_PROFILE
#  define _GLIBCXX_STD_D __norm
#  define _GLIBCXX_STD_P _GLIBCXX_STD
#  define _GLIBCXX_STD __cxx1998
#  define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X _GLIBCXX_VISIBILITY_ATTR(default) { 
#  define _GLIBCXX_END_NAMESPACE }
#  define _GLIBCXX_EXTERN_TEMPLATE -1
# endif

// parallel
# if _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL && !_GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG && !_GLIBCXX_NAMESPACE_ASSOCIATION_PROFILE
#  define _GLIBCXX_STD_D _GLIBCXX_STD
#  define _GLIBCXX_STD_P __norm
#  define _GLIBCXX_STD __cxx1998
#  define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X _GLIBCXX_VISIBILITY_ATTR(default) { 
#  define _GLIBCXX_END_NAMESPACE }
# endif

// debug + parallel
# if _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL && _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG  && !_GLIBCXX_NAMESPACE_ASSOCIATION_PROFILE
#  define _GLIBCXX_STD_D __norm
#  define _GLIBCXX_STD_P __norm
#  define _GLIBCXX_STD __cxx1998
#  define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X _GLIBCXX_VISIBILITY_ATTR(default) { 
#  define _GLIBCXX_END_NAMESPACE }
#  define _GLIBCXX_EXTERN_TEMPLATE -1
# endif

// profile
# if _GLIBCXX_NAMESPACE_ASSOCIATION_PROFILE
#  if _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL || _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG
#   error Cannot use -D_GLIBCXX_PROFILE with -D_GLIBCXX_DEBUG or \
    -D_GLIBCXX_PARALLEL
#  endif
#  define _GLIBCXX_STD_D __norm
#  define _GLIBCXX_STD_P _GLIBCXX_STD
#  define _GLIBCXX_STD_PR __norm
#  define _GLIBCXX_STD __cxx1998
#  define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X _GLIBCXX_VISIBILITY_ATTR(default) { 
#  define _GLIBCXX_END_NAMESPACE }
# endif

# if __NO_INLINE__ && !__GXX_WEAK__
#  warning currently using namespace associated mode which may fail \
   without inlining due to lack of weak symbols
# endif

# define _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, Y)  namespace X { namespace Y _GLIBCXX_VISIBILITY_ATTR(default) {
# define _GLIBCXX_END_NESTED_NAMESPACE } }
#endif

// Namespace associations for debug mode.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG && !_GLIBCXX_NAMESPACE_ASSOCIATION_PROFILE
namespace std
{ 
  namespace __norm { } 
  inline namespace __debug { }
  inline namespace __cxx1998 { }
}
#endif

// Namespace associations for parallel mode.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL
namespace std
{ 
  namespace __norm { } 
  inline namespace __parallel { }
  inline namespace __cxx1998 { }
}
#endif

// Namespace associations for profile mode
#if _GLIBCXX_NAMESPACE_ASSOCIATION_PROFILE
namespace std
{ 
  namespace __norm { } 
  inline namespace __profile { }
  inline namespace __cxx1998 { }
}
#endif

// Namespace associations for versioning mode.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION
namespace std
{
  inline namespace _6 { }
}

namespace __gnu_cxx 
{ 
  inline namespace _6 { }
}

namespace std
{
  namespace tr1 
  { 
    inline namespace _6 { }
  }
}
#endif

// XXX GLIBCXX_ABI Deprecated
// Define if compatibility should be provided for -mlong-double-64
#undef _GLIBCXX_LONG_DOUBLE_COMPAT

// Namespace associations for long double 128 mode.
#if defined _GLIBCXX_LONG_DOUBLE_COMPAT && defined __LONG_DOUBLE_128__ 
namespace std
{
  inline namespace __gnu_cxx_ldbl128 { }
}
# define _GLIBCXX_LDBL_NAMESPACE __gnu_cxx_ldbl128::
# define _GLIBCXX_BEGIN_LDBL_NAMESPACE namespace __gnu_cxx_ldbl128 {
# define _GLIBCXX_END_LDBL_NAMESPACE }
#else
# define _GLIBCXX_LDBL_NAMESPACE
# define _GLIBCXX_BEGIN_LDBL_NAMESPACE
# define _GLIBCXX_END_LDBL_NAMESPACE
#endif


// Defines for C compatibility. In particular, define extern "C"
// linkage only when using C++.
# define _GLIBCXX_BEGIN_EXTERN_C extern "C" {
# define _GLIBCXX_END_EXTERN_C }

#else // !__cplusplus
# undef _GLIBCXX_BEGIN_NAMESPACE
# undef _GLIBCXX_END_NAMESPACE
# define _GLIBCXX_BEGIN_NAMESPACE(X) 
# define _GLIBCXX_END_NAMESPACE 
# define _GLIBCXX_BEGIN_EXTERN_C
# define _GLIBCXX_END_EXTERN_C 
#endif

// First includes.

// Pick up any OS-specific definitions.
#include <bits/os_defines.h>

// Pick up any CPU-specific definitions.
#include <bits/cpu_defines.h>

// If platform uses neither visibility nor psuedo-visibility,
// specify empty default for namespace annotation macros.
#ifndef _GLIBCXX_PSEUDO_VISIBILITY
#define _GLIBCXX_PSEUDO_VISIBILITY(V)
#endif

// Allow use of "export template." This is currently not a feature
// that g++ supports.
// #define _GLIBCXX_EXPORT_TEMPLATE 1

// Allow use of the GNU syntax extension, "extern template." This
// extension is fully documented in the g++ manual, but in a nutshell,
// it inhibits all implicit instantiations and is used throughout the
// library to avoid multiple weak definitions for required types that
// are already explicitly instantiated in the library binary. This
// substantially reduces the binary size of resulting executables.

// Special case: _GLIBCXX_EXTERN_TEMPLATE == -1 disallows extern
// templates only in basic_string, thus activating its debug-mode
// checks even at -O0.
#ifndef _GLIBCXX_EXTERN_TEMPLATE
# define _GLIBCXX_EXTERN_TEMPLATE 1
#endif

// Certain function definitions that are meant to be overridable from
// user code are decorated with this macro.  For some targets, this
// macro causes these definitions to be weak.
#ifndef _GLIBCXX_WEAK_DEFINITION
# define _GLIBCXX_WEAK_DEFINITION
#endif

// Assert.
// Avoid the use of assert, because we're trying to keep the <cassert>
// include out of the mix.
#if !defined(_GLIBCXX_DEBUG) && !defined(_GLIBCXX_PARALLEL)
#define __glibcxx_assert(_Condition)
#else
_GLIBCXX_BEGIN_NAMESPACE(std)
  // Avoid the use of assert, because we're trying to keep the <cassert>
  // include out of the mix.
  inline void
  __replacement_assert(const char* __file, int __line, 
		       const char* __function, const char* __condition)
  {
    __builtin_printf("%s:%d: %s: Assertion '%s' failed.\n", __file, __line,
		     __function, __condition);
    __builtin_abort();
  }
_GLIBCXX_END_NAMESPACE

#define __glibcxx_assert(_Condition)                               	\
  do 								        \
  {							      		\
    if (! (_Condition))                                                 \
      std::__replacement_assert(__FILE__, __LINE__, 			\
				__PRETTY_FUNCTION__, #_Condition);	\
  } while (false)
#endif

// The remainder of the prewritten config is automatic; all the
// user hooks are listed above.

// Create a boolean flag to be used to determine if --fast-math is set.
#ifdef __FAST_MATH__
# define _GLIBCXX_FAST_MATH 1
#else
# define _GLIBCXX_FAST_MATH 0
#endif

// This marks string literals in header files to be extracted for eventual
// translation.  It is primarily used for messages in thrown exceptions; see
// src/functexcept.cc.  We use __N because the more traditional _N is used
// for something else under certain OSes (see BADNAMES).
#define __N(msgid)     (msgid)

// For example, <windows.h> is known to #define min and max as macros...
#undef min
#undef max

#ifndef _GLIBCXX_PURE
# define _GLIBCXX_PURE __attribute__ ((__pure__))
#endif

#ifndef _GLIBCXX_CONST
# define _GLIBCXX_CONST __attribute__ ((__const__))
#endif

#ifndef _GLIBCXX_NORETURN
# define _GLIBCXX_NORETURN __attribute__ ((__noreturn__))
#endif

#ifndef _GLIBCXX_NOTHROW
# ifdef __cplusplus
#  define _GLIBCXX_NOTHROW throw() 
# else
#  define _GLIBCXX_NOTHROW __attribute__((__nothrow__))
# endif
#endif

// End of prewritten config; the discovered settings follow.

/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the <stdint.h> header file. */
#define _GLIBCXX_HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define _GLIBCXX_HAVE_STDLIB_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define _GLIBCXX_HAVE_UNISTD_H 1


#if defined(ARCH_x86) || defined(ARCH_amd64)

/* Define if builtin atomic operations for bool are supported on this host. */
#define _GLIBCXX_ATOMIC_BUILTINS_1 1

/* Define if builtin atomic operations for short are supported on this host.
   */
#define _GLIBCXX_ATOMIC_BUILTINS_2 1

/* Define if builtin atomic operations for int are supported on this host. */
#define _GLIBCXX_ATOMIC_BUILTINS_4 1
#endif

#if defined(ARCH_amd64)
/* Define if builtin atomic operations for long long are supported on this
   host. */
#define _GLIBCXX_ATOMIC_BUILTINS_8 1
#endif

/* Define to use concept checking code from the boost libraries. */
#undef _GLIBCXX_CONCEPT_CHECKS

/* Define if a fully dynamic basic_string is wanted. */
#undef _GLIBCXX_FULLY_DYNAMIC_STRING

/* Define if gthreads library is available. */
#define _GLIBCXX_HAS_GTHREADS 1

/* Define to 1 if a full hosted library is built, or 0 if freestanding. */
#define _GLIBCXX_HOSTED 1

/* Define if compatibility should be provided for -mlong-double-64. */
#undef _GLIBCXX_LONG_DOUBLE_COMPAT

/* Define if ptrdiff_t is int. */
#undef _GLIBCXX_PTRDIFF_T_IS_INT

/* Define if using setrlimit to set resource limits during "make check" */
#undef _GLIBCXX_RES_LIMITS

/* Define if size_t is unsigned int. */
#undef _GLIBCXX_SIZE_T_IS_UINT

#ifdef __USING_SJLJ_EXCEPTIONS__
/* Define if the compiler is configured for setjmp/longjmp exceptions. */
#define _GLIBCXX_SJLJ_EXCEPTIONS 1
#endif

/* Define if EOF == -1, SEEK_CUR == 1, SEEK_END == 2. */
#undef _GLIBCXX_STDIO_MACROS

/* Define to use symbol versioning in the shared library. */
#undef _GLIBCXX_SYMVER

/* Define to use darwin versioning in the shared library. */
#undef _GLIBCXX_SYMVER_DARWIN

/* Define to use GNU versioning in the shared library. */
#undef _GLIBCXX_SYMVER_GNU

/* Define to use GNU namespace versioning in the shared library. */
#undef _GLIBCXX_SYMVER_GNU_NAMESPACE

/* Define if C99 functions or macros from <wchar.h>, <math.h>, <complex.h>,
   <stdio.h>, and <stdlib.h> can be used or exposed. */
#undef _GLIBCXX_USE_C99

/* Define if C99 functions in <complex.h> should be used in <complex>. Using
   compiler builtins for these functions requires corresponding C99 library
   functions to be present. */
#undef _GLIBCXX_USE_C99_COMPLEX

/* Define if C99 functions in <complex.h> should be used in <tr1/complex>.
   Using compiler builtins for these functions requires corresponding C99
   library functions to be present. */
#undef _GLIBCXX_USE_C99_COMPLEX_TR1

/* Define if C99 functions in <ctype.h> should be imported in <tr1/cctype> in
   namespace std::tr1. */
#undef _GLIBCXX_USE_C99_CTYPE_TR1

/* Define if C99 functions in <fenv.h> should be imported in <tr1/cfenv> in
   namespace std::tr1. */
#undef _GLIBCXX_USE_C99_FENV_TR1

/* Define if C99 functions in <inttypes.h> should be imported in
   <tr1/cinttypes> in namespace std::tr1. */
#undef _GLIBCXX_USE_C99_INTTYPES_TR1

/* Define if wchar_t C99 functions in <inttypes.h> should be imported in
   <tr1/cinttypes> in namespace std::tr1. */
#undef _GLIBCXX_USE_C99_INTTYPES_WCHAR_T_TR1

/* Define if C99 functions or macros in <math.h> should be imported in <cmath>
   in namespace std. */
#undef _GLIBCXX_USE_C99_MATH

/* Define if C99 functions or macros in <math.h> should be imported in
   <tr1/cmath> in namespace std::tr1. */
#undef _GLIBCXX_USE_C99_MATH_TR1

/* Define if C99 types in <stdint.h> should be imported in <tr1/cstdint> in
   namespace std::tr1. */
#undef _GLIBCXX_USE_C99_STDINT_TR1

/* Defined if clock_gettime has monotonic clock support. */
#undef _GLIBCXX_USE_CLOCK_MONOTONIC

/* Defined if clock_gettime has realtime clock support. */
#undef _GLIBCXX_USE_CLOCK_REALTIME

/* Define if ISO/IEC TR 24733 decimal floating point types are supported on
   this host. */
#undef _GLIBCXX_USE_DECIMAL_FLOAT

/* Defined if gettimeofday is available. */
#undef _GLIBCXX_USE_GETTIMEOFDAY

/* Define if LFS support is available. */
#undef _GLIBCXX_USE_LFS

/* Define if code specialized for long long should be used. */
#undef _GLIBCXX_USE_LONG_LONG

/* Defined if nanosleep is available. */
#undef _GLIBCXX_USE_NANOSLEEP

/* Define if NLS translations are to be used. */
#undef _GLIBCXX_USE_NLS

/* Define if /dev/random and /dev/urandom are available for the random_device
   of TR1 (Chapter 5.1). */
#undef _GLIBCXX_USE_RANDOM_TR1

/* Defined if sched_yield is available. */
#undef _GLIBCXX_USE_SCHED_YIELD

/* Define if code specialized for wchar_t should be used. */
#undef _GLIBCXX_USE_WCHAR_T


#endif
