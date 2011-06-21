#pragma once


// Pick up any OS-specific definitions.
#include <bits/os_defines.h>

// Pick up any CPU-specific definitions.
#include <bits/cpu_defines.h>

// The current version of the C++ library in compressed ISO date format.
#define __GLIBCXX__ 20090321

#ifdef __PIC__
#define __GXX_WEAK__ 1
#define _GLIBCXX_GTHREAD_USE_WEAK 1
#endif 

#if !defined(L4_MINIMAL_LIBC)
#define _GLIBCXX_HAVE_MBSTATE_T 1
#define _GLIBCXX_HAVE_WCHAR_H 1
#endif


// Macros for visibility.
// _GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY
// _GLIBCXX_VISIBILITY_ATTR
#define _GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY default

#if _GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY
# define _GLIBCXX_VISIBILITY_ATTR(V) __attribute__ ((__visibility__ (#V)))
#else
# define _GLIBCXX_VISIBILITY_ATTR(V) 
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

#ifdef _GLIBCXX_DEBUG
# define _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG 1
#endif

#ifdef _GLIBCXX_PARALLEL
# define _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL 1
#endif

// works with gcc 4.3.3
//#define _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION 1

#define _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION 0

// Defined if any namespace association modes are active.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG \
  || _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL \
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
// _GLIBCXX_BEGIN_POTENTIAL_NESTED_NAMESPACE
// _GLIBCXX_END_POTENTIAL_NESTED_NAMESPACE
#ifndef _GLIBCXX_USE_NAMESPACE_ASSOCIATION
# define _GLIBCXX_STD_D _GLIBCXX_STD
# define _GLIBCXX_STD_P _GLIBCXX_STD
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
# if _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG && !_GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL
#  define _GLIBCXX_STD_D __norm
#  define _GLIBCXX_STD_P _GLIBCXX_STD
#  define _GLIBCXX_STD __cxx1998
#  define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X _GLIBCXX_VISIBILITY_ATTR(default) { 
#  define _GLIBCXX_END_NAMESPACE }
#  define _GLIBCXX_EXTERN_TEMPLATE 0
# endif

// parallel
# if _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL && !_GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG 
#  define _GLIBCXX_STD_D _GLIBCXX_STD
#  define _GLIBCXX_STD_P __norm
#  define _GLIBCXX_STD __cxx1998
#  define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X _GLIBCXX_VISIBILITY_ATTR(default) { 
#  define _GLIBCXX_END_NAMESPACE }
#  define _GLIBCXX_EXTERN_TEMPLATE 0
# endif

// debug + parallel
# if _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL && _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG 
#  define _GLIBCXX_STD_D __norm
#  define _GLIBCXX_STD_P __norm
#  define _GLIBCXX_STD __cxx1998
#  define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X _GLIBCXX_VISIBILITY_ATTR(default) { 
#  define _GLIBCXX_END_NAMESPACE }
#  define _GLIBCXX_EXTERN_TEMPLATE 0
# endif

# if __NO_INLINE__ && !__GXX_WEAK__
#  warning currently using namespace associated mode which may fail \
   without inlining due to lack of weak symbols
# endif

# define _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, Y)  namespace X { namespace Y _GLIBCXX_VISIBILITY_ATTR(default) {
# define _GLIBCXX_END_NESTED_NAMESPACE } }
#endif

// Namespace associations for debug mode.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG
namespace std
{ 
  namespace __norm { } 
  namespace __debug { }
  namespace __cxx1998 { }

  using namespace __debug __attribute__ ((strong)); 
  using namespace __cxx1998 __attribute__ ((strong)); 
}
#endif

// Namespace associations for parallel mode.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_PARALLEL
namespace std
{ 
  namespace __norm { } 
  namespace __parallel { }
  namespace __cxx1998 { }

  using namespace __parallel __attribute__ ((strong));
  using namespace __cxx1998 __attribute__ ((strong)); 
}
#endif

// Namespace associations for versioning mode.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION
namespace std
{
  namespace _6 { }
  using namespace _6 __attribute__ ((strong));
}

namespace __gnu_cxx 
{ 
  namespace _6 { }
  using namespace _6 __attribute__ ((strong));
}

namespace std
{
  namespace tr1 
  { 
    namespace _6 { }
    using namespace _6 __attribute__ ((strong));
  }
}
#endif

// Define if compatibility should be provided for -mlong-double-64.
#undef _GLIBCXX_LONG_DOUBLE_COMPAT

// XXX GLIBCXX_ABI Deprecated
// Namespace associations for long double 128 mode.
_GLIBCXX_BEGIN_NAMESPACE(std)
#if defined _GLIBCXX_LONG_DOUBLE_COMPAT && defined __LONG_DOUBLE_128__
# define _GLIBCXX_LDBL_NAMESPACE __gnu_cxx_ldbl128::
# define _GLIBCXX_BEGIN_LDBL_NAMESPACE namespace __gnu_cxx_ldbl128 {
# define _GLIBCXX_END_LDBL_NAMESPACE }
  namespace __gnu_cxx_ldbl128 { }
  using namespace __gnu_cxx_ldbl128 __attribute__((__strong__));
#else
# define _GLIBCXX_LDBL_NAMESPACE
# define _GLIBCXX_BEGIN_LDBL_NAMESPACE
# define _GLIBCXX_END_LDBL_NAMESPACE
#endif
_GLIBCXX_END_NAMESPACE


// Allow use of "export template." This is currently not a feature
// that g++ supports.
// #define _GLIBCXX_EXPORT_TEMPLATE 1

// Allow use of the GNU syntax extension, "extern template." This
// extension is fully documented in the g++ manual, but in a nutshell,
// it inhibits all implicit instantiations and is used throughout the
// library to avoid multiple weak definitions for required types that
// are already explicitly instantiated in the library binary. This
// substantially reduces the binary size of resulting executables.
#ifndef _GLIBCXX_EXTERN_TEMPLATE
# define _GLIBCXX_EXTERN_TEMPLATE 1
#endif


// Certain function definitions that are meant to be overridable from
// user code are decorated with this macro.  For some targets, this
// macro causes these definitions to be weak.
#ifndef _GLIBCXX_WEAK_DEFINITION
# define _GLIBCXX_WEAK_DEFINITION
#endif

// Macro used to indicate that the native "C" includes, when compiled
// as "C++", have declarations in namespace std and not the global
// namespace. Note, this is unrelated to possible "C" compatibility
// includes that inject C90/C99 names into the global namespace.
#if __cplusplus == 199711L
# define _GLIBCXX_NAMESPACE_GLOBAL_INJECTION 1
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

// End of prewritten config; the discovered settings follow.


#define _GLIBCXX_HOSTED 1
#ifndef L4_MINIMAL_LIBC
# define _GLIBCXX__PTHREADS 1
#endif
#define _GLIBCXX_HAVE_UNISTD_H 1
// L4: for ARM builtins are usually linux specific functions, if at all
// available
#if defined(ARCH_x86) || defined(ARCH_amd64)
# define _GLIBCXX_ATOMIC_BUILTINS 1
#endif

#ifdef __USING_SJLJ_EXCEPTIONS__
# define _GLIBCXX_SJLJ_EXCEPTIONS 1
#endif
