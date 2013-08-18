/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/vbus/vdevice-ops.h>

#include "vicu.h"
#include "vbus_factory.h"
#include "server.h"
#include "main.h"
#include "debug.h"

#include <l4/re/util/cap_alloc>
#include <l4/re/namespace>
#include <l4/re/env>
#include <l4/re/error_helper>
#include <l4/sys/icu>
#include <l4/sys/irq>
#include <l4/sys/debugger.h>
#include <l4/sys/kdebug.h>

#include <cassert>
#include <map>

namespace Vi {

using L4Re::Util::Auto_cap;
using L4Re::chksys;
using L4Re::chkcap;

namespace {

typedef std::map<unsigned, Kernel_irq_pin *> Irq_map;
static Irq_map _real_irqs;

}

Kernel_irq_pin *
Sw_icu::real_irq(unsigned n)
{
  Irq_map::const_iterator f = _real_irqs.find(n);
  Kernel_irq_pin *r;
  if (f == _real_irqs.end() || !(*f).second)
    _real_irqs[n] = r = new Kernel_irq_pin(n);
  else
    r = (*f).second;

  return r;
}


Sw_icu::Sw_icu()
{
  add_feature(this);
  registry->register_obj(this);
}

Sw_icu::~Sw_icu()
{
  registry->unregister_obj(this);
}

int
Sw_icu::bind_irq(l4_msgtag_t tag, unsigned irqn, L4::Ipc::Snd_fpage const &/*irqc*/)
{
  if (tag.items() < 1)
    return -L4_EINVAL;

  d_printf(DBG_ALL, "%s[%p]: bind_irq(%d, ...)\n", name(), this, irqn);

  Irq_set::Iterator i = _irqs.find(irqn);
  if (i == _irqs.end())
    return -L4_ENOENT;

  int err = i->bind(rcv_cap);
  if (err < 0)
    d_printf(DBG_ERR, "ERROR: binding irq %d, result is %d (%s)\n", irqn, err, l4sys_errtostr(err));

  return err;
}

int
Sw_icu::unbind_irq(l4_msgtag_t tag, unsigned irqn, L4::Ipc::Snd_fpage const &/*irqc*/)
{
  if (tag.items() < 1)
    return -L4_EINVAL;

  d_printf(DBG_ALL, "%s[%p]: unbind_irq(%d, ...)\n", name(), this, irqn);

  Irq_set::Iterator i = _irqs.find(irqn);
  if (i == _irqs.end())
    return -L4_ENOENT;

  // could check the validity of the cap too, however we just don't care
  return i->unbind();
}

int
Sw_icu::set_mode(l4_msgtag_t /*tag*/, unsigned irqn, l4_umword_t mode)
{
  Irq_set::Iterator i = _irqs.find(irqn);
  if (i == _irqs.end())
    return -L4_ENOENT;

  return i->set_mode(mode);
}

int
Sw_icu::unmask_irq(l4_msgtag_t /*tag*/, unsigned irqn)
{
  Irq_set::Iterator i = _irqs.find(irqn);
  if (i == _irqs.end())
    return -L4_ENOENT;

  if (!i->unmask_via_icu())
    return -L4_EINVAL;

  return i->unmask();
}


bool
Sw_icu::irqs_allocated(Resource const *r)
{
  for (unsigned n = r->start(); n <= r->end(); ++n)
    {
      if (_irqs.find(n) == _irqs.end())
	return false;
    }

  return true;
}

bool
Sw_icu::add_irqs(Resource const *r)
{
  for (unsigned n = r->start(); n <= r->end(); ++n)
    {
      if (_irqs.find(n) != _irqs.end())
	continue;

      Kernel_irq_pin *ri = real_irq(n);
      if (!ri)
        {
          d_printf(DBG_ERR, "ERROR: No IRQ%d available.\n", n);
          continue;
        }
      Sw_irq_pin *irq = new Sw_irq_pin(ri, n, r->flags());
      _irqs.insert(irq);
    }
  return true;
}

bool
Sw_icu::add_irq(unsigned n, unsigned flags, Io_irq_pin *be)
{
  if (_irqs.find(n) == _irqs.end())
    return false;

  Sw_irq_pin *irq = new Sw_irq_pin(be, n, flags);
  _irqs.insert(irq);
  return true;
}

int
Sw_icu::alloc_irq(unsigned flags, Io_irq_pin *be)
{
  unsigned i;
  for (i = 1; i < 1000; ++i)
    if (_irqs.find(i) == _irqs.end())
      break;

  if (i == 1000)
    return -1;

  Sw_irq_pin *irq = new Sw_irq_pin(be, i, flags);
  _irqs.insert(irq);
  return i;
}


int
Sw_icu::dispatch(l4_umword_t /*obj*/, L4::Ipc::Iostream &ios)
{
  l4_umword_t op, irqn;
  L4::Ipc::Snd_fpage irqc;
  l4_msgtag_t tag;
  ios >> tag >> op >> irqn;

  if (tag.label() != L4_PROTO_IRQ)
    return -L4_EBADPROTO;

  switch (op)
    {
    case L4_ICU_OP_BIND:
      ios >> irqc;
      return bind_irq(tag, irqn, irqc);

    case L4_ICU_OP_UNBIND:
      ios >> irqc;
      return unbind_irq(tag, irqn, irqc);

    case L4_ICU_OP_UNMASK:
      unmask_irq(tag, irqn);
      return -L4_ENOREPLY;

    case L4_ICU_OP_SET_MODE:
	{
	  l4_umword_t mode;
	  ios >> mode;
	  return set_mode(tag, irqn, mode);
	}

    default: return -L4_ENOSYS;
    }
}

int
Sw_icu::dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc::Iostream &ios)
{
  if (func != L4vbus_vicu_get_cap)
    return -L4_ENOSYS;

  ios << obj_cap();
  return L4_EOK;
}

//static VBus_factory<Sw_icu> __vicu_factory("Vicu");

int
Sw_icu::Sw_irq_pin::trigger() const
{
  return l4_error(_irq->trigger());
}


unsigned
Sw_icu::Sw_irq_pin::l4_type() const
{
  unsigned m = type();
  unsigned r = (m & S_irq_type_mask) / Resource::Irq_type_base;
  return r;
}

int
Sw_icu::Sw_irq_pin::set_mode(l4_umword_t mode)
{
  if (!(_state & S_allow_set_mode))
    {
      unsigned t = l4_type();

      mode = (mode & L4_IRQ_F_MASK) | 1;

      if (t != L4_IRQ_F_NONE
          && t != mode)
        d_printf(DBG_WARN, "WARNING: Changing type of IRQ %d from %x to %lx prohibited\n",
                 _irqn, t, mode);
      return 0;
    }

  return _master->set_mode(mode);
}

void
Sw_icu::Sw_irq_pin::allocate_master_irq()
{
  assert (_master->shared());
  Auto_cap<L4::Irq>::Cap lirq = chkcap(L4Re::Util::cap_alloc.alloc<L4::Irq>(),
      "allocating IRQ capability");
  // printf("IRQ mode = %x -> %x\n", type(), l4_type());
  chksys(L4Re::Env::env()->factory()->create(lirq.get(), L4_PROTO_IRQ) << l4_umword_t(1), "allocating IRQ");
  chksys(_master->bind(lirq.get(), l4_type()), "binding IRQ");
  _master->irq(lirq.release());
  _master->set_chained(true);
}


int
Sw_icu::Sw_irq_pin::bind(L4::Cap<void> rc)
{
  if (_irq.is_valid())
    {
      if (_irq.get().validate(L4Re::This_task).label() > 0)
	return -L4_EEXIST;

      _unbind();
    }

  if (bound())
    return -L4_EPERM;

  Auto_cap<L4::Irq>::Cap irq =
    chkcap(L4Re::Util::cap_alloc.alloc<L4::Irq>(), "allocating IRQ capability");

  irq.get().move(L4::cap_cast<L4::Irq>(rc));

  if (_master->shared() && !_master->chained() && _master->sw_irqs() == 0)
    {
      allocate_master_irq();
      assert (_master->chained());
    }

  if (!_master->chained())
    {
      // the first irq shall be attached to a hw irq
      d_printf(DBG_DEBUG2, "IRQ %d -> client\nIRQ mode = %x -> %x\n",
              irqn(), type(), l4_type());
      int err = _master->bind(irq.get(), l4_type());
      if (err < 0)
	return err;

      _irq = irq;
      _master->irq(_irq.get());
      _master->inc_sw_irqs();
      _state |= S_bound;
      if (err == 1)
	_state |= S_unmask_via_icu;

      d_printf(DBG_DEBUG2, "  bound irq %u -> err=%d\n", irqn(), err);
      return err;
    }

  d_printf(DBG_DEBUG2, "IRQ %d -> proxy -> %d clients\n", irqn(), _master->sw_irqs() + 1);
  L4Re::chksys(_master->irq()->chain(l4_umword_t(_master), irq.get()), "attach");
  _irq = irq;
  _master->inc_sw_irqs();

  return 0;
}

int
Sw_icu::Sw_irq_pin::_unbind()
{
  int err = 0;
  _master->dec_sw_irqs();
  if (_master->sw_irqs() == 0)
    {
      if (_master->chained())
	L4Re::Util::cap_alloc.free(_master->irq());

      _master->irq(L4::Cap<L4::Irq>::Invalid);
      _master->set_chained(false);
    }

  _irq = L4::Cap<L4::Irq>::Invalid;

  _state &= ~S_bound;
  return err;
}

int
Sw_icu::Sw_irq_pin::unbind()
{
  if (!_master)
    return -L4_EINVAL;

  if (!_master->sw_irqs())
    return -L4_EINVAL;

  return _unbind();
}

}
