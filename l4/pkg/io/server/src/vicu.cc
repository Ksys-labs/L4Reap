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

#include <l4/re/util/cap_alloc>
#include <l4/re/namespace>
#include <l4/re/env>
#include <l4/re/error_helper>
#include <l4/sys/icu>
#include <l4/sys/irq>
#include <l4/sys/debugger.h>
#include <l4/sys/kdebug.h>

#include <cassert>
#include <pthread.h>
#include <pthread-l4.h>

namespace Vi {

using L4Re::Util::Auto_cap;
using L4Re::chksys;
using L4Re::chkcap;


Sw_icu::Real_irq_pin Sw_icu::_real_irqs[256];

Sw_icu::Real_irq_pin *
Sw_icu::real_irq(unsigned n)
{
  if (n >= sizeof(_real_irqs)/sizeof(_real_irqs[0]))
    return 0;

  return &_real_irqs[n];
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
Sw_icu::bind_irq(l4_msgtag_t tag, unsigned irqn, L4::Snd_fpage const &/*irqc*/)
{
  if (tag.items() < 1)
    return -L4_EINVAL;


  // printf("%s[%p]: bind_irq(%d, ...)\n", name(), this, irqn);

  Irq_set::iterator i = _irqs.find(irqn);
  if (i == _irqs.end())
    return -L4_ENOENT;

  int err = i->bind(rcv_cap);
  if (err < 0)
    printf("ERROR: binding irq %d, result is %d (%s)\n", irqn, err, l4sys_errtostr(err));

  return err;
}

int
Sw_icu::unbind_irq(l4_msgtag_t tag, unsigned irqn, L4::Snd_fpage const &/*irqc*/)
{
  if (tag.items() < 1)
    return -L4_EINVAL;
  // printf("%s[%p]: unbind_irq(%d, ...)\n", name(), this, irqn);

  Irq_set::iterator i = _irqs.find(irqn);
  if (i == _irqs.end())
    return -L4_ENOENT;

  // could check the validity of the cap too, however we just don't care
  return i->unbind();
}

int
Sw_icu::set_mode(l4_msgtag_t /*tag*/, unsigned irqn, l4_umword_t mode)
{
  Irq_set::iterator i = _irqs.find(irqn);
  if (i == _irqs.end())
    return -L4_ENOENT;

  if (i->l4_type() != (mode & 0x6))
    {
      printf("Changing type of IRQ %d from %x to %lx prohibited\n",
             irqn, i->l4_type(), mode);
      return 0;
    }
  return 0;
}

int
Sw_icu::unmask_irq(l4_msgtag_t /*tag*/, unsigned irqn)
{
  Irq_set::iterator i = _irqs.find(irqn);
  if (i == _irqs.end())
    return -L4_ENOENT;

  if (!i->unmask_via_icu())
    return -L4_EINVAL;

  return i->unmask();
}


bool
Sw_icu::irqs_allocated(Adr_resource const *r)
{
  for (unsigned n = r->_data().start(); n <= r->_data().end(); ++n)
    {
      if (_irqs.find(n) == _irqs.end())
	return false;
    }

  return true;
}

bool
Sw_icu::add_irqs(Adr_resource const *r)
{
  for (unsigned n = r->_data().start(); n <= r->_data().end(); ++n)
    {
      if (_irqs.find(n) != _irqs.end())
	continue;

      Sw_irq_pin *irq = new Sw_irq_pin(real_irq(n), n, r->flags());
      _irqs.insert(irq);
    }
  return true;
}


int
Sw_icu::dispatch(l4_umword_t /*obj*/, L4::Ipc_iostream &ios)
{
  l4_umword_t op, irqn;
  L4::Snd_fpage irqc;
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
Sw_icu::dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc_iostream &ios)
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

int
Sw_icu::Real_irq_pin::unbind()
{
  unsigned n = this - real_irq(0);
  if (n & 0x80)
    n = (n - 0x80) | L4::Icu::F_msi;

  int err = l4_error(system_icu()->icu->unbind(n, irq()));
  set_shareable(false);
  return err;
}

int
Sw_icu::Real_irq_pin::bind(L4::Cap<L4::Irq> irq, unsigned mode)
{
  unsigned n = this - real_irq(0);
  if (n & 0x80)
    n = (n - 0x80) | L4::Icu::F_msi;

  int err = l4_error(system_icu()->icu->bind(n, irq));

  // allow sharing if IRQ must be acknowledged via the IRQ object 
  if (err == 0)
    set_shareable(true);

  if (err < 0)
    return err;

  // printf(" IRQ[%x]: mode=%x ... ", n, mode);
  err = l4_error(system_icu()->icu->set_mode(n, mode));
  // printf("result=%d\n", err);

  return err;
}

int
Sw_icu::Real_irq_pin::unmask()
{
  unsigned n = this - real_irq(0);
  if (n & 0x80)
    n = (n - 0x80) | L4::Icu::F_msi;

  system_icu()->icu->unmask(n);
  return -L4_EINVAL;
}

unsigned
Sw_icu::Sw_irq_pin::l4_type() const
{
  unsigned m = type();
  unsigned r = (m & S_irq_type_mask) / Adr_resource::Irq_info_factor;
  return r;
}

void
Sw_icu::Sw_irq_pin::allocate_master_irq()
{
  assert (_master->shared());
  Auto_cap<L4::Irq>::Cap lirq = chkcap(L4Re::Util::cap_alloc.alloc<L4::Irq>(),
      "allocating IRQ capability");
  // printf("IRQ mode = %x -> %x\n", type(), l4_type());
  chksys(L4Re::Env::env()->factory()->create_irq(lirq.get()), "allocating IRQ");
  chksys(_master->bind(lirq.get(), l4_type()), "binding IRQ");
  _master->_irq  = lirq.release();
  _master->set_chained(true);
}

#if 0
int
Sw_icu::Sw_irq_pin::share(Auto_cap<L4::Irq>::Cap const &irq)
{
  if (!_master->shareable())
    {
      printf("WARNING: IRQ %d cannot be shared\n", irqn());
      return -L4_EINVAL;
    }

  try
    {
      // the second irq shall be attached to a single hw irq
      printf("IRQ %d -> proxy -> 2 clients\n", irqn());
      Auto_cap<L4::Irq>::Cap lirq(L4Re::Util::cap_alloc.alloc<L4::Irq>());
      if (!lirq.get().is_valid())
	return -L4_ENOMEM;

      int err = l4_error(L4Re::Env::env()->factory()->create_irq(lirq.get()));

      if (err < 0)
	return err;

      //enter_kdebug("XX");
      // XXX: level low is a hack
      L4Re::chksys(lirq->chain(l4_umword_t(_master), L4::Irq::F_level_low, _master->_irq));
      L4Re::chksys(lirq->chain(l4_umword_t(_master), L4::Irq::F_level_low, irq.get()));
      L4Re::chksys(lirq->set_mode(L4::Irq::F_level_low), "set mode");
      L4Re::chksys(err = _master->bind(lirq.get()));
      //enter_kdebug("XX");

      // do the internal list enqueuing and resource management
      _state |= S_bound;
      _master->_irq = lirq.release();
      _master->_sw_irqs++;
      _master->set_chained(true);
      _irq = irq;
      return 0;
    }
  catch (L4::Runtime_error const &e)
    {
      _master->bind(_master->_irq);
      return e.err_no();
    }
}
#endif

int
Sw_icu::Sw_irq_pin::bind(L4::Cap<void> rc)
{
  if (_irq.is_valid())
    {
      if (_irq.get().validate(L4Re::This_task).label() > 0)
	return -L4_EEXIST;

      _unbind();
    }
#if 0
  Real_irq_pin *ri;

  if (!(ri = real_irq(irqn())))
    return -L4_EPERM;
#endif
  if (bound())
    return -L4_EPERM;

  Auto_cap<L4::Irq>::Cap irq =
    chkcap(L4Re::Util::cap_alloc.alloc<L4::Irq>(), "allocating IRQ capability");

  irq.get().move(L4::cap_cast<L4::Irq>(rc));

  if (_master->shared() && !_master->chained() && _master->_sw_irqs == 0)
    {
      allocate_master_irq();
      assert (_master->chained());
    }

  if (!_master->chained())
    {
      // the first irq shall be attached to a hw irq
      // printf("IRQ %d -> client\n", irqn());
      // printf("IRQ mode = %x -> %x\n", type(), l4_type());
      int err = _master->bind(irq.get(), l4_type());
      if (err < 0)
	return err;

      _irq = irq;
      _master->_irq = _irq.get();
      ++_master->_sw_irqs;
      _state |= S_bound;
      if (err == 1)
	_state |= S_unmask_via_icu;

      // printf("  bound irq %u -> err=%d\n", irqn(), err);
      return err;
    }

  // printf("IRQ %d -> proxy -> %d clients\n", irqn(), _master->_sw_irqs + 1);
  L4Re::chksys(_master->_irq->chain(l4_umword_t(_master), irq.get()), "attach");
  _irq = irq;
  _master->_sw_irqs++;

  return 0;
}

int
Sw_icu::Sw_irq_pin::_unbind()
{
  int err = 0;
  --(_master->_sw_irqs);
  if (_master->_sw_irqs == 0)
    {
      if (_master->chained())
	L4Re::Util::cap_alloc.free(_master->_irq);

      _master->_irq = L4::Cap<L4::Irq>::Invalid;
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

  if (!_master->_sw_irqs)
    return -L4_EINVAL;

  return _unbind();
}

}
