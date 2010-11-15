/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/mag/server/input_driver>

#include <l4/re/namespace>
#include <l4/re/rm>
#include <l4/re/env>
#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>
#include <l4/cxx/exceptions>

#include <cstdio>

namespace Mag_server {

using L4Re::Util::Auto_cap;
using L4Re::chksys;


struct Iter
{
  L4Re::Event_buffer *_ev;
  L4Re::Event_buffer::Event *e;

  explicit Iter() : _ev(0), e(0) {}
  explicit Iter(L4Re::Event_buffer *ev) : _ev(ev), e(_ev->next()) {}

  Iter operator ++ ()
  {
    e = _ev->next();
    return *this;
  }

  bool operator != (Iter const &o) const
  { return e != o.e; }

  L4Re::Event_buffer::Event *operator -> () const { return e; }
  L4Re::Event_buffer::Event &operator * () const { return *e; }
};

struct Emit
{
  User_state *u;
  Emit(User_state *u) : u(u) {}
  void operator () (L4Re::Event_buffer::Event const &e) const
  { u->handle_event(e); }
};

class Input_driver_lxproxy : public Input_driver, public Input_source
{
private:
  Auto_cap<L4Re::Dataspace>::Cap _ev_ds;
  Auto_cap<L4::Irq>::Cap _ev_irq;
  L4Re::Rm::Auto_region<void *> _ev_ds_m;

  L4Re::Event_buffer _ev;

public:

  Input_driver_lxproxy() : Input_driver("L4Linux Proxy") {}
  void start(Core_api *core)
  {
    try
      {
	_ev_ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
	_ev_irq = L4Re::Util::cap_alloc.alloc<L4::Irq>();

	L4Re::Env const *e = L4Re::Env::env();
	L4::Cap<L4Re::Namespace> input_ns
	  = chkcap(e->get_cap<L4Re::Namespace>("ev"), "getting ev namespace", 0);
	chksys(input_ns->query("ev_buf", _ev_ds.get()));
	chksys(input_ns->query("ev_irq", _ev_irq.get()));
	chksys(e->rm()->attach(&_ev_ds_m, _ev_ds->size(), L4Re::Rm::Search_addr,
	      _ev_ds.get(), 0, L4_PAGESHIFT));

	_ev = L4Re::Event_buffer(_ev_ds_m.get(), _ev_ds->size());
	_core = core;
	core->add_input_source(this);
	printf("LXDD: buffer @%p\n", _ev_ds_m.get());
      }
    catch (...)
      {
	printf("could not find linux proxy input\n");
      }
  }

  void poll_events()
  {
    enum { Ax = 10 };

    //Motion_merger<Ax> mm;
    Motion_fwd mm;
    mm.process/*<L4Re::Event_buffer::Event>*/(Iter(&_ev), Iter(), Emit(_core->user_state()));
  }
};

static Input_driver_lxproxy _lxpinput;
}
