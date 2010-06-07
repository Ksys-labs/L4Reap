/*
 * (c) 2008-2009 Technische Universität Dresden
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

#include <l4/re/util/event>
#include <l4/sys/factory>

namespace L4Re { namespace Util {

int
Event::init(L4::Cap<L4Re::Event> event,
            L4Re::Env const *env, L4Re::Cap_alloc *ca)
{
  Auto_cap<L4Re::Dataspace>::Cap ev_ds = ca->alloc<L4Re::Dataspace>();
  if (!ev_ds.is_valid())
    return -L4_ENOMEM;

  Auto_del_cap<L4::Irq>::Cap ev_irq = ca->alloc<L4::Irq>();
  if (!ev_irq.is_valid())
    return -L4_ENOMEM;

  int r;
  if ((r = l4_error(env->factory()->create_irq(ev_irq.get()))))
    return r;

  if ((r = l4_error(event->bind(0, ev_irq.get()))))
    return r;

  if ((r = event->get_buffer(ev_ds.get())))
    return r;

  long sz = ev_ds->size();
  if (sz < 0)
    return sz;

  Rm::Auto_region<void*> buf;

  if ((r = env->rm()->attach(&buf, sz, L4Re::Rm::Search_addr,
          ev_ds.get())))
    return r;

  _ev_buffer = L4Re::Event_buffer(buf.get(), sz);
  _ev_ds     = ev_ds;
  _ev_irq    = ev_irq;
  _buf       = buf;

  return 0;
}

}}
