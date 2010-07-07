// vi:ft=cpp
/**
 * \internal
 * \file
 * \brief
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
#include <l4/sys/scheduler.h>
#include <l4/sys/types.h>
#include <l4/cxx/ipc_stream>
#include <l4/libkproxy/scheduler_svr>

namespace L4kproxy {

int Scheduler_svr::scheduler_dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  L4::Opcode op;
  ios >> op;
  switch (op)
    {
    case L4_SCHEDULER_INFO_OP:
        {
	  l4_sched_cpu_set_t cpus;
	  l4_umword_t cpu_max;

	    {
	      l4_umword_t gran_off;
	      ios >> gran_off;

	      cpus.offset = gran_off & 0x00ffffff;
	      cpus.granularity = (gran_off >> 24);
	      cpus.map = 0;
	    }

          int ret = _sched->info(&cpu_max, &cpus);

          if (ret == L4_EOK)
            ios << cpus.map << cpu_max;

          return ret;
        }
    case L4_SCHEDULER_RUN_THREAD_OP:
        {
	  l4_sched_param_t sp;
          L4::Snd_fpage thread;
	    {
	      l4_umword_t gran_off, prio, quantum;
	      ios >> gran_off >> sp.affinity.map >> prio >> quantum >> thread;
	      sp.prio = prio;
	      sp.quantum = quantum;
	      sp.affinity.offset = gran_off & 0x00ffffff;
	      sp.affinity.granularity = gran_off >> 24;
	    }

          return _sched->run_thread(received_thread(thread), sp);
        }
    case L4_SCHEDULER_IDLE_TIME_OP:
        {
	  l4_sched_cpu_set_t cpus;
	    {
	      l4_umword_t gran_off;
	      ios >> gran_off >> cpus.map;
	      cpus.offset = gran_off & 0x00ffffff;
	      cpus.granularity = gran_off >> 24;
	    }
          return _sched->idle_time(cpus);
        }
    default:
      return -L4_ENOSYS;
    }
}

}
