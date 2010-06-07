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
private:
  Unsigned32 volatile adr;
  Unsigned32 dummy[3];
  Unsigned32 volatile data;

  static Spin_lock _l;
  static Io_apic *_apic;
  static Acpi_madt const *_madt;
} __attribute__((packed));

IMPLEMENTATION:

#include "acpi.h"
#include "kmem.h"
#include "kdb_ke.h"
#include "kip.h"
#include "lock_guard.h"

Io_apic *Io_apic::_apic;
Spin_lock Io_apic::_l;
Acpi_madt const *Io_apic::_madt;



PRIVATE inline NEEDS["lock_guard.h"]
Mword
Io_apic::read(int reg)
{
  Lock_guard<Spin_lock> g(&_l);
  adr = reg;
  asm volatile ("": : :"memory");
  return data;
}

PRIVATE inline NEEDS["lock_guard.h"]
void
Io_apic::write(int reg, Mword value)
{
  Lock_guard<Spin_lock> g(&_l);
  adr = reg;
  asm volatile ("": : :"memory");
  data = value;
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


PUBLIC inline NEEDS["kdb_ke.h"]
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
  printf("MADT = %p\n", _madt);

  Acpi_madt::Io_apic const *ioapic
    = static_cast<Acpi_madt::Io_apic const *>(_madt->find(Acpi_madt::IOAPIC, 0));

  if (!ioapic)
    {
      printf("IO-APIC: Could not find IO-APIC in MADT, skip init\n");
      return false;
    }

  printf("IO-APIC: struct: %p adr=%x\n", ioapic, ioapic->adr);
  printf("IO-APIC: dual 8259: %s\n", _madt->apic_flags & 1 ? "yes" : "no");

  unsigned tmp = 0;
  for (;;++tmp)
    {
      Acpi_madt::Irq_source const *irq
	= static_cast<Acpi_madt::Irq_source const *>(_madt->find(Acpi_madt::Irq_src_ovr, tmp));

      if (!irq)
	break;

      printf("IO-APIC: ovr[%2u] %02x -> %x\n", tmp, irq->src, irq->irq);
    }

  if (tmp)
    printf("IO-APIC: NOTE IRQ overrides are ignored!\n");

  Address offs;
  Kmem::map_phys_page(ioapic->adr, Mem_layout::Io_apic_page,
		      false, true, &offs);

  Kip::k()->add_mem_region(Mem_desc(ioapic->adr, ioapic->adr + Config::PAGE_SIZE -1, Mem_desc::Reserved));


  Io_apic *apic = (Io_apic*)(Mem_layout::Io_apic_page + offs);
  _apic = apic;
  apic->write(0, 0);
  unsigned ne = apic->num_entries();

  for (unsigned i = 0; i <= ne; ++i)
    {
      int v = 0x20+i;
      Io_apic_entry e(v, Io_apic_entry::Fixed, Io_apic_entry::Physical,
                      Io_apic_entry::High_active, Io_apic_entry::Edge, 0);
      apic->write_entry(i, e);
    }

  printf("IO-APIC: pins %u\n", ne);
  dump();

  return true;
};

PUBLIC static 
unsigned
Io_apic::legacy_override(unsigned i)
{
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

PUBLIC static
void
Io_apic::dump()
{
  unsigned ne = _apic->num_entries();
  for (unsigned i = 0; i <= ne; ++i)
    {
      Io_apic_entry e = _apic->read_entry(i);
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
{ return _apic; }

PUBLIC static inline NEEDS["kdb_ke.h", Io_apic::read,Io_apic::write]
void
Io_apic::mask(unsigned irq)
{
  //assert_kdb(irq <= _apic->num_entries());
  _apic->write(0x10 + irq * 2, _apic->read(0x10 + irq * 2) | (1UL << 16));
}

PUBLIC static inline NEEDS["kdb_ke.h", Io_apic::read,Io_apic::write]
void
Io_apic::unmask(unsigned irq)
{
  //assert_kdb(irq <= _apic->num_entries());
  _apic->write(0x10 + irq * 2, _apic->read(0x10 + irq * 2) & ~(1UL << 16));
}

PUBLIC static inline NEEDS["kdb_ke.h", Io_apic::read]
bool
Io_apic::masked(unsigned irq)
{
  //assert_kdb(irq <= _apic->num_entries());
  return _apic->read(0x10 + irq * 2) & (1UL << 16);
}

PUBLIC static inline NEEDS["kdb_ke.h", Io_apic::read,Io_apic::write]
void
Io_apic::set_dest(unsigned irq, Mword dst)
{
  //assert_kdb(irq <= _apic->num_entries());
  _apic->write(0x11 + irq * 2, (_apic->read(0x11 + irq * 2) & ~(~0UL << 24)) | (dst & (~0UL << 24)));
}

PUBLIC static inline NEEDS[Io_apic::num_entries]
unsigned
Io_apic::nr_irqs()
{ return _apic->num_entries() + 1; }

PUBLIC static inline
Io_apic *
Io_apic::apic()
{ return _apic; }

