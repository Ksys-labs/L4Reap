INTERFACE:

#include <types.h>
#include "initcalls.h"
#include <spin_lock.h>

class Acpi_madt;

class Io_apic_entry
{
  friend class Io_apic;
private:
  Unsigned64 _e;

public:
  enum Delivery { Fixed, Lowest_prio, SMI, NMI = 4, INIT, ExtINT = 7 };
  enum Dest_mode { Physical, Logical };
  enum Polarity { High_active, Low_active };
  enum Trigger { Edge, Level };

  Io_apic_entry() {}
  Io_apic_entry(Unsigned8 vector, Delivery d, Dest_mode dm, Polarity p,
                Trigger t, Unsigned8 dest)
    : _e(vector | (d << 8) | (dm << 11) | (p << 13) | (t << 15) | (1<<16)
         | (((Unsigned64)dest) << 56))
  {}

  unsigned delivery() const { return (_e >> 8) & 7; }
  void delivery(unsigned m) { _e = (_e & ~(7 << 8)) | ((m & 7) << 8); }
  unsigned dest_mode() const { return _e & (1 << 11); }
  void dest_mode(bool p) { _e = (_e & ~(1<<11)) | ((unsigned long)p << 11); }
  unsigned dest() const { return _e >> 56; }
  void dest(unsigned d) { _e = (_e & ~(0xffULL << 56)) | ((Unsigned64)d << 56); }
  unsigned mask() const { return _e & (1U << 16); }
  void mask(bool m) { _e = (_e & ~(1ULL << 16)) | ((Unsigned64)m << 16); }
  unsigned trigger() const { return _e & (1 << 15); }
  unsigned polarity() const { return _e & (1 << 13); }
  unsigned vector() const { return _e & 0xff; }
  void vector(Unsigned8 v) { _e = (_e & ~0xff) | v; }
  void trigger(Mword tr) { _e = (_e & ~(1UL << 15)) | ((tr & 1) << 15); }
  void polarity(Mword pl) { _e = (_e & ~(1UL << 13)) | ((pl & 1) << 13); }
} __attribute__((packed));


class Io_apic
{
public:
  enum { Max_ioapics = 6 };

private:
  struct Apic
  {
    Unsigned32 volatile adr;
    Unsigned32 dummy[3];
    Unsigned32 volatile data;
  } __attribute__((packed));

  Apic *_apic;
  Spin_lock<> _l;
  unsigned _offset;

  static Io_apic _apics[Max_ioapics];
  static unsigned _nr_irqs;
  static Acpi_madt const *_madt;
};

IMPLEMENTATION:

#include "acpi.h"
#include "kmem.h"
#include "kdb_ke.h"
#include "kip.h"
#include "lock_guard.h"

Io_apic Io_apic::_apics[Io_apic::Max_ioapics];
Acpi_madt const *Io_apic::_madt;
unsigned Io_apic::_nr_irqs;


PRIVATE inline NEEDS["lock_guard.h"]
Mword
Io_apic::read(int reg)
{
  Lock_guard<typeof(_l)> g(&_l);
  _apic->adr = reg;
  asm volatile ("": : :"memory");
  return _apic->data;
}

PRIVATE inline NEEDS["lock_guard.h"]
void
Io_apic::modify(int reg, Mword set_bits, Mword del_bits)
{
  register Mword tmp;
  Lock_guard<typeof(_l)> g(&_l);
  _apic->adr = reg;
  asm volatile ("": : :"memory");
  tmp = _apic->data;
  tmp &= ~del_bits;
  tmp |= set_bits;
  _apic->data = tmp;
}

PRIVATE inline NEEDS["lock_guard.h"]
void
Io_apic::write(int reg, Mword value)
{
  Lock_guard<typeof(_l)> g(&_l);
  _apic->adr = reg;
  asm volatile ("": : :"memory");
  _apic->data = value;
}

PRIVATE inline
unsigned
Io_apic::num_entries()
{
  return (read(1) >> 16) & 0xff;
}


PUBLIC inline NEEDS["kdb_ke.h"]
Io_apic_entry
Io_apic::read_entry(unsigned i)
{
  Io_apic_entry e;
  //assert_kdb(i <= num_entries());
  e._e = (Unsigned64)read(0x10+2*i) | (((Unsigned64)read(0x11+2*i)) << 32);
  return e;
}


PUBLIC inline NEEDS["kdb_ke.h", Io_apic::write]
void
Io_apic::write_entry(unsigned i, Io_apic_entry const &e)
{
  //assert_kdb(i <= num_entries());
  write(0x10+2*i, e._e);
  write(0x11+2*i, e._e >> 32);
}

PUBLIC static FIASCO_INIT
bool
Io_apic::init()
{
  _madt = Acpi::find<Acpi_madt const *>("APIC");

  if (_madt == 0)
    {
      printf("Could not find APIC in RSDT nor XSDT, skipping init\n");
      return false;
    }
  printf("IO-APIC: MADT = %p\n", _madt);

  int n_apics = 0;

  for (n_apics = 0;
       Acpi_madt::Io_apic const *ioapic = static_cast<Acpi_madt::Io_apic const *>(_madt->find(Acpi_madt::IOAPIC, n_apics));
       ++n_apics)
    {

      if (n_apics >= Max_ioapics)
	{
	  printf("Maximum number of IO-APICs exceeded ignore further IO-APICs\n");
	  break;
	}

      printf("IO-APIC[%2d]: struct: %p adr=%x\n", n_apics, ioapic, ioapic->adr);

      Address offs;
      Address va = Mem_layout::alloc_io_vmem(Config::PAGE_SIZE);
      assert (va);

      Kmem::map_phys_page(ioapic->adr, va, false, true, &offs);

      Kip::k()->add_mem_region(Mem_desc(ioapic->adr, ioapic->adr + Config::PAGE_SIZE -1, Mem_desc::Reserved));


      Io_apic *apic = Io_apic::apic(n_apics);
      apic->_apic = (Io_apic::Apic*)(va + offs);
      apic->write(0, 0);
      unsigned ne = apic->num_entries();
      apic->_offset = _nr_irqs;
      _nr_irqs += ne + 1;

      for (unsigned i = 0; i <= ne; ++i)
        {
          int v = 0x20+i;
          Io_apic_entry e(v, Io_apic_entry::Fixed, Io_apic_entry::Physical,
              Io_apic_entry::High_active, Io_apic_entry::Edge, 0);
          apic->write_entry(i, e);
        }

      printf("IO-APIC[%2d]: pins %u\n", n_apics, ne);
      apic->dump();
    }

  if (!n_apics)
    {
      printf("IO-APIC: Could not find IO-APIC in MADT, skip init\n");
      return false;
    }


  printf("IO-APIC: dual 8259: %s\n", _madt->apic_flags & 1 ? "yes" : "no");

  for (unsigned tmp = 0;;++tmp)
    {
      Acpi_madt::Irq_source const *irq
	= static_cast<Acpi_madt::Irq_source const *>(_madt->find(Acpi_madt::Irq_src_ovr, tmp));

      if (!irq)
	break;

      printf("IO-APIC: ovr[%2u] %02x -> %x\n", tmp, irq->src, irq->irq);
    }

  return true;
};

PUBLIC static
unsigned
Io_apic::total_irqs()
{ return _nr_irqs; }

PUBLIC static 
unsigned
Io_apic::legacy_override(unsigned i)
{
  if (!_madt)
    return i;

  unsigned tmp = 0;
  for (;;++tmp)
    {
      Acpi_madt::Irq_source const *irq
	= static_cast<Acpi_madt::Irq_source const *>(_madt->find(Acpi_madt::Irq_src_ovr, tmp));

      if (!irq)
	break;

      if (irq->src == i)
	return irq->irq;
    }
  return i;
}

PUBLIC
void
Io_apic::dump()
{
  unsigned ne = num_entries();
  for (unsigned i = 0; i <= ne; ++i)
    {
      Io_apic_entry e = read_entry(i);
      printf("  PIN[%2u%c]: vector=%2x, del=%u, dm=%s, dest=%u (%s, %s)\n",
	     i, e.mask() ? 'm' : '.',
	     e.vector(), e.delivery(), e.dest_mode() ? "logical" : "physical",
	     e.dest(),
	     e.polarity() ? "low" : "high",
	     e.trigger() ? "level" : "edge");
    }

}

PUBLIC static inline
bool
Io_apic::active()
{ return _apics[0]._apic; }

PUBLIC inline
bool
Io_apic::valid() const { return _apic; }

PUBLIC inline NEEDS["kdb_ke.h", Io_apic::modify]
void
Io_apic::mask(unsigned irq)
{
  //assert_kdb(irq <= _apic->num_entries());
  modify(0x10 + irq * 2, 1UL << 16, 0);
}

PUBLIC inline NEEDS["kdb_ke.h", Io_apic::modify]
void
Io_apic::unmask(unsigned irq)
{
  //assert_kdb(irq <= _apic->num_entries());
  modify(0x10 + irq * 2, 0, 1UL << 16);
}

PUBLIC inline NEEDS["kdb_ke.h", Io_apic::read]
bool
Io_apic::masked(unsigned irq)
{
  //assert_kdb(irq <= _apic->num_entries());
  return read(0x10 + irq * 2) & (1UL << 16);
}

PUBLIC inline NEEDS[Io_apic::read]
void
Io_apic::sync()
{
  (void)_apic->data;
}

PUBLIC inline NEEDS["kdb_ke.h", Io_apic::modify]
void
Io_apic::set_dest(unsigned irq, Mword dst)
{
  //assert_kdb(irq <= _apic->num_entries());
  modify(0x11 + irq * 2, dst & (~0UL << 24), ~0UL << 24);
}

PUBLIC inline NEEDS[Io_apic::num_entries]
unsigned
Io_apic::nr_irqs()
{ return num_entries() + 1; }

PUBLIC inline
unsigned
Io_apic::gsi_offset() const { return _offset; }

PUBLIC static inline
Io_apic *
Io_apic::apic(unsigned idx)
{ return &_apics[idx]; }

PUBLIC static
unsigned
Io_apic::find_apic(unsigned irqnum)
{
  for (unsigned i = Max_ioapics; i > 0; --i)
    {
      if (_apics[i-1]._apic && _apics[i-1]._offset < irqnum)
	return i - 1;
    }
  return 0;
};

