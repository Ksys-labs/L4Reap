/**
 * \file   l4re/lib_main/src/__main.cc
 * \brief  Main
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

#include <stdlib.h>
#include <l4/crtn/crt0.h>
#include <l4/re/env>
#include <l4/util/util.h>


extern "C" void _exit(int code)  __attribute__ ((__noreturn__, __weak__));
#if 0
extern "C" void __thread_doexit(int code) throw();
extern "C" int main(int argc, char const * const*argv) throw();
extern l4re_env_t *l4re_global_env __attribute__((weak));

void
__main(int argc, char const *argv[]) throw()
{
  if (&l4re_global_env && argv)
    {
      l4_umword_t *auxp = (l4_umword_t*)&argv[argc] + 1;

      while (*auxp)
	++auxp;

      ++auxp;

      while (*auxp)
	{
	  if (*auxp == 0xf1)
	    {
	      l4re_global_env = (l4re_env_t*)auxp[1];
	      break;
	    }
	  auxp += 2;
	}
    }

  /* call constructors */
  crt0_construction();

  /* call main */
  exit(main(argc, argv));
}
#endif

void _exit(int code)
{
  L4Re::Env const *e;
  if (l4re_global_env && (e = L4Re::Env::env()) && e->parent().is_valid())
    e->parent()->signal(0, code);
  l4_sleep_forever();
}
#if 0
void __thread_doexit(int code) throw()
{
    /*
      fast fix for linker error
    */
}

void abort(void) throw() 
{
    /* We do not provide signals, hence no SIGABRT handling here */
    exit(1);
}
#endif
