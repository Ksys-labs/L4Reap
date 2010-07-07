/**
 * \file
 * \brief Event handling routines.
 */
/*
 * (c) 2008-2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
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

#include <l4/event/event>


namespace Event {

void
Event::attach_thread(pthread_t thread)
{
  if (!thread)
    thread = pthread_self();
  L4::Cap<L4::Thread> t(pthread_getl4cap(thread));
  _l4thread = t;
  attach();
}

Event::~Event()
{
  if (_pthread)
    pthread_cancel(_pthread);

  if (_irq.is_valid())
    {
      _irq->detach();
      L4Re::Util::cap_alloc.free(_irq, L4Re::This_task);
    }
}

void *
Event::event_loop(void *data)
{
  Event *e = reinterpret_cast<Event *>(data);
  while (1)
    {
      l4_msgtag_t res = e->_irq->receive(L4_IPC_NEVER);
      if (l4_ipc_error(res, l4_utcb()))
        continue;

      e->_event_func(data);
    }
}

}
