/*
 * (c) 2011 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#pragma once

#include <l4/sys/irq>

class Io_irq_pin
{
public:
  enum Flags
  {
    F_shareable = 0x1,
    F_chained   = 0x2,
  };

private:
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

public:
  L4::Cap<L4::Irq> irq() const { return _irq; }
  void irq(L4::Cap<L4::Irq> i) { _irq = i; }

  Io_irq_pin() : _sw_irqs(0), _irq(), _flags(0), _max_sw_irqs(0) {}

  void set_shareable(bool s)
  { chg_flags(s, F_shareable); }

  void set_chained(bool s)
  { chg_flags(s, F_chained); }

public:
  void add_sw_irq() { ++_max_sw_irqs; }
  int sw_irqs() const { return _sw_irqs; }
  void inc_sw_irqs() { ++_sw_irqs; }
  void dec_sw_irqs() { ++_sw_irqs; }
  virtual int bind(L4::Cap<L4::Irq> irq, unsigned mode) = 0;
  virtual int unmask() = 0;
  virtual int unbind() = 0;
  virtual int set_mode(unsigned mode) = 0;
  virtual ~Io_irq_pin() {}

  bool shared() const { return _max_sw_irqs > 1; }
  bool shareable() const { return _flags & F_shareable; }
  bool chained() const { return _flags & F_chained; }
};

class Kernel_irq_pin : public Io_irq_pin
{
private:
  unsigned _idx;
public:
  Kernel_irq_pin(unsigned idx) : Io_irq_pin(), _idx(idx) {}
  int bind(L4::Cap<L4::Irq> irq, unsigned mode);
  int unmask();
  int unbind();
  int set_mode(unsigned mode);

  unsigned pin() const { return _idx; }
};

