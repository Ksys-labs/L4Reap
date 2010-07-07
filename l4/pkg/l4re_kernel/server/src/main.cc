/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/namespace>
#include <l4/re/dataspace>
#include <l4/re/mem_alloc>
#include <l4/re/rm>
#include <l4/re/log>
#include <l4/re/env>
#include <l4/re/error_helper>
#include <l4/re/util/env_ns>

#include <l4/cxx/exceptions>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>
#include <l4/sys/debugger.h>
#include <l4/sys/scheduler>
#include <l4/sys/thread>


#include <cstdlib>
#include <cstring>

#include "region.h"
#include "page_alloc.h"
#include "globals.h"
#include "loader_elf.h"
#include "debug.h"
#include "dispatcher.h"

#include <l4/re/elf_aux.h>

L4RE_ELF_AUX_ELEM_T(l4re_elf_aux_mword_t, __stack_addr, L4RE_ELF_AUX_T_STACK_ADDR, 0xb1000000);

using L4::Cap;
using L4Re::Dataspace;

extern "C" int main(int argc, char const *argv[]);

static Elf_loader loader;
Region_map __local_rm;
L4::Cap<void> rcv_cap;

class Loop_hooks :
  public L4::Ipc_svr::Ignore_errors,
  public L4::Ipc_svr::Default_timeout,
  public L4::Ipc_svr::Compound_reply
{
public:
  static void setup_wait(L4::Ipc::Istream &istr, bool)
  {
    istr.reset();
    istr << L4::Small_buf(rcv_cap.cap(), L4_RCV_ITEM_LOCAL_ID);
    l4_utcb_br_u(istr.utcb())->bdr = 0;
  }
};


static L4::Server<Loop_hooks> server(l4_utcb());

static void insert_regions(l4re_aux_t *a)
{
  unsigned long n = a->cnt;
  for (unsigned long i = 0; i < n; ++i)
    {
      void *x = __local_rm.attach((void*)a->vm[i].start, a->vm[i].size,
	  Region_handler(L4::cap_reinterpret_cast<L4Re::Dataspace>(L4Re::Env::env()->rm()),
	                 L4_INVALID_CAP, 0, L4Re::Rm::Pager), 0);
      if (!x)
	{
	  L4::cerr << "l4re: error while initializing region mapper\n";
	  exit(1);
	}
    }
}

int main(int argc, char const *argv[])
{
  Dbg::level = Dbg::Info | Dbg::Warn;

  Dbg boot(Dbg::Boot);
  try
    {

  l4_umword_t *envp = (l4_umword_t*)&argv[argc] + 1;
  l4_umword_t *auxp = envp;
  while (*auxp)
    ++auxp;

  ++auxp;

  Global::argc = argc;
  Global::argv = argv;
  Global::envp = (char const * const*)envp;
#if 0
  L4::cout << "ARGC=" << argc << "\n"
           << "ARGV=" << argv << "\n"
	   << "ENVP=" << envp << "\n"
	   << "AUXP=" << auxp << "\n";

  for (int i = 0; i < argc; ++i)
    L4::cout << "  arg: '" << argv[i] << "'\n";

  for (char const *const *e = Global::envp; *e; ++e)
    L4::cout << "  env: '" << *e << "'\n";
#endif

  L4Re::Env *const env = const_cast<L4Re::Env*>(L4Re::Env::env());

  while (*auxp)
    {
      if (*auxp == 0xf0)
	Global::l4re_aux = (l4re_aux_t*)auxp[1];
      auxp += 2;
    }

#if 0
  L4::cout << "AUX=" << aux << "\n"
           << "ENV=" << env << "\n";
#endif

  //L4::cout << "Binary: " << aux->binary << " " << env << "\n";
  if (!env || !Global::l4re_aux)
    {
      Err(Err::Fatal).printf("invalid AUX vectors...\n");
      exit(1);
    }

    {
      char s[15];
      char *t = strstr(Global::l4re_aux->binary, "rom/");
      s[0] = '#';
      strncpy(s + 1, t ? t + 4 : Global::l4re_aux->binary, sizeof(s)-1);
      s[sizeof(s)-1] = 0;
      l4_debugger_set_object_name(L4_BASE_THREAD_CAP, s);
      l4_debugger_set_object_name(L4_BASE_TASK_CAP, s + 1);
    }

  Dbg::level = Global::l4re_aux->dbg_lvl;

  env->first_free_cap(env->first_free_cap() + Global::Max_local_rm_caps);
  boot.printf("first free cap for application: 0x%lx\n", env->first_free_cap());

    {
      Cap<L4Re::Mem_alloc> obj = env->mem_alloc();

      if (!obj.is_valid())
	{
	  Err(Err::Fatal).printf("could not find the base memory allocator\n");
	  return -1;
	}
      Global::allocator = obj;
    }

  boot.printf("setup page allocator\n");
  Single_page_alloc_base::init(Global::allocator);

  Global::local_rm = &__local_rm;

  rcv_cap = Global::cap_alloc.alloc<void>();

  boot.printf("setup local region mapping\n");
  __local_rm.init();
  boot.printf("adding regions from remote region mapper\n");
  insert_regions(Global::l4re_aux);

  boot.printf("initializing local page allocator\n");
  Single_page_alloc_base::init_local_rm(Global::local_rm);


  L4::Cap<Dataspace> file;

  boot.printf("load binary '%s'\n", Global::l4re_aux->binary);

  file = L4Re_app_model::open_file(Global::l4re_aux->binary);

  loader.start(file, Global::local_rm, Global::l4re_aux);

  // Raise RM prio to its MCP
  env->scheduler()->run_thread(env->main_thread(), l4_sched_param(L4_SCHED_MAX_PRIO));

  boot.printf("Start server loop\n");
  server.loop(Dispatcher());
  } catch (L4::Runtime_error const &e)
    {
      Err(Err::Fatal).printf("Exception %s: '%s'\n", e.str(), e.extra_str());
      L4::cerr << e;
      return 1;
    }
  catch (L4::Base_exception const &e)
    {
      Err(Err::Fatal).printf("Exception %s\n", e.str());
      L4::cerr << e;
      return 1;
    }

  return 0;
}
