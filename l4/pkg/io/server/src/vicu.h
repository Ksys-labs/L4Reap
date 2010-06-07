/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/capability>
#include <l4/sys/irq>
#include <l4/sys/icu>

#include <l4/cxx/ipc_server>
#include <l4/cxx/avl_tree>
#include <l4/cxx/list>

#include <l4/re/util/cap_alloc>

#include "vdevice.h"

namespace Vi {

class Sw_icu : public Device, public Dev_feature, public L4::Server_object
{
public:
  Sw_icu();
  virtual ~Sw_icu();

  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);

  char const *hid() const { return "L40009"; }
  int dispatch(l4_umword_t, l4_uint32_t func, L4::Ipc_iostream &ios);
  bool match_hw_feature(Hw::Dev_feature const *) const { return false; }

  bool add_irqs(Adr_resource const *r);
  bool irqs_allocated(Adr_resource const *r);

private:
  int bind_irq(l4_msgtag_t tag, unsigned irqn, L4::Snd_fpage const &irqc);
  int unbind_irq(l4_msgtag_t tag, unsigned irqn, L4::Snd_fpage const &irqc);
  int unmask_irq(l4_msgtag_t tag, unsigned irqn);
  int set_mode(l4_msgtag_t tag, unsigned irqn, l4_umword_t mode);

  class Sw_irq_pin;

  class Io_irq_pin
  {
  public:
    enum Flags
    {
      F_shareable = 0x1,
      F_chained   = 0x2,
    };

  private:
    friend class Sw_irq_pin;
    int _sw_irqs;
    L4::Cap<L4::Irq> _irq;
    unsigned short _flags;
    unsigned short _max_sw_irqs;

    void chg_flags(bool set, unsigned flags)
    {
      if (set)
	_flags |= flags;
      else
	_flags &= ~flags;
    }

  protected:
    L4::Cap<L4::Irq> irq() const { return _irq; }

    Io_irq_pin() : _sw_irqs(0), _irq(), _flags(0), _max_sw_irqs(0) {}

    void set_shareable(bool s)
    { chg_flags(s, F_shareable); }

    void set_chained(bool s)
    { chg_flags(s, F_chained); }

  public:
    void add_sw_irq() { ++_max_sw_irqs; }
    virtual int bind(L4::Cap<L4::Irq> irq, unsigned mode) = 0;
    virtual int unmask() = 0;
    virtual int unbind() = 0;
    virtual ~Io_irq_pin() {}

    bool shared() const { return _max_sw_irqs > 1; }
    bool shareable() const { return _flags & F_shareable; }
    bool chained() const { return _flags & F_chained; }
  };

  class Sw_irq_pin : public cxx::Avl_tree_node
  {
  private:
    enum
    {
      S_bound = 1,
      S_unmask_via_icu = 2,
    };

    unsigned _state;
    unsigned _irqn;
    Io_irq_pin *_master;
    L4Re::Util::Auto_cap<L4::Irq>::Cap _irq;

  public:
    enum Irq_type
    {
      S_irq_type_level = Adr_resource::Irq_level,
      S_irq_type_edge  = Adr_resource::Irq_edge,
      S_irq_type_high  = Adr_resource::Irq_high,
      S_irq_type_low   = Adr_resource::Irq_low,
      S_irq_type_mode_mask = S_irq_type_level | S_irq_type_edge,
      S_irq_type_polarity_mask = S_irq_type_high | S_irq_type_low,
      S_irq_type_mask  = S_irq_type_level | S_irq_type_edge
                         | S_irq_type_high | S_irq_type_low,
    };

    typedef unsigned Key_type;

    static unsigned key_of(Sw_irq_pin const *o) { return o->_irqn; }

    Sw_irq_pin(Io_irq_pin *master, unsigned irqn, unsigned flags)
    : _state(flags & S_irq_type_mask), _irqn(irqn), _master(master)
    {
      master->add_sw_irq();
    }

    unsigned irqn() const { return _irqn; }
    L4::Cap<L4::Irq> irq() const { return _irq.get(); }

    bool bound() const { return _state & S_bound; }
    bool unmask_via_icu() const { return _state & S_unmask_via_icu; }
    unsigned type() const { return _state & S_irq_type_mask; }
    unsigned l4_type() const;
    int bind(L4::Cap<void> rc);
    int unmask() { return _master->unmask(); }
    int unbind();
    int trigger() const;

  protected:
    int _unbind();
//    int share(L4Re::Util::Auto_cap<L4::Irq>::Cap const &irq);
    void allocate_master_irq();
  };


  class Real_irq_pin : public Io_irq_pin
  {
  public:
    Real_irq_pin() : Io_irq_pin() {}
    int bind(L4::Cap<L4::Irq> irq, unsigned mode);
    int unmask();
    int unbind();
  };

  static Real_irq_pin *real_irq(unsigned n);
  static Real_irq_pin _real_irqs[];


  typedef cxx::Avl_tree<Sw_irq_pin, Sw_irq_pin> Irq_set;
  Irq_set _irqs;
  //cxx::Bitmap<512> _allowed;
public:
  static void *irq_loop(void*);
};

}
